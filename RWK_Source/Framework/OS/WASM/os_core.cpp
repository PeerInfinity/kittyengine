#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "os_core.h"
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <emscripten.h>
#include "graphics_core.h"
#include "rapt_string.h"
#include "rapt_math.h"   // P0dprime: HASH8() lives here (Linux/os_core.cpp gets it transitively)

#define __HEADER
#include "common.h"
#undef __HEADER

namespace OS_Core
{
	//
	// Put port specific stuff here at the top...
	//
	::SDL_Window* gWindow=NULL;
	int gCurrentCursor=0;
	int gTouchHash=0;
	bool gFPSMouse=false;
	bool gXMouseButton[5]={0,0,0,0,0};

	int gResolutionWidth=800;
	int gResolutionHeight=600;
	bool gFullscreen=false;
	bool gBackButtonOverride=false;
    pthread_mutex_t gSystemLock;
	
	String gPasteData;
	
	bool gLegacySandbox=false;



	//
	// Stuff that is very common across all OS's... only changes here might be for syntax or language 
	// variability.
	//
	bool gWantShutdown=false;	// Whether the app has requested shutdow (i.e. stop pumping)

	//
	// Messaging helpers, for outputting messages while porting...
	//
	void Error(char *format, ...) {char aString[2048];va_list argp;va_start(argp, format);vsnprintf(aString,2048,format, argp);va_end(argp);Output(aString);exit(0);}
	void Printf(char *format, ...) {char aString[2048];va_list argp;va_start(argp, format);vsnprintf(aString,2048,format, argp);va_end(argp);Output(aString);}
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
	void SetAppVersion(char *theVersion) {strncpy(gAppVersion,theVersion,MAX_PATH);}
	void SetPublisher(char *theName) {strncpy(gAppPublisher,theName,MAX_PATH);}
	void GetPackageFolder(char *theResult) {strcpy(theResult,gBundlePath);}
	

	char gCommandLine[1024];
	void SetCommandLine(char *theCMD) {strcpy(gCommandLine,theCMD);}
	char* GetCommandLine() {return gCommandLine;}
	char* gUsername=NULL;
	
	//
	// Persistent Storage Stuff
	//
	static bool gIDBFS_Synced=false;
	static int gPersistantStorage=0;
	void SetPersistanceStorageSynced() {gIDBFS_Synced=true;}
	bool IsPersistantStorageSynced() {return gIDBFS_Synced;}
	void UpdatePersistantStorage()
	{
		if(gPersistantStorage)
		{
			gPersistantStorage--;
			if(gPersistantStorage==0)
			{
				EM_ASM(
					FS.syncfs(false, function (err) {
						assert(!err);
						ccall('IDBFS_WriteSync', 'v');
					});
				);
			}
		}
	}
	
	void StartPersistentStorage()
	{
		EM_ASM(
			FS.mkdir('/RAPTISOFT_SANDBOX');
			FS.mount(IDBFS, {}, '/RAPTISOFT_SANDBOX');
			FS.syncfs(true, function (err) {if(err) console.log("got some error"); else ccall('IDBFS_ReadSync', 'v');});
		);
	}
}

extern "C" 
{
     void IDBFS_ReadSync()
    {
        printf("IDBFS Files Synced from DB\n");
        OS_Core::SetPersistanceStorageSynced();
    }

    void IDBFS_WriteSync()
    {
        printf("IDBFS Files Synced to DB.\n");
    }
}


/*
char* FixPath(char *path)
{
	if (!path) return NULL;
	char *aPtr=path;
	while (*aPtr!=0)
	{
		if (*aPtr=='\\') *aPtr='/';
		aPtr++;
	}
	
	OS_Core::Printf("Fixpath: %s",path);
	
	return path;
}
*/

unsigned int _filelength(FILE *fp) 
{
	fseek(fp, 0L, SEEK_END);
	unsigned int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return sz;
}

bool _doespathexist(const char *path) 
{
	struct stat st;
	if (stat(path, &st) == 0) return (st.st_mode & S_IFDIR) != 0;
	return false;
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
	// Writes any kind of debug info.  This is the first thing to do, so you can print out messages to
	// yourself.
	//
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
	//
	printf("RAPT> %s\n",theString);
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
	#ifdef __EMSCRIPTEN_PTHREADS__
	if(theState) pthread_mutex_lock(&gSystemLock);
	else pthread_mutex_unlock(&gSystemLock);
	#endif
}



