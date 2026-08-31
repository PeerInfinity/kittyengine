//
// P0" : WASM entry point + a JS-callable step/observe API for the RWK engine.
// Written for this spike; NOT part of the mirror (kept out of source so the
// mirror stays a read-only reference).  The engine boot is modelled on the
// tree's own Main_Win.cpp non-threaded branch plus rapt_app.cpp's comment
// ("On some systems (WASM) we set up the main loop and Stop() elsewhere").
// The stepping half is a port of P0'-s `--oracle` mode in Linux/main.cpp.
//
#include "MyApp.h"
#include "os_core.h"
#include "graphics_core.h"
#include "World.h"
#include "Player.h"
#include "Game.h"
#include "Makermall.h"
#include "rapt_comm.h"
#include "rapt_iobuffer.h"
#include <emscripten.h>
#include <SDL2/SDL.h>
#include "rapt_input.h"
#include <cstdio>
#include <cstring>

MyApp gApp;
void BuildLocalLevels(void *theArg);   // Makermall.cpp:55, no header declares it

// ---- P0' oracle globals (World.cpp declares these extern; Linux/main.cpp owns them) ----
bool gOracleActive=false;   // bit0 L, 1 R, 2 jump, 3 shoot, 4 rocket, 5 rocketup
int  gOracleHeld=0;
int  gOraclePressed=0;

// ---- stubs: declared in WASM/graphics_core.h, never defined in the WASM GL layer ----
// Bodies follow Linux/graphics_core.cpp:1398/1453 (desktop behaviour).
#ifndef RWK_GL11_GLUE
bool Graphics_Core::IsTimeBeforeVSync(unsigned int lastDrawFinished, unsigned int lastDrawDuration) {return true;}
void Graphics_Core::Screenshot() {}
#endif

// ---------------------------------------------------------------------------
// The browser frame loop.  rwk_pause() freezes it so rwk_step() can advance the
// simulation with NO wall clock involved -- the same split P0' uses natively
// (warm up on Throttle(), step on ThrottleUpdate()).
// ---------------------------------------------------------------------------
static int gP0D_Paused=0;
static int gP0D_PrevHeld=0;

static void RWK_Frame(void)
{
	if (gP0D_Paused) return;
	gApp.Throttle();          // FrameController -> OS_Core::Pump + Update/Draw
}

#define OBS_FLOATS 19

