#include "MyApp.h"

#include "graphics_core.h"
#include "os_core.h"
#include "Game.h"

#include "transaction_core.h"
#include "MainMenu.h"
#include "Makermall.h"

#if defined(_WIN32) || defined(_WASM) || defined(_LINUX)
#include "Bundle_Controls.h"
class PickControls : public CPU
{
public:
	PickControls();
	virtual ~PickControls();

	void				Go(CPUHOOKPTR theHook);

	void				Draw();
	void				TouchStart(int x, int y);

	Bundle_Controls		mBundle;
};
#endif

TitleBox* gTitleBox=NULL;

int gReserveLeft=0;
int gReserveRight=0;

#define FORMAT		'_'

bool gLetterboxDevice=false;
bool gLetterboxOriginal=true;
Point gBeforeLetterboxPageSize;

static Quad gBKGUV;

void SetupPFilter()
{
	// Load your custom profanity filter here...
}

String PFilter(Font& theFont, String theText, bool doColorize)
{
	// Find and change any profanity in theText...
	return theText;
}

void FullscreenSmallScreen(Rect* theRect, Point theCenter)
{
	float aExpandX=30;
	float aExpandY=27;

	if (gLetterboxDevice) aExpandY-=gG.mLetterboxSize.mY/2;

	theRect->Size(-aExpandX,-aExpandY,gG.mWidth+(aExpandX*2),gG.mHeight+(aExpandY*2));
	theRect->CenterAt(theCenter);
}

MyApp::MyApp()
{
	OS_Core::Query("LEGACYSANDBOX");

#if (defined _WASM) || (defined _WIN32) || (defined _LINUX)
	mTouchControls=false;
#endif

	mLoadingScreen=NULL;
	mGame=NULL;

#ifdef _IOS
	mAdVersion=false;
#endif

	mMsgBox=NULL;
	mGameSettingsDialog=NULL;
	mHiliteLevel=0;
	mIsKittyConnect=false;
	mAskToRateLevels=true;
	mAddon_KittyConnect=true;
	mAddon_MoreLevels=true;

	mJustGotHS=-1;
	mJustGotHSGameType=-1;

	mVersion=VERSION;

	mMaxLevels=GameType_Max;
	mAutoGamecenter=false;
	mAcceptedTerms=false;

	// NEW COMMUNITY
	//SetAppName("RWK");
	SetAppName("RWK_COM");
	SetAppVersion("Alpha 0");

	String aCMD=GetCommandLine();
	if (aCMD.ContainsI("-portable")) SetPortableApp();

	gG.SetDefaultResolution(736,414,false);
	gApp.SetUpdatesPerSecond(50);
	SetOnscreenKeyboardExtraSpace(10);				// Extra space for onscreen keyboard (so you can make sure extra buttons are visible)

	//
	// Typically, you should not pause in the background when loading.
	// But be sure to do so when the game actually starts-- a good time to
	// re-enable this is after the title screen appears.
	//
	gApp.PauseWhenInBackground(false);
}

MyApp::~MyApp()
{
	if (gGame) gGame->Save();
	if (gWorldEditor) gWorldEditor->QuickSave();
	SaveSettings();
}

void MyApp::ResetGlobals()
{
	//
	// If you are networking, or otherwise using the MagicGlobalData class, it will call this
	// whenever data is reset.  If you're not networking, and not using the global data, there's no
	// need to do anything here.
	//
}


void MyApp::Initialize()
{
	#ifdef _ANDROID
	//
	// Because Google thinks nothing of changing storage locations every couple Android versions...
	// This is how we move people's files from the old location to the new, definitely-never-going-to-change-it-again location.
	//
	gOut.Out("LEGACY> Checking for legacy files...");
	String aResult=RestoreLegacyFiles(false);
	gOut.Out("LEGACY> RestoreLegacyFiles = %s",aResult.c());
	#endif

	gBKGUV.mCorner[0]=Point(0,0);
	gBKGUV.mCorner[1]=Point(1,0);
	gBKGUV.mCorner[2]=Point(0,1);
	gBKGUV.mCorner[3]=Point(1,1);


	RComm::SetGameID(MAKERMALL_GAMEID,VERSION);
	RComm::Connect();

	float aAspect=414.0f/gG.HeightF();
	gG.SetPageSize((int)(gG.WidthF()*aAspect),414);

	void* aNH=OS_Core::Query("RESERVESCREEN_TOP");if (aNH) gReserveLeft=(intptr_t)aNH;
	aNH=OS_Core::Query("RESERVESCREEN_BOTTOM");if (aNH) gReserveRight=(intptr_t)aNH;
	OS_Core::Query("ENABLE_MULTITOUCH");

	gAppPtr->OverrideBackButton();
}

void MyApp::Update()
{
	mHiss.Update();
	mThrob.Update();
	UpdatePendingQueryList();
}

RQuery gGoDownload;
RQuery gGoDownloadInfo;
void MyApp::Load()
{
	float aAspect=414.0f/gG.HeightF();
	gG.SetPageSize((int)(gG.WidthF()*aAspect),414);

	//
	// Load any spritebundles, etc, here.
	// Of Note: If you load a soundbundle here, make sure you tweak the _soundbundle.txt file
	// to increment gAppPtr->mLoadCount!
	//

	mLoader->Add(&mBundle_Play);
	mLoader->Add(&mBundle_Tiles);
	mLoader->Add(&mBundle_ETools);
	mSounds.Load("Loading sounds...");

	String aCommandLine=OS_Core::GetCommandLine();

	//
	// Allows invoking with go=levelID if you're hooked up to a level sharing system...
	//
	if (aCommandLine.ContainsI("go="))
	{
		String aLevelIDStr=aCommandLine.GetSegmentAfter("go=").GetSegmentBefore(" ");
		int aLevelID=aLevelIDStr.ToInt();

		gGoDownloadInfo=RComm::LevelSharing_DownloadLevelInfo(aLevelID);
		gGoDownload=RComm::LevelSharing_DownloadLevel(aLevelID);
		int* aInt=new int;*aInt=aLevelID;
		gGoDownload->mExtraData=aInt;
		mLoader->Add(gGoDownload,"Getting level...");
		mLoader->Add(gGoDownloadInfo,"Getting level...");
	}
}


void MyApp::LoadComplete()
{
	{
		SetupPFilter();
		mBundle_Play.mFont_Commodore64Angled18.GetCharacter('.')->mWidth=12;
		mBundle_Play.mFont_Commodore64Angled18Outline.GetCharacter('.')->mWidth=12;
		mBundle_Play.mFont_Commodore64Angled8.GetCharacter('.')->mWidth=4;
		mBundle_Play.mFont_Commodore64Angled8Outline.GetCharacter('.')->mWidth=4;
	
		mHiss.SetSoundLoop(gSounds->mLoop_Hiss);
		mThrob.SetSoundLoop(gSounds->mLoop_Throb);
	}

	gAppPtr->mSettings.RegisterVariable("AvatarVersion",mAvatarVersion,0);
	gAppPtr->mSettings.RegisterVariable("TutorDialog",mShowTutorDialog,true);

	// NEW COMMUNITY
	gAppPtr->mSettings.RegisterVariable("RestartWindow",mEnableRestartConfirm,true);
	// NEW COMMUNITY
	//gAppPtr->mSettings.RegisterVariable("AlwaysGlitchy",mAlwaysShowGlitchyLevels,false);
	gAppPtr->mSettings.RegisterVariable("AlwaysGlitchy",mAlwaysShowGlitchyLevels,true);

	gAppPtr->mSettings.RegisterVariable("DisableCustomMusic",mDisableCustomMusic,false);

	gAppPtr->mSettings.RegisterVariable("TouchControls",mTouchControls,mTouchControls);
	gAppPtr->mSettings.RegisterVariable("Controls.ZoomIn",gApp.mZoomIn,gApp.mZoomIn);
	gAppPtr->mSettings.RegisterVariable("Controls.ZoomOut",gApp.mZoomOut,gApp.mZoomOut);
	gAppPtr->mSettings.RegisterVariable("Controls.Left",gApp.mLeft,gApp.mLeft);
	gAppPtr->mSettings.RegisterVariable("Controls.Right",gApp.mRight,gApp.mRight);
	gAppPtr->mSettings.RegisterVariable("Controls.Up",gApp.mUp,gApp.mUp);
	gAppPtr->mSettings.RegisterVariable("Controls.Down",gApp.mDown,gApp.mDown);
	gAppPtr->mSettings.RegisterVariable("Controls.Jump",gApp.mJump,gApp.mJump);
	gAppPtr->mSettings.RegisterVariable("Controls.Shoot",gApp.mShoot,gApp.mShoot);
	gAppPtr->mSettings.RegisterVariable("Controls.Rocket",gApp.mRocket,gApp.mRocket);
	gAppPtr->mSettings.RegisterVariable("Controls.RocketUp",gApp.mRocketUp,gApp.mRocketUp);
	gAppPtr->mSettings.RegisterVariable("Controls.Menu",gApp.mMenu,gApp.mMenu);
	gAppPtr->mSettings.RegisterVariable("Controls.Map",gApp.mMap,gApp.mMap);
	gAppPtr->mSettings.RegisterVariable("Controls.Pick",gApp.mPickControls,true);

	gBeforeLetterboxPageSize=Point(gG.WidthF(),gG.HeightF());

	//
	// Letterboxing!
	//
	float aAspect=320.0f/gG.HeightF();
#define MINWIDTH 569.0f
#define WANTWIDTH 569.0f
	if (!gApp.IsSmallScreen() && gG.Width()*aAspect<MINWIDTH) gLetterboxDevice=true;
	if (!gApp.IsSmallScreen() && gG.Width()*aAspect<MINWIDTH) // Tablet!  Need adjustment!
	{
		float aAspect=WANTWIDTH/gG.mResolutionWidth;
		gG.SetPageSize((int)WANTWIDTH,(int)(gG.mResolutionHeight*aAspect));
		int aLBSize=gG.mResolutionHeight-gG.Height();
		gG.Letterbox(gG.Height()-320);
	}
	else gG.SetPageSize((int)(gG.WidthF()*aAspect),320);

	ResetGlobals();
	LoadGameData();

	mMusic_Title=&mSounds.mStream_Music_Title;
	mMusic_Gameplay[0]=&mSounds.mStream_Music_Gameplay1;
	mMusic_Gameplay[1]=&mSounds.mStream_Music_Gameplay1;
	mMusic_Gameplay[2]=&mSounds.mStream_Music_Gameplay2;
	mMusic_Gameplay[3]=&mSounds.mStream_Music_Gameplay3;
	mMusic_Erase=&mSounds.mStream_Music_Eraser;

	gAudio.SetMusicSample(mMusic_Title);
	for (int aCount=0;aCount<4;aCount++) gAudio.SetMusicSample(mMusic_Gameplay[aCount]);
	gAudio.SetMusicSample(&mCustomMusic[0]);
	gAudio.SetMusicSample(&mCustomMusic[1]);
	gAudio.SetMusicSample(&mCustomMusic[2]);
	gAudio.SetMusicSample(mMusic_Erase);


#if defined(_WIN32) || defined(_WASM)|| (defined _LINUX)
	if (mPickControls)
	{
		PickControls *aPC=new PickControls;
		aPC->Go(CPUHOOK({StartRunning();}));
	}
	else StartRunning();
#else
	StartRunning();
#endif
}

