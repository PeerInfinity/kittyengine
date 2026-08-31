#pragma once
#include "rapt.h"
#include "SaveGame.h"

#define ID_NULL				0
#define ID_ROBOT			1
#define ID_LAZOR			2
#define ID_KITTY			3
#define ID_PICKUP			4
#define ID_COMPUTRON		5
#define ID_REDGUY			6
#define ID_BLUEGUY			7
#define ID_DRIP				8
#define ID_ACID				9
#define ID_BOSS				10
#define ID_COMPUTER			11
#define ID_REDBOSS			12
#define ID_REDBABY			13
#define ID_TELEMATIC		14
#define ID_TELEMATIC_SLOT	15
#define ID_TELEPAD			16
#define ID_DRIPBOSS			17
#define ID_ACIDBURNER		18
#define ID_GUNTARGET		19
#define ID_GREENGUY			20
#define ID_BUZZOID			21
#define ID_COIN				22
#define ID_LEVER			23
#define ID_RADIO			24
#define ID_PHAGE			25
#define ID_MONSTERENERGY	26


class Player : public Object
{
public:
	Player(void);
	virtual ~Player(void);

	void			Update();
	void			Draw() {}
	virtual void	Ouch(int theExtraDamage=0) {}
	void			InstaKill() {}
	virtual void	CheckLive();
	virtual void	CheckMyFeet();

	Point			Move(Point theDir, float theSpeed);

	virtual void	Sync(SaveGame &theSG);

	virtual void	EatEnergy();
	void			EatEnergyAt(int x, int y);
	virtual void	Energize() {}


public:

	int				mID;
	Point			mPos;
	int				mHP;
	bool			mIsEnemy;
	IPoint			mGridPos;
	//IPoint			mPreviousGridPos;
	char			mEnergyLevel;

	bool			mDying;

	//
	// Rectangle size for collisions
	//
	Rect			mCollide;
	void			FixPosition(bool doDrop=true);

	//
	// Sleep counter...
	//
	int				mSleep;

	bool			mCollidedWithLava;
};

class Robot : public Player
{
public:
	Robot(void);
	virtual ~Robot(void);

	void			Update();
	void			UpdateControls();
	void			UpdateGravity();
	Point			Move(Point theDir, float theSpeed);
	virtual void	CheckMyFeet();

	void			GoLeft();
	void			GoRight();
	void			Jump();
	void			Zoing(int theCount,IPoint theGridPos);
	void			Shoot();
	void			Rocket();
	void			StopRocket();
	void			RocketUp();
	void			StopRocketUp();

	void			Draw();
	void			DrawBot();
	void			DrawBoltGlow(Point thePos, int theBolt);

	void			Die(bool isLava, bool instantKill=false);
	void			Respawn();

	void			Sync(SaveGame &theSG);
	void			LavaDoubleCheck(int howFar=5);

	void			CheckVelcro(float aExtraDistance=0);

public:

	float			mSpeed;
	float			mFacing;
	float			mDrawFacing;
	float			mGravity;
	float			mGMod;
	float			mBounceOffset;
	float			mBounceGravity;
	bool			mPushing;
	float			mBob;
	bool			mWantBounce;
	bool			mInJump;
	int				mDoubleJumpCount;
	int				mTouchingGround;
	float			mJumpPitch;
	bool			mWantGoLeft;
	bool			mWantGoRight;

	int				mJumpKludge;

	bool			mOnSlamwall;
	bool			mLastSlamwall;


	//
	// For shooting...
	//
	int				mShootAnimateCountdown;
	int				mShootWaitCountdown;
	bool			mShootHeldDown;
	float			mBoltGlowAngle[2];

	//
	// For rocketing...
	//
	int				mRocketCountdown;
	bool			mRocketed;

	//
	// For rocketing up...
	//
	int				mRocketUpCountdown;
	bool			mRocketedUp;

	//
	// To draw the robot red, for various reasons
	//
	float			mRed;

	//
	// Burnup if robot starts falling too far and too fast
	//
	int				mBurnupCounter;
	float			mBurnup;

	bool			mZoinging;
	int				mZoingCount;


	//
	// Powers	
	//
	bool			mCanJump;
	bool			mCanDoubleJump;
	bool			mCanShoot;
	bool			mCanAnnihiliate;
	bool			mCanRocket;
	bool			mCanRocketUp;
	bool			mHasHelmet;
	bool			mHasKey[3];
	bool			mHasExplozor;
	bool			mHasHaxxor;
	bool			mHasVelcro;
	int				mCrystals;
	int				mGoldKeyCount;
	int				mLazorLevel;

