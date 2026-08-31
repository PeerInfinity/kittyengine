//#pragma once
#include "rapt.h"

#include "MyApp.h"
#include "World.h"
#include "WorldEditor.h"
#include "Anim.h"
#include "Pathfinder_Atomica.h"
#include "MainMenu.h"
#include "Game.h"

//Tracer gTrace;

#define ZOINGWAIT 15

float gMessageYMod=0;


World *gWorld=NULL;
extern bool gOracleActive;   // P0prime
extern int  gOracleHeld;     // P0prime
extern int  gOraclePressed;  // P0prime
bool gSwipeScroll=false;
int gSwipeScrollHash=0;

bool gNoFixing=false;
int FixMe(int theMe)
{
	if (gNoFixing) return theMe;
	switch (theMe)
	{
	case 0:return 99;
	case 15:return 14;
	case 17:return 16;
	case 19:return 18;
	case 33:return 99;
	case 34:return 99;
	case 35:return 99;
	}
	return theMe;
}



World::World(void)
{
	//gTrace.Out("World::World (in)");
	mInGameMenu=NULL;
	mLastDisabledState=gApp.mDisableCustomMusic;
	mKitty=NULL;
	mConveyorTick=0;
	mMusicStarted=false;
	mNoMoreKitty=false;
	mConveyorSpeed=.5f;//2.5f;
	mWantConveyorSpeed=mConveyorSpeed;
	mCoins=0;
	mMaxCoins=0;
	mID=123456;
	mTesting=false;
	Size();
	gWorld=this;
	mWin=false;
	mFastTimeCountdown=0;
	mDied=false;
	mNeverPlayMusic=false;
	mScrollSpeedCapped=true;
	mPlayTime=0;

	mHideUI=false;

	//262144
	//Max Paint

	mPausedPlayerList.Unique();

	mHalfGrid=Point(mGridSize/2,mGridSize/2);
	mGrid=NULL;
	mLevelMap=NULL;
	mSuperGrid=NULL;
	mIsEditor=false;
	mOverGrid=NULL;

	mZoom=.9f;
	
	mInitialZoom=true;
	mInitialZoomSpeed=.005f;
	mFlashWhite=0;
	mShake=0;
	//mScrollSpeed=.1f;
	mScrollSpeed=.25f;

	mMessageFade=0.0f;
	mPlayMusic=true;
	mPauseCheckCountdown=0;
	
	mZoom=1.0f;
	mInitialZoom=false;
	mRemoveBossBlockCountdown=0;
	mRemoveBitcoinBlockCountdown=0;

	mDarkness=1.0f;
	mDarkFadeDir=-.1f;

	mTimer=0;
	mInitialTimerPause=(5*50);
	mTimerPause=0;

	//gTrace.Out("World::World (out)");
}

World::~World(void)
{
	if (gWorld==this) gWorld=NULL;
	mPlayerList.Free();
	mAnimatorList_Top.Free();
	mAnimatorList_UnderPlayers.Free();
	mAnimatorList_UnderWorld.Free();

	if (mPlayMusic) gApp.FadeAllMusicExcept(NULL);//gApp.mMusic.FadeOut(1);
	delete [] mGrid;
	delete [] mSuperGrid;
	delete [] mOverGrid;

	if (mPlayMusic && gWorldEditor)
	{
		gApp.mMusic_Gameplay[0]->Stop();
		gApp.mMusic_Gameplay[1]->Stop();
		gApp.mMusic_Gameplay[2]->Stop();
		gApp.mMusic_Gameplay[3]->Stop();
	}


	_FreeList(Player,mPausedPlayerList);
	_FreeList(Exploder,mExploderList);
	_FreeList(IPoint,mBossBlockList);
	_FreeList(IPoint,mBitcoinBlockList);
	_FreeList(struct Crack,mCrackList);
}

extern TitleBox* gTitleBox;
void World::Initialize()
{
	if (gMakermall) 
	{
		gMakermall->mInstaplay=false;
	}

	mX+=gReserveLeft;mWidth-=gReserveRight+gReserveLeft;

	StartMusic();
	//gTrace.Out("World::Initialize (in)");

	FocusBackButton();
	if (mPlayMusic) 
	{
		gApp.FadeInMusic(gApp.mMusic_Gameplay[1]);
	}
	mCurrentMusic=1;
	mNewMusic=1;
	mSpawnMusic=1;
	mNewMusic=1;
	mMusicDelay=0;
	mWantNewMusic=false;

	mButtons.AddButton(mButton_Move,Rect(2,252,191,66),true);
	mButtons.AddButton(mButton_Jump,Rect(412,253,66,65));
	mButtons.AddButton(mButton_Shoot,Rect(342,253,66,65),true);
	mButtons.AddButton(mButton_Rocket,Rect(272,253,66,65),true);
	mButtons.AddButton(mButton_RocketUp,Rect(412,183,66,65),true);
	
	mButtons.EnableButton(mButton_Jump,false);
	mButtons.EnableButton(mButton_Shoot,false);
	mButtons.EnableButton(mButton_Rocket,false);
	mButtons.EnableButton(mButton_RocketUp,false);

	mMenuButton.Size(32,32);
	mMenuButton.CenterAt(15,16);
	mMenuButton.SetSounds(&gSounds->mClick);
	mMenuButton.SetNotify(this);
	*this+=&mMenuButton;


/*
	if (gWorldEditor)
	{
		mDarkness=0;
		AddMessage("TOUCH ROBOT","TO END TESTING",1.5f);
	}
	else
	{
		AddMessage("TOUCH ROBOT","TO ACCESS MENU OR MAP",1.5f);
	}
*/

	PositionButtons();
	//gTrace.Out("World::Initialize (out)");
}


Point World::GridToWorld(int theX, int theY)
{
	return Point(theX*mGridSize,theY*mGridSize)+mHalfGrid;
}

Rect World::GridToWorldRect(int theX, int theY)
{
	Point aCenter=GridToWorld(theX,theY);
	return Rect((float)(aCenter.mX-mHalfGrid.mX),(float)(aCenter.mY-mHalfGrid.mY),(float)mGridSize,(float)mGridSize);
}


IPoint World::WorldToGrid(float theX, float theY)
{
	if (theX<0) theX-=40;
	if (theY<0) theY-=40;
	return IPoint((int)(theX/mGridSize),(int)(theY/mGridSize));
}

void World::InitializeGrid(int theWidth, int theHeight)
{
	mGridWidth=theWidth;
	mGridHeight=theHeight;
	mGrid=new Grid[mGridWidth*mGridHeight];
	mLevelMap=new MAPTYPE[mGridWidth*mGridHeight];
	memset(mGrid,0,mGridWidth*mGridHeight*sizeof(Grid));
	memset(mLevelMap,0,mGridWidth*mGridHeight*sizeof(MAPTYPE));

	mSuperGridWidth=(theWidth/SUPERGRID_X)+1;
	mSuperGridHeight=(theHeight/SUPERGRID_Y)+1;
	mSuperGrid=new SuperGrid[mSuperGridWidth*mSuperGridHeight];
}

void World::SetGrid(int theX, int theY, char theValue)
{
	if (theX<0 || theY<0) return;
	if (theX>=mGridWidth || theY>=mGridHeight) return;

	Grid* aGridPtr=&mGrid[theX+(theY*mGridWidth)];
	aGridPtr->mLayout=theValue;

	mLevelMap[theX+(theY*mGridWidth)]=FixMe(theValue);
}

void World::SetGridEX(int theX, int theY, char theValue)
{
	Grid* aG=GetGridPtr(theX,theY);
	if (aG)
	{
		_DLOG("Melt Layout: %d",aG->mLayout);
		switch (aG->mLayout)
		{
		case 37:
			{
				EnumList(IPoint,aIP,mBossBlockList)
				{
					if (aIP->mX==theX && aIP->mY==theY)
					{
						mBossBlockList-=aIP;
						delete aIP;
						break;
					}
				}
			}
			break;
		case 58:
			{
				EnumList(IPoint,aIP,mBitcoinBlockList)
				{
					if (aIP->mX==theX && aIP->mY==theY)
					{
						mBitcoinBlockList-=aIP;
						delete aIP;
						break;
					}
				}
			}
			break;
		case 48:
			{
				Point aPos=GridToWorld(theX,theY);
				EnumList(Player,aP,mPlayerList)
				{
					if (aP->mID==ID_GUNTARGET)
					{
						Rect aGTRect=Rect(aP->mPos.mX-35,aP->mPos.mY-20,70,40);
						if (aGTRect.ContainsPoint(aPos.mX,aPos.mY))
						{
							GunTarget* aGT=(GunTarget*)aP;
							aGT->Hit();
							OpenVirusDoor(IPoint(theX,theY));

							aGT->Kill();
						}
					}
				}
			}
			break;
		}
	}
	SetGrid(theX,theY,theValue);
}


int World::GetGrid(int theX, int theY)
{
	if (theX<0 || theY<0) return 0;
	if (theX>=mGridWidth || theY>=mGridHeight) return 0;
	return mGrid[theX+(theY*mGridWidth)].mLayout;
}

World::Grid *World::GetGridPtr(int theX, int theY)
{
	if (theX<0 || theY<0) return NULL;
	if (theX>=mGridWidth || theY>=mGridHeight) return NULL;
	return &mGrid[theX+(theY*mGridWidth)];
}


bool World::IsBlocked(int theX, int theY, bool lavaBlocks)
{
	int aGrid=GetGrid(theX,theY);
	if (aGrid==54 || aGrid==55 || aGrid==COSMIC_BLOCK || aGrid==SLAM_BLOCK || aGrid==SECRET_BLOCK || aGrid==MONSTERENERGY) 
	{
		return false;
	}
	if (ISLAVA(aGrid))
	{
		if (lavaBlocks) return true;
		return false;
	}
	if (aGrid!=0 && (aGrid!=33 && aGrid!=34 && aGrid!=35)) return true;
	return false;
}

bool World::IsBlockedDir(int theX, int theY, float theDir, bool lavaBlocks)
{
	int aGrid=GetGrid(theX,theY);
	if (aGrid==0 || aGrid==COSMIC_BLOCK || aGrid==SLAM_BLOCK || aGrid==SECRET_BLOCK || aGrid==MONSTERENERGY) return false;
	if (ISLAVA(aGrid))
	{
		if (lavaBlocks) return true;
		return false;
	}
	if (aGrid!=0 && (aGrid!=33 && aGrid!=34 && aGrid!=35)) 
	{
		//
		// Check theDir and the grid type...
		// 54 = Wall pushes left
		// 55 = Wall pushes right
		//

		if (aGrid==54 && theDir<0) return false;
		if (aGrid==55 && theDir>0) return false;
		return true;
	}
	return false;
}



bool World::IsBlockedGround(int theX, int theY, bool lavaBlocks)
{
	int aGrid=GetGrid(theX,theY);
	if (aGrid==SECRET_BLOCK || aGrid==MONSTERENERGY) return false;
//	if (aGrid==54 || aGrid==55) return true;
	if (ISLAVA(aGrid))
	{
		if (lavaBlocks) return true;
		return false;
	}
	if (aGrid!=0 && aGrid!=COSMIC_BLOCK && (aGrid!=33 && aGrid!=34 && aGrid!=35)) return true;
	return false;
}

bool World::IsBlockedY(int theX, int theY, float theYDir, bool lavaBlocks)
{
	int aGrid=GetGrid(theX,theY);
	if (aGrid==54 || aGrid==55 || aGrid==64 || aGrid==SECRET_BLOCK || aGrid==MONSTERENERGY || aGrid==SPIKE_BLOCK || aGrid==SPIKE_CEILING_BLOCK) return false;
	//	if (aGrid==54 || aGrid==55) return true;
	if (ISLAVA(aGrid))
	{
		if (lavaBlocks) return true;
		return false;
	}
	if (aGrid!=0 && aGrid!=COSMIC_BLOCK && (aGrid!=33 && aGrid!=34 && aGrid!=35)) return true;
	return false;
}

bool World::IsBlockedOneWayWall(int theX, int theY, float theDir)
{
	int aGrid=GetGrid(theX,theY);
	if (aGrid==54 || aGrid==55)
	{

		if (aGrid==54 && theDir<0) return false;
		if (aGrid==55 && theDir>0) return false;
		return true;
	}
	return false;
}

void World::StartMusic()
{
	if (mLastDisabledState!=gApp.mDisableCustomMusic)
	{
		gApp.mMusic_Gameplay[0]->Stop();
		gApp.mMusic_Gameplay[1]->Stop();
		gApp.mMusic_Gameplay[2]->Stop();
		gApp.mMusic_Gameplay[3]->Stop();
		mLastDisabledState=gApp.mDisableCustomMusic;
	}

	gApp.mMusic_Gameplay[0]=&gApp.mSounds.mStream_Music_Gameplay1;
	gApp.mMusic_Gameplay[1]=&gApp.mSounds.mStream_Music_Gameplay1;
	gApp.mMusic_Gameplay[2]=&gApp.mSounds.mStream_Music_Gameplay2;
	gApp.mMusic_Gameplay[3]=&gApp.mSounds.mStream_Music_Gameplay3;

	if (!gApp.mDisableCustomMusic)
	{
		if (mCustomSong[0].Len())
		{
			if (gApp.mCustomMusic[0].IsLoaded()) gApp.mCustomMusic[0].Unload();
			gApp.mCustomMusic[0].Load(mCustomSong[0]);
			gApp.mMusic_Gameplay[0]=&gApp.mCustomMusic[0];
			gApp.mMusic_Gameplay[1]=&gApp.mCustomMusic[0];
		}
		if (mCustomSong[1].Len())
		{
			if (gApp.mCustomMusic[1].IsLoaded()) gApp.mCustomMusic[1].Unload();
			gApp.mCustomMusic[1].Load(mCustomSong[1]);
			gApp.mMusic_Gameplay[2]=&gApp.mCustomMusic[1];
		}
		if (mCustomSong[2].Len())
		{
			if (gApp.mCustomMusic[2].IsLoaded()) gApp.mCustomMusic[2].Unload();
			gApp.mCustomMusic[2].Load(mCustomSong[2]);
			gApp.mMusic_Gameplay[3]=&gApp.mCustomMusic[2];
		}
	}

	SoundStream* aMusic=gApp.mMusic_Gameplay[_clamp(0,mCurrentMusic,3)];
	gApp.FadeInMusic(aMusic,.1f);

	mMusicStarted=true;
}


void World::LoadLevel(String theName, bool oldVersion)
{
	//gTrace.Out("LoadLevel (in) [%s]",theName.c());
	SaveGame aSave;
	aSave.mVersion=SAVEGAME_VERSION;
	if (oldVersion) aSave.mVersion=0x0001;
	aSave.Loading(theName);

	gApp.mCustomMusic[0].Unload();
	gApp.mCustomMusic[1].Unload();
	gApp.mCustomMusic[2].Unload();

	//
	// Skip the meta info chunk..
	//
	aSave.StartChunk();
	aSave.EndChunk();

	Sync(aSave);

	PositionPlayers();

	//
	// Initial setup for game...
	//
	mSpawnSpot=mRobot->mPos;
	mSpawnMusic=mCurrentMusic;
	mZoom=.9f;
	mInitialZoom=true;
	mInitialZoomSpeed=.005f;
	mFlashWhite=0;
	mShake=0;
	mMessageFade=0.0f;

	mRobot->FixPosition();
	
	mZoom=1.0f;

	if (mCurrentMusic>3) mCurrentMusic=0;
	StartMusic();
}