extern "C"
{

// bit0 initComplete, bit1 loadComplete, bit2 world, bit3 robot, bit4 paused
EMSCRIPTEN_KEEPALIVE int rwk_state(void)
{
	int r=0;
	if (gApp.mInitComplete) r|=1;
	if (gApp.mLoadComplete) r|=2;
	if (gWorld)             r|=4;
	if (gWorld && gWorld->mRobot) r|=8;
	if (gP0D_Paused)        r|=16;
	return r;
}

EMSCRIPTEN_KEEPALIVE void rwk_pause(int theOn) {gP0D_Paused=theOn?1:0;}

// Take over the six polled keys in World::Update (P0' injection point).
EMSCRIPTEN_KEEPALIVE void rwk_take_input(int theOn) {gOracleActive=theOn?true:false;}

EMSCRIPTEN_KEEPALIVE void rwk_set_input(unsigned int theHeld)
{
	gOracleHeld=(int)theHeld;
	gOraclePressed=(int)theHeld & ~gP0D_PrevHeld;
	gP0D_PrevHeld=(int)theHeld;
}

EMSCRIPTEN_KEEPALIVE void rwk_seed(int theSeed) {gRand.Seed(theSeed);}

// Mirrors P0' OracleThread: GoNewGame CONSTRUCTS the Game CPU; it is the
// Transition CPU that AddCPU's it in the shipped game, so we do that ourselves.
// Returns the number of settle iterations, or -1 if no world/robot appeared.
EMSCRIPTEN_KEEPALIVE int rwk_load_level(const char *thePath, int theSettleCap)
{
	if (theSettleCap<1) theSettleCap=400;
	// Tear the previous game down FIRST and flush the deferred kill list.
	// GoNewGame() calls Cleanup() itself, but the killed CPUs are not deleted
	// until the next Core_Update -- and one of those destructors takes the NEW
	// world down with it (measured: a 2nd tape in the same page died at tick 1).
	gApp.Cleanup();
	for (int k=0;k<8;k++) gApp.ThrottleUpdate();
	CPU *aGameCPU=gApp.GoNewGame(thePath);
	if (aGameCPU) gApp.AddCPU(aGameCPU);
	// NEVER Throttle() here: FrameController is WALL-CLOCK driven and runs a
	// variable number of updates, which shifts mTouchingGround by the pre-roll
	// (measured: three tapes in one page came out 1 tick further along each).
	// StartLevel() builds the World synchronously, so this normally spins 0x.
	int aSettle=0;
	while (aSettle<theSettleCap && !(gWorld && gWorld->mRobot)) {gApp.ThrottleUpdate();aSettle++;}
	if (!gWorld || !gWorld->mRobot) return -1;
	gP0D_PrevHeld=0;
	return aSettle;
}

EMSCRIPTEN_KEEPALIVE int rwk_observe(float *theOut)
{
	if (!theOut) return 0;
	memset(theOut,0,sizeof(float)*OBS_FLOATS);
	if (!gWorld || !gWorld->mRobot) return 0;
	Robot *r=gWorld->mRobot;
	int p=0;
	if (r->mCanJump)        p|=1<<0;
	if (r->mCanDoubleJump)  p|=1<<1;
	if (r->mCanShoot)       p|=1<<2;
	if (r->mCanAnnihiliate) p|=1<<3;
	if (r->mCanRocket)      p|=1<<4;
	if (r->mCanRocketUp)    p|=1<<5;
	if (r->mHasHelmet)      p|=1<<6;
	if (r->mHasExplozor)    p|=1<<7;
	if (r->mHasHaxxor)      p|=1<<8;
	if (r->mHasVelcro)      p|=1<<9;
	for (int k=0;k<3;k++) if (r->mHasKey[k]) p|=1<<(10+k);

	theOut[ 0]=r->mPos.mX;             theOut[ 1]=r->mPos.mY;
	theOut[ 2]=r->mSpeed;              theOut[ 3]=r->mGravity;
	theOut[ 4]=r->mGMod;               theOut[ 5]=(float)r->mTouchingGround;
	theOut[ 6]=(float)r->mJumpKludge;  theOut[ 7]=(float)(r->mInJump?1:0);
	theOut[ 8]=(float)r->mDoubleJumpCount;
	theOut[ 9]=(float)r->mRocketCountdown;
	theOut[10]=(float)gWorld->mTimer;
	theOut[11]=(float)(gWorld->mDied?1:0);
	theOut[12]=(float)(gWorld->mWin?1:0);
	theOut[13]=(float)r->mHP;          theOut[14]=(float)p;
	theOut[15]=(float)gWorld->mGridWidth;
	theOut[16]=(float)gWorld->mGridHeight;
	theOut[17]=(float)r->mRocketUpCountdown;
	theOut[18]=r->mFacing;
	return OBS_FLOATS;
}

// One logical tick, no wall clock.  theDraw=1 also renders (so a panel can show
// the walk); the simulation is identical either way.
EMSCRIPTEN_KEEPALIVE int rwk_step(int theTicks, int theDraw)
{
	if (!gWorld || !gWorld->mRobot) return -1;
	int t=0;
	for (;t<theTicks;t++)
	{
		gApp.ThrottleUpdate();
		if (theDraw) gApp.ThrottleDraw();
		if (!gWorld || !gWorld->mRobot) break;
	}
	return t;
}

// The solver primitive: step a whole tape in ONE call and write the whole
// observation stream.  theTape holds one held-mask byte per tick.
// theOut must have room for theTicks*OBS_FLOATS floats.
EMSCRIPTEN_KEEPALIVE int rwk_run_tape(const unsigned char *theTape, int theTicks,
                                      float *theOut, int theDraw)
{
	if (!gWorld || !gWorld->mRobot) return -1;
	int t=0;
	for (;t<theTicks;t++)
	{
		rwk_set_input(theTape?theTape[t]:0u);
		gApp.ThrottleUpdate();
		if (theDraw) gApp.ThrottleDraw();
		if (theOut) rwk_observe(theOut+(size_t)t*OBS_FLOATS);
		if (!gWorld || !gWorld->mRobot) {t++;break;}
	}
	return t;
}


// ---------------------------------------------------------------------------
// E3: level injection.  StartPersistentStorage() mounts IDBFS at
// /RAPTISOFT_SANDBOX -- it is DEFINED in OS/WASM/os_core.cpp and CALLED FROM
// NOWHERE in the published mirror, so a reconstructed build must call it.
// ---------------------------------------------------------------------------
EMSCRIPTEN_KEEPALIVE void rwk_start_persistent(void) {OS_Core::StartPersistentStorage();}
EMSCRIPTEN_KEEPALIVE int  rwk_persistent_synced(void) {return OS_Core::IsPersistantStorageSynced()?1:0;}

// The engine's OWN sandbox path, slashes normalised.  Do not hard-code it:
// MyApp.cpp:65 calls Query("LEGACYSANDBOX"), which drops the "_sandbox/"
// component, so the directory the game reads is NOT the one os_core.cpp's
// default branch spells.
EMSCRIPTEN_KEEPALIVE int rwk_sandbox_dir(char *theOut, int theCap)
{
	char aBuf[2048]; aBuf[0]=0;
	OS_Core::GetSandboxFolder(aBuf);
	for (char *c=aBuf;*c;c++) if (*c=='\\') *c='/';
	int n=(int)strlen(aBuf); if (n>theCap-1) n=theCap-1;
	memcpy(theOut,aBuf,n); theOut[n]=0;
	return n;
}

static int p0d_copy(Array<String> &theList, char *theOut, int theCap)
{
	int n=0;
	for (int i=0;i<theList.Size();i++)
	{
		const char *c=theList[i].c();
		int l=(int)strlen(c);
		if (n+l+2>=theCap) break;
		memcpy(theOut+n,c,l); n+=l; theOut[n++]='\n';
	}
	if (theCap>0) theOut[n]=0;
	return theList.Size();
}

// The same enumeration BuildLocalLevels does, exposed on its own.
EMSCRIPTEN_KEEPALIVE int rwk_list_local_levels(char *theOut, int theCap)
{
	Array<String> aList;
	EnumDirectoryFiles("sandbox://EXTRALEVELS64\\",aList);
	return p0d_copy(aList,theOut,theCap);
}

// The in-game "My Levels" path: Makermall.cpp:650 does exactly this.
EMSCRIPTEN_KEEPALIVE int rwk_makermall_local(char *theOut, int theCap, int thePump)
{
	// RComm::Custom defers through RunAfterNextDraw, so the pump must DRAW.
	RComm::RQuery aQ=RComm::Custom(BuildLocalLevels,NULL);
	for (int i=0;i<thePump;i++) {gApp.ThrottleUpdate();gApp.ThrottleDraw();}
	if (!aQ) {if (theCap>0) theOut[0]=0;return -1;}
	IOBuffer &aR=aQ->GetResult();
	int aLen=aR.Length();
	int n=aLen; if (n>theCap-1) n=theCap-1;
	if (n>0) memcpy(theOut,aR.mData,n);
	if (theCap>0) theOut[n]=0;
	return aLen;
}

// Diagnostic: is the key DOWN as far as RAPT is concerned?  Separates
// "the browser never gave SDL the event" from "the game ignored it".
EMSCRIPTEN_KEEPALIVE int rwk_key_down(int theScancode) {return IsKeyDown(theScancode)?1:0;}
EMSCRIPTEN_KEEPALIVE int rwk_sdl_key_raw(int theScancode)
{
	int aN=0; const Uint8 *aS=SDL_GetKeyboardState(&aN);
	if (!aS || theScancode<0 || theScancode>=aN) return -1;
	return aS[theScancode]?1:0;
}
EMSCRIPTEN_KEEPALIVE int rwk_obs_floats(void) {return OBS_FLOATS;}

} // extern "C"

int main(int argc, char **argv)
{
	// IDBFS must be mounted BEFORE anything touches /RAPTISOFT_SANDBOX --
	// StartPersistentStorage() does a bare FS.mkdir and throws "File exists"
	// once the engine has created the directory in MEMFS.  Nothing in the
	// published mirror calls this function at all.
	// SDL2-emscripten attaches its keyboard listeners to the element named by
	// this hint; the port default did not see keys dispatched to our canvas.
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT,"#document");
	OS_Core::StartPersistentStorage();
	OS_Core::Startup();
	OS_Core::SetCommandLine("RWK");
	printf("[p0dprime] OS_Core::Startup done, entering App::Go(false)\n");
	gApp.Go(false);           // full init, no blocking main loop
	printf("[p0dprime] App::Go(false) returned, installing main loop\n");
	emscripten_set_main_loop(RWK_Frame, 0, 1);
	return 0;
}
