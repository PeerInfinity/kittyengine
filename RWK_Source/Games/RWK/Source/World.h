
#pragma once
#include "rapt.h"
#include "MyApp.h"
#include "Player.h"
#include "SaveGame.h"
#include "TouchButtons.h"

#define EXPLODE_BLOCK 29
#define ACID_BLOCK 47
#define GUN_TARGET 48
#define EXPLODE_CRATE 30
#define COMPUTER_DOOR 32
#define VIRUS_DOOR 49
#define BITCOIN_DOOR 58
#define BOSS_BLOCK 37
#define TELEMATIC_BLOCK	38
#define TELEMATIC_BLOCK2 43
#define TELEMATIC_SLOT	39
#define TELEPAD_BLOCK	40
#define BREAK_BRICK	3
#define CONVEYOR_RIGHT 59
#define CONVEYOR_LEFT 60
#define ZOING_BLOCK 61
#define LEVER_BLOCK 62
#define COSMIC_BLOCK 63
#define SLAM_BLOCK 64
#define PICKUP_VELCRO 65
#define VELCRO_BLOCK 66
#define SECRET_BLOCK 67
#define SPIKE_BLOCK 68
#define SPIKE_CEILING_BLOCK 69
#define RADIO_BLOCK 70
#define PHAGE_BLOCK 71
#define PICKUP_SHOOTUP 72
#define MONSTERENERGY 73


#define SUPERGRID_X 30
#define SUPERGRID_Y 25

#define MAPTYPE char

#define ISSPIKE(x) (x==SPIKE_BLOCK || x==SPIKE_CEILING_BLOCK)
#define ISLAVA(x) (x==2)


class Processor : public Object
{
public:
	Rect					mRect;
	Point					mPos;
};

class Bubbler : public Processor
{
public:
	Bubbler()
	{
		mCountdown=gRand.Get(100);
	}

	void					Update();
	int						mCountdown;
	int						mType; // Red or Green
};


struct ZoingTimer
{
	IPoint	mGridPos;
	int		mTime;
};

