#pragma once
#include "rapt.h"

String					ClickOrTouch(String theString, bool forceTouchDevice=false);
String					ClickOrTap(String theString, bool forceTouchDevice=false);

enum
{
	CLOUDSTATUS_NULL=-1,
	CLOUDSTATUS_PENDING,
	CLOUDSTATUS_FAIL,
	CLOUDSTATUS_SUCCESS,
};

int* TempInt();
float* TempFloat();


class AssetLoader;
class Graphics;
class Input;
class App : public CPU
{
public:
	//
	// Probably should override and you need
	//
	App();
	virtual ~App();

	virtual void			Initialize() {}			// Gets called after program is all set up and ready to go (you can load graphics here and whatnot)
	virtual void			Update() {}				// Called every update loop
	virtual void			Draw() {}				// Called to draw
	virtual void			DrawOverlay() {}		// Called to draw overlay
	virtual void			Shutdown() {}			// Gets called right when the program quits... so you can do saves and things here, rather than in the deconstructor.
	virtual void			ResetGlobals() {}		// Mainly for multiplayer-- gets called by the MagicGlobalData class when IT gets reset.  If you aren't using MagicGlobalData, no need to implement.
	virtual void			SaveEverything() {}		// Gets called when the app needs to save everything because it could be terminated.  Different invocations by OS.  On PC, it's when they quit the app.  On mobile, it can happen when they background it.

	virtual void			Load() {}				// Main load... calls LoadComplete on finish...
	virtual void			LoadBackground() {}		// Stuff that loads in the background after main load finishes
	virtual void			LoadComplete() {}
	virtual void			LoadBackgroundComplete() {}

	virtual void			Rehup(String theReason) {}			// To rehup the app if anything major might have changed.

	virtual bool			DrawWorking() {return false;}		// Something we draw when a "deep function" is working... return TRUE so that it doesn't just get shut down...

public:
	//
	// Load steps
	//
	bool					mInitComplete=false;
	bool					mLoadComplete;
	virtual float			GetLoadComplete();
	String					GetLoadStatus();
	//float					GetCloudLoadComplete();
	
public:

	//
	// Shouldn't override unless you really know
	// what you're doing
	//

	virtual void			Go(bool runMainLoop=true);	// Initializes everything... your total interface to the entire App
	virtual void			Stop();				// The code that de-initializes everything
	virtual void			Throttle();			// Inner game loops
	virtual void			FrameController();	// Timer controller
	virtual void			FrameControllerTimeSlice();	// Timer controller for updating by time slice
	virtual void			ThrottleUpdate();	// Core processing for updates
	virtual void			ThrottleDraw();		// Core processing for draws
	virtual void			PreRender();		// Interfaces with OS graphics...
	virtual void			Render();			// Core processing for rendering
	virtual void			PostRender();		// Interfaces with OS graphics...
	virtual void			Quit();				// Call this to make the program quit
	virtual bool			IsQuit();			// Have we quit?  (Covers if the OS quits the program, and not you)
	virtual void			ThrottleShutdown();	// Shuts down the program (kills all gadgets, invokes Shutdown())
	virtual void			Reboot();			// Reboots the app...

	//
	// We might have work that needs doing... we can't shutdown until it's done...
	// This is for threads that need to run after the app finishes...
	//
	inline bool				WantShutdown() {return (OS_Core::WantShutdown() && mPendingWork<=0);}
	int						mPendingWork=0;		// We can only shut down if this is zero
	inline void				StartPendingWork() {mPendingWork=_max(mPendingWork+1,1);}
	inline void				FinishPendingWork() {mPendingWork=_max(mPendingWork-1,0);}



	//
	// Manual draw functionality... put in so that when we flesh out "Working" we can easily
	// draw everything, draw stuff on top of it, then then show, without having to think too much.
	//
	void					CustomDraw(bool startOrFinish, bool drawMainPipeline=true);	//True=start, false=finish

	//
	// Various functions can call this to draw a working screen, for non-threaded background processing...
	// If your app ever returns "false" then this gets shut down permanently.  If true, it'll dump into 
	// DrawWorking with the time passed since last time it was drawn.
	//
	inline void				Working(int theInterval) {static int aInterval=0;if (++aInterval>=theInterval) {aInterval=0;Working();}}
	inline void				Working()
	{
		static bool aEnableWorking=true;
		if (aEnableWorking)
		{
			static unsigned int aWorkingTime=-500;
			if (Tick()-aWorkingTime>150) if (aEnableWorking) 
			{
				aEnableWorking=DrawWorking();
				aWorkingTime=Tick();
			}
		}
	}



	void					LoadSettings();							// Loads the game settings data

	//static void				PullFromCloud(void* theArg);	// Grabs the game's settings from the cloud
	//bool					mCloudLoadPending;				// True if we're waiting on the cloud for our initial data...

	bool					HasCloud();

	//
	// Helpers
	//
	void					RefreshCursor();	// Refreshes the cursor in case its lost sync...
	void					RefreshTouches();	// Re-submits the touch position to the system queue... useful if anything onscreen changed and you need some processing to reflect that.
												// For example, if you had a button, then killed it, you could call this if something underneath the button needed to be hilited.

