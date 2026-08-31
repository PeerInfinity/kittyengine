#include "MyApp.h"
#include "os_core.h"

// ===== P0prime: headless oracle mode (LOCAL investigation branch) =====
#include "World.h"
#include "Player.h"
#include "Game.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

bool gOracleActive=false;     // read by World.cpp's input poll
int  gOracleHeld=0;           // bit0 L, 1 R, 2 jump, 3 shoot, 4 rocket, 5 rocketup  (held this tick)
int  gOraclePressed=0;        // same bits, but only on the rising edge (IsKeyPressed semantics)
// ======================================================================

MyApp gApp;

void AppThread(void *theArg)
{
	gApp.Go();
}

// ===== P0prime oracle driver =====
#define ORACLE_MAXTICKS 2000000
static unsigned char *gTape=NULL;   // one held-mask per tick

static int OracleBit(const char *n)
{
	if (!strcmp(n,"left"))     return 1;
	if (!strcmp(n,"right"))    return 2;
	if (!strcmp(n,"jump"))     return 4;
	if (!strcmp(n,"shoot"))    return 8;
	if (!strcmp(n,"rocket"))   return 16;
	if (!strcmp(n,"rocketup")) return 32;
	return 0;
}

// tape CSV: one span per line, "button,fromTick,toTick" (inclusive), '#' comments
static void OracleLoadTape(const char *theFile, int theTicks)
{
	gTape=(unsigned char*)calloc(theTicks+2,1);
	if (!theFile) return;
	FILE *f=fopen(theFile,"r");
	if (!f) {fprintf(stderr,"oracle: cannot open tape %s\n",theFile);exit(2);}
	char aLine[512];
	while (fgets(aLine,sizeof(aLine),f))
	{
		if (aLine[0]=='#' || aLine[0]=='\n') continue;
		char aName[64];int aFrom=0,aTo=0;
		if (sscanf(aLine,"%63[^,],%d,%d",aName,&aFrom,&aTo)!=3) continue;
		int aBit=OracleBit(aName);
		if (!aBit) {fprintf(stderr,"oracle: unknown button '%s'\n",aName);exit(2);}
		for (int t=aFrom;t<=aTo && t<theTicks;t++) if (t>=0) gTape[t]|=aBit;
	}
	fclose(f);
}

// The oracle runs on the SAME app thread the shipped main() uses (OS_Core::Thread);
// main() keeps pumping OS messages, exactly as in the normal build.
static const char *gO_Level="data://NOVICELEVEL.kitty";
static const char *gO_Tape=NULL;
static const char *gO_Out=NULL;
static int gO_Ticks=1000;
static int gO_Seed=-1;
static int gO_WarmCap=20000;
static int gO_Draw=0;

