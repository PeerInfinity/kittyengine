#ifdef _STEAM

#ifdef _DEBUG
#define _VERBOSE
#endif

#include <iostream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Moving to Steam:
// 1. Create Steam Debug/Steam Release builds
// 2. define _STEAM in the preprocessor directives for RAPT
// 3. Drag steam_api.lib into project (framework/os/steam/redistributable_bin)
// 4. Put steam_api.dll in same folder as resultant exe
// 5. Put steam_appid.txt in same folder as exe, with just text of the steam app ID in it (just one line, only text in the file)
//    Note: Actual app ID can mess up, use "480" as the generic testing app id.
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Social_Core
{
	bool gSteamOK;
	bool gConnecting=false;
	
	int64 gSteamAppID;

	class SteamHandler
	{
	public:
		STEAM_CALLBACK(SteamHandler, OnUserStatsReceived, UserStatsReceived_t, mUserStatsReceived);

		SteamHandler() :
		mUserStatsReceived( this, &SteamHandler::OnUserStatsReceived )
			//m_CallbackUserStatsStored( this, &CSteamAchievements::OnUserStatsStored ),
			//m_CallbackAchievementStored( this, &CSteamAchievements::OnAchievementStored )
		{
		}

	};

	void SteamHandler::OnUserStatsReceived(UserStatsReceived_t *theCallback)
	{
	}

	SteamHandler* gSteamHandler=NULL;

	void			Startup()
	{
	}

	void			Shutdown()
	{
	}

	void			Pump()
	{
	}

	void			SignOut()
	{
		// Thou canst not ever escapeth Steam
	}


	bool			IsConnected()
	{
		return gSteamOK;
	}
	
	bool			IsConnecting()
	{
		return gConnecting;
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
#endif