void MyApp::StartRunning()
{
	GoMainMenu();
}

void MyApp::StartInstaplay(int theLevelID)
{
	TitleBox* aTB=new TitleBox;
	aTB->HookTo(&gTitleBox);
	
	aTB->mName=gBundle_Play->mFont_Commodore64Angled18.Wrap(gMakermall->mInstaplayName,mWidth-150);
	aTB->mAuthor=gBundle_Play->mFont_Commodore64Angled11.Wrap(gMakermall->mInstaplayAuthor,mWidth-150);
	aTB->mLevelID=theLevelID;
	
	aTB->Go(CPUHOOK(
		{
			TitleBox* aTB=(TitleBox*)theThis;
			int aLevelID=aTB->mLevelID;
			gMakermall->StartPlaying(aLevelID);
		}
	));
}
	

void MyApp::FadeAllMusicExcept(SoundStream* theException, float theTime)
{
	if (theException!=mMusic_Title) 
	{
		mMusic_Title->FadeOut(theTime);
	}
	if (theException!=mMusic_Erase) 
	{
		mMusic_Erase->FadeOut(theTime);
	}
	for (int aCount=0;aCount<4;aCount++) if (mMusic_Gameplay[aCount]!=theException) mMusic_Gameplay[aCount]->FadeOut(theTime);
}

void MyApp::FadeInMusic(SoundStream* theMusic, float theTime)
{
	FadeAllMusicExcept(theMusic,theTime);
	if (theMusic) 
	{
		theMusic->FadeIn(theTime);
	}
}

void MyApp::LoadBackground()
{
}

void MyApp::LoadBackgroundComplete()
{
}

void MyApp::Multitasking(bool isForeground)
{
	//
	// Called when the App goes into the foreground or background...
	//

	//
	// This will put a pause overlay over everything, and pause
	// the app.  You technically should only do this during gameplay,
	// because you don't really want a pause overlay over menus
	// or anything like that... so this line might be better in
	// Game::Multitasking()
	//
	if (!isForeground) 
	{
		if (gGame) gGame->Save();
		if (gWorldEditor) gWorldEditor->QuickSave();
		SaveSettings();
		if (ShouldPauseWhenInBackground()) new MyPauseOverlay;
	}
	else 
	{
		RComm::FreshenNews();
	}

}


class LoadingScreenBundle;LoadingScreenBundle* gLoadingScreenBundle=NULL;
class LoadingScreenBundle : public Object {public:Bundle_Loader mBundle;int mLastDraw=0;LoadingScreenBundle() {mBundle.Load();mLastDraw=gApp.mAppDrawNumber;}~LoadingScreenBundle() {gLoadingScreenBundle=NULL;}void Update() {if (gApp.mAppDrawNumber>mLastDraw+3) {Kill();}}};
Bundle_Loader& GetBundleLoader() {if (!gLoadingScreenBundle) {gLoadingScreenBundle=new LoadingScreenBundle;gAppPtr->mBackgroundProcesses+=gLoadingScreenBundle;}gLoadingScreenBundle->mLastDraw=gApp.mAppDrawNumber;return gLoadingScreenBundle->mBundle;}

void MyApp::DrawLoadingScreen(int theFlag)
{
	Bundle_Loader& aLoader=GetBundleLoader();

	gG.Clear(0,0,.32f);

	Point aCenter=Point(mWidth/2,mHeight/2);

	aLoader.mLogo.Center(aCenter+Point(-125,-55));
	aLoader.mHLogo.Center(aCenter+Point(125,-55));

	float aLoadComplete=gApp.GetLoadComplete();

	aLoader.mLoadbar.DrawRotated(aCenter+Point(0,125),90);//240,290,90);
	Point aUL=aCenter+Point(0,125)-Point(aLoader.mFull.mHeight/2,aLoader.mFull.mWidth/2);
	gG.Clip(aUL.mX,aUL.mY,(192*aLoadComplete),320.0f);
	aLoader.mFull.DrawRotated(aCenter+Point(0.0f,126.0f),90);
	gG.Clip();

	aLoader.mRURL.Center(aCenter+Point(-125,65));
	aLoader.mHURL.Center(aCenter+Point(125,65));

}


/////////////////////////////////////////////////////////////
//
//
// Implementation of custom pause overlay...
// 
//
/////////////////////////////////////////////////////////////
void MyPauseOverlay::Draw()
{
	//
	// Quickie implement of a checkboard overlay just to
	// be the pause overlay.
	//
	float aStepX=mWidth/15;
	float aStepY=mHeight/12;

	gG.SetColor(0,0,0,.5f);
	bool aHoldColor=false;
	bool aColor=aHoldColor;
	for (float aSpanX=0;aSpanX<mWidth;aSpanX+=aStepX)
	{
		aColor=aHoldColor;
		aHoldColor=!aHoldColor;
		for (float aSpanY=0;aSpanY<mHeight;aSpanY+=aStepY)
		{
			if (aColor) gG.SetColor(0,0,0,.48f);
			else gG.SetColor(0,0,0,.5f);
			gG.FillRect(aSpanX,aSpanY,aStepX,aStepY);
			aColor=!aColor;
		}
	}
	gG.SetColor();
}

void CPUPlus::Core_TouchStart(int x, int y)
{
	CPU::Core_TouchStart(x,y);

	mMaybeDrag=true;
	mDrag=false;
}

void CPUPlus::Core_TouchMove(int x, int y)
{
	CPU::Core_TouchMove(x,y);
	if (mMaybeDrag && gMath.DistanceSquared(gAppPtr->mMessageData_TouchPosition,gAppPtr->mMessageData_TouchStartPosition)>5)
	{
		mMaybeDrag=false;
		mDrag=true;
		StartDrag(x,y);
	}

	if (mDrag) Drag(x,y);
}

void CPUPlus::Core_TouchEnd(int x, int y)
{
	CPU::Core_TouchEnd(x,y);
	mMaybeDrag=false;
	mDrag=false;
}

void MyApp::GoKittyConnect()
{
	Cleanup();

	{
		KittyConnect *aKK=new KittyConnect();
		AddCPU(aKK);
	}
}

void MyApp::LoadGameData()
{
	Settings& aRegistry=mSettings;

	mAutoFix=aRegistry.GetBool("Autofix",true);
	mUseJoypad=aRegistry.GetBool("Usejoypad",true);

	mControlPos[0]=aRegistry.GetPoint("ControlPos_Joypad",Rect(2,252,191,66).Translate(0,-10).Center());
	mControlPos[1]=aRegistry.GetPoint("ControlPos_Jump",Rect(gG.WidthF()-70,252,66,66).Translate(0,-10).Center());
	mControlPos[2]=aRegistry.GetPoint("ControlPos_Shoot",Rect(gG.WidthF()-70-70,252,66,66).Translate(0,-10).Center());
	mControlPos[3]=aRegistry.GetPoint("ControlPos_Rocket",Rect(gG.WidthF()-70-35,252-55,66,66).Translate(0,-10).Center());
	mControlPos[4]=aRegistry.GetPoint("ControlPos_RocketUp",Rect(gG.WidthF()-70,252-55-55,66,66).Translate(0,-10).Center());
	mHiliteLevel=aRegistry.GetInt("Hilite_Level",0);

	mRememberName=aRegistry.GetString("Remember_Name","ROBOT");

	for (int aCount=0;aCount<GameType_Max;aCount++)
	{
		mWinLevel[aCount]=aRegistry.GetBool(Sprintf("WIN_%d",aCount),false);
		mWinNoDying[aCount]=aRegistry.GetBool(Sprintf("WIN_NODYING_%d",aCount),false);
		mWinKillEverything[aCount]=aRegistry.GetBool(Sprintf("WIN_KILLEVERYTHING_%d",aCount),false);
		mWinGotEverything[aCount]=aRegistry.GetBool(Sprintf("WIN_GOTEVERYTHING_%d",aCount),false);
	}

	for (int aCount=0;aCount<GameType_Max;aCount++)
	{
		mHS[aCount].mReverseScoring=true;
		mHS[aCount].SetScoreCount(100);
		mHS[aCount].Load(Sprintf("sandbox://HS64_%d.HS",aCount));
	}
}

