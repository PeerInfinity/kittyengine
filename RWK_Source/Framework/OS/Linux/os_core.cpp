#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include "os_core.h"
#include <sys/time.h>
#include <sys/file.h>
#include "graphics_core.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include "input_core.h"

#define __HEADER
#include <unistd.h>
#include "../common.h"
#undef __HEADER

#define FAKECLOUD

#ifdef _STEAM
#include "isteamutils.h"
#endif

// So that we can set the window icon
#include <rapt_iobuffer.h>

#include <SDL_image.h>

void Echo(char *format, ...) {char aString[10000];va_list argp;va_start(argp, format);vsprintf(aString,format, argp);va_end(argp);printf(aString);printf("\n");}


namespace OS_Core
{
	//
	// Put port specific stuff here at the top...
	//
	::SDL_Window* gWindow=NULL;
	int gCurrentCursor=0;
	SDL_Cursor* gCursor[SYSCURSOR_MAX];
	int gTouchHash=0;
	bool gFPSMouse=false;
	bool gXMouseButton[5]={0,0,0,0,0};

	#ifdef _DEBUG
	char gWadFN[MAX_PATH]; // Debug Wad Filename (if on desktop)
	char gDebugSandbox[MAX_PATH];
	#endif

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
	void Error(char *format, ...) {char aString[2048];va_list argp;va_start(argp, format);vsprintf(aString,format, argp);va_end(argp);Output(aString);exit(0);}
	void Printf(char *format, ...) {char aString[10000];va_list argp;va_start(argp, format);vsprintf(aString,format, argp);va_end(argp);Output(aString);}
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
	void SetAppVersion(char *theVersion){strncpy(gAppVersion,theVersion,MAX_PATH);}
	void SetPublisher(char *theName) {strncpy(gAppPublisher,theName,MAX_PATH);}
	void GetPackageFolder(char *theResult) {strcpy(theResult,gBundlePath);}

	char gCommandLine[MAX_PATH];
	void SetCommandLine(char *theCMD) {strcpy(gCommandLine,theCMD);}
	char* GetCommandLine() {return gCommandLine;}

	char* gUsername=NULL;

	bool gBackButtonOverride=false;	// Whether ESC will send OS_BACK or not...
	int gReserveTopScreen=0;		// For faking a notch (Win32 only)
	int gReserveBottomScreen=0;		// For faking a notch (Win32 only)
}

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
	if (stat(path, &st) == 0) { return (st.st_mode & S_IFDIR) != 0;}
	return false;
}

char *QuickString(char *theString)
{
	char *aResult=new char[strlen(theString)+1];
	strcpy(aResult,theString);
	return aResult;
}

#include <fstream>
bool DoesProcessExist(char* theName)
{
	DIR* dir=opendir("/proc");
	if (!dir) return false;

	struct dirent* entry;
	while ((entry=readdir(dir))!=nullptr)
	{
		if (entry->d_type==DT_DIR)
		{
			std::string dirName=entry->d_name;
			// Check if the directory name is a number (PID)
			if (std::all_of(dirName.begin(),dirName.end(),::isdigit))
			{
				std::string commPath="/proc/"+dirName+"/comm";
				std::ifstream commFile(commPath);
				if (commFile)
				{
					std::string commName;
					std::getline(commFile,commName);
					// Remove trailing newline character
					commName.erase(commName.find_last_not_of("\n\r")+1);
					if (commName==theName)
					{
						closedir(dir);
						return true;
					}
				}
			}
		}
	}

	closedir(dir);
	return false;
}


#ifdef _OLD_RUTILITY
#include <mqueue.h>
mqd_t gRUtilityQueue;
#define QUEUE_NAME  "/rutility_comm"
#define MAX_SIZE    1024
void QueueCleanup()
{
	char aString[1024];
	sprintf(aString,">BYE");
	mq_send(gRUtilityQueue,aString,strlen(aString)+1,0);
	//mq_close(gRUtilityQueue);
	mq_unlink(QUEUE_NAME);
}
#else
static int gWriterFd=-1;
void InitRUtility()
{
	// gWriterFd=open(RUTILITY_FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0666);
}
void ShutdownRUtility() {if (gWriterFd>=0) {close(gWriterFd);gWriterFd=-1;}}
void WriteRUtility(char* theLog)
{
	// if (gWriterFd<0) return;
	// uint32_t aLen=(uint32_t)strlen(theLog);
	// write(gWriterFd,&aLen,sizeof(aLen));
	// write(gWriterFd,theLog,aLen);
}
void ClearRUtility()
{
	// int aFd=open(RUTILITY_FILE_PATH, O_RDWR | O_CREAT, 0666);
	// if (aFd<0) return;
	// flock(aFd,LOCK_EX);
	// ftruncate(aFd,0);
	// fsync(aFd);
	// flock(aFd,LOCK_UN);
	// close(aFd);
}
#endif