	Array<int>		mPowerupOrder;
	int				mPowerupCount;

	IPoint			mGooPos;
	char			mOnVelcro;


	//
	// Player animators that need to draw on the same plane
	//
	ObjectManager	mAnimatorList;

	//
	// Invulnerability for a time after getting hit...
	//
	int				mInvulnerableCount;
	int				mNoControlCount;

	//
	// Hit points...
	//
	int				mMaxHP;

	//
	// Killpause
	//
	int				mKillPause;
};

class Lazor : public Player
{
public:
	Lazor(void);
	virtual ~Lazor(void);

	void			Update();
	void			Draw();
	void			ExplodeBlocks();

public:
	float			mGlint;
	float			mDir;
	float			mSpeed;

	int				mCheck;
	int				mDamage;
	bool			mExplode;

	void			Sync(SaveGame &theSG);

};

class Kitty : public Player
{
public:
	Kitty(void);
	virtual ~Kitty(void);

	void			Update();
	void			Draw();
	void			CheckMyFeet();

	void			DieKitty(int theType,IPoint theGridPos=IPoint(0,0));

public:
	float			mTail;
	int				mBlinkCountdown;

	bool			mFirstUpdate;
	bool			mFalling;
	float			mGravity;

	void			Sync(SaveGame &theSG);
	bool			mNullKitty;

};

class Pickup : public Player
{
public:
	Pickup(void);
	virtual ~Pickup();

	void			Update();
	void			Draw();

	enum
	{
		Pickup_Jump=0,
		Pickup_DoubleJump,
		Pickup_Shoot,
		Pickup_Rocket,
		Pickup_RocketUp,
		Pickup_Annihiliate,
		Pickup_Helmet,
		Pickup_Redkey,
		Pickup_Greenkey,
		Pickup_Bluekey,
		Pickup_Explozor,
		Pickup_Haxxor,
		Pickup_Timesaver,
		Pickup_Teleport,
		Pickup_Life,

		Pickup_ShootCancel,	//15
		Pickup_RocketUpCancel,
		Pickup_HelmetCancel,
		Pickup_Goldkey,
		Pickup_Velcro,
		Pickup_ShootUp,	// Extra shoot level

		Pickup_Max
	};

public:
	int				mType;
	float			mBob;
	bool			mShrink;
	float			mScale;

	void			Sync(SaveGame &theSG);

};

class Computron : public Player
{
public:
	Computron(void);
	virtual ~Computron();

	void			Update();
	void			Draw();

public:
	bool			mActivated;
	float			mEyeOffset;
	float			mEyeDir;
	int				mEyeWait;

	int				mRandSeed;
	int				mRandCountdown;

	int				mBlinkCountdown;
	int				mBlinkWait;

	float			mFlash;

	void			Sync(SaveGame &theSG);

};

class RedGuy : public Player
{
public:
	RedGuy(void);
	virtual ~RedGuy();

	void			Update();
	void			Draw();
	virtual void	DrawGuy();
	void			Ouch(int theExtraDamage=0);
	void			CheckMyFeet();

	virtual void	Energize() {mHP*=2;mEnergyLevel=_min(mEnergyLevel+1,3);}


public:
	float			mChomp;
	float			mStep;
	float			mFacing;

	float			mChompSpeed;
	float			mFlashWhite;

	int				mDyingCountdown;
	bool			mIsChild;
	int				mAge;

	void			Sync(SaveGame &theSG);

	bool			mIsFalling;
	float			mFallSpeed;
	int				mChecker;

};

class BlueGuy : public Player
{
public:
	BlueGuy(void);
	virtual ~BlueGuy();

	void			Update();
	void			Draw();
	void			DrawGuy();
	void			Ouch(int theExtraDamage=0);
	virtual void	CheckMyFeet() {}

	virtual void	Energize() {mHP*=2;mEnergyLevel=_min(mEnergyLevel+1,3);}

public:
	float			mFrame;
	float			mFrameDir;
	float			mFlashWhite;

	int				mDyingCountdown;

	float			mDir;
	int				mBlinkCountdown;

	float			mLookatAngle;
	float			mWantLookatAngle;
	float			mLookatDistance;
	float			mWantLookatDistance;

	void			Sync(SaveGame &theSG);
};

class Drip : public Player
{
public:
	Drip(void);
	virtual ~Drip();

	void			Update();
	void			Draw();
	void			DrawGuy();
	void			Ouch(int theExtraDamage=0);