	void					ForceTouchEnd(bool ignoreNextTouchEnd,Point thePos=Point(FLT_MAX,FLT_MAX));	// Forces an immediate TouchEnd at the current position.  Put in to help us get around quirkiness in ControlPanel
	void					ForceTouchStart(Point thePos=Point(FLT_MAX,FLT_MAX));	// Forces an immediate TouchStart at the currnet position

	inline void				SetAppName(String theName) {mAppName=theName;}
	inline String			GetAppName() {return mAppName;}
	inline void				SetPublisher(String thePublisher) {mPublisher=thePublisher;}
	inline String			GetPublisher() {return mPublisher;}
	inline String			GetUserName() {return mUserName;}
	inline String			GetUserPassword() {return mUserPassword;}
	inline String			GetSessionToken() {return mSessionToken;}
	inline String			GetComputerName() {return mComputerName;}
	inline String			GetIPAddres() {return mIPAddress;}
	void					SetUserName(String theName);
	void					SetUserPassword(String theName);
	void					SetSessionToken(String theToken);
	void					SetComputerName(String theName);
	void					SetIPAddress(String theAddress);
	inline void				SetAppVersion(String theVersion) {mAppVersion=theVersion;}
	String					mAppName;
	String					mAppVersion;
	String					mPublisher;
	String					mUserName;
	String					mUserPassword;
	String					mSessionToken;
	String					mComputerName;
	String					mIPAddress;
	int						mToolVersion;		// Just some versioning for any tools the app might provide.  Server sents [EXE]_settoolversion:int[/EXE].
												// Sometimes we have tools in the build that aren't in the app yet, and we want those disabled.

	void					SetCursorPos(int x, int y);
	void					SetFPSCursor(CPU* theCPU=NULL);
	CPU*					mFPSCursorCPU=NULL;
	inline bool				IsFPSCursor() {return mFPSCursorCPU!=NULL;}

	inline int				ToolVersion() {return mToolVersion;}
	inline void				SetToolVersion(int theVersion) 
	{
		mToolVersion=theVersion;
#ifndef ALLOW_REGISTER_VARIABLES
		mSettings.SetInt("System.ToolVersion",mToolVersion);
#endif
	}

	void					CatchCrashes(String theURL);
	void					SetCrashInfo(String theInfo);
	String					mCrashInfo;
	int						mCrashInfoThreadlock;	// Threadlock so we can set crash info from any thread.



	unsigned int			Tick();
	unsigned int			TickUpdates();

	unsigned int			mAppStartTick=-1;
	inline unsigned int		AppStartTick() {return mAppStartTick;}
	inline unsigned int		TicksSinceAppBegan() {return Tick()-mAppStartTick;}
	inline unsigned int		TickUpdatesSinceAppBegan() {return ((Tick()-mAppStartTick)/1000)*(unsigned int)mUpdatesPerSecond;}

	//
	// CPU manager for background CPUs.  These only update, nothing else.
	// Use them for anything you want to run in the background, but not display or touch.
	//
	ObjectManager			mBackgroundProcesses;
	void					AddBackgroundProcess(Object *theObject) {mBackgroundProcesses+=theObject;}
	void					RemoveBackgroundProcess(Object *theObject) {mBackgroundProcesses-=theObject;}

	CPU*					mToolTipCPU=NULL;	// Object to draw on top of everything (used basically for tooltips)
	Rect					mToolTipClip;
	Point					mToolTipTranslate;
	void					ToolTip(CPU* theCPU);
	inline void				Tooltip(CPU* theCPU) {ToolTip(theCPU);}
	inline void				SetTooltip(CPU* theCPU) {ToolTip(theCPU);}
	inline void				SetToolTip(CPU* theCPU) {ToolTip(theCPU);}

	//
	// Timesliced objects... these just call UpdateTimesliced...
	// A CPU with CPU_TIMESLICE will get added to this list automatically when it add it to a CPUManager...
	//
	CPUManager				mTimeSliceProcesses;
	int						mLastTimeSlice=-1;

public:
	//
	// Big Classes that shouldn't be global...
	//
	Graphics				mGraphics; // <-- PROBLEM HERE?  Make sure RAPT_LEGACY is defined in addition to LEGACY_GL
	Audio					mAudio;
	Input					mInput;


public:

	//
	// This is the command line that was passed in, in case you ever need that
	//
	String					mCommandLine;
	inline String			GetCommandLine() {return mCommandLine;}

	//
	// Control stuff...
	//
	float					mUpdatesPerSecond;
	inline void				SetUpdatesPerSecond(float theUpdates) {mUpdatesPerSecond=theUpdates;}
	int						mThrottleSleep=1;
	inline void				SetThrottleSleep(int howLong=1) {mThrottleSleep=howLong;}

	void					RecoverTime();
	inline void				CatchUp() {RecoverTime();}

	struct Frame
	{
		unsigned int			mBaseUpdateTime;
		int						mCurrentUpdateNumber;
		bool					mBreakUpdate;
		float					mDesiredUpdate;
	} mFrame;

	int						mFPS;
	inline int				FPS() {return mFPS;}
	inline void				ResetFPS() {mFPS=60;}

public:

	//
	// App Statistics
	//

	inline int				AppTime() {return mAge;}
	inline int				GetAppTime() {return mAge;}
	inline float			AppTimeF() {return (float)mAge;}
	inline float			GetAppTimeF() {return (float)mAge;}

