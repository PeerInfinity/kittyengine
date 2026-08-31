#pragma once

#include "rapt.h"
#include "rapt_utils.h"

#define COPY(x) x=theSource->x


static const int	gBind_To_Keyboard=			0x0000001;
static const int	gBind_To_Joystick=			0x0000002;
static const int	gBind_To_FPSMouse=			0x0000004;
static const int	gBind_To_DoubleKeyboard=	0x0000008;

enum
{
	CPUID_CPU=0,
	CPUID_TEXTBOX,
	CPUID_CHECKBOX,
	CPUID_NOTIFIER,
	CPUID_SLIDER,
	CPUID_CYCLER,
	CPUID_SWIPEBOX,
};

inline bool IsTextbox(CPU* theCPU) {if (!theCPU) return false;return theCPU->mID==CPUID_TEXTBOX;}
inline bool IsCheckbox(CPU* theCPU) {if (!theCPU) return false;return theCPU->mID==CPUID_CHECKBOX;}

//
// Notify extradata params...
//
enum
{
	HELPER_NOTIFY_PRESS=1,
	HELPER_NOTIFY_RELEASE,
	HELPER_NOTIFY_MOVE,
	HELPER_NOTIFY_ENTER,
	HELPER_NOTIFY_ESC,
	HELPER_NOTIFY_LOSTFOCUS,
	HELPER_NOTIFY_HOLDING,
	HELPER_NOTIFY_CLEAR,
	HELPER_NOTIFY_DRAG,
	HELPER_NOTIFY_TAB,

	HELPER_NOTIFY_MAX
};


//
// What's defined in here...
//
class PauseOverlay;
class Button;
class Slider;
//
// 
//

//
// A Pause Overlay is a helper class for if you pause the App (or otherwise want to
// stall things until the user clicks anywhere)
//
class PauseOverlay : public CPU
{
public:
	PauseOverlay();
	inline PauseOverlay*	SetID(int theID) {mID=theID;return this;}
	virtual void			TouchStart(int x, int y);
};

//
// A joypad CPU.  Designed to be visible/touchable on a touch device, but invisible
// and hooked to the keyboard or a joystick on a Win type device
//
class Joypad : public CPU
{
public:
	Joypad();
	inline Joypad*		SetID(int theID) {mID=theID;return this;}


	void				Go(Point theCenter, float theRadius, Sprite* theSprite=NULL);
	void				Go(Point theCenter, Sprite* theSprite=NULL);
	void				Update();
	void				Draw();
	
	void				TouchStart(int x, int y);
	void				TouchMove(int x, int y);
	void				TouchEnd(int x, int y);

	inline void			SetSprite(Sprite* theSprite) {mSprite=theSprite;}


public:
	Sprite				*mSprite;
	Point				mStickPosition;		// Position of Stick (can be 0,0)
	float				mStickAngle;		// Angle of stick (will not change angle for 0,0)
	float				mStickAngleOffset;	// Helper offset: use for SK type thumbwheels, so the joypad won't hop around.

	inline bool			IsDown() {return mIsDown;}
	bool				mIsDown;
	inline bool			IsRDown() {return mIsRDown;}
	bool				mIsRDown;	// If bound to FPSMouse, right button can also be flagged

	//
	// Lets us track whether state changed (useful in multiplayer, so you can send your messages right when
	// it changes)
	//
	inline bool			IsStateChanged() {bool aResult=mStateChanged;mStateChanged=false;return aResult;}
	bool				mStateChanged;

	//
	// This makes the joypad point at the touch position.  If this is false, it'll
	// use an offset so the pad doesn't hop around.  So if you have a joypad with an arrow on it or
	// something, this should be true;
	//
	bool				mPointAtTouch;
	inline void			PointAtTouch(bool theState=true) {mPointAtTouch=theState;}

	//
	// This lets you get stick info
	//
	inline float		GetAngle() {return mStickAngle;}
	inline Point		GetPosition() {return mStickPosition;}
	inline Point		GetDirection() {return mStickPosition.Normal();}

	//
	// If we want to manually tweak the Joypad, we can do this:
	//
	inline void			SetAngle(float theAngle) {mStickAngle=theAngle;mStickPosition=gMath.AngleToVector(theAngle);}
	inline void			SetDirection(Point theDir) {mStickPosition=theDir;mStickAngle=gMath.VectorToAngle(theDir);}

	//
	// This lets you bind the joypad to another device... this will not do ANYTHING on a touch
	// device, only on PCs
	//
	void				BindToKeyboard(int theUp=KB_UP, int theDown=KB_DOWN, int theLeft=KB_LEFT, int theRight=KB_RIGHT);
	void				BindToJoystick(int theJoystick, int theThumbstick=0);
	void				BindToFPSMouse(Point theCenter);

	bool				IsBoundToKeyboard() {return (mBinding&gBind_To_Keyboard)!=0;}
	bool				IsBoundToJoystick() {return (mBinding&gBind_To_Joystick)!=0;}
	bool				IsBoundToFPSMouse() {return (mBinding&gBind_To_FPSMouse)!=0;}

	int					mBinding;
	int					mKeyBind[4];
	int					mJoystickNumber;
	int					mThumbstickNumber;

	//
	// Center offset... you only need this for FPSMouse Joypads.  This allows you to move the center of the
	// joypad so that your character, or whatnot, represents the center.
	//
	Point				mCenterOffset;

	//
	// Data that stores some core data about the stick.  Used to allow us to refresh the stick...
	//
	IPoint				mLastTouchPos;
	void				RefreshTouch() {TouchMove(mLastTouchPos.mX,mLastTouchPos.mY);}

	//
	// Radius of pad
	//
	float				mRadius;

	//
	// This lets you make it so you actually drag the pad itself,
	// it's not just touch-sensitive.
	//
	bool				mDragPad;
	inline void			DragPad() {mDragPad=true;}
	Point				mDragOffset;
};

//
// Static text is just a simple thing that displays some text.
// I usually draw this manually, but ran into a situation where I wanted to build little elements
// inside a widget, and this is the easiest way to do so.
//
class StaticText : public CPU
{
public:
	StaticText() {}
	StaticText(String theText, Font* theFont, Color theColor, Point theTextPos)
	{
		mText=theText;
		mFont=theFont;
		mColor=theColor;
		mTextPos=theTextPos;
	}

	inline StaticText*	SetID(int theID) {mID=theID;return this;}

	void				Draw();

public:
	String				mText;
	Font*				mFont;
	Color				mColor;
	Point				mTextPos;
};



//
// Button is a simple push button set to notify the parent when pressed,
// released, or moved over.
//
class Button : public CPU
{
public:
	Button();
	virtual~ Button() {}
	inline Button*		SetID(int theID) {mID=theID;return this;}


	void				Update();
	virtual void		Draw();
	void				TouchStart(int x, int y);
	void				TouchMove(int x, int y);
	void				TouchEnd(int x, int y);
	void				DragStart(int x, int y);
	void				Enter();
	void				Leave();

	char*				WhoAreYou() {return "BUTTON";}


public:
	XBool				mIsDown;
	inline bool			IsDown() {return *mIsDown;}
	bool				mIsOver;
	inline bool			IsOver() {return mIsOver;}
	bool				mHolding;	// If we're holding it with the mouse
	bool				mDrawCentered;	// If we draw the sprites centered

	static bool			mSkipNotify;	// If we're going to skip notify... why?  If we derived a control, and we want to call Button's base functions but we want to have the custom class notify...

	inline bool			IsPressed() {bool aResult=*mIsDown;*mIsDown=false;return aResult;}


	//
	// Tells us the draw state:
	// 0 = normal
	// 1 = pressed
	// 2 = hilited
	//
	int					GetDrawState(int theButtonStates=3);
	//
	// If this is true, the button notifies on release.
	// Otherwise it notifies on press.
	//
	inline void			NotifyOnRelease() {SetNotifyConditions(false,true,false,false,false);}
	inline void			NotifyOnPress() {SetNotifyConditions(true,false,false,false,false);}
	inline void			NotifyOnMove() {SetNotifyConditions(false,false,true,false,false);}
	inline void			NotifyOnDrag() {SetNotifyConditions(false,false,false,true,false);EnableDrag(0);}
	inline void			NotifyOnDoubleClick() {SetNotifyConditions(false,false,false,false,true);}
	inline void			SetNotifyConditions(bool onPress, bool onRelease, bool onMove, bool onDrag, bool onDoubleClick) {mNotifyOnPress=onPress;mNotifyOnRelease=onRelease;mNotifyOnMove=onMove;mNotifyOnDrag=onDrag;mNotifyOnDoubleClick=onDoubleClick;}
	bool				mNotifyOnRelease;
	bool				mNotifyOnPress;
	bool				mNotifyOnMove;
	bool				mNotifyOnDrag;
	bool				mNotifyOnDoubleClick;
	bool				mAlwaysNotifyOnRelease=false; // Even notify on release if we're not over it (this is for slider thumbs)
	//
	// Sounds for press and hover...
	//
	Sound				*mHiliteSound;
	Sound				*mPressSound;
	Sound				*mReleaseSound;
	inline void			SetHiliteSound(Sound *theSound) {mHiliteSound=theSound;}
	inline void			SetPressSound(Sound *theSound) {mPressSound=theSound;}
	inline void			SetReleaseSound(Sound *theSound) {mReleaseSound=theSound;}
	inline void			SetSounds(Sound *thePressSound=NULL, Sound *theReleaseSound=NULL, Sound *theHiliteSound=NULL) {SetPressSound(thePressSound);SetReleaseSound(theReleaseSound);SetHiliteSound(theHiliteSound);}
	//
	// Sprites, for auto-draw
	//
#ifndef LEGACY_GL
	Array<Sprite*>		mSprite;
    float               mSpriteScale;
	void				SetSprites(Sprite& theNormal) {mSprite[0]=&theNormal;EnableDraw();Autosize();}
	void				SetSprites(Sprite& theNormal, Sprite& thePressed) {mSprite.GuaranteeSize(2,true);mSprite[0]=&theNormal;mSprite[1]=&thePressed;EnableDraw();Autosize();}
	void				SetSprites(Sprite& theNormal, Sprite& thePressed, Sprite& theHilite) {mSprite.GuaranteeSize(3,true);mSprite[0]=&theNormal;mSprite[1]=&thePressed;mSprite[2]=&theHilite;EnableDraw();Autosize();}
	void				SetSprites(Array<Sprite> &theArray);
	inline void			SetSprites(Array<Sprite*> &theArray) {mSprite=theArray;mSprite.mLockSize=true;}
	void				SetSprites(Point thePos, Array<Sprite> &theArray, bool thePosIsCenter=false);
	void				Autosize() {if (mSprite[0]) {mWidth=mSprite[0]->WidthF();mHeight=mSprite[0]->HeightF();}}
#else
	Sprite*				mSprite[3]={NULL,NULL,NULL};
	float               mSpriteScale;
	void				SetSprites(Sprite& theNormal) {mSprite[0]=&theNormal;EnableDraw();Autosize();}
	void				SetSprites(Sprite& theNormal, Sprite& thePressed) {mSprite[0]=&theNormal;mSprite[1]=&thePressed;EnableDraw();Autosize();}
	void				SetSprites(Sprite& theNormal, Sprite& thePressed, Sprite& theHilite) {mSprite[0]=&theNormal;mSprite[1]=&thePressed;mSprite[2]=&theHilite;EnableDraw();Autosize();}
	void				SetSprites(Array<Sprite> &theArray);
	inline void			SetSprites(Array<Sprite*> &theArray) {mSprite[0]=theArray[0];mSprite[1]=theArray[1];}
	void				SetSprites(Point thePos, Array<Sprite> &theArray, bool thePosIsCenter=false);
	void				Autosize() {if (mSprite[0]) {mWidth=mSprite[0]->WidthF();mHeight=mSprite[0]->HeightF();}}
#endif