void QueueCleanup()
{
	WriteRUtility(">BYE");
	ShutdownRUtility();
}

void QueueCrashCatcher(int sig)
{
	QueueCleanup();
	exit(sig);
}

bool OS_Core::Output(char* theString) 
{
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
	OS_Core::gCursor[OS_Core::SYSCURSOR_POINTER]=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	OS_Core::gCursor[OS_Core::SYSCURSOR_IBEAM]=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	OS_Core::gCursor[OS_Core::SYSCURSOR_GRABHAND]=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	OS_Core::gCursor[OS_Core::SYSCURSOR_BLANK]=NULL;//LoadCursor(NULL,IDC_NO);
	OS_Core::gCursor[OS_Core::SYSCURSOR_FINGER]=NULL;//LoadCursor(NULL,IDC_NO);
	OS_Core::gCursor[OS_Core::SYSCURSOR_FINGER2]=NULL;//LoadCursor(NULL,IDC_NO);
	OS_Core::gCursor[OS_Core::SYSCURSOR_DRAG]=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	OS_Core::gCursor[OS_Core::SYSCURSOR_DRAGV]=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	OS_Core::gCursor[OS_Core::SYSCURSOR_DRAGH]=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);


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
	OS_Core::gCursor[OS_Core::SYSCURSOR_BLANK]=SDL_CreateCursor(aANDMask,aXORMask,16,16,32,32);
		//CreateCursor(GetModuleHandle(0),16,16,32,32,aANDMask,aXORMask);	// CURSOR_BLANK

	//HotSpot: 15 10
	//Size: 32 32
	/*
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
	*/
	OS_Core::gCursor[OS_Core::SYSCURSOR_GRABHAND]=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

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
		OS_Core::gCursor[OS_Core::SYSCURSOR_FINGER]=SDL_CreateCursor(aFingerData,aFingerData+(sizeof(aFingerData)/2),11,4,32,32);
		//OS_Core::gCursor[OS_Core::SYSCURSOR_FINGER]=CreateCursor(GetModuleHandle(0),11,4,32,32,aFingerData,aFingerData+(sizeof(aFingerData)/2)); // CURSOR_FINGER if it wasn't in the OS
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
	PartnerQuery(HASH8("PRESTART")); // Init after SDL so that we can receive events (like gamecontrollers changed)

	//
	// Not needed right now, only needed if we want desktop size...
	//
	//SDL_DisplayMode aMode;
	//if (SDL_GetDesktopDisplayMode(0,&aMode)!=0) OS_Core::Error("Could not get desktop mode: [%s]",SDL_GetError());

	if (PartnerQuery(HASH8("FORCEREZ"))) PartnerQuery(HASH8("QUERYREZ"),&gResolutionWidth,&gResolutionHeight);

	Graphics_Core::PreStartup();


//	gWindow=SDL_CreateWindow("",(aMode.w/2)-(gResolutionWidth/2),((aMode.h/2)-(gResolutionHeight/2))-50,gResolutionWidth,gResolutionHeight,SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);//SDL_WINDOW_HIDDEN util we set resolution!
//	gWindow=SDL_CreateWindow("",(aMode.w/2)-(gResolutionWidth/2),25,gResolutionWidth,gResolutionHeight,SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);//SDL_WINDOW_HIDDEN util we set resolution!
	gWindow=SDL_CreateWindow("",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,gResolutionWidth,gResolutionHeight,SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);//SDL_WINDOW_HIDDEN util we set resolution!
	if (!gWindow) Error("Error:SDL_CreateWindow -> %s",SDL_GetError());
	else
	{
		SDL_Surface* aIcon = IMG_Load("icon.png");
		SDL_SetWindowIcon(gWindow,aIcon);
		SDL_FreeSurface(aIcon);
	}

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
	//GetModuleFileNameA(NULL,aPackageDir,MAX_PATH);
	//for (int aCount=strlen(aPackageDir);aCount>0;aCount--) if (aPackageDir[aCount]=='\\') {aPackageDir[aCount]=0;break;}
	//getcwd(aPackageDir,MAX_PATH);

	//
	// Getting the CWD on linux...
	//
	ssize_t count=readlink("/proc/self/exe",aPackageDir, MAX_PATH);
	if (count!=-1)
	{
		int aLen=strlen(aPackageDir)-1;
		while (aLen>0)
		{
			if (aPackageDir[aLen]=='/') {aPackageDir[aLen]=0;break;}
			aLen--;
		}
	}
	else getcwd(aPackageDir,MAX_PATH);

	strcat(aPackageDir,"/");
	strncpy(gBundlePath,aPackageDir,MAX_PATH);
	CreateCursors();

	SDL_version linked;
	SDL_GetVersion(&linked);
	#ifdef _DEBUG
	OS_Core::Printf("_SDLVersion = %u.%u.%u.",linked.major,linked.minor,linked.patch);
	#endif
	PartnerQuery(HASH8("STARTUP!"));

}

