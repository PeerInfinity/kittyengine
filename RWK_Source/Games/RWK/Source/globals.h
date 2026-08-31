#ifndef GLOBALS
#define GLOBALS

#define PATCHED

#define gGCenter() (HalfSize()+Point(mX,mY))

#define CONVEYOR_SPEED (gWorld->mConveyorSpeed*5.0f)

#define NOKILLBIT 0x8000000000000000

#define PRE_2022_GAME

#define _LOG(...) gOut.Out(__VA_ARGS__)
#ifdef _DEBUG
#define _DLOG(...) gOut.Out(__VA_ARGS__)
#else 
#define _DLOG(...)
#endif

#define _OVERLOG(...) gOut.Out(__VA_ARGS__)
//#define _OVERLOG(...)


#define MAKERMALL_GAMEID 0xBE40
#define VERSION							"10.0"
#define SAVEGAME_VERSION				0x0010

#define COIN 58
#define COINDOOR 59

///////////////////////////////////////////////////////////////
//
// Global defines and whatnot...
// Put anything here that you'll reference everywhere.
//
///////////////////////////////////////////////////////////////

#define MAX_PLAYERS			4

///////////////////////////////////////////////////////////////
//
// End of globals...
//
///////////////////////////////////////////////////////////////

//
// If you have something common but need to localize it across all OS's, here's the place to do it.
// Usually this is for turning a game string like ACHIEVEMENT_WONGAME into whatever the local version is for
// whatever system.
//
inline char* LocalizeOS(char* theString)
{
	return theString;
}

//
// Depreciated...
//
#include <string>
inline char* GetStartupCommand()
{
	static std::string aResult;
	return (char*)aResult.c_str();
}

#endif