	union
	{
		int						mAppDrawNumber;
		int						mDrawCount;
	};
	inline int				AppDrawNumber() {return mAppDrawNumber;}
	inline int				DrawNumber() {return mAppDrawNumber;}

	//
	// Allows us to skip draws-- this is useful if things are going to
	// be changing across a couple frames, but you don't want a flicker
	// or in-between stages to show.
	//
	int						mSkipDraw;
	inline void				SkipDraw(int theCount=1) {mSkipDraw=theCount;}

	//
	// For multithreading... since it's possible for the App to be quit, but not
	// shut down.  This was added explicitely to be watched in the WinMain thread
	// to run multithreaded on Windows.
	//
	bool					mIsShutdown;
	inline bool				IsShutdown() {return mIsShutdown;}

	//
	// Make the App minimalist... meaning it does not save config files or
	// anything else automatically.
	//
	bool					mMinimalistApp;
	inline bool				IsMinimalistApp() {return mMinimalistApp;}
	void					SetMinimalistApp(bool theState=true) {mMinimalistApp=theState;}

	//
	// Make the App portable...
	// This means the Sandbox folder goes inside the package.
	//
	bool					mPortableApp;
	inline bool				IsPortableApp() {return mPortableApp;}
	void					SetPortableApp(bool theState=true);


	//
	// Special function for if you drop a file or files on the app...
	//
	virtual void			DroppedFile(String theFilename) {}

	//
	// Convert updates into seconds (and vice versa)
	//
	inline float			UpdatesToTicks(float theUpdates) {return UpdatesToSeconds(theUpdates)*1000.0f;}
	inline float			UpdatesToSeconds(float theUpdates) {return theUpdates/mUpdatesPerSecond;}
	inline float			UpdatesToMinutes(float theUpdates) {return UpdatesToSeconds(theUpdates)/60;}
	inline float			UpdatesToHours(float theUpdates) {return (UpdatesToSeconds(theUpdates)/60)/60;}
	inline float			SecondsToUpdates(float theSeconds) {return theSeconds*mUpdatesPerSecond;}
	inline float			MinutesToUpdates(float theMinutes) {return SecondsToUpdates(theMinutes*60);}
	inline float			HoursToUpdates(float theHours) {return MinutesToUpdates(theHours*60);}

	//
	// For handling background (defaults to pause when in background)
	//
	bool					mPauseWhenInBackground;
	bool					mIdleWhenInBackground;
	inline bool				ShouldPauseWhenInBackground() {return mPauseWhenInBackground;}
	inline void				PauseWhenInBackground(bool theState=true) {mPauseWhenInBackground=theState;}
	inline bool				ShouldIdleWhenInBackground() {return mIdleWhenInBackground;}
	inline void				IdleWhenInBackground(bool theState=true) {mIdleWhenInBackground=theState;}
	bool					mInBackground;
	bool					mNeedBackgroundDraw;

	//
	// Specific background pausing stuff, for individual features
	//
	bool					mPauseSoundWhenInBackground;
	inline bool				ShouldPauseSoundWhenInBackground() {return mPauseSoundWhenInBackground;}
	inline void				PauseSoundWhenInBackground(bool theState=true) {mPauseSoundWhenInBackground=theState;}

	//
	// Unpausing the main app should force a CatchUp....
	//
	inline void				Unpause() {CPU::Unpause();CatchUp();}


public:
	//
	// System functions (to get info about
	// what we're on, or what's happening)
	//
	bool					IsTouchDevice();
	bool					IsSmallScreen();
	inline bool				IsPhone() {return IsSmallScreen();}
	bool					IsRetinaDevice();
	String					GetOSName();
	String					GetOSVersion();

	//
	// To minimize the app (if applicable)
	//
	void					Minimize();

	//
	// Extra functions to let us get at
	// some data that we will not necessarily need all the time...
	//
	int						mMessageData_TouchID;
	IPoint					mMessageData_TouchPosition;
	IPoint					mMessageData_LastTouchPosition;
	IPoint					mMessageData_TouchStartPosition;
	int						mMessageData_MouseButton;
	int						mMessageData_IsTouching;
	int						mMessageData_TouchTime;
	int						mMessageData_LastTouchTime;
	int						mMessageData_TouchNumber=0;
	float					mMessageData_MaxMove=0; // Maximum we ever were from our original start pos
	bool					mCanRefreshTouches;