void World::Update()
{
#ifdef _DEBUG
	if (IsKeyPressed(KB_W)) Win();
#endif

/*
#ifdef _DEBUG
	if (IsKeyPressed(KB_S)) 
	{
		gGame->mSaveName="xxx.test";
		gGame->Save();
		return;
	}
	if (IsKeyPressed(KB_W)) Win();
	if (IsKeyPressed(KB_1)) 
	{
		mRobot->mPos.mX-=.5f;
		gOut.Out("Robot Pos: %s",mRobot->mPos.ToString(3));
	}
	if (IsKeyPressed(KB_2)) 
	{
		mRobot->mPos.mX+=.5f;
		gOut.Out("Robot Pos: %s",mRobot->mPos.ToString(3));
	}
	if (IsKeyPressed(KB_3)) mRobot->mPos.mX=193.5f;
	if (IsKeyPressed(KB_C)) 
	{
		mWantConveyorSpeed*=-1;
	}

	if (IsKeyPressed(KB_M))
	{
		InGameMenu *aIG=new InGameMenu();
		aIG->HookTo(&mInGameMenu);
		//aIG->mIsMap=true;
		gApp.AddCPU(aIG);
		Pause(true);
	}
	
#endif
*/

	mConveyorTick+=CONVEYOR_SPEED;//mConveyorSpeed;

	if (mConveyorSpeed!=mWantConveyorSpeed)
	{
		if (mConveyorSpeed<mWantConveyorSpeed) 
		{
			mConveyorSpeed+=.025f/2;
			if (mConveyorSpeed>=mWantConveyorSpeed) mConveyorSpeed=mWantConveyorSpeed;
		}
		if (mConveyorSpeed>mWantConveyorSpeed) 
		{
			mConveyorSpeed-=.025f/2;
			if (mConveyorSpeed<=mWantConveyorSpeed) mConveyorSpeed=mWantConveyorSpeed;
		}
	}

	if (mDarkFadeDir)
	{
		mDarkness+=mDarkFadeDir;
		if (mDarkness<=0)
		{
			mDarkness=0;
			mDarkFadeDir=0;
		}
		if (mDarkness>=1.0f)
		{
			mDarkness=1.0f;
			mDarkFadeDir=1.0f;

			//
			// DO SOMETHING!
			//
		}
	}
	mMessageFade=_max(0,mMessageFade-.25f);
	mFlashWhite=_max(0,mFlashWhite-.1f);
	mShake=_max(0,mShake-1);

	EnumSmartList(ZoingTimer,aZT,mZoingTimerList)
	{
		if (gAppPtr->AppTime()>aZT->mTime+ZOINGWAIT)
		{
			mZoingTimerList-=aZT;
			EnumSmartListRewind(ZoingTimer);
		}
	}

	if (!gSwipeScroll) mSwipeOffset*=.9f;

	if (mInitialZoom)
	{
		mZoom-=mInitialZoomSpeed;
		mInitialZoomSpeed*=.99f;
		if (mZoom<.9f)
		{
			mZoom=.9f;
			mInitialZoom=false;
		}
	}

//	mZoom=.25f;
//	mInitialZoom=false;
	
	if (--mPauseCheckCountdown<=0)
	{
		mPauseCheckCountdown=10;
		CheckPausedPlayers();
	}

	//mTimerPause=0;
	if (mInitialTimerPause || mTimerPause)
	{
		mInitialTimerPause=_max(0,mInitialTimerPause-1);
		mTimerPause=_max(0,mTimerPause-1);
	}
	else
	{
		if (!mWin) 
		{
			if (mFastTimeCountdown==0) mTimer++;
			else
			{
				for (int aCount=0;aCount<2;aCount++)
				{
					mFastTimeCountdown=_max(0,mFastTimeCountdown-1);
					if (mFastTimeCountdown) mTimer+=8;
				}
			}

		}
	}

	if (!mWin || gWorldEditor)
	{
		//
		// Robot control!
		//
		if (gApp.mTouchControls)
		{
			Point aTouchPos;
			{
				if (mButtons.IsPressed(mButton_Move,&aTouchPos))
				{
					//if (aTouchPos.mX<=97) mRobot->GoLeft();
					if (aTouchPos.mX<=0) mRobot->GoLeft();
					else mRobot->GoRight();
				}
			}
	
			if (mButtons.IsPressed(mButton_Jump)) mRobot->Jump();
			if (mButtons.IsPressed(mButton_Shoot)) mRobot->Shoot();
			if (mButtons.IsPressed(mButton_Rocket)) mRobot->Rocket();
			else mRobot->StopRocket();
			if (mButtons.IsPressed(mButton_RocketUp)) mRobot->RocketUp();
			else mRobot->StopRocketUp();
		}
		else if (gOracleActive)
		{
			// P0prime: tape injection point (replaces the six polled keys below)
			if (gOracleHeld&1)  mRobot->GoLeft();
			if (gOracleHeld&2)  mRobot->GoRight();
			if (gOraclePressed&4) mRobot->Jump();
			if (gOracleHeld&8)  mRobot->Shoot();
			if (gOracleHeld&16) mRobot->Rocket();   else mRobot->StopRocket();
			if (gOracleHeld&32) mRobot->RocketUp(); else mRobot->StopRocketUp();
		}
		else
		{
			if (IsKeyPressed(gApp.mZoomIn)) ChangeZoom(.1f);
			if (IsKeyPressed(gApp.mZoomOut)) ChangeZoom(-.1f);

			if (IsKeyDown(gApp.mLeft)) mRobot->GoLeft();
			if (IsKeyDown(gApp.mRight)) mRobot->GoRight();
			if (IsKeyPressed(gApp.mJump)) mRobot->Jump();
			if (IsKeyPressed(gApp.mMenu)) Notify(&mMenuButton);
			if (IsKeyPressed(gApp.mMap))
			{
				InGameMenu *aIG=new InGameMenu();
				aIG->mIsMap=true;
				aIG->HookTo(&mInGameMenu);
				gApp.AddCPU(aIG);
				Pause(true);
			}
			if (IsKeyDown(gApp.mShoot)) mRobot->Shoot();
			if (IsKeyDown(gApp.mRocket)) mRobot->Rocket();
			else mRobot->StopRocket();
			if (IsKeyDown(gApp.mRocketUp)) mRobot->RocketUp();
			else mRobot->StopRocketUp();
		}

		//
		// Update other processing
		//
		UpdateExploders();
		UpdateAmbient();

		//
		// Update camera
		//
		Point aMoveCamera=mRobot->mPos-mScrollCenter;
	//	mScrollCenter+=aMoveCamera*.15f;
		mScrollCenter+=aMoveCamera*mScrollSpeed;//.10f;
		//mScrollSpeed=min(.10f,mScrollSpeed+.003f);

		if (!mScrollSpeedCapped) mScrollSpeed=_min(2.0f,mScrollSpeed+.0075f);
		else mScrollSpeed=_min(.18f,mScrollSpeed+.003f);


		mVisibleRect.mX=mScrollCenter.mX-((gG.WidthF()/2)/mZoom);
		mVisibleRect.mY=mScrollCenter.mY-((320/2)/mZoom);
		mVisibleRect.mWidth=((gG.WidthF())/mZoom);
		mVisibleRect.mHeight=((320)/mZoom);

		if (!mVisibleRect.ContainsPoint(mRobot->mPos.mX,mRobot->mPos.mY)) mScrollSpeedCapped=false;
		else if (mRobot->mGravity<10) mScrollSpeedCapped=true;

		mVisibleRect=mVisibleRect.Expand((float)mGridSize);

		//
		// LiveRect is pretty big, to accomodate tall wells or 
		// long halls.  Things outside of mLiveRect go to sleep
		//
		mLiveRect=mVisibleRect.Expand(mGridSize*40);
		mLiveRect.mY-=(mGridSize*40);
		mLiveRect.mHeight+=(mGridSize*80);

		//
		// Things inside mWakeRect wake up...
		//
		mWakeRect=mVisibleRect.Expand(mGridSize*10);
		mWakeRect.mX-=(mGridSize*40);
		mWakeRect.mWidth+=(mGridSize*80);

		//
		// Make sure this happens after LiveRect is done...
		//
		mPlayerList.Update();
		mAnimatorList_Top.Update();
		mAnimatorList_UnderPlayers.Update();
		mAnimatorList_UnderWorld.Update();
		mPlayTime++;
	}
	else
	{
		if (mFlashWhite<=0)
		{
			mZoom*=1.1f;
			if (mZoom>8.0f) mZoom=8.0f;
			mDarkness=_min(1.0f,mDarkness+.05f);
		}
	}

}

void World::ChangeZoom(float theChange)
{
	mZoom+=theChange;
	if (mZoom>1.30f) mZoom=1.30f;
	if (mZoom<.5f) mZoom=.5f;

	mInitialZoom=false;
}

void World::Draw()
{
	Point aShake;
	if (mShake) 
	{
		aShake=gMath.AngleToVector()*(mShake/2);
		gG.Translate(aShake);
	}

	DrawBackground();

	Point aRealCenter=-mScrollCenter+mScrollOffset;
	gG.PushTranslate();
	gG.Translate(aRealCenter+(Point(gG.WidthF()/2,320.0f/2))+aShake);
//	gG.Translate(aRealCenter+(Point(mWidth/2,320.0f/2))+aShake);
	DrawWorld();
	gG.PopTranslate();
	gG.Translate(mX,mY);

	if (mFlashWhite)
	{
		gG.SetColor(_min(1.0f,mFlashWhite));
		gG.FillRect(-10,-10,gG.WidthF()+20,320+20);
		gG.SetColor();
	}

	if (mNoMoreKitty)
	{
		mMessageFade=1.0f;
		mUpMessage="ROBOT IS SAD";
		mMessage="NO MORE KITTY";
	}

	if (mMessageFade>0)
	{
		gG.Translate(0,-10);
		gG.SetColor(mMessageFade);
		CENTERGLOWBIG(mUpMessage,mWidth/2,(320/2)-70+25+gMessageYMod);
		CENTERGLOWBIG(mMessage,mWidth/2,(320/2)+25+50+gMessageYMod);
		gG.SetColor();
		gG.Translate(0,10);
	}

	if (!mHideUI)
	{
		if (gApp.mTouchControls)
		{
			if (gApp.mUseJoypad) gBundle_Play->mControl_Move.Draw(mButtonPos[mButton_Move]); //2,252,191,66
			if (mRobot->mCanJump) gBundle_Play->mControl_Jump.Draw(mButtonPos[mButton_Jump]);//412,253,66,65
			if (mRobot->mCanShoot) gBundle_Play->mControl_Shoot.Draw(mButtonPos[mButton_Shoot]);//342,253,66,65
			if (mRobot->mCanRocket) gBundle_Play->mControl_Rocket.Draw(mButtonPos[mButton_Rocket]);//272,253,66,65
			if (mRobot->mCanRocketUp) gBundle_Play->mControl_Rocketup.Draw(mButtonPos[mButton_RocketUp]);//412,183,66,65
		}

		if (mRobot->mMaxHP>1)
		{
			float aPosY=37;
			float aPosX=mWidth-26;
			aPosX+=10;

			int aCountTo=(mRobot->mMaxHP-1);
			if (aCountTo>10) aCountTo=10;
			for (int aCount=0;aCount<aCountTo;aCount++)
			//for (int aCount=0;aCount<(mRobot->mMaxHP-1);aCount++)
			{
				gBundle_Play->mHeart_Empty.DrawScaled(aPosX,aPosY,.75f);
				if (mRobot->mHP-1>aCount) gBundle_Play->mHeart_Full.DrawScaled(aPosX,aPosY,.75f);
				aPosX-=(28.0f*.75f);
			}
		}

		gG.Translate(-mX,-mY);

		if (((gAppPtr->AppTime()/5)%2)==0 || !mTimerPause) 
		{
			Point aOffset=Point(0,25);
			int aSeconds=mTimer/50;
			String aString;
			if (mFastTimeCountdown) gG.ClipColor(Color(1,0,0));

			if (aSeconds>3600) aString=Sprintf("%d:%.2d:%.2d",aSeconds/60/60,(aSeconds/60)%60,(aSeconds%60));
			else aString=Sprintf("%d:%.2d",aSeconds/60,(aSeconds%60));

			RIGHTGLOWBIG(aString,gG.WidthF()-5-44,-2+aOffset.mY);
			aString=Sprintf(":%.2d",mTimer%60);
			RIGHTGLOW(aString,gG.WidthF()-5,-2+aOffset.mY);
			if (mFastTimeCountdown) gG.ClipColor();

		}

		if (!mTesting) 
		{
			gBundle_Play->mMenubutton.Center(mMenuButton.Center()+Point(mX,mMenuButton.IsDown()*2.0f));
		}
	}
	if (mDarkness)
	{
		gG.SetColor(0,0,0,mDarkness);
		gG.FillRect();
		gG.SetColor();
	}

}

Sprite *World::GetDisplayTile(IPoint thePos)
{
	Grid *aG=GetGridPtr(thePos.mX,thePos.mY);
	if (aG)
	{
//		if (aG->mPaintID) return &gBundle_Tiles->mTile_Paint[aG->mPaint];
//		else return &gBundle_Tiles->mTile_Block[aG->mLayout];

		if (aG->mPaintID) return gBundle_Tiles->mTile_Paint[aG->mPaint];
		else return &gBundle_Tiles->mBlock[aG->mLayout];

	}
	return &gBundle_Tiles->mBlock[0];
}

