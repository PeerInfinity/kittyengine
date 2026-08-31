#pragma once
#include "rapt.h"

class HighScore {public: String mName;int mScore;};
class Achievement {public: String mName;float mPercent;};

class Leaderboard
{
public:
	String					mName;
	SmartList(HighScore)	mScoreList;
};

class Social
{
public:
	Social();
	virtual ~Social();

	void					Go();
	void					Stop();
	void					SignOut();

public:

	bool					mStarted;
	bool					IsConnected();
	bool					IsConnecting();

	String					mName;
	String					GetName();

	//
	// These invoke system functions for showing scores and achievements.
	// If you are not connected, they will do nothing.
	//
	void					ShowScores();
	void					ShowAchievements();

	//
	// These submit scores and achievements.  Local copies are kept so that you
	// can query things for non-connected use.
	//
	bool					mIsHighestScore;
	inline bool				IsHighestScore() {return mIsHighestScore;}	// Only works after SubmitScore
	void					SubmitScore(String theLeaderboard, String theName, int theScore);
	void					SubmitAchievement(String theAchievement, float thePercentComplete=1.0f);

	//
	// Tells us if a score is higher than a certain position in the score list
	//
	bool					IsHighScore(String theLeaderboard, int theScore, int theMaxListNumber=5);

public:
	//
	// This stuff is for easy local management of scores, achievements, etc, that will be
	// consistent with submitting it to game centers and stuff, while at the same time making
	// it fetchable for local games.
	//

	SmartList(Leaderboard)	mLeaderboardList;
	SmartList(Achievement)	mAchievementList;

	void					ResetLocalData();
	void					LoadLocalData();

	//
	// You don't need to call these, they get called automatically by SubmitScore and SubmitAchievement
	// Or, you can call them to store something locally, if you want.
	//
	void					SubmitLocalScore(String theLeaderboard, String theName, int theScore);
	bool					SubmitLocalAchievement(String theAchievement, float thePercentComplete);

	//
	// Queries to fetch score stuff
	//
	bool					DoesLeaderboardExist(String theLeaderboard);
	void					GetAllScores(String theLeaderboard, Array<String>& theNameArray, Array<int>& theScoreArray);
	float					GetAchievement(String theName);
	void					GetAllAchievements(Array<String>& theNameArray, Array<float>& thePercentArray);
};

extern Social gSocial;