	//
	// Dragthru means if you click on the button, and move a distance, the button will lose focus and instead will instigate a click below it.
	// This was put in explicitely for slideboxes containing buttons-- so that if you click on a button and drag, it'll go through and let you
	// drag the rest of it.
	//
	bool				mDragThruH;
	bool				mDragThruV;
	float				mDragThruThreshold;
	bool				mDragThruIgnoreTouchEnd;	// I put this in to kludge my MLRender stuff...
													// When a button has DragThru, if this is true (default) it'll ignore next touch end,
													// otherwise it won't!

	inline void			DragThruV() {mDragThruV=true;}
	inline void			DragThruH() {mDragThruH=true;}
	inline void			DragThru() {DragThruV();DragThruH();}
	inline void			SetDragThruThreshold(float theAmount) {mDragThruThreshold=theAmount;}

	void				BindToKeyboard(int theKey, int theKey2=-1, int theKey3=-1);
	void				BindToDoubleKeyboard(int theKey, int theOtherKey, int theKey2=-1, int theKey3=-1);
	int					mBinding;
	int					mKey[4];

	bool				IsBoundToKeyboard() {return (mBinding&gBind_To_Keyboard)!=0;}
	bool				IsBoundToJoystick() {return (mBinding&gBind_To_Joystick)!=0;}
	bool				IsBoundToFPSMouse() {return (mBinding&gBind_To_FPSMouse)!=0;}

	//
	// You can set an exe, if you don't want to notify another CPU
	// HookArg passes "this" as theArg to the lambda
	//
	inline void			Exe(HOOKARGPTR theHook=NULL) {mExe=theHook;} 
	inline void			SetExe(HOOKARGPTR theHook=NULL) {mExe=theHook;} 
	inline void			SetNotifyHook(HOOKARGPTR theHook=NULL) {mExe=theHook;} 

	HOOKARGPTR			mExe=NULL;
	inline Button*		PointAt(bool* thePtr) {mIsDown.PointAt(thePtr);return this;}

	inline bool			GetState() {return *mIsDown;}
	inline void			SetState(bool theState) {*mIsDown=theState;}
};

class RepeatingButton : public Button
{
public:
	RepeatingButton();
	inline RepeatingButton*		SetID(int theID) {mID=theID;return this;}

	void				Update();
	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);

	int					mRepeatDelay;
	int					mRepeatCount;
};
//
// CheckBox is a simple button that switches state whenever pressed
//
class CheckBox : public Button
{
public:
	CheckBox() {mID=CPUID_CHECKBOX;}
	inline CheckBox*	SetID(int theID) {mID=theID;return this;}

	void				TouchStart(int x, int y);
	void				TouchMove(int x, int y) {}
	void				TouchEnd(int x, int y) {}
	void				Leave();
	void				Update();
};

//
// Cycler is a checkbox that walks through all the sprites for its state
//
class Cycler : public Button
{
public:
	Cycler() {mID=CPUID_CYCLER;}
	inline Cycler*		SetID(int theID) {mID=theID;return this;}

	void				TouchStart(int x, int y);
	void				TouchMove(int x, int y) {}
	void				TouchEnd(int x, int y) {}
	void				Leave();
	void				Update();
	void				Draw();

	int					mState=0;
	inline int			GetState() {return mState;}
	inline void			SetState(int theState) {mState=theState;}
};


//
// Slider is a horizontal slider bar (such as a volume slider) that
// always returns a value from 0 to 1, and notifies the parent when
// the thumb is moved.
//
#define SLIDERHOOK(func) [&](Slider* theThis) func
#define SLIDERHOOKPTR std::function<void(Slider* theThis)>


class Slider : public CPU
{
public:
	Slider();
	inline Slider*		SetID(int theID) {mID=theID;return this;}

	void				Draw();
	void				DrawOverlay();
	void				DrawToolTip(); // Draws over every single widget
	void				Notify(void *theData);
	void				SetBorder(float theBorderWidth);
	char*				WhoAreYou() {return "SLIDER";}

public:
	float				mBorder;	// How much space to force at left/right edges
	Button				mThumb;
	inline Button&		GetThumb() {return mThumb;}

	float				mInternalValue;
	XFloat				mValue;

	Point				mMinMax=Point(0,1);
	inline float		GetValue() {if (!isnan(mSnap)) return ::Snap(mMinMax.mX+(mMinMax.Diff()*mInternalValue),mSnap);return mMinMax.mX+(mMinMax.Diff()*mInternalValue);}
	Slider*				SetValue(float theValue);
	Slider*				SetInternalValue(float theValue);
	Slider*				SetValue();
	inline Slider*		PointAt(float* thePtr) {mValue.PointAt(thePtr);SetInternalValue(gMath.ReverseInterpolate(mMinMax.mX,*thePtr,mMinMax.mY));*mValue=GetValue();return this;}
	inline float*		GetPointAt() {return mValue.WhoAreYou();}

	inline Slider*		SetRange(float theLow, float theHigh) {mMinMax.mX=theLow;mMinMax.mY=theHigh;SetValue(_clamp(theLow,GetValue(),theHigh));return this;}
	inline Slider*		SetMinMax(float theLow, float theHigh) {mMinMax.mX=theLow;mMinMax.mY=theHigh;SetValue(_clamp(theLow,GetValue(),theHigh));return this;}
	inline Slider*		SetMin(float theLow) {mMinMax.mX=theLow;return this;}
	inline Slider*		SetMax(float theHigh) {mMinMax.mY=theHigh;return this;}

	float				mSnap=NAN;
	inline Slider*		SetSnap(float theSnap) {mSnap=theSnap;SetInternalValue(mInternalValue);return this;}

	void				ChangeValue(float theStep);

	void				Copy(Slider* theSource)
	{
		CPU::Copy(theSource);
		COPY(mBorder);
		mThumb.Copy(&theSource->mThumb);
		COPY(mInternalValue);
		COPY(mMinMax);
		COPY(mBackground);
		COPY(mThumbDragOffset);
		COPY(mIsRepeater);
		COPY(mSimple);
		COPY(mSimple_BackgroundColor);
		COPY(mSimple_ThumbColor[0]);
		COPY(mSimple_ThumbColor[1]);
		COPY(mExe);
		COPY(mSDrawHook);
		mThumb.SetNotify(this);
	}

	//
	// To set the sprite for the slider.  If isRepeater is true, it will break the
	// sprite into three pieces, so that the length can be varied.
	//
	void				SetSprite(Sprite& theSprite, bool isRepeater=true) {mBackground=&theSprite;mIsRepeater=isRepeater;EnableFeature(CPU_DRAW);}

	void				Size(Point thePos) {if (mBackground) Size(thePos,(float)mBackground->mWidth); else Size(thePos,10);}
	void				Size(Point thePos, float theWidth);
	inline void			Size(float theX, float theY) {Size(Point(theX,theY));}
	inline void			Size(float theX, float theY, float theWidth) {Size(Point(theX,theY),theWidth);}
	inline void			Size(Rect theRect) {CPU::Size(theRect);}
	inline Slider*		SetHeight(float theHeight) {mHeight=theHeight;return this;}

	Sprite				*mBackground;
	Point				mThumbDragOffset;
	bool				mIsRepeater;

	bool				mSimple=false;
	Color				mSimple_BackgroundColor;
	Color				mSimple_ThumbColor[2];
	inline Slider*		Simple(Rect theRect, Color theBKGColor, Color theThumbColor=Color(1), Color theThumbHiliteColor=Color(0,1,0))
	{
		Size(theRect);
		mBorder=3;
		mThumb.Size(0,0,mWidth/5,mHeight);
		mSimple=true;
		mSimple_BackgroundColor=theBKGColor;
		mSimple_ThumbColor[0]=theThumbColor;
		mSimple_ThumbColor[1]=theThumbHiliteColor;
		EnableFeature(CPU_DRAW);
		SetBorder(mThumb.mWidth/2);
		return this;
	}

	inline Slider*		SetThumbSize(Rect theRect) {mThumb.Size(theRect);SetValue(GetValue());return this;}
	inline Slider*		SetThumbSize(float theWidth) {mThumb.Size(Rect(0,0,theWidth,mHeight));SetValue(GetValue());return this;}
	inline Slider*		SetThumbSize(float theWidth, float theHeight) {mThumb.Size(Rect(0,0,theWidth,theHeight));SetValue(GetValue());return this;}


	//
	// You can set an exe, if you don't want to notify another CPU
	// HookArg passes "this" to the lambda
	//
	inline void			Exe(HOOKARGPTR theHook=NULL) {mExe=theHook;} 
	HOOKARGPTR			mExe=NULL;
	inline Slider*		SetExe(HOOKARGPTR theHook) {mExe=theHook;return this;}

	HOOKARGPTR			mReleaseHook=NULL;
	inline Slider*		SetReleaseHook(HOOKARGPTR theHook) {mReleaseHook=theHook;return this;}

	SLIDERHOOKPTR		mSDrawHook=NULL;
	inline Slider*		SetDrawHook(SLIDERHOOKPTR theHook) {mSDrawHook=theHook;EnableFeature(CPU_DRAW);return this;}

	// Lets you optinally draw over the thumb, it's translated to the thumb's center.  theArg is this
	SLIDERHOOKPTR		mDrawThumbHook=NULL;
	inline Slider*		SetDrawThumbHook(SLIDERHOOKPTR theHook) {mDrawThumbHook=theHook;EnableFeature(CPU_DRAW);return this;}

	inline Slider*		SetExtraData(void* theData) {ExtraData(0)=theData;return this;}
	inline Slider*		SetExtraData(int theSlot, void* theData) {ExtraData(theSlot)=theData;return this;}

	/*
		//
		// Simplest slider hook...
		//
		gG.SetColor(...);
		gG.FillRect(0,0,theThis->mWidth,theThis->mHeight,_min(theThis->mWidth/2,theThis->mHeight/2));
		gG.SetColor(thumbcolor[mThumb.IsDown()]);
		Rect aTRect=theThis->mThumb.Expand(-3);
		gG.FillRect(aTRect,_min(aTRect.mWidth/2,aTRect.mHeight/2));
		gG.SetColor();
	*/


};

//
// Ticker is a background object that just changes state after a set duration.
// You can use it for everything from flashing a cursor to timing a metronome.
//
class Ticker : public CPU
{
public:
	Ticker();
	inline Ticker*		SetID(int theID) {mID=theID;return this;}

	void				Update();
	void				SetTick(int theTick) {mTick=theTick;mTickCounter=0;}
	void				SetState(int theState) {mState=theState;mTickCounter=0;}
	void				SetStateMax(int theStateMax) {mStateMax=theStateMax;}
	bool				IsChanged() {bool aResult=mIsChanged;mIsChanged=false;return aResult;}
	int					GetState() {return mState;}

public:
	int					mTick;
	int					mTickCounter;
	int					mState;
	int					mStateMax;
	bool				mIsChanged;
};

//
// SwipeBox is a CPU that allows the player to scroll it, for instance, a map.
// Drawing is entirely up to the user-- all drawing is centered at 0,0, which 
// is the center of the Swiper's scroll region.
//
// ConvertULWorldToWorld can be used to convert draw coordinates into Upper-Left
// format.
//
class SwipeBox : public CPU
{
public:
	SwipeBox();
	inline SwipeBox*	SetID(int theID) {mID=theID;return this;}

	void				Core_Update();
	void				Core_Draw();
	virtual void		Core_DrawUnder();

	//
	// These are for drawing immobile backgrounds or overlays.  These parts won't scroll
	// with the swipe box.
	//
	virtual void		DrawUnder() {}
	virtual void		DrawOver() {}

	inline void			SetFriction(float theFriction) {mFriction=theFriction;}

