#include <sys\types.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include "os_core.h"
#include <winuser.h>
#include <tchar.h>
#include <shellapi.h>
#include <shlobj.h>

#include "graphics_core.h"

#define __HEADER
#include "..\common.h"
#undef __HEADER

#define FAKECLOUD


namespace OS_Core
{
	//
	// Put port specific stuff here at the top...
	//
	::SDL_Window* gWindow=NULL;
	int gCurrentCursor=0;
	HCURSOR gCursor[SYSCURSOR_MAX];
	int gTouchHash=0;
	bool gFPSMouse=false;

	int gResolutionWidth=800;
	int gResolutionHeight=600;
	bool gFullscreen=false;
	bool gLegacySandbox=false;

	int gCriticalThreadCount=0; // We don't leave until this is zero (we do stop processing)


	//
	// Stuff that is very common across all OS's... only changes here might be for syntax or language 
	// variability.
	//
	bool gWantShutdown=false;	// Whether the app has requested shutdow (i.e. stop pumping)

	//
	// Messaging helpers, for outputting messages while porting...
	//
	void Error(char *format, ...) {char aString[2048];va_list argp;va_start(argp, format);vsprintf_s(aString,2048,format, argp);va_end(argp);Output(aString);exit(0);}
	void Printf(char *format, ...) {char aString[10000];va_list argp;va_start(argp, format);vsprintf_s(aString,10000,format, argp);va_end(argp);Output(aString);}
	bool WantShutdown() {return OS_Core::gWantShutdown;}

	//
	// OS Messages... conversion from native to a portable format that RAPT can process/read
	//
	Array<OS_MessageData>	gOSMessageQueue;
	int						gOSMessageMax=0;
	int						gOSMessageCursor=0;

	bool GetFirstOSMessage(OS_MessageData *theData) {gOSMessageCursor=0;return GetNextOSMessage(theData);}
	bool GetNextOSMessage(OS_MessageData *theData)	{if (gOSMessageCursor>=gOSMessageMax) {gOSMessageMax=0;gOSMessageCursor=0;memset(theData,0,sizeof(OS_MessageData));return false;} memcpy(theData,&gOSMessageQueue[gOSMessageCursor++],sizeof(OS_MessageData)); return true;}
	void PushOSMessage(int theMessage, int theID, int theData1, int theData2, int theData3, void* theDataPtr) {LockSystemThread(true);gOSMessageQueue[gOSMessageMax].mMessage=theMessage;gOSMessageQueue[gOSMessageMax].mID=theID;gOSMessageQueue[gOSMessageMax].mData[0]=theData1;gOSMessageQueue[gOSMessageMax].mData[1]=theData2;gOSMessageQueue[gOSMessageMax].mData[2]=theData3;gOSMessageQueue[gOSMessageMax].mDataPtr=theDataPtr;gOSMessageMax++;LockSystemThread(false);}
	void PushOSMessageNoThread(int theMessage, int theID, int theData1, int theData2, int theData3, void* theDataPtr) {gOSMessageQueue[gOSMessageMax].mMessage=theMessage;gOSMessageQueue[gOSMessageMax].mID=theID;gOSMessageQueue[gOSMessageMax].mData[0]=theData1;gOSMessageQueue[gOSMessageMax].mData[1]=theData2;gOSMessageQueue[gOSMessageMax].mData[2]=theData3;gOSMessageQueue[gOSMessageMax].mDataPtr=theDataPtr;gOSMessageMax++;}

	char gAppName[MAX_PATH];
	char gAppVersion[MAX_PATH];
	char gAppPublisher[MAX_PATH];
	char gBundlePath[MAX_PATH];
	void SetAppVersion(char *theVersion) {strcpy_s(gAppVersion,MAX_PATH,theVersion);}
	void SetPublisher(char *theName) {strcpy_s(gAppPublisher,MAX_PATH,theName);}
	void OS_Core::GetPackageFolder(char *theResult) {strcpy(theResult,gBundlePath);}

	char gCommandLine[MAX_PATH];
	void OS_Core::SetCommandLine(char *theCMD) {strcpy(gCommandLine,theCMD);}
	char* OS_Core::GetCommandLine() {return gCommandLine;}

	char* gUsername=NULL;


	bool gBackButtonOverride=true;	// Whether ESC will send OS_BACK or not...
	int gReserveTopScreen=0;		// For faking a notch (Win32 only)
	int gReserveBottomScreen=0;		// For faking a notch (Win32 only)
}

char *QuickString(char *theString)
{
	char *aResult=new char[strlen(theString)+1];
	strcpy(aResult,theString);
	return aResult;
}

bool OS_Core::Output(char *theString) // PORT_STARTUP
{
	//
	// However you want to dump debug info/messages, on whatever system
	// 
	printf(theString);
	return true;
}

void OS_Core::LockSystemThread(bool theState) // Port 2
{
	//
	// IF this system runs in a thread (iOS), then this function should lock that thread.
	// If it doesn't run in a thread, just leave it blank.
	//
	// #ifdef _PORT_PUMP
	// Error!
	// #endif
	//
}

//_PORT_PUMP Remove this...
void CreateCursors()
{
	OS_Core::gCursor[OS_Core::SYSCURSOR_POINTER]=LoadCursor(NULL,IDC_ARROW);
	OS_Core::gCursor[OS_Core::SYSCURSOR_IBEAM]=LoadCursor(NULL,IDC_IBEAM);
	OS_Core::gCursor[OS_Core::SYSCURSOR_GRABHAND]=LoadCursor(NULL,IDC_HAND);
	OS_Core::gCursor[OS_Core::SYSCURSOR_BLANK]=NULL;//LoadCursor(NULL,IDC_NO);
	OS_Core::gCursor[OS_Core::SYSCURSOR_FINGER]=NULL;//LoadCursor(NULL,IDC_NO);
	OS_Core::gCursor[OS_Core::SYSCURSOR_FINGER2]=NULL;//LoadCursor(NULL,IDC_NO);
	OS_Core::gCursor[OS_Core::SYSCURSOR_DRAG]=LoadCursor(NULL,IDC_SIZEALL);
	OS_Core::gCursor[OS_Core::SYSCURSOR_DRAGV]=LoadCursor(NULL,IDC_SIZENS);
	OS_Core::gCursor[OS_Core::SYSCURSOR_DRAGH]=LoadCursor(NULL,IDC_SIZEWE);


	//
	// This allows us to make cursors specific to our app.
	// The framework creates a hand cursor and a blank cursor as a matter
	// of course.
	//
	// And/Xor masks to use:
	//
	// 0 0 Black  
	// 0 1 White  
	// 1 0 Screen  
	// 1 1 Reverse screen  
	//
	unsigned char aANDMask[32*32];
	unsigned char aXORMask[32*32];
	memset(aANDMask,255,sizeof(aANDMask));
	memset(aXORMask,0,sizeof(aXORMask));
	OS_Core::gCursor[OS_Core::SYSCURSOR_BLANK]=CreateCursor(GetModuleHandle(0),16,16,32,32,aANDMask,aXORMask);	// CURSOR_BLANK

	//HotSpot: 15 10
	//Size: 32 32
	unsigned char aDragData[]= 
	{
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0, 
		0x01, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 
		0xe0, 0x00, 0xff, 0xfe, 0x60, 0x00, 0xff, 0xfc, 0x20, 0x00, 0xff, 0xfc, 0x00, 0x00, 0xff, 
		0xfe, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 
		0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xf0, 
		0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
		0x80, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x0d, 0xb0, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00, 
		0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00, 
		0x01, 0x8d, 0xb6, 0x00, 0x01, 0xcf, 0xfe, 0x00, 0x00, 0xef, 0xfe, 0x00, 0x00, 0xff, 0xfe, 
		0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x1f, 
		0xfc, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 
		0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00
	};
	OS_Core::gCursor[OS_Core::SYSCURSOR_GRABHAND]=CreateCursor(GetModuleHandle(0),15,10,32,32,aDragData,aDragData+(sizeof(aDragData)/2)); // CURSOR_HAND

	if (!OS_Core::gCursor[OS_Core::SYSCURSOR_FINGER])
	{
		//HotSpot: 11 4
		//Size: 32 32
		unsigned char aFingerData[]=
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
			0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xc3, 
			0xff, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 
			0xc0, 0x07, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xfc, 0x40, 0x01, 0xff, 0xfc, 0x00, 0x01, 0xff, 
			0xfc, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0x00, 0x01, 
			0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xc0, 
			0x03, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 
			0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 
			0x18, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x1b, 0x60, 0x00, 0x00, 0x1b, 0x68, 0x00, 
			0x00, 0x1b, 0x6c, 0x00, 0x01, 0x9f, 0xec, 0x00, 0x01, 0xdf, 0xfc, 0x00, 0x00, 0xdf, 0xfc, 
			0x00, 0x00, 0x5f, 0xfc, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x3f, 
			0xf8, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 
			0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00
		};
		OS_Core::gCursor[OS_Core::SYSCURSOR_FINGER]=CreateCursor(GetModuleHandle(0),11,4,32,32,aFingerData,aFingerData+(sizeof(aFingerData)/2)); // CURSOR_FINGER if it wasn't in the OS
	}
}