void MyApp::SaveGameData()
{
	Settings& aRegistry=mSettings;
	
	aRegistry.SetInt("Hilite_Level",mHiliteLevel);
	aRegistry.SetBool("Autofix",mAutoFix);
	aRegistry.SetBool("Usejoypad",mUseJoypad);
	aRegistry.SetString("ControlPos_Joypad",Sprintf("%d,%d",(int)mControlPos[0].mX,(int)mControlPos[0].mY));
	aRegistry.SetString("ControlPos_Jump",Sprintf("%d,%d",(int)mControlPos[1].mX,(int)mControlPos[1].mY));
	aRegistry.SetString("ControlPos_Shoot",Sprintf("%d,%d",(int)mControlPos[2].mX,(int)mControlPos[2].mY));
	aRegistry.SetString("ControlPos_Rocket",Sprintf("%d,%d",(int)mControlPos[3].mX,(int)mControlPos[3].mY));
	aRegistry.SetString("ControlPos_RocketUp",Sprintf("%d,%d",(int)mControlPos[4].mX,(int)mControlPos[4].mY));
	aRegistry.SetString("Remember_Name",mRememberName);

	for (int aCount=0;aCount<GameType_Max;aCount++)
	{
		mHS[aCount].Save();
	}


	for (int aCount=0;aCount<GameType_Max;aCount++)
	{
		aRegistry.SetBool(Sprintf("WIN_%d",aCount),mWinLevel[aCount]);
		aRegistry.SetBool(Sprintf("WIN_NODYING_%d",aCount),mWinNoDying[aCount]);
		aRegistry.SetBool(Sprintf("WIN_KILLEVERYTHING_%d",aCount),mWinKillEverything[aCount]);
		aRegistry.SetBool(Sprintf("WIN_GOTEVERYTHING_%d",aCount),mWinGotEverything[aCount]);
	}

	SaveSettings();
}

String MyApp::StringDisplayFit(String theString, Font *theFont, int theWidth)
{
	String aName=theString;
	if(theFont->Width(aName)>theWidth)
	{
		bool aTooLong=true;
		int aLength=aName.mLength;
		aName+="...";
		while(aTooLong&&aLength>3)
		{
			aName.Truncate(aName.Len()-4);
			aName+="...";
			if(theFont->Width(aName)<=theWidth)
			{
				aTooLong=false;
			}
		}
	}
	return aName;
}

void MyApp::Cleanup()
{
	//
	// Cleans up objects (gets called specifically on
	// destructor, but can also be used when you switch game
	// modes)
	// 
	//if (mTitleScreen) mTitleScreen->Kill();mTitleScreen=NULL;
	if (gGame) gGame->Kill();gGame=NULL;
	if (gWorldEditor) 
	{
		gWorldEditor->QuickSave();
		if (gWorldEditor!=gWorld) gWorldEditor->Kill();gWorldEditor=NULL;
	}
	if (gWorld) gWorld->Kill();gWorld=NULL;
	if (mMsgBox) mMsgBox->Kill();mMsgBox=NULL;
	if (gMainMenu) gMainMenu->Kill();gMainMenu=NULL;
	if (gControlGadget) gControlGadget->Kill();gControlGadget=NULL;
	if (gKittyConnect) gKittyConnect->Kill();gKittyConnect=NULL;
	if (gInGameMenu) gInGameMenu->Kill();gInGameMenu=NULL;
	if (gWin) gWin->Kill();gWin=NULL;
	if (gMakermall) gMakermall->FocusSpins(false);
}

CPU* MyApp::GoNewGame(String theLevel)
{
	Cleanup();

	Game *aGame=new Game();
	aGame->mSaveName=gTempSaveName;
	String aSaveGame="sandbox://";aSaveGame+=gTempSaveName;
	_DLOG("Look for savegame: [%s]",aSaveGame.c());
	if (gTempSaveName.Len() && DoesFileExist(aSaveGame))
	{
		aGame->Load();
	}
	else 
	{
		aGame->StartLevel(theLevel);
	}
	return aGame;
}

void MyApp::GoMainMenu()
{
	Cleanup();

	mIsKittyConnect=false;
	MainMenu *aMM=new MainMenu();
	AddCPU(aMM);
}

void MyApp::GoEditor(String aLevelName)
{
	Cleanup();
	Game *aGame=new Game();
	aGame->StartEditor(aLevelName);
	AddCPU(aGame);
}

bool MyApp::GetAchievement(String theAchievement)
{
	gSocial.GetAchievement(LocalizeOS(theAchievement));
	return true;
}

void MyApp::WinLevel(int theLevel)
{
	mWinLevel[theLevel]=true;

	int aWinAll=0;
	for (int aCount=0;aCount<GameType_Max;aCount++) if (mWinLevel[aCount]) aWinAll++;
	if (aWinAll>=6) GetAchievement("CatLady");
	if (aWinAll>=10) GetAchievement("UltraCatLady");
}

void MyApp::WinLevelNoDying(int theLevel)
{
	WinLevel(theLevel);

	GetAchievement("SmartAlgorithm");

	mWinNoDying[theLevel]=true;
	int aWinAll=0;
	for (int aCount=0;aCount<GameType_Max;aCount++) if (mWinNoDying[aCount]) aWinAll++;
	if (aWinAll>=6) GetAchievement("ArtificialIntelligence");
	if (aWinAll>=10) GetAchievement("CloudBackup");

}

void MyApp::WinLevelGotEverything(int theLevel)
{
	WinLevel(theLevel);

	mWinGotEverything[theLevel]=true;
	int aWinAll=0;
	for (int aCount=0;aCount<GameType_Max;aCount++) if (mWinGotEverything[aCount]) aWinAll++;
	if (aWinAll>=10) GetAchievement("Gatherbot");

}

void MyApp::WinLevelKillEverything(int theLevel)
{
	WinLevel(theLevel);

	GetAchievement("Killbot");

	mWinKillEverything[theLevel]=true;
	int aWinAll=0;
	for (int aCount=0;aCount<GameType_Max;aCount++) if (mWinKillEverything[aCount]) aWinAll++;
	if (aWinAll>=10) GetAchievement("DeathzoneKillbot");
}

void MyApp::SubmitHighScore(String theBoard, String theName, int theScore)
{
	gSocial.SubmitScore(theBoard,theName,theScore);
}

void FunDialog::Initialize()
{
	FocusBackButton();
	Dialog::Initialize();
}

void FunDialog::SetupButtons()
{
	Dialog::SetupButtons();
	mDone.CenterAt(mWidth-75+10,52+3);
}

void FunDialog::DrawOverlay()
{
	gBundle_FunDialog->mDialogRim_Hollow.DrawMagicBorderEX_AllCorners(Rect(0,0,mWidth,mHeight),true);
	gBundle_FunDialog->mMenuTop_Hollow.DrawMagicBorderEX_LeftRight_TopOnly(0,0,mWidth,105);
	if (mBack.mFeatures&CPU_DRAW) mBack.Core_Draw();
	if (mDone.mFeatures&CPU_DRAW) mDone.Core_Draw();
}



GameSettingsDialog* MyApp::GoSettings()
{
	if (mGameSettingsDialog) {mGameSettingsDialog->Kill();return NULL;}

	GameSettingsDialog* aDialog=new GameSettingsDialog;
	aDialog->HookTo(&mGameSettingsDialog);
	aDialog->mSoundVolume=gAudio.GetSoundVolume();
	aDialog->mMusicVolume=gAudio.GetMusicVolume();
	//	aDialog->Size(-25,0,mWidth-118,mHeight);
	//aDialog->Size(-25,0,mWidth,mHeight);
	aDialog->Size();
	FullscreenSmallScreen(aDialog,gG.Center());

	aDialog->mX+=gReserveLeft;
	aDialog->mWidth-=gReserveRight+gReserveLeft;

	//aDialog->Size(-25,-20,mWidth-118,mHeight-40);
	aDialog->SetBundle(&mBundle_FunDialog);
	aDialog->SetTitle("SETUP");				// Sets the dialog title

	aDialog->mHack=aDialog->MakeNewPanel("H@XX0R STUFF",false);
	aDialog->mControls=aDialog->MakeNewPanel("CONTROLS",false);

	//
	// Build the dialog...
	//
	DialogPanel* aPanel=aDialog->MakeNewPanel("SETUP");

	//aPanel->Add(new DialogGroupHeader("Visuals"));
	aPanel->Add(new DialogSlider("SOUND VOLUME",&aDialog->mSoundVolume));
	aPanel->Add(new DialogSlider("MUSIC VOLUME",&aDialog->mMusicVolume));
	aPanel->Add(new DialogGroupHeader(""));
	//aPanel->Add(new DialogNextPanel("TWEAK GAME","TWEAK",aDialog->mTweaks));
	if (gSocial.IsConnected()) 
	{
		if (gAppPtr->GetOSName()=="IOS") aPanel->Add(new DialogButton("OPEN GAMECENTER"));
	}

	aPanel->Add(new DialogNextPanel("CONTROLS","TWEAK",aDialog->mControls));
	aPanel->Add(new DialogCheckbox("ASK ABOUT TUTORIAL",&gApp.mShowTutorDialog));
	aPanel->Add(new DialogCheckbox("DISABLE CUSTOM MUSIC",&gApp.mDisableCustomMusic));
	aPanel->Add(new DialogCheckbox("ALLOW GLITCHY LEVELS",&gApp.mAlwaysShowGlitchyLevels));

	// NEW COMMUNITY
	aPanel->Add(new DialogCheckbox("ENABLE RESTART CONFIRM",&gApp.mEnableRestartConfirm));

	aPanel->Add(new DialogNextPanel("H@XX0R STUFF","HACK",aDialog->mHack));

	//aDialog->mControls->Add(new DialogCheckbox("USE JOYPAD",&mUseJoypad));

	if (!gApp.mTouchControls) 
	{
		aDialog->mControls->Add(new DialogKeyGrabber("LEFT",&gApp.mLeft));
		aDialog->mControls->Add(new DialogKeyGrabber("RIGHT",&gApp.mRight));
		aDialog->mControls->Add(new DialogKeyGrabber("UP",&gApp.mUp));
		aDialog->mControls->Add(new DialogKeyGrabber("DOWN",&gApp.mDown));
		aDialog->mControls->Add(new DialogGroupSpacer(10));
		aDialog->mControls->Add(new DialogKeyGrabber("JUMP",&gApp.mJump));
		aDialog->mControls->Add(new DialogKeyGrabber("SHOOT",&gApp.mShoot));
		aDialog->mControls->Add(new DialogKeyGrabber("ROCKET",&gApp.mRocket));
		aDialog->mControls->Add(new DialogKeyGrabber("ROCKET UP",&gApp.mRocketUp));
		aDialog->mControls->Add(new DialogGroupSpacer(10));
		aDialog->mControls->Add(new DialogKeyGrabber("ZOOM IN",&gApp.mZoomIn));
		aDialog->mControls->Add(new DialogKeyGrabber("ZOOM OUT",&gApp.mZoomOut));
		aDialog->mControls->Add(new DialogKeyGrabber("SHOW MAP",&gApp.mMap));
		aDialog->mControls->Add(new DialogGroupSpacer(10));
		aDialog->mControls->Add(new DialogKeyGrabber("MENU",&gApp.mMenu));
	}

	if (gApp.mTouchControls) aDialog->mControls->Add(new DialogButton("CUSTOMIZE TOUCH CONTROLS"));
	aDialog->mControls->Add(new DialogGroupHeader(""));
	aDialog->mControls->Add(new DialogButton("RESET TO DEFAULTS"));

	aDialog->mHack->Add(new DialogTextbox("SECRET CODE:",&mSecretCode));
	aDialog->mHack->Add(new DialogButton("TOUCH HERE TO ACTIVATE CODE"));
	//
	//
	//
	return aDialog;
}