void World::DrawWorld()
{
	//
	// Set it up to only draw what should be seen...
	//
	Point aRealCenter=mScrollCenter+mScrollOffset-mSwipeOffset;
	gG.Translate(mSwipeOffset);

	if (mAnimatorList_UnderWorld.GetCount())
	{
		if (mZoom!=1.0f)  gG.ScaleView(mZoom);
		mAnimatorList_UnderWorld.Draw();
		if (mZoom!=1.0f) gG.ScaleView(1.0f);
	}

	//
	// A modification of grid space to prevent negative numbers
	// Pretend we're always positive (at least to 100 spaces)
	//
	int aModSpace=100;
	float aModWorldSpace=(float)aModSpace*mGridSize;
	aModWorldSpace-=mHalfGrid.mX;
	IPoint aUpperLeft=WorldToGrid(aRealCenter.mX-(gG.WidthF()/2/mZoom)+aModWorldSpace,aRealCenter.mY-(320/2/mZoom)+aModWorldSpace);
	IPoint aLowerRight=WorldToGrid(aRealCenter.mX+(gG.WidthF()/2/mZoom)+aModWorldSpace,aRealCenter.mY+(320/2/mZoom)+aModWorldSpace);
	aLowerRight+=IPoint(1,1);
	gG.Translate(-aModWorldSpace,-aModWorldSpace);
	gG.Translate(0.0f,.5f);

	//119,137
	//121,139
	

	if (mZoom!=1.0f) gG.ScaleView(mZoom);

	if (mIsEditor)
	{
		gG.RenderWhite(true);
		gG.SetColor(.5f);
		for (int aSpanX=aUpperLeft.mX;aSpanX<=aLowerRight.mX;aSpanX++)
		{
			for (int aSpanY=aUpperLeft.mY;aSpanY<=aLowerRight.mY;aSpanY++)
			{
				gBundle_Tiles->mBlock[0].Center((float)(aSpanX*mGridSize),(float)(aSpanY*mGridSize));
			}
		}
		gG.SetColor();
		gG.RenderWhite(false);
	}

	float aWallVal[4];
	aWallVal[0]=(float)((gApp.AppTime()/4)%20)/20;
	aWallVal[1]=(float)(((gApp.AppTime()/4)+5)%20)/20;
	aWallVal[2]=(float)(((gApp.AppTime()/4)+10)%20)/20;
	aWallVal[3]=(float)(((gApp.AppTime()/4)+15)%20)/20;

	float aWallOff[4];
	for (int aCount=0;aCount<4;aCount++) aWallOff[aCount]=((aWallVal[aCount])*40);

	float aWallFade[4];
	for (int aCount=0;aCount<4;aCount++) aWallFade[aCount]=gMath.Sin((aWallVal[aCount]*aWallVal[aCount])*180);

	for (int aSpanX=aUpperLeft.mX;aSpanX<=aLowerRight.mX;aSpanX++)
	{
		for (int aSpanY=aUpperLeft.mY;aSpanY<=aLowerRight.mY;aSpanY++)
		{
			//char aValue=GetGrid(aSpanX-aModSpace,aSpanY-aModSpace);
			//if (aValue>0)
			Grid *aG=GetGridPtr(aSpanX-aModSpace,aSpanY-aModSpace);
			if (aG)
			{
				//if (mPlayMusic) {gG.RenderWhite(true);gG.SetColor(.5f);}
				if (aG->mPaintID)
				{
					gBundle_Tiles->mTile_Paint[aG->mPaint]->Center((float)(aSpanX*mGridSize),(float)(aSpanY*mGridSize));
				}
//				else if (aG->mLayout>0 && (aG->mLayout<33 || aG->mLayout>35 || mIsEditor) && ((aG->mLayout!=54 && aG->mLayout!=55 && aG->mLayout!=CONVEYOR_LEFT && aG->mLayout!=CONVEYOR_RIGHT) || mIsEditor))
				else if (aG->mLayout>0 && (aG->mLayout<33 || aG->mLayout>35 || mIsEditor) && ((aG->mCustomDraw==0) || mIsEditor))
				//else if (aG->mLayout>0 && (aG->mLayout<33 || aG->mLayout>35 || mIsEditor))
				{
					gBundle_Tiles->mBlock[aG->mLayout].Center((float)(aSpanX*mGridSize),(float)(aSpanY*mGridSize));
				}
				else
				{
					Point aPos=Point(aSpanX*mGridSize,aSpanY*mGridSize);
					switch (aG->mLayout)
					{
					case 54:
						{
							for (int aCount=0;aCount<4;aCount++)
							{
								gG.SetColor(aWallFade[aCount]);
								gBundle_Tiles->mForceField.Center(aPos.mX+20-aWallOff[aCount],aPos.mY);
							}
							gG.SetColor();
							break;
						}
					case 55:
						for (int aCount=0;aCount<4;aCount++)
						{
							gG.SetColor(aWallFade[aCount]);
							gBundle_Tiles->mForceField.Center(aPos.mX-20+aWallOff[aCount],aPos.mY);
						}
						gG.SetColor();
						break;
					case CONVEYOR_LEFT:
						gBundle_Tiles->mConveyor.Center(aPos.mX,aPos.mY);
						gG.PushClip();
						gG.Clip(aPos.mX-20,aPos.mY-20,40,40);
						gBundle_Tiles->mConveyorTop.Center(aPos.mX-((int)(mConveyorTick)%20),aPos.mY);
						gBundle_Tiles->mConveyorBottom.Center(aPos.mX+((int)(mConveyorTick)%20),aPos.mY);
						gG.PopClip();
						break;
					case CONVEYOR_RIGHT:
						gBundle_Tiles->mConveyor.Center(aPos.mX,aPos.mY);
						gG.PushClip();
						gG.Clip(aPos.mX-20,aPos.mY-20,40,40);
						gBundle_Tiles->mConveyorTop.Center(aPos.mX+((int)(mConveyorTick)%20),aPos.mY);
						gBundle_Tiles->mConveyorBottom.Center(aPos.mX-((int)(mConveyorTick)%20),aPos.mY);
						gG.PopClip();
						break;
					case ZOING_BLOCK:
						gBundle_Tiles->mZoinger.Center(aPos.mX,aPos.mY);
						if (aG->mExtraData==1)
						{
							float aOffset=0;
							if (mZoingTimerList.GetCount())
							{
								EnumSmartList(ZoingTimer,aZT,mZoingTimerList)
								{
//									gOut.Out("Compare: %d,%d == %d,%d",aZT->mGridPos.mX,aZT->mGridPos.mY,aSpanX-aModSpace,aSpanY-aModSpace);
									if (aZT->mGridPos.mX==aSpanX-aModSpace && aZT->mGridPos.mY==aSpanY-aModSpace)
									{
										int aDiff=gAppPtr->AppTime()-aZT->mTime;
										if (aDiff<8) aOffset=-14;
										else aOffset=(float)_min(0,-(14-aDiff));
										break;
									}
								}
							}
							gBundle_Tiles->mZoingertop.Center(aPos.mX,aPos.mY+aOffset);
						}
						break;
					case COSMIC_BLOCK:
						{
#define RISESPEED 20
							float aScale=1.0f;
							for (int aCount=2;aCount<=4;aCount++)
							{
								int aMod=QuickRand((aSpanX*aSpanY)*aCount*2);
								static Random aRand;
								aRand.Seed((aSpanX*aSpanY)+(((gAppPtr->AppTime()+aMod)/2)/RISESPEED));
								float aStep=(float)(((gAppPtr->AppTime()+aMod)/2)%RISESPEED)/(float)RISESPEED;
								float aFade=gMath.Sin(aStep*180);
								gG.SetColor(aFade*.5f);
								Point aWorkPos=aPos+Point(0.0f,20-aStep*35);
								aWorkPos+=gMath.AngleToVector(&aRand)*aRand.GetF(25);
								gBundle_Play->mLifter.DrawScaled(aWorkPos,aScale);
								aScale*=.75f;
								gG.SetColor();
							}
						}
						break;
					case SLAM_BLOCK:
						{
							gG.ClipColor(0,1,.5f,.5f);
							float aRand=QuickRandFraction(aSpanX+gApp.AppTime())*.5f;
							gBundle_Tiles->mForceField.DrawScaled(aPos.mX,aPos.mY,Point(1.0f+aRand,1.0f));
							for (int aCount=0;aCount<4;aCount++)
							{
								gG.SetColor((1.0f-(aWallOff[aCount]/20.0f)));
								gBundle_Tiles->mForceField.Center(aPos.mX-aWallOff[aCount],aPos.mY);
								gBundle_Tiles->mForceField.Center(aPos.mX+aWallOff[aCount],aPos.mY);
							}
							gG.ClipColor();
							gG.SetColor();
						}
						break;
					}
				}

				if (mOverGrid)
				{
					char aType=mOverGrid[(aSpanX-aModSpace)+((aSpanY-aModSpace)*mGridWidth)];
					if (aType>0 && aType<64) gBundle_Tiles->mPipes[aType-1].Center((float)(aSpanX*mGridSize),(float)(aSpanY*mGridSize));
				}

				//gG.RenderWhite(false);gG.SetColor();
				//gBundle_ETools->mFont_Bimini12.Center(Sprintf("%d",aG->mPaintID),(float)(aSpanX*mGridSize),(float)(aSpanY*mGridSize));
			}
		}
	}
	if (mCrackList.GetCount())
	{
		EnumList(struct Crack,aC,mCrackList)
		{
			Point aPos(aC->mPos.mX*mGridSize,aC->mPos.mY*mGridSize);
			gBundle_Play->mCrack[aC->mLevel].Center(aPos+Point(aModWorldSpace,aModWorldSpace)+mHalfGrid);
		}
	}

	if (mZoom!=1.0f) gG.ScaleView(1.0f);

	gG.Translate(0.0f,-.5f);
	gG.Translate(aModWorldSpace,aModWorldSpace);
	//
	// Don't draw anything but the grid within the ModWorldSpace...
	//

	if (mZoom!=1.0f)  gG.ScaleView(mZoom);
	mAnimatorList_UnderPlayers.Draw();

	mPlayerList.Draw();

	mAnimatorList_Top.Draw();
	if (mZoom!=1.0f) gG.ScaleView(1.0f);

	gG.Translate(-mSwipeOffset);


}