void OS_Core::Startup()	// PORT_STARTUP
{
	gCommandLine[0]=0;
	gAppName[0]=0;
	gAppVersion[0]=0;
	gAppPublisher[0]=0;
	gBundlePath[0]=0;
	
	//
	// Initializes everything you need... start video/audio system,
	// create windows, whatever.  On mobile this will often by done by the
	// mobile framework itself, and can be blank.
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif

	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_EVENTS)!=0) Error("Failed to start SDL");

	SDL_DisplayMode aMode;
	if (SDL_GetDesktopDisplayMode(0,&aMode)!=0) OS_Core::Error("Could not get desktop mode: [%s]",SDL_GetError());

	Graphics_Core::PreStartup();


//	gWindow=SDL_CreateWindow("",(aMode.w/2)-(gResolutionWidth/2),((aMode.h/2)-(gResolutionHeight/2))-50,gResolutionWidth,gResolutionHeight,SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);//SDL_WINDOW_HIDDEN util we set resolution!
	gWindow=SDL_CreateWindow("",(aMode.w/2)-(gResolutionWidth/2),25,gResolutionWidth,gResolutionHeight,SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);//SDL_WINDOW_HIDDEN util we set resolution!
	if (!gWindow) Error("Error:SDL_CreateWindow -> %s",SDL_GetError());

	//
	// The game will need a pointer to the path where data is stored.
	// So you should fill gBundlePath with the data here...
	//
	//
	// *** This is how to do it on Windows:
	// char aPackageDir[MAX_PATH];
	// GetModuleFileName(NULL,aPackageDir,MAX_PATH);
	// for (int aCount=strlen(aPackageDir);aCount>0;aCount--) if (aPackageDir[aCount]=='\\') {aPackageDir[aCount]=0;break;}
	// strcat(aPackageDir,"\\");
	// strcpy(gBundlePath,aPackageDir);
	//
	// *** This is how to do it with Apple's Cocoa or whatever:
	// CFURLRef aAppURL=CFBundleCopyBundleURL(CFBundleGetMainBundle());
	// CFURLRef aParentURL=CFURLCopyAbsoluteURL(aAppURL);
	// UInt8 aPath[MAX_PATH];
	// CFURLGetFileSystemRepresentation(aParentURL, TRUE,aPath,PATH_MAX);
	// strcat((char*)aPath,"/Contents/Resources/");
	// strcpy((char*)gBundlePath,(char*)aPath);
	//
	//#ifdef _PORT_DIRECTORIES
	//strcpy(gBundlePath,!Error); // You need to copy the Bundle's path to gBundlePath!
	//#endif
	//

	//
	// Remove this on cleanup!
	//
	
	char aPackageDir[MAX_PATH];
	GetModuleFileNameA(NULL,aPackageDir,MAX_PATH);
	for (int aCount=strlen(aPackageDir);aCount>0;aCount--) if (aPackageDir[aCount]=='\\') {aPackageDir[aCount]=0;break;}
	strcat_s(aPackageDir,MAX_PATH,"\\");
	strcpy_s(gBundlePath,MAX_PATH,aPackageDir);
	CreateCursors();

	SDL_version linked;
	SDL_GetVersion(&linked);
	OS_Core::Printf("SDLVersion = %u.%u.%u.",linked.major,linked.minor,linked.patch);
}

void OS_Core::Shutdown() // PORT_STARTUP
{
	//
	// Whatever you need to release everything and bring it all down.
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif

	SDL_DestroyWindow(gWindow);
	ReleaseAllThreadLocks();
	SDL_Quit();
	while (gCriticalThreadCount>0) Sleep(100);
}

#include "include/SDL_syswm.h"
void* OS_Core::Query(char *theInfo, void* extraInfo)
{
	//
	// A general purpose function that lets you get info out of the core for very specific system
	// requirements (example: on Windows, for certain things, you might need HWND, and this would
	// provide it).  Other cores can communicate with OS this way too.  Most significantly is
	// that the "graphics core" will probably query this to get the window or main delegate or
	// context or whatever.
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
	if (strcmp(theInfo,"WINDOW")==0) return (void*)gWindow;
	else if (strcmp(theInfo,"HWND")==0)
	{
		static struct ::SDL_SysWMinfo aInfo;
		SDL_VERSION(&aInfo.version);
		SDL_GetWindowWMInfo(gWindow,&aInfo);
		return (void*)&aInfo.info.win.window;
	}
	else if (_strcmpi(theInfo,"backbutton_override")==0) gBackButtonOverride=true;
	else if (_strcmpi(theInfo,"backbutton_normal")==0) gBackButtonOverride=false;
	else if (_strnicmp(theInfo,"rate:",5)==0) {OS_Core::Printf("Request Rating...");}
	else if (_stricmp(theInfo,"glrecover:?")==0) return (void*)1;
	else if (_stricmp(theInfo,"glrecover:on")==0) {OS_Core::Printf("GLRECOVER is on...");}
	else if (_stricmp(theInfo,"glrecover:off")==0) {OS_Core::Printf("GLRECOVER is off...");}
	else if (_strnicmp(theInfo,"fakephone:",10)==0) {return Graphics_Core::Query(theInfo);}
	else if (_strnicmp(theInfo,"fakeweb:",8)==0) {return Graphics_Core::Query(theInfo);}
	else if (_stricmp(theInfo,"RESERVESCREEN_TOP")==0) {return (void*)gReserveTopScreen;}
	else if (_stricmp(theInfo,"RESERVESCREEN_BOTTOM")==0) {return (void*)gReserveBottomScreen;}
	else if (_stricmp(theInfo,"LEGACYSANDBOX")==0) {gLegacySandbox=true;}
	else if (_stricmp(theInfo,"CLOUD_IN_SANDBOX")==0) {return (void*)gLegacySandbox;}
	else if (_stricmp(theInfo,"CRITICAL++")==0) {gCriticalThreadCount++;}
	else if (_stricmp(theInfo,"CRITICAL--")==0) {gCriticalThreadCount--;}
	else if (_stricmp(theInfo,"FPSMouse:on")==0) {SDL_SetRelativeMouseMode(SDL_TRUE);gFPSMouse=true;}
	else if (_stricmp(theInfo,"FPSMouse:off")==0) {SDL_SetRelativeMouseMode(SDL_FALSE);gFPSMouse=false;}

	return NULL;
}