void OS_Core::Shutdown() // PORT_STARTUP
{
	//
	// Whatever you need to release everything and bring it all down.
	//
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif

	PartnerQuery(HASH8("SHUTDOWN"));

	SDL_DestroyWindow(gWindow);
	ReleaseAllThreadLocks();

	QueueCleanup();
	while (gCriticalThreadCount>0) Sleep(100);
	SDL_Quit();

}

// Why is this here?  It causes all kinds of interference problems with X11 name reservations (things like Font and XColor)
//#include "SDL2/SDL_syswm.h"
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
	static bool aCanLockMouse=true;
	if (strcmp(theInfo,"WINDOW")==0) return (void*)gWindow;
	/*
	else if (strcmp(theInfo,"HWND")==0)
	{
		static struct ::SDL_SysWMinfo aInfo;
		SDL_VERSION(&aInfo.version);
		SDL_GetWindowWMInfo(gWindow,&aInfo);
		return (void*)&aInfo.info.win.window;
	}
	*/
	else if (strcasecmp(theInfo,"backbutton_override")==0) gBackButtonOverride=true;
	else if (strcasecmp(theInfo,"backbutton_normal")==0) gBackButtonOverride=false;
	else if (strncasecmp(theInfo,"rate:",5)==0) {OS_Core::Printf("Request Rating...");}
	else if (strcasecmp(theInfo,"glrecover:?")==0) return (void*)1;
	else if (strcasecmp(theInfo,"glrecover:on")==0) {OS_Core::Printf("GLRECOVER is on...");}
	else if (strcasecmp(theInfo,"glrecover:off")==0) {OS_Core::Printf("GLRECOVER is off...");}
	else if (strncasecmp(theInfo,"fakephone:",10)==0) {return Graphics_Core::Query(theInfo);}
	else if (strncasecmp(theInfo,"fakeweb:",8)==0) {return Graphics_Core::Query(theInfo);}
	else if (strcasecmp(theInfo,"RESERVESCREEN_TOP")==0) {return (void*)gReserveTopScreen;}
	else if (strcasecmp(theInfo,"RESERVESCREEN_BOTTOM")==0) {return (void*)gReserveBottomScreen;}
	else if (strcasecmp(theInfo,"LEGACYSANDBOX")==0) {gLegacySandbox=true;}
	else if (strcasecmp(theInfo,"CLOUD_IN_SANDBOX")==0) {return (void*)gLegacySandbox;}
	else if (strcasecmp(theInfo,"CRITICAL++")==0) {gCriticalThreadCount++;}
	else if (strcasecmp(theInfo,"CRITICAL--")==0) {gCriticalThreadCount--;}
	else if (strcasecmp(theInfo,"FPSMouse:on")==0) {SDL_SetRelativeMouseMode(SDL_TRUE);gFPSMouse=true;}
	else if (strcasecmp(theInfo,"FPSMouse:off")==0) {SDL_SetRelativeMouseMode(SDL_FALSE);gFPSMouse=false;}
	else if (strcasecmp(theInfo,"lockmouse:on")==0) {if (aCanLockMouse) SDL_SetWindowGrab(gWindow,SDL_TRUE);}
	else if (strcasecmp(theInfo,"lockmouse:off")==0) {SDL_SetWindowGrab(gWindow,SDL_FALSE);}
	else if (strcasecmp(theInfo,"lockmouse:never")==0) {aCanLockMouse=false;SDL_SetWindowGrab(gWindow,SDL_FALSE);}

	#ifdef _DEBUG
	else if (strcasecmp(theInfo,"debug_wad")==0){ if (strlen(gWadFN)>0) return gWadFN; return NULL;}
	else if (strcasecmp(theInfo,"debug_sandbox_location")==0){ if (strlen(gDebugSandbox)>0) return gDebugSandbox; return NULL;}
	#endif

	return PartnerQuery(theInfo, extraInfo);
}

char* OS_Core::QueryString(char* theQuery, void* extraInfo)
{
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
	if (strcasecmp(theQuery,"core:version")==0) return "Linux Version";

	return PartnerQueryString(theQuery, extraInfo);
}