bool World::Collide(Rect theRect)
{
	String aTest;
	IPoint aGP=WorldToGrid(theRect.Center());
	IPoint aDir;
	if (IsBlocked(aGP.mX+aDir.mX,aGP.mY+aDir.mY)) if (theRect.Intersects(GridToWorldRect(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) 
	{
		if (ISLAVA(GetGrid(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) mCollidedWithLava=true;
		return true;
	}
	for (int aCount=0;aCount<8;aCount++)
	{
		aDir=gMath.GetCardinal8Direction(aCount);
		if (IsBlocked(aGP.mX+aDir.mX,aGP.mY+aDir.mY)) if (theRect.Intersects(GridToWorldRect(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) 
		{
			//if (GetGrid(aGP.mX+aDir.mX,aGP.mY+aDir.mY)==2) mCollidedWithLava=true;
			if (ISLAVA(GetGrid(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) 
			{
				if (aDir.mY>=0 || mRobot->mHasHelmet==false) mCollidedWithLava=true;
			}

			return true;
		}
	}
	return false;
}

bool World::CollideY(Rect theRect, float theDir)
{
	String aTest;
	IPoint aGP=WorldToGrid(theRect.Center());
	IPoint aDir;
	
	if (IsBlockedY(aGP.mX+aDir.mX,aGP.mY+aDir.mY,theDir)) if (theRect.Intersects(GridToWorldRect(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) 
	{
		if (ISLAVA(GetGrid(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) mCollidedWithLava=true;
		return true;
	}
	for (int aCount=0;aCount<8;aCount++)
	{
		aDir=gMath.GetCardinal8Direction(aCount);
		if (IsBlockedY(aGP.mX+aDir.mX,aGP.mY+aDir.mY,theDir)) if (theRect.Intersects(GridToWorldRect(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) 
		{
			//if (GetGrid(aGP.mX+aDir.mX,aGP.mY+aDir.mY)==2) mCollidedWithLava=true;
			if (ISLAVA(GetGrid(aGP.mX+aDir.mX,aGP.mY+aDir.mY)))
			{
				if (aDir.mY>=0 || mRobot->mHasHelmet==false) 
				{
					mCollidedWithLava=true;
				}
			}

			return true;
		}
	}
	return false;
}

bool World::CollideDir(Rect theRect, float theMoveDir)
{
	String aTest;
	IPoint aGP=WorldToGrid(theRect.Center());
	IPoint aDir;
	if (IsBlockedDir(aGP.mX+aDir.mX,aGP.mY+aDir.mY,theMoveDir)) if (theRect.Intersects(GridToWorldRect(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) 
	{
		if (ISLAVA(GetGrid(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) mCollidedWithLava=true;
		return true;
	}
	for (int aCount=0;aCount<8;aCount++)
	{
		aDir=gMath.GetCardinal8Direction(aCount);
		if (IsBlockedDir(aGP.mX+aDir.mX,aGP.mY+aDir.mY,theMoveDir)) if (theRect.Intersects(GridToWorldRect(aGP.mX+aDir.mX,aGP.mY+aDir.mY))) 
		{
			if (ISLAVA(GetGrid(aGP.mX+aDir.mX,aGP.mY+aDir.mY)))
			{
				if (aDir.mY>=0 || mRobot->mHasHelmet==false) mCollidedWithLava=true;
			}
			return true;
		}
	}
	return false;
}


void World::DrawBackground()
{
	
//	Point aOffset=-(mScrollCenter+mScrollOffset)/2;
	Point aOffset=-(mScrollCenter+mScrollOffset)*.9f;
	//Sprite *aRepeater=&gBundle_Tiles->mMoonbase;
	int aRWidth=256;
	int aRHeight=256;

	float aModX=(float)fmod(aOffset.mX,(float)aRWidth);
	float aModY=(float)fmod(aOffset.mY,(float)aRHeight);

	if (mZoom==0) mZoom=1.0f;

	float aAreaWidth=(((gG.WidthF()/aRWidth)+10)/2)/mZoom;
	float aAreaHeight=(((320/aRHeight)+10)/2)/mZoom;
	gG.Translate(gG.WidthF()/2,320.0f/2);

	if (mZoom!=1.0f) gG.ScaleView(mZoom);

	for (float aSpanX=-aAreaWidth;aSpanX<=aAreaWidth;aSpanX++)
	{
		for (float aSpanY=-aAreaHeight;aSpanY<=aAreaHeight;aSpanY++)
		{
			DrawBKGTile(aModX+((int)aSpanX*aRWidth),aModY+((int)aSpanY*aRHeight));
			//aRepeater->Draw(aModX+((int)aSpanX*aRepeater->mWidth),aModY+((int)aSpanY*aRepeater->mHeight));
		}
	}

	if (mZoom!=1.0f) gG.ScaleView(1.0f);
	gG.Translate();

	Quad aQ;
	Color aColor[4];

	Color aHazeTopColor=Color(.5f,0,.5f,0);
	Color aHazeColor=Color(.5f,0,.5f,.25f);

	aQ.mCorner[0]=Point(0,0);
	aQ.mCorner[1]=Point(gG.WidthF(),0.0f);
	aQ.mCorner[2]=Point(0.0f,320.0f);
	aQ.mCorner[3]=Point(gG.WidthF(),320.0f);

	aColor[0]=aHazeTopColor;
	aColor[1]=aHazeTopColor;
	aColor[2]=aHazeColor;
	aColor[3]=aHazeColor;

	gG.SetColor(.99f);
	gG.FillQuad(aQ,aColor[0],aColor[1],aColor[2],aColor[3]);
	gG.SetColor();
}

void World::Sync(SaveGame &theSaveGame)
{
	if (theSaveGame.IsSaving())
	{
		if (!mKitty)
		{
			mKitty=new Kitty;
			mKitty->mPos=Point(-9999,-9999);
			mKitty->mNullKitty=true;
		}
	}

	//
	// Sync grid area...
	//
	theSaveGame.StartChunk();
	theSaveGame.Sync(&mGridWidth);
	theSaveGame.Sync(&mGridHeight);

	if (theSaveGame.IsLoading()) 
	{
		delete [] mGrid;
		InitializeGrid(mGridWidth,mGridHeight);
	}
	theSaveGame.Sync(mGrid,mGridWidth*mGridHeight*sizeof(Grid));
	theSaveGame.Sync(mLevelMap,mGridWidth*mGridHeight*sizeof(MAPTYPE));

	theSaveGame.StartChunk();
	int aMessageCount=mRadioTextList.GetCount();
	theSaveGame.Sync(&aMessageCount);
	while (mRadioTextList.GetCount()<aMessageCount) mRadioTextList+=new RadioText;
	EnumSmartList(RadioText,aRT,mRadioTextList)
	{
		theSaveGame.Sync(&aRT->mPos);
		theSaveGame.Sync(&aRT->mText);
	}
	theSaveGame.EndChunk();
	theSaveGame.EndChunk();

	//
	// Sync robot location...
	//
	theSaveGame.StartChunk();
	if (theSaveGame.IsLoading()) {mRobot=new Robot();mPlayerList+=mRobot;}
	theSaveGame.Sync(&mRobot->mPos.mX);
	theSaveGame.Sync(&mRobot->mPos.mY);
	if (theSaveGame.IsLoading()) mScrollCenter=mRobot->mPos;
	theSaveGame.EndChunk();

	//
	// Sync kitty location...
	//
	theSaveGame.StartChunk();
	if (theSaveGame.IsLoading()) {mKitty=new Kitty();mPlayerList+=mKitty;}
	theSaveGame.Sync(&mKitty->mPos.mX);
	theSaveGame.Sync(&mKitty->mPos.mY);
	theSaveGame.EndChunk();

	//
	// Sync extra game data
	//
	theSaveGame.StartChunk();
	theSaveGame.Sync(&mMessage);
	theSaveGame.Sync(&mUpMessage);
	theSaveGame.Sync(&mMessageFade);
	theSaveGame.Sync(&mSpawnSpot);
	theSaveGame.Sync(&mFlashWhite);
	theSaveGame.Sync(&mShake);
	theSaveGame.Sync(&mZoom);
	theSaveGame.Sync(&mScrollOffset);
	theSaveGame.Sync(&mSpawnMusic);
	theSaveGame.Sync(&mDied);
	theSaveGame.Sync(&mCoins);
	theSaveGame.Sync(&mMaxCoins);
	theSaveGame.Sync(&mRemoveBitcoinBlockCountdown);
	theSaveGame.Sync(&mNoMoreKitty);
	theSaveGame.Sync(&mConveyorSpeed);
	theSaveGame.Sync(&mWantConveyorSpeed);

	if (mConveyorSpeed==0) mConveyorSpeed=.5f;
	if (mWantConveyorSpeed==0) mWantConveyorSpeed=mConveyorSpeed;
	mWantConveyorSpeed=_clamp(-1.1f,mWantConveyorSpeed,1.1f);

	theSaveGame.Sync(&mCustomSong[0]);
	theSaveGame.Sync(&mCustomSong[1]);
	theSaveGame.Sync(&mCustomSong[2]);

	theSaveGame.EndChunk();
	//
	// Sync all powerups and creatures
	//

	if (!gWorldEditor)
	{
		//
		// Lets us catch whether we are just loading this or not...
		//
		bool aExists=true;
		theSaveGame.Sync(&aExists);
		if (!aExists) return;

		//
		// Sync world data...
		//
		theSaveGame.StartChunk();

		theSaveGame.Sync(&mPlayMusic);
		theSaveGame.Sync(&mSpawnSpot);
		theSaveGame.Sync(&mSpawnMusic);
		theSaveGame.Sync(&mCurrentMusic);
		theSaveGame.Sync(&mWantNewMusic);
		theSaveGame.Sync(&mNewMusic);
		theSaveGame.Sync(&mMusicDelay);
		IPoint aDummy;
		theSaveGame.Sync(&aDummy);
		theSaveGame.Sync(&mRemoveBossBlockCountdown);
		theSaveGame.Sync(&mDarkness);
		theSaveGame.Sync(&mDarkFadeDir);
		theSaveGame.Sync(&mTimer);
		theSaveGame.Sync(&mInitialTimerPause);
		theSaveGame.Sync(&mTimerPause);
		theSaveGame.Sync(&mWin);
		theSaveGame.Sync(&mFastTimeCountdown);
		theSaveGame.Sync(&mRemoveBitcoinBlockCountdown);

		//
		// Boss opening doors...
		//
		int aBBCount=mRemoveBossBlockPos.Size();
		theSaveGame.Sync(&aBBCount);
		mRemoveBossBlockPos.Reset();
		mRemoveBossBlockPos.GuaranteeSize(aBBCount);
		for (int aCount=0;aCount<aBBCount;aCount++) theSaveGame.Sync(&mRemoveBossBlockPos[aCount]);

		int aComputerDoorCount=mComputerDoorRect.GetRectCount();
		theSaveGame.Sync(&aComputerDoorCount);
		while (mComputerDoorRect.GetRectCount()<aComputerDoorCount) mComputerDoorRect.mRectList+=new Rect;
		for (int aCount=0;aCount<aComputerDoorCount;aCount++) {Rect* aRect=(Rect*)mComputerDoorRect.mRectList[aCount];theSaveGame.Sync(aRect);}

		int aVirusDoorCount=mVirusDoorRect.GetRectCount();
		theSaveGame.Sync(&aVirusDoorCount);
		while (mVirusDoorRect.GetRectCount()<aVirusDoorCount) mVirusDoorRect.mRectList+=new Rect;
		for (int aCount=0;aCount<aVirusDoorCount;aCount++) {Rect* aRect=(Rect*)mVirusDoorRect.mRectList[aCount];theSaveGame.Sync(aRect);}

		theSaveGame.EndChunk();

		//
		// Sync players
		//
		theSaveGame.StartChunk();
		if (theSaveGame.IsLoading()) {mPlayerList-=mRobot;delete mRobot;mRobot=new Robot();mPlayerList+=mRobot;}
		mRobot->Sync(theSaveGame);
		theSaveGame.EndChunk();
		theSaveGame.StartChunk();
		if (theSaveGame.IsLoading()) {mPlayerList-=mKitty;delete mKitty;mKitty=new Kitty();mPlayerList+=mKitty;}
		mKitty->Sync(theSaveGame);
		theSaveGame.EndChunk();
		theSaveGame.StartChunk();
		List aSyncPlayerList;

		if (theSaveGame.IsSaving())
		{
			EnumList(Player,aP,mPlayerList)
			{
				if (aP->mKill) continue;
				if (aP==mRobot || aP==mKitty) continue;
				if (aP->mID==ID_NULL) continue;
				aSyncPlayerList+=aP;
			}
			EnumList(Player,aP,mPausedPlayerList)
			{
				if (aP->mKill) continue;
				if (aP==mRobot || aP==mKitty) continue;
				if (aP->mID==ID_NULL) continue;
				aSyncPlayerList+=aP;
			}
		}

		SyncListStart(theSaveGame,Player,aSyncPlayerList)
		{
			int aID=refItem->mID;
			theSaveGame.Sync(&aID);

			if (theSaveGame.IsLoading())
			{
				aSyncPlayerList-=refItem;
				delete refItem;	// It was created of type Player within the SyncListStart macro...

				refItem=CreatePlayerByID(aID);
				aSyncPlayerList+=refItem;
			}
			refItem->Sync(theSaveGame);
		}
		SyncListEnd();

		//mPlayerList+=aSyncPlayerList;
		if (theSaveGame.IsLoading()) 
		{
			EnumList(Player,aP,aSyncPlayerList) mPlayerList+=aP;
		}

		theSaveGame.EndChunk();
		theSaveGame.StartChunk();
		//
		// Sync exploders (mExploderList)
		//
		SyncList(theSaveGame,Exploder,mExploderList);
		theSaveGame.EndChunk();

		if (theSaveGame.IsLoading()) PositionPlayers();



		//gApp.mMusic.FadeIn(Sprintf("gameplay%d",mCurrentMusic),.1f);

		//
		// Wait... we're always "in game" at this point!
		//
		if(!mNeverPlayMusic) mPlayMusic=true;
	}
}

void World::AddMessage(String theMessage, String theUpMessage, float thePlusFade)
{
	gMessageYMod=0;
	mMessage=gBundle_Play->mFont_Commodore64Angled18.Wrap(theMessage,gG.WidthF()-70);//410);
	mUpMessage=gBundle_Play->mFont_Commodore64Angled18.Wrap(theUpMessage,gG.WidthF()-70);//410);
	mMessageFade=27.0f+thePlusFade;

	if (theMessage=="") gMessageYMod=-40;
}

void World::Smash(Point thePos)
{
	IPoint aPos=WorldToGrid(thePos.mX,thePos.mY);
	int aBlock=GetGrid(aPos.mX,aPos.mY);
	if (aBlock==3)
	{
		SetGrid(aPos.mX,aPos.mY,0);
		Point aBrickCenter=GridToWorld(aPos.mX,aPos.mY);
		gSounds->mCrumble.Play();

		EnumList(struct Crack,aC,mCrackList)
		{
			if (aC->mPos.mX==aPos.mX && aC->mPos.mY==aPos.mY) 
			{
				mCrackList-=aC;
				delete aC;
				EnumListRewind(struct Crack);
			}
		}

		{
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mJumpburst);
			aS->mColor=Color(1,0,0,1);
			aS->mFade=2.0f;
			aS->mFadeSpeed=-.1f;
			aS->mPos=aBrickCenter;
			aS->mScale=2.0f;
			mAnimatorList_Top+=aS;
		}
		{
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchburst);
			aS->mColor=Color(1,0,0,1);
			aS->mFade=1.0f;
			aS->mFadeSpeed=-.15f;
			aS->mPos=aBrickCenter;
			aS->mScale=1.1f;
			mAnimatorList_Top+=aS;
		}


		for (float aAngle=0;aAngle<360;aAngle+=20)
		{
			Point aPos=aBrickCenter+gMath.AngleToVector(aAngle)*((mGridSize/2)+gRand.GetSpanF(-3,3));

			aPos+=gMath.AngleToVector()*gRand.GetF(5);

			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mBrickfragment);
			aS->mPos=aPos;
			aS->mDir=(aPos-aBrickCenter)/4;
			aS->mDir.mX*=2;
			aS->mGravity=Point(0.0f,.5f);
			aS->mFade=1.5f;
			aS->mScale=gRand.GetF(.5f)+.5f;
			aS->mRotation=gRand.GetF(360);
			aS->mFadeSpeed=-.1f;

			mAnimatorList_Top+=aS;
		}
	}
}

bool World::PushWall(Point thePos)
{
	IPoint aPos=WorldToGrid(thePos);
	int aType=GetGrid(aPos.mX,aPos.mY);
	bool aOpenDoor=false;
	IPoint aDoorBottom;
	IPoint aDoorTop;

	if ((aType==14 || aType==15) && mRobot->mHasKey[0])
	{
		aOpenDoor=true;
		if (aType==14) 
		{
			aDoorBottom=aPos+IPoint(0,1);
			aDoorTop=aPos;
		}
		if (aType==15)
		{
			aDoorBottom=aPos;
			aDoorTop=aPos+IPoint(0,-1);
		}
	}
	if ((aType==16 || aType==17) && mRobot->mHasKey[1])
	{
		aOpenDoor=true;
		if (aType==16) 
		{
			aDoorBottom=aPos+IPoint(0,1);
			aDoorTop=aPos;
		}
		if (aType==17)
		{
			aDoorBottom=aPos;
			aDoorTop=aPos+IPoint(0,-1);
		}
	}
	if ((aType==18 || aType==19) && mRobot->mHasKey[2])
	{
		aOpenDoor=true;
		if (aType==18) 
		{
			aDoorBottom=aPos+IPoint(0,1);
			aDoorTop=aPos;
		}
		if (aType==19)
		{
			aDoorBottom=aPos;
			aDoorTop=aPos+IPoint(0,-1);
		}
	}

	if ((aType==51 || aType==52) && mRobot->mGoldKeyCount>0)
	{
		aOpenDoor=true;
		if (aType==51) 
		{
			aDoorBottom=aPos+IPoint(0,1);
			aDoorTop=aPos;
		}
		if (aType==52)
		{
			aDoorBottom=aPos;
			aDoorTop=aPos+IPoint(0,-1);
		}
		mRobot->mGoldKeyCount--;
	}


	if (aOpenDoor)
	{
		gSounds->mOpendoor.Play();
		int aBottom=GetGrid(aDoorBottom.mX,aDoorBottom.mY);
		int aTop=GetGrid(aDoorTop.mX,aDoorTop.mY);

		Anim_Sprite *aS;
		if (aTop==14 || aTop==16 || aTop==18 || aTop==51)
		{
			SetGrid(aDoorTop.mX,aDoorTop.mY,0);

			aS=new Anim_Sprite(&gBundle_Tiles->mBlock[aTop]);
			aS->mPos=GridToWorld(aDoorTop.mX,aDoorTop.mY);
			aS->mDir=Point(0.0f,-2.0f);
			aS->mTimer=mGridSize/2;
			mAnimatorList_UnderWorld+=aS;
		}
		if (aBottom==15 || aBottom==17 || aBottom==19 || aBottom==52)
		{
			SetGrid(aDoorBottom.mX,aDoorBottom.mY,0);
			aS=new Anim_Sprite(&gBundle_Tiles->mBlock[aBottom]);
			aS->mPos=GridToWorld(aDoorBottom.mX,aDoorBottom.mY);
			aS->mDir=Point(0.0f,2.0f);
			aS->mTimer=mGridSize/2;
			mAnimatorList_UnderWorld+=aS;
		}

	}

	return aOpenDoor;
}

void World::TurnOffComputrons()
{
	EnumList(Player,aP,mPlayerList)
	{
		if (aP->mID==ID_COMPUTRON)
		{
			Computron *aC=(Computron*)aP;
			aC->mActivated=false;
		}
	}
}

void World::CheckPausedPlayers()
{
	List aWakeList;
	EnumList(Player,aP,mPausedPlayerList)
	{
		if (mWakeRect.ContainsPoint(aP->mPos.mX,aP->mPos.mY))
		{
			mPausedPlayerList-=aP;
			mPlayerList+=aP;

			aWakeList+=aP;
			EnumListRewind(Player);
		}
	}

	//
	// Wake up any players near anyone in aWakeList...
	//
	if (aWakeList.GetCount())
	{
		EnumList(Player,aWake,aWakeList)
		{
			EnumList(Player,aP,mPausedPlayerList)
			{
				if (aP==aWake) continue;

				Point aVec=aP->mPos-aWake->mPos;
				if (aVec.LengthSquared()<=Squared(mGridSize*3))
				{
					mPausedPlayerList-=aP;
					mPlayerList+=aP;
					aWakeList+=aP;
					EnumListRewind(Player);
				}
			}
		}
	}
}

float World::GetVolume(Point thePos)
{
	Point aVec=thePos-mScrollCenter;
	float aLen=aVec.Length();

	aLen-=240;
	if (aLen<0) return 1.0f;
	if (aLen>300) return 0.0f;

	return 1.0f-(aLen/(300));
}

int gPinchyCount=0;
int gPinchyHash[5];
Point gPinchPos[2];
float gPinchyDistance;
bool gPinching=false;

void World::TouchStart(int x, int y)
{
	if (gPinchyCount==2) gPinchyCount=0;
	
	gPinching=false;
//	if (Rect(189,109,102,102).ContainsPoint(x,y))
	if (Rect(102,102).GetCenterAt(gG.HalfWidthF(),gG.HalfHeightF()).ContainsPoint(x,y))
	{
		//
		// Start a swipe...
		//
		gSwipeScroll=true;
		gSwipeScrollHash=gAppPtr->mMessageData_TouchID;//GetTouchHash();
		gPinchyCount=0;
		return;
	}
	if (mButtons.MouseDown(x,y)) 
	{
		gPinchyCount=0;
		return;
	}

	if (gPinchyCount>=2) gPinchyCount=0;
	
	gPinchyHash[gPinchyCount]=gAppPtr->mMessageData_TouchID;
	gPinchPos[gPinchyCount]=Point(x,y);
	gPinchyCount++;
	
	if (gPinchyCount>=2)
	{
		gPinchyDistance=(gPinchPos[0]-gPinchPos[1]).Length();
		gPinching=true;
	}
}

void World::Pause(bool theState)
{
	mHideUI=theState;
	if (!theState) PositionButtons();

	if (theState) CPU::Pause();
	else CPU::UnpauseAll();
}


void World::TouchMove(int x, int y)
{
	if (gPinching)
	{
		if (gAppPtr->mMessageData_TouchID==gPinchyHash[0]) gPinchPos[0]=Point(x,y);
		if (gAppPtr->mMessageData_TouchID==gPinchyHash[1]) gPinchPos[1]=Point(x,y);
		float aPinchDist=(gPinchPos[0]-gPinchPos[1]).Length();
		float aDiff=aPinchDist-gPinchyDistance;
		
		aDiff*=.001f;
		mZoom+=aDiff;
		if (mZoom>1.0f) mZoom=1.0f;
		if (mZoom<.8f) mZoom=.8f;
		
		gPinchyDistance=aPinchDist;
		return;
	}
	
	if (gSwipeScroll)
	{
		if (gAppPtr->mMessageData_TouchID==gSwipeScrollHash)
		{
			//
			// Swipe it!
			//
			// Make a swipe scroll offset, and use mousemove to work it.
			// If gSwipeScroll is false, the swipe scroll offset will reduce itself.
			//
			//
			mSwipeOffset=Point(x,y)-Point(gG.WidthF()/2,320.0f/2);
			mSwipeOffset*=1.25f;
			return;
		}
	}
	mButtons.MouseMove(x,y);
}

void World::TouchEnd(int x, int y)
{
	if (gPinching)
	{
		gPinchyCount=_max(0,gPinchyCount-1);
		if (gPinchyCount<=0) gPinching=false;
		return;
	}
	
	gPinchyCount=0;
	gSwipeScroll=false;

	mButtons.MouseUp(x,y);
}

void World::MouseDoubleClick(int x, int y, int theButton)
{
	mButtons.MouseDown(x,y);
}


void World::Explode(int theX, int theY, int theCountdown)
{
	EnumList(Exploder,aP,mExploderList) if (aP->mPos.mX==theX && aP->mPos.mY==theY) return;

	Exploder *aE=new Exploder;
	aE->mPos=IPoint(theX,theY);
//	if (theCountdown==-1) theCountdown=10+gRand.Get(15);
	if (theCountdown==-1) theCountdown=10+gRand.Get(5);
	aE->mCountdown=theCountdown;
	mExploderList+=aE;

}

void World::Melt(int theX, int theY)
{
	//
	// Make sure a melter isn't already here on this spot!
	//
	EnumList(Player,aP,mPlayerList)
	{
		if (aP->mID==ID_ACIDBURNER)
		{
			AcidBurner* aA=(AcidBurner*)aP;
			if (aA->mFirstPos.mX==theX && aA->mFirstPos.mY==theY) return;
		}
	}

	AcidBurner* aAB=(AcidBurner*)CreatePlayerByID(ID_ACIDBURNER);
	aAB->mFirstPos=IPoint(theX,theY);
	aAB->mPos=GridToWorld(theX,theY);
	mPlayerList+=aAB;

}


void World::UpdateExploders()
{
	EnumList(Exploder,aP,mExploderList)
	{
		if (--aP->mCountdown<=0)
		{
			SetGrid(aP->mPos.mX,aP->mPos.mY,0);
			Point aExplodePos=GridToWorld(aP->mPos.mX,aP->mPos.mY);
			gSounds->mExplodecrate.PlayPitched(.75f+gRand.GetF(.75f),gWorld->GetVolume(aExplodePos));

			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mExplode);
			aS->mPos=aExplodePos;
			aS->mRotation=gRand.GetF(360);
			aS->mScale=1.0f+gRand.GetF(.25f);
			aS->mFade=1.0f;
			aS->mFadeSpeed=-(.075f+gRand.GetF(.1f));
			aS->mColor=Color(1.0f,gRand.GetF(.25f),.1f);
			aS->mColorStep=Color(-.05f,.05f,0);
			mAnimatorList_Top+=aS;

			mFlashWhite=.75f;
			Shake(20);

			Point aGravity=Point(0.0f,.5f);
			float aSpeedMod=5;

			aS=new Anim_Sprite(&gBundle_Play->mSplodefragment);
			aS->mPos=aExplodePos;
			aS->mRotation=0;
			aS->mRotation-=gRand.GetF(20);
			aS->mDir=Point(-1,-1)*(5+gRand.GetF(aSpeedMod));
			aS->mDirMult=Point(.95f,.95f);
			aS->mRotationDir=-1;
			aS->mFade=3.0f;
			aS->mFadeSpeed=-(.2f+gRand.GetF(.1f));
			aS->mGravity=aGravity;
			aS->mColor=Color(1,0,0);
			aS->ColorStep(Color(0,.15f,.15f));
			mAnimatorList_Top+=aS;

			aS=new Anim_Sprite(&gBundle_Play->mSplodefragment);
			aS->mPos=aExplodePos;
			aS->mRotation=90;
			aS->mRotation+=gRand.GetF(20);
			aS->mDir=Point(1,-1)*(5+gRand.GetF(aSpeedMod));
			aS->mDirMult=Point(.95f,.95f);
			aS->mRotationDir=1;
			aS->mFade=3.0f;
			aS->mFadeSpeed=-(.2f+gRand.GetF(.1f));
			aS->mGravity=aGravity;
			aS->mColor=Color(1,0,0);
			aS->ColorStep(Color(0,.15f,.15f));
			mAnimatorList_Top+=aS;

			aS=new Anim_Sprite(&gBundle_Play->mSplodefragment);
			aS->mPos=aExplodePos;
			aS->mRotation=180;
			aS->mRotation+=gRand.GetF(20);
			aS->mDir=Point(1,1)*(5+gRand.GetF(aSpeedMod));
			aS->mDirMult=Point(.95f,.95f);
			aS->mRotationDir=1;
			aS->mFade=3.0f;
			aS->mFadeSpeed=-(.2f+gRand.GetF(.1f));
			aS->mGravity=aGravity;
			aS->mColor=Color(1,0,0);
			aS->ColorStep(Color(0,.15f,.15f));
			mAnimatorList_Top+=aS;

			aS=new Anim_Sprite(&gBundle_Play->mSplodefragment);
			aS->mPos=aExplodePos;
			aS->mRotation=270;
			aS->mRotation+=gRand.GetF(20);
			aS->mDir=Point(-1,1)*(5+gRand.GetF(aSpeedMod));
			aS->mDirMult=Point(.95f,.95f);
			aS->mRotationDir=-1;
			aS->mFade=3.0f;
			aS->mFadeSpeed=-(.2f+gRand.GetF(.1f));
			aS->mGravity=aGravity;
			aS->mColor=Color(1,0,0);
			aS->ColorStep(Color(0,.15f,.15f));
			mAnimatorList_Top+=aS;



/*
			aS=new Anim_Sprite(&gBundle_Play->mExplode);
			aS->mPos=aExplodePos;
			aS->mRotation=gRand.GetF(360);
			aS->mScale=1.0f+gRand.GetF(.25f);
			aS->mScale*=1.5f;
			aS->mFade=1.0f;
			aS->mFadeSpeed=-(.1f+gRand.GetF(.1f));
			mAnimatorList_Top+=aS;
*/

			float aAngleOffset=gRand.GetF(360);
			for (float aCount=0;aCount<360;aCount+=30)
			{
				aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mDir=gMath.AngleToVector(aCount+aAngleOffset+gRand.GetSpanF(-10,10))*(5+gRand.GetF(5));
				aS->mPos=aExplodePos+(aS->mDir*(2+gRand.GetF(.5f)));

				aS->mColor=Color(1.0f,gRand.GetF(1.0f),.1f);
				aS->mRotation=gRand.GetF(360);
				aS->mScale=1.0f+gRand.GetF(.5f);
				aS->mFade=1.0f;
				aS->mDirMult=Point(.9f,.9f);
				aS->mFadeSpeed=-(.1f+gRand.GetF(.1f));
				mAnimatorList_Top+=aS;
			}



			//
			// Test adjacents!
			//
			for (int aCount=0;aCount<8;aCount++)
			{
				IPoint aTest=aP->mPos+gMath.GetCardinal8Direction(aCount);
				int aBlock=gWorld->GetGrid(aTest.mX,aTest.mY);

				if (aBlock==EXPLODE_BLOCK || aBlock==EXPLODE_CRATE)
				{
					Explode(aTest.mX,aTest.mY);
				}
				if (aBlock==ACID_BLOCK)
				{
					Melt(aTest.mX,aTest.mY);
				}
				if (aBlock==BREAK_BRICK) Smash(GridToWorld(aTest));
			}
			mExploderList-=aP;
			delete aP;
			EnumListRewind(Exploder);
		}
	}
}

void World::UpdateAmbient()
{
	//
	// Handle any in-game animators or generators or processors...
	//
	Point aRealCenter=mScrollCenter+mScrollOffset;
	IPoint aGridPos=WorldToGrid(aRealCenter);
	IPoint aSuperGridPos=GridToSuperGrid(aGridPos);

	SuperGrid *aSG[10];
	int aSCount=0;

	aSG[aSCount]=GetSuperGrid(aSuperGridPos);
	if (aSG[aSCount]) if (aSG[aSCount]->mProcessorList.GetCount()) aSCount++;

	for (int aCount=0;aCount<8;aCount++)
	{
		aSG[aSCount]=GetSuperGrid(aSuperGridPos+gMath.GetCardinal8Direction(aCount));
		if (aSG[aSCount]) if (aSG[aSCount]->mProcessorList.GetCount()) aSCount++;
	}

	for (int aCount=0;aCount<aSCount;aCount++)
	{
		EnumList(Processor,aP,aSG[aCount]->mProcessorList)
		{
			aP->Update();
		}
	}
	

	//
	// Handle music changes...
	//
	mMusicDelay=_max(0,mMusicDelay-1);
	if (mWantNewMusic)
	{
		if (mMusicDelay<=0)
		{
			mWantNewMusic=false;
			if (mCurrentMusic!=mNewMusic)
			{
				mCurrentMusic=mNewMusic;
				mMusicDelay=5*50;			// Don't let it switch music for five seconds...

				//gApp.mMusic.FadeOut(.1f);

				if (mPlayMusic) gApp.FadeInMusic(gApp.mMusic_Gameplay[_clamp(0,mCurrentMusic,4)],.1f);//.FadeIn(Sprintf("gameplay%d",mCurrentMusic),.1f);
				else gApp.FadeAllMusicExcept(NULL);
			}
		}
	}

	//
	// Handle removing boss blocks...
	//
	//if (mRemoveBossBlockCountdown>0)
	if (mRemoveBossBlockPos.Size()>0)
	{
		if (--mRemoveBossBlockCountdown<=0)
		{
			IPoint aBossBlockPos=mRemoveBossBlockPos[0];
			mRemoveBossBlockPos.DeleteElement(0);

			IPoint aClosest;
			float aBestDist=-1;
			EnumList(IPoint,aIP,mBossBlockList)
			{
				float aDist=gMath.DistanceSquared((float)aBossBlockPos.mX,(float)aBossBlockPos.mY,(float)aIP->mX,(float)aIP->mY);
				if (aDist<aBestDist || aBestDist==-1)
				{
					aBestDist=aDist;
					aClosest=*aIP;
				}
			}

			if (aBestDist!=-1) if (RemoveBossBlock(aClosest))
			{
				//
				// Play sound...
				//
				gSounds->mEvaporate.Play();//GetVolume(GridToWorld(aClosest)));
			}
		}
	}
	if (mRemoveBitcoinBlockCountdown>0)
	{
		if (--mRemoveBitcoinBlockCountdown==0) if (AllBitcoins()) gSounds->mEvaporate.PlayPitched(1.5f);
	}
}

void Bubbler::Update()
{
	mCountdown--;
	if (mCountdown<=0)
	{
		if (mRect.Intersects(gWorld->mLiveRect))
		{
			Anim_Bubble *aB=new Anim_Bubble(mPos);
			aB->mType=mType;
			gWorld->mAnimatorList_Top+=aB;
		}

		mCountdown=50+gRand.GetSpan(-25,25);
	}
}

World::SuperGrid *World::GetSuperGrid(int theX, int theY)
{
	if (theX<0 || theY<0) return NULL;
	if (theX>=mSuperGridWidth || theY>=mSuperGridHeight) return NULL;
	return &mSuperGrid[theX+(theY*mSuperGridWidth)];
}

void World::OpenComputerDoor(IPoint thePos)
{
	//
	// Opens nearest computer door to this spot...
	// Search onscreen for a computer door...
	// Find nearest...
	//
	IPoint aBest;
	float aBestDist=-1;
	
	Point aRealCenter=mScrollCenter+mScrollOffset;
	IPoint aUpperLeft=WorldToGrid(aRealCenter.mX-(gG.WidthF()/2/mZoom),aRealCenter.mY-(320/2/mZoom));
	IPoint aLowerRight=WorldToGrid(aRealCenter.mX+(gG.WidthF()/2/mZoom),aRealCenter.mY+(320/2/mZoom));
	aLowerRight+=IPoint(1,1);

	if (mComputerDoorRect.GetRectCount()==0)
	{
		for (int aSpanX=aUpperLeft.mX;aSpanX<=aLowerRight.mX;aSpanX++)
		{
			for (int aSpanY=aUpperLeft.mY;aSpanY<=aLowerRight.mY;aSpanY++)
			{
				Grid *aG=GetGridPtr(aSpanX,aSpanY);
				if (aG) if (aG->mLayout==COMPUTER_DOOR)
				{
					float aDist=gMath.DistanceSquared((float)thePos.mX,(float)thePos.mY,(float)aSpanX,(float)aSpanY);
					if (aDist<aBestDist || aBestDist==-1)
					{
						aBest=IPoint(aSpanX,aSpanY);
						aBestDist=aDist;
					}
				}
			}
		}
	}
	else
	{
		for (int aCount=0;aCount<mComputerDoorRect.GetRectCount();aCount++)
		{
			IPoint aUpperLeft=mComputerDoorRect.GetRect(aCount).UpperLeft();
			IPoint aLowerRight=mComputerDoorRect.GetRect(aCount).LowerRight();
			for (int aSpanX=aUpperLeft.mX;aSpanX<=aLowerRight.mX;aSpanX++)
			{
				for (int aSpanY=aUpperLeft.mY;aSpanY<=aLowerRight.mY;aSpanY++)
				{
					Grid *aG=GetGridPtr(aSpanX,aSpanY);
					if (aG) if (aG->mLayout==COMPUTER_DOOR)
					{
						float aDist=gMath.DistanceSquared((float)thePos.mX,(float)thePos.mY,(float)aSpanX,(float)aSpanY);
						if (aDist<aBestDist || aBestDist==-1)
						{
							aBest=IPoint(aSpanX,aSpanY);
							aBestDist=aDist;
						}
					}
				}
			}
		}
	}

	if (aBestDist>=0)
	{
		//
		// Okay, we have our door... 
		// By the game rules, it'll either be vertical, or horizontal, right?
		// So check H first, then check V...
		//
		bool aIsH=false;
		Grid *aG;

		aG=GetGridPtr(aBest.mX-1,aBest.mY);
		if (aG) if (aG->mLayout==COMPUTER_DOOR) aIsH=true;
		aG=GetGridPtr(aBest.mX+1,aBest.mY);
		if (aG) if (aG->mLayout==COMPUTER_DOOR) aIsH=true;

		List aMoveList;
		aMoveList+=new IPoint(aBest.mX,aBest.mY);
		IPoint aMoveDir;

		if (aIsH)
		{
			aMoveDir=Point(-1,0);
			if (thePos.mX>aBest.mX) aMoveDir=Point(1,0);

			int aLeftStep=1;
			int aRightStep=1;

			while (aLeftStep!=-1 || aRightStep!=-1)
			{
				if (aLeftStep!=-1)
				{
					aG=GetGridPtr(aBest.mX-aLeftStep,aBest.mY);
					if (!aG) aLeftStep=-1;
					if (aG)
					{
						if (aG->mLayout==COMPUTER_DOOR)
						{
							aMoveList+=new IPoint(aBest.mX-aLeftStep,aBest.mY);
							aLeftStep++;
						}
						else aLeftStep=-1;
					}

				}
				if (aRightStep!=-1)
				{
					aG=GetGridPtr(aBest.mX+aRightStep,aBest.mY);
					if (!aG) aRightStep=-1;
					if (aG)
					{
						if (aG->mLayout==COMPUTER_DOOR)
						{
							aMoveList+=new IPoint(aBest.mX+aRightStep,aBest.mY);
							aRightStep++;
						}
						else aRightStep=-1;
					}
				}
			}
		}
		else
		{
			aMoveDir=Point(0,1);
			if (thePos.mY>=aBest.mY) aMoveDir=Point(0,-1);

			int aUpStep=1;
			int aDownStep=1;

			while (aUpStep!=-1 || aDownStep!=-1)
			{
				if (aUpStep!=-1)
				{
					aG=GetGridPtr(aBest.mX,aBest.mY-aUpStep);
					if (!aG) aUpStep=-1;
					if (aG)
					{
						if (aG->mLayout==COMPUTER_DOOR)
						{
							aMoveList+=new IPoint(aBest.mX,aBest.mY-aUpStep);
							aUpStep++;
						}
						else aUpStep=-1;
					}

				}
				if (aDownStep!=-1)
				{
					aG=GetGridPtr(aBest.mX,aBest.mY+aDownStep);
					if (!aG) aDownStep=-1;
					if (aG)
					{
						if (aG->mLayout==COMPUTER_DOOR)
						{
							aMoveList+=new IPoint(aBest.mX,aBest.mY+aDownStep);
							aDownStep++;
						}
						else aDownStep=-1;
					}
				}
			}
		}

		EnumList(IPoint,aP,aMoveList)
		{
			int aSteps=1;
			//
			// Count from here to where we're going...
			//
			while (aSteps<mGridWidth)
			{
				IPoint aTest=(*aP)+(aMoveDir*aSteps);
				
//				if (GetGrid(aP->mX+(aSteps*(int)aMoveDir.mX),aP->mY)!=COMPUTER_DOOR) break;
				if (GetGrid(aTest.mX,aTest.mY)!=COMPUTER_DOOR) break;
				aSteps++;
			}

			Sprite *aSprite=GetDisplayTile(*aP);
			Anim_Sprite *aS=new Anim_Sprite(aSprite);
			aS->mPos=GridToWorld(aP->mX,aP->mY);
			aS->mTimer=(40*aSteps)/20;
			aS->mDir=Point(aMoveDir.mX,aMoveDir.mY)*20;
			mAnimatorList_UnderWorld+=aS;
		}

		EnumList(IPoint,aP,aMoveList)
		{
			Grid *aG=GetGridPtr(aP->mX,aP->mY);
			if (aG)
			{
				SetGrid(aP->mX,aP->mY,0);
				//aG->mLayout=0;
				aG->mPaint=0;
				aG->mPaintID=0;
			}
			aMoveList-=aP;
			delete aP;
			EnumListRewind(IPoint);
		}

		gSounds->mComputerdoor.Play();
	}

}

void World::OpenVirusDoor(IPoint thePos)
{
	//
	// Opens nearest virus door to this spot...
	// Search onscreen for a computer door...
	// Find nearest...
	//
	IPoint aBest;
	float aBestDist=-1;

	Point aRealCenter=GridToWorld(thePos);//mScrollCenter+mScrollOffset;
	IPoint aUpperLeft=WorldToGrid(aRealCenter.mX-(480/2/mZoom),aRealCenter.mY-(320/2/mZoom));
	IPoint aLowerRight=WorldToGrid(aRealCenter.mX+(480/2/mZoom),aRealCenter.mY+(320/2/mZoom));

	aLowerRight+=IPoint(1,1);

	if (mVirusDoorRect.GetRectCount()==0)
	{
		for (int aSpanX=aUpperLeft.mX;aSpanX<=aLowerRight.mX;aSpanX++)
		{
			for (int aSpanY=aUpperLeft.mY;aSpanY<=aLowerRight.mY;aSpanY++)
			{
				Grid *aG=GetGridPtr(aSpanX,aSpanY);
				if (aG) if (aG->mLayout==VIRUS_DOOR)
				{
					float aDist=gMath.DistanceSquared((float)thePos.mX,(float)thePos.mY,(float)aSpanX,(float)aSpanY);
					if (aDist<aBestDist || aBestDist==-1)
					{
						aBest=IPoint(aSpanX,aSpanY);
						aBestDist=aDist;
					}
				}
			}
		}
	}
	else
	{
		for (int aCount=0;aCount<mVirusDoorRect.GetRectCount();aCount++)
		{
			IPoint aUpperLeft=mVirusDoorRect.GetRect(aCount).UpperLeft();
			IPoint aLowerRight=mVirusDoorRect.GetRect(aCount).LowerRight();
			for (int aSpanX=aUpperLeft.mX;aSpanX<=aLowerRight.mX;aSpanX++)
			{
				for (int aSpanY=aUpperLeft.mY;aSpanY<=aLowerRight.mY;aSpanY++)
				{
					Grid *aG=GetGridPtr(aSpanX,aSpanY);
					if (aG) if (aG->mLayout==VIRUS_DOOR)
					{
						float aDist=gMath.DistanceSquared((float)thePos.mX,(float)thePos.mY,(float)aSpanX,(float)aSpanY);
						if (aDist<aBestDist || aBestDist==-1)
						{
							aBest=IPoint(aSpanX,aSpanY);
							aBestDist=aDist;
						}
					}
				}
			}
		}
	}

	if (aBestDist>=0)
	{
		//
		// Okay, we have our door... 
		// By the game rules, it'll either be vertical, or horizontal, right?
		// So check H first, then check V...
		//
		bool aIsH=false;
		Grid *aG;

		aG=GetGridPtr(aBest.mX-1,aBest.mY);
		if (aG) if (aG->mLayout==VIRUS_DOOR) aIsH=true;
		aG=GetGridPtr(aBest.mX+1,aBest.mY);
		if (aG) if (aG->mLayout==VIRUS_DOOR) aIsH=true;

		List aMoveList;
		aMoveList+=new IPoint(aBest.mX,aBest.mY);
		IPoint aMoveDir;

		if (aIsH)
		{
			aMoveDir=Point(-1,0);
			if (thePos.mX>aBest.mX) aMoveDir=Point(1,0);

			int aLeftStep=1;
			int aRightStep=1;

			while (aLeftStep!=-1 || aRightStep!=-1)
			{
				if (aLeftStep!=-1)
				{
					aG=GetGridPtr(aBest.mX-aLeftStep,aBest.mY);
					if (!aG) aLeftStep=-1;
					if (aG)
					{
						if (aG->mLayout==VIRUS_DOOR)
						{
							aMoveList+=new IPoint(aBest.mX-aLeftStep,aBest.mY);
							aLeftStep++;
						}
						else aLeftStep=-1;
					}

				}
				if (aRightStep!=-1)
				{
					aG=GetGridPtr(aBest.mX+aRightStep,aBest.mY);
					if (!aG) aRightStep=-1;
					if (aG)
					{
						if (aG->mLayout==VIRUS_DOOR)
						{
							aMoveList+=new IPoint(aBest.mX+aRightStep,aBest.mY);
							aRightStep++;
						}
						else aRightStep=-1;
					}
				}
			}
		}
		else
		{
			aMoveDir=Point(0,1);
			if (thePos.mY>=aBest.mY) aMoveDir=Point(0,-1);

			int aUpStep=1;
			int aDownStep=1;

			while (aUpStep!=-1 || aDownStep!=-1)
			{
				if (aUpStep!=-1)
				{
					aG=GetGridPtr(aBest.mX,aBest.mY-aUpStep);
					if (!aG) aUpStep=-1;
					if (aG)
					{
						if (aG->mLayout==VIRUS_DOOR)
						{
							aMoveList+=new IPoint(aBest.mX,aBest.mY-aUpStep);
							aUpStep++;
						}
						else aUpStep=-1;
					}

				}
				if (aDownStep!=-1)
				{
					aG=GetGridPtr(aBest.mX,aBest.mY+aDownStep);
					if (!aG) aDownStep=-1;
					if (aG)
					{
						if (aG->mLayout==VIRUS_DOOR)
						{
							aMoveList+=new IPoint(aBest.mX,aBest.mY+aDownStep);
							aDownStep++;
						}
						else aDownStep=-1;
					}

				}
			}
		}

		EnumList(IPoint,aP,aMoveList)
		{
			int aSteps=1;
			//
			// Count from here to where we're going...
			//
			while (aSteps<mGridWidth)
			{
				IPoint aTest=(*aP)+(aMoveDir*aSteps);

				//				if (GetGrid(aP->mX+(aSteps*(int)aMoveDir.mX),aP->mY)!=VIRUS_DOOR) break;
				if (GetGrid(aTest.mX,aTest.mY)!=VIRUS_DOOR) break;
				aSteps++;
			}

			Sprite *aSprite=GetDisplayTile(*aP);
			Anim_Sprite *aS=new Anim_Sprite(aSprite);
			aS->mPos=GridToWorld(aP->mX,aP->mY);
			aS->mTimer=(40*aSteps)/20;
			aS->mDir=Point(aMoveDir.mX,aMoveDir.mY)*20;
			mAnimatorList_UnderWorld+=aS;

		}
		EnumList(IPoint,aP,aMoveList)
		{
			Grid *aG=GetGridPtr(aP->mX,aP->mY);
			if (aG)
			{
				SetGrid(aP->mX,aP->mY,0);
				//aG->mLayout=0;
				aG->mPaint=0;
				aG->mPaintID=0;
			}
			aMoveList-=aP;
			delete aP;
			EnumListRewind(IPoint);
		}

		gSounds->mComputerdoor.Play();
	}

}

void World::Touch(IPoint thePos)
{
	int aLayout=GetGrid(thePos.mX,thePos.mY);
	if (aLayout==33) SwitchMusic(1);
	if (aLayout==34) SwitchMusic(2);
	if (aLayout==35) SwitchMusic(3);

	//
	// Reveal grid around us...
	//
	if (mLastTouch.mX!=thePos.mX || mLastTouch.mY!=thePos.mY)
	{
		RevealMap(thePos);
	}
	mLastTouch=thePos;
}

void World::RevealMap(IPoint thePos)
{
	
	MAPTYPE *aGPtr=GetMapPtr(thePos.mX,thePos.mY);
	if (aGPtr)
	{
		//
		// -1 means we already revealed here...
		//
		if (*aGPtr==-1) return;
		if (*aGPtr==99 || *aGPtr==SECRET_BLOCK) *aGPtr=-1;
	}
	
	//
	// Okay, process our map positions!
	//
#define REVEALSIZEX 7
#define REVEALSIZEY 5
	for (int aSpanX=thePos.mX-REVEALSIZEX;aSpanX<=thePos.mX+REVEALSIZEX;aSpanX++)
	{
		for (int aSpanY=thePos.mY-REVEALSIZEY;aSpanY<=thePos.mY+REVEALSIZEY;aSpanY++)
		{
			MAPTYPE *aMapPtr=GetMapPtr(aSpanX,aSpanY);
			if (aMapPtr) if (*aMapPtr==0)
			{
				MAPTYPE aMe=IsMapMarkable(aSpanX,aSpanY);
				if (aMe)
				{
					*aMapPtr=aMe;
				}
			}
		}
	}
}

char gCanMark[]=
{
	1,1,2,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,0,
	0,0,0,1,1,1,1,1,1,1,1,
	0,0,0,0,1,1,1,1,0,1,1,
	1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,
	// ADDITEM Add Item
};


MAPTYPE World::IsMapMarkable(int theX, int theY)
{
	int aMe=GetGrid(theX,theY);
	if (aMe==2)
	{
		Grid *aGG=GetGridPtr(theX,theY);
		if (aGG->mPaint>46+47+47+47+47 && aGG->mPaint<=46+47+47+47+47+47) return FixMe(98);
	}
	return FixMe(aMe);

	if (gCanMark[aMe]==2) return FixMe(aMe);
	if (gCanMark[aMe]==1)
	{
		for (int aCount=0;aCount<8;aCount++)
		{
			IPoint aDir=gMath.GetCardinal8Direction(aCount);
			int aOther=GetGrid(theX+aDir.mX,theY+aDir.mY);
			if (aOther!=aMe) return FixMe(aMe);
		}
	}

	return 0;
}


MAPTYPE *World::GetMapPtr(int theX, int theY)
{
	if (theX<0 || theY<0) return NULL;
	if (theX>=mGridWidth || theY>=mGridHeight) return NULL;
	return &mLevelMap[theX+(theY*mGridWidth)];
}


void World::KilledBoss(IPoint thePos)
{
	mTimerPause+=(10*50);

	if (mBossBlockList.GetCount()==0) return;

	{
		if (mRemoveBossBlockCountdown<=0) mRemoveBossBlockCountdown=75;
		mRemoveBossBlockPos+=thePos;//aClosest;
	}
}

bool World::AllBitcoins()
{
	if (mBitcoinBlockList.GetCount()<=0) return false;
	EnumList(IPoint,aIP,mBitcoinBlockList)
	{
		Anim_Evaporate *aE=new Anim_Evaporate(GetDisplayTile(*aIP),GridToWorld(*aIP));
		mAnimatorList_UnderPlayers+=aE;

		SetGrid(aIP->mX,aIP->mY,0);
		//aG->mLayout=0;
		Grid *aG=GetGridPtr(aIP->mX,aIP->mY);
		aG->mPaint=0;
		aG->mPaintID=0;
	}

	_FreeList(IPoint,mBitcoinBlockList);
	return true;
}

bool World::RemoveBossBlock(IPoint thePos)
{
	Grid *aG=GetGridPtr(thePos.mX,thePos.mY);
	if (aG) if (aG->mLayout==BOSS_BLOCK)
	{
		EnumList(IPoint,aIP,mBossBlockList)
		{
			if (aIP->mX==thePos.mX && aIP->mY==thePos.mY)
			{
				mBossBlockList-=aIP;
				delete aIP;
				EnumListRewind(IPoint);
			}
		}

		Anim_Evaporate *aE=new Anim_Evaporate(GetDisplayTile(thePos),GridToWorld(thePos));
		mAnimatorList_UnderPlayers+=aE;

		SetGrid(thePos.mX,thePos.mY,0);
		//aG->mLayout=0;
		aG->mPaint=0;
		aG->mPaintID=0;

		for (int aCount=0;aCount<8;aCount++) RemoveBossBlock(thePos+gMath.GetCardinal8Direction(aCount));
		return true;
	}
	return false;
}

bool World::IsPlayerAlreadyHere(int thePlayer, Point thePos)
{
	EnumList(Player,aP,mPlayerList)
	{
		if (aP->mID==thePlayer) if (gMath.DistanceSquared(thePos,aP->mPos)<=2*2) return true;
	}
	return false;
}

void World::PositionPlayers(bool addPlayers)
{
	gNoFixing=true;
	int aGooMax=0;
	//
	// First pass... replace tiles with players...
	//

	mComputerDoorRect.Reset();
	mVirusDoorRect.Reset();

	bool aHasTelematic=false;
	List aTopperList;
	List aBottomList;
	for (int aSpanX=0;aSpanX<mGridWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mGridHeight;aSpanY++)
		{
			Player *aP=NULL;
			bool aDontClearBlock=false;
			int aItem=GetGrid(aSpanX,aSpanY);
			bool aBottomGrid=true;
			bool aTopper=false;
			if (aItem==COMPUTER_DOOR) mComputerDoorRect.Add(Rect((float)aSpanX,(float)aSpanY,1,1));
			if (aItem==VIRUS_DOOR) mVirusDoorRect.Add(Rect((float)aSpanX,(float)aSpanY,1,1));
			if (aItem==2) 
			{
				int aItem=GetGrid(aSpanX,aSpanY+1);
				if (aItem!=2)
				{
					//
					// Bottom of goo... add a bubbler...
					//
					Grid *aGG=GetGridPtr(aSpanX,aSpanY);
					int aGooType=-1;
					if (aGG->mPaint>46+47+47+47 && aGG->mPaint<=46+47+47+47+47)	aGooType=0; // Red Goo
					if (aGG->mPaint>46+47+47+47+47 && aGG->mPaint<=46+47+47+47+47+47)	aGooType=1; // Green Goo

					if (aGooType>=0)
					{
						Bubbler *aB=new Bubbler();
						aB->mPos=GridToWorld(aSpanX,aSpanY);
						int aTopY=aSpanY;

						for (int aTestY=aSpanY;aTestY>0;aTestY--)
						{
							int aIsGoo=GetGrid(aSpanX,aTestY);
							if (ISLAVA(aIsGoo)) aTopY=aTestY;
						}

						Point aTop=GridToWorld(aSpanX,aTopY);
						aB->mRect=Rect(aTop.mX-mHalfGrid.mX,aTop.mY-mHalfGrid.mY,(float)mGridSize,(aB->mPos.mY+mHalfGrid.mY)-aTop.mY);
						aB->mRect.Expand(100);
						aB->mType=aGooType;

						SuperGrid *aS=GetSuperGrid(GridToSuperGrid(aSpanX,aSpanY));
						if (aS) aS->mProcessorList+=aB;
						else delete aB;
					}
				}
			}

			if (addPlayers)
			{
				if (aItem==4) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Jump;aP=aPP;}
				if (aItem==5) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_DoubleJump;aP=aPP;}
				if (aItem==6) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Shoot;aP=aPP;}
				if (aItem==7) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Rocket;aP=aPP;}
				if (aItem==8) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_RocketUp;aP=aPP;}
				if (aItem==9) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Annihiliate;aP=aPP;}
				if (aItem==10) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Helmet;aP=aPP;}
				if (aItem==11) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Redkey;aP=aPP;}
				if (aItem==12) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Greenkey;aP=aPP;}
				if (aItem==13) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Bluekey;aP=aPP;}
				if (aItem==25) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Explozor;aP=aPP;}
				if (aItem==26) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Haxxor;aP=aPP;}
				if (aItem==27) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Timesaver;aP=aPP;}
				if (aItem==28) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Teleport;aP=aPP;}
				if (aItem==PICKUP_VELCRO) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Velcro;aP=aPP;}
				if (aItem==PICKUP_SHOOTUP) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_ShootUp;aP=aPP;}
				if (aItem==20) {Computron *aC=new Computron;aP=aC;aBottomGrid=true;}
				if (aItem==21) {RedGuy *aR=new RedGuy;aP=aR;aBottomGrid=false;}
				if (aItem==22) {BlueGuy *aR=new BlueGuy;aP=aR;aBottomGrid=false;}
				if (aItem==23) {Drip *aR=new Drip;aP=aR;aBottomGrid=false;}
				if (aItem==24) {Boss *aB=new Boss;aP=aB;aBottomGrid=false;}
				if (aItem==31) {Computer *aB=new Computer;aP=aB;aBottomGrid=true;}
				if (aItem==36) {RedBoss *aB=new RedBoss;aP=aB;aBottomGrid=false;}
				if (aItem==41) {DripBoss *aB=new DripBoss;aP=aB;aBottomGrid=false;}
				if (aItem==BOSS_BLOCK) {IPoint *aPoint=new IPoint(aSpanX,aSpanY);mBossBlockList+=aPoint;}
				if (aItem==BITCOIN_DOOR) {IPoint *aPoint=new IPoint(aSpanX,aSpanY);mBitcoinBlockList+=aPoint;}
				if (aItem==TELEMATIC_BLOCK) {aHasTelematic=true;aDontClearBlock=true;Telematic *aT=new Telematic;aP=aT;aBottomGrid=false;aT->mClockwise=false;}
				if (aItem==TELEMATIC_SLOT) {aDontClearBlock=true;Telematic_Slot *aT=new Telematic_Slot;aP=aT;aBottomGrid=false;}
				if (aItem==TELEPAD_BLOCK) {Telepad *aPad=new Telepad;aP=aPad;aBottomGrid=true;aTopper=true;}
				if (aItem==42) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Life;aP=aPP;}
				if (aItem==TELEMATIC_BLOCK2) {aHasTelematic=true;aDontClearBlock=true;Telematic *aT=new Telematic;aP=aT;aBottomGrid=false;aT->mClockwise=true;}
				if (aItem==GUN_TARGET) {GunTarget* aGT=new GunTarget;aP=aGT;aDontClearBlock=true;}
				if (aItem==53) {Greenguy* aGG=new Greenguy;aP=aGG;aBottomGrid=true;}
				if (aItem==56) {Buzzoid* aB=new Buzzoid;aP=aB;aBottomGrid=false;}
				if (aItem==57) {Coin* aC=new Coin;aP=aC;aBottomGrid=false;mMaxCoins++;}
				if (aItem==LEVER_BLOCK) {Lever *aL=new Lever;aP=aL;aBottomGrid=false;}
				if (aItem==RADIO_BLOCK) {Radio *aR=new Radio;aR->mGridPos=IPoint(aSpanX,aSpanY);aP=aR;aBottomGrid=false;}
				if (aItem==PHAGE_BLOCK) {Phage* aPh=new Phage;aP=aPh;aBottomGrid=false;}
				if (aItem==MONSTERENERGY) {aDontClearBlock=true;MonsterEnergy* aE=new MonsterEnergy;aP=aE;}

				//
				// New Stuff
				//
				/*
				if (aItem==44) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_JumpCancel;aP=aPP;}
				if (aItem==45) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_DoubleJumpCancel;aP=aPP;}
				if (aItem==46) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_ShootCancel;aP=aPP;}
				if (aItem==47) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_RocketCancel;aP=aPP;}
				if (aItem==48) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_RocketUpCancel;aP=aPP;}
				if (aItem==49) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_AnnihiliateCancel;aP=aPP;}
				if (aItem==50) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_HelmetCancel;aP=aPP;}
				if (aItem==51) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_ExplozorCancel;aP=aPP;}
				*/
				if (aItem==44) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_ShootCancel;aP=aPP;}
				if (aItem==45) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_RocketUpCancel;aP=aPP;}
				if (aItem==46) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_HelmetCancel;aP=aPP;}
				if (aItem==50) {Pickup *aPP=new Pickup();aPP->mType=Pickup::Pickup_Goldkey;aP=aPP;}
			}
			else
			{
				if (aItem==BOSS_BLOCK) {IPoint *aPoint=new IPoint(aSpanX,aSpanY);mBossBlockList+=aPoint;}
				if (aItem==BITCOIN_DOOR) {IPoint *aPoint=new IPoint(aSpanX,aSpanY);mBitcoinBlockList+=aPoint;}
				if (aItem==TELEMATIC_BLOCK) {aHasTelematic=true;}
			}

			if (aItem==54 || aItem==55 || aItem==CONVEYOR_RIGHT || aItem==CONVEYOR_LEFT || aItem==ZOING_BLOCK || aItem==COSMIC_BLOCK || aItem==SLAM_BLOCK) 
			{
				Grid *aGG=GetGridPtr(aSpanX,aSpanY);
				if (aGG) 
				{
					aGG->mCustomDraw=1;
					if (aItem==ZOING_BLOCK)
					{
						Grid *aGGUp=GetGridPtr(aSpanX,aSpanY-1);
						if (aGGUp && aGGUp->mLayout!=ZOING_BLOCK) aGG->mExtraData=1;	// Indicates top of zoinger!
					}
				}
			}


			if (aP)
			{
				if (!aDontClearBlock) SetGrid(aSpanX,aSpanY,0);

				aP->mPos=GridToWorld(aSpanX,aSpanY);

				if (aP->mID==ID_TELEMATIC || aP->mID==ID_TELEMATIC_SLOT || aP->mID==ID_TELEPAD || aP->mID==ID_GUNTARGET)
				{
					if (IsPlayerAlreadyHere(aP->mID,aP->mPos))
					{
						delete aP;
						aP=NULL;
					}
				}

				if (aP)
				{
					if (aTopper) aTopperList+=aP;
					else 
					{
						if (aBottomGrid) aBottomList+=aP;//mPlayerList.mList.Insert(aP);
						else mPlayerList+=aP;
					}
				}
			}
		}
	}

	//
	// Put all bottom list things into the bottom list...
	// 
	EnumList(Player,aP,aBottomList) mPlayerList.Insert(aP);

	//
	// Second pass... connect teleporters to nearest Telematic...
	//

	if (addPlayers)
	{
		mPlayerList-=mRobot;
		mPlayerList+=mRobot;
		mPlayerList-=mKitty;
		mPlayerList+=mKitty;
		EnumList(Player,aP,aTopperList) mPlayerList+=aP;
	}
	if (aHasTelematic) ConnectTeleporters();

	List aPadList;
	EnumList(Player,aP,mPlayerList)
	{
		if (aP->mID==ID_TELEMATIC)
		{
			Telematic *aT=(Telematic*)aP;
			aT->AddCrystal(0);
		}
		if (aP->mID==ID_TELEPAD)
		{
			mPlayerList-=aP;
			aPadList+=aP;
			EnumListRewind(Player);
		}
	}

	EnumList(Player,aP,aPadList) mPlayerList+=aP;
	//mPlayerList+=aPadList;
	gNoFixing=false;

	mComputerDoorRect.Consolidate();
	mVirusDoorRect.Consolidate();
	_DLOG("***********************************************************");
	_DLOG("Computer Door Rects: %d",mComputerDoorRect.GetRectCount());
	_DLOG("Virus Door Rects: %d",mVirusDoorRect.GetRectCount());
	_DLOG("***********************************************************");

}