void DrawProgress(CPU* theCPU)
{
	StepProcessor* aCPU=(StepProcessor*)theCPU;
	gG.SetColor(0,0,0,.85f);
	gG.FillRect();
	gG.SetColor();

	Rect aRect=Rect(400,170);
	aRect.CenterAt(gG.Center());
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(aRect.Translate(0,5),true);
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(aRect,true);
	CENTERGLOW(aCPU->mCaption,aRect.CenterX(),aRect.mY+50);

	{
		Rect aR2=aRect.ExpandH(-50);
		aR2.mHeight=30;
		aR2.CenterAt(gG.Center());
		aR2.mY+=15;
		gG.PushClip();
		gG.Clip(aR2);
		for (int aCount=0;aCount<aR2.mWidth+25;aCount+=25)
		{
			gG.FillRectGradientH(aR2.mX+aCount-(gApp.AppTime()%25),aR2.mY,25,aR2.mHeight,Color(1),Color(.75f,1,1,0));
			gG.FillRectGradientH(aR2.mX+aCount-(gApp.AppTime()%25),aR2.mY,25,aR2.mHeight,Color(1),Color(.75f,1,1,0));
		}
		gG.PopClip();
		gG.SetColor();
		gG.FillRect(aR2.mX,aR2.mY,aR2.mWidth*aCPU->mProgress,aR2.mHeight);
		gG.SetColor();
	}

}


void GameSettingsDialog::ChangedControl(DialogWidget* theWidget)
{
	if (theWidget==mSignOut)
	{
		OS_Core::Query("playservices:signout");
		MsgBox* aBox=new MsgBox;
		aBox->GoNoThrottle("YOU HAVE BEEN SIGNED OUT OF GOOGLE PLAY GAMES!",gGCenter(),0);
		Done();
	}
	if (theWidget==mSignIn)
	{
		gSocial.Go();
		Pending* aP=new Pending("GOOGLESIGNIN","SIGNING IN...",false);
		aP->ExtraData()=this;
		gApp+=aP;
	}
	if (theWidget->mName=="SOUND VOLUME") 
	{
		gAudio.SetSoundVolume(mSoundVolume);
		static int aPause=-1;
		if (gApp.AppTime()>aPause)
		{
			gSounds->mCrystal.Play();
			//gSounds->mBuysell.Play();
			aPause=gApp.AppTime()+5;
		}
	}
	if (theWidget->mName=="MUSIC VOLUME") 
	{
		gAudio.SetMusicVolume(mMusicVolume);
	}

	if (theWidget->mName=="TOUCH HERE TO ACTIVATE CODE")
	{
		if (gApp.mSecretCode=="WHATEVS")
		{
			// Do whatever
		}
	}

	if (theWidget->mName=="CUSTOMIZE TOUCH CONTROLS")
	{
		ControlGadget *aCG=new ControlGadget();
		gApp.AddCPU(aCG);
	}
	if (theWidget->mName=="DISABLE CUSTOM MUSIC")
	{
		if (gWorld) gWorld->StartMusic();
	}

	if (theWidget->mName=="RESET TO DEFAULTS")
	{
		gSounds->mClick.Play();
		gSounds->mClick.PlayPitched(1.5f);
		gApp.mUseJoypad=true;

		float aWidth=gG.WidthF()-(gReserveLeft+gReserveRight);

		gApp.mControlPos[0]=Rect(2,252,191,66).Translate(0,-10).Center();
		gApp.mControlPos[1]=Rect(aWidth-70,252,66,66).Translate(0,-10).Center();
		gApp.mControlPos[2]=Rect(aWidth-70-70,252,66,66).Translate(0,-10).Center();
		gApp.mControlPos[3]=Rect(aWidth-70-35,252-55,66,66).Translate(0,-10).Center();
		gApp.mControlPos[4]=Rect(aWidth-70,252-55-55,66,66).Translate(0,-10).Center();

		gApp.mZoomIn=KB_PAGEUP;
		gApp.mZoomOut=KB_PAGEDOWN;
		gApp.mLeft=KB_LEFTARROW;
		gApp.mRight=KB_RIGHTARROW;
		gApp.mJump=KB_UPARROW;
		gApp.mShoot=KB_X;
		gApp.mRocket=KB_C;
		gApp.mRocketUp=KB_V;
		gApp.mMenu=KB_ESCAPE;


		gApp.SaveGameData();
	}
}
	
bool GameSettingsDialog::Done()
{
	mNoKill=true;
	if (!mReturnTo)
	{
		MainMenu *aMM=new MainMenu();
		Transition* aT=new Transition(this,aMM);
	}
	else
	{
		Transition* aT=new Transition(this,mReturnTo);
	}

	return true;
}

List gMsgBoxList;
bool IsMsgBoxActive()
{
	return (gMsgBoxList.GetCount()>0);
}

MsgBox::MsgBox()
{
	mIsThrottled=false;
	gMsgBoxList+=this;
	FocusBackButton();
}
MsgBox::~MsgBox()
{
	gMsgBoxList-=this;
}

void MsgBox::GoX(String theText, Point theCenter, char theButtons,MBHOOKPTR theCallback)
{
	mCallback=theCallback;
	mIsThrottled=true;
	gSounds->mMenu.Play();
	float aMaxWidth=400;
	mText=gBundle_Play->mFont_Commodore64Angled11.Wrap(theText,aMaxWidth);
	mHoggyOffset=1.0f;

	Rect aDims=gBundle_Play->mFont_Commodore64Angled11.GetBoundingRect(mText,Point(0,0),0);

	float aBorder=70;
	mBoxRect.Size(0,0,aDims.mWidth+aBorder*2,aDims.mHeight+aBorder*2);
	mBoxRect.mHeight+=80;	// Room for buttons
	mBoxRect.CenterAt(theCenter);

	mButtonType=theButtons;
	if (theButtons==0)
	{
		mYes.Size(118,718);
		mYes.CenterAt(mBoxRect.CenterX(),mBoxRect.LowerRight().mY-90);
		mYes.SetNotify(this);
		mYes.SetSounds(&gSounds->mClick);
		mNo.Size(0,0,-1,-1);
		*this+=&mYes;
	}
	else
	{
		mYes.Size(118,718);
		mYes.CenterAt(mBoxRect.CenterX()-65,mBoxRect.LowerRight().mY-90);
		mYes.SetNotify(this);
		mYes.SetSounds(&gSounds->mClick);
		*this+=&mYes;

		mNo.Size(118,718);
		mNo.CenterAt(mBoxRect.CenterX()+65,mBoxRect.LowerRight().mY-90);
		mNo.SetNotify(this);
		mNo.SetSounds(&gSounds->mClick);
		*this+=&mNo;
	}

	gAppPtr->AddCPU(this);
	char aResult=false;
}


void MsgBox::GoNoThrottle(String theText, Point theCenter, char theButtons)
{
	mIsThrottled=false;
	gSounds->mMenu.Play();
	float aMaxWidth=400;
	mText=gBundle_Play->mFont_Commodore64Angled11.Wrap(theText,aMaxWidth);
	mHoggyOffset=1.0f;

	Rect aDims=gBundle_Play->mFont_Commodore64Angled11.GetBoundingRect(mText,Point(0,0),0);

	float aBorder=70;
	mBoxRect.Size(0,0,aDims.mWidth+aBorder*2,aDims.mHeight+aBorder*2);
	mBoxRect.mHeight+=80;	// Room for buttons
	mBoxRect.CenterAt(theCenter);

	mButtonType=theButtons;
	if (theButtons==0)
	{
		mYes.Size(118,718);
		mYes.CenterAt(mBoxRect.CenterX(),mBoxRect.LowerRight().mY-90);

		mYes.SetNotify(this);
		mYes.SetSounds(&gSounds->mClick);
		mNo.Size(0,0,-1,-1);
		*this+=&mYes;
	}
	else
	{
		mYes.Size(118,718);
		mYes.CenterAt(mBoxRect.CenterX()-65,mBoxRect.LowerRight().mY-90);
		mYes.SetNotify(this);
		mYes.SetSounds(&gSounds->mClick);
		*this+=&mYes;

		mNo.Size(118,718);
		mNo.CenterAt(mBoxRect.CenterX()+65,mBoxRect.LowerRight().mY-90);
		mNo.SetNotify(this);
		mNo.SetSounds(&gSounds->mClick);
		*this+=&mNo;
	}
	gAppPtr->AddCPU(this);
}



void MsgBox::Update()
{
}

void MsgBox::Draw()
{
	gG.SetColor(0,0,0,.5f);
	gG.FillRect();
	gG.SetColor();
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(mBoxRect.Translate(0,5),true);
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(mBoxRect,true);
	CENTERGLOW(mText,mBoxRect.CenterX(),mBoxRect.mY+80);
}

void MsgBox::TouchStart(int x, int y)
{
	if (!mBoxRect.ContainsPoint(x,y)) 
	{
		if (mIsThrottled) Click("NO");
		Kill();
	}
}

void MsgBox::Click(String theValue)
{
	if (mCallback) mCallback(theValue);
	Kill();
}