void OS_Core::QueryBytes(char* theQuery, void** thePtr, int* theSize)
{
	// #ifdef _PORT_STARTUP
	// Error!
	// #endif
	PartnerQueryBytes(theQuery,thePtr,theSize);
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

	SDL_Event aEvent;
	while ( SDL_PollEvent (&aEvent) )
	{
		switch (aEvent.type)
		{
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			Input_Core::Query("REHUPCONTROLLERS");
			//OS_Core::Printf("Rehup Controllers %d",SDL_NumJoysticks());
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
			// Turned off screenshot since Linux can take a screenshot of a window.
			//if (aEvent.key.keysym.sym==SDLK_PRINTSCREEN) PushOSMessage(OS_SCREENSHOT);
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

			}
			break;
		}
	}
}

void OS_Core::SetAppName(char *theName)
{
	strcpy(gAppName,theName);
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
	//return GetTickCount();
	//return SDL_GetTicks(); // Not viable, gives bad timing
	struct timeval aTime;
	gettimeofday(&aTime,NULL);
	return (aTime.tv_sec * 1000000 + aTime.tv_usec)/1000;
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
	strcpy(theResult,getenv("HOME"));
	strcat(theResult,"/Desktop/");
	//char aPath[MAX_PATH];
	//if (SUCCEEDED(SHGetFolderPathA(NULL,CSIDL_DESKTOP,NULL,SHGFP_TYPE_CURRENT,aPath))) strcpy(theResult,aPath);
	//else strcpy(theResult,"");
	//strcat(theResult,"\\");
}

void OS_Core::GetTempFolder(char *theResult)
{
	strcpy(theResult,"/var/tmp/Raptisoft/");
	strcat(theResult,gAppName);
	strcat(theResult,"/");
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

	char aSandboxDir[MAX_PATH];

	strcpy(aSandboxDir,getenv("APPDATA"));
	strcat(aSandboxDir,"\\");
	strcat(aSandboxDir,gAppName);
	strcat(aSandboxDir,"\\");
	strcpy(theResult,aSandboxDir);
}
void OS_Core::GetCloudFolder(char *theResult) {GetSandboxFolder(theResult);strcat(theResult,"cloud\\");}
void OS_Core::GetCacheFolder(char *theResult) {GetSandboxFolder(theResult);strcat(theResult,"cache\\");MakeDirectory(theResult);}

#else

char* GetStorageBase()
{
	static char aStorageBase[MAX_PATH];
	static bool aFirst=true;
	if (aFirst)
	{
		strcpy(aStorageBase,getenv("HOME"));
		strcat(aStorageBase,"/.local/share/.raptisoft/");
	}
	return aStorageBase;
}


#ifdef _DEBUG
char* gLocalOverride="";
bool gInitOverrides=true;

void InitOverrides()
{
	//
	// Is a .wad on the desktop?
	// If so, our overrides go into /Debug/
	//
	Array<char*> aFiles;
	char aDesktop[MAX_PATH];
	OS_Core::GetDesktopFolder(aDesktop);
	OS_Core::EnumDirectory(aDesktop,aFiles,false);

	bool aFoundWad=false;
	for (int aCount=0;aCount<aFiles.Size();aCount++)
	{

		if (strstr(aFiles[aCount],".wad"))
		{
			OS_Core::Printf("*****************************************************************");
			OS_Core::Printf("FOUND WAD! Game is saving to /DEBUG/");
			OS_Core::Printf("*****************************************************************");
			aFoundWad=true;

			sprintf(OS_Core::gWadFN,"%s%s",aDesktop,aFiles[aCount]);
			break;
		}
	}

	if (aFoundWad)
	{
		gLocalOverride="/DEBUG/";


		//strcpy(OS_Core::gDebugSandbox,getenv("HOME"));
		//strcat(OS_Core::gDebugSandbox,"/.raptisoft/");
		strcpy(OS_Core::gDebugSandbox,GetStorageBase());
		strcat(OS_Core::gDebugSandbox,OS_Core::gAppName);
		strcat(OS_Core::gDebugSandbox,gLocalOverride);

		gInitOverrides=false;
	}
}
#endif

void OS_Core::GetSandboxFolder(char *theResult)
{
	//~/.local/share/<GameName>/

	static char aSandboxDir[MAX_PATH];
	static bool aFirst=true;
	if (aFirst)
	{
		//strcpy(aSandboxDir,getenv("HOME"));
		//strcat(aSandboxDir,"/.raptisoft/");
		strcpy(aSandboxDir,GetStorageBase());
		strcat(aSandboxDir,gAppName);

		#ifdef _DEBUG
		if (gInitOverrides) InitOverrides();
		if (gLocalOverride[0]!=0) strcat(aSandboxDir,gLocalOverride);
		#endif

		strcat(aSandboxDir,"/_sandbox/");
		aFirst=false;
	}

	strcat(theResult,aSandboxDir);
}

