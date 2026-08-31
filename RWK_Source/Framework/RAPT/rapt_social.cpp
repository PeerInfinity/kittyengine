#include "rapt_social.h"
#include "social_core.h"

Social gSocial;


Social::Social()
{
	mStarted=false;
	mIsHighestScore=false;
}

Social::~Social()
{
	if (mStarted) Stop();
}

void Social::Go()
{
	Social_Core::Startup();
	if (!mStarted)
	{
		mStarted=true;
		LoadLocalData();
	}
	mName=Social_Core::GetPlayerName();
}

void Social::Stop()
{
	Social_Core::Shutdown();
	mStarted=false;
	mName="";
}

void Social::SignOut()
{
	Social_Core::SignOut();
	mStarted=false;
	mName="";
}


bool Social::IsConnected() {return Social_Core::IsConnected();}
bool Social::IsConnecting() {return Social_Core::IsConnecting();}
void Social::ShowScores() {if (IsConnected()) Social_Core::ShowScores();}
void Social::ShowAchievements() {if (IsConnected()) Social_Core::ShowAchievements();}

String Social::GetName()
{
	if (mName.Len()<=0)
	{
		if (IsConnected()) mName=Social_Core::GetPlayerName();
		else mName=gAppPtr->GetUserName();
	
	}
	return mName;
}

void Social::SubmitScore(String theLeaderboard, String theName, int theScore)
{
	mIsHighestScore=false;
	if (theScore>0)
	{
		if (IsConnected()) Social_Core::SubmitScore(theLeaderboard.c(),theScore);
		SubmitLocalScore(theLeaderboard,theName,theScore);
	}
}

void Social::SubmitAchievement(String theAchievement, float thePercentComplete)
{
	thePercentComplete=gMath.Clamp(thePercentComplete);
	SubmitLocalAchievement(theAchievement,thePercentComplete);
  
    if (IsConnected()) Social_Core::SubmitAchievement(theAchievement.c(),thePercentComplete);
}

void Social::ResetLocalData()
{
	mLeaderboardList.Clear();
}

void Social::LoadLocalData()
{
	ResetLocalData();


	String aSocial="social://";
	MakeDirectory(aSocial);

	Array<String> aResult;
	EnumDirectoryFiles(aSocial,aResult);

	String aAchievementFile="_achievements.dat";

	for (int aCount=0;aCount<aResult.Size();aCount++)
	{
		if (aAchievementFile==aResult[aCount])
		{
		//
			// Load Achievements
			//
			IOBuffer aBuffer;
			aBuffer.Load("social://_achievements.dat");

			int aAchCount=aBuffer.ReadInt();
			for (int aAchCounter=0;aAchCounter<aAchCount;aAchCounter++)
			{
				Smart(Achievement) aA=new Achievement;
				mAchievementList+=aA;
				aA->mName=aBuffer.ReadString();
				aA->mPercent=aBuffer.ReadFloat();
			}
		}
		else
		{
			//
			// Load Leaderboard
			//
			IOBuffer aBuffer;
			aBuffer.Load(Sprintf("social://scores.%s",aResult[aCount].c()));

			Smart(Leaderboard) aLB=new Leaderboard;
			mLeaderboardList+=aLB;
			aLB->mName=aBuffer.ReadString();
			int aHSCount=aBuffer.ReadInt();

			for (int aHSCounter=0;aHSCounter<aHSCount;aHSCounter++)
			{
				Smart(HighScore) aHS=new HighScore;
				aHS->mName=aBuffer.ReadString();
				aHS->mScore=aBuffer.ReadInt();
				aLB->mScoreList+=aHS;
			}
		}
	}
}

