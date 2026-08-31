#include "HighScoreList.h"

HighScoreList::HighScoreList(void)
{
	mScoreList=NULL;
	mScoreCount=0;
	mReverseScoring=false;
}

HighScoreList::~HighScoreList(void)
{
	delete [] mScoreList;
}

void HighScoreList::SetScoreCount(int theCount)
{
	mScoreCount=theCount;
	mScoreList=new struct ScoreEntry[theCount];

	for (int aCount=0;aCount<theCount;aCount++)
	{
		mScoreList[aCount].mName="";
		mScoreList[aCount].mScore=0;
	}
}

void HighScoreList::Load(String theListName)
{
	mListName=theListName;
	Settings aRegistry;
	aRegistry.Load(theListName);
	for (int aCount=0;aCount<mScoreCount;aCount++)
	{
		String aFindString;
		aFindString.sprintf("NAME%d",aCount);if (aRegistry.Exists(aFindString)) mScoreList[aCount].mName=aRegistry.GetString(aFindString);
		aFindString.sprintf("SCORE%d",aCount);if (aRegistry.Exists(aFindString)) mScoreList[aCount].mScore=aRegistry.GetInt(aFindString);
		if (mScoreList[aCount].mScore==0) break;
	}
	aRegistry.Save();
}

void HighScoreList::Save()
{
	Settings aRegistry;
	aRegistry.Load(mListName);

	for (int aCount=0;aCount<mScoreCount;aCount++)
	{
		String aFindString;
		if (mScoreList[aCount].mScore!=0)
		{
			aFindString.sprintf("NAME%d",aCount);aRegistry.SetString(aFindString,mScoreList[aCount].mName);
			aFindString.sprintf("SCORE%d",aCount);aRegistry.SetInt(aFindString,mScoreList[aCount].mScore);
		}
	}

	aRegistry.Save();
}

bool HighScoreList::IsHighScore(int theScore)
{
	if (!mReverseScoring) 
	{
		for (int aCount=0;aCount<mScoreCount;aCount++) if (theScore>mScoreList[aCount].mScore || mScoreList[aCount].mScore==0) return true;
	}
	else 
	{
		for (int aCount=0;aCount<mScoreCount;aCount++) if (theScore<mScoreList[aCount].mScore || mScoreList[aCount].mScore==0) return true;
	}
	return false;
}

int HighScoreList::AddScore(String theName, int theScore)
{
	if (!IsHighScore(theScore)) return -1;

	if (!mReverseScoring) 
	{
		for (int aCount=0;aCount<mScoreCount;aCount++)
		{
			if (theScore>mScoreList[aCount].mScore || mScoreList[aCount].mScore==0)
			{
				//
				// Insert the score here... move everything else down
				//
				for (int aBackCount=mScoreCount-1;aBackCount>aCount;aBackCount--)
				{
					mScoreList[aBackCount].mName=mScoreList[aBackCount-1].mName;
					mScoreList[aBackCount].mScore=mScoreList[aBackCount-1].mScore;
				}

				mScoreList[aCount].mName=theName;
				mScoreList[aCount].mScore=theScore;
				return aCount;
				break;
			}
		}
	}
	else
	{
		for (int aCount=0;aCount<mScoreCount;aCount++)
		{
			if (theScore<mScoreList[aCount].mScore || mScoreList[aCount].mScore==0)
			{
				//
				// Insert the score here... move everything else down
				//
				for (int aBackCount=mScoreCount-1;aBackCount>aCount;aBackCount--)
				{
					mScoreList[aBackCount].mName=mScoreList[aBackCount-1].mName;
					mScoreList[aBackCount].mScore=mScoreList[aBackCount-1].mScore;
				}

				mScoreList[aCount].mName=theName;
				mScoreList[aCount].mScore=theScore;
				return aCount;
				break;
			}
		}
	}

	return -1;
}

String HighScoreList::GetName(int theSlot)
{
	if (theSlot<0 || theSlot>=mScoreCount) return "";
	return mScoreList[theSlot].mName;
}

int HighScoreList::GetScore(int theSlot)
{
	if (theSlot<0 || theSlot>=mScoreCount) return 0;
	return mScoreList[theSlot].mScore;
}