void OS_Core::GetAppStorageFolder(char *theResult)
{
	//~/.local/share/<GameName>/

	static char aStorageDir[MAX_PATH];
	static bool aFirst=true;
	if (aFirst)
	{
		//strcpy(aSandboxDir,getenv("HOME"));
		//strcat(aSandboxDir,"/.raptisoft/");
		strcpy(aStorageDir,GetStorageBase());
		strcat(aStorageDir,gAppName);
		strcat(aStorageDir,"/");
		aFirst=false;
	}

	strcat(theResult,aStorageDir);
}

void OS_Core::GetCloudFolder(char *theResult)
{
	static char aSandboxDir[MAX_PATH];
	static bool aFirst=true;
	if (aFirst)
	{
		//strcpy(aSandboxDir,getenv("HOME"));
		//strcat(aSandboxDir,"/.raptisoft/");
		strcpy(aSandboxDir,GetStorageBase());
		strcat(aSandboxDir,gAppName);
		#ifdef _DEBUG
		if (gInitOverrides) InitOverrides();
		strcat(aSandboxDir,gLocalOverride);
		#endif
		strcat(aSandboxDir,"/_cloud/");
		aFirst=false;
	}
	strcat(theResult,aSandboxDir);
}

void OS_Core::GetHomeFolder(char*theResult)
{
	static char aSandboxDir[MAX_PATH];
	static bool aFirst=true;
	if (aFirst)
	{
		strcpy(aSandboxDir,getenv("HOME"));
		strcat(aSandboxDir,"/");
		aFirst=false;
	}
	strcat(theResult,aSandboxDir);
}

void OS_Core::GetCacheFolder(char *theResult)
{
	if (gLegacySandbox) {GetSandboxFolder(theResult);strcat(theResult,"cache/");MakeDirectory(theResult);return;}

	strcpy(theResult,"/var/cache/");
	strcat(theResult,gAppName);
	strcat(theResult,"/");

	//GetTempPathA(MAX_PATH,aPath);
	//strcpy(theResult,aPath);
	//strcat(theResult,gAppName);
	//strcat(theResult,"\\cache\\");

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

	/*
	if (access(theFilename,0)==0)
	{
		struct stat statBuffer;
		if (stat(theFilename,&statBuffer)==0) return !S_ISDIR(statBuffer.st_mode);
	}
	return false;
	*/

	struct stat statBuffer;
	if(stat(theFilename,&statBuffer)!=0) return false;
	// Check if it is a regular file
	return !(S_ISDIR(statBuffer.st_mode));
}

char *OS_Core::ReadFile(char *theFilename, int *theBytesRead)
{
	//
	// #ifdef _PORT_FILES
	// Error!
	// #endif
	//

    _FixPath(theFilename);

	//OS_Core::Printf("#Read: %s",theFilename);

	errno=0;
	FILE* aFile = fopen(theFilename,"rb");
	if(aFile==NULL)
	{
		//OS_Core::Printf("unable[%d] to read file %s",errno, theFilename);
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

	//OS_Core::Printf("WRITEFILE: %s",theFilename);

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

	//OS_Core::Printf("Makedirectory1: %s",theDirectoryName);

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
				//aMakeDir[aPtr]='/';aPtr++;
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

	//OS_Core::Printf("DELETE DIRECTORY... [%s]",theDirectoryName);

	//
	// Okay, a very dangerous bit of code, here.
	//
	char aStartingDirectory[MAX_PATH];
	strcpy(aStartingDirectory,theDirectoryName);

	//OS_Core::Printf("Start @ [%s]",aStartingDirectory);


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

	//OS_Core::Printf("FindDirectory @ [%s]",aFindDirectory);

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

				//OS_Core::Printf("RemoveFile: [%s]",aFilename);

				remove(aFilename);
			}
			aDirEntPtr=readdir(aDirPtr);
		}
	}
	if(aDirPtr) closedir(aDirPtr);



	//OS_Core::Printf("rmdir: [%s]",theDirectoryName);

	if (aStartingDirectory[strlen(aStartingDirectory)-1]=='/') aStartingDirectory[strlen(aStartingDirectory)-1]=0;
	rmdir(aStartingDirectory);
//	rmdir(theDirectoryName);
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