	void				Core_TouchStart(int x, int y);
	void				Core_TouchMove(int x, int y);
	void				Core_TouchEnd(int x, int y);
/*
	CPU*				HitTest(float x, float y);
*/

	void				SetSwipeWidth(float theWidth) {if (mSwipeWidth!=theWidth) {mSwipeWidth=theWidth;SwipeAreaResized();}}
	void				SetSwipeHeight(float theHeight) {if (mSwipeHeight!=theHeight) {mSwipeHeight=theHeight;SwipeAreaResized();}}
	void				SetSwipeSize(float theWidth, float theHeight) {if (mSwipeWidth!=theWidth || mSwipeHeight!=theHeight) {mSwipeWidth=theWidth;mSwipeHeight=theHeight;SwipeAreaResized();}}

	virtual void		SwipeAreaResized();

	Rect				GetVisibleArea();


	//
	// Convert coordinates from current scroll position
	// to the "real" position.  You can send the x/y from
	// touch commands through this to get the real position
	// on the SwipeBox.
	//
	Point				ConvertVisibleToWorld(float theX, float theY);
	inline Point		ConvertVisibleToWorld(int theX, int theY) {return ConvertVisibleToWorld((float)theX,(float)theY);}
	inline Point		ConvertVisibleToWorld(Point thePos) {return ConvertVisibleToWorld(thePos.mX,thePos.mY);}

	Point				ConvertWorldToVisible(float theX, float theY);
	inline Point		ConvertWorldToVisible(int theX, int theY) {return ConvertWorldToVisible((float)theX,(float)theY);}
	inline Point		ConvertWorldToVisible(Point thePos) {return ConvertWorldToVisible(thePos.mX,thePos.mY);}

public:

	bool				mHitThreshold;
	float				mSwipeThreshold;
	float				mFriction;
	float				mSwipeWidth;
	float				mSwipeHeight;

	float				mScrollX;
	float				mScrollY;
	char				mCanScroll;	// If we can scroll at all
	bool				mAbortSwipe=false;

	virtual void		SetPos(float theX=0, float theY=0);
	inline void			SetPos(Point thePos) {SetPos(thePos.mX,thePos.mY);}
	inline Point		GetPos() {return Point(mScrollX,mScrollY);}
	void				Clamp();

	inline bool			CanScroll() {return (mCanScroll!=0);}
	inline bool			CanScrollV() {return ((mCanScroll&0x01)!=0);}
	inline bool			CanScrollH() {return ((mCanScroll&0x02)!=0);}

	Point				mSwipeAnchor;
	Point				mSwipeSpeed;
	bool				mIsDown;
	float				mSwipeSpeedMod;

	//
	// You can lock it so that you can't scroll horizontally or vertically
	//
	Point				mScrollLock;
	void				LockScrollX(bool theState=true) {if (theState) mScrollLock.mX=0;else mScrollLock.mX=1;}
	void				LockScrollY(bool theState=true) {if (theState) mScrollLock.mY=0;else mScrollLock.mY=1;}

	//
	// Tells us if we're scrollable
	//
	bool				IsScrollableX() {return (mSwipeWidth>mWidth);}
	bool				IsScrollableY() {return (mSwipeHeight>mHeight);}
	bool				IsScrollable() {return IsScrollableX()|IsScrollableY();}

	//
	// Lets you stop swipe scrolling... so if you click on a particular place, you could say, okay, we're not going
	// to scroll...
	//
	void				StopSwiping() {mAbortSwipe=true;}

	//
	// Lets you bind mousewheel...
	//
	inline void 		BindSpinsH() {mSpinDir=Point(1,0);FocusSpins(true);}
	inline void			BindSpinsV() {mSpinDir=Point(0,1);FocusSpins(true);}
	inline void			UnbindSpins() {mSpinDir=Point(0,0);FocusSpins(false);}
	Point				mSpinDir=Point(0,1);
	float				mSpinSpeed;
	void				Spin(int theDir);

	//
	// Guarantees a visible position...
	//
	void				GuaranteeVisible(float theX, float theY);
	inline void			GuaranteeVisible(Point thePos) {GuaranteeVisible(thePos.mX,thePos.mY);}

	//
	// The draw offset is for various custom uses... normally it's just 0,0,
	// but if you want to do transitions or something, it's useful.
	//
	Point				mDrawOffset;

	//
	// You can reverse the way the swipebox handles swipes...
	//
	bool				mReverseMouse;
	inline void			ReverseMouse(bool theState=true) {mReverseMouse=theState;}

	//
	// Converts Touch coordinates into swipey zone
	//
	inline Point		ToLocal(Point thePos) {return thePos+Point(mScrollX,mScrollY);}
	inline Point		ToLocal(int theX, int theY) {return ToLocal(Point(theX,theY));}
	inline Point		ToLocal(float theX, float theY) {return ToLocal(Point(theX,theY));}
};

class SwipePages : public SwipeBox
{
public:
	SwipePages()
	{
		mPageSizeX=0;	// 0 means use the size of the swipebox
		mPageSizeY=0;

		mWantScrollX=0;
		mWantScrollY=0;
	}
	inline SwipePages*	SetID(int theID) {mID=theID;return this;}


public:
	float				mPageSizeX;
	float				mPageSizeY;

	float				mWantScrollX;
	float				mWantScrollY;

	inline void			SetPageSizes(float theX, float theY) {mPageSizeX=theX;mPageSizeY=theY;}
	inline void			SetPageSizeX(float theX) {mPageSizeX=theX;}
	inline void			SetPageSizeY(float theY) {mPageSizeY=theY;}

	void				Core_Update();
	void				Core_TouchStart(int x, int y);
	void				Core_TouchMove(int x, int y);
	void				Core_TouchEnd(int x, int y);

	void				ScrollTo(int thePageX, int thePageY);

};

//
// Swipelist is a container class to hold buttons in a way that allows users
// to swipe on the iPhone.  You simply add sprites to the Swipelist and
// it'll handle it all until it's time to tell you someone selected one.
//

template <class var_type>
class SwipeList : public SwipeBox
{
public:
	SwipeList()
	{
		mFriction=.95f;
		mAddXPos=0;
		mXSpacing=10;
		mYSpacing=10;
		mIsDown=false;
		mPickedItem=NULL;
		mPreviousPickedItem=NULL;
		mHoverItem=NULL;
		mRows=1;
		mRowCounter=0;
		mYOffset=0;
		mFont=NULL;

		EnableFeature(CPU_CLIP);
	}
	virtual ~SwipeList() {Reset();}
	inline SwipeList*	SetID(int theID) {mID=theID;return this;}

	void				Reset()
	{
		mAddXPos=0;
		mRowCounter=0;
		mSwipeWidth=0;
		mSwipeHeight=0;
		mPickedItem=NULL;
		mPreviousPickedItem=NULL;
		mItemList.Clear();
	}
	void				Go() {if (mSwipeWidth<mWidth) EnumSmartList(SwipeListItem,aItem,mItemList) aItem->mCenter.mX+=(mWidth-mSwipeWidth)/2;}
	void				Update() {mScrollY=0;}

	void				Draw() {G()->PushTranslate();DrawItems();if (mPickedItem) DrawHilite();G()->PopTranslate();}

	//
	// Should override this, default behavior is lame.
	//
	virtual void		DrawHilite() {if (mPickedItem->mSprite) {G()->RenderWhite(true);G()->SetColor(.5f);mPickedItem->mSprite->Center(mPickedItem->mCenter);G()->SetColor();G()->RenderWhite(false);}}

	//
	// Gets called if the cursor mouses over an item (PC/Mac only)
	// Only gets called if item you're over CHANGES.
	//
	virtual void		Hover() {}

	//
	// You should override this if you need anything better than "draw item's sprite and text."
	//
	virtual void		DrawItems()	{EnumSmartList(SwipeListItem,aItem,mItemList) {if (!aItem->mSprite) continue;aItem->mSprite->Center(aItem->mCenter);if (aItem->mText.Len()>0 && mFont) mFont->Center(aItem->mText,aItem->mCenter.mX,(aItem->mCenter.mY+aItem->mSprite->mHeight/2)+mFont->mPointSize);}}


	inline void			SetFriction(float theFriction) {mFriction=theFriction;}
	inline void			SetSpacing(float theSpacing) {mXSpacing=theSpacing;mYSpacing=theSpacing;}
	inline void			SetSpacing(float theXSpacing, float theYSpacing) {mXSpacing=theXSpacing;mYSpacing=theYSpacing;}
	inline void			SetYOffset(float theYOffset) {mYOffset=theYOffset;}

	void				TouchStart(int x, int y) {mCanSelect=true;mFirstScrollAnchor=(float)x;Pick(x,y);}
	void				TouchMove(int x, int y)
	{
		void* aOldHoverItem=mHoverItem.mRef;
		mHoverItem=GetItem(x,y);

		if (mIsDown)
		{
			SetCursor(CURSOR_HAND);
			if (mCanSelect) if (fabs(mFirstScrollAnchor-x)>mXSpacing) {	mPickedItem=NULL;mCanSelect=false; }
		}
		else { if (mHoverItem) SetCursor(CURSOR_FINGER); else SetCursor(CURSOR_HAND); }


		if (mHoverItem.mRef!=aOldHoverItem) Hover();
	}
	void				TouchEnd(int x, int y) { if (mCanSelect) if (mNotify) mNotify->Notify(this); }


public:

	struct SwipeListItem
	{
		SwipeListItem() {mSprite=NULL;mExtraData=NULL;}
		Point		mCenter;
		Sprite*		mSprite;
		int			mID;
		var_type	mExtraData;
		String		mText;
		IPoint		mSize;
	};
	SmartList(SwipeListItem)	mItemList;

	inline void					SetFont(Font *theFont) {mFont=theFont;}
	Font						*mFont;

	Smart(SwipeListItem)		AddItem(int theID, int theWidth, int theHeight, var_type theExtraData=NULL)
	{
		if (mAddXPos==0)
		{
			mAddXPos+=mXSpacing;
			mSwipeWidth+=mXSpacing;
		}

		Smart(SwipeListItem) aItem=new SwipeListItem();
		mItemList+=aItem;

		aItem->mID=theID;
		aItem->mSprite=NULL;
		aItem->mSize=IPoint(theWidth,theHeight);
		aItem->mExtraData=theExtraData;
		aItem->mCenter.mX=mAddXPos+(theWidth/2);
		aItem->mCenter.mY=(mHeight/2);
		aItem->mCenter.mY=0;
		if (mRows>1) 
		{
			float aTotalHeight=(theHeight*mRows)+(mYSpacing*(mRows-1));

			float aR=(float)(mRows-1)/2.0f;
			aItem->mCenter.mY+=(mRowCounter-aR)*(theHeight+mYSpacing);
		}
		aItem->mCenter.mY+=mYOffset;
		aItem->mCenter.mY+=mHeight/2;

		if (mRowCounter==0)
		{
			mSwipeWidth+=theWidth;
			mSwipeWidth+=mXSpacing;
		}
		mRowCounter++;
		if (mRowCounter>=mRows) 
		{
			mAddXPos+=theWidth;
			mRowCounter=0;
			mAddXPos+=mXSpacing;
		}

		return aItem;
	}
	Smart(SwipeListItem)		AddItem(int theID, Sprite *theSprite, var_type theExtraData=NULL)
	{
		Smart(SwipeListItem) aItem=AddItem(theID,theSprite->Width(),theSprite->Height(),theExtraData);
		if (!aItem) return aItem;
		aItem->mSprite=theSprite;
		return aItem;
	}
	Smart(SwipeListItem)		AddItem(int theID, Sprite *theSprite, String theText, var_type theExtraData)
	{
		Smart(SwipeListItem) aItem=AddItem(theID,theSprite,theExtraData);
		if (mFont) aItem->mText=mFont->WrapEX(theText,theSprite->mWidth+(mXSpacing/2));
		return aItem;
	}