//
// Callback to ensure that everything is saved and the database gets written on quit.
//
#include <emscripten/html5.h>
void gApp_SaveEverything();
const char* onBeforeUnload(int eventType, const void *reserved, void *userData)
{
	OS_Core::Printf("About to save everything....");
	gApp_SaveEverything();
	OS_Core::gPersistantStorage=1;
	OS_Core::UpdatePersistantStorage();
    return NULL;// If you return text here, it will show a generic message (it used to display your text, but browsers eliminated that because of sploits)
}


void OS_Core::Startup()	// PORT_STARTUP
{
	// Sets the callback for BeforeUnload....

	const char * ( *on_before_unload_fptr )(int, const void *, void *) = onBeforeUnload;
	emscripten_set_beforeunload_callback(NULL, on_before_unload_fptr);

	emscripten_run_script("Startup();\n");



	//gCommandLine[0]=0; // Moved into main to avoid collision...
	gAppName[0]=0;
	gAppVersion[0]=0;
	gAppPublisher[0]=0;
	gBundlePath[0]=0;
	
	/*
	int aWidth;
	int aHeight;
	int aFullscreen;
	emscripten_get_canvas_size(&aWidth,&aHeight,&aFullscreen);
	OS_Core::Printf("STARTUP CANVAS: %d,%d",aWidth,aHeight);
	*/
	
	
	//
	// Initializes everything you need... start video/audio system,
	// create windows, whatever.  On mobile this will often by done by the
	// mobile framework itself, and can be blank.
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif

	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER)!=0) Error("Failed to start SDL");
	Graphics_Core::PreStartup();

	//gWindow=NULL;
	gWindow=SDL_CreateWindow("",100,100,gResolutionWidth,gResolutionHeight,SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);//SDL_WINDOW_HIDDEN util we set resolution!
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
	
	//#ifdef _PORT_DIRECTORIES
	//!ERROR
	//#endif

	strcpy(gBundlePath,"/");
}

void OS_Core::Shutdown() // PORT_STARTUP
{
	ReleaseAllThreadLocks();
	//
	// Whatever you need to release everything and bring it all down.
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif

	SDL_DestroyWindow(gWindow);
	SDL_Quit();
}

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
	//if (strcmp(theInfo,"WINDOW")==0) return (void*)gWindow;
	
	if (strcasecmp(theInfo,"WINDOW")==0) return (void*)gWindow;
	else if (strcasecmp(theInfo,"backbutton_override")==0) gBackButtonOverride=true;
	else if (strcasecmp(theInfo,"backbutton_normal")==0) gBackButtonOverride=false;
	else if (strcasecmp(theInfo,"ADS:NEEDCONSENT")==0) return (void*)0;
	else if (strcasecmp(theInfo,"PASTE:ON")==0) {emscripten_run_script("ShowPasteHelper(1);\n");}
	else if (strcasecmp(theInfo,"PASTE:OFF")==0) {emscripten_run_script("ShowPasteHelper(0);\n");}
	else if (strcasecmp(theInfo,"LEGACYSANDBOX")==0) {gLegacySandbox=true;}
	else if (strcasecmp(theInfo,"CLOUD_IN_SANDBOX")==0) {return (void*)gLegacySandbox;}
	else if (strcasecmp(theInfo,"FPSMouse:on")==0) {SDL_SetRelativeMouseMode(SDL_TRUE);gFPSMouse=true;}
	else if (strcasecmp(theInfo,"FPSMouse:off")==0) {SDL_SetRelativeMouseMode(SDL_FALSE);gFPSMouse=true;}
	else if (strcasecmp(theInfo,"lockmouse:on")==0) {SDL_SetWindowGrab(gWindow,SDL_TRUE);}
	else if (strcasecmp(theInfo,"lockmouse:off")==0) {SDL_SetWindowGrab(gWindow,SDL_FALSE);}


	return NULL;
}

char* OS_Core::QueryString(char* theQuery, void* extraInfo)
{
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
	return "";
}

void OS_Core::QueryBytes(char* theQuery, void** thePtr, int* theSize)
{
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
}

void* OS_Core::Query(longlong theHash,void* extraInfo)
{
	switch(theHash)
	{
		case HASH8("MOUSEBUT"):return (void*)gXMouseButton;break;
	}
	return NULL;
}