	void			Sync(SaveGame &theSG);
	virtual void	CheckMyFeet() {}

	virtual void	Energize() {mHP*=2;mEnergyLevel=_min(mEnergyLevel+1,3);}



public:
	float			mFrame;
	float			mFlashWhite;

	int				mDyingCountdown;

	int				mShootCountdown;
	int				mShootPause;

	int				mCheckFooting;
};

class Acid : public Player
{
public:
	Acid();
	virtual ~Acid();

	void			Sync(SaveGame &theSG);


	void			Update();
	void			Draw();
	void			Plop();
	virtual void	CheckMyFeet() {}

	virtual void	Energize();


public:
	float			mRotate;
	float			mFallSpeed;
	Player*			mMyDrip;
};

class Boss : public Player
{
public:
	Boss();
	virtual ~Boss();

	void			Update();
	void			Draw();
	void			DrawGuy();
	void			Ouch(int theExtraDamage=0);
	void			CheckMyFeet();
	virtual void	EatEnergy();

	void			Sync(SaveGame &theSG);

	virtual void	Energize() {mHP*=2;mEnergyLevel=_min(mEnergyLevel+1,3);FixBossGraphic();}

public:
	Array<Sprite>*	mBoss;
	void			FixBossGraphic();
	float			mMouthFrame;
	int				mBlinkCountdown[11];
	float			mLookat[11];
	float			mBob;

	float			mSpeed;
	float			mDir;
	float			mFlashWhite;
	int				mDyingCountdown;
	int				mSlimeWait;

	int				mFireCountdown;
	int				mSpeedDownCounter;

	bool			mIsFalling;
	float			mFallSpeed;

};

class EnergyBolt : public Player
{
public:
	EnergyBolt();
	virtual ~EnergyBolt();

	void			Update();
	void			Draw();

public:
	float			mRotate;
	float			mPhase;

	Point			mDir;

	void			Sync(SaveGame &theSG);

};

class Computer : public Player
{
public:
	Computer(void);
	virtual ~Computer();

	void			Update();
	void			Draw();

public:
	bool			mHappy;
	int				mBlinkCountdown;
	float			mBob;

	float			mFlash;
	int				mHappyCountdown;

	void			Sync(SaveGame &theSG);

};

class RedBoss : public RedGuy
{
public:
	RedBoss();
	virtual ~RedBoss();

	void			Update();
	void			DrawGuy();
	void			Ouch(int theExtraDamage=0);
	void			CheckMyFeet();
	Array<Sprite>*	mBoss;

	void			FixBossGraphic();
	virtual void	Energize() {mHP*=2;mEnergyLevel=_min(mEnergyLevel+1,3);FixBossGraphic();}
	virtual void	EatEnergy();


public:
	int				mBirthing;
	int				mBirthingDelay;

	void			Sync(SaveGame &theSG);

};

class RedBaby : public Player
{
public:
	RedBaby(Point thePos);
	virtual ~RedBaby();

	void			Update();
	void			Draw();
	void			Ouch(int theExtraDamage=0);

public:
	float			mGravity;
	Point			mDir;
	float			mFlash;
	float			mRotate;
	int				mBounce;

	void			FixBoss();
	Array<Sprite>*	mBoss;

	void			Sync(SaveGame &theSG);
	virtual void	Energize() {mHP*=2;mEnergyLevel=_min(mEnergyLevel+1,3);}


};

class Telematic : public Player
{
public:
	Telematic();
	virtual ~Telematic();

	void			Update();
	void			Draw();
	void			AddCrystal(int theCount);

	void			Teleport(Player *theFrom);

	void			Sync(SaveGame &theSG);


public:
	int				mCrystalsNeeded;
	int				mCrystals;
	bool			mActivated;

	float			mEyeOffset;
	float			mEyeDir;
	int				mEyeWait;

	int				mRandSeed;
	int				mRandCountdown;

	int				mBlinkCountdown;
	int				mBlinkWait;

	float			mLizer[6];
	float			mLizerWant[6];

	float			mCycle;
	float			mFlash;

	IPoint			mLastTeleportTo;

	List			mTelepadList;
	void			SortTelepads();

	bool			mClockwise;


};

class Telematic_Slot : public Player
{
public:
	Telematic_Slot();
	virtual ~Telematic_Slot();

	void			Update();
	void			Draw();
public:

	bool			mActivated;
	float			mGlow;

	IPoint			mMyTelematic;
	bool			mProximity;

	void			Sync(SaveGame &theSG);

};

class Telepad : public Player
{
public:
	Telepad();
	~Telepad();

