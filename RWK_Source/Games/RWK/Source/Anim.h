#pragma once
#include "rapt.h"
#include "Player.h"

class Anim :
	public Object
{
public:
	Anim(void);
	virtual ~Anim(void);

public:
	Point		mPos;
};

class Anim_Sprite: public Anim
{ 
public:
	Anim_Sprite(Sprite *theSprite);
	virtual ~Anim_Sprite(void);

	virtual void			Update();
	virtual void			Draw();

public:
	Sprite					*mSprite;

	Point					mDir;
	Point					mDirMult;
	Point					mGravity;
	Point					mGravityMult;

	float					mFade;
	float					mFadeSpeed;
	float					mFadeMult;

	Color					mColor;
	Color					mColorStep;

	bool					mDoColorStep;
	inline void				ColorStep(Color theColor)
	{
		mColorStep=theColor;
		mDoColorStep=true;
	}
	bool					mWhite;

	float					mRotation;
	float					mRotationDir;

	float					mScale;
	float					mScaleSpeed;

	int						mTimer;

	bool					mAdditive;
};

class Anim_ExplodePop : public Anim
{
public:
	Anim_ExplodePop(Point thePos);

	void			Update();
	void			Draw();

public:
	float					mFade1;
	float					mFade2;
	float					mScale;
	float					mTotalScale;
	float					mOrbit;
	float					mOrbitSpeed;
	float					mWhiten;
};

class Anim_SpriteStrip : public Anim_Sprite
{
public:
	Anim_SpriteStrip(Array<Sprite> *theStrip);
	void			Update();
	void			Draw();

	Array<Sprite>	*mStrip;
	float			mMaxFrame;
	float			mFrame;
	float			mFrameSpeed;
	bool			mLoop;

};

class Anim_Glob : public Anim
{
public:
	Anim_Glob(Point thePos, Sprite* theSprite);

	void			Update();
	void			Draw();

public:
	int				mWait;
	float			mScale;
	float			mMelt;
	float			mMeltSpeed;
	Sprite*			mSprite;

};

class Anim_Bubble : public Anim
{
public:
	Anim_Bubble(Point thePos);
	
	void			Update();
	void			Draw();

public:
	float			mScale;
	float			mSpeed;
	float			mFade;
	int				mType;
	float			mPopOffset;

};

class Anim_Evaporate : public Anim
{
public:
	Anim_Evaporate(Sprite *theSprite, Point thePos);
	void			Update();
	void			Draw();

public:
	float			mWhiten;
	float			mFade;
	Sprite			*mSprite;
};

class Anim_GooSink : public Anim
{
public:
	Anim_GooSink(Point theRobotPos, IPoint theGooPos);
	void			Update();
	void			Draw();

public:
	float			mClipY;
	float			mGooLeft;
	float			mGooRight;
	float			mGooTop;
	Robot			mRobot;
	float			mSinkSpeed;
	int				mGooType;

};

class Anim_GooSinkKitty : public Anim
{
public:
	Anim_GooSinkKitty(Point theRobotPos, IPoint theGooPos);
	void			Update();
	void			Draw();

public:
	float			mClipY;
	float			mGooLeft;
	float			mGooRight;
	float			mGooTop;
	Kitty			mKitty;
	float			mSinkSpeed;
	int				mGooType;

};

class Anim_GooSinkRedguy : public Anim
{
public:
	Anim_GooSinkRedguy(Point theRobotPos, IPoint theGooPos);
	void			Update();
	void			Draw();

public:
	float			mClipY;
	float			mGooLeft;
	float			mGooRight;
	float			mGooTop;
	RedGuy			mKitty;
	float			mSinkSpeed;
	int				mGooType;

};

class Anim_BouncyBit: public Anim
{
public:
	Anim_BouncyBit(Sprite *theSprite, Point thePos);

	void			Update();
	void			Draw();

public:
	Sprite			*mSprite;
	float			mXDir;
	float			mBounce;
	float			mBounceMax;
	float			mFade;
	float			mRotate;
	float			mRotateSpeed;
	float			mScale;
};

class Anim_Wake : public Anim
{
public:
	Anim_Wake(int theType, Point thePos);
	void			Update();
	void			Draw();

	Sprite			*mSprite;

public:
	float			mScale;
	float			mModScale;
	float			mScaleCount;


};