void OS_Core::CatchCrashes(char* theURL)
{
	//
	// Probably will not be used... but if you have a way to catch the crash address to apply to a map
	// file institute it here.  The URL is there for you to save for a place to post the crashes.
	//
}
	
namespace OS_Core {void PumpHelper();}
namespace Input_Core {void Startup();}
void OS_Core::PumpHelper()	// This is stubbed out so the whole function doesn't get emterpreted...
{
	SDL_Event aEvent;
	while ( SDL_PollEvent (&aEvent) ) 
	{
		switch (aEvent.type) 
		{
		case SDL_QUIT:gWantShutdown=true;break;
		case SDL_WINDOWEVENT:
			switch (aEvent.window.event)
			{
			case SDL_WINDOWEVENT_FOCUS_GAINED:PushOSMessage(OS_MULTITASKING,0,true);break;
			case SDL_WINDOWEVENT_FOCUS_LOST:PushOSMessage(OS_MULTITASKING,0,false);break;
			}
			break;
		case SDL_MOUSEMOTION:
			{
				if (aEvent.button.which==SDL_TOUCH_MOUSEID) break;
				
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
				PushOSMessage(OS_TOUCHMOVE,gTouchHash,aXPos,aYPos,0);				
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			{
				if (aEvent.button.which==SDL_TOUCH_MOUSEID) break;
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
				
				//OS_Core::Printf("TOUCHHASH: %d",gTouchHash);

				switch (aEvent.button.button)
				{
				case SDL_BUTTON_LEFT:PushOSMessage(OS_TOUCHSTART,++gTouchHash,aXPos,aYPos,-1);gXMouseButton[0]=true;break;
				case SDL_BUTTON_MIDDLE:PushOSMessage(OS_TOUCHSTART,++gTouchHash,aXPos,aYPos,0);gXMouseButton[1]=true;break;
				case SDL_BUTTON_RIGHT:PushOSMessage(OS_TOUCHSTART,++gTouchHash,aXPos,aYPos,1);gXMouseButton[2]=true;break;
				//
				// Extra mouse buttons... these won't register as touches, they can only be queried via keyboard.
				//
				case SDL_BUTTON_X1:gXMouseButton[3]=true;break;
				case SDL_BUTTON_X2:gXMouseButton[4]=true;break;
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			{
				if (aEvent.button.which==SDL_TOUCH_MOUSEID) break;
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
				case SDL_BUTTON_LEFT:PushOSMessage(OS_TOUCHEND,gTouchHash,aXPos,aYPos,-1);gXMouseButton[0]=false;break;
				case SDL_BUTTON_MIDDLE:PushOSMessage(OS_TOUCHEND,gTouchHash,aXPos,aYPos,0);gXMouseButton[1]=false;break;
				case SDL_BUTTON_RIGHT:PushOSMessage(OS_TOUCHEND,gTouchHash,aXPos,aYPos,1);gXMouseButton[2]=false;break;
				case SDL_BUTTON_X1:gXMouseButton[3]=false;break;
				case SDL_BUTTON_X2:gXMouseButton[4]=false;break;
				}
			}
			break;
		case SDL_FINGERMOTION:
			{
				int aXPos=aEvent.tfinger.x*(float)gResolutionWidth;
				int aYPos=aEvent.tfinger.y*(float)gResolutionHeight;
				Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				PushOSMessage(OS_TOUCHMOVE,aEvent.tfinger.fingerId,aXPos,aYPos);				
			}
			break;
		case SDL_FINGERDOWN:
			{
				int aXPos=aEvent.tfinger.x*(float)gResolutionWidth;
				int aYPos=aEvent.tfinger.y*(float)gResolutionHeight;
				Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				PushOSMessage(OS_TOUCHSTART,aEvent.tfinger.fingerId,aXPos,aYPos,-1);break;
			}
			break;
		case SDL_FINGERUP:
			{
				int aXPos=aEvent.tfinger.x*(float)gResolutionWidth;
				int aYPos=aEvent.tfinger.y*(float)gResolutionHeight;
				Graphics_Core::ConvertTouchResolutionToPage(&aXPos,&aYPos);
				PushOSMessage(OS_TOUCHEND,aEvent.tfinger.fingerId,aXPos,aYPos,-1);break;
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
			case KB_DELETE: 
			case KB_LEFTARROW:
			case KB_RIGHTARROW:
			case KB_UPARROW:
			case KB_DOWNARROW:
			case KB_PAGEUP:
			case KB_PAGEDOWN:
			case KB_END:
			case KB_HOME:
				PushOSMessage(OS_KEYDOWN,0,aKey);
				break;
			}
			
			//PushOSMessage(OS_KEYDOWN,0,aKey);
			break;
		}
		case SDL_KEYUP:PushOSMessage(OS_KEYUP,0,aEvent.key.keysym.sym);break;
		case SDL_DROPFILE:
			{
				char *aFN=new char[strlen(aEvent.drop.file)+1];
				strcpy(aFN,aEvent.drop.file);
				SDL_free(aEvent.drop.file);
				PushOSMessage(OS_DROPFILE,0,(int)aFN);
			}
			break;
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			Input_Core::Startup();
			break;
		}
	}
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
	Sleep(1);
	PumpHelper(); // Stubbed out because in WASM, we need anything that sleeps to be emterpreted...
}

void OS_Core::SetAppName(char *theName)
{
	strncpy(gAppName,theName,MAX_PATH);
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
	return SDL_GetTicks();
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
	#if defined(EMTERPRETED) || defined (THREAD_OK)
	emscripten_sleep(theAmount);
	#endif
}

void OS_Core::LocationAlias(char* theFolder, char* theAlias) {Common::SetAlias(theFolder,theAlias);}

void OS_Core::GetDesktopFolder(char *theResult)
{
	// No desktop folder with WebAssembly...
	strcpy(theResult,"");
}

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
	strcpy(aSandboxDir,"\\RAPTISOFT_SANDBOX");
	strcat(aSandboxDir,"\\");
	strcat(aSandboxDir,gAppName);
	strcat(aSandboxDir,"\\");
	if (!gLegacySandbox) strcat(aSandboxDir,"_sandbox\\");
	strcpy(theResult,aSandboxDir);
}

