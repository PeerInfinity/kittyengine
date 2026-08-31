#pragma once

#include "rapt.h"
#include "Bundle_Loader.h"
#include "Bundle_Play.h"
#include "Bundle_Tiles.h"
#include "Bundle_ETools.h"
#include "Bundle_Sounds.h"
#include "Bundle_FunDialog.h"
#include "HighScoreList.h"
#include "Dialog.h"
#include "MLRender.h"
#include "Beepbox.h"

#ifndef WIN32_LEAN_AND_MEAN
#define CONTROLUI
#endif

#ifdef _WASM
#undef CONTROLUI
#endif

#ifdef _LINUX
#undef CONTROLUI
#endif

extern int gReserveLeft;
extern int gReserveRight;


extern float gNotched;


class MyLoadingScreen;
class Game;
class TitleScreen;
class GameSettingsDialog;
class MyApp : public App
{
public:
	MyApp();
	~MyApp();

	void					Initialize();

	void					Update();
	void					Load();
	void					LoadComplete();
	void					DrawOverlay();
	void					StartRunning();
	void					StartInstaplay(int theLevelID);

	void					LoadBackground();
	void					LoadBackgroundComplete();

	void					Multitasking(bool isForeground);

	void					ResetGlobals();
	void					GoKittyConnect();

	void					LoadGameData();
	void					SaveGameData();
	void					Cleanup();

	String					StringDisplayFit(String theString, Font *theFont, int theWidth);
	CPU*					GoNewGame(String theLevel);
	void					GoMainMenu();
	void					GoEditor(String theLevelName);
	GameSettingsDialog*		GoSettings();

	void					ResolveCloudIssues();

	

public:
	MyLoadingScreen*		mLoadingScreen;
	Game*					mGame;
	GameSettingsDialog*		mGameSettingsDialog;
	String					mSecretCode;
	String					mVersion;

	RComm::RQueryList		mPendingQueryList;
	void					UpdatePendingQueryList();

	BeepBox					mCustomMusic[3];


	Bundle_Play				mBundle_Play;
	Bundle_Tiles			mBundle_Tiles;
	Bundle_ETools			mBundle_ETools;
	Bundle_Sounds			mSounds;
	Bundle_FunDialog		mBundle_FunDialog;

	String					mGlobalNotify;

	String					mAvatarVersion;


	void					FadeAllMusicExcept(SoundStream* theException, float theTime=1);
	void					FadeInMusic(SoundStream* theMusic, float theTime=1);
	SoundStream*			mMusic_Title;
	SoundStream*			mMusic_Gameplay[4];
	SoundStream*			mMusic_Erase;

	bool					mAutoFix;
	bool					mUseJoypad;
	Point					mControlPos[5];
	String					mRememberName;

	AmbientSound			mHiss;
	AmbientSound			mThrob;

	int						mGameType;
	int						mGameOrder;	// For high scores... the order in the list.

	bool					mShowTutorDialog;
	bool					mDisableCustomMusic;

	enum
	{
		GameType_Kid=0,
		GameType_Flash,
		GameType_Expert,
		GameType_Master,
		GameType_Bossy,
		GameType_Mayhem,

		GameType_Disarmed,
		GameType_GirderLand,
		GameType_LoveNRockets,
		GameType_Telefort,
		GameType_SoCloseSoFar,

		GameType_Max
	};

	int							mMaxLevels;

	bool						mWinNoDying[GameType_Max];
	bool						mWinKillEverything[GameType_Max];
	bool						mWinGotEverything[GameType_Max];
	bool						mWinLevel[GameType_Max];

	bool						GetAchievement(String theAchievement);
	void						SubmitHighScore(String theBoard, String theName, int theScore);
	void						WinLevel(int theLevel);
	void						WinLevelNoDying(int theLevel);
	void						WinLevelGotEverything(int theLevel);
	void						WinLevelKillEverything(int theLevel);

#if defined(_WIN32) || defined(_LINUX)
	void						DroppedFile(String theFilename);
#endif

	void						DrawLoadingScreen(int theFlag);

	bool						mIsAdult=false;
	bool						mIsAdultChecked=false;
	bool						mHardSetChildMode=false;
	inline bool					NeedAdultCheck() {return false;}
	inline bool					IsAdult() {return true;}
	inline bool					IsAdultForMenu() {return true;}

	//
	// High Scores...
	//
	HighScoreList				mHS[GameType_Max];
	int							mJustGotHS;
	int							mJustGotHSGameType;

	CPU*						mMsgBox;
	//TitleScreen*				mTitleScreen;

	int							mHiliteLevel;