class World :
	public CPUPlus
{
public:
	World(void);
	virtual ~World(void);

	void			Update();
	void			Draw();
	virtual void	DrawBackground();
	void			DrawWorld();
	Sprite			*GetDisplayTile(IPoint thePos);

	void			Initialize();

	void			InitializeGrid(int theWidth, int theHeight);
	void			SetGrid(int theX, int theY, char theValue);
	void			SetGridEX(int theX, int theY, char theValue);
	int				GetGrid(int theX, int theY);
	bool			IsBlocked(int theX, int theY, bool lavaBlocks=true);
	bool			IsBlockedY(int theX, int theY, float theYDir, bool lavaBlocks=true);
	bool			IsBlockedGround(int theX, int theY, bool lavaBlocks=true);
	bool			IsBlockedDir(int theX, int theY, float theDir, bool lavaBlocks=true);		// Addition of theDir allows us to go through one-way walls
	inline bool		IsBlocked(IPoint thePos, bool lavaBlocks=true) {return IsBlocked(thePos.mX,thePos.mY,lavaBlocks);}
	inline bool		IsBlockedDir(IPoint thePos, float theDir, bool lavaBlocks=true) {return IsBlockedDir(thePos.mX,thePos.mY,theDir,lavaBlocks);}
	bool			IsBlockedOneWayWall(int theX, int theY, float theDir);		// Addition of theDir allows us to go through one-way walls

	void			TouchStart(int x, int y);
	void			TouchMove(int x, int y);
	void			TouchEnd(int x, int y);
	void			MouseDoubleClick(int x, int y, int theButton);


	Point			GridToWorld(int theX, int theY);
	inline Point	GridToWorld(Point thePos) {return GridToWorld((int)thePos.mX,(int)thePos.mY);}
	inline Point	GridToWorld(IPoint thePos) {return GridToWorld(thePos.mX,thePos.mY);}
	Rect			GridToWorldRect(int theX, int theY);
	inline Rect		GridToWorldRect(Point thePos) {return GridToWorldRect((int)thePos.mX,(int)thePos.mY);}
	inline Rect		GridToWorldRect(IPoint thePos) {return GridToWorldRect(thePos.mX,thePos.mY);}

	IPoint			WorldToGrid(float theX, float theY);
	inline IPoint	WorldToGrid(Point thePos) {return WorldToGrid(thePos.mX,thePos.mY);}

	bool			Collide(Rect theRect);
	bool			CollideDir(Rect theRect, float theMoveDir);
	bool			CollideY(Rect theRect, float theMoveDir);
	bool			mCollidedWithLava;

	void			LoadLevel(String theName, bool oldVersion);
	void			Sync(SaveGame &theSaveGame);
	void			PositionPlayers(bool addPlayers=true);
	bool			IsPlayerAlreadyHere(int thePlayer, Point thePos);

	
public:
	static const int		mGridSize=40;

	//
	// Just a point for halfgrid, since we'll
	// use it a ton!
	//
	Point					mHalfGrid;
	int						mGridWidth;
	int						mGridHeight;
	struct Grid
	{
		unsigned int		mLayout:7;
		unsigned int		mPaint:9;
		unsigned int		mCustomDraw:1;	// If we draw it manually (conveyors, force fields, etc)
		unsigned int		mExtraData:6;	// Extra data for a custom draw
		unsigned int		mPaintID:9;
	};
	Grid					*mGrid;
	Grid					*GetGridPtr(int theX, int theY);
	MAPTYPE					*mLevelMap;

	char					*mOverGrid;

	//
	// No more Kitty... cannot win!
	//
	bool					mNoMoreKitty;
	float					mConveyorSpeed;
	float					mWantConveyorSpeed;
	float					mConveyorTick;

	inline void				ReverseConveyor() {mWantConveyorSpeed*=-1;}
	String					GetRadioMessage(IPoint thePos);

	//
	// Swipe
	//
	Point					mSwipeOffset;

	SmartList(ZoingTimer)	mZoingTimerList;


	//
	// Amount to zoom the world
	//
	float					mZoom;
	float					mInitialZoomSpeed;
	bool					mInitialZoom;
	void					ChangeZoom(float theChange);

	//
	// Position of center, and offset from
	// center to scroll.
	//
	Point					mScrollCenter;
	Point					mScrollOffset;
	float					mScrollSpeed;
	bool					mScrollSpeedCapped;

	//
	// If we're testing (vs playing)
	//
	bool					mTesting;
	int						mPlayTime;

	//
	// Players
	//
	ObjectManager			mPlayerList;
	ObjectManager			mAnimatorList_Top;
	ObjectManager			mAnimatorList_UnderPlayers;
	ObjectManager			mAnimatorList_UnderWorld;
	Robot					*mRobot;
	Kitty					*mKitty;

	//
	// Paused players
	//
	List					mPausedPlayerList;
	int						mPauseCheckCountdown;
	void					CheckPausedPlayers();

	//
	// If we died...
	//
	bool					mDied;

	//
	// Coin count
	//
	int						mCoins;
	int						mMaxCoins;

	//
	// Visible and Live Rects
	//
	Rect					mVisibleRect;
	Rect					mLiveRect;

	Rect					mWakeRect;
	Rect					mSleepRect;

	//
	// Various world effects
	//
	float					mFlashWhite;
	float					mShake;
	inline void				Shake(float theAmount) {mShake=theAmount;}

	//
	// Onscreen messages
	//
	void					AddMessage(String theMessage, String theUpMessage="", float thePlusFade=0.0f);
	String					mMessage;
	String					mUpMessage;
	float					mMessageFade;
	Rect					mMessageSize;
	Rect					mUpMessageSize;

	//
	// To break bricks (only one type can do this, but here it is)
	//
	void					Smash(Point thePos);
	void					Crack(Point thePos);

	//
	// Indicates we pushed on a block, like maybe a door block?
	//
	bool					PushWall(Point thePos);
	void					TurnOffComputrons();

	//
	// Whether to play music (editor doesn't)
	//
	bool					mPlayMusic;
	bool					mNeverPlayMusic;

	//
	// Respawn spot, if you die
	//
	Point					mSpawnSpot;
	int						mSpawnMusic;

	//
	// Get a volume that's adjusted for how close things are...
	//
	float					GetVolume(Point thePos);

	//
	// Button controller
	//
	TouchButtons			mButtons;
	enum
	{
		mButton_Move=0,
		mButton_Jump,
		mButton_Shoot,
		mButton_Rocket,
		mButton_RocketUp,

		mButton_Max
	};
	Point					mButtonPos[mButton_Max];

	void					Explode(int theX, int theY, int theCountdown=-1);
	void					UpdateExploders();
	struct Exploder
	{
		IPoint		mPos;
		int			mCountdown;

		void		Sync(SaveGame &theSaveGame)
		{
			theSaveGame.Sync(&mPos);
			theSaveGame.Sync(&mCountdown);
		}
	};
	List			mExploderList;

	void					Melt(int theX, int theY);

	void					UpdateAmbient();

	struct SuperGrid
	{
		~SuperGrid() {_FreeList(Processor,mProcessorList);}
		List		mProcessorList;
	};
	SuperGrid				*mSuperGrid;
	int						mSuperGridWidth;
	int						mSuperGridHeight;

	inline IPoint			GridToSuperGrid(int theX, int theY) {return IPoint(theX/SUPERGRID_X,theY/SUPERGRID_Y);}
	inline IPoint			GridToSuperGrid(IPoint thePos) {return GridToSuperGrid(thePos.mX,thePos.mY);}
	SuperGrid				*GetSuperGrid(int theX, int theY);
	inline SuperGrid		*GetSuperGrid(IPoint thePos) {return GetSuperGrid(thePos.mX,thePos.mY);}

	void					OpenComputerDoor(IPoint thePos);
	void					OpenVirusDoor(IPoint thePos);
	void					Touch(IPoint thePos);	// Activate anything here...
	IPoint					mLastTouch;				// Last thing we touched...

	void					RevealMap(IPoint thePos);
	MAPTYPE					*GetMapPtr(int theX, int theY);
	MAPTYPE					IsMapMarkable(int theX, int theY);

	//
	// Music stuff
	//
	int						mCurrentMusic;
	bool					mWantNewMusic;
	int						mNewMusic;
	int						mMusicDelay;
	inline void				SwitchMusic(int theSong) {if (mCurrentMusic!=theSong) {mWantNewMusic=true;mNewMusic=theSong;}}

	//
	// Boss blocks, for easy finding...
	//
	List					mBossBlockList;
	void					KilledBoss(IPoint thePos);
	bool					RemoveBossBlock(IPoint thePos);
	Array<IPoint>			mRemoveBossBlockPos;
	int						mRemoveBossBlockCountdown;

	//
	// Coin blocks, for easy finding
	//
	List					mBitcoinBlockList;
	bool					AllBitcoins();
	int						mRemoveBitcoinBlockCountdown;

	//
	// To connect teleporters up...
	//
	void					ConnectTeleporters();
	Telematic				*GetTelematic(IPoint thePos);

	//
	// Cracks...
	//
	struct Crack
	{
		IPoint mPos;
		int mLevel;
	};
	List					mCrackList;

	//
	// Darkness...
	//
	float					mDarkness;
	float					mDarkFadeDir;


	//
	// Timer
	//
	int						mTimer;
	int						mInitialTimerPause;		// Don't count up when this is positive... this goes to zero when you move
	int						mTimerPause;			// Pause from the timer apps

	//
	// To pause the game more better
	//
	void					Pause(bool theState);
	bool					mHideUI;

	//
	// To win!
	//
	virtual void			Win();
	bool					mWin;

	//
	// Fast time... when you die, adds to the clock
	//
	int						mFastTimeCountdown;

	//
	// Position the buttons from the setup thingie...
	//
	void					PositionButtons();
	

	//
	// To create players
	//
	Player					*CreatePlayerByID(int theID);

	//
	// Whether to draw gridding
	//
	bool					mIsEditor;

	Button					mMenuButton;
	void*					mInGameMenu;
	void					Notify(void *theData);
	void					Back() {Notify(&mMenuButton);}

	//
	// Custom Music
	//
	String					mCustomSong[3];
	void					StartMusic();
	bool					mMusicStarted;
	bool					mLastDisabledState;

	//
	// Radio tower text...
	//
	struct RadioText
	{
		IPoint	mPos;
		String	mText;
	};
	SmartList(RadioText)	mRadioTextList;


	RectComplex				mComputerDoorRect;
	RectComplex				mVirusDoorRect;

};


extern World *gWorld;