	inline int				GetTouchID() {return mMessageData_TouchID;}
	inline int				GetTouchHash() {return GetTouchID();}
	inline IPoint			GetTouchPosition() {return mMessageData_TouchPosition;}
	inline Point			GetTouchPositionF() {return Point(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);}
	inline IPoint			GetLastTouchPosition() {return mMessageData_LastTouchPosition;}
	inline Point			GetLastTouchPositionF() {return Point(mMessageData_LastTouchPosition.mX,mMessageData_LastTouchPosition.mY);}
	inline IPoint			GetTouchStartPosition() {return mMessageData_TouchStartPosition;}
	inline Point			GetTouchStartPositionF() {return Point(mMessageData_TouchStartPosition.mX,mMessageData_TouchStartPosition.mY);}
	inline IPoint			GetTouchDelta() {return mMessageData_TouchPosition-mMessageData_LastTouchPosition;}
	int						GetMouseButton() {return mMessageData_MouseButton;}
	inline bool				IsTouching() {return (mMessageData_IsTouching!=0);}
	inline void				SetIsTouching(bool theState) {mMessageData_IsTouching=theState;}
	inline bool				IsDoubleClick(int theThreshold=25) {return ((mMessageData_TouchTime-mMessageData_LastTouchTime)<=theThreshold);}
	inline float			GetTouchLength() {return gMath.Distance(mMessageData_TouchStartPosition,mMessageData_TouchPosition);}
	inline float			GetTouchLengthSquared() {return gMath.DistanceSquared(mMessageData_TouchStartPosition,mMessageData_TouchPosition);}
	inline float			GetTouchMoveLength() {return GetTouchLength();}
	inline float			GetTouchMoveLengthSquared() {return GetTouchLengthSquared();}
	inline Point			GetTouchVector() {return mMessageData_TouchPosition-mMessageData_TouchStartPosition;}
	inline Point			GetLastTouchMove() {return mMessageData_TouchPosition-mMessageData_LastTouchPosition;}
	void					UniquifyTouchID(CPU* theCPU);
	inline int				GetTouchNumber() {return mMessageData_TouchNumber;}
	inline float			GetTouchMoveMax() {return mMessageData_MaxMove;}
	inline float			GetMaxTouchMove() {return GetTouchMoveMax();}


	inline int				TouchX() {return mMessageData_TouchPosition.mX;}
	inline int				TouchY() {return mMessageData_TouchPosition.mY;}
	inline float			TouchXF() {return (float)mMessageData_TouchPosition.mX;}
	inline float			TouchYF() {return (float)mMessageData_TouchPosition.mY;}
	inline Point			TouchPos() {return mMessageData_TouchPosition;}

	//
	// Lets us quickly/easily see which mouse button is down.
	//
	short					mMessageData_TouchStatus;
	inline short			GetTouchStatus() {return mMessageData_TouchStatus;}

	//
	// Extra functions to help us with gadget maintainance... this is for really
	// fancy stuff (for instance, ControlPanel uses this stuff to help it defocus
	// whatever you're clicked on if you seem to want to be scrolling)
	//
	CPU*					GetCPUAtPos(int x, int y);

	//
	// System message handling functions, to preprocess and send on data...
	// (Not to be confused with Core, which is CPU processing)
	//
	virtual void			System_Process();
	virtual void			System_Clear();
	virtual void			System_ProcessMessage(int theMessage, int theID, int theData1, int theData2, int theData3, void* theDataPtr);
	virtual void			System_PostMessage(int theMessage, int theID=0, int theData1=0, int theData2=0, int theData3=0, void* theDataPtr=NULL);
	virtual void			System_TouchMove(int x, int y);
	virtual void			System_TouchStart(int x, int y);
	virtual void			System_TouchEnd(int x, int y);
	virtual void			System_Spin(int theDir);
	virtual void			System_KeyDown(int theKey);
	virtual void			System_KeyUp(int theKey);
	virtual void			System_Char(int theChar);
	virtual void			System_Back();
	virtual void			System_Accelerometer(int theX, int theY, int theZ);	// Remember, all accelerometer values will be multiplied by like 10,000 or something, to make them ints.

	virtual void			System_PostTouch(int theX, int theY);
	inline void				System_PostTouch(Point thePos) {System_PostTouch((int)thePos.mX,(int)thePos.mY);}

public:
	//
	// This is a universal object factory that is here to assist with syncing.
	// When you use SyncBuffer to sync an ObjectManager, it will refer to this
	// function to create new objects.
	//
	virtual Object*			Factory(int theID) {return NULL;}

public:

	//
	// CPU focusing... if a CPU is focused, it will receive
	// all messages from a given device.
	//
	CPU*					mFocusKeyboardCPU;
	CPU*					mFocusTouchesCPU;
	Stack<CPU*>				mFocusSpinsCPU;
	CPU*					mFocusAccelerometerCPU;
	List					mFocusBackButtonCPUList;

	//CPU*					mOverCPU;	// The CPU we're over-- always, even if we're focused on one
	void					FocusKeyboard(CPU *theCPU);
	inline void				FocusTouches(CPU *theCPU) {mFocusTouchesCPU=theCPU;SetHoverCPU(theCPU);}
	inline void				FocusBackButton(CPU* theCPU) {mFocusBackButtonCPUList+=theCPU;SetHoverCPU(theCPU);}
	inline void				FocusSpins(CPU *theCPU) {mFocusSpinsCPU.Unpush(theCPU);mFocusSpinsCPU.Push(theCPU);} //Unpush first makes it so we just come to the top if we're already in the list
	inline void				FocusAccelerometer(CPU *theCPU) {mFocusAccelerometerCPU=theCPU;}
	inline void				UnfocusKeyboard(CPU *theCPU) {if (GetFocusKeyboard()==theCPU || theCPU==NULL) FocusKeyboard(NULL);}
	inline void				UnfocusBackButton(CPU* theCPU) {mFocusBackButtonCPUList-=theCPU;} 
	inline void				UnfocusTouches(CPU *theCPU) {if (GetFocusTouches()==theCPU) FocusTouches(NULL);}
	inline void				UnfocusSpins(CPU *theCPU) {mFocusSpinsCPU.Unpush(theCPU);}
	inline void				UnfocusAccelerometer(CPU *theCPU) {if (GetFocusAccelerometer()==theCPU) FocusAccelerometer(NULL);}
	inline CPU*				GetFocusKeyboard() {return mFocusKeyboardCPU;}
	inline CPU*				GetFocusBackButton() {if (mFocusBackButtonCPUList.GetCount()) return (CPU*)mFocusBackButtonCPUList.Last();return NULL;}
	CPU*					GetFocusTouches(int theID=0);
	inline CPU*				GetFocusSpins() {if (mFocusSpinsCPU.GetStackCount()==0) return NULL;return mFocusSpinsCPU.Peek();}
	inline CPU*				GetFocusAccelerometer() {return mFocusAccelerometerCPU;}