char* OS_Core::QueryString(char* theQuery, void* extraInfo)
{
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
	if (_stricmp(theQuery,"core:version")==0) return "Win32 Version";
	return "";
}

void OS_Core::QueryBytes(char* theQuery, void** thePtr, int* theSize)
{
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
}


void OS_Core::CatchCrashes(char* theURL)
{
	//
	// Probably will not be used... but if you have a way to catch the crash address to apply to a map
	// file institute it here.  The URL is there for you to save for a place to post the crashes.
	//
}
	
void OS_Core::Pump()	// PORT_PUMP
{
	//
	// Main loop... this just gets called over and over again to pump messages.
	// Whatever your port requires.  In a theaded app, (for instance, iOS) pump
	// will likely be blank, in favor of just letting the add update/draw constantly,
	// and filling the OS messages from the mobile API (touchstart, etc).
	//
	// These events need to be taken from the porting system and passed into the core:
	//
	//	OS_MULTITASKING,		// mData[0=state] (true = go to foreground, false = go to background)
	//	OS_TOUCHMOVE,			// mData[0=XPos, 1=YPos]
	//	OS_TOUCHSTART,			// mData[0=XPos, 1=YPos, 2=Mouse Button]
	//	OS_TOUCHEND,			// mData[0=XPos, 1=YPos, 2=Mouse Button]
	//	OS_SPIN,				// mData[0=direction of spin] (Mousewheel)
	//	OS_KEYDOWN,				// mData[0=key pressed] 
	//	OS_KEYUP,				// mData[0=key pressed]
	//	OS_CHAR,				// mData[0=character typed]
	//	OS_ACCELEROMETER,		// mData[0=XTip, 1=YTip, 2=ZTip] (if supported)
	//	OS_DROPFILE,			// mData[0=char * pointer to filename (needs to be released)]
	//
	// Don't forget, for OS_TOUCHMOVE, OS_TOUCHSTART, and OS_TOUCHEND,you want to call
	// Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos) to convert to the new dimensions
	// in case of stretching!
	//
	// #ifdef _PORT_PUMP
	// Error!
	// #endif
	//

	//
	// Easy: Just make it so the system responds to the quit messages.
	// This just means setting things up so that gWantShutdown goes true
	// when the local OS says quit (usually a response to an event like
	// WM_CLOSE or SDL_QUIT)
	//
	// #ifdef _PORT_QUITTING
	// Error!
	// #endif
	//
	// #ifdef _PORT_TOUCHMOVES
	// Error!
	// #endif
	//
	// #ifdef _PORT_TOUCHES
	// Error!
	// #endif
	//
	// #ifdef _PORT_MOUSEWHEEL
	// Error!
	// #endif
	//
	// #ifdef _PORT_KEYEVENTS
	// Error!
	// #endif
	//
	// #ifdef _PORT_ACCEL
	// Error!
	// #endif
	//
	// #ifdef _PORT_DROPFILE
	// Error!
	// #endif
	//

	SDL_Event aEvent;
	while ( SDL_PollEvent (&aEvent) ) 
	{
		switch (aEvent.type) 
		{
			/*
		case SDL_JOYDEVICEADDED:
			OS_Core::Printf("!JOYDEVICEADDED");
			break;
			*/
		case SDL_JOYDEVICEREMOVED:
			OS_Core::Printf("!JOYDEVICEREMOVED");
			break;
		case SDL_QUIT:gWantShutdown=true;break;
		case SDL_WINDOWEVENT:
			switch (aEvent.window.event)
			{
			case SDL_WINDOWEVENT_FOCUS_GAINED:PushOSMessage(OS_MULTITASKING,0,true);break;
			case SDL_WINDOWEVENT_FOCUS_LOST:PushOSMessage(OS_MULTITASKING,0,false);break;
			case SDL_WINDOWEVENT_MOVED:PushOSMessage(OS_MISC,0);break;
			}
			break;
		case SDL_MOUSEMOTION:
			{
				if (aEvent.button.which==SDL_TOUCH_MOUSEID) break; // This disables a touch device's

				int aXPos;
				int aYPos;

				if (!gFPSMouse)
				{
					aXPos=aEvent.motion.x;
					aYPos=aEvent.motion.y;
					Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				}
				else
				{
					aXPos=aEvent.motion.xrel;
					aYPos=aEvent.motion.yrel;
				}

				//
				// Here we convert resolution to the actual page's resolution!
				// Zoomed screens should be mapped identically.
				//
				// #ifdef _PORT_RESOLUTION
				// Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				// !ERROR
				// #endif
				PushOSMessage(OS_TOUCHMOVE,gTouchHash,aXPos,aYPos);				
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			{
				SDL_CaptureMouse(SDL_TRUE);
				
				int aXPos=aEvent.button.x;
				int aYPos=aEvent.button.y;
				//
				// Here we convert resolution to the actual page's resolution!
				// Zoomed screens should be mapped identically.
				//
				// #ifdef _PORT_RESOLUTION
				// Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				// !ERROR
				// #endif
				Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);

				switch (aEvent.button.button)
				{
				case SDL_BUTTON_LEFT:PushOSMessage(OS_TOUCHSTART,++gTouchHash,aXPos,aYPos,-1);break;
				case SDL_BUTTON_MIDDLE:PushOSMessage(OS_TOUCHSTART,++gTouchHash,aXPos,aYPos,0);break;
				case SDL_BUTTON_RIGHT:PushOSMessage(OS_TOUCHSTART,++gTouchHash,aXPos,aYPos,1);break;
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			{
				int aXPos=aEvent.button.x;
				int aYPos=aEvent.button.y;
				//
				// Here we convert resolution to the actual page's resolution!
				// Zoomed screens should be mapped identically.
				//
				// #ifdef _PORT_RESOLUTION
				// Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				// !ERROR
				// #endif

				Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				switch (aEvent.button.button)
				{
				case SDL_BUTTON_LEFT:PushOSMessage(OS_TOUCHEND,gTouchHash,aXPos,aYPos,-1);break;
				case SDL_BUTTON_MIDDLE:PushOSMessage(OS_TOUCHEND,gTouchHash,aXPos,aYPos,0);break;
				case SDL_BUTTON_RIGHT:PushOSMessage(OS_TOUCHEND,gTouchHash,aXPos,aYPos,1);break;
				}

				SDL_CaptureMouse(SDL_FALSE);
			}
			break;
		case SDL_FINGERMOTION:
			{
				int aXPos=(int)(aEvent.tfinger.x*(float)gResolutionWidth);
				int aYPos=(int)(aEvent.tfinger.y*(float)gResolutionHeight);
				Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				PushOSMessage(OS_TOUCHMOVE,(int)aEvent.tfinger.fingerId,aXPos,aYPos);	
				
			}
			break;
		case SDL_FINGERDOWN:
			{
				int aXPos=(int)(aEvent.tfinger.x*(float)gResolutionWidth);
				int aYPos=(int)(aEvent.tfinger.y*(float)gResolutionHeight);
				Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				PushOSMessage(OS_TOUCHSTART,(int)aEvent.tfinger.fingerId,aXPos,aYPos,-1);break;
			}
			break;
		case SDL_FINGERUP:
			{
				int aXPos=(int)(aEvent.tfinger.x*(float)gResolutionWidth);
				int aYPos=(int)(aEvent.tfinger.y*(float)gResolutionHeight);
				Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				PushOSMessage(OS_TOUCHEND,(int)aEvent.tfinger.fingerId,aXPos,aYPos,-1);break;
			}
			break;

		case SDL_MOUSEWHEEL:
			if (aEvent.wheel.y>0) PushOSMessage(OS_SPIN,0,20);
			if (aEvent.wheel.y<0) PushOSMessage(OS_SPIN,0,-20);
			break;
		case SDL_TEXTINPUT:
		{
			int aKey=aEvent.text.text[0];
			if (gBackButtonOverride && aKey==27) {PushOSMessage(OS_BACK);break;}
			if (aKey>=32 && aKey<=128) PushOSMessage(OS_CHAR,0,aKey);
			break;
		}
		case SDL_KEYDOWN:
		{
			if (aEvent.key.keysym.sym==SDLK_PRINTSCREEN) PushOSMessage(OS_SCREENSHOT);
			//else if (gBackButtonOverride && aEvent.key.keysym.sym==SDLK_ESCAPE) PushOSMessage(OS_BACK);
			//else PushOSMessage(OS_KEYDOWN,0,aEvent.key.keysym.sym);

			int aKey=aEvent.key.keysym.scancode;
			if (gBackButtonOverride && aKey==KB_ESCAPE) {PushOSMessage(OS_BACK);break;}
			switch (aKey)
			{
			case KB_BACKSPACE: PushOSMessage(OS_CHAR,0,CHAR_BACKSPACE);break;
			case KB_ENTER: PushOSMessage(OS_KEYDOWN,0,13);break;
				//
				// SDL2 busted things by killing off unicode, so we can't just send every single 
				// keydown any more.  This function mostly exists to pass keys to GUI operators
				// like textbox.
				//
			case KB_DELETE:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_DELETE);break;
			case KB_LEFTARROW:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_LEFTARROW);break;
			case KB_RIGHTARROW:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_RIGHTARROW);break;
			case KB_UPARROW:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_UPARROW);break;
			case KB_DOWNARROW:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_DOWNARROW);break;
			case KB_PAGEUP:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_PAGEUP);break;
			case KB_PAGEDOWN:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_PAGEDOWN);break;
			case KB_END:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_END);break;
			case KB_HOME:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_HOME);break;
			case KB_TAB:PushOSMessage(OS_KEYDOWN,0,KEYDOWN_TAB);break;
			}
			break;
		}
		case SDL_KEYUP:PushOSMessage(OS_KEYUP,0,aEvent.key.keysym.sym);break;
		case SDL_DROPFILE:
			{
				std::string aStrFN=aEvent.drop.file;
				//char *aFN=new char[strlen(aEvent.drop.file)+1];
				//strcpy_s(aFN,strlen(aEvent.drop.file),aEvent.drop.file);
				SDL_free(aEvent.drop.file);

				static char* aFN=NULL; // Gets deleted in RAPT
				aFN=new char[aStrFN.size()+1];
				strcpy(aFN,aStrFN.c_str());
				//PushOSMessage(OS_DROPFILE,0,(int)aFN);
				PushOSMessage(OS_DROPFILE,0,0,0,0,(void*)(aFN));

				/*
				std::string aStrFN=aEvent.drop.file;
				//char *aFN=new char[strlen(aEvent.drop.file)+1];
				//strcpy_s(aFN,strlen(aEvent.drop.file),aEvent.drop.file);
				SDL_free(aEvent.drop.file);

				char* aFN=new char[aStrFN.size()+1];
				strcpy(aFN,aStrFN.c_str());
				PushOSMessage(OS_DROPFILE,0,(int)aFN);
				*/
			}
			break;
		}
	}
}