void MsgBox::DrawOverlay()
{
	gG.PushTranslate();
	gG.Translate(2,-9);
	if (mButtonType==0)
	{
		gBundle_Play->mGlowy_Ok.Center(mYes.Center()+Point(0,mYes.IsDown()*10));
		//if (gShowCaret) DrawCaret(&mYes);
	}
	else if (mButtonType==1)
	{
		gBundle_Play->mGlowy_Yes.Center(mYes.Center()+Point(0,mYes.IsDown()*10));
		gBundle_Play->mGlowy_No.Center(mNo.Center()+Point(0,mNo.IsDown()*10));
	}
	gG.PopTranslate();
}

void MsgBox::Notify(void* theData)
{
	{
		if (theData==&mYes) Click("YES");
		if (theData==&mNo) Click("NO");
		if (theData==&mInvisibleCloseButton) Click("NO");
	}
	Kill();
}

GCPopup *gShowingPopup=NULL;

GCPopup::GCPopup(String theLine1, String theLine2)
{
	mLine1=theLine1;
	mLine2=theLine2;

	mWidth=gBundle_Play->mFont_Commodore64Angled8.Width(mLine1);
	mWidth=_max(mWidth,gBundle_Play->mFont_Commodore64Angled11.Width(mLine2));
	mWidth+=20;

	mYPos=-45;
	mDelay=100;
	mMode=false;

	//Ghost();
	DisableTouch();
	gAppPtr->AddCPU(this);
}

GCPopup::~GCPopup()
{
}

void GCPopup::Update()
{
	if (gShowingPopup==NULL) 
	{
		gSounds->mMeow.PlayPitched(2.0f);
		gSounds->mMeow.PlayPitched(1.0f);
		gShowingPopup=this;
	}
	if (gShowingPopup!=this) return;

	if (!mMode)
	{
		mYPos+=2;
		if (mYPos>20)
		{
			mYPos=20;
			mMode=true;
		}
	}
	else
	{
		if (--mDelay<=0)
		{
			mYPos--;
			if (mYPos<-45) 
			{
				Kill();
				if (gShowingPopup==this) gShowingPopup=NULL;
			}
		}
	}
}

void GCPopup::Draw()
{
	Rect aRect=Rect((float)((gG.WidthF()/2)-(mWidth/2)),(float)mYPos-3,(float)mWidth,35).Expand(20);
	gG.SetColor(.75f+(gMath.Sin(gAppPtr->AppTimeF()*20.0f)*.25f));
	gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(aRect);

	float aXP=3;
	aRect=aRect.Expand(aXP);

	CENTERUNDERGLOWSMALL(mLine1,480/2,mYPos-15);
	CENTERUNDERGLOW(mLine2,480/2,mYPos+12-15);

	CENTEROVERGLOWSMALL(mLine1,480/2,mYPos-15);
	CENTEROVERGLOW(mLine2,480/2,mYPos+12-15);
}

bool IsNews() {return RComm::IsNews();}
String GetNews() {return RComm::GetNews();}

Browser::Browser()
{
	mBackTo=0;
	mStep=0;
	gBundle_FunDialog->Load();
	FocusBackButton();
}

Browser::~Browser()
{
	gBundle_FunDialog->Unload();
}

void Browser::Initialize()
{
	mX+=gReserveLeft;
	mWidth-=gReserveRight+gReserveLeft;


	float aXBorder=15;
	float aYBorder=30;
	mMLBox.mParent=this;
	mMLBox.Size(aXBorder,aYBorder,mWidth-(aXBorder*2),mHeight-(aYBorder*2));

	mMLBox.mY+=29;
	mMLBox.mHeight-=5;
	mMLBox.mBKGColor=Color(0);
	*this+=&mMLBox;

	mMLBox.mSetup.mPadLeft=20;
	mMLBox.mSetup.mPadRight=20;
	mMLBox.mSetup.mPadTop=20;
	mMLBox.mSetup.mPadBottom=20;
	mMLBox.AddFont(&gBundle_Play->mFont_Commodore64Angled11,"NORMAL");
	mMLBox.AddFont(&gBundle_Play->mFont_Commodore64Angled11,"DEFAULT");
	mMLBox.AddFont(&gBundle_Play->mFont_Commodore64Angled8,"SMALL");
	mMLBox.AddFont(&gBundle_Play->mFont_Commodore64Angled18,"LARGE");
	mMLBox.AddFont(&gBundle_Play->mFont_Tiny,"TINY");
	mMLBox.AddIMG(&gBundle_Play->mLong_OK,"OK_BUTTON");
	mMLBox.AddIMG(&gBundle_Play->mLong_Cancel,"CANCEL_BUTTON");
	mMLBox.Define("#width",mMLBox.mWidth);
	mMLBox.Define("#username",gAppPtr->mUserName);
	mMLBox.Define("#password",gAppPtr->mUserPassword);
	mMLBox.Define("#avatar_width",110);
	mMLBox.Define("#avatar_height",42);
	mMLBox.AddIMG(&gBundle_Play->mWhiteheart,"karma");
	mMLBox.AddIMG(&gBundle_Play->mTrash,"trash");

	if (mData.Len())
	{
		mMLBox.Format(mData);
		mMLBox.mReadyToDisplay=true;
		mData="";
	}
	else
	{
		mMLBox.Format("<SETUP hborders=20><BKGCOLOR #004770><BR>");
		mMLBox.mReadyToDisplay=false;
	}

	if (mAllowClose)
	{
		mClose.Size(57,57);
		mClose.mX=mWidth-57;
		mClose.mY=-3;
		mClose.SetSounds(&gSounds->mClick);
		mClose.SetNotify(this);
		*this+=&mClose;
	}
	else mClose.Size(-1000,-1000,57,57);
}

void Browser::TouchStart(int x, int y)
{
	if (mAllowTapClose) Kill();
}

void Browser::Update()
{
	mStep-=.25f;
	if (mStep<=0) mStep+=256;
}

void DrawBKGTile(float x, float y, int theType)
{
	gBundle_Tiles->mBKG[theType].Draw(x,y);
}

void Browser::Draw()
{
	gG.PushClip();
	gG.Clip(mMLBox.Expand(10));
	gG.Translate(-mStep,0.0f);

	gG.ClipColor(.5f,.5f,.5f);

	int aW=(gG.Width()/256)+2;
	int aH=(gG.Height()/256)+1;

	for (int aSpanX=0;aSpanX<aW;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<aH;aSpanY++)
		{
			DrawBKGTile(aSpanX*256.0f,aSpanY*256.0f);
//			gBundle_Tiles->mMoonbase.Draw(aSpanX*252.0f,aSpanY*252.0f);
		}
	}
	gG.Translate(mStep,0.0f);
	gG.SetColor(.99f);
	Quad aP;
	Color aC[4];
	float aRed=0.0f;
	float aGreen=0.0f;
	float aBlue=.5f;
	aP.mCorner[0]=Point(0,0);
	aP.mCorner[1]=Point(gG.Width(),0);
	aP.mCorner[2]=Point(0,320);
	aP.mCorner[3]=Point(gG.Width(),320);
	aC[0]=Color(aRed,aGreen,aBlue,0);
	aC[1]=Color(aRed,aGreen,aBlue,0);
	aC[2]=Color(aRed,aGreen,aBlue,.5f);
	aC[3]=Color(aRed,aGreen,aBlue,.5f);
	gG.FillQuad(aP,aC[0],aC[1],aC[2],aC[3]);
	gG.ClipColor();
	gG.SetColor();

	gG.PopClip();
}

void Browser::DrawOverlay()
{
	Rect aRect=Rect(0,0,mWidth,mHeight).ExpandH(30).ExpandV(30);

	gBundle_FunDialog->mDialogRim_Hollow.DrawMagicBorderEX_AllCorners(aRect.mX,aRect.mY+10,aRect.mWidth,aRect.mHeight-10,true);
	gBundle_FunDialog->mMenuTop_Hollow.DrawMagicBorderEX_LeftRight_TopOnly(aRect.mX,aRect.mY,aRect.mWidth,105);
	gBundle_FunDialog->mFont_Header.Center(mTitle,aRect.CenterX(),aRect.mY+72);

	gBundle_FunDialog->mDoneButton[mClose.IsDown()].Center(mClose.Center());
}

void Browser::Notify(void* theData)
{
	if (theData==&mClose)
	{
		switch (mBackTo)
		{
		case 0:
			{
				Transition* aT=new Transition(this, new MainMenu);
				break;
			}
		default:
			{
				Transition* aT=new Transition(this, new MainMenu);
				break;
			}
		}
	}
}

Pending::Pending(String theFunction, String theText, bool canCancel)
{
	mFunction=theFunction;
	mText=theText;
	mCanCancel=canCancel;
	mDidLog=false;
	FocusBackButton();
}

void Pending::Initialize()
{
}

void Pending::Update()
{
}

void Pending::TouchStart(int x, int y)
{
	if (mCanCancel)
	{
		DisableTouch();
		DisableDraw();
	}
}

void Pending::Draw()
{
	gG.SetColor(0,0,0,.85f);
	gG.FillRect();
	gG.SetColor();

	Rect aRect=Rect(400,133);
	aRect.CenterAt(gG.Center());
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(aRect.Translate(0,5),true);
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(aRect,true);
	CENTERGLOW(mText,CenterX(),aRect.mY+50);
	if (mCanCancel) CENTERGLOWSMALL("TOUCH TO CANCEL",CenterX(),aRect.mY+aRect.mHeight-30);

	{
		gG.SetColor(.5f);
		Rect aR2=aRect.ExpandH(-50);
		aR2.mHeight=30;
		aR2.CenterAt(Center());
		aR2.mY+=5;
		gG.PushClip();
		gG.Clip(aR2);
		for (int aCount=0;aCount<aR2.mWidth+25;aCount+=25)
		{
			gG.FillRectGradientH(aR2.mX+aCount-(gApp.AppTime()%25),aR2.mY,25,aR2.mHeight,Color(1),Color(.75f,1,1,0));
			gG.FillRectGradientH(aR2.mX+aCount-(gApp.AppTime()%25),aR2.mY,25,aR2.mHeight,Color(1),Color(.75f,1,1,0));
		}
		gG.PopClip();
	}

}

struct AvatarContainer
{
	SmartList(AvatarSprite) mList;
};

