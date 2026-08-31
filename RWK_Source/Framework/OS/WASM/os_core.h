#pragma once
#include "os_headers.h"
#pragma comment (lib,"winmm.lib") 

#include "util_core.h"
namespace OS_Core
{

	//
	// First helpers, to make sure you can output return codes and whatnot...
	//
	void			Error(char *format, ...);				// Helper function.  Remove in final.
	void			Printf(char *format, ...);
	bool			Output(char *theString);

	//
	// Starts up and shutdowns the system.  Does whatever needs to be done
	// on the OS to start handling input and whatnot...
	//
	void			Startup();								// Start OS system stuff (non-threaded)
	void			Shutdown();								// Shut down OS system stuff
	void			CatchCrashes(char* theURL);				// Allows us to catch crashes and report to a url


	//
	// You need to call this more or less constantly, every update, to pump
	// the OS.  This handles things like input messages or whatever else the OS
	// needs to keep running.
	//
	void			Pump();						// Yield to any processing the OS needs to do

	//
	// This just tells you if the OS has signalled that it's time to shutdown,
	// such as by closing the window or terminating the app.  If this is true,
	// you should immediately shut down the program.
	//
	void			RequestShutdown();			// Requests shutdown from outside (i.e. by pressing a quit button in-game)
	bool			WantShutdown();				// Just a way to query the core to see if shutdown is requested

	//
	// We take the App name and Publisher here because we use it to
	// set some system variables, like sandbox location and registry information.
	//
	void			SetAppName(char *theName);
	void			SetAppVersion(char *theVersion);
	void			SetPublisher(char *theName);

	void			SetWindowName(char *theName); // Lets us set the name of the window without modifying stuff
	void			SetWindowAsTool();				// Makes the window a tool window

	//
	// Some functions to make the App go into the tray, etc.
	// These are all Win32 ONLY and should not be made part of frameworks, etc.
	//
	void			MakeTrayApp(char* theCloseMenuText);
	void			SetTrayTooltip(char* theTooltip);
	void			SetTrayBubble(char* theTitle, char* theText);

	//
	// This is just a way to get info out of the core without making a specific
	// function for every single piece of information we need.  For instance, I use
	// this on Windows with DirectX to get the Window Handle.
	//
	void*			Query(char *theInfo, void* extraInfo=NULL);	
	char*			QueryString(char* theQuery, void* extraInfo=NULL);
	void			QueryBytes(char* theQuery, void** thePtr, int* theSize);
	void*			Query(longlong theHash,void* extraInfo=NULL);

	
	inline bool		HasCloud() {return false;}
	inline char		GetCloudStatus() {return -1;}
	inline bool		PutCloud(char* theKey, void* thePtr, int theSize, bool waitForResults=false) {return false;}
	inline bool		GetCloud(char* theKey, void** thePtr, int* theSize) {return false;}
	inline void		DeleteCloud(char* theKey) {}
	inline void		EnumCloud(Array<char*>& theArray) {theArray.Reset();}
	inline bool		IsCloudEmpty() {return true;}
	
	

	//
	// Gets system time, in milliseconds.
	//
	unsigned int	Tick();			


	//
	// Gets some folders that we'll use frequently.  
	// Desktop = user's desktop
	// Sandbox = where you should store all user data, save games, etc, etc.
	// Package = where the game's graphics, sounds, etc, are.
	//
	void			GetDesktopFolder(char *theResult);
	void			GetSandboxFolder(char *theResult);
	void			GetCloudFolder(char *theResult);
	void			GetPackageFolder(char *theResult);
	void			GetCacheFolder(char *theResult);
	void 			GetTempFolder(char *theResult);
	void 			LocationAlias(char* theFolder, char* theAlias);
	//
	// File handling... since we've found that we can't trust all
	// file functions to be portable.  For the purposes of portability,
	// files are dumped into memory, open file access is not allowed.
	//
	bool			DoesFileExist(char *theFilename);
	char*			ReadFile(char *theFilename, int *theBytesRead);
	char*			ReadPartialFile(char *theFilename, int theBytesToRead);
	bool			WriteFile(char *theFilename, char *theDataPtr, int theLen);
	void			MakeDirectory(char *theDirectoryName);
	void			DeleteDirectory(char *theDirectoryName);
	void			DeleteFile(char *theFilename);
	bool			RenameFile(char* theOldName, char* theNewName);
	void			EnumDirectory(char *theDirectoryName, Array<char *>&theArray, bool includeSubdirs);
	bool			IsDirectory(char* theFilename);	// Tells if it's a file or dir...
	void			SecureLog(char* theFilename, char* theData);	// Appends text to a file for logging purposes
	