void OS_Core::SetAppName(char *theName)
{
	strcpy_s(gAppName,MAX_PATH,theName);
	//
	// Sets the name of the app... you should, if necessary, set the window caption, etc
	// to the App's name...
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//
	SDL_SetWindowTitle(gWindow,theName);
}

void OS_Core::SetWindowName(char *theName)
{
	//
	// Just allows to set the window name without setting
	// the app name...
	//
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//
	SDL_SetWindowTitle(gWindow,theName);
}

void OS_Core::SetWindowAsTool()
{
	//
	// If appropriate, allows making the window a "tool window" so that
	// you have the smaller title bar.  Put in for working with windows, and probably quite
	// unnecessary.
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//
}

//
// These functions are pretty specific for PCs, so only port them if you really
// need them.
//
// #ifdef _PORT_DIRECTORIES
// Error!
// #endif
//
void OS_Core::MakeTrayApp(char* theCloseMenuText) {}
void OS_Core::SetTrayTooltip(char* theTooltip) {}
void OS_Core::SetTrayBubble(char* theTitle, char* theText) {}

unsigned int OS_Core::Tick()
{
	//
	// Just gets the ticks.  Ticks should be "NUMBER OF MILLISECONDS"
	//
	// #ifdef _PORT_PUMP
	// Error!
	// #endif
	//
	return GetTickCount();
	//return SDL_GetTicks();
}

void OS_Core::Sleep(int theAmount)
{
	//
	// Sleeps for a duration (in milliseconds)
	//
	// #ifdef _PORT_PUMP
	// Error!
	// #endif
	//
	SDL_Delay(theAmount);
}

void OS_Core::LocationAlias(char* theFolder, char* theAlias) {Common::SetAlias(theFolder,theAlias);}

void OS_Core::GetDesktopFolder(char *theResult)
{
	//
	// Gets the path of the desktop folder.
	//
	// **This is how you do it on PC:
	//		char aPath[MAX_PATH];
	//		if (SUCCEEDED(SHGetFolderPath(NULL,CSIDL_DESKTOP,NULL,SHGFP_TYPE_CURRENT,aPath))) strcpy(theResult,aPath);
	//		else strcpy(theResult,"");

	// **This is how you do it on Mac:
	//		char *aHome=getenv("HOME");
	//		char aResult[MAX_PATH];
	//		strcpy(aResult,aHome);
	//		strcat(aResult,"/Desktop/");
	//		strcpy(theResult,aResult);
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//
	char aPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL,CSIDL_DESKTOP,NULL,SHGFP_TYPE_CURRENT,aPath))) strcpy(theResult,aPath);
	else strcpy(theResult,"");
	strcat(theResult,"\\");
}

void OS_Core::GetTempFolder(char *theResult)
{
	char aPath[MAX_PATH];
	GetTempPathA(MAX_PATH,aPath); 
	strcpy(theResult,aPath);
	strcat(theResult,gAppName);
	strcat(theResult,"\\");
}




#ifdef HOGGY_COMPATIBILITY
void OS_Core::GetSandboxFolder(char *theResult)
{
	//
	// Gets the path of our sandbox... this is the REAL sandbox, that the OS wants.  The client might modify it to
	// make it portable, etc.
	//
	// ** This is how you do it on Windows:
	//		static char aSandboxDir[MAX_PATH];
	//		strcpy(aSandboxDir,getenv("APPDATA"));
	//		strcat(aSandboxDir,"\\");
	//		strcat(aSandboxDir,gAppName);
	//		strcat(aSandboxDir,"\\");
	//		strcpy(theResult,aSandboxDir);
	// 
	// ** This is how you do it on Mac:
	//		struct passwd *aP=getpwuid(getuid());
	//		char *aHome=aP->pw_dir;
	//		char aResult[MAX_PATH];
	//		snprintf(aResult,MAX_PATH,"%s/Library/Application Support/%s/",aHome,gAppName);
	//		strcpy(theResult,aResult);
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//

	static char aSandboxDir[MAX_PATH];

	strcpy(aSandboxDir,getenv("APPDATA"));
	strcat(aSandboxDir,"\\");
	strcat(aSandboxDir,gAppName);
	strcat(aSandboxDir,"\\");
	strcpy(theResult,aSandboxDir);
}
void OS_Core::GetCloudFolder(char *theResult) {GetSandboxFolder(theResult);strcat(theResult,"cloud\\");}
void OS_Core::GetCacheFolder(char *theResult) {GetSandboxFolder(theResult);strcat(theResult,"cache\\");MakeDirectory(theResult);}