	//inline CPU*				GetOverCPU() {return mOverCPU;}
	bool					IsCPUTouchFocused(CPU *theCPU);
	//
	// For multiple touch devices, we can focus touches by touchID. This
	// happens automatically in CPU's, so that a user can focus on more than one.
	//
	struct FocusTouchID
	{
		CPU*			mCPU;
		int				mID;
	};
	List					mFocusTouchesByIDList;
	void					FocusTouchesByID(int theID, CPU *theCPU);
	void					UnfocusTouchesByID(int theID);
	void					UnfocusTouchesByCPU(CPU *theCPU);
	//
	// Which CPU is being hovered over (simply which CPU
	// got the last TouchMove)
	//
	CPU*					mHoverCPU;
	void					SetHoverCPU(CPU *theCPU);
	inline void				UnsetHoverCPU(CPU *theCPU) {if (GetHoverCPU()==theCPU) SetHoverCPU(NULL);}
	inline CPU*				GetHoverCPU() {return mHoverCPU;}

	//
	// Allows you to intercept any time a CPU is clicked on (if you want to!)
	//
	virtual void			TouchedCPU(CPU* theCPU) {}

public:
	//
	// App Settings (not saved if it's a minimalist game)
	//
	Settings				mSettings;
	void					SaveSettings();	// Saves all the settings right now (need this for mobile, where app shutdown is not guaranteed in any way)
	bool					mSaveSettingsWhenMultitasking=true; // Put in for Chuzzle, which crashes a lot in SaveSettings when multitasking.  Possibly not safe for mobile?

public:
	//
	// List of every CPU in the game (can be used to do various things with)
	//
	List					mCPUList;

	//
	// You can isolate a CPU and make it the only visible and processed CPU.
	// This is useful if you want to put up a settings screen or something else
	// that completely obscures everything else.
	// 
	void					IsolateCPU(CPU* theCPU, bool touchesOnly=false);
	void					UnIsolateCPU(CPU* theCPU);
	CPUManager				*mIsolatedCPUManager;
	bool					mIsolateTouchesOnly;

	//
	// File helpers...
	//
	String					GetLoadFilename(String thePath, String theExtensions);
	String					GetSaveFilename(String thePath, String theExtensions);





public:
	//
	// Visibility helper: This is used to make sure that an onscreen point is visible
	// if the keyboard is up.  The Y is the y position that must be visible.
	//
	float					mVisibilityScroll;
	float					mVisibilityY;
	float					mVisibilityYExtraSpace;
    float                   mOnscreenKeyboardY;
    bool                    mOnscreenKeyboardChanged;
    inline bool             IsOnscreenKeyboardChanged() {return mOnscreenKeyboardChanged;}
    inline float            GetOnscreenKeyboardHeight() {return mOnscreenKeyboardY;}
    inline void				SetOnscreenKeyboardExtraSpace(float theSpace) {mVisibilityYExtraSpace=theSpace;}

	Stack<float>			mVisibilityYExtraSpaceStack;
	inline void				PushOnscreenKeyboardExtraSpace() {mVisibilityYExtraSpaceStack.Push(mVisibilityYExtraSpace);}
	inline void				PopOnscreenKeyboardExtraSpace() {if (!mVisibilityYExtraSpaceStack.IsEmpty()) mVisibilityYExtraSpace=mVisibilityYExtraSpaceStack.Pop();}

    
    void                    TranslateForVisibility();
    void                    UnTranslateForVisibility();
	void					RehupVisibilityScroll();

public:
	//
	// Managed language helper... since I rely on destructors so much, I will want to do
	// garbage collection whenever a CPU Manager is destroyed.  So, this is just a flag
	// to set to indicate that we should clear out garbage before the next update
	//
	bool					mGarbageCollect;
	inline void				GarbageCollect() {mGarbageCollect=true;}

public:
	// 
	// Clouding helpers... we can set up files to automatically cloud, if a clouding system exists.
	//
	bool					mAutoCloud=false;
	bool					mAskedAutoCloud=false;
	inline bool				DidAskAutoCloud() {return mAskedAutoCloud;}

	//
	// App will check if cloud data exists on startup, and set this boolean.
	//
	bool					DoesCloudDataExist();



public:

	//
	// Play Counter... this just lets us know how many times we've played.
	//
	int						mPlayCounter;
	inline int				GetPlayCounter() {return mPlayCounter;}
	inline bool				IsFirstGame() {return (mPlayCounter==0);}
	void					IncrementPlayCounter() {mPlayCounter++;}