static void OracleThread(void *theArg)
{
	FILE *aF=stdout;
	if (gO_Out) {aF=fopen(gO_Out,"w");if (!aF){fprintf(stderr,"oracle: cannot write %s\n",gO_Out);_exit(2);}}

	gApp.Go(false);                       // full init, but WE drive the updates

	// The asset loader completes inside App::Throttle() (mRunLoadBackgroundComplete),
	// NOT inside ThrottleUpdate() -- so warm-up must use the real frame controller.
	int aWarm=0;
	for (;aWarm<gO_WarmCap;aWarm++)
	{
		gApp.Throttle();
		if (gApp.mLoadComplete) break;
		OS_Core::Sleep(1);
	}
	fprintf(stderr,"oracle: load complete after %d warm iterations (mLoadComplete=%d)\n",aWarm,(int)gApp.mLoadComplete);

	if (gO_Seed>=0) gRand.Seed(gO_Seed);

	// NOTE: GoNewGame() only CONSTRUCTS the Game CPU. In the shipped game it is the
	// Transition CPU (MainMenu.cpp:2049) that calls AddCPU on it -- without that the
	// Game/World never enters the app's CPU tree and never updates.
	CPU *aGameCPU=gApp.GoNewGame(gO_Level);
	if (aGameCPU) gApp.AddCPU(aGameCPU);

	int aSettle=0;
	for (;aSettle<400;aSettle++)
	{
		gApp.Throttle();
		if (gWorld && gWorld->mRobot) break;
		OS_Core::Sleep(1);
	}
	if (!gWorld || !gWorld->mRobot)
	{
		fprintf(stderr,"oracle: FAILED to reach a world/robot for [%s]\n",gO_Level);
		_exit(3);
	}
	fprintf(stderr,"oracle: world %dx%d, robot at (%.4f,%.4f) after %d settle iterations\n",
		gWorld->mGridWidth,gWorld->mGridHeight,gWorld->mRobot->mPos.mX,gWorld->mRobot->mPos.mY,aSettle);

	fprintf(aF,"# rwk-oracle level=%s tape=%s ticks=%d seed=%d\n",gO_Level,gO_Tape?gO_Tape:"(none)",gO_Ticks,gO_Seed);
	fprintf(aF,"tick,held,x,y,speed,gravity,gmod,touchingGround,jumpKludge,inJump,doubleJump,rocketCd,timer,died,win\n");

	unsigned int aT0=OS_Core::Tick();
	int aPrevHeld=0;
	for (int t=0;t<gO_Ticks;t++)
	{
		int aHeld=gTape?gTape[t]:0;
		gOracleHeld=aHeld;
		gOraclePressed=aHeld&~aPrevHeld;
		aPrevHeld=aHeld;

		gApp.ThrottleUpdate();            // ONE logical tick, no wall clock involved
		if (gO_Draw) gApp.ThrottleDraw();

		if (!gWorld || !gWorld->mRobot) {fprintf(stderr,"oracle: world vanished at tick %d\n",t);break;}
		Robot *r=gWorld->mRobot;
		fprintf(aF,"%d,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%d,%d,%d,%d,%d,%d,%d\n",
			t,aHeld,r->mPos.mX,r->mPos.mY,r->mSpeed,r->mGravity,r->mGMod,
			r->mTouchingGround,r->mJumpKludge,(int)r->mInJump,r->mDoubleJumpCount,
			r->mRocketCountdown,gWorld->mTimer,(int)gWorld->mDied,(int)gWorld->mWin);
	}
	unsigned int aT1=OS_Core::Tick();
	if (aF!=stdout) fclose(aF);
	fprintf(stderr,"oracle: %d ticks stepped in %u ms (%.1f ticks/s)\n",
		gO_Ticks,aT1-aT0,(aT1>aT0)?(gO_Ticks*1000.0/(double)(aT1-aT0)):0.0);
	_exit(0);   // skip the app's shutdown/save path entirely
}


// ===== AUDIO-SDL: --soundtest, a MEASURED audio probe (LOCAL branch only) =====
//
// The physics gate below is blind to audio by construction (see the mutant
// control in the report), and no campaign level carries a BeepBox song, so
// "sounds fine" would otherwise rest entirely on ears.  This mode drives the
// three real RAPT paths and reads their OWN level meters back, so each one is
// a number rather than an impression:
//
//   sample  gSounds->mJump          -> Sound_Core kVoice
//   stream  gSounds->mStream_Music_Title -> kStream (incremental stb_vorbis)
//   dynamic a SoundStreamDynamic with a sine callback -> kDynamic, the
//           BeepBox contract: short* buffer, LENGTH IN BYTES, device rate
//   beepbox the real BeepBox synth, if --song= is given
//
#include "Beepbox.h"
#include "Bundle_Sounds.h"

static volatile int   gST_Calls=0;
static volatile int   gST_LastLen=0;
static volatile int   gST_MinLen=1<<30;
static volatile int   gST_MaxLen=0;
static double         gST_Phase=0;

// A 440 Hz sine, written the way BeepBox writes: theLength is in BYTES.
static void SoundTestCallback(short* theBuffer, unsigned int theLength, void* theExtra)
{
	int aShorts=(int)theLength/2;          // 16-bit
	int aFrames=aShorts/2;                 // stereo interleaved
	gST_Calls++;
	gST_LastLen=(int)theLength;
	if ((int)theLength<gST_MinLen) gST_MinLen=(int)theLength;
	if ((int)theLength>gST_MaxLen) gST_MaxLen=(int)theLength;
	double aRate=(double)Sound_Core::GetFrequency();
	if (aRate<=0) aRate=44100;
	double aStep=2.0*3.14159265358979*440.0/aRate;
	for (int f=0;f<aFrames;f++)
	{
		short aV=(short)(sin(gST_Phase)*12000.0);
		theBuffer[f*2]=aV;theBuffer[f*2+1]=aV;
		gST_Phase+=aStep;
	}
}

static float PeakOver(SoundStream* theS, int theMs)
{
	float aPeak=0;
	for (int i=0;i<theMs/10;i++) {OS_Core::Sleep(10);float aL=theS->GetLevel();if (aL>aPeak) aPeak=aL;}
	return aPeak;
}