void World::ConnectTeleporters()
{
	List aFixList;
	List aClearList;
	EnumList(Player,aP,mPlayerList)
	{
		bool aConnect=false;
		if (aP->mID==ID_TELEMATIC_SLOT) aConnect=true;
		if (aP->mID==ID_TELEPAD) aConnect=true;

		if (aConnect)
		{
			//
			// Find nearest telematic
			// Make a temporary grid with some random obstacles...
			// Pathfind there!
			//
			Telematic *aTelematic=NULL;
			float aBestDist=9999999;

			EnumList(Player,aTTest,mPlayerList)
			{
				if (aTTest->mID!=ID_TELEMATIC) continue;
				float aDist=gMath.Distance(aTTest->mPos,aP->mPos);
				if (aDist<aBestDist)
				{
					aBestDist=aDist;
					aTelematic=(Telematic*)aTTest;
				}
			}
			if (aTelematic)
			{
				//
				// Tell the telematic here's a telepad...
				//
				if (aP->mID==ID_TELEPAD) aTelematic->mTelepadList+=aP;

				IPoint aLinkerPos=WorldToGrid(aP->mPos);

				IPoint aTMPos=WorldToGrid(aTelematic->mPos);
				IPoint aUpperLeft=IPoint(_min(aLinkerPos.mX,aTMPos.mX),_min(aLinkerPos.mY,aTMPos.mY));
				aUpperLeft+=IPoint(-1,-1);

				int aGWidth=(abs(aLinkerPos.mX-aTMPos.mX)+3);
				int aGHeight=(abs(aLinkerPos.mY-aTMPos.mY)+3);

				IPoint aPath1=aLinkerPos-aUpperLeft;
				IPoint aPath2=aTMPos-aUpperLeft;

				//
				// If you mod the pipes at all, you can set them up to target a different place here.
				//

				//if (aLinkerPos.mY<aTMPos.mY) aPath2.mY--;
				//if (aLinkerPos.mY>aTMPos.mY) aPath2.mY++;

				char *aGrid=new char[aGWidth*aGHeight];
				memset(aGrid,0,aGWidth*aGHeight);

				//
				// Pipes can only go through walls...
				//
				for (int aSpanX=0;aSpanX<aGWidth;aSpanX++)
				{
					for (int aSpanY=0;aSpanY<aGHeight;aSpanY++)
					{
						if (GetGrid(aSpanX+aUpperLeft.mX,aSpanY+aUpperLeft.mY)==0)
						{
							aGrid[aSpanX+(aSpanY*aGWidth)]=1;
						}
					}
				}

				{
					Pathfinder aPathfinder;
					aPathfinder.HookToGrid(aGrid,aGWidth,aGHeight);
					bool aResult=aPathfinder.Go(aPath1.mX,aPath1.mY,aPath2.mX,aPath2.mY);
					if (aResult)
					{
					
						//
						// Plot the pipe.
						//
						if (!mOverGrid)
						{
							mOverGrid=new char[mGridWidth*mGridHeight];
							memset(mOverGrid,0,mGridWidth*mGridHeight);
						}

						//
						// Plot on our machines so that we get proper pipes...
						//
						{
							int aPlotPos=aLinkerPos.mX+(aLinkerPos.mY*mGridWidth);
							mOverGrid[aPlotPos]=64;
							aClearList+=new IPoint(aLinkerPos.mX,aLinkerPos.mY);
						}
						{
							int aPlotPos=aTMPos.mX+(aTMPos.mY*mGridWidth);
							mOverGrid[aPlotPos]=64;
							aClearList+=new IPoint(aTMPos.mX,aTMPos.mY);
						}


						List &aList=aPathfinder.GetPath();
						EnumList(Pathfinder::PathInfo,aPI,aList)
						{
							IPoint aGridPos=aPI->mPos;
							aGridPos+=aUpperLeft;

							int aPlot=1;
							switch (aPI->mLineType)
							{
							case STRAIGHT_HORIZONTAL:aPlot=2;break;
							case STRAIGHT_VERTICAL:aPlot=3;break;
							case CORNER_UPLEFT:aPlot=5;break;
							case CORNER_UPRIGHT:aPlot=4;break;
							case CORNER_DOWNLEFT:aPlot=6;break;
							case CORNER_DOWNRIGHT:aPlot=7;break;
							}

							int aPlotPos=aGridPos.mX+(aGridPos.mY*mGridWidth);
							if (mOverGrid[aPlotPos]==0 || mOverGrid[aPlotPos]==aPlot) mOverGrid[aPlotPos]=aPlot;
							else 
							{
								aFixList+=new IPoint(aGridPos.mX,aGridPos.mY);
								//break;
							}
						}

						//
						// Hook up so we know who owns this telematic...
						//
						if (aP->mID==ID_TELEMATIC_SLOT) {Telematic_Slot *aS=(Telematic_Slot*)aP;aS->mMyTelematic=IPoint(aTMPos.mX,aTMPos.mY);/*aS->mMyTelematicPointer=aTelematic;*/aTelematic->mCrystalsNeeded++;}
						if (aP->mID==ID_TELEPAD) 
						{
							Telepad *aPad=(Telepad*)aP;
							aPad->mMyTelematic=IPoint(aTMPos.mX,aTMPos.mY);

							Pathfinder::PathInfo *aPI=(Pathfinder::PathInfo*)aList[0];
							aPI->mPos+=aUpperLeft;
							if (aPI->mPos.mX==aLinkerPos.mX)
							{
								if (aPI->mPos.mY>aLinkerPos.mY) aPad->mFacing=0;
								else aPad->mFacing=1;
							}
							else
							{
								if (aPI->mPos.mX<aLinkerPos.mX) aPad->mFacing=3;
								else aPad->mFacing=2;
							}

						}

					}
				}


				delete [] aGrid;
			}
		}
	}

	EnumList(IPoint,aP,aFixList)
	{
		//
		// This fixes grid positions that might be crossed over
		// or T shaped...
		//
		int aPos=aP->mX+(aP->mY*mGridWidth);
		int aPlot=0;

		if (mOverGrid[aPos-1] && mOverGrid[aPos+1] && mOverGrid[aPos-mGridWidth] && mOverGrid[aPos+mGridWidth]) aPlot=1;
		else if (mOverGrid[aPos-1] && mOverGrid[aPos+1] && mOverGrid[aPos-mGridWidth]) aPlot=9;
		else if (mOverGrid[aPos-1] && mOverGrid[aPos+1] && mOverGrid[aPos+mGridWidth]) aPlot=8;
		else if (mOverGrid[aPos-1] && mOverGrid[aPos+mGridWidth] && mOverGrid[aPos-mGridWidth]) aPlot=10;
		else if (mOverGrid[aPos+1] && mOverGrid[aPos+mGridWidth] && mOverGrid[aPos-mGridWidth]) aPlot=11;

		//aPlot=1;
		if (aPlot) mOverGrid[aPos]=aPlot;
	}

	//
	// These are grid positions to clear... they're just helper data...
	//
	EnumList(IPoint,aP,aClearList)
	{
		int aPos=aP->mX+(aP->mY*mGridWidth);
		mOverGrid[aPos]=0;
	}


	_FreeList(IPoint,aFixList);
	_FreeList(IPoint,aClearList);

	EnumList(Player,aP,mPlayerList)
	{
		if (aP->mID==ID_TELEMATIC)
		{
			Telematic *aT=(Telematic*)aP;
			aT->SortTelepads();
		}
	}

	//
	// Fix telematics for "Crystals needed"
	//
	EnumList(Telematic,aP,mPlayerList) if (aP->mID==ID_TELEMATIC) aP->mCrystalsNeeded=0;
	EnumList(Telematic_Slot,aT,mPlayerList) if (aT->mID==ID_TELEMATIC_SLOT)
	{
		Telematic* aTT=GetTelematic(aT->mMyTelematic);
		if (aTT) aTT->mCrystalsNeeded++;
		//aT->mMyTelematic=IPoint(aTMPos.mX,aTMPos.mY);/*aS->mMyTelematicPointer=aTelematic;*/aTelematic->mCrystalsNeeded++;}

	}

}