	//
	// Tells you the version of the OS, if you need it.
	//
	inline char*	GetOSName() {return "WASM";}
	void			GetOSVersion(char *theResult);
	inline char*	GetOSQuarantine() {return "";}
										// This is used to tag our system for settings that need to be specific to the system.
										// An example: When SteamCloud is used, you might end up getting your resolution swapped
										// with, say, the Steamdeck.  So we want to tag our settings file with like, "resolution.linux.steamdeck" or
										// "resolution.fullscreen.win32.steam" for such situations.  Also use this on mobile to lock them into a resolution.

	inline bool		IsTouchDevice() {return false;}
	inline void		PopupKeyboard(bool doShow) {}
	

	//
	// Controls for getting/setting the command line.
	// Setting just stores it here so that Get can get it.
	//
	void			SetCommandLine(char *theCMD);	
	char*			GetCommandLine();


	//
	// Sets the resolution of the App.  On most devices, this will do nothing,
	// it's only here for devices capable of multiple resolutions, like Win/Mac.
	//
	void			SetResolution(int theWidth, int theHeight, bool isFullscreen);
	void			GetResolution(int *theWidth, int *theHeight);

	//
	// System message stuff... contains input data and other pertinent
	// messages that come from the system.  Passes on:
	//			
	//		o Mouse/Touch motion
	//		o Accelerometer (for mobile devices)
	//		o Suspend/Resume
	//		o App Termination
	//		o Dropped files (for desktop devices)
	//		o Keyboard Input
	//
	enum 
	{
		OS_NULL=0,
		OS_TOUCHMOVE,			// mData[0=XPos, 1=YPos]
		OS_TOUCHSTART,			// mData[0=XPos, 1=YPos, 2=Mouse Button]
		OS_TOUCHEND,			// mData[0=XPos, 1=YPos, 2=Mouse Button]
		OS_DROPFILE,			// mData[0=char * pointer to filename (needs to be released)]
		OS_QUIT,				// (No data)
		OS_KEYDOWN,				// mData[0=key pressed] 
		OS_KEYUP,				// mData[0=key pressed]
		OS_CHAR,				// mData[0=character typed]
		OS_ACCELEROMETER,		// mData[0=XTip, 1=YTip, 2=ZTip]
		OS_MULTITASKING,		// mData[0=state] (true = go to foreground, false = go to background)
		OS_REFRESHTOUCHES,		// Just refreshes the touch position
		OS_SPIN,				// mData[0=direction of spin] (Mousewheel)
		OS_SCREENSHOT,			// (No data) ... user pressed screenshot key.  This is to fix multithreading issues
		OS_SWAPFULLSCREEN,		// (No data) ... the user has requested to swap between windowed and fullscreen
		OS_OBSCURESCREENBOTTOM,	// Implemented on portable devices only
		OS_SYSTEMNOTIFY,		// Something big changed system-wide (like resolution, etc)
		OS_BACK,				// Back button was pressed
		OS_REBOOT,				// App needs to reboot (calls gApp.Reboot)... put in for iOS Deep Links
		OS_MISC,				// Misc event... 

		OS_NOMESSAGE
	};
	struct OS_MessageData
	{
		int				mMessage;		// The message itself
		int				mData[3];		// The data to go with the message
		void*			mDataPtr;		// Data Pointer
		int				mID;			// System hash for this message (for instance, on touch devices, each touch gets its own ID so you can track them)
	};
	bool			GetFirstOSMessage(OS_MessageData *theData);
	bool			GetNextOSMessage(OS_MessageData *theData);
	void			PushOSMessage(int theMessage, int theID=0, int theData1=0, int theData2=0, int theData3=0, void* theDataPtr=NULL);
	void			PushOSMessageNoThread(int theMessage, int theID=0, int theData1=0, int theData2=0, int theData3=0, void* theDataPtr=NULL);
	//
	// Tells us if we're a touch device or not (this would affect how we'd handle
	// mouseovers and whatnot)
	//
	bool			IsTouchDevice();

	//
	// For touch devices, pops up (or down) a keyboard
	//
	void			PopupKeyboard(bool doShow);

	//
	// Portable Sleep (mostly for cocoa, which doesn't have a proper sleep)
	//
	void			Sleep(int theAmount);

	//
	// For Thread blocking... the system runs in one thread, the app itself in another...
	// In the client, you should ifdef out the whole function if the system isn't running in a thread.
	//
	void			LockSystemThread(bool theState);