void OS_Core::GetCloudFolder(char *theResult) 
{
	if (gLegacySandbox) {GetSandboxFolder(theResult);strcat(theResult,"cloud\\");}
	else
	{
		static char aSandboxDir[MAX_PATH];
		strcpy(aSandboxDir,"\\RAPTISOFT_SANDBOX");
		strcat(aSandboxDir,"\\");
		strcat(aSandboxDir,gAppName);
		strcat(aSandboxDir,"\\");
		strcat(aSandboxDir,"_cloud\\");
		strcpy(theResult,aSandboxDir);
	}
}
void OS_Core::GetCacheFolder(char *theResult)
{
	if (gLegacySandbox) {GetSandboxFolder(theResult);strcat(theResult,"cache\\");}
	else
	{
		static char aSandboxDir[MAX_PATH];
		strcpy(aSandboxDir,"\\RAPTISOFT_SANDBOX");
		strcat(aSandboxDir,"\\");
		strcat(aSandboxDir,gAppName);
		strcat(aSandboxDir,"\\_cache\\");
		strcpy(theResult,aSandboxDir);
	}
}

void OS_Core::GetTempFolder(char *theResult)
{
	static char aSandboxDir[MAX_PATH];
	strcpy(aSandboxDir,"\\RAPTISOFT_SANDBOX");
	strcat(aSandboxDir,"\\");
	strcat(aSandboxDir,gAppName);
	strcat(aSandboxDir,"\\_temp\\");
	strcpy(theResult,aSandboxDir);
	/*
	static char aSandboxDir[MAX_PATH];
	strcpy(aSandboxDir,"\\RAPTISOFT_SANDBOX");
	strcat(aSandboxDir,"\\_temp\\");
	strcpy(theResult,aSandboxDir);
	*/
}



//
// This doesn't necessarily need to be ported.  This just exists to convert Windows style slashes to
// unix style.  It's flagged for port so that you're aware it exists.  It's just a helper function,
// don't call it if not needed!
//
//char* OS_Core::FixPath(char *path) {if (!path) return NULL;char *aPtr=path;while (*aPtr!=0) {if (*aPtr=='\\') *aPtr='/';aPtr++;}return path;}

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
	return (access(theFilename,0)==0);
}

char* OS_Core::ReadFile(char *theFilename, int *theBytesRead)
{
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif
	//

    _FixPath(theFilename);

	errno=0;
	FILE* aFile = fopen(theFilename,"rb");
	if(aFile==NULL)
	{
		OS_Core::Printf("unable[%d] to read file %s",errno, theFilename);
		*theBytesRead=NULL;
		return NULL;
	}

	int aLen=_filelength(aFile);

	char *aData=new char[aLen];
	fread(aData,1,aLen,aFile);
	fclose(aFile);
	*theBytesRead=aLen;

	return aData;
}

