#pragma once

#include "rapt.h"

class HighScoreList
{
public:
	HighScoreList(void);
	virtual ~HighScoreList(void);

	void				SetScoreCount(int theCount);

	void				Load(String theListName);
	void				Save();

	bool				IsHighScore(int theScore);
	int					AddScore(String theName, int theScore);

	String				GetName(int theSlot);
	int					GetScore(int theSlot);

public:
	int					mScoreCount;
	String				mListName;

	struct ScoreEntry
	{
		String			mName;
		int				mScore;
	};

	struct ScoreEntry	*mScoreList;

	bool				mReverseScoring;		// Sort bottom to top
};