void Social::SubmitLocalScore(String theLeaderboard, String theName, int theScore)
{
	Smart(Leaderboard) aBoard=NULL;
	EnumSmartList(Leaderboard,aL,mLeaderboardList) if (aL->mName==theLeaderboard)
	{
		aBoard=aL;
		break;
	}
	if (aBoard.IsNull()) {aBoard=new Leaderboard;aBoard->mName=theLeaderboard;mLeaderboardList+=aBoard;}

	Smart(HighScore) aHS=new HighScore;
	aHS->mName=theName;
	aHS->mScore=theScore;

	if (aBoard->mScoreList.GetCount()==0) aBoard->mScoreList+=aHS;
	else
	{
		//
		// What about descending score?
		//
		bool aInserted=false;
		for (int aCount=0;aCount<aBoard->mScoreList.GetCount();aCount++)
		{
			Smart(HighScore) aCheckHS=aBoard->mScoreList[aCount];
			if (theScore>=aCheckHS->mScore) 
			{
				aBoard->mScoreList.Insert(aHS,aCount);
				aInserted=true;
				if (aCount==0) mIsHighestScore=true;
				break;
			}
		}
		if (!aInserted) aBoard->mScoreList+=aHS;
		while (aBoard->mScoreList.GetCount()>25) aBoard->mScoreList-=aBoard->mScoreList.FetchLast();
	}

	IOBuffer aBuffer;
	aBuffer.WriteString(aBoard->mName);
	aBuffer.WriteInt(aBoard->mScoreList.GetCount());

	EnumSmartList(HighScore,aHS,aBoard->mScoreList)
	{
		aBuffer.WriteString(aHS->mName);
		aBuffer.WriteInt(aHS->mScore);
	}
	aBuffer.CommitFile(Sprintf("social://_scores.%s.dat",theLeaderboard.ToLower().c()));
}

bool Social::SubmitLocalAchievement(String theAchievement, float thePercentComplete)
{
	Smart(Achievement) aA=NULL;
	EnumSmartList(Achievement,aAch,mAchievementList)
	{
		if (aAch->mName==theAchievement)
		{
			aA=aAch;
			break;
		}
	}
	if (aA.IsNull())
	{
		aA=new Achievement;
		aA->mName=theAchievement;
		aA->mPercent=0;
		mAchievementList+=aA;
	}

	float aLastPercent=aA->mPercent;
	aA->mPercent=_max(aA->mPercent,thePercentComplete);

	IOBuffer aBuffer;
	aBuffer.WriteInt(mAchievementList.GetCount());
	EnumSmartList(Achievement,aAc,mAchievementList)
	{
		aBuffer.WriteString(aAc->mName);
		aBuffer.WriteFloat(aAc->mPercent);
	}
	aBuffer.CommitFile("social://_achievements.dat");

	return (aA->mPercent>aLastPercent);
}

bool Social::DoesLeaderboardExist(String theLeaderboard)
{
	EnumSmartList(Leaderboard,aL,mLeaderboardList) if (aL->mName==theLeaderboard) return true;
	return false;
}

void Social::GetAllScores(String theLeaderboard, Array<String>& theNameArray, Array<int>& theScoreArray)
{
	theNameArray.Reset();
	theScoreArray.Reset();

	EnumSmartList(Leaderboard,aL,mLeaderboardList)
	{
		if (aL->mName==theLeaderboard)
		{
			EnumSmartList(HighScore,aHS,aL->mScoreList)
			{
				theNameArray+=aHS->mName;
				theScoreArray+=aHS->mScore;
			}
			break;
		}
	}
}

float Social::GetAchievement(String theName)
{
	EnumSmartList(Achievement,aC,mAchievementList) if (aC->mName==theName) return aC->mPercent;
	return 0;
}

void Social::GetAllAchievements(Array<String>& theNameArray, Array<float>& thePercentArray)
{
	theNameArray.Reset();
	thePercentArray.Reset();
	EnumSmartList(Achievement,aC,mAchievementList)
	{
		theNameArray+=aC->mName;
		thePercentArray+=aC->mPercent;
	}
}

bool Social::IsHighScore(String theLeaderboard, int theScore, int theMaxListNumber)
{
	Smart(Leaderboard) aBoard=NULL;
	EnumSmartList(Leaderboard,aL,mLeaderboardList) if (aL->mName==theLeaderboard)
	{
		aBoard=aL;
		break;
	}
	if (aBoard.IsNull()) return true;
	if (aBoard->mScoreList.GetCount()==0) return true;

	for (int aCount=0;aCount<theMaxListNumber;aCount++)
	{
		Smart(HighScore) aCheckHS=aBoard->mScoreList[aCount];
		if (aCheckHS.IsNull()) return true;
		if (theScore>=aCheckHS->mScore) return true;
	}


	return false;
}