MyMLBox::MyMLBox()
{
	mParent=NULL;
	mAvatarContainer=new AvatarContainer;
}

void MyMLBox::DrawCustom(String theName, MLImage* theImage)
{
	gG.PushClip();
	gG.Clip();
	float aEPlus=0;
	if ((gAppPtr->AppTime()/25)%2) aEPlus=2;
	if (theName.StartsWith("_box"))
	{

		gG.SetColor(.25f);
//		Rect aR=Rect(theImage->mX,theImage->mY,theImage->mWidth-theImage->mX,theImage->mHeight-theImage->mY);
		Rect aR=Rect(theImage->mX,theImage->mY,theImage->mWidth,theImage->mHeight);
		//Rect aR=Rect(0,0,theImage->mWidth,theImage->mHeight);
		gBundle_Play->mTinyCorner.DrawMagicBorder(aR.Expand(3).ExpandH(6),true);
		gG.SetColor();

	}
	else if (theName.StartsWith("_fix"))
	{

		gG.SetColor(.25f);
		//		Rect aR=Rect(theImage->mX,theImage->mY,theImage->mWidth-theImage->mX,theImage->mHeight-theImage->mY);
		Rect aR=Rect(0,20,theImage->mWidth,theImage->mHeight);
		aR.CenterXAt(((mWidth-(mSetup.mPadLeft+mSetup.mPadRight))/2));//mSetup.mPadLeft+(CenterX()-(mSetup.mPadLeft)));
		gBundle_Play->mTinyCorner.DrawMagicBorder(aR.Expand(3).ExpandH(6),true);
		gG.SetColor();

	}
	else if (theName.StartsWith("x:"))
	{
		//Rect aR=Rect(theImage->mX,theImage->mY,theImage->mWidth-theImage->mX,theImage->mHeight-theImage->mY);
		Rect aR=Rect(-aEPlus,-aEPlus,theImage->mWidth+aEPlus*2,theImage->mHeight+aEPlus*2);
		gG.SetColor(0,0,0,.5f);
		gBundle_Play->mTinyCorner.DrawMagicBorder(aR.Translate(5,5),true);
		gG.SetColor();
		//gG.FillRect(aR);
		gBundle_Play->mTinyCorner.DrawMagicBorder(aR,true);
		gG.SetColor(mBKGColor,.75f);
		gBundle_Play->mFont_Commodore64Angled11.Center(theName.GetSegmentAfter(':'),aR.CenterX(),aR.CenterY()+6);
		gG.SetColor();

	}
	else if (theName.StartsWith("_cb"))
	{
		gG.SetColor(.25f);
		//		Rect aR=Rect(theImage->mX,theImage->mY,theImage->mWidth-theImage->mX,theImage->mHeight-theImage->mY);
		Rect aR=Rect(0,0,theImage->mWidth,theImage->mHeight);
		gBundle_Play->mTinyCorner.DrawMagicBorder(aR.Expand(10),true);
		gG.SetColor();
	}
	else if (theName.StartsWith("_xp"))
	{
		String aWork=theName.GetSegmentAfter(':');
		int aTotalXP=aWork.GetToken(':').ToInt();
		int aMaxXP=aWork.GetNextToken(':').ToInt();
		float aXP=_min(1.0f,(float)aTotalXP/(float)aMaxXP);

		Rect aRect=Rect(0,0,theImage->mWidth,theImage->mHeight);
		gG.FillRect(aRect);
		gG.SetColor(mBKGColor);
		Rect aR2=aRect.Expand(-4);
		gG.FillRect(aR2);
		gG.SetColor(.5f);
		aR2.mWidth*=aXP;
		gG.FillRect(aR2);
		gG.SetColor();

		if (aTotalXP>=aMaxXP)
		{
			if ((gApp.AppTime()/10)%2) gBundle_Play->mFont_Commodore64Angled11.Center("YOU CAN LEVEL UP!",aRect.CenterX(),aRect.mY+aRect.mHeight-6);
		}
		else gBundle_Play->mFont_Commodore64Angled11.Center(Sprintf("%d/%d",aTotalXP,aMaxXP),aRect.CenterX(),aRect.mY+aRect.mHeight-6);

	}
	else if (theName.StartsWith("ava") || theName.StartsWith("xava"))
	{
		//
		// Need to get the avatar and display it...
		//
		//Rect aRect=Rect(0,0,theImage->mWidth,theImage->mHeight);
		//gG.FillRect(aRect);

		if (!theImage->mExtraData)
		{
			Smart(AvatarSprite) aAS=GetAvatar(theImage->ID().GetSegmentAfter(':'),false);
			aAS->mExtraData=theImage;
			AvatarContainer* aAC=(AvatarContainer*)mAvatarContainer;
			aAC->mList+=aAS;
			theImage->mExtraData=aAS.GetPointer();

			AvatarSprite* aAS2=(AvatarSprite*)theImage->mExtraData;
		}

		gG.PushColor();
		gG.SetColor();
		AvatarSprite* aAS=(AvatarSprite*)theImage->mExtraData;
		gG.SetColor(0,0,0,1);
		if (theName[0]!='x') 
		{
			gG.FillRect(0,0,theImage->mWidth+10,theImage->mHeight+25);
			gG.SetColor();
			gBundle_Play->mFont_Tiny.Center("TOUCH TO CHANGE",(theImage->mWidth+10)/2,theImage->mHeight+20);
		}
		else gG.FillRect(0,0,theImage->mWidth+10,theImage->mHeight+10);
		gG.SetColor();
		aAS->mSprite->Draw(5,5);
		aAS->mCountdown=25;
		gG.PopColor();

	}
	else if (theName.StartsWith("no"))
	{
		gG.PushColor();
		gG.SetColor();
		AvatarSprite* aAS=(AvatarSprite*)theImage->mExtraData;
		gG.SetColor(0,0,0,1);
		gG.FillRect(0,0,theImage->mWidth+10,theImage->mHeight+10);
		gG.SetColor();
		gBundle_Play->mFont_Commodore64Angled18.Center("?",(theImage->mWidth/2)+10,theImage->mHeight-7);
		gG.PopColor();
	}

	else gOut.Out("Unhandled Custom: %s",theName.c());
	gG.PopClip();
}


MyMLBox::~MyMLBox()
{
	if (GetNotifyMLBox()==this) SetNotifyMLBox(NULL);
	delete mAvatarContainer;
}

void MyMLBox::GoPending(String theString)
{
	Reset();
	mPendingString=theString;
	mPendingTime=gAppPtr->AppTime();
	mReadyToDisplay=false;
}

void MyMLBox::DrawPending()
{
	CENTERGLOW(mPendingString,CenterX(),(mHeight/2)-38);

	Rect aRect=Rect(300,60);
	aRect.CenterAt(CenterX(),(mHeight/2)+3);

	gG.SetColor(0,0,0,.5f);
	gBundle_Play->mTinyCorner.DrawMagicBorder(aRect.Expand(5),true);
	gG.SetColor(.1f,0,.1f,1);
	gBundle_Play->mTinyCorner.DrawMagicBorder(aRect,true);
	gG.SetColor();

	static float aOffset=0;
	static float aLastOffset=0;
	static float aBob=0;
	float aX=CenterX();
	aLastOffset=aOffset;
	aOffset=(gMath.Sin((gAppPtr->AppTime()*3)-mPendingTime)*100);
	aX+=aOffset;
	float aFlip=1;
	float aDiff=aOffset-aLastOffset;
	if (aDiff>0) aFlip=-1;
	aBob+=(aDiff*-aFlip)*5;
	gBundle_Play->mRobot_Runwheel.DrawFlipped(aX,(mHeight/2),aFlip,1);
	gBundle_Play->mRobot_Run.DrawFlipped(aX,(mHeight/2)+gMath.Sin(aBob)*1,aFlip,1);
	gG.PushClip();
	gG.Clip(aRect.Expand(-2).Translate(1,0));

	gBundle_Play->mKitty.DrawFlipped((CenterX()+180)+(aOffset/3),(mHeight/2)-5,-1,1);
	gBundle_Play->mKitty.DrawFlipped((CenterX()-180)+(aOffset/3),(mHeight/2)-5,1,1);
	gG.PopClip();
}

void MyMLBox::Kill()
{
	if (mParent) mParent->Kill();
	MLBox::Kill();
}

void MyMLBox::Changed()
{
	EnumSmartList(MLObject,aM,mObjectList)
	{
		if (aM->mType==ML_TEXTBOX)
		{
			Smart(MLTextBox) aT=aM;
			aT->mTextBox.SetBackgroundColor(1);
			aT->mTextBox.SetTextColor(mBKGColor);
			aT->mTextBox.SetTextOffset(3,-2);
			aT->mTextBox.SetCursorOffset(Point(0,2));
			aT->mTextBox.SetCursorWidth(4);
		}
	}
}


