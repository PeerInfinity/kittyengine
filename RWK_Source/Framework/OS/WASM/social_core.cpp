#include "social_core.h"
#include "os_core.h"
#include "graphics_core.h"
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#define __HEADER
#include "common.h"
#undef __HEADER


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
	}
}