char* OS_Core::ReadPartialFile(char *theFilename, int theBytesToRead)
{
	_FixPath(theFilename);
	
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif
	//
	

	FILE* aFile = fopen(theFilename,"rb");
	if(aFile==NULL)
	{
		OS_Core::Printf("unable to read file %s", theFilename);
		return NULL;
	}

	int aLen = theBytesToRead;//FileTools::GetFileSize(aFile);

	char *aData=new char[aLen];
	fread(aData,1,aLen,aFile);
	fclose(aFile);

	return aData;
}

bool OS_Core::WriteFile(char *theFilename, char *theDataPtr, int theLen)
{
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif
	//

	_FixPath(theFilename);
	MakeDirectory(theFilename);
	
	// NOTE : No android S_IREAD or S_IWRITE definitions
	FILE* aFile=fopen(theFilename,"w+");
	if(!aFile)
	{
		OS_Core::Printf("unable to create file %s with error %d", theFilename, errno);
	}

	if(aFile) fclose(aFile);
	aFile=fopen(theFilename,"w+b");
	if(!aFile) OS_Core::Printf("unable to open file %s", theFilename);
	if (aFile)
	{
		fwrite(theDataPtr, 1, theLen,aFile);
		fclose(aFile);
        gPersistantStorage=5;
		return true;
	}
		
	return false;
}

void OS_Core::SecureLog(char* theFilename, char* theData)
{
		if (!DoesFileExist(theFilename))
		{
			char* aSecureLog="*** Previous SecureLog did not exist, created new ***\r\n";
			WriteFile(theFilename,aSecureLog,strlen(aSecureLog));
		}
		else
		{
			static bool aFirst=true;
			if (aFirst)
			{
				aFirst=false;
				struct stat aStat;
				if (stat(theFilename,&aStat)==0)
				{
					if (aStat.st_size>64000)
					{
						char aNewFN[MAX_PATH];
						strcpy(aNewFN,theFilename);strcat(aNewFN,"_backup");
						DeleteFile(aNewFN);RenameFile(theFilename,aNewFN);
	
						char* aSecureLog="*** Previous SecureLog was too big, created new ***\r\n";
						WriteFile(theFilename,aSecureLog,strlen(aSecureLog));
					}
				}
			}
		}
		
		FILE* aFile=fopen(theFilename,"a");
		if (aFile)
		{
			char* aCRLF="\r\n";
			fwrite(theData, 1, strlen(theData),aFile);
			fwrite(aCRLF, 1, 2,aFile);
			fclose(aFile);
		}
}


void OS_Core::MakeDirectory(char *theDirectoryName)
{
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//


	_FixPath(theDirectoryName);
		
	//
	// Okay, here's what we do... we build strings, looking for a '\'
	// symbol.  No slash symbol?  Then it's just a simple directory that
	// we're gonna make.  But if there's a slash symbol, then we're trying
	// to watch for filename.
	//
	if (strchr(theDirectoryName,'/')==NULL) mkdir(theDirectoryName,0777);
	else
	{
		//
		// Okay, every string 'token' that ends in '/' is a directory we need 
		// to make.
		//
		char aMakeDir[MAX_PATH];
			
		int aPtr=0;
		for (;;)
		{
			if (theDirectoryName[aPtr]==0) break;
			if (theDirectoryName[aPtr]=='/')
			{
				//
				// It's a directory!
				//
				strncpy(aMakeDir,theDirectoryName,aPtr);
				aMakeDir[aPtr]=0;
				if (strlen(aMakeDir)>0)
				{
					if(!_doespathexist(aMakeDir))
					{
						int ret = mkdir(aMakeDir,0777);
						if(ret!=0)
						{
							OS_Core::Printf("failed to make dir %s", aMakeDir);
						}
					}
				}
			}
				
			aPtr++;
		}
	}
}
	