static const char* gST_Song=NULL;

static void SoundTestThread(void *theArg)
{
	gApp.Go(false);
	int aWarm=0;
	for (;aWarm<gO_WarmCap;aWarm++) {gApp.Throttle();if (gApp.mLoadComplete) break;OS_Core::Sleep(1);}
	fprintf(stderr,"soundtest: load complete after %d warm iterations\n",aWarm);

	unsigned int aMin=0,aMax=0;Sound_Core::GetSampleRate(aMin,aMax);
	fprintf(stderr,"soundtest: device rate=%u  GetSampleRate=[%u,%u]\n",
			Sound_Core::GetFrequency(),aMin,aMax);

	int aFail=0;

	// --- 1. SAMPLE (a decoded .ogg played through a kVoice) ---
	if (gSounds)
	{
		gAudio.SetSoundVolume(1.0f);
		gSounds->mJump.Play(1.0f);
		float aPeak=0;
		for (int i=0;i<80;i++) {OS_Core::Sleep(10);
			EnumList(SoundInstance,aSI,gSounds->mJump.mBufferList) {float aL=aSI->GetLevel();if (aL>aPeak) aPeak=aL;}}
		fprintf(stderr,"soundtest: SAMPLE  sounds://jump   peak level = %.4f  %s\n",
				aPeak,aPeak>0.001f?"OK":"*** SILENT ***");
		if (aPeak<=0.001f) aFail++;
	}
	else {fprintf(stderr,"soundtest: SAMPLE  *** gSounds is NULL ***\n");aFail++;}

	// --- 2. STREAM (incremental stb_vorbis decode + seek) ---
	if (gSounds)
	{
		SoundStream* aS=&gSounds->mStream_Music_Title;
		gAudio.SetMusicVolume(1.0f);
		aS->SetLooping(true);
		aS->Play(1.0f);
		float aPeak=PeakOver(aS,800);
		fprintf(stderr,"soundtest: STREAM  music_title      peak level = %.4f  pos=%.3fs playing=%d  %s\n",
				aPeak,aS->GetPosition(),(int)aS->IsPlaying(),aPeak>0.001f?"OK":"*** SILENT ***");
		if (aPeak<=0.001f) aFail++;

		// real seek: jump forward and confirm the position moved and audio continues
		aS->SetPosition(10.0f);
		float aAfter=aS->GetPosition();
		float aPeak2=PeakOver(aS,400);
		fprintf(stderr,"soundtest: SEEK    SetPosition(10) -> GetPosition=%.3fs  peak after = %.4f  %s\n",
				aAfter,aPeak2,(aAfter>9.0f && aAfter<11.5f && aPeak2>0.001f)?"OK":"*** BAD ***");
		if (!(aAfter>9.0f && aAfter<11.5f && aPeak2>0.001f)) aFail++;
		aS->Stop();
	}

	// --- 3. DYNAMIC (the BeepBox contract, with a sine instead of a synth) ---
	{
		SoundStreamDynamic aD;
		aD.Load(SoundTestCallback,NULL,2048);
		fprintf(stderr,"soundtest: DYNAMIC handle=%d\n",aD.mHandle);
		aD.Play(1.0f);
		float aPeak=PeakOver(&aD,600);
		fprintf(stderr,"soundtest: DYNAMIC callback calls=%d  len bytes min=%d max=%d last=%d  peak level = %.4f  %s\n",
				gST_Calls,gST_Calls?gST_MinLen:0,gST_MaxLen,gST_LastLen,aPeak,
				(gST_Calls>0 && aPeak>0.001f)?"OK":"*** DEAD ***");
		if (!(gST_Calls>0 && aPeak>0.001f)) aFail++;
		aD.Stop();
	}

	// --- 4. BEEPBOX itself, if a song was supplied ---
	if (gST_Song)
	{
		BeepBox* aBB=new BeepBox;
		aBB->Load(String(gST_Song));
		fprintf(stderr,"soundtest: BEEPBOX loaded=%d handle=%d\n",(int)aBB->IsLoaded(),aBB->mHandle);
		if (aBB->IsLoaded())
		{
			aBB->Play(1.0f);
			float aPeak=PeakOver(aBB,1500);
			fprintf(stderr,"soundtest: BEEPBOX peak level = %.4f  playing=%d  %s\n",
					aPeak,(int)aBB->IsPlaying(),aPeak>0.001f?"OK":"*** SILENT ***");
			if (aPeak<=0.001f) aFail++;
			aBB->Stop();
		}
		else {fprintf(stderr,"soundtest: BEEPBOX *** song string rejected by BeepBoxSong::Load ***\n");aFail++;}
	}

	fprintf(stderr,"soundtest: %s (%d failing check(s))\n",aFail?"FAIL":"ALL PASS",aFail);
	_exit(aFail?1:0);
}