Telematic *World::GetTelematic(IPoint thePos)
{
	EnumList(Player,aP,mPlayerList)
	{
		if (aP->mID==ID_TELEMATIC)
		{
			IPoint aGrid=WorldToGrid(aP->mPos.mX,aP->mPos.mY);
			if (aGrid.mX==thePos.mX && aGrid.mY==thePos.mY) return (Telematic*)aP;
		}
	}
	return NULL;
}

void World::Crack(Point thePos)
{
	IPoint aPos=WorldToGrid(thePos.mX,thePos.mY);
	int aBlock=GetGrid(aPos.mX,aPos.mY);
	if (aBlock==3)
	{
		struct Crack *aFound=NULL;
		EnumList(struct Crack,aC,mCrackList)
		{
			if (aC->mPos.mX==aPos.mX && aC->mPos.mY==aPos.mY) 
			{
				aFound=aC;
				aFound->mLevel++;
				break;
			}
		}
		if (!aFound)
		{
			aFound=new struct Crack();
			aFound->mPos=aPos;
			aFound->mLevel=0;
			mCrackList+=aFound;
		}
		if (aFound->mLevel>=2)
		{
			Smash(thePos);
		}
	}
}

void World::Win()
{
	if (mWin) return;

	gAppPtr->ForceTouchEnd(true);
	mButtons.Reset();

	if (gWorldEditor)
	{
		gSounds->mMeow.Play();
		mWin=true;
		AddMessage("ROBOT GOT KITTY","YOU WIN");
		mFlashWhite=1.0f;

		gWorldEditor->mTestedOK=true;
		if (!mDied) 
		{
			gWorldEditor->mTestedNoDying=true;
			//gOut.Out("Tested no dying!");
		}
		return;
	}

	gSounds->mMeow.Play();
	gApp.FadeAllMusicExcept(NULL,.5f);
	//gApp.mMusic.FadeOut(.5f);
	mWin=true;
	mFlashWhite=1.0f;

	if (gMakermall && gMakermall->mPlayingID>=0)
	{
		gMakermall->SendIWon(mPlayTime);
		if (gGame) gGame->Save();	// Removes the savegame...
		//gMakermall->mPlayingID=-1;
	}

	WinGame *aWin=new WinGame();
	gApp.AddCPU(aWin);

	DisableTouch();
	gApp.RefreshTouches();

	if (!gMakermall)//!gApp.mIsKittyConnect)
	{
		gApp.mJustGotHS=gApp.mHS[gApp.mGameType].AddScore(gApp.mRememberName,mTimer);
		gApp.mJustGotHSGameType=gApp.mGameType;

		for (int aCount=0;aCount<MyApp::GameType_Max;aCount++)
		{
			if (gApp.mGameType==gLevelOrder[aCount])
			{
				gHighScoreListNumber=aCount;
				break;
			}
		}

		gApp.SubmitHighScore(gApp.mGameType,"MY*NAME",mTimer);
		//
		// Let app know this level is won
		//
		gApp.WinLevel(gApp.mGameType);

		if (!mDied) gApp.WinLevelNoDying(gApp.mGameType);

		//
		// Are any items still around?
		//
		bool aItems=false;
		EnumList(Player,aP,mPlayerList) if (aP->mID==ID_PICKUP) aItems=true;
		if (!aItems) gApp.WinLevelGotEverything(gApp.mGameType);

		//
		// Any enemies still around?
		//
		bool aEnemies=false;
		EnumList(Player,aP,mPlayerList) if (aP->mID==ID_REDGUY || aP->mID==ID_BLUEGUY || aP->mID==ID_DRIP || aP->mID==ID_BOSS || aP->mID==ID_REDBOSS || aP->mID==ID_DRIPBOSS) aEnemies=true;
		EnumList(Player,aP,mPausedPlayerList) if (aP->mID==ID_REDGUY || aP->mID==ID_BLUEGUY || aP->mID==ID_DRIP || aP->mID==ID_BOSS || aP->mID==ID_REDBOSS || aP->mID==ID_DRIPBOSS) aEnemies=true;
		if (!aEnemies) gApp.WinLevelKillEverything(gApp.mGameType);

		gApp.SaveGameData();
	}
}