void OS_Core::DeleteDirectory(char *theDirectoryName)
{
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//


	_FixPath(theDirectoryName);
	
	OS_Core::Printf("DELETE DIRECTORY... [%s]",theDirectoryName);
		
	//
	// Okay, a very dangerous bit of code, here.
	//
	char aStartingDirectory[MAX_PATH];
	strcpy(aStartingDirectory,theDirectoryName);
	
	OS_Core::Printf("Start @ [%s]",aStartingDirectory);
	
	
	if (strlen(aStartingDirectory)<2) return;
	if (aStartingDirectory[0]=='.') return;
	if (aStartingDirectory[1]==':')
	{
		if (strlen(aStartingDirectory)==3) return;
		if (strlen(aStartingDirectory)==2) return;
	}
	if (strcasecmp(aStartingDirectory,"HOME")==0) return;
	if (strcasecmp(aStartingDirectory,"HOME/")==0) return; 
	if (aStartingDirectory[strlen(aStartingDirectory)-1]!='/') strcat(aStartingDirectory,"/");
	char aFindDirectory[MAX_PATH];
	strcpy(aFindDirectory,aStartingDirectory);
	strcat(aFindDirectory,"*.*");
		
	OS_Core::Printf("FindDirectory @ [%s]",aFindDirectory);
		
	struct dirent *aDirEntPtr;
	DIR *aDirPtr=opendir(aStartingDirectory);
	if (aDirPtr) 
	{
		aDirEntPtr=readdir(aDirPtr);
		while (aDirEntPtr)
		{
			if (aDirEntPtr->d_type==DT_DIR)
			{
				if ((strcmp(aDirEntPtr->d_name,".")!=0) && (strcmp(aDirEntPtr->d_name,"..")!=0))
				{
					char aNewDir[MAX_PATH];
					strcpy(aNewDir,aStartingDirectory);
					strcat(aNewDir,aDirEntPtr->d_name);
					strcat(aNewDir,"/");
					DeleteDirectory(aNewDir);
				}
			}
			else
			{	
				char aFilename[MAX_PATH];
				strcpy(aFilename,aStartingDirectory);
				strcat(aFilename,aDirEntPtr->d_name);
				
				OS_Core::Printf("RemoveFile: [%s]",aFilename);
				
				remove(aFilename);
			}
			aDirEntPtr=readdir(aDirPtr);
		}
	}
	if(aDirPtr) closedir(aDirPtr);
	
	
	
	OS_Core::Printf("rmdir: [%s]",theDirectoryName);
	
	if (aStartingDirectory[strlen(aStartingDirectory)-1]=='/') aStartingDirectory[strlen(aStartingDirectory)-1]=0;
	rmdir(aStartingDirectory);
//	rmdir(theDirectoryName);
}
	
void OS_Core::DeleteFile(char *theFilename)
{

	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif
	//

	_FixPath(theFilename);
	//OS_Core::Printf("Delete File: %s",theFilename);
	remove(theFilename);
}

bool OS_Core::RenameFile(char* theOldName, char* theNewName)
{
	_FixPath(theOldName);
	_FixPath(theNewName);
	
	OS_Core::Sleep(1);
	
	//OS_Core::Printf("FIX'D FROM: [%s]",theOldName);
	//OS_Core::Printf("         TO: [%s]",theNewName);
	
	
	//
	// In WASM, rename will fail to rename a directory if the last character is a '/' ....
	//
	if (theOldName[strlen(theOldName)-1]=='/') theOldName[strlen(theOldName)-1]=0;
	if (theNewName[strlen(theNewName)-1]=='/') theNewName[strlen(theNewName)-1]=0;
	
	//OS_Core::Printf("RENAME FROM: [%s]",theOldName);
	//OS_Core::Printf("         TO: [%s]",theNewName);
	
	OS_Core::Sleep(1);
	
	int aResult=(rename(theOldName,theNewName));
	//OS_Core::Printf("RESULT = %d",aResult);
	return (aResult==0);
	
	
	//return (rename(theOldName,theNewName)==0);
}

	
void OS_Core::EnumDirectory(char *theDirectoryName, Array<char *>&theArray, bool includeSubdirs)
{

	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//

	_FixPath(theDirectoryName);
	
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
		
	struct dirent *aDirEntPtr;
	DIR *aDirPtr=opendir(aStartingDirectory);
	if (!aDirPtr) return;		
	aDirEntPtr=readdir(aDirPtr);	
	if (!aDirEntPtr) return;
	do
	{		
		if (aDirEntPtr->d_type==DT_DIR)
		{
			if (aDirEntPtr->d_name[0]!='.')
			{
				if (includeSubdirs)
				{
					char aFilename[MAX_PATH];
					sprintf(aFilename,"::%s",aDirEntPtr->d_name);
					int aSlot=theArray.Size();
					
					theArray[aSlot]=new char[strlen(aFilename)+1];
					strcpy(theArray[aSlot],aFilename);
				}
			}
		}
		else
		{	
			int aSlot=theArray.Size();
			theArray[aSlot]=new char[strlen(aDirEntPtr->d_name)+1];
			strcpy(theArray[aSlot],aDirEntPtr->d_name);
		}
	}
	while ((aDirEntPtr=readdir(aDirPtr)));
	closedir(aDirPtr);
}