	void						AddSpace(float theAmount) { mSwipeWidth+=theAmount; mAddXPos+=theAmount; }
	void						CenterOnItem(int theID) {EnumSmartList(SwipeListItem,aItem,mItemList) {if (aItem->mID==theID) {SetPos(aItem->mCenter.mX-(mWidth/2)+aItem->mSprite->HalfWidthF(),0);break;}}}


public:

	float				mFirstScrollAnchor;
	bool				mCanSelect;

	float				mAddXPos;
	float				mXSpacing;
	float				mYSpacing;
	float				mYOffset;

	Smart(SwipeListItem)	mHoverItem;

	Smart(SwipeListItem)	mPickedItem;
	Smart(SwipeListItem)	mPreviousPickedItem;
	void					Pick(int x, int y) {mPreviousPickedItem=mPickedItem; mPickedItem=NULL; mPickedItem=GetItem(x,y);}
	Smart(SwipeListItem)	GetItem(int x, int y, bool includeSpacing=false)
	{
		Point aPos=ConvertVisibleToWorld(x,y);
		EnumSmartList(SwipeListItem,aItem,mItemList)
		{
			Rect aRect=Rect(aItem->mCenter.mX-(aItem->mSize.mX/2),aItem->mCenter.mY-(aItem->mSize.mY/2),(float)aItem->mSize.mX,(float)aItem->mSize.mY);
			if (includeSpacing)
			{
				aRect.mX-=mXSpacing/2;
				aRect.mWidth+=mXSpacing;
				aRect.mY-=mYSpacing/2;
				aRect.mHeight+=mYSpacing;
			}
			if (aRect.ContainsPoint(aPos.mX,aPos.mY))
			{
				return aItem;
				break;
			}
		}

		return NULL;
	}
	int						GetSelected() { if (mPickedItem) return mPickedItem->mID; else return -1; }
	int						GetItemNumber(int x, int y, bool includeSpacing=false)
	{
		Point aPos=ConvertVisibleToWorld(x,y);
		int aCounter=0;
		EnumSmartList(SwipeListItem,aItem,mItemList)
		{
			Rect aRect=Rect(aItem->mCenter.mX-(aItem->mSize.mX/2),aItem->mCenter.mY-(aItem->mSize.mY/2),(float)aItem->mSize.mX,(float)aItem->mSize.mY);
			if (includeSpacing) {aRect.mX-=mXSpacing/2;aRect.mWidth+=mXSpacing;aRect.mY-=mYSpacing/2;aRect.mHeight+=mYSpacing;}
			if (aRect.ContainsPoint(aPos.mX,aPos.mY)) return aCounter;
			aCounter++;
		}
		return -1;
	}

	int					mRows;
	int					mRowCounter;
	inline void			SetRows(int theRows) {mRows=theRows;}

	//
	// A way to get all our swipeable items
	//
	int						GetItemCount() {return mItemList.GetCount();}
	Smart(SwipeListItem)	GetItem(int theNumber) {return mItemList[theNumber];}
	void					SetItemData(int theNumber, var_type theData) {if (mItemList[theNumber]) mItemList[theNumber]->mExtraData=theData;}

	var_type 				GetItemData(int theNumber) {if (mItemList[theNumber]) return mItemList[theNumber]->mExtraData; else return NULL;}
	Point 					GetItemPos(int theNumber) {if (mItemList[theNumber]) return mItemList[theNumber]->mCenter; else return Point(0,0);}

	void					ShuffleItems(Random& theRand=gRand)
	{
		EnumSmartList(SwipeListItem,aItem,mItemList)
		{
			Smart(SwipeListItem) aOtherItem;
			aOtherItem=mItemList[theRand.Get(mItemList.GetCount())];

			Point aHold=aOtherItem->mCenter;
			aOtherItem->mCenter=aItem->mCenter;
			aItem->mCenter=aHold;
		}
	}

};

//typedef TSwipeList<void*> SwipeList;

//
// TextBox is a simple place to enter text. 
// It will automatically pop up the keyboard on a touch device, and
// move the screen to ensure the box is visible, if it's behind the keyboard.
// TextBox is only capable of handling one line.  For multi-line, use
// TextBoxEX.
//
class TextBox : public CPU
{
public:
	TextBox();
	virtual ~TextBox();
	inline TextBox*		SetID(int theID) {mID=theID;return this;}


	void				Initialize();
	void				Update();
	void				Draw();
	void				Char(int theKey);
	void				KeyDown(int theKey);
	void				GotKeyboardFocus();
	void				LostKeyboardFocus();
	void				LostUserFocus();
	inline TextBox*		PointAtString(String* theString) {mText=theString;return this;}
	inline TextBox*		PointAt(String* theString) {mText=theString;return this;}
	void				ChangedText();

	char*				WhoAreYou() {return "TEXTBOX";}


	void				TouchStart(int x, int y);

	inline TextBox*		SetFont(Font *theFont) {mFont=theFont;return this;}
	inline TextBox*		SetTextColor(Color theColor) {mTextColor=theColor;return this;}
	inline TextBox*		SetBackgroundColor(Color theColor) {mBackgroundColor=theColor;return this;}
	inline TextBox*		SetBackgroundSprite(Sprite *theSprite) {mBackgroundColor=Color(1);mBackgroundSprite=theSprite;return this;} // Uses DrawTelescoped
	inline TextBox*		SetBackgroundMagicSprite(Sprite *theSprite) {mBackgroundColor=Color(1);mBackgroundMagicSprite=theSprite;return this;} // Uses DrawMagicBorder
	inline TextBox*		SetBorder(int theBorder) {mBorder=IPoint(theBorder,theBorder);return this;}
	inline TextBox*		SetBorder(int theBorderX, int theBorderY) {mBorder=IPoint(theBorderX,theBorderY);return this;}
	inline TextBox*		SetStroke(int theStroke) {mStroke=IPoint(theStroke,theStroke);return this;}
	inline TextBox*		SetStroke(int theStrokeX, int theStrokeY) {mStroke=IPoint(theStrokeX,theStrokeY);return this;}
	inline TextBox*		SetBevel(float theBevel) {mBevel=theBevel;return this;}
	inline TextBox*		SetTextOffset(Point theOffset) {mTextOffset=theOffset;return this;}
	inline TextBox*		SetTextOffset(float theX, float theY) {mTextOffset=Point(theX,theY);return this;}
	inline TextBox*		SetCursorPos(int thePos) {mCursor=_min(_max(0,thePos),mText->Len());return this;};
	inline TextBox*		SetCursorWidth(float theWidth) {mCursorWidth=theWidth;return this;}
	inline TextBox*		SetCursorOffset(Point theOffset) {mCursorOffset=theOffset;return this;}
	inline TextBox*		SetCursorTweak(Point theTweak) {mCursorTweak=theTweak;return this;}
	inline TextBox*		SetCursorTweak(float x, float y) {mCursorTweak.mX=x;mCursorTweak.mY=y;return this;}

	Point				GetDrawTextPos();
	Rect				GetDrawCursorRect();

	inline TextBox*		Sharpen(bool theState) {mSharpen=theState;return this;}
	inline TextBox*		IsPassword(bool theState) {mIsPassword=theState;return this;}
	inline TextBox*		HasX(bool theState=true) {mHasX=theState;return this;}

public:
	Font*				mFont;
	Sprite*				mBackgroundSprite;
	Sprite*				mBackgroundMagicSprite;
	Color				mBackgroundColor;
	Color				mTextColor;
	IPoint				mBorder;
	float				mBevel=0;
	Point				mStroke=0;
	Point				mTextOffset;
	Point				mCursorTweak; // Calls "Expand" on the Cursor Rect with this tweak.
	String				mGhostText;	// Dim text that appears if no text is around
	String				mSuffix;	// Suffix text (put in so I could put "seconds" or something after it)
	Font*				mSuffixFont;

	inline TextBox*		SetGhostText(String theText) {mGhostText=theText;return this;}
	inline TextBox*		SetShadowText(String theText) {return SetGhostText(theText);}
	inline TextBox*		SetSuffix(String theText, Font* theFont=NULL) {mSuffix=theText;mSuffixFont=theFont;return this;}

	void				AdjustTextArea(); // Adjusts for suffix, etc

	bool				mIsChanged;		// If the textbox was changed...
	inline bool			IsChanged() {return mIsChanged;}

	inline void			Copy(TextBox* theSource)
	{
		CPU::Copy(theSource);
		COPY(mFont);
		COPY(mBackgroundSprite);
		COPY(mBackgroundMagicSprite);
		COPY(mBackgroundColor);
		COPY(mTextColor);
		COPY(mBorder);
		COPY(mBevel);
		COPY(mStroke);
		COPY(mTextOffset);
		COPY(mGhostText);
		COPY(mCursorWidth);
		COPY(mCursorOffset);
		COPY(mSharpen);
		COPY(mScroll);
		COPY(mIsPassword);
		COPY(mHasX);
		COPY(mFilterInput);
		COPY(mAllowString);
		COPY(mForbidString);
		COPY(mForceUppercase);
		COPY(mJustify);
		COPY(mMaxTextWidth);
		COPY(mMaxCharCount);
		COPY(mClickClear);
		COPY(mXSprite);
		COPY(mMultiLine);
		COPY(mAllowEnter);
		COPY(mScrollY);
		COPY(mExe);
		COPY(mCursorTweak);
	}

	//
	// Last cursor pos before a keypress of any kind... this can be used to see if the most recent keypress was used by the
	// textbox, and thus should not be used for anything else.
	//
	inline bool			WasLastKeypressUsed() {return mCursor!=mOldCursor;}

//	inline bool			WasLastKeypressUsed() {return mCursor!=mLastCursorPos;}
//	int					mLastCursorPos;

	Ticker*				mCursorBlink;
	int					mCursor;
	int					mOldCursor;
	float				mCursorWidth;
	Point				mCursorOffset;

	bool				mSharpen;

	float				mScroll;

	bool				mIsPassword;	// Whether to hide characters
	bool				mHasX;			// Whether it has the little X to clear the field.

	String				mRealText;
	String				mDisplayText;	// For multiline...
	String*				mText;
	inline String		GetText() {return *mText;}
	inline TextBox*		SetText(String theText) {*mText=theText;mCursor=theText.Len();ChangedText();return this;}
	inline TextBox*		SetTextPtr(String* theTextPtr) 
	{
		mText=theTextPtr;
		mCursor=0;
		ChangedText();
		GuaranteeCursorVisible(true);
		mCursor=mText->Len();

		return this;
	}

	short				mFilterInput;
	String				mAllowString;
	String				mForbidString;
	inline TextBox*		AllowCharacters(String theAllow) {mFilterInput|=1;mAllowString=theAllow;return this;}
	inline TextBox*		ForbidCharacters(String theForbid) {mFilterInput|=2;mForbidString=theForbid;return this;}

	inline TextBox*		Numeric() {AllowCharacters("0123456789.-");return this;}
	inline TextBox*		Filename() {ForbidCharacters("/*.\"\\[]:;|,<>?");return this;}

	bool				mForceUppercase;
	inline TextBox*		ForceUppercase() {mForceUppercase=true;return this;}

	char				mJustify=-1;
	inline TextBox*		CenterText() {mJustify=0;return this;}
	inline TextBox*		LeftJustify() {mJustify=-1;return this;}
	inline TextBox*		RightJustify() {mJustify=1;return this;}
	inline TextBox*		CenterJustify() {mJustify=0;return this;}

	float				mMaxTextWidth;
	inline TextBox*		SetMaxTextWidth(float theWidth) {mMaxTextWidth=theWidth;return this;}

	int					mMaxCharCount;
	inline TextBox*		SetMaxCharCount(int theCount) {mMaxCharCount=theCount;return this;}

	bool				mClickClear;
	inline TextBox*		ClickClear() {mClickClear=true;return this;}

