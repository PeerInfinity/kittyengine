#include "Game.h"
#include "MyApp.h"
#include "MainMenu.h"

Game *gGame=NULL;
String gTempSaveName;


Game::Game(void)
{
	Size();
	gGame=this;
}

Game::~Game(void)
{
	if (gWorld) gWorld->Kill();
	if (gGame==this) gGame=NULL;
}

void Game::Kill()
{
	CPUPlus::Kill();
	if (gWorld) 
	{
		if (!gWorldEditor) Save();
	}
}

void Game::Initialize()
{
}

void Game::StartLevel(String theName)
{
	gApp.IncrementPlayCounter();
	gOut.Out("Play Count: %d",gApp.GetPlayCounter());

	gApp.SaveSettings();

	mLevelName=theName;
	World *aWorld=new World;
	*this+=aWorld;

	//if (theName.Contains('\\')) aWorld->LoadLevel(PointAtPackage(Sprintf("%s.kitty",theName.c())));
	//else aWorld->LoadLevel(PointAtPackage(Sprintf("data\\%s.kitty",theName.c())));

	gOut.Out("Load Level: [%s]",theName.c());
	bool aOld=true;if (gMakermall) aOld=false;
	aWorld->LoadLevel(theName.c(),aOld);
}

void Game::RestartLevel()
{
	gWorld->Kill();
	StartLevel(mLevelName);
}


void Game::StartEditor(String theName)
{
	WorldEditor *aWorld=new WorldEditor;
	*this+=aWorld;

	if (theName[0]=='.') 
	{
		aWorld->mName="";
		aWorld->Load("");
	}
	else 
	{
		aWorld->Load(Sprintf("sandbox://EXTRALEVELS64\\%s.kitty",theName.c()));
		aWorld->mName=theName;
		aWorld->mLevelname=theName;
	}
}

void Game::Update()
{
}

void Game::DrawOverlay()
{
	//
	// Draw the controls...
	//
}

void Game::Save()
{
	//
	// Okay, we are saving... but we need to save it properly...
	//FUNK
	//

	if (!mSaveName.Len()) return;


	_DLOG("Resume FN: [%s]",mSaveName.c());

	if (gWorld) if (gWorld->mWin) 
	{
		// FIXME... what is this?
		//if (gApp.mIsKittyConnect) KittyConnect::NoResume();
		_DLOG("Won... delete resume file...");
		String aWork="sandbox://";aWork+=mSaveName;
		DeleteFile(aWork);
		//DeleteFile(PointAtSandbox(Sprintf("tempsave64%d.sav",gApp.mGameType)));
		return;
	}

	SaveGame aGame;
	String aWork="sandbox://";aWork+=mSaveName;
	aGame.Saving(aWork);
	Sync(aGame);
	aGame.Commit();
}

void Game::Load()
{
	if (!gTempSaveName.Len()) return;
	SaveGame aGame;

	String aWork="sandbox://";aWork+=gTempSaveName;
	aGame.Loading(aWork);

	World *aWorld=new World;
	*this+=aWorld;

	Sync(aGame);
	aGame.Commit();
	
	aWorld->PositionButtons();
}

void Game::Sync(SaveGame &theSG)
{
	theSG.StartChunk(); // Game uber-info chunk
	theSG.Sync(&gApp.mGameType);
	theSG.Sync(&mLevelName);
	theSG.EndChunk();

	theSG.StartChunk(); // World chunk
	SaveGame &aSG=theSG.GetNestedSaveGame("World");
	if (gWorld) gWorld->Sync(aSG);
	theSG.EndChunk();

}