bool OS_Core::IsDirectory(char* theFilename)
{
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//

	_FixPath(theFilename);
	struct stat st_buf;
	int status = stat (theFilename, &st_buf);
	if (status != 0) { return false; }
	if (S_ISDIR (st_buf.st_mode)) { return true; }
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
	strcpy(theResult, "WebAssembly");
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
	
	//
	// No messagebox for webassembly...
	//
	return 0;
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
	return "Anonymous";
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
	int aLen=strlen(theCommand)+512;
	char* aText=new char[aLen];
	memset(aText,0,aLen);
	sprintf(aText,"window.open(\"%s\",\"_blank\");\n",theCommand);
	emscripten_run_script(aText);
	delete [] aText;
	return true;
}



void OS_Core::SetCursor(int theCursor)
{
	//
	// Sets the system's cursor for the current API.  Like finger or crosshairs, etc.
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//
}

void OS_Core::SetCursorPos(int x, int y)
{
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_WARP_MOTION,"1");
	SDL_WarpMouseInWindow(gWindow,x,y);
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

	#ifdef __EMSCRIPTEN_PTHREADS__
    pthread_t aThread;
    pthread_attr_t aAttr;
    pthread_attr_init(&aAttr);
    pthread_attr_setdetachstate(&aAttr, PTHREAD_CREATE_DETACHED);
    pthread_create(&aThread,&aAttr,(void*(*)(void*))theFunction,theArg);
	#else
	theFunction(theArg);
	#endif

	/*
	/**/
}

void OS_Core::SetThreadPriority(float thePriority)
{
	#ifdef __EMSCRIPTEN_PTHREADS__
	
	//
	// Set the current thread's priority.
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//

	int aPriority=0;
	int aMax=sched_get_priority_max(SCHED_OTHER);
	int aMin=sched_get_priority_min(SCHED_OTHER);
	int aMid=(aMax+aMin)/2;

	if (thePriority>0) aPriority=aMid+(int)((thePriority*(float)aMid)+.5f);
	if (thePriority<0) aPriority=aMid-(int)((thePriority*(float)aMid)-.5f);

	sched_param aParam;
	aParam.sched_priority=aPriority;
	pthread_setschedparam(pthread_self(),SCHED_OTHER,&aParam);
	#endif
}

char* OS_Core::GetFilename(char* thePath, char* theExtensions, bool isLoading)
{
	//
	// This is a request to open a file dialog within the OS itself, I.E. the standard save/load dialog...
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//
	return "";
}