	//
	// For overrides, if you need them.
	// This just gets called at the same time a notify would go out.
	//
	virtual void		SaveChanges() {}

	//
	// Button for the little X
	//
	Button				mXButton;
	Sprite*				mXSprite;
	void				Notify(void* theData);

	inline TextBox*		RemoveX() {mHasX=false;*this-=&mXButton;mXButton.Size(-1,-1,-1,-1);return this;}

	//
	// Multiline works differently...
	// It caps the width, but allows vertical scroll.
	//
	bool				mMultiLine;
	inline TextBox*		SetMultiLine() {mMultiLine=true;mHasX=false;mAllowEnter=true;return this;}
	int					PickMLCursor(int x, int y);
	void				MoveMLCursor(int theYDir);
	void				GuaranteeCursorVisible(bool doFast=false); // doFast means instantaneously-- does a while loop.  When false it'll do it smoothly for effect.
	bool				mAllowEnter;
	float				mScrollY=0;

	void				MLDraw();

	inline TextBox*		Exe(HOOKARGPTR theHook=NULL) {mExe=theHook;return this;} 
	HOOKARGPTR			mExe=NULL;
	inline void			SetExe(HOOKARGPTR theHook=NULL) {mExe=theHook;} 

	//
	// Total Conversions... turns textbox into a sort of widgety thing
	//
	enum
	{
		TC_NUMBERF=1,
		TC_UPDATES,

		TC_STARTUP=0x8000
	};
	struct TotalConversion
	{
		TotalConversion(int theID, void* thePtr, String theFormat="")
		{
			mID=theID|TC_STARTUP;
			mPtr=thePtr;
			mFormat=theFormat;
		}
		unsigned short		mID=0;
		void*				mPtr=NULL;

		String				mFormat;
		Array<Button*>		mButtonList; // Any additional buttons we might need for this total conversion
		Array<Sound*>		mSoundList;
		Array<Sprite*>		mSpriteList;

		union
		{
			Vector			mLowHiStep;
		};

	};
	Smart(TotalConversion)	mTotalConversion;

	void				InitializeTC();
	void				LostKeyboardFocusTC();
	void				DrawTC();
	void				NotifyTC(void* theData);
	void				TCSetText();

	inline TextBox*		TotalConversion_NumberF(float* theHook, float theMin, float theMax, float theStep=.1f, String theFormat="%.01f") {mTotalConversion=new TotalConversion(TC_NUMBERF,theHook,theFormat);mTotalConversion->mLowHiStep=Vector(theMin,theMax,theStep);return this;}
	TextBox*			TotalConversion_Updates(int* theHook, float theMin, float theMax, float theStep=.25f) {mTotalConversion=new TotalConversion(TC_UPDATES,theHook,"%.02f");mTotalConversion->mLowHiStep=Vector(theMin,theMax,theStep);return this;}

	inline TextBox*		TotalConversion_SetSounds(Array<Sound*> theSounds) {if (mTotalConversion.IsNotNull()) mTotalConversion->mSoundList=theSounds;return this;}
	inline TextBox*		TotalConversion_SetSprites(Array<Sprite*> theSprites) {if (mTotalConversion.IsNotNull()) mTotalConversion->mSpriteList=theSprites;return this;}

};


//
// KeyGrabber is a fullscreen CPU that halts the entire system
// until you either click it, or press a key.  Just call Go, and
// it'll return the key (or -1 if the user clicked to cancel)
//
class KeyGrabber : public CPU
{
public:
	KeyGrabber();
	virtual ~KeyGrabber();
	inline KeyGrabber*					SetID(int theID) {mID=theID;return this;}


	void								Go(CPUHOOKPTR theHook);
	void								Update();
	void								TouchStart(int x, int y);

	bool								mWaitForClear;

};

class JoyButtonGrabber : public CPU
{
public:
	JoyButtonGrabber();
	virtual ~JoyButtonGrabber();
	inline JoyButtonGrabber*			SetID(int theID) {mID=theID;return this;}

	void								Go(CPUHOOKPTR theHook);
	void								Update();
	void								TouchStart(int x, int y);

	bool								mWaitForClear;
	int									mResult;
};


/////////////////////////////////////////////////////////////////////////////////
//
// New ControlPanel, to replace old one, which sucks!
//
/////////////////////////////////////////////////////////////////////////////////
class CPanelRollout;
class CPanelControl;
class CPanelControl_Rollout;
class CPanelControl_Checkbox;
class CPanelControl_Slider;
class CPanelControl_Radiobutton;
class CPanelControl_Button;
class CPanelControl_Textbox;
class CPanelControl_Keygrabber;
class CPanelControl_Listbox;
//
// To make a custom control, derive from CPanelControl, and use AddControl to add it.
// Scroll down to CPanelControl's definition for additional notes.
//

class CPanelGroup
{
public:
	CPanelGroup() {mHiliteRect=Rect(-1,-1,-1,-1);mCanClose=false;mIsOpen=true;mExpand.NotifyOnPress();}
	virtual ~CPanelGroup() {_FreeList(AlignStruct,mAlignList);}

public:
	CPanelRollout*					mMyRollout;

	//
	// Name of this group
	//
	inline CPanelGroup*				SetName(String theName) {mName=theName;return this;}

	inline String					GetName() {return mName;}
	String							mName;

	//
	// Rect of this group
	//
	Rect							mRect;

	//
	// If group can be closed/opened, and current state
	//
	bool							mCanClose;
	bool							mIsOpen;
	inline bool						CanClose() {return mCanClose;}
	inline bool						IsOpen() {return mIsOpen;}
	inline CPanelGroup*				SetCanClose(bool theState=true) {mCanClose=theState;return this;}
	inline CPanelGroup*				SetIsOpen(bool theState=true) {mIsOpen=theState;return this;}

	//
	// A rect we can draw hilited, if we want to...
	//
	Rect							mHiliteRect;

	//
	// If we don't draw backing for this group
	//
	bool							mShowGroupBackground;
	inline CPanelGroup*				SetShowBackground(bool theState=true) {mShowGroupBackground=theState;return this;}

	//
	// List of controls in this group
	//
	CPanelControl*					AddControl(CPanelControl* theControl);
	List							mControlList;
	void							GetControls(int theID, List& theList);
	CPanelControl*					FindControl(String theName);
	CPanelControl*					FindControl(int theID);

	void							ClearControls();

	CPanelControl_Checkbox*			AddCheckbox(String theName, bool theState=false);
	CPanelControl*					AddStatic(String theName);
	CPanelControl_Rollout*			AddRollout(String theName, String theRolloutText="", CPanelRollout* theRolloutPanel=NULL);
	CPanelControl_Slider*			AddSlider(String theName, float theValue=0.0f);
	CPanelControl_Radiobutton*		AddRadiobutton(String theName, bool theState=false);
	CPanelControl_Button*			AddButton(String theName);
	CPanelControl_Button*			AddButton(String theName, Sprite* theSprite);
	CPanelControl_Textbox*			AddTextbox(String theName, String theText="", String allowCharacters="");
	CPanelControl_Keygrabber*		AddKeyGrabber(String theName, int theKey=0);
	CPanelControl_Listbox*			AddListbox(String theName, String theElements, int theDefault=0, char theSeperator='|');
	CPanelControl_Listbox*			AddListbox(Sprite& theSprite, Array<Sprite>& theElements, int theDefault=0);

	Button							mExpand;


	struct AlignStruct
	{
		Array<CPanelControl*>		mControlList;
	};
	List							mAlignList;

	void							Align(String theControls);	// Format: ControlName|ControlName|ControlName
};

//
// CPanel is the control panel itself, and acts as a liason to all the other things.
//
#define CPANELHOOK(func) [&](CPanel* panel, CPanelControl* control) func
#define CPANELHOOKPTR std::function<void(CPanel* panel, CPanelControl* control)>

class CPanel : public CPU
{
public:
	CPanel();
	virtual ~CPanel();

	//
	// Override these with local info...
	//
	virtual void			Initialize();

	//
	// Things like default text box, default slider, etc, should be set in SetDefaults.
	// This makes sure that if we, say, add a textbox, the default is there to be referred to.
	//
	virtual void			SetDefaults() {} // This will get called before add anything, to make sure that stuff like our default fonts are working, etc.
	bool					mDidSetDefaults=false;
	inline void				AreDefaultsSet() {if (!mDidSetDefaults) {mDidSetDefaults=true;SetDefaults();}}

	//virtual void			SetupControls() {} // Depreciated... setup controls in mInitializeHook
	virtual void			ControlChanged(CPanelControl* theControl) {}
	virtual void			ApplyChanges() {}
	virtual void			PressedDoneButton() {}
	virtual void			SwitchToRollout(CPanelRollout* theRollout, CPanelRollout *theLastRollout) {}
	virtual void			RepositionButton(Button* theButton, int theButtonType) {}
	virtual void			ExpandedGroup(CPanelGroup* theGroup) {}

	static bool				mForceNoTouching; // If true, it will temporarily set DoControlChanged to have the mouse be "not touching"
	void					DoControlChanged(CPanelControl* theControl);

	//
	// Although these can be overridden with a new class, overrides are usually game-wide, and
	// these hooks are used for individual cpanels.
	//
	CPANELHOOKPTR			mInitializeHook=NULL;
	CPANELHOOKPTR			mControlChangedHook=NULL;
	CPANELHOOKPTR			mApplyChangesHook=NULL;
	CPANELHOOKPTR			mPressedDoneButtonHook=NULL;
	inline CPanel*			SetInitializeHook(CPANELHOOKPTR theHook) {mInitializeHook=theHook;return this;}
	inline CPanel*			SetControlChangedHook(CPANELHOOKPTR theHook) {mControlChangedHook=theHook;return this;}
	inline CPanel*			SetApplyChangesHook(CPANELHOOKPTR theHook) {mApplyChangesHook=theHook;return this;}
	inline CPanel*			SetPressedDoneButtonHook(CPANELHOOKPTR theHook) {mPressedDoneButtonHook=theHook;return this;}

	bool					mApplyChangesOnClose=false;
	inline CPanel*			ApplyChangesOnClose(bool theState=true) {mApplyChangesOnClose=theState;return this;}

	bool					mShrinkWrap=false;
	inline CPanel*			ShrinkWrap(bool theState=true) {mShrinkWrap=theState;return this;}

	//
	// Note: Draggable will not work with scrollable, because it makes the Rollout the tool is on pass touches up to the parent.
	// 	     As of 9/22/23 Draggable should only be used with teeny tiny little toolboxes and not whole interfaces that scroll.
	//
	bool					mDraggable=false;
	HOOKARGPTR				mDragHook=NULL;
	inline CPanel*			Draggable(bool theState=true) {mDraggable=theState;return this;}
	inline CPanel*			Draggable(HOOKARGPTR theHook) {mDraggable=true;mDragHook=theHook;return this;}


	bool					mIsDown=false;
	char					mDragState=0; // 1 = startdrag, 0 = dragging, -1 = enddrag
	inline bool				IsFirstDrag() {return (mDragState==1);}
	inline bool				IsLastDrag() {return (mDragState==-1);}
	inline bool				IsDragging() {return (mDragState==0);}

	virtual void			TouchStart(int x, int y) {if (mDraggable) {mIsDown=true;if (mDragHook) {mDragState=1;mDragHook(this);}}}
	virtual void			TouchMove(int x, int y);
	virtual void			TouchEnd(int x, int y) {mIsDown=false;if (mDragHook) {mDragState=-1;mDragHook(this);}}


	void					Kill();


	//
	// Called if we buttonize a control.  Typically fleshed out to add sound
	// to the button, or tweak it in some way.
	//
	virtual void			ButtonizedControl(CPanelControl* theControl, Button *theButton) {}