	void			Update();
	void			Draw();
	void			Sync(SaveGame &theSG);

public:
	int				mFacing;
	bool			mActivated;

	IPoint			mMyTelematic;
	float			mGlow;
	float			mPulse;

	float			mGrab;
	int				mNoGrab;
};

class DripBoss : public Player
{
public:
	DripBoss();
	~DripBoss();

	void			Update();
	void			Draw();
	void			DrawGuy();
	void			Ouch(int theExtraDamage=0);
	virtual void	EatEnergy();

	void			Sync(SaveGame &theSG);
	virtual void	CheckMyFeet() {}
	void			FixBossGraphic();
	Array<Sprite>*	mBoss;

	virtual void	Energize() {mHP*=2;mEnergyLevel=_min(mEnergyLevel+1,3);FixBossGraphic();}


public:
	int				mDyingCountdown;
	float			mFlashWhite;

	int				mBlinkCountdown;
	int				mBlinkWait;

	Point			mWiggle[7];
	Point			mOldPos;

	Point			mDir;
	Point			mDirMod;
	float			mSpeed;
	float			mSpeedDecay;

	int				mDripCountdown;
	int				mDripCountdownMax;

	int				mBadCounter;

};

class AcidBurner : public Player
{
public:
	AcidBurner();
	~AcidBurner();

	void			Update();
	void			Draw();

	void			Sync(SaveGame &theSG);
	void			MeltDown();
	void			HitRobot();

public:
	float			mFade;
	bool			mDidMelt;
	char			mFadeDir;
	IPoint			mFirstPos;
	IPoint			mGridPos;

	int				mMeltDownCountdown;
	int				mMeltDownType;
	float			mMeltDownMoveSpeed;

	int				mMeltCount;

	bool			mDoneMode;

	int				mKillCountdown;
};

class GunTarget : public Player
{
public:
	GunTarget();

	void			Update();
	void			Draw();
	void			Hit();



	void			Sync(SaveGame &theSG);

	bool			mOn;

	int				mChangeCount;

	float			mScale;
	float			mFlip;

	int				mCuredCountdown;
};

class Greenguy : public Player
{
public:
	Greenguy();

	void			Update();
	void			Draw();
	void			Sync(SaveGame &theSG);

public:
	int				mAge;
	float			mFacing;
	float			mRise;
	float			mWantRise;
	int				mSinkDelay;
	float			mFeelerOffset;

	Point			mArmPos[20][2];

	char			mCheckFooting;
};

class Buzzoid : public Player
{
public:
	Buzzoid();
	void			Update();
	void			Draw();
	void			Sync(SaveGame &theSG);
	void			Ouch(int theExtraDamage=0);
	virtual void	CheckMyFeet() {}
	virtual void	Energize() {mHP*=2;mEnergyLevel=_min(mEnergyLevel+1,3);}

public:
	float			mFacing;
	int				mWait;
	float			mSpeed;
	float			mFrame;
	int				mDyingCountdown;
	float			mFlashWhite;
	bool			mFirst;

};

class Coin : public Player
{
public:
	Coin();
	void			Update();
	void			Draw();

	void			Sync(SaveGame &theSG);

public:
	float			mFrame;
	float			mLift;
	float			mWhite;

};

class Lever : public Player
{
public:
	Lever();

	void			Update();
	void			Draw();
	void			Sync(SaveGame &theSG);

public:
	bool			mState;
	bool			mOnRobot;

};

class Radio : public Player
{
public:
	Radio();

	void			Update();
	void			Draw();
	void			Sync(SaveGame &theSG);

public:
	bool			mOnRobot;
	String			mText;
	int				mTick;

	struct Wireless
	{
		float		mScale;
		float		mAngle;
	};
	List			mWirelessList;

};

class Phage : public Player
{
public:
	Phage(void);
	virtual ~Phage(void);

	virtual void			Update();
	virtual void			Draw();
	void					Sync(SaveGame &theSG);
	void					Ouch(int theExtraDamage=0);

public:
	float					mAngle;
	float					mSpeed;
	float					mMaxSpeed;
	float					mTurnSpeed;
	float					mEatPitch;
	float					mScale;

	int						mMode;

	int						mMouth;
	char					mMouthTick;
	float					mEatScale;

	bool					mInList;
	int						mActiveCountdown;

};

class MonsterEnergy : public Player
{
public:
	MonsterEnergy();
	virtual void			Draw();

public:


};

#ifdef _DEBUG
extern Player* gCurrentPlayer;
#endif