	//
	// OS Specific message boxes...
	//
	enum
	{
		MESSAGEBOX_OK=0,
		MESSAGEBOX_YESNO=1,
		MESSAGEBOX_YESNOCANCEL=2,

		MESSAGEBOX_MAX
	};
	int				MessageBox(char *theCaption, char *theText, int theType=0);

	//
	// Some devices (iOS) will not run the app in the background at all, so the sleep
	// routine in rapt_app doesn't need to process.  Other systems (Win/Mac) will run
	// the app, so it needs to go into the rapt_app sleep routine to make it not take
	// up runtime.  This determines whether you need to process in slo-mo in the
	// background or not.
	//
	bool			WillAppProcessInBackground();

	//
	// This lets you get the user name on the device...
	//
	char*			GetLocalName();

	//
	// Opens a URL...
	//
	bool			Execute(char *theCommand);

	//
	// Sets mouse cursor
	//
	enum
	{
		SYSCURSOR_POINTER=0,
		SYSCURSOR_IBEAM,
		SYSCURSOR_FINGER,
		SYSCURSOR_BLANK,
		SYSCURSOR_GRABHAND,
		SYSCURSOR_FINGER2,
		SYSCURSOR_DRAG,
		SYSCURSOR_DRAGV,
		SYSCURSOR_DRAGH,
		SYSCURSOR_MAX
	};
	void			SetCursor(int theCursor);
	void			SetCursorPos(int x, int y);


	//
	// Runs a function in a thread
	// Priority goes from -1 to 1, with zero being normal,
	// -1 being low priority, and 1 being high priority
	//
	inline bool		AllowThreads() {return false;}
	void			Thread(void (*theFunction)(void *theArg), void* theArg=NULL);
	void			SetThreadPriority(float thePriority);

	//
	// Local system filename save/load
	// TheExtensions is a comma separated list of extensions like this: text=*.txt,config=*.cfg,executable=*.exe
	//
	char*			GetFilename(char* thePath, char* theExtensions, bool isLoading);

	//
	// Windows needs to draw when not in foreground, so...
	// Determine if your OS can draw when behind things (typically, a windowed OS does)
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
	inline bool		CanBackgroundDraw() {return true;} 

	//
	// To get a unique ID for the machine...
	//
	void			GetAppID(char* theResult);

	//
	// Lets us get time and date, portably...
	//
	void			GetTime(int addTime, int* theHour, int *theMinute, int* theSecond,int* theMonth, int* theDay, int* theYear, int* theWeekday=NULL);
	inline void		GetTime(int* theHour, int *theMinute, int* theSecond,int* theMonth, int* theDay, int* theYear, int* theWeekday=NULL) {GetTime(0,theHour,theMinute,theSecond,theMonth,theDay,theYear,theWeekday);}

	//
	// Creates thread locks...
	//
	int				CreateThreadLock();
	void			ReleaseThreadLock(int theLock);
	void			ReleaseAllThreadLocks();
	void			ThreadLock(int theLock);
	void			ThreadUnlock(int theLock);

	//
	// Gives a name for WAD files... (Android requires MP3)
	// This only should matter on Android, but we'll see!
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
	inline char*	GetWADName() {return "game.wad";}

	//
	// For copy/paste
	//
	void			CopyToClipboard(char* theString);
	char*			PasteFromClipboard();

	//
	// Fixes slashes in paths.
	//
	char*			FixPath(char *path);

	//
	// Just to minimize the app
	//
	void			Minimize();

	//
	// Show or hide the system cursor, on PCs
	//
	void			ShowCursor(bool theState);
	
	//
	// Callbacks...
	//
	void			RegisterCallback(int theID, bool (*theFunction)(void *theArg)=NULL);
	
	//
	// WASM exclusive:
	// For persistent storage
	//
	void StartPersistentStorage();
	void SetPersistanceStorageSynced();
	bool IsPersistantStorageSynced();
	void UpdatePersistantStorage();
	
	//
	// Archive function... simply puts an array of filenames into an archive
	// "Root" is the root folder that gets removed from the filenames in the zip file...
	//
	void			Archive(char* theRootFolder, Array<char*>& theFiles, char* theZipFilename, float* theProgress=NULL);
	void			UnArchive(char* theRootFolder, char* theZipFilename, float* theProgress=NULL);
}

namespace CrashHandler
{
   inline void    SetExtendedInfo(char* theInfo) {}
};