void World::PositionButtons()
{
	mButtons.MoveButton(mButton_Move,gApp.mControlPos[0]);
	mButtons.MoveButton(mButton_Jump,gApp.mControlPos[1]);
	mButtons.MoveButton(mButton_Shoot,gApp.mControlPos[2]);
	mButtons.MoveButton(mButton_Rocket,gApp.mControlPos[3]);
	mButtons.MoveButton(mButton_RocketUp,gApp.mControlPos[4]);

	mButtonPos[mButton_Move]=mButtons.GetButtonRect(mButton_Move).UpperLeft();
	mButtonPos[mButton_Jump]=mButtons.GetButtonRect(mButton_Jump).UpperLeft();
	mButtonPos[mButton_Shoot]=mButtons.GetButtonRect(mButton_Shoot).UpperLeft();
	mButtonPos[mButton_Rocket]=mButtons.GetButtonRect(mButton_Rocket).UpperLeft();
	mButtonPos[mButton_RocketUp]=mButtons.GetButtonRect(mButton_RocketUp).UpperLeft();
	
	if (mRobot)
	{
		mButtons.EnableButton(mButton_Jump,mRobot->mCanJump);
		mButtons.EnableButton(mButton_Shoot,mRobot->mCanShoot);
		mButtons.EnableButton(mButton_Rocket,mRobot->mCanRocket);
		mButtons.EnableButton(mButton_RocketUp,mRobot->mCanRocketUp);
	}
	
	gApp.mUseJoypad=true;
	if (gApp.mUseJoypad) mButtons.EnableButton(mButton_Move,true);
	else mButtons.EnableButton(mButton_Move,false);

}


