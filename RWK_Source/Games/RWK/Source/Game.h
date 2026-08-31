
#pragma once
#include "rapt.h"
#include "World.h"
#include "WorldEditor.h"
#include "MyApp.h"

class Game :
	public CPUPlus
{
public:
	Game(void);
	virtual ~Game(void);

	void			Initialize();
	void			Update();
	void			DrawOverlay();

	void			StartLevel(String theName);
	void			StartEditor(String theName);

	void			RestartLevel();

	void			Save();
	void			Load();
	void			Sync(SaveGame &theSG);

	void			Kill();

public:

	String			mLevelName;
	String			mSaveName;

};

extern Game *gGame;
extern String gTempSaveName;
