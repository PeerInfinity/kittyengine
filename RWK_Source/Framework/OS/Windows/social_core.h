#include "os_headers.h"

#pragma comment (lib,"winmm.lib")

#include "util_core.h"
#include "leaktester.h"

namespace Social_Core
{
	//
	// Startup and shutdown of THE SOCIAL
	//
	void			Startup();
	void			Shutdown();
	void			SignOut();
	void			Pump();

	//
	// Tells if we're connected to THE SOCIAL.  If we're not, everything still works on a local level.
	//
	bool			IsConnected();
	bool			IsConnecting();
	
	//
	// Gets name of player
	//
	char*			GetPlayerName();

	//
	// Shows the system score and achievement screens.  If not connected, these will not
	// do anything.
	//
	void			ShowScores();
	void			ShowAchievements();

	//
	// Lets us submit scores and achievements to THE SOCIAL
	//
	void			SubmitScore(char* theLeaderboard, int theScore);
	void			SubmitAchievement(char *theAchievement, float thePercentComplete);
}