Player *World::CreatePlayerByID(int theID)
{
	Player *aP=NULL;
	switch (theID)
	{
	case ID_NULL:aP=new Player();break;
	case ID_ROBOT:aP=new Robot();break;
	case ID_LAZOR:aP=new Lazor();break;
	case ID_KITTY:aP=new Kitty();break;
	case ID_PICKUP:aP=new Pickup();break;
	case ID_COMPUTRON:aP=new Computron();break;
	case ID_REDGUY:aP=new RedGuy();break;
	case ID_BLUEGUY:aP=new BlueGuy();break;
	case ID_DRIP:aP=new Drip();break;
	case ID_ACID:aP=new Acid();break;
	case ID_BOSS:aP=new Boss();break;
	case ID_COMPUTER:aP=new Computer();break;
	case ID_REDBOSS:aP=new RedBoss();break;
	case ID_REDBABY:aP=new RedBaby(Point(0,0));break;
	case ID_TELEMATIC:aP=new Telematic();break;
	case ID_TELEMATIC_SLOT:aP=new Telematic_Slot();break;
	case ID_TELEPAD:aP=new Telepad();break;
	case ID_DRIPBOSS:aP=new DripBoss();break;
	case ID_ACIDBURNER:aP=new AcidBurner();break;
	case ID_GUNTARGET:aP=new GunTarget();break;
	case ID_GREENGUY:aP=new Greenguy();break;
	case ID_BUZZOID:aP=new Buzzoid();break;
	case ID_COIN:aP=new Coin();break;
	case ID_LEVER:aP=new Lever();break;
	case ID_RADIO:aP=new Radio();break;
	case ID_PHAGE:aP=new Phage();break;
	case ID_MONSTERENERGY:aP=new MonsterEnergy;break;
	}

	return aP;
}

void World::Notify(void *theData)
{
	if (mTesting) return;
	if (theData==&mMenuButton)
	{
		//gSounds->mClick.Play();
		if (mInGameMenu)
		{
			InGameMenu *aIG=(InGameMenu*)mInGameMenu;
			gSounds->mSelect.PlayPitched(1.25f);
			aIG->mFadeSpeed=-.1f;
			aIG->mFadeResult=0;
		}
		else
		{
			//gSounds->mSelect.Play();
			InGameMenu *aIG=new InGameMenu();
			aIG->HookTo(&mInGameMenu);
			gApp.AddCPU(aIG);
			Pause(true);
		}
	}
}

String World::GetRadioMessage(IPoint thePos)
{
	EnumSmartList(RadioText,aRT,mRadioTextList)
	{
		if (aRT->mPos==thePos) return aRT->mText;
	}
	return "";
}