void OS_Core::GetAppID(char* theResult)
{
	//
	// This grabs the computer's GUID (like the mac address)
	//
	// #ifdef _PORT_SYS
	// Error!
	// #endif
	//
	
	strcpy(theResult,"????");
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

	time_t aRawTime;
	time(&aRawTime);
	aRawTime+=addTime;
	struct tm* aTI=localtime (&aRawTime);

	if (theHour) *theHour=aTI->tm_hour;
	if (theMinute) *theMinute=aTI->tm_min;
	if (theSecond) *theSecond=aTI->tm_sec;
	if (theDay) *theDay=aTI->tm_mday;
	if (theMonth) *theMonth=aTI->tm_mon+1;
	if (theYear) *theYear=aTI->tm_year+1900;
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
	//SDL_SetClipboardText(theString);
	
	int aLen=strlen(theString)+512;
	char* aText=new char[aLen];
	memset(aText,0,aLen);
	sprintf(aText,"CopyToClipboard(\"%s\");\n",theString);
	
	//OS_Core::Printf("Run This: [%s]",aText);
	emscripten_run_script(aText);
	delete [] aText;
	
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
	return gPasteData.c();
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
	static bool aShowState=true;
	if (theState==aShowState) return;
	aShowState=theState;

	if (theState) SDL_ShowCursor(SDL_ENABLE);
	else
	{
		SDL_ShowCursor(SDL_DISABLE);

		int x, y;
		SDL_GetMouseState(&x, &y);
		SDL_WarpMouseInWindow(NULL,x,y);
	}
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

namespace Graphics_Core
{
	void PreStartup();
	extern int gResolutionWidth;
	extern int gResolutionHeight;
}

void OS_Core::SetResolution(int theWidth, int theHeight, bool isFullscreen)
{
	/*
	if (gWindow) return;
	gWindow=SDL_CreateWindow("",100,100,theWidth,theHeight,SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);//SDL_WINDOW_HIDDEN util we set resolution!
	if (!gWindow) Error("Error:SDL_CreateWindow -> %s",SDL_GetError());
	*/
	
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
	//char* aTest=emscripten_run_script_string("getCanvasWidth();\n");OS_Core::Printf("TEST RECEIVED: %s",aTest);
	
	int aWidth=emscripten_run_script_int("getCanvasWidth();\n");
	int aHeight=emscripten_run_script_int("getCanvasHeight();\n");
	if (aWidth>0 && aHeight>0) {theWidth=aWidth;theHeight=aHeight;}
	
	gResolutionWidth=theWidth;
	gResolutionHeight=theHeight;
	Graphics_Core::gResolutionWidth=theWidth;
	Graphics_Core::gResolutionHeight=theHeight;
	gFullscreen=isFullscreen;

	if (gWindow)
	{
		SDL_SetWindowSize(gWindow,theWidth,theHeight);
		char *aNotify=QuickString("SYSTEMNOTIFY_RESOLUTION");
		PushOSMessage(OS_SYSTEMNOTIFY,0,0,0,0,aNotify);
		
		//Graphics_Core::Startup_SDL();
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




//
// If threading is ever WASM enabled...
//
namespace OS_Core {
/*
    struct ThreadLockStruct {
        pthread_mutex_t mLock;

        ThreadLockStruct() {
            pthread_mutexattr_t Attr;
            pthread_mutexattr_init(&Attr);
            pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&mLock, &Attr);
        }

        ~ThreadLockStruct() { pthread_mutex_destroy(&mLock); }
    };

    Array<ThreadLockStruct *> gThreadLockList;
    int CreateThreadLock() {int aCount;for (aCount = 0; aCount < gThreadLockList.Size(); aCount++)if (gThreadLockList[aCount] == NULL)break;gThreadLockList[aCount] = new ThreadLockStruct;return aCount;}
    void ReleaseThreadLock(int theLock) {if (theLock >= 0 && gThreadLockList[theLock]) {delete gThreadLockList[theLock];gThreadLockList[theLock] = NULL;}}
	void ReleaseAllThreadLocks() {for (int aCount = 0; aCount < gThreadLockList.Size(); aCount++) ReleaseThreadLock(aCount);}
    void ThreadLock(int theLock) {if (theLock >= 0) if (gThreadLockList[theLock]) pthread_mutex_lock(&gThreadLockList[theLock]->mLock);}
	void ThreadUnlock(int theLock) {if (theLock >= 0) if (gThreadLockList[theLock]) pthread_mutex_unlock(&gThreadLockList[theLock]->mLock);}
*/

    int CreateThreadLock() {return 0;}
    void ReleaseThreadLock(int theLock) {}
	void ReleaseAllThreadLocks() {}
    void ThreadLock(int theLock) {}
	void ThreadUnlock(int theLock) {}
}


namespace OS_Core
{
	bool (*gLoadProgressCallback)(void *theArg)=NULL;	// Loader callback function
	void RegisterCallback(int theID, bool (*theFunction)(void *theArg))
	{
		switch (theID)
		{
		case 0:gLoadProgressCallback=theFunction;break;
		default:OS_Core::Printf("Unsupported ID with Register Callback: theID=%d",theID);break;
		}
	}
}

extern "C" {
	void SetPasteData(char* theResult)
	{
		OS_Core::Printf("SET PASTE: [%s]",theResult);
		OS_Core::gPasteData=theResult;
	}
}

void OS_Core::Archive(char* theRootFolder, Array<char*>& theFiles, char* theZipFilename, float* theProgress)
{
}

void OS_Core::UnArchive(char* theRootFolder, char* theZipFilename, float* theProgress)
{
}


#define __CPP
#include "common.h"
#undef __CPP