	//
	// Draw overrides...
	//
	virtual void			DrawPanelName(String theName) {}															// Requests Draw of the name of current panel, up on caption bar
	virtual void			DrawPanelButton(int theType, Button* theButton) {}											// Requests Draw of a done or back button
	virtual void			DrawGroupBox(String theName, Rect theRect, Rect theHiliteRect, CPanelGroup* theGroup) {}	// Requests Draw of a group box
	virtual void			DrawControl_Border(Rect theRect) {}															// Requests Draw of a thin border between controls
	virtual void			DrawControl_Static(CPanelControl* theControl) {}											// Requests Draw of a static text control
	virtual void			DrawControl_Rollout(CPanelControl_Rollout* theControl) {}									// Requests Draw of a rollout summoner
	virtual void			DrawControl_Checkbox(CPanelControl_Checkbox* theControl) {}			// Requests Draw of a checkbox
	virtual void			DrawControl_Slider(CPanelControl_Slider* theControl) {}				// Requests Draw of a slider
	virtual void			DrawControl_Radiobutton(CPanelControl_Radiobutton* theControl) {}	// Requests Draw of a radiobutton
	virtual void			DrawControl_Button(CPanelControl_Button* theControl) {}				// Requests Draw of a regular button
	virtual void			DrawControl_Textbox(CPanelControl_Textbox* theControl) {}			// Requests Draw of a textbox
	virtual void			DrawControl_Keygrabber(CPanelControl_Keygrabber* theControl) {}		// Requests Draw of a key grabber
	virtual void			DrawControl_Listbox(CPanelControl_Listbox* theControl) {DrawControl_Rollout((CPanelControl_Rollout*)theControl);}			// Requests Draw of a List Box (Ususally not overridden since it can just be a rollout)

	//
	// Should CALL these...
	//
	inline void				SetControlSpacing(float theSpacing) {mControlSpacing=theSpacing;}
	inline void				SetGroupSpacing(float theSpacing) {mGroupSpacing=theSpacing;}
	inline void				SetGroupVBorders(float theBorder) {mGroupVBorders=theBorder;}
	inline void				SetPanelBorder(float theBorder) {mPanelBorder=theBorder;}
	inline void				SetFont(Font* theFont) {mFont=theFont;if (!mDefaultTextBox.mFont) mDefaultTextBox.mFont=theFont;}
	inline Font*			GetFont() {AreDefaultsSet();return mFont;}

	//
	// Only override these if you've ovberridden CPanelRollout...
	//
	virtual CPanelRollout*	CreateRollout(String theName="");
	virtual CPanelRollout*	GetMainRollout();

	//
	// These let you get data in and out...
	//
	bool					mSyncState;	// 0 = loading, 1 = saving
	inline void				Loading() {mSyncState=false;}
	inline void				Saving() {mSyncState=true;}

	void					SetControlValue(String theName, SmartVariable theValue);
	SmartVariable			GetControlValue(String theName);
	inline CPanelControl*	GetControl(String theName) {return FindControl(theName);}
	inline CPanelControl*	GetControl(int theID) {return FindControl(theID);}

	void					Sync(String theName, bool& theVariable);
	void					Sync(String theName, char& theVariable);
	void					Sync(String theName, float& theVariable);
	void					Sync(String theName, int& theVariable);
	void					Sync(String theName, String& theVariable);

	Color					mDefaultTextColor=Color(0,0,0,0);
	inline void				SetDefaultTextColor(Color theColor) {mDefaultTextColor=theColor;}


	//
	// Less likely to override
	//
	void					Core_Update();
	void					Notify(void* theData);

	void					Core_Draw();

	inline void				SetWorkAreaRect(Rect theRect) {mWorkAreaRect=theRect;}
	inline void				SetWorkAreaRect(float theX, float theY, float theWidth, float theHeight) {SetWorkAreaRect(Rect(theX,theY,theWidth,theHeight));}
	void					SetName(String theName);

	virtual void			GoToNextPanel(CPanelRollout* thePanel);
	virtual void			GoToPreviousPanel();

public:
	List							mCleanupRolloutList;	// Panels that were created, and need to be cleaned...

	Stack<CPanelRollout*>			mWorkAreaStack;
	Rect							mWorkAreaRect;

	Button							mButton;	// This button functions as both the done button and the back button.

	String							mName;		// Name of CPanel (and Panel #1)

	//
	// Some spacers...
	//
	float							mDefaultControlHeight;	// Default height of a new control
	float							mControlSpacing;	// Spacing between controls
	float							mGroupSpacing;		// Spacing between groups
	float							mGroupVBorders;		// Extra white space at tops and bottoms of group boxes, to make it look better
	float							mPanelBorder;		// Border space at edge of panels (when rehupping)


	//
	// Fonts we'll be using... this is just here to prevent
	// certain overrides that would be necessary, like sizing a button.
	//
	Font*							mFont;

	//
	// For scrolling panels in and out...
	//
	float							mScrollDir;
	float							mScrollCount;

	bool							mInstantScroll;
	inline void						InstantScroll() {mInstantScroll=true;}

	CPanelRollout*					mVanishingRollout;
	CPanelRollout*					mCurrentRollout;

	//
	// A variable so we can access whatever control was recently clicked
	//
	CPanelControl*					mClickedControl;
	inline CPanelControl*			GetClickedControl() {return mClickedControl;}

public:
	//
	// Helper functions... these let us quickly make menus
	//
	CPanelRollout*					mMainRollout;
	CPanelGroup*					GetCurrentGroup();
	CPanelGroup*					StartGroup(String theName, bool showBackground=true, bool canClose=false, bool initiallyOpen=true);

	CPanelControl_Checkbox*			AddCheckbox(String theName, bool theState=false) {AreDefaultsSet();return GetCurrentGroup()->AddCheckbox(theName,theState);}
	CPanelControl*					AddStatic(String theName) {AreDefaultsSet();return GetCurrentGroup()->AddStatic(theName);}
	CPanelControl_Rollout*			AddRollout(String theName, String theRolloutText="", CPanelRollout* theRolloutPanel=NULL) {AreDefaultsSet();return GetCurrentGroup()->AddRollout(theName,theRolloutText,theRolloutPanel);}
	CPanelControl_Slider*			AddSlider(String theName, float theValue=0.0f) {AreDefaultsSet();return GetCurrentGroup()->AddSlider(theName,theValue);}
	CPanelControl_Radiobutton*		AddRadiobutton(String theName, bool theState=false) {AreDefaultsSet();return GetCurrentGroup()->AddRadiobutton(theName,theState);}
	CPanelControl_Button*			AddButton(String theName) {AreDefaultsSet();return GetCurrentGroup()->AddButton(theName);}
	CPanelControl_Button*			AddButton(String theName, Sprite* theSprite) {AreDefaultsSet();return GetCurrentGroup()->AddButton(theName,theSprite);}
	CPanelControl_Textbox*			AddTextbox(String theName, String theText="",  String allowCharacters="") {AreDefaultsSet();return GetCurrentGroup()->AddTextbox(theName, theText, allowCharacters);}
	CPanelControl_Keygrabber*		AddKeyGrabber(String theName, int theKey=0) {AreDefaultsSet();return GetCurrentGroup()->AddKeyGrabber(theName, theKey);}
	CPanelControl_Listbox*			AddListbox(String theName, String theElements, int theDefault=0, char theSeperator='|') {AreDefaultsSet();return GetCurrentGroup()->AddListbox(theName,theElements,theDefault,theSeperator);}
	CPanelControl_Listbox*			AddListbox(Sprite& theSprite, Array<Sprite>& theElements, int theDefault=0) {AreDefaultsSet();return GetCurrentGroup()->AddListbox(theSprite,theElements,theDefault);}
	CPanelControl*					AddControl(CPanelControl* theControl) {AreDefaultsSet();return GetCurrentGroup()->AddControl(theControl);}

	//
	// Other helpers
	//
	CPanelControl*					FindControl(String theName, char theSeperator='|');	// Note: You can specify "Rollout|ControlName" to get a control on a specific rollout.
	CPanelControl*					FindControl(int theID);	// Note: You can specify "Rollout|ControlName" to get a control on a specific rollout.
	CPanelRollout*					FindRollout(String theName, bool doCreate=true);

	//
	// Default textbox... will default a textbox to this when creating
	//
	TextBox							mDefaultTextBox;
	inline TextBox&					DefaultTextBox() {return mDefaultTextBox;}
	Slider							mDefaultSlider;
	inline Slider&					DefaultSlider() {return mDefaultSlider;}

};

//
// CPanelWorkArea is the place where the actual controls will reside.
// It is seperate from CPanel so that CPanel can maintain non-scrolling stuff.
// Also, if you want to make panels that slide in, you'll make one of these.
//
class CPanelRollout : public SwipeBox
{
public:
	CPanelRollout();
	virtual ~CPanelRollout();

	void							Initialize();
	void							Update();
	void							Draw();
	void							UnfocusKeyboard(); // Just makes sure no children are keyboard focused here...

	//
	// Clears the whole rollout for re-use
	//
	void							Reset();

public:

	CPanel*							mCPanel;

	//
	// Notify for opening/closing groups
	//
	void							Notify(void *theData);

	//
	// Name of this panel
	//
	inline void						SetName(String theName) {mName=theName;}
	inline String					GetName() {return mName;}
	String							mName;

	//
	// Allows us to rehup the panel, if we've changed the contents of a group
	// or something.
	//
	void							Rehup();
	bool							mWantRehup;

	//
	// List of all our groups
	//
	List							mGroupList;
	CPanelGroup*					CreateGroup(String theName="", bool showGroupBackground=true);
	void							BackupGroupState(Array<bool>& theArray);
	void							RestoreGroupState(Array<bool>& theArray);
	CPanelGroup*					mMustSeeGroup;		// A group we must see (most useful for expanding/contracting)

	//
	// List of all our controls
	//
	List							mControlList;

	//
	// Helps us find controls...
	//
	CPanelGroup*					FindGroup(String theName);
	CPanelControl*					FindControl(String theName);
	CPanelControl*					FindControl(int x, int y);
	CPanelControl*					FindControl(int theID);

	//
	// Helpers, redundant
	//
	CPanelGroup*					mCurrentGroup;
	CPanelGroup*					GetCurrentGroup();
	CPanelGroup*					StartGroup(String theName, bool showBackground=true, bool canClose=false, bool initiallyOpen=true);

	inline void						AreDefaultsSet() {if (mCPanel) mCPanel->AreDefaultsSet();}

	CPanelControl_Checkbox*			AddCheckbox(String theName, bool theState=false) {AreDefaultsSet();return GetCurrentGroup()->AddCheckbox(theName,theState);}
	CPanelControl*					AddStatic(String theName) {AreDefaultsSet();return GetCurrentGroup()->AddStatic(theName);}
	CPanelControl_Rollout*			AddRollout(String theName, String theRolloutText="", CPanelRollout* theRolloutPanel=NULL) {AreDefaultsSet();return GetCurrentGroup()->AddRollout(theName,theRolloutText,theRolloutPanel);}
	CPanelControl_Slider*			AddSlider(String theName, float theValue=0.0f) {AreDefaultsSet();return GetCurrentGroup()->AddSlider(theName,theValue);}
	CPanelControl_Radiobutton*		AddRadiobutton(String theName, bool theState=false) {AreDefaultsSet();return GetCurrentGroup()->AddRadiobutton(theName,theState);}
	CPanelControl_Button*			AddButton(String theName) {AreDefaultsSet();return GetCurrentGroup()->AddButton(theName);}
	CPanelControl_Button*			AddButton(String theName, Sprite* theSprite) {AreDefaultsSet();return GetCurrentGroup()->AddButton(theName,theSprite);}
	CPanelControl_Textbox*			AddTextbox(String theName, String theText="",  String allowCharacters="") {AreDefaultsSet();return GetCurrentGroup()->AddTextbox(theName, theText,allowCharacters);}
	CPanelControl_Keygrabber*		AddKeyGrabber(String theName, int theKey=0) {AreDefaultsSet();return GetCurrentGroup()->AddKeyGrabber(theName, theKey);}
	CPanelControl_Listbox*			AddListbox(String theName, String theElements, int theDefault=0, char theSeperator='|') {AreDefaultsSet();return GetCurrentGroup()->AddListbox(theName,theElements,theDefault,theSeperator);}
	CPanelControl_Listbox*			AddListbox(Sprite& theSprite, Array<Sprite>& theElements, int theDefault=0) {AreDefaultsSet();return GetCurrentGroup()->AddListbox(theSprite,theElements,theDefault);}