#else
void OS_Core::GetSandboxFolder(char *theResult)
{
	static char aSandboxDir[MAX_PATH];
	if (gLegacySandbox)
	{
		strcpy(aSandboxDir,getenv("APPDATA"));
		strcat(aSandboxDir,"\\");
		strcat(aSandboxDir,gAppName);
		strcat(aSandboxDir,"\\");
		strcpy(theResult,aSandboxDir);
	}
	else
	{
		strcpy(aSandboxDir,getenv("APPDATA"));
		strcat(aSandboxDir,"\\");
		strcat(aSandboxDir,gAppName);
		strcat(aSandboxDir,"\\_sandbox\\");
		strcpy(theResult,aSandboxDir);
	}
}

void OS_Core::GetCloudFolder(char *theResult) 
{
	if (gLegacySandbox) {GetSandboxFolder(theResult);strcat(theResult,"cloud\\");return;}
	static char aSandboxDir[MAX_PATH];
	strcpy(aSandboxDir,getenv("APPDATA"));
	strcat(aSandboxDir,"\\");
	strcat(aSandboxDir,gAppName);
	strcat(aSandboxDir,"\\_cloud\\");
	strcpy(theResult,aSandboxDir);
}

void OS_Core::GetCacheFolder(char *theResult) 
{
	if (gLegacySandbox) {GetSandboxFolder(theResult);strcat(theResult,"cache\\");MakeDirectory(theResult);return;}
	
	char aPath[MAX_PATH];
	GetTempPathA(MAX_PATH,aPath); 
	strcpy(theResult,aPath);
	strcat(theResult,gAppName);
	strcat(theResult,"\\cache\\");

/*	
	static char aSandboxDir[MAX_PATH];

	strcpy(aSandboxDir,getenv("APPDATA"));
	strcat(aSandboxDir,"\\");
	strcat(aSandboxDir,gAppName);
	strcat(aSandboxDir,"\\_cache\\");
	strcpy(theResult,aSandboxDir);
*/
}
#endif


//
// This doesn't necessarily need to be ported.  This just exists to convert Windows style slashes to
// unix style.  It's flagged for port so that you're aware it exists.  It's just a helper function,
// don't call it if not needed!
//
#include <string>

bool OS_Core::DoesFileExist(char *theFilename)
{
	_FixPath(theFilename);

	//
	// Simply checks for the existence of a file, returning true if it exists.
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif
	//

	// FIXMAC
	return (_access(theFilename,0)==0);
}

char* OS_Core::ReadFile(char *theFilename, int *theBytesRead)
{
	_FixPath(theFilename);

	//
	// Reads an entire file in, and returns the char array of the file, and sets
	// *theBytesRead to the filelength.
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif

	// FIXMAC
	int aFile=_open(theFilename,O_BINARY|O_RDONLY);
	if (aFile==-1)
	{
		*theBytesRead=0;
		return NULL;
	}
	int aLen=_filelength(aFile);
	*theBytesRead=aLen;
	char *aData=new char[aLen];
	_read(aFile,aData,aLen);
	_close(aFile);

	return aData;
}

char* OS_Core::ReadPartialFile(char *theFilename, int theBytesToRead)
{
	_FixPath(theFilename);

	//
	// Reads in part of a file.
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif

	// FIXMAC
	int aFile=_open(theFilename,O_BINARY|O_RDONLY);
	if (aFile==-1) return NULL;
	int aLen=theBytesToRead;

	char *aData=new char[aLen];
	_read(aFile,aData,aLen);
	_close(aFile);

	return aData;
}

bool OS_Core::WriteFile(char *theFilename, char *theDataPtr, int theLen)
{
	_FixPath(theFilename);
	MakeDirectory(theFilename);

	//
	// Writes out an entire byte arrow to a file.
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif

	// FIXMAC
	int aFile=_creat(theFilename,S_IREAD|S_IWRITE);
	if (aFile!=-1)
	{
		_close(aFile);
		aFile=_open(theFilename,O_BINARY|O_RDWR);
		_write(aFile,theDataPtr,theLen);
		_close(aFile);

		return true;
	}
	return false;
}

void OS_Core::SecureLog(char* theFilename, char* theData)
{
	_FixPath(theFilename);
	if (!DoesFileExist(theFilename))
	{
		char* aSecureLog="*** Previous SecureLog did not exist, created new ***\r\n";
		WriteFile(theFilename,aSecureLog,strlen(aSecureLog));
	}

	int aFile=_open(theFilename,O_BINARY|O_RDWR|O_APPEND);
	if (aFile!=-1)
	{
		char* aCRLF="\r\n";
		_write(aFile,theData,strlen(theData));
		_write(aFile,aCRLF,2);
		_close(aFile);
	}
}


void OS_Core::MakeDirectory(char *theDirectoryName)
{
	_FixPath(theDirectoryName);

	//
	// Creates a new directory (name will end in a slash!)
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif

	// FIXMAC
	if (strchr(theDirectoryName,'\\')==NULL) _mkdir(theDirectoryName);
	else
	{
		//
		// Okay, every string 'token' that ends in '\' is a directory we need 
		// to make.
		//
		char aMakeDir[MAX_PATH];

		int aPtr=0;
		for (;;)
		{
			if (theDirectoryName[aPtr]==0) break;
			if (theDirectoryName[aPtr]=='\\')
			{
				//
				// It's a directory!
				//
				strncpy(aMakeDir,theDirectoryName,aPtr);
				aMakeDir[aPtr]=0;
				if (strlen(aMakeDir)>0)
				{
					_mkdir(aMakeDir);
				}
			}

			aPtr++;
		}
	}
}