bool MyMLBox::ClickLink(String theCommand)
{
	//
	// Return true if we handled it
	//
	if (theCommand.StartsWith("local:\\\\"))
	{
		gSounds->mClick.Play();
		ClearContents();
		theCommand.Replace("local:\\\\","local://");
		Load(theCommand);
		mReadyToDisplay=true;
		return true;
	}
	else if (theCommand=="signin")
	{
		Smart(MLTextBox) aU=GetObjectByID("username");
		Smart(MLTextBox) aP=GetObjectByID("password");

		bool aOK=false;
		if (aU && aP)
		{
			if (aU->mTextBox.GetText().Len() && aP->mTextBox.GetText().Len())
			{
				aOK=true;
			}
		}
		if (!aOK)
		{
			MsgBox* aBox=new MsgBox;
			aBox->GoNoThrottle("PLEASE ENTER YOUR USERNAME AND PASSWORD TO SIGN IN!",gGCenter(),0);
		}
		else
		{
			gAppPtr->mUserName=aU->mTextBox.GetText();
			gAppPtr->mUserPassword=aP->mTextBox.GetText();
			if (gMakermall) 
			{
				gSounds->mClick.Play();
				GoPending("SIGNING IN...");
				SetNotifyMLBox(this);
				gMakermall->CheckLogin();
			}
		}
		return true;
	}
	else if (theCommand=="recover") 
	{
		Load("data://recover.ml");
		mReadyToDisplay=true;
		return true;
	}
	else if (theCommand=="dorecover")
	{
		Smart(MLTextBox) aU=GetObjectByID("username");
		bool aOK=false;
		if (aU && aU->mTextBox.GetText().Len()) aOK=true;
		if (!aOK)
		{
			MsgBox* aBox=new MsgBox;
			aBox->GoNoThrottle("PLEASE ENTER YOUR USERNAME OR E-MAIL!",gGCenter(),0);
		}
		else
		{
			SetNotifyMLBox(this);
			GoPending("RECOVERING...");
			RComm::RQuery aRQ=RComm::LevelSharing_Recover(aU->mTextBox.GetText());
			gApp.mPendingQueryList+=aRQ;
		}
	}
	else if (theCommand=="createprofile")
	{
		Smart(MLTextBox) aU=GetObjectByID("username");
		Smart(MLTextBox) aP=GetObjectByID("password");
		Smart(MLTextBox) aE=GetObjectByID("email");

		bool aOK=false;
		if (aU && aP && aE)
		{
			if (aU->mTextBox.GetText().Len() && aP->mTextBox.GetText().Len() && aE->mTextBox.GetText().Len()) aOK=true;
		}
		if (!aOK)
		{
			MsgBox* aBox=new MsgBox;
			aBox->GoNoThrottle("PLEASE ENTER A USERNAME, PASSWORD, AND E-MAIL ADDRESS TO CREATE AN ACCOUNT!",gGCenter(),0);
		}
		else
		{
			Define("#youremail",aE->mTextBox.GetText().ToUpper());
			SetNotifyMLBox(this);
			GoPending("CREATING ACCOUNT...");
			RComm::RQuery aRQ=RComm::LevelSharing_CreateAccount(aU->mTextBox.GetText(),aP->mTextBox.GetText(),aE->mTextBox.GetText());
			gApp.mPendingQueryList+=aRQ;
			//
			// Fire off the command!
			//
		}
		return true;
	}
	else if (theCommand=="mainlogin")
	{
		Load("data://signin.ml");
		mReadyToDisplay=true;
		return true;
	}
	else if (theCommand=="sendactivation")
	{
		SetNotifyMLBox(this);
		GoPending("SENDING ACTIVATION...");
		RComm::RQuery aRQ=RComm::LevelSharing_SendActivation(gApp.mUserName);
		gApp.mPendingQueryList+=aRQ;
		return true;
	}
	else if (theCommand=="addcomment")
	{
		if (gMakermall)
		{
			if (!IsLoginValid())
			{
				//
				// Somehow we need to "return" to the comment box, yes?
				//
				Define("#done","addcomment");
				Load("data://signin.ml");
				mReadyToDisplay=true;
				return true;
			}
			else
			{
				int aPlayingID=GetDefine("#playingID").ToInt();
				AddCommentWidget(aPlayingID);
			}
		}
	}

	else if (theCommand.StartsWith("trash:"))
	{
		gSounds->mClick.Play();
		int aTrashID=theCommand.GetSegmentAfter(':').ToInt();
		if (gMakermall) gMakermall->RemoveMail(aTrashID);
		return true;
	}
	else if (theCommand=="checkmessages")
	{
		gSounds->mClick.Play();
		SetNotifyMLBox(this);
		gMailBrowser=(MMBrowser*)mParent;
		ShowMail();
		return true;
	}
	else if (theCommand=="manageaccount")
	{
		if (gMakermall)
		{
			SetNotifyMLBox(this);
			GoPending("LOADING...");
			gMakermall->mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"manageaccount","",this);
		}
	}
	else if (theCommand=="main")
	{
		if (gMakermall)
		{
			SetNotifyMLBox(this);
			GoPending("LOADING...");
			gMakermall->mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"main","",this);
		}
	}
	else if (theCommand=="signout")
	{
		gSounds->mClick.Play();
		MsgBox* aBox=new MsgBox;
		aBox->GoX("ARE YOU SURE YOU WANT TO SIGN OUT?",gGCenter(),1,MBHOOK(
			{
				if (theResult=="YES") 
				{
					gAppPtr->mUserName="";
					gAppPtr->mUserPassword="";
					gValidLogin=false;
					gApp.SaveSettings();
					Kill();

					gMakermall->FixBottomOfScreen();
				}
			}));
		return true;
	}
	else if (theCommand=="changepassword")
	{
		gSounds->mClick.Play();
		if (gMakermall)
		{
			Smart(MLTextBox) aOPBox=GetObjectByID("oldpassword");
			Smart(MLTextBox) aNPBox=GetObjectByID("newpassword");
			if (aOPBox && aNPBox)
			{
				String aOP=aOPBox->mTextBox.GetText();
				String aNP=aNPBox->mTextBox.GetText();
				if (aNP.Len() && aOP.Len()) 
				{
					GoPending("Loading...");
					gMakermall->mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"manageaccount",Sprintf("&oldpassword=%s&newpassword=%s",aOP.c(),aNP.c()),this);
				}
				else
				{
					MsgBox* aMB=new MsgBox;
					aMB->GoNoThrottle("PLEASE ENTER BOTH YOUR OLD AND NEW PASSWORD!",gGCenter(),0);
				}
			}
		}
		return true;
	}
	else if (theCommand=="changeemail")
	{
		gSounds->mClick.Play();
		Smart(MLTextBox) aOPBox=GetObjectByID("oldpassword2");
		Smart(MLTextBox) aNEBox=GetObjectByID("newemail");
		if (aOPBox && aNEBox)
		{
			String aOP=aOPBox->mTextBox.GetText();
			String aNEmail=aNEBox->mTextBox.GetText();
			if (aNEmail.Len() && aOP.Len()) 
			{
				GoPending("Loading...");
				gMakermall->mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"manageaccount",Sprintf("&oldpassword=%s&newemail=%s",aOP.c(),aNEmail.c()),this);
			}
			else
			{
				MsgBox* aMB=new MsgBox;
				aMB->GoNoThrottle("PLEASE ENTER BOTH YOUR PASSWORD AND AN E-MAIL ADDRESS!",gGCenter(),0);
			}
		}
		return true;
	}
	else if (theCommand=="asklevelup")
	{
		gSounds->mClick.Play();
		if (gMakermall)
		{
			GoPending("Loading...");
			gMakermall->mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"asklevelup","",this);
		}
		return true;
	}
	else if (theCommand=="levelup")
	{
		gSounds->mClick.Play();
		if (gMakermall)
		{
			GoPending("Loading...");
			gMakermall->mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"levelup","",this);
		}
		return true;
	}
	else if (theCommand=="changeavatar")
	{
		gSounds->mClick.Play();
		if (gMakermall)
		{
			GoPending("Loading...");
			gMakermall->mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"getavatarlist","",this);
		}
		return true;
	}
	else if (theCommand.StartsWith("hilite"))
	{
		gSounds->mClick.Play();
		MsgBox* aMB=new MsgBox;
		String aStr=theCommand.GetSegmentAfter(':').GetQuotes();
		aMB->GoNoThrottle(aStr,gGCenter(),0);
		return true;
	}
	else if (theCommand.StartsWith("newavatar:"))
	{
		gSounds->mClick.Play();
		if (gMakermall)
		{
			GoPending("Loading...");
			gMakermall->mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"newavatar",Sprintf("&avatar=%s",theCommand.GetSegmentAfter(':').c()),this);
		}
		return true;
	}
	else if (theCommand=="close") 
	{
		gSounds->mClick.Play();
		mParent->Kill();
		return true;
	}
	else if (theCommand.StartsWith("SetRadioMessage:"))
	{
		IPoint aPos=theCommand.GetSegmentAfter(':').ToPoint();

		Smart(MLTextBox) aTBox=GetObjectByID("RADIOMESSAGE");
		if (aTBox && gWorldEditor) gWorldEditor->SetRadioMessage(aPos,aTBox->mTextBox.GetText());

		gSounds->mClick.Play();
		mParent->Kill();
		return true;
	}
	else gOut.Out("Unhandled: %s",theCommand.c());
	return MLBox::ClickLink(theCommand);
}