namespace OS_Core
{
	int AlphabetizePartition(Array<char*>& theList,int theP,int theQ);
	void Alphabetize(Array<char*>& theList,int theStart=0,int theEnd=-1);
}

//
// Linux helper to put Enum results into alphabetical order.  Do they NEED to be in alphabetical order?
// Maybe... maybe not.  Put this in for bundler.  You have to define _SORTDIRECTORIES to make it happen.
//
#ifdef _SORTDIRECTORIES
int OS_Core::AlphabetizePartition(Array<char*>& theList,int theP,int theQ)
{
	char *aX=theList.Element(theP);
	int aI=theP;
	for(int aJ=theP+1;aJ<theQ;aJ++)
	{
		if(strcasecmp(theList.Element(aJ),aX)<0)
		{
			aI=aI+1;
			theList.Swap(aI,aJ);
		}
	}
	theList.Swap(aI,theP);
	return aI;
}

void OS_Core::Alphabetize(Array<char*>& theList,int theStart,int theEnd)
{
	if(theEnd==-1) theEnd=theList.Size();
	if(theStart<theEnd)
	{
		int aR=AlphabetizePartition(theList,theStart,theEnd);
		Alphabetize(theList,theStart,aR);
		Alphabetize(theList,aR+1,theEnd);
	}
}
#endif


void OS_Core::EnumDirectory(char *theDirectoryName, Array<char *>&theArray, bool includeSubdirs)
{

	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//

	_FixPath(theDirectoryName);

	//OS_Core::Printf("----------ENUMDIR: %s",theDirectoryName);

	char aStartingDirectory[MAX_PATH];
	strcpy(aStartingDirectory,theDirectoryName);

	if (strlen(aStartingDirectory)<2) return;
	if (aStartingDirectory[0]=='.') return;
	if (aStartingDirectory[1]==':')
	{
		if (strlen(aStartingDirectory)==3) return;
		if (strlen(aStartingDirectory)==2) return;
	}

	if (aStartingDirectory[strlen(aStartingDirectory)-1]!='\\' && aStartingDirectory[strlen(aStartingDirectory)-1]!='/') strcat(aStartingDirectory,"/");

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

	#ifdef _SORTDIRECTORIES
	Alphabetize(theArray);
	#endif
}

bool OS_Core::IsDirectory(char* theFilename)
{
	//
	// #ifdef _PORT_DIRECTORIES
	// Error!
	// #endif
	//

	_FixPath(theFilename);

	struct stat statBuffer;
	if(stat(theFilename,&statBuffer)!=0) return false;
	// Check if it is a regular file
	return (S_ISDIR(statBuffer.st_mode));

	/*
	struct stat st_buf;
	int status = stat (theFilename, &st_buf);
	if (status != 0) { return false; }
	if (S_ISDIR (st_buf.st_mode)) { return true; }
	return false;
	*/
}

std::string getLinuxDistributionName()
{
	std::ifstream file("/etc/os-release");
	std::string line;
	std::string distroName;

	if (file.is_open())
	{
		while (std::getline(file, line))
		{
			if (line.find("PRETTY_NAME=") == 0)
			{
				distroName = line.substr(13);
				if (!distroName.empty())
				{
					if (distroName.front() == '"') distroName=distroName.substr(1);
					if (distroName.back() == '"') distroName=distroName.substr(0,distroName.size()-1);
				}
				break;
			}
		}
		file.close();
	}
	else distroName="Unknown";
	return distroName;
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
	strcpy(theResult,getLinuxDistributionName().c_str());
}