static int SoundTestMain(int argc, char **argv)
{
	for (int i=1;i<argc;i++)
	{
		if (!strncmp(argv[i],"--song=",7)) gST_Song=argv[i]+7;
		else if (!strncmp(argv[i],"--warmcap=",10)) gO_WarmCap=atoi(argv[i]+10);
	}
	OS_Core::Startup();
	OS_Core::SetCommandLine("RWKAUD");
	OS_Core::Thread(SoundTestThread,NULL);
	OS_Core::SetThreadPriority(-.9f);
	while (!gApp.mInitComplete) OS_Core::Sleep(1);
	while (!gApp.IsShutdown()) {OS_Core::Pump();OS_Core::Sleep(1);}
	OS_Core::Shutdown();
	return 0;
}
// ===== end --soundtest =====

static int OracleMain(int argc, char **argv)
{
	for (int i=1;i<argc;i++)
	{
		if (!strncmp(argv[i],"--level=",8))  gO_Level=argv[i]+8;
		else if (!strncmp(argv[i],"--tape=",7))   gO_Tape=argv[i]+7;
		else if (!strncmp(argv[i],"--out=",6))    gO_Out=argv[i]+6;
		else if (!strncmp(argv[i],"--ticks=",8))  gO_Ticks=atoi(argv[i]+8);
		else if (!strncmp(argv[i],"--seed=",7))   gO_Seed=atoi(argv[i]+7);
		else if (!strncmp(argv[i],"--warmcap=",10)) gO_WarmCap=atoi(argv[i]+10);
		else if (!strcmp(argv[i],"--draw")) gO_Draw=1;
	}
	if (gO_Ticks<1 || gO_Ticks>ORACLE_MAXTICKS) gO_Ticks=1000;

	OracleLoadTape(gO_Tape,gO_Ticks);
	gOracleActive=true;

	OS_Core::Startup();
	OS_Core::SetCommandLine("RWKP0");
	OS_Core::Thread(OracleThread,NULL);
	OS_Core::SetThreadPriority(-.9f);

	while (!gApp.mInitComplete) OS_Core::Sleep(1);
	while (!gApp.IsShutdown()) {OS_Core::Pump();OS_Core::Sleep(1);}
	OS_Core::Shutdown();
	return 0;
}
// ===== end P0prime oracle driver =====

//
// Windows entry point...
//
int main(const int argc, char **argv)
{
	// P0prime: headless oracle mode
	for (int i=1;i<argc;i++) if (!strcmp(argv[i],"--oracle")) return OracleMain(argc,argv);
	// AUDIO-SDL: audio probe mode
	for (int i=1;i<argc;i++) if (!strcmp(argv[i],"--soundtest")) return SoundTestMain(argc,argv);

	String aCMD;
	for (int aCount=0;aCount<argc;aCount++)
	{
		if (aCount>0) aCMD+=" ";
		String aFix=argv[aCount];
		aFix.Replace(" ","{::SPACE::}");
		aCMD+=aFix;
	}

#ifdef AppRunsInThread

	//
	// Note: This main tries to emulate the behavior of Cocoa and other visual systems, where
	// the startup and shutdown will be handled by an overarching class.  On those systems, AppThread will
	// usually be invoked in a function within that overarching class.
	//

	OS_Core::Startup();
	OS_Core::SetCommandLine(aCMD);
    OS_Core::Thread(AppThread,NULL);
    OS_Core::SetThreadPriority(-.9f);

	while (!gApp.mInitComplete) OS_Core::Sleep(1);
	while (!gApp.IsShutdown()) {OS_Core::Pump();OS_Core::Sleep(1);}
	OS_Core::Shutdown();
#else

	//
	// Note: This Winmain runs threadless. Use it on systems when you can use something like PeekMessage
	// to push message processing.
	//

	OS_Core::Startup();
	OS_Core::SetCommandLine(aCMD);
	AppThread(NULL);
	OS_Core::Shutdown();

#endif
}