void MyApp::UpdatePendingQueryList()
{
	RComm::RCThreadLock();
	EnumSmartList(RComm::RaptisoftQuery,aRQ,mPendingQueryList)
	{
		if (aRQ->IsDone())
		{
			if (aRQ->mFunctionPointer==&RComm::LevelSharing_Recover)
			{
				String aML="<SETUP hborders=25><BKGCOLOR #004770><font small><br><movecursor +0,+10><center>";

				bool aSuccess=false;
				if (aRQ->IsRML()) aML+="<font normal>AN E-MAIL HAS BEEN SENT TO YOU WITH INSTRUCTIONS ON HOW TO RESET YOUR PROFILE PASSWORD!";
				else aML+="<font normal>THIS PROFILE COULD NOT BE CREATED BECAUSE THE SERVER HAS... GONE COMPLETELY BONKERS?  YOU CAN'T POSSIBLY BE SEEING THIS MESSAGE.  THIS IS ARGMAGEDDON.";

				MyMLBox* aBox=GetNotifyMLBox();
				if (aBox)
				{
					gSounds->mMenu.Play();
					aML+="<BR><BR><link cmd=\"local:\\\\data\\signin.ml\"><custom 200,60 x:\"OK!\"></link><BR>";
					aBox->LoadFromString(aML);
				}
			}
			if (aRQ->mFunctionPointer==&RComm::LevelSharing_SendActivation)
			{
				String aML="<SETUP hborders=25><BKGCOLOR #004770><font small><br><movecursor +0,+10><center>";

				bool aSuccess=false;
				if (aRQ->IsRML())
				{
					String aServerSays=aRQ->mResult.ToString();
					String aResult=aServerSays.GetRML("RESULT");
					if (aResult=="FAIL")
					{
						String aFailReason=aServerSays.GetRML("FAILREASON");
						String aNewReason;
						if (aFailReason=="NOTFOUND") aNewReason="THIS PROFILE WAS NOT FOUND";
						if (aFailReason=="ALREADYACTIVATED") aNewReason="THIS PROFILE HAS ALREADY BEEN ACTIVATED!";

						aML+="<font normal>";
						aML+=aNewReason;
					}
					else
					{
						aML+="<font normal>A NEW ACTIVATION E-MAIL HAS BEEN SENT TO YOUR E-MAIL ADDRESS!";
					}
				}
				else
				{
					aML+="<font normal>THIS PROFILE COULD NOT BE CREATED BECAUSE THE SERVER HAS... GONE COMPLETELY BONKERS?  YOU CAN'T POSSIBLY BE SEEING THIS MESSAGE.  THIS IS ARGMAGEDDON.";
				}

				MyMLBox* aBox=GetNotifyMLBox();
				if (aBox)
				{
					gSounds->mMenu.Play();
					aML+="<BR><BR><link cmd=\"local:\\\\data\\signin.ml\"><custom 200,60 x:\"OK!\"></link><BR>";
					aBox->LoadFromString(aML);
				}


			}
			if (aRQ->mFunctionPointer==&RComm::LevelSharing_CreateAccount)
			{
				String aML="<SETUP hborders=25><BKGCOLOR #004770><font small><br><movecursor +0,+10><center>";

				bool aSuccess=false;
				if (aRQ->IsRML())
				{
					String aServerSays=aRQ->mResult.ToString();
					String aResult=aServerSays.GetRML("RESULT");
					if (aResult=="FAIL")
					{
						String aFailReason=aServerSays.GetRML("FAILREASON");
						String aNewReason;

						if (aFailReason=="NOUSERNAME") aNewReason="NO USERNAME WAS SPECIFIED!";
						else if (aFailReason=="NOPASSWORD") aNewReason="NO PASSWORD WAS SPECIFIED!";
						else if (aFailReason=="NOEMAIL") aNewReason="NO E-MAIL WAS SPECIFIED!";
						else if (aFailReason=="USERNAMEEXISTS") aNewReason="THAT USERNAME ALREADY EXISTS!  PLEASE CHOOSE ANOTHER!";
						else if (aFailReason=="EMAILEXISTS") aNewReason="A PROFILE ASSOCIATED WITH THIS E-MAIL ACCOUNT ALREADY EXISTS!";
						else if (aFailReason=="BADCHARS") aNewReason="YOUR USERNAME CONTAINS FORBIDDEN CHARACTERS (LETTERS AND NUMBERS ONLY, PLEASE)!";
						else aNewReason="THE SERVER IS REPORTING A PROBLEM WITH ACCOUNT CREATION!  PLEASE TRY AGAIN IN A MOMENT.";


						aML+="<font normal>THIS PROFILE COULD NOT BE CREATED BECAUSE ";
						aML+=aNewReason;
					}
					else
					{
						aML+="<font normal>YOUR PROFILE HAS BEEN CREATED... BUT IT NEEDS TO BE ACTIVATED!<BR><BR>";
						aML+="<color .75>An e-mail has been sent to <color 1>#youremail<color .75> with activation instructions!";

						aSuccess=true;
					}
				}
				else
				{
					aML+="<font normal>THIS PROFILE COULD NOT BE CREATED BECAUSE THE SERVER HAS... GONE COMPLETELY BONKERS?  YOU CAN'T POSSIBLY BE SEEING THIS MESSAGE.  THIS IS ARGMAGEDDON.";
				}

				MyMLBox* aBox=GetNotifyMLBox();
				if (aBox)
				{
					gSounds->mMenu.Play();

					if (aSuccess)
					{
						aML+="<BR><BR><link cmd=\"local:\\\\data\\signin.ml\"><custom 200,60 x:\"OK!\"></link><BR>";
					}
					else
					{
						aML+="<BR><BR><link cmd=\"local:\\\\data\\createprofile.ml\"><custom 200,60 x:\"TRY AGAIN\"></link><null><SP 15>";
						aML+="<link cmd=\"local:\\\\data\\signin.ml\"><custom 200,60 x:\"NEVER MIND\"></link><BR>";
					}
					aBox->LoadFromString(aML);
				}

			}
			mPendingQueryList-=aRQ;
			EnumSmartListRewind(RComm::RaptisoftQuery);
		}
	}
	RComm::RCThreadUnlock();
}

void MyMLBox::Exe(Array<String>& theParams)
{
	if (theParams[0]=="fixpassword") gApp.mUserPassword=theParams[1].GetSegment(1,theParams[1].Len()-2);
	if (theParams[0]=="hello") if (gMakermall) {SetNotifyMLBox(NULL);gMakermall->CheckLogin();}
}

void MsgBrowser::Initialize()
{
	Browser::Initialize();
	CenterAt(gG.Center());
	mMLBox.mWidth-=50;
	mMLBox.mHeight-=25;
	mMLBox.CenterAt(mWidth/2,mHeight/2);
	mClose.CenterAt(-9999,-9999);
	mMLBox.Format(mData);mData="";
	mMLBox.mBKGColor=Color(0,0,0,0);
}

void MsgBrowser::DrawOverlay()
{
}

void MsgBrowser::Draw()
{
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(mMLBox,true);
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(mMLBox,true);
}

#if defined(_WIN32) || defined(_LINUX)
void MyApp::DroppedFile(String theFilename)
{
	IOBuffer aBuffer;
	aBuffer.Load(theFilename);

	MakeDirectory("temp://");

	String aFN=GetFilenameFromPath(theFilename).RemoveTail('.');
	aFN+=".kitty";

	String aTempFile=Sprintf("temp://convert.kitty");
	aBuffer.CommitFile(aTempFile);

	WorldEditor aWE;
	aWE.Load(aTempFile);

	String aLocal=Sprintf("sandbox://EXTRALEVELS64\\%s",aFN.c());
	aWE.Save(aLocal);
}
#endif

void DrawWaitForAd()
{
}

void MyApp::DrawOverlay()
{
	if (gReserveLeft+gReserveRight==0) return;
	if (!gApp.mLoadComplete) return;

	gG.Translate();

#define BORDERTWEAK 5
	gG.SetColorGrey(0);
	gG.FillRect(0,0,(float)gReserveLeft,gG.HeightF());
	gG.FillRect(gG.WidthF()-gReserveRight,0,(float)gReserveRight,gG.HeightF());
	gG.SetColor();
}

#if defined(_WIN32) || defined(_WASM) || defined(_LINUX)
PickControls::PickControls()
{
	mBundle.Load();
	gSounds->mAlert.Play();
}

PickControls::~PickControls()
{
	mBundle.Unload();
}

void PickControls::TouchStart(int x, int y)
{
	gApp.mZoomIn=KB_PAGEUP;
	gApp.mZoomOut=KB_PAGEDOWN;
	gApp.mJump=KB_SPACE;
	gApp.mShoot=KB_X;
	gApp.mRocket=KB_C;
	gApp.mRocketUp=KB_V;
	gApp.mMenu=KB_ESCAPE;
	gApp.mMap=KB_TAB;
	gApp.mPickControls=false;

	if (x>mWidth/2)
	{
		// WASD!!!!
		gApp.mUp=KB_W;
		gApp.mDown=KB_S;
		gApp.mLeft=KB_A;
		gApp.mRight=KB_D;
		gApp.mShoot=KB_J;
		gApp.mRocket=KB_L;
		gApp.mRocketUp=KB_I;
	}
	else
	{
		// ARROWS!!!!
		gApp.mUp=KB_UPARROW;
		gApp.mDown=KB_DOWNARROW;
		gApp.mLeft=KB_LEFTARROW;
		gApp.mRight=KB_RIGHTARROW;
	}
	gSounds->mClick.Play();
	gApp.mSettings.Save();
	ThrottleRelease(0);
	Kill();
}

void PickControls::Draw()
{
	gG.Clear(0,0,.32f);
	mBundle.mArrows.Center(150,mHeight/2);
	mBundle.mWASD.Center(mWidth-150,mHeight/2);

	CENTERGLOWBIG("CHOOSE YOUR CONTROLS!",mWidth/2,30);
	CENTERGLOW("(You can customize this in settings)",mWidth/2,mHeight-15);
}

void PickControls::Go(CPUHOOKPTR theHook)
{
	gAppPtr->AddCPU(this);
	Throttle(theHook);
}
#endif

TitleBox::TitleBox()
{
	mCountdown=250;
}

void TitleBox::Initialize()
{
	mName=gBundle_Play->mFont_Commodore64Angled18.Wrap(mName,mWidth-150);
}

void TitleBox::Update()
{
	if (
		IsKeyPressed(gApp.mUp) ||
		IsKeyPressed(gApp.mDown) ||
		IsKeyPressed(gApp.mLeft) ||
		IsKeyPressed(gApp.mRight) ||
		IsKeyPressed(gApp.mJump) ||
		IsKeyPressed(gApp.mShoot) ||
		IsKeyPressed(gApp.mRocket) ||
		IsKeyPressed(gApp.mRocketUp) ||
		IsKeyPressed(gApp.mMenu) ||
		IsKeyPressed(gApp.mMap) ||
		IsKeyPressed(KB_SPACE) ||
		IsKeyPressed(KB_ENTER) ||
		IsKeyPressed(KB_ESCAPE) ||

		--mCountdown<=0 ||
		mName.Len()==0
		) ThrottleRelease(0);
}

void TitleBox::Draw()
{
	gG.Clear(0,0,.32f);

	Rect aBoxRect=Rect(mWidth-100,mHeight-75);
	aBoxRect.CenterAt(gG.Center());
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(aBoxRect.Translate(0,5),true);
	gBundle_Play->mDialogRim.DrawMagicBorderEX_AllCorners(aBoxRect,true);
	CENTERGLOWBIG(mName,aBoxRect.CenterX(),aBoxRect.mY+80);
	CENTERGLOW("by",aBoxRect.CenterX(),aBoxRect.CenterY()+20);
	CENTERGLOW(mAuthor,aBoxRect.CenterX(),aBoxRect.CenterY()+50);
}

void TitleBox::TouchStart(int x, int y)
{
	ThrottleRelease(0);
}

void TitleBox::Go(CPUHOOKPTR theHook)
{
	gAppPtr->AddCPU(this);
	Throttle(theHook);
}