	//
	// Startup counter (counts successful loads)
	//
	int						mStartups=0; 
	inline int				GetStarts() {return mStartups;}
	inline int				IsFirstStartup() {return mStartups<=1;}

    
    //
    // Just a flag that lets us know it's time to invoke the LoadComplete
    // functions.  We want these to happen in the main thread, thus the flag.
    //
	bool                    mRunLoadBackgroundComplete;

	//
	// Lets us update much faster-- for debugging, etc.
	//
	int						mUpdateMultiplier;

	//
	// Gets the unique APP ID (usually the machine's MAC Address or unique address + app name)
	//
	String					GetAppID();

	//
	// Tells us the current App's system state... i.e. what the system is doing
	//
	enum
	{
		SYSTEMSTATE_NULL=0,
		SYSTEMSTATE_STARTUP,
		SYSTEMSTATE_DRAWING,
		SYSTEMSTATE_UPDATING,

		SYSTEMSTATE_MAX
	};
	int						mSystemState;
	bool					IsDrawing() {return mSystemState==SYSTEMSTATE_DRAWING;}
	bool					IsUpdating() {return mSystemState==SYSTEMSTATE_UPDATING;}
	bool					IsStarting() {return mSystemState==SYSTEMSTATE_STARTUP;}

	//
	// Sometimes, we don't want the app to do multitasking.  This is here to prevent Multitasking from being called
	// if we don't want it to (this was explicitely put in because Android calls multitasking when ads are shown, which
	// causes problems).
	//
	// If you disable multitasking with a number, it will skip calling multitasking that many times.  I put this in because
	// Admob on Android forces an app stop/resume, which in turn ends up calling multitasking.  So this is basically here for
	// anything that might take focus from the app, when you don't want to process that.
	//
	inline void				DisableMultitasking(int theCount=-1) {mDisableMultitasking=theCount;}
	inline void				EnableMultitasking() {mDisableMultitasking=0;}
	int						mDisableMultitasking;

	//
	// Some helpers for overriding the back button...
	//
	void					OverrideBackButton(bool theState=true);

	//
	// The user's personalized save area (so that you can override, like: "profile:\\myfile.txt");
	//
	virtual String			GetStorageLocation(String thePrefix) {return thePrefix;}

	//
	// Draws the loading screen...
	//
	enum 
	{
		LOADINGSCREEN_LOADGRAPHICS=0x01,
		LOADINGSCREEN_UNLOADGRAPHICS=0x02,
	};

	virtual void			DrawLoadingScreen(int theFlag) {}	// TheFlag is one of the LOADINGSCREEN flags above...

	AssetLoader*			mLoader;
	virtual void			CustomLoad(String theID) {};		// For a custom load step, override.


	int						GetNowMonth(int thePlusDay=0);
	int						GetNowDay(int thePlusDay=0);
	int						GetNowYear(int thePlusDay=0);
	int						GetNowHour(int thePlusDay=0);
	int						GetNowMinute(int thePlusDay=0);
	int						GetNowWeekday(int thePlusDay=0);

	virtual int				DailySeed(int addDays=0);
	virtual int				MonthlySeed(int addDays=0);
	virtual int				NextWeekdaySeed(int theWeekday, bool allowTodayAsNext=true); // So like, ask for weekday, and it'll give you the seed of the NEXT one
	virtual void			DailySeedToMDY(int theSeed, int& theMonth, int& theDay, int& theYear);
	virtual int				DailySeedToInt(int theSeed); // Turns daily seed into a single int representing the date, like DateToInt
	char					mDailySeedEncoding=0; // 0 = YMD, 1 = YDM (for Chuzzle Snap)


};

extern App *gAppPtr;

//
// Thread helpers, so we don't need to include OS_Core...
//
#ifndef NO_THREADS
void Thread(void (*theFunction)(void *theArg), void* theArg=NULL);
#endif
int CreateThreadLock();
void KillThreadLock(int theLock);
void DeleteThreadLock(int theLock);
inline void DestroyThreadLock(int theLock) {KillThreadLock(theLock);}
inline void ReleaseThreadLock(int theLock) {KillThreadLock(theLock);}
void ThreadLock(int theLock);
void ThreadUnlock(int theLock);
void SetThreadPriority(float thePriority);
void Sleep(int theAmount);
void CopyToClipboard(String theText);
String PasteFromClipboard();

void ThrottleLock();
void ThrottleUnlock();

//
// Debugging functions... just to give us an easy way to track time of certain game things...
//
#ifdef _DEBUG
void StartTimeLog();
void ShowTimeLog(String theString="null");
#else
inline void StartTimeLog() {}
inline void ShowTimeLog(String theString="null") {}
#endif

inline int		AppTime() {if (gAppPtr) return gAppPtr->AppTime();return 0;}
inline float	AppTimeF() {if (gAppPtr) return gAppPtr->AppTimeF();return 0;}
extern bool gIgnoreNextTouchEnd; // So we can access it for special stuff

inline unsigned int		TicksToUpdates(unsigned int theTicks) {return (unsigned int)((theTicks/1000)*gAppPtr->mUpdatesPerSecond);}
inline float			TicksToUpdatesF(unsigned int theTicks) {return (float)(((float)theTicks/1000.0f)*(float)gAppPtr->mUpdatesPerSecond);}