void OS_Core::DeleteDirectory(char *theDirectoryName)
{
	_FixPath(theDirectoryName);

	//
	// Deletes an entire directory.  It expects NOTHING to be left, a total wipe.
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif

	//FIXMAC
	char aStartingDirectory[MAX_PATH];
	strcpy(aStartingDirectory,theDirectoryName);

	if (strlen(aStartingDirectory)<2) return;
	if (aStartingDirectory[0]=='.') return;
	if (aStartingDirectory[1]==':')
	{
		if (strlen(aStartingDirectory)==3) return;
		if (strlen(aStartingDirectory)==2) return;
	}

	if (aStartingDirectory[strlen(aStartingDirectory)-1]!='\\' && aStartingDirectory[strlen(aStartingDirectory)-1]!='/') strcat(aStartingDirectory,"\\");

	char aFindDirectory[MAX_PATH];
	strcpy(aFindDirectory,aStartingDirectory);
	strcat(aFindDirectory,"*.*");

	WIN32_FIND_DATAA aFindData;

	HANDLE aFindHandle = FindFirstFileA(aFindDirectory, &aFindData); 
	if (aFindHandle==INVALID_HANDLE_VALUE) return;
	do
	{		
		if ((aFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0)
		{
			if ((strcmp(aFindData.cFileName,".")!=0) && (strcmp(aFindData.cFileName,"..")!=0))
			{
				char aNewDir[MAX_PATH];
				strcpy(aNewDir,aStartingDirectory);
				strcat(aNewDir,aFindData.cFileName);

				DeleteDirectory(aNewDir);
			}
		}
		else
		{	
			char aFilename[MAX_PATH];
			strcpy(aFilename,aStartingDirectory);
			strcat(aFilename,aFindData.cFileName);

			remove(aFilename);
		}
	}
	while (FindNextFileA(aFindHandle,&aFindData));
	FindClose(aFindHandle);
	RemoveDirectoryA(theDirectoryName);
}

void OS_Core::DeleteFile(char *theFilename)
{
	_FixPath(theFilename);

	//
	// Deletes a file
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif

	//FIXMAC
	remove(theFilename);
}

bool OS_Core::RenameFile(char* theOldName, char* theNewName)
{
	_FixPath(theOldName);
	_FixPath(theNewName);
	return (rename(theOldName,theNewName)==0);
}


void OS_Core::EnumDirectory(char *theDirectoryName, Array<char *>&theArray, bool includeSubdirs)
{
	_FixPath(theDirectoryName);

	//
	// Enumerates an entire directory into the array.  Subdirs are not enumerated, they are just included in the
	// array with a '::' preceding the name, so that they can be enumerated from outside.
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif

	//FIXMAC
	char aStartingDirectory[MAX_PATH];
	strcpy(aStartingDirectory,theDirectoryName);

	if (strlen(aStartingDirectory)<2) return;
	if (aStartingDirectory[0]=='.') return;
	if (aStartingDirectory[1]==':')
	{
		if (strlen(aStartingDirectory)==3) return;
		if (strlen(aStartingDirectory)==2) return;
	}

	if (aStartingDirectory[strlen(aStartingDirectory)-1]!='\\' && aStartingDirectory[strlen(aStartingDirectory)-1]!='/') strcat(aStartingDirectory,"\\");

	char aFindDirectory[MAX_PATH];
	strcpy(aFindDirectory,aStartingDirectory);
	strcat(aFindDirectory,"*.*");

	WIN32_FIND_DATAA aFindData;

	HANDLE aFindHandle = FindFirstFileA(aFindDirectory, &aFindData); 
	if (aFindHandle==INVALID_HANDLE_VALUE) return;
	do
	{		
		if ((aFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0)
		{
			if (aFindData.cFileName[0]!='.')
			{
				if (includeSubdirs)
				{
					char aFilename[MAX_PATH];
					sprintf(aFilename,"::%s",aFindData.cFileName);
					int aSlot=theArray.Size();

					theArray[aSlot]=new char[strlen(aFilename)+1];
					strcpy(theArray[aSlot],aFilename);
				}
			}
		}
		else
		{	
			int aSlot=theArray.Size();
			theArray[aSlot]=new char[strlen(aFindData.cFileName)+1];
			strcpy(theArray[aSlot],aFindData.cFileName);
		}
	}
	while (FindNextFileA(aFindHandle,&aFindData));
	FindClose(aFindHandle);
}

bool OS_Core::IsDirectory(char* theFilename)
{
	_FixPath(theFilename);

	//
	// Tells if a filename is pointing at a directory/folder.
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif
	//

	//FIXMAC
	long aFileAttr=GetFileAttributesA(theFilename);
	if (aFileAttr&FILE_ATTRIBUTE_DIRECTORY) return true;
	return false;
}

void OS_Core::GetOSVersion(char *theResult)
{
	//
	// Grabs the version string for the OS... usually used to check for weirdness.
	//
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	// FIXMAC
	OSVERSIONINFO aVersionInfo;
	aVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&aVersionInfo); 

	if (aVersionInfo.dwMajorVersion==4) strcpy(theResult,"98");
	if (aVersionInfo.dwMajorVersion==5) strcpy(theResult,"XP");
	if (aVersionInfo.dwMajorVersion==6) strcpy(theResult,"VISTA");
	if (aVersionInfo.dwMajorVersion==7) strcpy(theResult,"WIN7");
	strcpy(theResult,"unknown");
}

int	OS_Core::MessageBox(char *theCaption, char *theText, int theType)
{
	//
	// Displays a standard, by OS messagebox.  This is for flagging problems or debugging.
	// Type is 1 for YESNO, 2 for YESNOCANCEL, anything else is just "OK"
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	// FIXMAC
	int aResult;
	switch (theType)
	{
	case 1:
		aResult=::MessageBoxA(NULL,theText,theCaption,MB_YESNO);
		if (aResult==IDYES) aResult=1;
		else if (aResult==IDNO) aResult=0;
		break;
	case 2:
		aResult=::MessageBoxA(NULL,theText,theCaption,MB_YESNOCANCEL);
		if (aResult==IDYES) aResult=1;
		else if (aResult==IDNO) aResult=0;
		else if (aResult==IDCANCEL) aResult=-1;
		break;
	default:
		aResult=::MessageBoxA(NULL,theText,theCaption,MB_OK);
		break;
	}
	return aResult;
}

bool OS_Core::WillAppProcessInBackground()
{
	//
	// Just return true if it can run in the background, false if not.  The rule is, if it can run in the
	// background, the app should do some sleeping when it's backgrounded... but otherwise no.
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//
	return true;
}

char* OS_Core::GetLocalName()
{
	//
	// Returns the "User's Name" on the computer.  Like who is logged in, or whatever is
	// appropriate.
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	// FIXMAC
	if (!gUsername)
	{
		gUsername=new char[MAX_PATH];
		DWORD aSize=MAX_PATH-1;
		GetUserNameA(gUsername,&aSize);
	}
	return gUsername;
}

bool OS_Core::Execute(char *theCommand)
{
	//
	// Executes a shell command locally (usually to open a URL or folder)
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	// FIXMAC
	if (PtrToUlong(ShellExecuteA(NULL,"open",theCommand, NULL, NULL, SW_SHOWNORMAL))>32) return true;
	else return false;
}



void OS_Core::SetCursor(int theCursor)
{
	static int aLastCursor=-1;
	if (theCursor==aLastCursor) return;
	aLastCursor=theCursor;
	//
	// Sets the system's cursor for the current API.  Like finger or crosshairs, etc.
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	// FIXMAC
	gCurrentCursor=theCursor;
	::SetCursor(gCursor[gCurrentCursor]);
}

void OS_Core::Thread(void (*theFunction)(void *theArg), void* theArg)
{
	//
	// Run a thread!
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	//FIXMAC
	HANDLE aThread=(HANDLE)_beginthread(theFunction,0,theArg);
}

void OS_Core::SetThreadPriority(float thePriority)
{
	//
	// Set the current thread's priority.
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	//FIXMAC
	int aPriority=0;
	if (thePriority>0) aPriority=(int)((thePriority*10)+.5f);
	if (thePriority<0) aPriority=(int)((thePriority*10)-.5f);
	::SetThreadPriority(::GetCurrentThread(),aPriority);
}

#include <commdlg.h>
char* OS_Core::GetFilename(char* thePath, char* theExtensions, bool isLoading)
{
	//
	// This is a request to open a file dialog within the OS itself, I.E. the standard save/load dialog...
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	//FIXMAC
	static char aFilePath[MAX_PATH];
	char aEXT[MAX_PATH];
	char aDefaultEXT[MAX_PATH];
	char aInPtr=0;
	char aOutPtr=0;
	char aDefaultPtr=0;
	char aGetDefault=0;
	for (int aCount=0;aCount<(int)strlen(theExtensions);aCount++)
	{
		if (theExtensions[aInPtr]==',' || theExtensions[aInPtr]=='=') 
		{
			aEXT[aOutPtr++]=0;
			aGetDefault++;
		}
		else
		{
			if (aGetDefault==1) 
			{
				if (isalnum(theExtensions[aInPtr])) aDefaultEXT[aDefaultPtr++]=theExtensions[aInPtr];
			}
			aEXT[aOutPtr++]=theExtensions[aInPtr];
		}

		aInPtr++;
	}
	aEXT[aOutPtr++]=0;
	aEXT[aOutPtr++]=0;
	aDefaultEXT[aDefaultPtr]=0;

	static OPENFILENAMEA aFN;
	memset(&aFN,0,sizeof(aFN));
	aFN.lStructSize=sizeof(aFN);
	aFN.hwndOwner=NULL;
	aFN.lpstrFile=aFilePath;
	aFN.nMaxFile=MAX_PATH-1;
	aFN.lpstrFilter=aEXT;
	aFN.nFilterIndex=1;
	aFN.lpstrFileTitle=NULL;
	aFN.nMaxFileTitle=0;
	aFN.lpstrInitialDir=thePath;
	aFN.lpstrDefExt=aDefaultEXT;

	if (isLoading)
	{
		aFN.lpstrFile[0] = '\0';
		aFN.Flags=OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
		if (GetOpenFileNameA(&aFN)==TRUE) return aFN.lpstrFile;
	}
	else
	{
		aFN.Flags=OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
		if (GetSaveFileNameA(&aFN)==TRUE) return aFN.lpstrFile;
	}
	return "";
}

#include "Iphlpapi.h"
#pragma comment(lib, "IPHLPAPI.lib")
void OS_Core::GetAppID(char* theResult)
{
	//
	// Any method you want to use to get a uniqueID for this user.
	// Not actually necessary in RwK...
	//
	strcpy(theResult,"000");
}

#include <time.h>
void OS_Core::GetTime(int addTime, int* theHour, int *theMinute, int* theSecond,int* theMonth, int* theDay, int* theYear, int* theWeekday)
{
	//
	// Return the current date/time info from the OS in the variable pointers...
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	//FIXMAC
	time_t aRawTime;
	time(&aRawTime);
	aRawTime+=addTime;
	struct tm* aTI=localtime (&aRawTime);

	*theHour=aTI->tm_hour;
	*theMinute=aTI->tm_min;
	*theSecond=aTI->tm_sec;
	*theDay=aTI->tm_mday;
	*theMonth=aTI->tm_mon+1;
	*theYear=aTI->tm_year+1900;
	if (theWeekday) *theWeekday=aTI->tm_wday;
}

void OS_Core::CopyToClipboard(char* theString)
{
	//
	// Puts a string on the clipboard
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//
	SDL_SetClipboardText(theString);
}

char* OS_Core::PasteFromClipboard()
{
	//
	// Gets a string off the clipboard
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//
	return SDL_GetClipboardText();
}

void OS_Core::Minimize()
{
	//
	// A call to minimize the app
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif

	SDL_MinimizeWindow(gWindow);
}

void OS_Core::ShowCursor(bool theState)
{
	//
	// Shows or hides the system cursor entirely
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	if (theState) SDL_ShowCursor(SDL_ENABLE);
	else SDL_ShowCursor(SDL_DISABLE);
}



void OS_Core::RequestShutdown()
{
	//
	// This is the way for a client to request a shutdown.  This should NOT simply set the shutdown boolean-- what
	// this should do is post an event or otherwise go through the local API to shut down the system.  This would be
	// invoked by, say, hitting an "EXIT" button in-game.
	//
	// #ifdef _PORT_QUITTING
	// Error!
	// #endif
	//
	SDL_Event aEvent;
	aEvent.type=SDL_QUIT;
	SDL_PushEvent(&aEvent);
}


void OS_Core::SetResolution(int theWidth, int theHeight, bool isFullscreen)
{
	if (Graphics_Core::IsFakeDevice())
	{
		theWidth=Graphics_Core::GetFakeDeviceResolutionX();
		theHeight=Graphics_Core::GetFakeDeviceResolutionY();
		isFullscreen=false;
	}

	//
	// Sets the WINDOW resolution.  This does not set the graphical resolution, but only the window soze.
	// On most systems, this will not do anything except push OS_SYSTEMNOTIFY message with "SYSTEMNOTIFY_RESOLUTION" as the string.
	// If there's a fixed resolution, this doesn't matter at all, except you could fill gResolutionWidth/Height with actual values.
	//
	// HOW IT WORKS: This SetResolution is the window size.  In Graphics_Core, it sets the page size (or the
	// fullscreen resolution).  While these usually are kept the same, they're left seperated for flexibility
	// reasons.  An App that is setting the resolution will call both of these:
	//
	//				OS_Core::SetResolution(w,h,f);
	//				Graphics_Core::SetResolution(w,h,f);
	//
	//
	// #ifdef _PORT_RESOLUTION
	// !ERROR
	// #endif
	//
	gResolutionWidth=theWidth;
	gResolutionHeight=theHeight;
	gFullscreen=isFullscreen;

	if (gWindow)
	{
		SDL_SetWindowSize(gWindow,theWidth,theHeight);
		char *aNotify=QuickString("SYSTEMNOTIFY_RESOLUTION");
		PushOSMessage(OS_SYSTEMNOTIFY,0,0,0,0,aNotify);
	}
}

void OS_Core::GetResolution(int *theWidth, int *theHeight)
{
	//
	// Just gets the OS window's resolution back...
	//
	// #ifdef _PORT_RESOLUTION
	// !ERROR
	// #endif
	//
	*theWidth=gResolutionWidth;
	*theHeight=gResolutionHeight;
}






namespace OS_Core
{
	//
	// Need to define a threadlock structure that has the proper variables, initialization, and destruction
	// for your system.  (The college-boys call these semaphores, I guess.  That name kinda makes sense, but
	// is overly cutesy.
	//
	//		struct ThreadLockStruct
	//		{
	//			THREADLOCK_VARIABLE mLock;
	//			ThreadLockStruct() {THREADLOCK_CONSTRUCT;}
	//			~ThreadLockStruct() {THREADLOCK_DESTRUCT;}
	//			Lock() {DO WHAT NEEDS TO BE DONE}
	//			Unlock() {DO WHAT NEEDS TO BE DONE}
	//		};
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	// FIXMAC
	struct ThreadLockStruct
	{
		CRITICAL_SECTION mLock;
		ThreadLockStruct() {InitializeCriticalSection(&mLock);}
		~ThreadLockStruct() {DeleteCriticalSection(&mLock);}
		void Lock() {EnterCriticalSection(&mLock);}
		void Unlock() {LeaveCriticalSection(&mLock);}
	};
	Array<ThreadLockStruct*> gThreadLockList;
}

int OS_Core::CreateThreadLock() {int aCount;for (aCount=0;aCount<gThreadLockList.Size();aCount++) if (gThreadLockList[aCount]==NULL) break;gThreadLockList[aCount]=new ThreadLockStruct;return aCount;}
void OS_Core::ReleaseThreadLock(int theLock) {if (theLock>=0) if (gThreadLockList[theLock]) {delete gThreadLockList[theLock];gThreadLockList[theLock]=NULL;}}
void OS_Core::ReleaseAllThreadLocks() {for (int aCount=0;aCount<gThreadLockList.Size();aCount++) ReleaseThreadLock(aCount);gThreadLockList.Reset();}
void OS_Core::ThreadLock(int theLock) {if (theLock>=0) if (gThreadLockList[theLock]) gThreadLockList[theLock]->Lock();}
void OS_Core::ThreadUnlock(int theLock)  {if (theLock>=0) if (gThreadLockList[theLock]) gThreadLockList[theLock]->Unlock();}

void OS_Core::Archive(char* theRootFolder, Array<char*>& theFiles, char* theZipFilename, float* theProgress) 
{
	#ifdef _PORT_FINAL
	error!
	#endif
}
void OS_Core::UnArchive(char* theRootFolder, char* theZipFilename, float* theProgress) 
{
	#ifdef _PORT_FINAL
	error!
	#endif
}

bool OS_Core::IsTouchDevice() {return Graphics_Core::IsFakePhone();}
void OS_Core::PopupKeyboard(bool doShow)
{
	if (Graphics_Core::IsFakePhone()) OS_Core::PushOSMessage(OS_Core::OS_OBSCURESCREENBOTTOM,0,(Graphics_Core::GetPageHeight()/2)*doShow);
	/*
	#ifdef _STEAM
	SteamUtils()->ShowGamepadTextInput(EGamepadTextInputMode::k_EGamepadTextInputModeNormal, EGamepadTextInputLineMode::k_EGamepadTextInputLineModeSingleLine, "pchDescription", 99, "pchExistingText");
	#endif
	/**/
}

std::string CloudKeyEncode(const char* theFN, char theEncodeChar='~')
{
	std::string aNewKey=theFN;
	// Encode it however you want, if you want to...
	return aNewKey;
}

std::string CloudKeyUnencode(const char* theFN, char theEncodeChar='~')
{
	std::string aNewKey=theFN;
	// Unencode however you want
	return aNewKey;
}


#ifdef FAKECLOUD
bool OS_Core::HasCloud() {return true;}
char OS_Core::GetCloudStatus() {return 1;}
#else
bool OS_Core::HasCloud() {return false;}
char OS_Core::GetCloudStatus() {return -1;}
#endif
bool OS_Core::PutCloud(char* theKey, void* thePtr, int theSize, bool waitForResults)
{
#ifdef FAKECLOUD
	char aPath[MAX_PATH];
	GetDesktopFolder(aPath);
	strcat(aPath,"\\FakeCloud\\");
	MakeDirectory(aPath);
	//strcat(aPath,theKey);

	std::string aNewKey=CloudKeyEncode(theKey);
	theKey=(char*)aNewKey.c_str();

	char aKey[MAX_PATH];int aKP=0;char aLastChar=0;
	for (unsigned int aCount=0;aCount<strlen(theKey);aCount++) 
	{
		if (theKey[aCount]=='/' || theKey[aCount]=='\\') if (aLastChar==theKey[aCount]) continue;
		aLastChar=theKey[aCount];aKey[aKP]=aLastChar;if (aKey[aKP]=='/' || aKey[aKP]=='\\') aKey[aKP]='~';
		aKP++;
	}
	aKey[aKP]=0;
	//OS_Core::Printf("MakeKey: [%s]",aKey);


	/*
	char aKey[MAX_PATH];
	strcpy(aKey,theKey);
	for (unsigned int aCount=0;aCount<strlen(aKey);aCount++) if (aKey[aCount]=='/' || aKey[aCount]=='\\') aKey[aCount]='~';
	strcat(aPath,aKey);
	*/
	strcat(aPath,aKey);

	return WriteFile(aPath,(char*)thePtr,theSize);
#endif
	return true;
}

bool OS_Core::GetCloud(char* theKey, void** thePtr, int* theSize)
{
#ifdef FAKECLOUD
	char aPath[MAX_PATH];
	GetDesktopFolder(aPath);
	strcat(aPath,"\\FakeCloud\\");
	//strcat(aPath,theKey);
	MakeDirectory(aPath);

	std::string aNewKey=CloudKeyEncode(theKey);
	theKey=(char*)aNewKey.c_str();

	char aKey[MAX_PATH];int aKP=0;char aLastChar=0;
	for (unsigned int aCount=0;aCount<strlen(theKey);aCount++) 
	{
		if (theKey[aCount]=='/' || theKey[aCount]=='\\') if (aLastChar==theKey[aCount]) continue;
		aLastChar=theKey[aCount];aKey[aKP]=aLastChar;if (aKey[aKP]=='/' || aKey[aKP]=='\\') aKey[aKP]='~';
		aKP++;
	}
	aKey[aKP]=0;
	strcat(aPath,aKey);


	int aSize=0;
	char* aResult=ReadFile(aPath,&aSize);
	if (aSize>0)
	{
		*theSize=aSize;
		*thePtr=aResult;
		return true;
	}
#endif

	*theSize=0;
	*thePtr=NULL;
	return false;
}

void OS_Core::DeleteCloud(char* theKey)
{
#ifdef FAKECLOUD

	char aPath[MAX_PATH];
	GetDesktopFolder(aPath);
	strcat(aPath,"\\FakeCloud\\");

	if (strcmp(theKey,"*.*")==0)
	{
		OS_Core::DeleteDirectory(aPath);
		return;
	}

	std::string aNewKey=CloudKeyEncode(theKey);
	theKey=(char*)aNewKey.c_str();


	char aKey[MAX_PATH];int aKP=0;char aLastChar=0;
	for (unsigned int aCount=0;aCount<strlen(theKey);aCount++) 
	{
		if (theKey[aCount]=='/' || theKey[aCount]=='\\') if (aLastChar==theKey[aCount]) continue;
		aLastChar=theKey[aCount];aKey[aKP]=aLastChar;if (aKey[aKP]=='/' || aKey[aKP]=='\\') aKey[aKP]='~';
		aKP++;
	}
	aKey[aKP]=0;
	strcat(aPath,aKey);
	OS_Core::DeleteFile(aPath);

	int aLen=strlen(aPath);
	if (aPath[aLen-1]=='\\' || aPath[aLen-1]=='/') OS_Core::DeleteDirectory(aPath);
#endif
}

bool OS_Core::IsCloudEmpty()
{
	//
	// Debug... pretends cloud is taking 5 seconds to connect (if you need to test that!)
	//
	/*
	static int aPause=0;
	if (++aPause<500) return true;
	return false;
	*/


	static char aResult=-1;
	if (aResult==-1)
	{
		Array<char*> aEnum;
		EnumCloud(aEnum);
		aResult=(aEnum.Size()>0);
	}
	return (aResult<=0);
}

void OS_Core::EnumCloud(Array<char *>&theArray)
{
#ifdef FAKECLOUD
	char aPath[MAX_PATH];
	GetDesktopFolder(aPath);
	strcat(aPath,"\\FakeCloud\\");

	Array<char*> aHoldArray;
	EnumDirectory(aPath,aHoldArray,false);
	//for (int aFileCount=0;aFileCount<theArray.Size();aFileCount++) for (unsigned int aCount=0;aCount<strlen(theArray[aFileCount]);aCount++) if (theArray[aFileCount][aCount]=='~') theArray[aFileCount][aCount]='\\';

	theArray.GuaranteeSize(aHoldArray.Size());

	for (int aFileCount=0;aFileCount<theArray.Size();aFileCount++)
	{
		std::string aNewStr=CloudKeyUnencode(aHoldArray[aFileCount]);
		theArray[aFileCount]=new char[aNewStr.length()+1];
		strcpy(theArray[aFileCount],aNewStr.c_str());

		delete [] aHoldArray[aFileCount];
	}

#else
	theArray.Reset();
#endif
}

void OS_Core::SetCursorPos(int x, int y)
{
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_WARP_MOTION,"1");
	SDL_WarpMouseInWindow(gWindow,x,y);
}


#define __CPP
#include "..\common.h"
#undef __CPP








