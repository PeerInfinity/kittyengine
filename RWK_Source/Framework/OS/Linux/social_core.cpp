#include "social_core.h"
#include "os_core.h"
#include "graphics_core.h"
#include <fcntl.h>
#include <string.h>
#include <stdio.h>


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
		// return (PartnerQuery(HASH8("ISREADY?"))!=NULL);
		return (false);
	}

	bool			IsConnecting()
	{
		return (false);
		// return (PartnerQuery(HASH8("ISPENDING?"))!=NULL);
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
		// PartnerQuery(HASH8("ACHIEVE!"),theAchievement,thePercentComplete);
	}
}