char* OS_Core::GetOSQuarantine()
{
	// Just return a string for any specific specialized hardware that gets it own resolution/etc data, like "Steamdeck"
	return "";
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
	int aResult=0;
	char aCommand[MAX_PATH];
	sprintf(aCommand,"zenity --title=\"%s\" --text=\"%s\" ",theCaption,theText);


	switch (theType)
	{
	case 1:
		{
			strcat(aCommand,"--question");
			aResult=system(aCommand);
			int aStatus = WEXITSTATUS(aResult);
			if (aStatus==0) aResult=1;
			else if (aStatus==1) aResult=0;
		}
		break;
	case 2:
		{
			strcat(aCommand,"--question --extra-button=\"Cancel\"");
			aResult=system(aCommand);
			int aStatus = WEXITSTATUS(aResult);
			if (aStatus==0) aResult=1;
			else if (aStatus==1) aResult=0;
			else aResult=-1;
		}
		break;
	default:
		strcat(aCommand,"--info");
		system(aCommand);
		aResult=1;
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

	if (!gUsername)
	{
		char* aU=getenv("USER");
		size_t aLen=strlen(aU);
		if (aLen>0)
		{
			gUsername=new char[aLen];
			strcpy(gUsername,aU);
		}
		else
		{
			gUsername=(char*)"Anonymous";
		}
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
	int aResult=system(theCommand);
	int aStatus = WEXITSTATUS(aResult);
	return aStatus!=0;
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
	SDL_SetCursor(gCursor[gCurrentCursor]);
}

void OS_Core::Thread(void (*theFunction)(void *theArg), void* theArg)
{
    pthread_t aThread;
    pthread_attr_t aAttr;
    pthread_attr_init(&aAttr);
    pthread_attr_setdetachstate(&aAttr, PTHREAD_CREATE_DETACHED);
    pthread_create(&aThread,&aAttr,(void*(*)(void*))theFunction,theArg);
}

void OS_Core::SetThreadPriority(float thePriority)
{
	int aPriority=0;
	int aMax=sched_get_priority_max(SCHED_OTHER);
	int aMin=sched_get_priority_min(SCHED_OTHER);
	int aMid=(aMax+aMin)/2;

	if (thePriority>0) aPriority=aMid+(int)((thePriority*(float)aMid)+.5f);
	if (thePriority<0) aPriority=aMid-(int)((thePriority*(float)aMid)-.5f);

	sched_param aParam;
	aParam.sched_priority=aPriority;
	pthread_setschedparam(pthread_self(),SCHED_OTHER,&aParam);
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

	// FIXLINUX

	return "";

	//zenity --file-selection --file-filter="MP3 Files | *.mp3"

	/*
	std::string getFilenameFromZenity()
	{
		const char*command="zenity --file-selection --title=\"Select a file\"";
		std::array<char,128> buffer;
		std::string result;
		std::shared_ptr<FILE> pipe(popen(command,"r"),pclose);
		if(!pipe) throw std::runtime_error("popen() failed!");
		while(fgets(buffer.data(),buffer.size(),pipe.get())!=nullptr) { result+=buffer.data(); }
		// Remove trailing newline, if any
		if(!result.empty()&&result.back()=='\n') { result.pop_back(); }
		return result;
	}
	*/

}

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
void OS_Core::GetAppID(char* theResult)
{
	// Return any kind of unique GUID for this user.  Not needed for RwK tho.
	
	// return "000";
	return;
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
	if (PartnerQuery(HASH8("FORCEREZ"))) {isFullscreen=true;} // Never change resolution, it was set at start time!
	else
	{
		gResolutionWidth=theWidth;
		gResolutionHeight=theHeight;
	}
	gFullscreen=isFullscreen;

	if (gWindow)
	{
		SDL_SetWindowSize(gWindow,gResolutionWidth,gResolutionHeight);

		unsigned int aFlag=0;
		if (gFullscreen) aFlag=SDL_WINDOW_FULLSCREEN;
		//if (gFullscreen) aFlag=SDL_WINDOW_FULLSCREEN_DESKTOP;
		SDL_SetWindowFullscreen(gWindow,aFlag);
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

	struct ThreadLockStruct
	{
		pthread_mutex_t mLock;

		ThreadLockStruct()
		{
			pthread_mutexattr_t Attr;
			pthread_mutexattr_init(&Attr);
			pthread_mutexattr_settype(&Attr,PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&mLock,&Attr);
		}

		~ThreadLockStruct() { pthread_mutex_destroy(&mLock); }
    };

    Array<ThreadLockStruct *> gThreadLockList;

    int CreateThreadLock()
	{
        int aCount;
        for (aCount = 0; aCount < gThreadLockList.Size(); aCount++)
            if (gThreadLockList[aCount] == NULL)break;
        gThreadLockList[aCount] = new ThreadLockStruct;
        return aCount;
    }

    void ReleaseThreadLock(int theLock)
	{
        if (theLock>=0 && theLock<gThreadLockList.Size()) if(gThreadLockList[theLock])
		{
			delete gThreadLockList[theLock];
			gThreadLockList[theLock]=NULL;
		}
    }

    void ReleaseAllThreadLocks()
	{
        for (int aCount = 0; aCount < gThreadLockList.Size(); aCount++) ReleaseThreadLock(aCount);
		gThreadLockList.Reset();
    }

    void ThreadLock(int theLock)
	{
        if (theLock >= 0 && theLock<gThreadLockList.Size())
            if (gThreadLockList[theLock]) pthread_mutex_lock(&gThreadLockList[theLock]->mLock);
    }

    void ThreadUnlock(int theLock)
	{
        if (theLock >= 0 && theLock<gThreadLockList.Size())
            if (gThreadLockList[theLock]) pthread_mutex_unlock(&gThreadLockList[theLock]->mLock);
    }
}

/*
int OS_Core::CreateThreadLock() {int aCount;for (aCount=0;aCount<gThreadLockList.Size();aCount++) if (gThreadLockList[aCount]==NULL) break;gThreadLockList[aCount]=new ThreadLockStruct;return aCount;}
void OS_Core::ReleaseThreadLock(int theLock) {if (theLock>=0) if (gThreadLockList[theLock]) {delete gThreadLockList[theLock];gThreadLockList[theLock]=NULL;}}
void OS_Core::ReleaseAllThreadLocks() {for (int aCount=0;aCount<gThreadLockList.Size();aCount++) ReleaseThreadLock(aCount);gThreadLockList.Reset();}
void OS_Core::ThreadLock(int theLock) {if (theLock>=0) if (gThreadLockList[theLock]) gThreadLockList[theLock]->Lock();}
void OS_Core::ThreadUnlock(int theLock)  {if (theLock>=0) if (gThreadLockList[theLock]) gThreadLockList[theLock]->Unlock();}
*/

void OS_Core::Archive(char* theRootFolder, Array<char*>& theFiles, char* theZipFilename, float* theProgress)
{
}
void OS_Core::UnArchive(char* theRootFolder, char* theZipFilename, float* theProgress)
{
}

bool OS_Core::IsTouchDevice() {return Graphics_Core::IsFakePhone();}
void OS_Core::PopupKeyboard(bool doShow)
{
	#ifdef _DEBUG
	OS_Core::Printf("_OS_Core::PopupKeyboard(%d)",doShow);
	#endif
	if (Graphics_Core::IsFakePhone()) OS_Core::PushOSMessage(OS_Core::OS_OBSCURESCREENBOTTOM,0,(Graphics_Core::GetPageHeight()/2)*doShow);
	if (doShow) PartnerQuery(HASH8("SHOWKB!!"));
	else PartnerQuery(HASH8("HIDEKB!!"));
}

std::string CloudKeyEncode(const char* theFN, char theEncodeChar='~') 
{
	std::string aNewKey=theFN;
	// Encode however you want...
	return aNewKey;
}

std::string CloudKeyUnencode(const char* theFN, char theEncodeChar='~')
{
	std::string aNewKey=theFN;
	// Decode however you want...
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
	strcat(aPath,"/FakeCloud/");
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
	strcat(aPath,"/FakeCloud/");
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
	strcat(aPath,"/FakeCloud/");

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
	strcat(aPath,"/FakeCloud/");

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

bool gLogEnabled=false;
void OS_Core::LogEnable(bool theState) {gLogEnabled=theState;}
void OS_Core::Log(char *format, ...)
{
	if (!gLogEnabled) return;

	char aString[10000];
	va_list argp;
	va_start(argp, format);
	vsprintf(aString,format, argp);
	va_end(argp);

	static char aPath[MAX_PATH];
	static bool aFirst=true;
	if (aFirst)
	{
		GetAppStorageFolder(aPath);
		strcat(aPath,"log.txt");
		DeleteFile(aPath);
		aFirst=false;
		Log("--- Starting  %s --------------------------",gAppName);
	}

	FILE *aFile=fopen(aPath,"a");
	if (!aFile) return;
	fprintf(aFile, "%s\n", aString);
	fclose(aFile);
}


#ifndef _STEAM
void* PartnerQuery(longlong theHash,...)
{
	/*
	switch (theHash)
	{
		case HASH8("FORCEREZ"):
		{
			return (void*)1;
			break; // Force fullscreen for Steamdeck!
		}
		case HASH8("QUERYREZ"):
		{
			//PartnerQuery(HASH8("QUERYREZ"),&recv_w,&racv_h);
			va_list args;
			va_start(args, theHash);
			int* aW=va_arg(args, int*);
			int* aH=va_arg(args, int*);
			va_end(args);

			SDL_DisplayMode aMode;
			SDL_GetDesktopDisplayMode(0,&aMode);
			*aW=aMode.w;
			*aH=aMode.h;
		}
	}
	/**/
	return NULL;
}

void* PartnerQuery(char *theInfo, void* extraInfo) {return NULL;}
char* PartnerQueryString(char* theQuery, void* extraInfo) {return NULL;}
void PartnerQueryBytes(char* theQuery, void** thePtr, int* theSize) {}

#ifdef _DEBUG
void OS_Core::DebugBreak()
{
	OS_Core::Printf("...debugbreak");
}
#endif

#endif


#define __CPP
#include "../common.h"
#undef __CPP








