#include "..\..\source\myapp.h"
#include "os_core.h"
#include <process.h>

// ===== P0prime: the headless-oracle tape hook =====
// World.cpp reads these three (extern) instead of polling the six keys.  They
// are defined by whichever main() the platform builds -- Project/Linux/main.cpp
// on Linux, main_wasm.cpp under emscripten -- so the Windows entry point has to
// define them too or World.cpp will not link.  The Windows build has no
// --oracle driver, so gOracleActive simply stays false and the game polls keys
// exactly as before.
bool gOracleActive=false;
int  gOracleHeld=0;
int  gOraclePressed=0;
// ==================================================

MyApp gApp;

void AppThread(void *theArg)
{
	gApp.Go();
}

//
// Windows entry point...
//
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
#ifdef AppRunsInThread

	//
	// Note: This Winmain tries to emulate the behavior of Cocoa and other visual systems, where
	// the startup and shutdown will be handled by an overarching class.  On those systems, AppThread will
	// usually be invoked in a function within that overarching class.
	//

	OS_Core::Startup();
	_beginthread(AppThread,0,NULL);

	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);		

	while (!gApp.IsShutdown()) {OS_Core::Pump();OS_Core::Sleep(1);}
	OS_Core::Shutdown();
#else

	//
	// Note: This Winmain runs threadless. Use it on systems when you can use something like PeekMessage
	// to push message processing.
	//

	OS_Core::Startup();
	AppThread(NULL);
	OS_Core::Shutdown();

#endif
}