class CallSoonObject : public Object {public:CallSoonObject() {mFunction=NULL;mArg=NULL;} public:void Update();void	(*mFunction)(void *theArg);void* mArg;};
inline void CallSoon(void (*theFunction)(void *theArg), void* theArg) {if (gAppPtr && !gAppPtr->IsQuit()) {CallSoonObject* aCS=new CallSoonObject;aCS->mFunction=theFunction;aCS->mArg=theArg;gAppPtr->AddBackgroundProcess(aCS);}}

//
// Text Translation
//
char* _TT(char* theText);

namespace RComm
{
	class RaptisoftQuery;
	typedef Smart(RaptisoftQuery) RQuery;
}

//
// Assetloader mFlag bits...
//
enum
{
	ASSETLOADER_KILL=0x01,
	ASSETLOADER_FORCETHREADED=0x02,
};
class AssetLoader : public CPU
{
public:
	AssetLoader() 
	{
		mLoadMax=-1;mFlag=App::LOADINGSCREEN_LOADGRAPHICS;
		#ifdef MULTICORE_LOAD
		mThreadLock=CreateThreadLock();
		#endif
	}
	#ifdef MULTICORE_LOAD
	virtual ~AssetLoader() {DeleteThreadLock(mThreadLock);}
	#endif

	float				mLoadMax;
	int					mFlag;
	bool				mReloading=false; // When true, don't reload anything that's instantiated...

	void				Initialize();
	void				Update();
	void				Draw();
	
	float				GetLoadComplete();
	String				GetStatus();

	#ifdef MULTICORE_LOAD
	inline void			Lock() {ThreadLock(mThreadLock);}
	inline void			Unlock() {ThreadUnlock(mThreadLock);}
	int					mThreadLock=-1;
	int					mThreadCount=0;
	int					mOldThreadCount=-100;
	float				mOldLoadComplete=-1;
	#else
	inline void			Lock() {}
	inline void			Unlock() {}
	#endif

public:
	enum
	{
		ASSET_CUSTOM=0,			//0
		ASSET_SPRITEBUNDLE,		//1
		ASSET_SOUND,			//2
		ASSET_MUSIC,			//3
		ASSET_SOUNDSTREAM,		//4
		ASSET_SOUNDLOOP,		//5
		ASSET_SOUNDLOOPREF,		//6
		ASSET_SPRITE,			//7
		ASSET_IMAGE,			//8
		//ASSET_CLOUDDOUBLECHECK,	// Double checks our connection
		ASSET_TEXTURE,			//11 Loads up a texture
		ASSET_RQUERY,			//12 Something that's downloading in the background...

		ASSET_NULL,				// Does nothing, we use it to pad our loader to look better...
	};

	struct Asset
	{
		Asset() {mTimeout=0;mPointer=NULL;mRefPointer=NULL;}
		Asset(int theStage) {mStage=theStage;mTimeout=0;mPointer=NULL;mRefPointer=NULL;}
		char								mType;
		void*								mPointer;
		void*								mRefPointer;
		String								mText;
		String								mDesc;
		int									mTimeout;
		void								(*mFunction)(String theText)=NULL;
		RComm::RQuery						mQuery;
		unsigned int						mStage=0;	// Loading tier (0-255, custom defaults to 255, everyone else defaults to 0)

		char								mFlag=0;

		inline Asset* SetDescription(String theDesc) {mDesc=theDesc;return this;}
		inline Asset* SetDesc(String theDesc) {mDesc=theDesc;return this;}
		inline Asset* AddDesc(String theDesc) {mDesc=theDesc;return this;}
		inline Asset* SetStage(int theTier) {mStage=theTier;return this;}
		inline Asset* Stage(int theTier) {mStage=theTier;return this;}
		inline Asset* Final() {mStage=255;return this;}

		//
		// ForceThreaded means nothing on a non-threaded load.  On a threaded load, it'll make the "custom asset" happen in a thread
		// instead of being pushed onto the main thread.  So if you have long customs that can happen concurrently, this is a way to
		// make it work.
		//
		inline Asset* ForceThreaded() {mFlag|=ASSETLOADER_FORCETHREADED;return this;}
		inline Asset* Threaded() {mFlag|=ASSETLOADER_FORCETHREADED;return this;}

	};
	SmartList(Asset)					mAssetList;
	SmartList(RComm::RaptisoftQuery)	mRQueryList;
	unsigned int						mAssetStage=0;
	unsigned int						mAssetStageCursor=0;

	#ifdef MULTICORE_LOAD
	Smart(Asset)						mRunMe;	// Run on the main thread (custom assets!)
	#endif