	inline Font*					GetFont() {return mCPanel->GetFont();}
};


enum
{
	CPANELCONTROL_STATIC=0,
	CPANELCONTROL_ROLLOUT,
	CPANELCONTROL_CHECKBOX,
	CPANELCONTROL_SLIDER,
	CPANELCONTROL_RADIOBUTTON,
	CPANELCONTROL_BUTTON,
	CPANELCONTROL_TEXTBOX,
	CPANELCONTROL_KEYGRABBER,
	CPANELCONTROL_LISTBOX,

	CPANELCONTROL_MAX,
	CPANELCONTROL_CUSTOM,
};

//
// CPanelControl is for deriving controls to rest inside the CPanelWorkArea
// REMEMBER: You set mHeight, otherwise it will do the default panel's height when you AddControl(my_control)
//

#define _TWEAK(type) inline type* Tweak(String theCommand) {mTweak=theCommand;return this;} 


class CPanelControl : public CPU
{
public:
	CPanelControl() {mID=CPANELCONTROL_STATIC;mHeight=0;DisableFeature(CPU_TOUCH);EnableFeature(CPU_TOUCHCHILDREN);mTextColor=Color(0,0,0,1);mIsSizeChanged=false;mSprite=NULL;mWasDown=false;mControlButton=NULL;}

	//
	// Rehup is called whenever the control is sized or moved.
	// Override it if you have a control that needs adjustments for size (many do!)
	//
	virtual void					Rehup() {}
	virtual void					Update();
	virtual void					Draw() {mMyRollout->mCPanel->DrawControl_Static(this);}
	virtual float					GetHeight() {return mHeight;}

	virtual void					Notify(void *theData) {mMyRollout->mCPanel->mClickedControl=this;mMyRollout->mCPanel->ControlChanged(this);}


	inline bool						operator==(const String &theString) {return mName==theString;}
	inline bool						operator==(const char* theString) {return mName.strcmpi(theString);}

public:
	CPanelRollout*					mMyRollout;
	CPanelGroup*					mMyGroup;

	virtual CPanelControl*			SetName(String theName) {mName=theName;return this;}
	virtual CPanelControl*			SetSprite(Sprite* theSprite) {mSprite=theSprite;mHeight=theSprite->HeightF();return this;}
	inline String					GetName() {return mName;}
	String							mName;
	Sprite*							mSprite;

	//
	// This is the area the control was allotted... it's not the control panel's SIZE,
	// it's just the screen area that the Panel reserved for it, if needed.
	//
	Rect							mReservedArea;

	//
	// Extra data, so you can attach other things to the control.  HORRIBLE things.
	// Wait, CPU already has this...
	//
	//void							*mExtraData;

	//
	// Special Stuff
	//
	virtual CPanelControl*			SetTextColor(Color theColor) {mTextColor=theColor;return this;}
	Color							mTextColor;

	//
	// If the control has changed, conform to whatever you need before you draw...
	//
	inline bool						IsSizeChanged() {return mIsSizeChanged;}
	inline void						SizeChangeFixed() {mIsSizeChanged=false;ApplyTweak();}
	inline void						SizeChanged() {mIsSizeChanged=true;}
	bool							mIsSizeChanged;

	virtual CPanelControl*			SetValue(SmartVariable theVariable) {return this;}
	virtual SmartVariable			GetValue() {return 0;}

	//
	// Allows you to adjust the size of an internal control after it's set up (i.e. if you want to tweak the size of a textbox or slider)
	// See Rect::ApplySizeCommand for diction
	//
	String							mTweak;
	_TWEAK(CPanelControl);
	virtual void					ApplyTweak() {}
	//
	// Any control can be turned into a button...
	//
	virtual CPanelControl*			Buttonize(bool isCheckbox=false, bool isDown=false);
	inline bool						IsButtonized() {return (mControlButton!=NULL);}
	Button*							mControlButton;
	bool							mWasDown;

	void							ResizeControlButton();

	inline Font*					GetFont() {return mMyRollout->GetFont();}
	inline CPanel*					GetPanel() {return mMyRollout->mCPanel;}


	//
	// Inheritors should override these so they can return the correct object...
	// 	   	DEFAULT_ALL(CPanelControl_[CustomName]);
	//
	//
	/*
	#define MY_TYPE CPanelControl_Rollout
	MY_TYPE* SetName(String theName) {return (MY_TYPE*)CPanelControl::SetName(theName);}
	MY_TYPE* SetSprite(Sprite* theSprite) {return (MY_TYPE*)CPanelControl::SetSprite(theSprite);}
	MY_TYPE* Buttonize(bool isCheckbox=false, bool isDown=false) {return (MY_TYPE*)CPanelControl::Buttonize(isCheckbox,isDown);}
	MY_TYPE* SetValue(SmartVariable theVariable) {return (MY_TYPE*)CPanelControl::SetValue(theVariable);}
	MY_TYPE* SetTextColor(Color theColor) {return (MY_TYPE*)CPanelControl::SetTextColor(theColor);}
	#undef MY_TYPE
	*/

#define DEFAULT_SETNAME(MY_TYPE) MY_TYPE* SetName(String theName) {return (MY_TYPE*)CPanelControl::SetName(theName);}
#define DEFAULT_SETSPRITE(MY_TYPE) MY_TYPE* SetSprite(Sprite* theSprite) {return (MY_TYPE*)CPanelControl::SetSprite(theSprite);}
#define DEFAULT_BUTTONIZE(MY_TYPE) MY_TYPE* Buttonize(bool isCheckbox=false, bool isDown=false) {return (MY_TYPE*)CPanelControl::Buttonize(isCheckbox,isDown);}
#define DEFAULT_SETTEXTCOLOR(MY_TYPE) MY_TYPE* SetTextColor(Color theColor) {return (MY_TYPE*)CPanelControl::SetTextColor(theColor);}
#define DEFAULT_ALL(MY_TYPE) DEFAULT_SETNAME(MY_TYPE);DEFAULT_SETSPRITE(MY_TYPE);DEFAULT_BUTTONIZE(MY_TYPE);DEFAULT_SETTEXTCOLOR(MY_TYPE);

};

class CPanelControl_Rollout : public CPanelControl
{
public:
	CPanelControl_Rollout() {mID=CPANELCONTROL_ROLLOUT;*this+=&mButton;mButton.NotifyOnPress();mButton.SetNotify(this);}

	virtual void					Draw() {if (mControlButton) ResizeControlButton();mMyRollout->mCPanel->DrawControl_Rollout(this);}

public:

	DEFAULT_ALL(CPanelControl_Rollout);

	inline CPanelControl_Rollout*	SetRolloutText(String theText) {mRolloutText=theText;return this;}
	inline String					GetRolloutText() {return mRolloutText;}
	String							mRolloutText;

	inline CPanelControl_Rollout*	SetRollout(CPanelRollout* thePanel) {mRollout=thePanel;return this;}
	CPanelRollout*					mRollout;		// Panel to open

	Button							mButton;		// Panel open button
	Rect							GetButtonRect() {return (Rect)mButton;}

	virtual void					Notify(void *theData);

	_TWEAK(CPanelControl_Rollout);



	/*

		Quick Tutorial on creating a sub-panel...

		1. Create the control that will invoke the panel, and name it:

				CPanelControl_Panel *aCP=new CPanelControl_Panel;
				aCP->SetName("RESOLUTION");
				aCP->SetPanelText("1024x768");

		2. Create a panel through the CPanel class to be the
		   subpanel (must create it through CreatePanel(), don't try to "new" it)

				CPanelRollout* aNewPanel=CreatePanel();
				aCP->SetNextPanel(aNewPanel);

		3. Now work with the panel the same way you would
		   normally:

				CPanelGroup* aGroup2=aNewPanel->CreateGroup("");
				aC=new CPanelControl;
				aC->SetName("STATIC CONTROL PANEL 2");
				aGroup2->AddControl(aC);

	*/

};

class CPanelControl_Checkbox : public CPanelControl
{
public:
	CPanelControl_Checkbox() {mID=CPANELCONTROL_CHECKBOX;*this+=&mButton;mButton.SetNotify(this);}

	virtual void					Draw() {if (mControlButton) ResizeControlButton();mMyRollout->mCPanel->DrawControl_Checkbox(this);}
	DEFAULT_ALL(CPanelControl_Checkbox);

public:

	//
	// State of checkbox
	//
	CPanelControl_Checkbox*			SetValue(SmartVariable theValue) {mButton.SetState(theValue);return this;}
	inline CPanelControl_Checkbox*	PointAt(bool* thePtr) {mButton.PointAt(thePtr);SetValue(*thePtr);return this;}
	SmartVariable					GetValue() {return mButton.GetState();}

	CheckBox						mButton;		// Checkbox button
	Rect							GetButtonRect() {return (Rect)mButton;}
	inline CheckBox*				GetButton() {return &mButton;}
	inline CheckBox*				GetCheckBox() {return &mButton;}

	void							ApplyTweak() {mButton.Tweak(mTweak);}
	_TWEAK(CPanelControl_Checkbox);

};

class CPanelControl_Slider : public CPanelControl
{
public:
	CPanelControl_Slider() {mID=CPANELCONTROL_SLIDER;*this+=&mSlider;mSlider.SetNotify(this);}
	DEFAULT_ALL(CPanelControl_Slider);

	virtual void					Draw() {if (mControlButton) ResizeControlButton();mMyRollout->mCPanel->DrawControl_Slider(this);}

public:

	CPanelControl_Slider*			SetValue(SmartVariable theValue) {mSlider.SetValue(theValue);return this;}
	SmartVariable					GetValue() {return mSlider.GetValue();}

	Slider							mSlider;		// Slider
	Rect							GetSliderRect() {return (Rect)mSlider;}
	Slider*							GetSlider() {return &mSlider;}

	void							ApplyTweak() {mSlider.Tweak(mTweak);}
	_TWEAK(CPanelControl_Slider);

};

class CPanelControl_Radiobutton : public CPanelControl
{
public:
	CPanelControl_Radiobutton() {mID=CPANELCONTROL_RADIOBUTTON;*this+=&mButton;mButton.SetNotify(this);}
	DEFAULT_ALL(CPanelControl_Radiobutton);

	virtual void					Draw() {if (mControlButton) ResizeControlButton();mMyRollout->mCPanel->DrawControl_Radiobutton(this);}
	virtual void					Notify(void *theData);

	CPanelControl_Radiobutton*		SetValue(SmartVariable theValue) {mButton.SetState(theValue);return this;}
	SmartVariable					GetValue() {return mButton.GetState();}

public:
	CheckBox						mButton;
	Rect							GetButtonRect() {return (Rect)mButton;}
	inline CheckBox*				GetButton() {return &mButton;}
	inline CheckBox*				GetRadioButton() {return &mButton;}
	inline CheckBox*				GetCheckBox() {return &mButton;}

	void							ApplyTweak() {mButton.Tweak(mTweak);}
	_TWEAK(CPanelControl_Radiobutton);

};

class CPanelControl_Button : public CPanelControl
{
public:
//	CPanelControl_Button() {mID=CPANELCONTROL_BUTTON;*this+=&mButton;mButton.SetNotify(this);mButton.NotifyOnRelease();mWasDown=false;}
	CPanelControl_Button() {mID=CPANELCONTROL_BUTTON;mWasDown=false;}
	DEFAULT_ALL(CPanelControl_Button);

