
#pragma once
#include "rapt.h"
#include "MyApp.h"
#include "Makermall.h"

class Robot;

class SetupController : public CPUPlus
{
public:
	SetupController(void);
	virtual ~SetupController();

	void				Draw();
	void				Go();
	void				MouseMove(int x, int y);
	void				MouseDown(int x, int y, int theButton);
	void				MouseUp(int x, int y, int theButton);

	void				DrawMenuLevel0();	// Choose setup
	void				DrawMenuLevel1();	// Volume
	void				DrawMenuLevel2();	// OpenFeint
	void				DrawMenuLevel3();	// About
	void				DrawMenuLevel4();	// Controls

	void				CheckDefaulty();

public:
	int					mLevel;	// What level of setup
	//SpriteRect			mRim;

	CPUPlus				*mNotify;	// Who we notify when we press done...
	unsigned int		mLastPlay;

	Slider				mMusicVolume;
	Slider				mSoundVolume;
	void				Notify(void *theData);

	float				mYOffset;

};


class MainMenu :
	public CPUPlus
{
public:
	MainMenu(void);
	virtual ~MainMenu(void);

	void				Initialize();
	void				Update();
	void				Draw();
	void				Back();

	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);
	void				ProcessTouchMainTitle(int x, int y);
	void				ProcessTouchHighScores(int x, int y);
	void				ProcessTouchAddons(int x, int y);
	void				ProcessTouchLevelPicker(int x, int y);
	void				ProcessUnTouchLevelPicker(int x, int y);
	void				ProcessTouchResume(int x, int y);

	void				UpdateHighScores();

	void				DrawMainTitle();
	void				DrawHighScores();
	void				DrawAddons();
	void				DrawLevelPicker();
	void				DrawResume();
	void				Notify(void *theData);

	void				StartDrag(int x, int y);
	void				Drag(int x, int y);

public:
	float				mPulse[4];
	float				mStep;
	float				mForeStep;
	//SpriteRect			mSmallRim;
	int					mHiliteLevel;



	Robot				*mRobot;
	float				mFlash;

	bool				mFadeOut;
	float				mFade;
	int					mFadeAction;
	int					mWantLevel;

	void				StartAGame(int theLevel);

	int					mScrolling;
	float				mMainTitleOffset;
	float				mSetupOffset;
	float				mHighScoreOffset;
	float				mAddonsOffset;
	float				mLevelPickerOffset;
	float				mResumeOffset;

	Rect				mScoreTouchyRect;
	bool				mHideFlashingScore;

	TextBox				mHSEditBox;
	Button				mDoneButton;

	float				mHSScroll;
	float				mBlinkerScrollTo;
	bool				mManualScroll;
	float				mScrollSpeed;
	float				mMaxScroll;

	void				ComputeMaxScroll();

	SetupController		mSetup;
};

extern MainMenu *gMainMenu;

class ControlGadget : public CPUPlus
{
public:
	ControlGadget();
	virtual ~ControlGadget();

	void				Initialize();
	void				Update();
	void				Draw();

public:
	int					mSelected;

	Point				mControlSize[5];

	Point				mDragOffset;

	void				TouchMove(int x, int y);
	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);

};
extern ControlGadget *gControlGadget;

class KittyConnect : public CPUPlus
{
public:
	KittyConnect();
	virtual ~KittyConnect();

	void				Update();
	void				Draw();
	void				MouseDown(int x, int y, int theButton);

public:
	float				mStep;
	float				mGlow;
	//SpriteRect			mRim;

	float				mFade;
	float				mFadeSpeed;

	int					mFadeResult;

};

extern KittyConnect *gKittyConnect;

class InGameMenu : public CPUPlus
{
public:
	InGameMenu();
	virtual ~InGameMenu();

	void				Initialize();
	void				Update();
	void				Draw();

	void				TouchStart(int x, int y);
	void				TouchMove(int x, int y);
	void				TouchEnd(int x, int y);
	void				Notify(void *theData);

public:
	float				mFade;
	float				mFadeSpeed;

	Point				mMapScroll;

	SetupController		mSetup;
	bool				mIsSetup;
	bool				mIsMap;
	int					mFadeResult;

	void				DrawMap();

	struct Powerup
	{
		Powerup() {mCount=0;mStep=0;}
		int mID;
		float mX;
		int mCount;
		float mStep;
	};
	List				mPowerupList;
	float				mPowerupPos;
};

extern InGameMenu *gInGameMenu;

class WinGame : public CPUPlus
{
public:
	WinGame();
	virtual ~WinGame();

	void				Update();
	void				Draw();
	void				InstaPlayReponse();

	void				TouchStart(int x, int y);

public:
	float				mFade;
	float				mBumpScale;
	bool				mFadeOut;

	int					mWait;
};

extern WinGame *gWin;
extern int gHighScoreListNumber;
extern char gLevelOrder[];

class Transition : public CPU
{
public:
	Transition(CPU* theOldCPU, CPU* theNewCPU);
	virtual ~Transition();

	void				Update();
	void				Draw();

public:
	CPU*				mOldCPU;
	CPU*				mNewCPU;
	SmartList(Point)	mList;

	float				mProgress;
};

class PickGame : public CPU
{
public:
	PickGame();
	virtual ~PickGame();

	void				Initialize();
	void				Update();
	void				Draw();
	void				Notify(void* theData);
	void				StartAGame();

	void				TouchStart(int x, int y);
	void				TouchMove(int x, int y);
	void				TouchEnd(int x, int y);

	void				FinishScroll();


	float				mStep;
	bool				mDidSwipe;

	Button				mBack;
	Button				mPlay;
	Button				mReset;

	int					mNextPickLevel;
	int					mPickLevel;
	int					mLevelID;
	float				mPickScroll;
	float				mPickScrollStep;

	float				mPulse;

	bool				mDown;

	String				mLevelName;
	String				mLevelFN;
	bool				mIsSaveGame;
	void				FixLevelName(int theLevel);

	HighScoreList*		mScores;

	int					mCheckAgain;

	void				Back() {Notify(&mBack);}
};

