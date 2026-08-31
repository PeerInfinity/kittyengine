#include "social_core.h"
#include "os_core.h"
#include "graphics_core.h"

#include <shellapi.h>
#include <shlobj.h>

#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <string.h>
#include <stdio.h>

#ifdef _STEAM
#include "social_core_steam.cpp"
#else
namespace Social_Core
{
	void			Startup()
	{
	}

	void			Shutdown()
	{
	}

	void			SignOut()
	{
	}


	void			Pump()
	{
	}

	bool			IsConnected()
	{
		return false;
	}

	bool			IsConnecting()
	{
		return false;
	}

	char*			GetPlayerName()
	{
		return "";
	}

	void			ShowScores()
	{
	}

	void			ShowAchievements()
	{
	}

	void			SubmitScore(char* theLeaderboard, int theScore)
	{
	}

	void			SubmitAchievement(char *theAchievement, float thePercentComplete)
	{
		//
		// On iOS, remember to multiply percent complete by 100!
		//
		//OS_Core::MessageBox("ACHIEVEMENT!",theAchievement,0);
	}
}
#endif