	bool						mIsKittyConnect;
	bool						mAskToRateLevels;

	bool						mAddon_KittyConnect;
	bool						mAddon_MoreLevels;

	bool						mAutoGamecenter;
	bool						mAcceptedTerms;

	// NEW COMMUNITY - default ON
	//bool						mAlwaysShowGlitchyLevels;
	bool						mAlwaysShowGlitchyLevels=true;

	// NEW COMMUNITY - default ON
	bool						mEnableRestartConfirm=true;

	//
	// Control stuff
	//
	bool						mTouchControls=true;

	int							mZoomIn=KB_PAGEUP;
	int							mZoomOut=KB_PAGEDOWN;
	int							mLeft=KB_LEFTARROW;
	int							mRight=KB_RIGHTARROW;
	int							mUp=KB_UPARROW;
	int							mDown=KB_DOWNARROW;
	int							mJump=KB_SPACE;
	int							mShoot=KB_X;
	int							mRocket=KB_C;
	int							mRocketUp=KB_V;
	int							mMenu=KB_ESCAPE;
	int							mMap=KB_TAB;
	bool						mPickControls=true;

};

class MyPauseOverlay : public PauseOverlay
{
public:
	void					Draw();
};

class CPUPlus : public CPU
{
public:
	CPUPlus() {mMaybeDrag=false;mDrag=false;}

	void				Core_TouchStart(int x, int y);
	void				Core_TouchMove(int x, int y);
	void				Core_TouchEnd(int x, int y);

	virtual void		StartDrag(int x, int y) {}
	virtual void		Drag(int x, int y) {}

	virtual void		Kill() {CPU::Kill();}


	bool				mMaybeDrag;
	bool				mDrag;
};


extern MyApp gApp;

#define DRAWGLOW(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled11Outline.Draw(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled11.Draw(text,x,y);
#define CENTERGLOW(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled11Outline.Center(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled11.Center(text,x,y);
#define RIGHTGLOW(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled11Outline.Right(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled11.Right(text,x,y);
#define CENTERUNDERGLOW(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled11Outline.Center(text,x,y);
#define RIGHTUNDERGLOW(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled11Outline.Right(text,x,y);
#define DRAWUNDERGLOW(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled11Outline.Draw(text,x,y);
#define DRAWOVERGLOW(text,x,y) 	gG.SetColor();gBundle_Play->mFont_Commodore64Angled11.Draw(text,x,y);
#define CENTEROVERGLOW(text,x,y) 	gG.SetColor();gBundle_Play->mFont_Commodore64Angled11.Center(text,x,y);
#define RIGHTOVERGLOW(text,x,y) 	gG.SetColor();gBundle_Play->mFont_Commodore64Angled11.Right(text,x,y);

#define DRAWGLOWBIG(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled18Outline.Draw(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled18.Draw(text,x,y);
#define CENTERGLOWBIG(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled18Outline.Center(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled18.Center(text,x,y);
#define RIGHTGLOWBIG(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled18Outline.Right(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled18.Right(text,x,y);
#define RIGHTGLOWBIG(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled18Outline.Right(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled18.Right(text,x,y);
#define CENTERUNDERGLOWBIG(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled18Outline.Center(text,x,y);
#define DRAWUNDERGLOWBIG(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled18Outline.Draw(text,x,y);
#define DRAWOVERGLOWBIG(text,x,y) 	gG.SetColor();gBundle_Play->mFont_Commodore64Angled18.Draw(text,x,y);
#define CENTEROVERGLOWBIG(text,x,y) 	gG.SetColor();gBundle_Play->mFont_Commodore64Angled18.Center(text,x,y);

#define DRAWGLOWSMALL(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled8Outline.Draw(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled8.Draw(text,x,y);
#define CENTERGLOWSMALL(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled8Outline.Center(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled8.Center(text,x,y);
#define RIGHTGLOWSMALL(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled8Outline.Right(text,x,y);gG.SetColor();gBundle_Play->mFont_Commodore64Angled8.Right(text,x,y);
#define CENTERUNDERGLOWSMALL(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled8Outline.Center(text,x,y);
#define DRAWUNDERGLOWSMALL(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled8Outline.Draw(text,x,y);
#define DRAWOVERGLOWSMALL(text,x,y) 	gG.SetColor();gBundle_Play->mFont_Commodore64Angled8.Draw(text,x,y);
#define CENTEROVERGLOWSMALL(text,x,y) 	gG.SetColor();gBundle_Play->mFont_Commodore64Angled8.Center(text,x,y);
#define RIGHTUNDERGLOWSMALL(text,x,y) 	gG.SetColor(.18f,.73f,.80f);gBundle_Play->mFont_Commodore64Angled8Outline.Right(text,x,y);gG.SetColor();
#define RIGHTOVERGLOWSMALL(text,x,y) 	gG.SetColor();gBundle_Play->mFont_Commodore64Angled8.Right(text,x,y);

