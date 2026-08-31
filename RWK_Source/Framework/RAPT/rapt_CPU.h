#pragma once
#include "rapt_object.h"
#include "rapt_rect.h"
#include "rapt_string.h"

#define CPUHOOK(func) [&](CPU* theThis) func
#define CPUHOOKPTR std::function<void(CPU* theThis)>
#define COPY(x) x=theSource->x

// NEW COMMUNITY
#ifndef RAPT_LEGACY //(NEW_COMMUNITY_2026)
#define RAPT_LEGACY //(NEW_COMMUNITY_2026)
#endif //(NEW_COMMUNITY_2026)

#ifndef LEGACY_GL //(NEW_COMMUNITY_2026)
#define LEGACY_GL //(NEW_COMMUNITY_2026)
#endif //(NEW_COMMUNITY_2026)
// NEW COMMUNITY

//
// The CPU class replaces the Gadget class from the old framework.
// A CPU is an object that is intended to act as a "program" in its 
// own right.  You can add CPUs to one to make them `child CPUs.
//
// CPUs respond to mouse/keyboard/system events.
//

#define DISPATCHPROCESS(forwarder,process) {SetCurrentCPU(this);forwarder->process;}

//
// CPU Features
//
enum
{
	CPU_TOUCH =				0x0001,
	CPU_DRAW =				0x0002,
	CPU_UPDATE =			0x0004,
	CPU_CLIP =				0x0008,
	CPU_CENTERED =			0x0010,
	CPU_TOUCHCHILDREN =		0x0020,
	CPU_DRAG =				0x0040,
	CPU_TIMESLICE =			0x0010,
	CPU_OUTERCHILDREN=		0x0020, // Means children can be outside the CPU's rect...

	CPU_MAX
};

#define CPU_DEFAULT (CPU_TOUCH|CPU_DRAW|CPU_UPDATE)


class CPUManager;
class Sprite;
class Graphics;

class CPU : public Object, public Rect
{
public:
	CPU();
	virtual ~CPU();
	
public:
	//
	// CPU Variable pointer... we can hook the CPU to a variable
	// that will get null'd out when the CPU is destroyed (if it matches)
	//
	virtual void		HookTo(void* thePointer);
	virtual void		HookToSingleton(void* thePointer);
	inline void			Singleton(void* thePointer) {HookToSingleton(thePointer);}
	void				Unhook();
	void*				mVariablePointer;

	static void			SetCurrentCPU(CPU *theCPU);
	static CPU*			GetCurrentCPU();

	void				KidnapTouch();	// Kidnaps system touches-- use to swap to another CPU (does not call TouchStart-- only TouchMove and TouchEnd)
	inline void			KidnapTouches() {KidnapTouch();}
	inline void			StealTouches() {KidnapTouch();}

	//
	// Little helper, just to locally see if FPSCursor is on...
	//
	bool				IsFPSCursor();


public:

	//
	// Sizing calls (so we can override)
	//
	void				Resize(float theX, float theY, float theWidth, float theHeight);
	void				Resize(Point theP1, Point theP2);
	void				SetSize(float theX, float theY, float theWidth, float theHeight) {Resize(theX,theY,theWidth,theHeight);}
	void				Size(float theX, float theY, float theWidth, float theHeight) {Resize(theX,theY,theWidth,theHeight);}
	void				Size(Rect theRect) {Resize(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	void				Size(float theWidth, float theHeight);
	void				Size(Point theSize) {Size(theSize.mX,theSize.mY);}
	void				Size();

	inline Rect			LocalRect() {return Rect(0,0,mWidth,mHeight);}

	virtual void		SizeChanged() {}

public:
	//
	// Various bits of data about the CPU
	//
	int					mAge;			// Age of CPU
	bool				mManualAge;		// Turn this on if you want to age the CPU manually.
	inline int			Age() {return mAge;}
	inline int			GetAge() {return Age();}
	void				ManualAge(bool theState=true) {mManualAge=theState;}

	//
	// Lets us return stuff about our CPU as MISC info...
	//
	virtual char*		WhoAreYou() {return "CPU";}


	//
	// Helper to see if we're over this CPU...
	//
	bool				IsHover();
	inline bool			IsOver() {return IsHover();}

public:

	//
	// The core update/draw
	//
	virtual void		Core_Initialize();
	virtual void		Core_Update();
	virtual void		Core_Draw();
	virtual void		DrawOverlay() {}
	virtual void		DrawToolTip() {}

	//
	// For forwarding... this lets us make a child CPU without having to make a whole class
	// for it, for the least little thing.
	//
	CPU					*mForwardUpdate;
	CPU					*mForwardDraw;
	CPU					*mForwardTouches;
	CPU					*mForwardKeyboard;
	CPU					*mForwardAccelerometer;
	inline void			ForwardUpdate(CPU *forwardTo=NULL) {if (!forwardTo) forwardTo=this;mForwardUpdate=forwardTo;}
	inline void			ForwardDraw(CPU *forwardTo) {if (!forwardTo) forwardTo=this;mForwardDraw=forwardTo;}
	inline void			ForwardTouches(CPU *forwardTo) {if (!forwardTo) forwardTo=this;mForwardTouches=forwardTo;}
	inline void			ForwardKeyboard(CPU *forwardTo) {if (!forwardTo) forwardTo=this;mForwardKeyboard=forwardTo;}
	inline void			ForwardAccelerometer(CPU *forwardTo) {if (!forwardTo) forwardTo=this;mForwardAccelerometer=forwardTo;}
	inline void			Forward(CPU *forwardTo) {if (!forwardTo) forwardTo=this;mForwardUpdate=forwardTo;mForwardDraw=forwardTo;mForwardTouches=forwardTo;mForwardKeyboard=forwardTo;mForwardAccelerometer=forwardTo;}

	//
	// CPU does extra work on Kill
	//
	void				Kill();

	//
	// Gets calls from the main app if the app gets
	// minimized or restored...
	//
	virtual void		Core_Multitasking(bool isForeground);	// Multitasking was invoked... if isForeground is false, we've become backgrounded.
	virtual void		Multitasking(bool isForeground) {}

	//
	// For system notifies... this is to receive notifications from 
	// the OS_Core for various things...
	//
	virtual void		Core_SystemNotify(String theNotify);
	virtual void		SystemNotify(String theNotify) {}

	//
	// Other notifies...
	//
	virtual void		Back() {}	// Back/Esc button was pressed 

	//
	// Mouse/finger interfaces
	// Query the App for the Mouse Button or Touch Hash
	//
	virtual void		Core_TouchStart(int x, int y);
	virtual void		Core_TouchMove(int x, int y);
	virtual void		Core_TouchEnd(int x, int y);
	virtual void		Core_Spin(int theDir);			// Mousewheel
	virtual void		Core_KeyDown(int theKey);
	virtual void		Core_KeyUp(int theKey);
	virtual void		Core_Char(int theKey);
	virtual void		Core_Accelerometer(int theX, int theY, int theZ);	// Values multiplied by 10000
	virtual void		Core_GotKeyboardFocus();
	virtual void		Core_LostKeyboardFocus();

	virtual void		Drag(int x, int y) {}
	virtual void		DragStart(int x, int y) {}
	virtual void		DragEnd(int x, int y) {}
	bool				mDragging=false;
	char				mDragThreshold=5;
	//
	// Overrides for input...
	//
	virtual void		TouchStart(int x, int y) {}
	virtual void		TouchMove(int x, int y) {}
	virtual void		TouchEnd(int x, int y) {}
	virtual void		Spin(int theDir) {}
	virtual void		KeyDown(int theKey) {}
	virtual void		KeyUp(int theKey) {}
	virtual void		Char(int theKey) {}
	virtual void		Accelerometer(int theX, int theY, int theZ) {}
	virtual void		GotKeyboardFocus() {}
	virtual void		LostKeyboardFocus() {}
	//
	// Converts absolute coordinates to CPU coordinates...
	// What this does is take into effect whether the CPU is centered or not.
	//
	virtual IPoint		FixCPUCoordinates(int x, int y);
	//
	// Tells us if the mouse or touch has entered or left
	// the CPU... 
	//
	virtual void		Core_Enter();
	virtual void		Core_Leave();
	virtual void		Enter() {}
	virtual void		Leave() {}
	//
	// Helper functions for when this CPU is "user focused."  User Focused just means it
	// was the most recent one touched/clicked.  Some CPUs, like textboxes, might want
	// to know if the user has clicked somewhere else, so they can save the
	// data.
	//
	virtual void		LostUserFocus() {}

		
	//
	// These let us lock various devices onto
	// this CPU
	//
	void				FocusTouches(bool theState=true);			// Also focuses the mousewheel when called, but is smart about unfocusing the wheel
	void				FocusSpins(bool theState=true);				// This is the mousewheel
	void				FocusKeyboard(bool theState=true);			// Keyboard (will pop up keyboard on Touch devices)
	void				FocusAccelerometer(bool theState=true);		// Acceleromater (for touch devices that support it)
	void				FocusBackButton(bool theState=true);		// Back button (also ESC on PC)

	bool				IsFocusKeyboard(bool includeChildren=true);

	//
	// Tells us if this point is within the CPU's physical bounds...
	//
	virtual CPU			*HitTest(float x, float y);
	inline CPU			*HitTest(int x, int y) {return HitTest((float)x,(float)y);}
	inline CPU			*HitTest(Point thePos) {return HitTest(thePos.mX,thePos.mY);}
	inline CPU			*HitTest(IPoint thePos) {return HitTest(thePos.mX,thePos.mY);}

	inline void			SetHitBorder(Point theBorder) {mHitBorder=theBorder;}
	inline void			SetHitBorder(float theBorder) {mHitBorder.mX=theBorder;mHitBorder.mY=theBorder;}
	inline void			SetHitBorder(float theBX, float theBY) {mHitBorder.mX=theBX;mHitBorder.mY=theBY;}
	Point				mHitBorder;	// Allows us to add a little bit of a border for hittesting.

	//
	// Child CPUs...
	//
	CPUManager					*mCPUManager;
	void operator+=				(CPU *theCPU) {AddCPU(theCPU);}
	void operator-=				(CPU *theCPU) {RemoveCPU(theCPU);}
	virtual void				AddCPU(CPU *theCPU);
	void						RemoveCPU(CPU *theCPU);
	void						AddCPUToBottom(CPU *theCPU);
	void						KillChildren();


	//
	// CPU Timers (these notify when they count down to zero)
	//
	struct CPUTimer
	{
		void*		mNotify;
		int			mTime;
	};
	void						AddTimer(void *theNotify, int theTime);
	List						*mTimerList;

	//
	// Cursor stuff...
	// Doesn't do anything on a touch device.
	// 
	int							mCursor;
	void						SetCursor(int theCursor=CURSOR_NORMAL);
	void						SetFingerCursor() {SetCursor(CURSOR_FINGER);}
	void						SetHandCursor() {SetCursor(CURSOR_HAND);}

	//
	// For notification
	//
	Object						*mNotify;
	inline void					SetNotify(Object *theObject) {mNotify=theObject;}
	//
	// Extra data for notifications... use this
	// if you're notifying, but have to send extra info...
	//
	struct NotifyData
	{
		int						mParam;
		void					*mExtraData;
	} mNotifyData;

	//
	// Throttling...
	//

#ifndef NO_THREADS
	int							Throttle();
#endif
	void						ThrottleRelease(int theResult=0);
	bool						mThrottle;
	bool						mThrottleFinishing;
	int							mThrottleResult;

	//
	// Special throttle for non-threaded apps... calls the HOOKARG function with theArg when ThrottleRelease is called...
	// if autoKill is true, it will call this->Kill() on the CPU when ThrottleRelease happens (good for messagesboxes)
	// if runHookOnlyOnNonNullResult is true, it only calls the hook if mThrottleResult!=0 (good for quick yesno messageboxes)
	//
	virtual void				Throttle(CPUHOOKPTR theHook, bool autoKill=false, bool runHookOnlyOnNonNullResult=false);
	CPUHOOKPTR					mThrottleHook;
	bool						mThrottleAutokill=false;
	bool						mThrottleNonNullHook=false;

	//
	// Features
	//
	short						mFeatures;
	inline void					SetFeatures(int theFeatures) {mFeatures=theFeatures;FeaturesChanged();}
	inline void					EnableFeature(int theFeature) {mFeatures|=theFeature;FeaturesChanged();}
	inline void					DisableFeature(int theFeature) {mFeatures|=theFeature;mFeatures^=theFeature;FeaturesChanged();}
	virtual void				Hide(bool doHide=true) {if (doHide) {DisableFeature(CPU_DRAW);DisableFeature(CPU_TOUCH);} else {EnableFeature(CPU_DRAW);EnableFeature(CPU_TOUCH);}}
	inline void					EnableDraw() {EnableFeature(CPU_DRAW);}
	inline void					EnableUpdate() {EnableFeature(CPU_UPDATE);}
	inline void					EnableTouch() {EnableFeature(CPU_TOUCH|CPU_TOUCHCHILDREN);}
	inline void					EnableTouchChildren() {EnableFeature(CPU_TOUCHCHILDREN);}
	inline void					EnableTimeslice() {EnableFeature(CPU_TIMESLICE);}
	inline void					EnableTS() {EnableFeature(CPU_TIMESLICE);}
	inline void					EnableClip() {EnableFeature(CPU_CLIP);}
	inline void					EnableDrag(char theThreshold=5) {EnableFeature(CPU_DRAG);if (theThreshold!=0) mDragThreshold=theThreshold;}
	inline void					DisableDraw() {DisableFeature(CPU_DRAW);}
	inline void					DisableUpdate() {DisableFeature(CPU_UPDATE);}
	inline void					DisableTouch() {DisableFeature(CPU_TOUCH|CPU_TOUCHCHILDREN);}
	inline void					DisableClip() {DisableFeature(CPU_CLIP);}
	inline void					DisableTouchChildren() {DisableFeature(CPU_TOUCHCHILDREN);}
	inline void					DisableDrag() {DisableFeature(CPU_DRAG);}
	inline void					DisableTimeslice() {DisableFeature(CPU_TIMESLICE);}
	inline void					DisableTS() {DisableFeature(CPU_TIMESLICE);}
	inline bool					IsVisible() {return (mFeatures&CPU_DRAW)!=0;}
	inline void					EnableOuterChildren() {EnableFeature(CPU_OUTERCHILDREN);}
	inline void					DisableOuterChildren() {DisableFeature(CPU_OUTERCHILDREN);}
	inline void					Ghost() {DisableFeature(CPU_TOUCH);EnableFeature(CPU_TOUCHCHILDREN);}
	inline void					UnGhost() {EnableFeature(CPU_TOUCH);}

	//
	// When features are changed, this is called.
	// Override if you're doing fancy stuff, such as with an
	// isolated CPU
	//
	virtual void				FeaturesChanged() {}


	//
	// CPU Pausing
	//
	int							mPauseCount;
	inline void					Pause() {mPauseCount++;}
	//inline void					Pause(int theCount) {mPause=theCount;}
	inline void					Unpause() {mPauseCount=_max(0,mPauseCount-1);}
	inline void					UnpauseAll() {mPauseCount=0;}
	inline bool					IsPaused() {return !(mPauseCount==0);}

	//
	// Extra Data... for anything you might need it for
	//
	CleanArray<void*>			mExtraDataList;
	void*&						ExtraData(int aSlot=0) {return mExtraDataList[aSlot];}

	//
	// Parent CPU... when we're added to a CPU, this gets set to whoever we're added to...
	//
	CPU*						mParent;

	//
	// If first draw
	//
	bool						mFirstDraw=true;

	//
	// Convert screen coordinates to this CPU's local coordinates,
	// and vice versa.
	//
	Point						ConvertScreenToLocal(Point thePos);
	Point						ConvertLocalToScreen(Point thePos);
	inline Point				ScreenToLocal(Point thePos) {return ConvertScreenToLocal(thePos);}
	inline Point				LocalToScreen(Point thePos) {return ConvertLocalToScreen(thePos);}
	//
	// And this will give us the real, onscreen location of the CPU, not relative
	// to parents/children/etc
	//
	Rect						ConvertRectToScreen();
	Rect						ConvertRectToLocal();
	Rect						ConvertRectToLocal(Rect theRect);

	inline Rect					RectToScreen() {return ConvertRectToScreen();}
	inline Rect					RectToLocal()  {return ConvertRectToLocal();}
	inline Rect					RectToLocal(Rect theRect) {return ConvertRectToLocal(theRect);}

	//
	// Tells whether the CPU captures touches when you touch.
	// If it does capture touches (default) then the CPU will never receive a "Leave()"
	// call so long as the touch is ongoing.  If it does not capture, it will behave
	// the same way it does just from a hovering mouse.
	//
	bool						mCaptureTouches;
	inline void					CaptureTouches(bool theState=true) {mCaptureTouches=theState;}

	//
	// Quickie function to bring a child CPU to the top...
	//
	void						BringChildToTop(CPU* theChild) {*this-=theChild;*this+=theChild;}

	//
	// This horrible, and pointless function is put here specifically to provide Mac compatibility.
	// In XCode, templates are nightmarish, horrible.  I can't get them to have access to my main app
	// no matter what I do.  So this just gives us a pointer to the graphics class, specifically for use
	// when we have a drawing template.  God.
	//
	Graphics*					G();

	//
	// An extended version of notify, used when you need to return some results.
	//
	virtual void*				NotifyEX(void *theData) {return NULL;}

#ifdef _DEBUG
	String						mName;
#endif

	//
	// Hooks for creating quick and dirty CPUs that don't mean anything...
	//
	CPUHOOKPTR			mInitializeHook=NULL;
	CPUHOOKPTR			mUpdateHook=NULL;
	CPUHOOKPTR			mDrawHook=NULL;
	CPUHOOKPTR			mDrawOverHook=NULL;
	CPUHOOKPTR			mDrawTopmostHook=NULL;
	inline CPU*			SetDrawHook(CPUHOOKPTR theHook) {mDrawHook=theHook;EnableFeature(CPU_DRAW);return this;}
	inline CPU*			SetDrawOverHook(CPUHOOKPTR theHook) {mDrawOverHook=theHook;EnableFeature(CPU_DRAW);return this;}
	inline CPU*			SetDrawTopmostHook(CPUHOOKPTR theHook) {mDrawTopmostHook=theHook;EnableFeature(CPU_DRAW);return this;}
	inline CPU*			SetUpdateHook(CPUHOOKPTR theHook) {mUpdateHook=theHook;EnableFeature(CPU_UPDATE);return this;}
	inline CPU*			SetInitializeHook(CPUHOOKPTR theHook) {mInitializeHook=theHook;EnableFeature(CPU_UPDATE);return this;}

	//
	// Function that gets called if we specified CPU_TIMESLICE in features.  This gets called once every draw, with the time since the last draw.
	// The number that comes in is in UPDATES, but it can be fractional.  So a perfect update would have theTime=1.0f, a slightly slow one would be 1.1f, slightly slow would be .9f
	//
	virtual void		UpdateTimeslice(float theTime) {}

	CPU*				Copy(CPU* theSource)
	{
		COPY(mInitializeHook);
		COPY(mUpdateHook);
		COPY(mDrawHook);
		COPY(mDrawOverHook);
		COPY(mFeatures);
		COPY(mCursor);
		COPY(mHitBorder);
		COPY(mNotify);			// Whether to copy notify is questionable!  I.E. in CPanel, if we copy a slider, we DO want the slider's notify... we don't want the thumb's to copy (thumb needs to notify parent)
		COPY(mDragThreshold);
		COPY(mCaptureTouches);
		return this;
	}
};

//
// A CPU manager class is an ObjectManager that has some extra functions for 
// touches and things.
//
class CPUManager : public ObjectManager
//class CPUManager : public SmartObjectManager<CPU>
{
public:
	virtual				~CPUManager();
	void				Update();
	void				Draw();

	//
	// Pick just picks the CPU under the coordinates.
	//
	CPU*				Pick(float x, float y);
	inline CPU*			Pick(Point thePos) {return Pick(thePos.mX,thePos.mY);}
	inline CPU*			Top() {return (CPU*)this->Last();}
	inline CPU*			Topmost() {return (CPU*)this->Last();}

	void				Multitasking(bool theState);
	void				SystemNotify(String theNotify);

	virtual void		DestroyObject(Object *theObject);

	//
	// If you need to manually kill off the kill lists for whatever
	// reason, here's your place.
	//
	void				ClearKillLists();
	List				mThrottledList;

};

void PushFocusSpins(CPU* theCPU);
void PopFocusSpins();

#undef COPY