	Asset* Add(SpriteBundle* thePtr) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_SPRITEBUNDLE;aAsset->mPointer=thePtr;return aAsset.GetPointer();}
	Asset* Add(Sound* thePtr, String theText) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_SOUND;aAsset->mPointer=thePtr;aAsset->mText=theText;return aAsset.GetPointer();}
	Asset* Add(SoundStream* thePtr, String theText) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_SOUNDSTREAM;aAsset->mPointer=thePtr;aAsset->mText=theText;return aAsset.GetPointer();}
	Asset* Add(SoundLoop* thePtr, String theText) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_SOUNDLOOP;aAsset->mPointer=thePtr;aAsset->mText=theText;return aAsset.GetPointer();}
	Asset* Add(SoundLoop* thePtr, Sound* theSound) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_SOUNDLOOPREF;aAsset->mPointer=thePtr;aAsset->mRefPointer=theSound;return aAsset.GetPointer();}
	Asset* Add(Music* thePtr, String theText) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_MUSIC;aAsset->mPointer=thePtr;aAsset->mText=theText;return aAsset.GetPointer();}
	Asset* Add(String theText) {Smart(Asset) aAsset=new Asset(++mAssetStageCursor);mAssetStageCursor++;mAssetList+=aAsset;aAsset->mType=ASSET_CUSTOM;aAsset->mText=theText;return aAsset.GetPointer();}
	Asset* Add(Sprite* thePtr, String theText) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_SPRITE;aAsset->mPointer=thePtr;aAsset->mText=theText;return aAsset.GetPointer();}
	Asset* Add(Image* thePtr, String theText) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_IMAGE;aAsset->mPointer=thePtr;aAsset->mText=theText;return aAsset.GetPointer();}
	Asset* Add(RComm::RQuery theQuery, String theText) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;mRQueryList+=theQuery;aAsset->mType=ASSET_RQUERY;aAsset->mQuery=theQuery;aAsset->mText=theText;return aAsset.GetPointer();}
	Asset* AddTexture(int* thePtr, String theFilename) {Smart(Asset) aAsset=new Asset(mAssetStageCursor);mAssetList+=aAsset;aAsset->mType=ASSET_TEXTURE;aAsset->mPointer=thePtr;aAsset->mText=theFilename;return aAsset.GetPointer();}
	//	void AddCloud() {Smart(Asset) aAsset=new Asset;mAssetList+=aAsset;aAsset->mType=ASSET_CLOUDDOUBLECHECK;aAsset=new Asset;mAssetList+=aAsset;aAsset->mType=ASSET_CLOUD;}
	Asset* AddNull() {Smart(Asset) aAsset=new Asset(++mAssetStageCursor);mAssetStageCursor++;mAssetList+=aAsset;aAsset->mType=ASSET_NULL;return aAsset.GetPointer();}
};

#ifdef _RECORDER
void Playback(String theFilename);
#endif

//
// Helper for older games-- to make sure data gets moved over from legacy file locations...
// As of the time of this writing, it's only an issue on Android.
//
#ifdef LEGACY_GL
String RestoreLegacyFiles(bool evenIfMoved);
#endif



//
// Seeds for various events...
// GIMPSEED = minutes of seed
//
/*
#define GIMPSEED 5
WARNING("Gimped Daily Seeds Activated");
inline int DailySeed(int addDays=0) {int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;OS_Core::GetTime(addDays*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);return Sprintf("%.2d%.2d%.2d%.2d",aDay,aMon,aHour,aMinute/GIMPSEED).ToInt();}
inline int MonthlySeed(int addDays=0) {int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;OS_Core::GetTime(addDays*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);return Sprintf("%.2d%.2d.%2d",aMon,aHour,aMinute/GIMPSEED).ToInt();}
/**/
//*
inline int DailySeed(int addDays=0) {if (gAppPtr) return gAppPtr->DailySeed(addDays);int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;OS_Core::GetTime(addDays*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);return Sprintf("%.4d%.2d%.2d",aYear,aDay,aMon).ToInt();}
inline int MonthlySeed(int addDays=0) {if (gAppPtr) return gAppPtr->MonthlySeed(addDays);int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;OS_Core::GetTime(addDays*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);return Sprintf("%.4d%.2d",aYear,aMon).ToInt();}
/**/
inline int GetDailySeed(int addDays=0) {return DailySeed(addDays);}
inline int GetMonthlySeed(int addDays=0) {return MonthlySeed(addDays);}

#ifdef _DEBUG
void RandomizeDailySeed();
void FakeSeason(String theSeason);
#endif

String GetSeason();
bool IsSeason(String theSeason);
inline bool IsHolidaySeason(String theSeason) {return IsSeason(theSeason);}
inline char* GetHolidaySeason() {return GetSeason();}
bool IsHoliday(String theHoliday);
bool IsWithinDaysBefore(int theMon, int theDay, int theYear, int theDaysBefore); // If "today" is within theDaysBefore of date...
bool IsToday(int theMon, int theDay, int theYear); // If this date is today
int DateToInt(int theMonth=-1, int theDay=-1, int theYear=-1);

bool WasProfanity();
String ProfanityFilter(String theText, String replaceWith="***", bool replaceWholeWord=true);
bool IsProfanity(String theText);

void SecureLog(char *theText, ...);	// Hard logging to a file called log.txt in sandbox...

void* OSCore_Query(String theQuery);
void* TransactionCore_Query(String theQuery);
void* GraphicsCore_Query(String theQuery);	
inline void* OCore_Query(String theQuery) {return OSCore_Query(theQuery);}
inline void* TCore_Query(String theQuery) {return TransactionCore_Query(theQuery);}
inline void* GCore_Query(String theQuery) {return GraphicsCore_Query(theQuery);}


#define Crash() {int* aInt=NULL;*aInt=25;}
#ifdef _DEBUG
#define DebugCrash() {Crash();}
#else
#define	DebugCrash() {}
#endif

//
// Lets us look at how the core is evaluating a file path (i.e. sandbox://)
//
String PeekPath(String thePath);
String ResolvePath(String thePath);
#ifdef _DEBUG
extern bool gWatching;
#endif