class FunDialog : public Dialog
{
public:
	void				Initialize();
	void				DrawOverlay();
	void				SetupButtons();
	void				Back() {Notify(&this->mDone);}

public:
};

class GameSettingsDialog : public FunDialog
{
public:
	GameSettingsDialog() {mReturnTo=NULL;}
	void				ChangedControl(DialogWidget* theWidget);
	bool				Done();

public:
	DialogPanel*		mControls;
	DialogPanel*		mHack;
	DialogPanel*		mCloud;
	DialogButton*		mSignOut;
	DialogButton*		mSignIn;

	CPU*				mCloser;

	float				mSoundVolume;
	float				mMusicVolume;

	CPU*				mReturnTo;
};

bool IsMsgBoxActive();

#define MBHOOKPTR std::function<void(String theResult)>
#define MBHOOK(x) [=](String theResult) x

//#define OLDGO


class MsgBox : public CPU
{
public:

	MsgBox();
	virtual ~MsgBox();

#ifdef OLDGO
	bool					Go(String theText, Point theCenter, char theButtons);
#endif
	void					GoX(String theText, Point theCenter, char theButtons,MBHOOKPTR theCallback);
	void					GoNoThrottle(String theText, Point theCenter, char theButtons);
	void					Update();
	void					Draw();
	void					DrawOverlay();
	void					TouchStart(int x, int y);
	void					Notify(void* theData);
	void					Back() {Kill();}

	void					Click(String theValue);

public:
	bool					mIsThrottled;
	Rect					mBoxRect;
	String					mText;
	Button					mYes;
	Button					mNo;
	Button					mInvisibleCloseButton;
	char					mButtonType;
	float					mHoggyOffset;

	MBHOOKPTR				mCallback=NULL;
};


class GCPopup : public CPU
{
public:
	GCPopup(String theLine1, String theLine2);
	virtual ~GCPopup();

	void						Update();
	void						Draw();


public:
	//SpriteRect					mBorder;

	String						mLine1;
	String						mLine2;

	float						mYPos;
	int							mDelay;

	float						mWidth;
	bool						mMode;
};

bool IsNews();
String GetNews();

class MyMLBox : public MLBox
{
public:
	MyMLBox();
	virtual ~MyMLBox();
	void		DrawCustom(String theName, MLImage* theImage);
	bool		ClickLink(String theCommand);
	void		DrawPending();
	void		GoPending(String theString);
	void		Kill();
	void		Changed();
	void		Exe(Array<String>& theParams);

	String		mPendingString;
	int			mPendingTime;

	CPU*		mParent;

	void*		mAvatarContainer;
};

class Browser : public CPU
{
public:
	Browser();
	virtual ~Browser();

	void					Initialize();
	void					TouchStart(int x, int y);
	void					Update();
	void					Draw();
	void					DrawOverlay();
	void					Notify(void* theData);
	void					Back() {Notify(&mClose);}

	MyMLBox					mMLBox;
	String					mData;
	String					mTitle;

	char					mBackTo;
	float					mStep;

	Button					mClose;
	bool					mAllowTapClose=false;
	bool					mAllowClose=true;
};

class MsgBrowser : public Browser
{
public:
	void					Initialize();
	void					Draw();
	void					DrawOverlay();

	String					mData;
};


class Pending : public CPU
{
public:
	Pending(String theFunction, String theText, bool canCancel=true);

	void				Initialize();
	void				Update();
	void				Draw();
	void				TouchStart(int x, int y);

public:
	String				mFunction;
	String				mText;
	bool				mCanCancel;
	RComm::RQuery		mQuery;
	bool				mDidLog;
};
void FullscreenSmallScreen(Rect* theRect, Point theCenter);
String PFilter(Font& theFont, String theText, bool doColorize);

void DrawBKGTile(float x, float y, int theType=0);

extern MsgBrowser* gAdBrowser;

class TitleBox : public CPU
{
public:
	TitleBox();

	void				Initialize();
	void				Update();
	void				Draw();
	void				TouchStart(int x, int y);
	void				Go(CPUHOOKPTR theHook);

	String				mName;
	String				mAuthor;
	int					mLevelID;
	CPUHOOKPTR			mHook;

	int					mCountdown;
};