	//void							Update();
	virtual void					Draw() {if (mControlButton) ResizeControlButton();mMyRollout->mCPanel->DrawControl_Button(this);}

public:

	inline CPanelControl_Button*	SetExe(HOOKARGPTR theHook) {mControlButton->SetExe(theHook);return this;}

	//bool							mWasDown;
	//Button						mButton;
	Rect							GetButtonRect() {return (Rect)*mControlButton;}

	void							ApplyTweak() {mControlButton->Tweak(mTweak);}
	_TWEAK(CPanelControl_Button);

};

class CPanelControl_Textbox : public CPanelControl
{
public:
	CPanelControl_Textbox() {mID=CPANELCONTROL_TEXTBOX;*this+=&mTextBox;mTextBox.SetNotify(this);}
	DEFAULT_ALL(CPanelControl_Textbox);

	virtual void					Draw() {if (mControlButton) ResizeControlButton();mMyRollout->mCPanel->DrawControl_Textbox(this);}

public:
	TextBox							mTextBox;
	inline TextBox*					GetTextBox() {return &mTextBox;}
	Rect							GetButtonRect() {return (Rect)mTextBox;}

	CPanelControl_Textbox*			SetValue(SmartVariable theValue) {mTextBox.SetText(theValue);return this;}
	SmartVariable					GetValue() {return mTextBox.GetText();}

	void							ApplyTweak() {mTextBox.Tweak(mTweak);}
	_TWEAK(CPanelControl_Textbox);
};

class CPanelControl_Keygrabber : public CPanelControl
{
public:
	CPanelControl_Keygrabber() {mID=CPANELCONTROL_KEYGRABBER;*this+=&mButton;mButton.SetNotify(this);mButton.NotifyOnPress();mWaitingForKey=false;mValue=KB_SPACE;}
	DEFAULT_ALL(CPanelControl_Keygrabber);

	virtual void					Draw() {if (mControlButton) ResizeControlButton();mMyRollout->mCPanel->DrawControl_Keygrabber(this);}
	virtual void					Notify(void *theData);

public:
	Button							mButton;
	Rect							GetButtonRect() {return (Rect)mButton;}

	int								mValue;
	String							mTextValue;

	CPanelControl_Keygrabber*		SetValue(SmartVariable theValue);
	SmartVariable					GetValue() {return mValue;}
	inline String					GetTextValue() {return mTextValue;}
	bool							mWaitingForKey;

	void							ApplyTweak() {mButton.Tweak(mTweak);}
	_TWEAK(CPanelControl_Keygrabber);

};

class CPanelControl_Listbox : public CPanelControl_Rollout
{
public:
	CPanelControl_Listbox() {mID=CPANELCONTROL_LISTBOX;*this+=&mButton;mButton.NotifyOnPress();mButton.SetNotify(this);mIsSpriteList=false;}
	DEFAULT_ALL(CPanelControl_Listbox);
	virtual void					Draw() {if (mControlButton) ResizeControlButton();mMyRollout->mCPanel->DrawControl_Rollout(this);}
	virtual void					Notify(void* theData);

public:
	CPanelControl_Listbox*			SetValue(SmartVariable theValue);
	SmartVariable					GetValue();

	XInt							mValue;
	inline CPanelControl_Listbox*	PointAt(int* thePtr) {mValue.PointAt(thePtr);SetValue(*thePtr);return this;}

	List							mChildButtonList;

	bool							mIsSpriteList;

	_TWEAK(CPanelControl_Listbox);

};

class TextGroup : public Rect
{
public:
	TextGroup() {mWidth=320;mHeight=0;mNeedRehup=false;}
	void							Add(Font& theFont, String theString, Color theColor=Color(1));
	void							AddSpace(float theSpace);
	void							Rehup();

public:
	bool							mNeedRehup;
	struct TG
	{
		Font*						mFont;
		String						mString;
		Color						mColor;
		Rect						mRect;
	};
	SmartList(TG)					mTGList;

	inline Rect&					GetSize()
	{
		if (mNeedRehup) Rehup();
		return *this;
	}

	inline int						GetTextCount() {if (mNeedRehup) Rehup();return mTGList.GetCount();}
	inline String					GetText(int theIndex) {if (mNeedRehup) Rehup();return mTGList[theIndex]->mString;}
	inline Color					GetTextColor(int theIndex) {if (mNeedRehup) Rehup();return mTGList[theIndex]->mColor;}
	inline Font*					GetTextFont(int theIndex) {if (mNeedRehup) Rehup();return mTGList[theIndex]->mFont;}
	inline Rect&					GetTextSize(int theIndex) {if (mNeedRehup) Rehup();return mTGList[theIndex]->mRect;}

};

class DelayedMouseTouch : public Object
{
public:
	DelayedMouseTouch(Point thePos, int theDelay);
	virtual ~DelayedMouseTouch();
	void						Update();

public:
	Point						mPos;
	int							mCountdown;

};

#define LOGMAX 150

class DevConsole : public CPU
{
public:
	DevConsole();
	virtual ~DevConsole();

	void						Initialize();
	void						Update();
	void						Draw();

	Font*						mFont;
	Font*						mHeaderFont;
	Rect						mWorkArea;

	inline void					SetFont(Font* theFont) {mFont=theFont;}
	inline void					SetHeaderFont(Font* theFont) {mHeaderFont=theFont;}

	String						mHeader;
	String						mFooter;
	inline void					SetHeader(String theHeader) {mHeader=theHeader;}
	inline void					SetFooter(String theFooter) {mFooter=theFooter;}

	CPU							*HitTest(float x, float y);

	String						mLog[LOGMAX];
	short						mPlotLog;

	void						Log(String theString);
	void						Clear();

	float						mBorderSize;
	inline void					SetBorderSize(float theBorderSize) {mBorderSize=theBorderSize;}

public:
	void						TouchStart(int x, int y);
	void						TouchMove(int x, int y);
	void						TouchEnd(int x, int y);
	char						mDragWhat;
	Point						mDragLast;
	bool						mDrag;
};

//
// Just a CPU that, when you hit it, it'll kill off whatever it's pointed at.
// I use this to put behind non-fullscreen CPUs that should close if you touch off them.
//
class Closer : public CPU
{
public:
	Closer() {mCloseWhom=NULL;mColor=Color(0.0f);}
	Closer(CPU* theCPU) {mCloseWhom=theCPU;mColor=Color(0.0f);}

	CPU*						mCloseWhom;
	inline Closer*				SetCloseWhom(CPU* theCPU) {mCloseWhom=theCPU;return this;}
	Color						mColor;
	inline Closer*				SetColor(Color theColor) {mColor=theColor;return this;}
	inline Closer*				SetColor(float theR, float theG, float theB, float theA=1.0f) {mColor.mR=theR;mColor.mG=theG;mColor.mB=theB;mColor.mA=theA;return this;}

	void						Update() {if (mCloseWhom) if (mCloseWhom->mKill) Kill();}
	void						TouchStart(int x, int y)
    {
        if (mCloseWhom)
        {
            //if (mCloseWhom->mThrottle) mCloseWhom->ThrottleRelease(0);
            //else mCloseWhom->Kill();
			mCloseWhom->Kill();
        }
        Kill();
    }
	void						Draw();
};

//
// This sends strings to SystemNotify of the desired CPU, one per update (unless you pause it)
//
class Notifier : public CPU
{
public:
	Notifier();
	void						Update();
	void						AddNotify(CPU* notifyWho, String theCode, int theDelay=0);

public:
	struct NotifyStruct 
	{
		CPU*			mWho;
		String			mWhat;
		int				mDelay;
	};
	SmartList(NotifyStruct)		mNotifyList;
};

class DelayedEXE : public CPU
{
public:
	DelayedEXE(int theDelay=100, bool guaranteeADraw=true);
	DelayedEXE(void (*theCallback)(void* theExtraData),int theDelay=100, bool guaranteeADraw=true, void* theExtraData=NULL);
	void			Update();
	virtual void	EXE() {}
public:
	int				mDelay;
	int				mAppDrawNumber;
	bool			mWaitForDraw;
	bool			mDone;

	void			(*mCallback)(void* theExtraData);
	void*			mExtraData;
};

class DelayedHOOK : public CPU
{
public:
	DelayedHOOK(HOOKPTR theHook,int theDelay=100, bool guaranteeADraw=true);
	void			Update();
public:
	int				mDelay;
	int				mAppDrawNumber;
	bool			mWaitForDraw;
	bool			mDone;

	HOOKPTR			mHook;
};


//
// Runs some code on next update or next draw.  Can use Lambda functions!
// RunNextDraw(DFUNC({gOut.Out("Running after next draw!");}),NULL);
//
void RunAfterUpdates(int theUpdates, void (*theCallback)(void* theExtraData), void* theExtraData=NULL);
void RunAfterNextDraw(void (*theCallback)(void* theExtraData), void* theExtraData=NULL);
#define RunSoon(delay,x) RunAfterUpdates(delay,[](void* theExtraData) x)
#define RunASAP(x) RunAfterUpdates(0,[](void* theExtraData) x)
#define RunAfterDraw(x) RunAfterNextDraw([](void* theExtraData) x)
#define RunAfterUpdate(x) RunAfterUpdates(1,[](void* theExtraData) x)

void RunAfterUpdates(int theUpdates, HOOKPTR theHook);
void RunAfterNextDraw(HOOKPTR theHook);


extern class DevConsole* gDevConsole;

//
// Processes one string per draw... useful for any kind of weird progress bars of sys-level operations.
// Put in specifically to allow for "MoveFilesToCloud."
// Add a DrawHook to render it...
//
class StepProcessor : public CPU
{
public:

	Array<String>	mProcessList;
	int				mPCounter=0;
	inline String	GetProcess() {return mProcessList[mPCounter];}

	String			mCaption="MOVING DATA...";
	float			mProgress=0;
	inline float	GetProgress() {return mProgress;}

	void			Initialize() {}
	void			Update();
	void			Draw();
	virtual void	Process(String theData) {OS_Core::Printf("Process: %s",theData.c());OS_Core::Sleep(100);}

	int				mLastDrawUpdate=999999999;

	StepProcessor*	mNextProcessor=NULL; // Optional next processor to run

	CPUHOOKPTR		mOnComplete=NULL;	// Hook to run on completion...
	CPUHOOKPTR		mProcessHook=NULL;	// Process Hook

	inline StepProcessor* Caption(String theCaption) {mCaption=theCaption;return this;}
	inline StepProcessor* OnComplete(CPUHOOKPTR theHook) {mOnComplete=theHook;return this;}
	inline StepProcessor* DrawHook(CPUHOOKPTR theHook) {SetDrawHook(theHook);return this;}
	inline StepProcessor* ProcessHook(HOOKSTRINGPTR theHook) {mProcessHook=theHook;return this;}
	inline StepProcessor* Next(StepProcessor* theNext) {mNextProcessor=theNext;return this;}

};

void SetCloudConversions(String theConversions);	// If you want to convert, say, sandbox:// to something non-default in cloud, you do this:  "sandbox://->appdata/;cloud://->users/" ... only needed for special cases.
StepProcessor* BackupToCloud(String theFolder="sandbox://");	// If we want to move just one folder (this is really legacy compatibility, everything post 2021 shoud just call BackupToCloud)
StepProcessor* RestoreFromCloud(bool wipeLocalData=true);

#ifdef LEGACY_GL
class MoveFolderToCloud : public StepProcessor
{
public:
	String			mFromFolder;
	bool			mError=false;
	void			Process(String theData);
};

class MoveCloudToFolder : public StepProcessor
{
public:
	String			mToFolder;
	bool			mError=false;
	void			Process(String theData);
};
#endif

#undef COPY
