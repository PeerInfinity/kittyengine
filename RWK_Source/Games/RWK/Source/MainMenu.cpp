#include "MainMenu.h"
#include "MyApp.h"
#include "Player.h"
#include "World.h"
#include "Game.h"

#define SCREENSIZE (gG.Width())
#define SCREENSIZEF (gG.WidthF())

float gNotched=0;

float gAdOffset_Keys=50;
float gAdOffset_Resume=40;	// Ad offset for resume screen

bool ReturnToMakermall(CPU* theFrom)
{
	if (!gMakermall) return false;

	Transition* aT=new Transition(theFrom,gMakermall);
	return true;
}

#define GAMESPACING 95

bool gStartedOF=false;



MainMenu *gMainMenu=NULL;
ControlGadget *gControlGadget=NULL;
KittyConnect *gKittyConnect=NULL;
InGameMenu *gInGameMenu=NULL;
WinGame	*gWin=NULL;
int gHighScoreListNumber=0;
int gCanDefault=true;
Point gScrollLevels=Point(119,0);

char *gLevelNames[]=
{
	"NOVICE LEVEL",
	"CLASSIC LEVEL",
	"EXPERT LEVEL",
	"MASTER LEVEL",
	"SHORT & BOSSY",
	"ELECTRIC MAYHEM",
	""
};

char gLevelOrder[]=
{
	MyApp::GameType_Kid,
	MyApp::GameType_Expert,
	MyApp::GameType_Bossy,
	MyApp::GameType_Mayhem,
	MyApp::GameType_Master,
	MyApp::GameType_Flash,
	MyApp::GameType_Max
};

MainMenu::MainMenu(void)
{
	mHiliteLevel=-1;
	gMainMenu=this;
	mHideFlashingScore=false;

	mHSScroll=0;
	mBlinkerScrollTo=180;
	mManualScroll=false;
	mScrollSpeed=0;

	ComputeMaxScroll();

	gApp.FadeInMusic(gApp.mMusic_Title,.5f);

	float aNeg=gRand.NegateF(1.0f);
	mPulse[0]=gRand.GetF(360);
	mPulse[1]=mPulse[0]+90;
	mPulse[2]=mPulse[0]+90+90;
	mPulse[3]=mPulse[0]+90+90+90;

	mStep=0;
	mForeStep=0;

	mRobot=new Robot;
	mRobot->mPos=Point(50,240);
	mRobot->mPushing=true;
	mFlash=0;
	mFade=0;
	mFadeOut=false;
	
	mMainTitleOffset=0;
	mSetupOffset=-1000;
	mHighScoreOffset=-1000;
	mAddonsOffset=-1000;
	mLevelPickerOffset=-1000;
	mResumeOffset=-1000;
	mScrolling=0;

	mSetup.Go();
	mSetup.mNotify=this;

	FocusBackButton();
}

MainMenu::~MainMenu(void)
{
	if (gMainMenu==this) gMainMenu=NULL;
}

void MainMenu::Back()
{
	MsgBox* aBox=new MsgBox;
	aBox->GoX("DO YOU WANT TO EXIT KITTYENGINE?",gGCenter(),1,MBHOOK(
		{
			if (theResult=="YES") gAppPtr->Quit();
		}
	));
}


void MainMenu::ComputeMaxScroll()
{
	mMaxScroll=0;
	for (int aCount=0;aCount<gApp.mHS[gLevelOrder[gHighScoreListNumber]].mScoreCount;aCount++)
	{
		int aSeconds=gApp.mHS[gLevelOrder[gHighScoreListNumber]].GetScore(aCount);
		if (aSeconds==0) break;
		mMaxScroll+=25;
	}
	mMaxScroll-=182;
	if (gApp.mJustGotHS!=-1) mMaxScroll+=20;
	else mMaxScroll+=10;
	if (mMaxScroll<0) mMaxScroll=0;
}

void MainMenu::Initialize()
{
	mX+=gReserveLeft;mWidth-=gReserveRight+gReserveLeft;
}


void MainMenu::Update()
{
	float aSpeed=2;
	mPulse[0]+=aSpeed;
	mPulse[1]+=aSpeed;
	mPulse[2]+=aSpeed;
	mPulse[3]+=aSpeed;

	mStep+=.25f;
	if (mStep>=256) mStep-=256;

	mForeStep+=.5f;
	if (mForeStep>=40) mForeStep-=40;

	mRobot->mBob+=(1.5f)*7.0f;
	if (mRobot->mBob>=3600) mRobot->mBob-=3600;

	mFlash=_max(0,mFlash-.1f);
	if (mFadeOut && mFlash==0)
	{
		mFade=_min(1.0f,mFade+.025f);
		if (mFadeAction==2) mFade=_min(1.0f,mFade+.075f);
		if (mFade>=1.0f)
		{
#ifdef EDITORONLY
			gApp.GoKittyConnect();
#else
			switch (mFadeAction)
			{
			case 1:
				{
					gApp.mGameOrder=mWantLevel;
					if (mWantLevel==0) {gApp.mGameType=MyApp::GameType_Kid;gApp.GoNewGame("NOVICELEVEL");}
					if (mWantLevel==1) {gApp.mGameType=MyApp::GameType_Flash;gApp.GoNewGame("FLASHLEVEL");}
					if (mWantLevel==2) {gApp.mGameType=MyApp::GameType_Expert;gApp.GoNewGame("EXPERTLEVEL");}
					if (mWantLevel==3) {gApp.mGameType=MyApp::GameType_Master;gApp.GoNewGame("MASTERLEVEL");}
					if (mWantLevel==4) {gApp.mGameType=MyApp::GameType_Bossy;gApp.GoNewGame("SHORTBOSSY");}
					if (mWantLevel==5) {gApp.mGameType=MyApp::GameType_Mayhem;gApp.GoNewGame("ELECTRICMAYHEM");}
					break;
				}

			case 2:gApp.GoKittyConnect();break;
			}
#endif

		}
	}
	else if (!mFadeOut)
	{
		mFade=_max(0,mFade-.1f);
		if (!gStartedOF)
		if (mFade<=0.0f)
		{
			gStartedOF=true;
		}
		
	}

	if (mScrolling>0) for (int aCount=0;aCount<25;aCount++)
	{
		if (mScrolling>0)
		{
			mScrolling--;
			mMainTitleOffset--;
			mSetupOffset--;
			mHighScoreOffset--;
			mAddonsOffset--;
			mLevelPickerOffset--;
		}
	}
	if (mHighScoreOffset>=0) UpdateHighScores();
}

Point gMenu_SetupPos;
Point gMenu_PlayPos;
Point gMenu_ScoresPos;
Point gMenu_NewsPos;
Point gMenu_DeleteAdsPos;
void MainMenu::DrawMainTitle()
{
	gG.PushTranslate();

	gG.Translate(mMainTitleOffset,0.0f);
	gBundle_Play->mRWK.Center(mWidth/2,79.0f+10.0f);

	gMenu_PlayPos=Point((mWidth/2)-70,207.0f+15-10+10);
	gMenu_ScoresPos=Point((mWidth/2)+70,207.0f+15-10+10);
	gMenu_SetupPos=Point(mWidth-49-gNotched,46.0f);
	gMenu_NewsPos=Point(42.0f+gNotched,46.0f);
	gMenu_DeleteAdsPos=Point(-999999,-999999);

	gG.SetColor(.75f+(gMath.Sin(mPulse[0])*.2f));
	gBundle_Play->mPlay.Center(gMenu_PlayPos);
	gG.SetColor(.75f+(gMath.Sin(mPulse[1])*.2f));
	gBundle_Play->mMakerMall.Center(gMenu_ScoresPos);
	gG.SetColor(.75f+(gMath.Sin(mPulse[2])*.2f));
	gBundle_Play->mSetup.Center(gMenu_SetupPos);
	gG.SetColor(.75f+(gMath.Sin(mPulse[3])*.2f));

	int aNNumber=0;
	if (IsNews()) aNNumber=(gApp.AppTime()/10)%2;
	gBundle_Play->mNews[aNNumber].DrawScaled(gMenu_NewsPos,.9f);

	gG.SetColor();

	gG.PushTranslate();
	gG.Translate(gNotched,15.0f);
	mRobot->DrawBot();
	Point aAngle;
	aAngle.mX=gMath.AngleToVector(mRobot->mBob/10).mX*15;
	aAngle.mY=gMath.AngleToVector((mRobot->mBob/5)+90).mY*10;
	gG.Translate(-(gReserveRight+gReserveLeft),0);

	gG.SetColor((float)fabs(gMath.Sin(mRobot->mBob))*.5f);
	gBundle_Play->mEngineglow.Center(Point(gG.WidthF()-70,202.5f+30)+aAngle);
	gG.SetColor();
	gBundle_Play->mKittyship.Center(Point(gG.WidthF()-70,202.5f)+aAngle);
	gG.PopTranslate();

	
	gG.Translate(-mMainTitleOffset,0.0f);

	gG.PopTranslate();
	
}

void MainMenu::Draw()
{
	gG.Translate(-mStep,0.0f);

	int aW=(gG.Width()/256)+2;
	int aH=(gG.Height()/256)+1;

	for (int aSpanX=0;aSpanX<aW;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<aH;aSpanY++)
		{
			DrawBKGTile(aSpanX*256.0f,aSpanY*256.0f);
		}
	}

	gG.Translate(mStep,0.0f);
	gG.SetColor(.99f);
	Quad aP;
	Color aC[4];
	float aRed=(float)fabs(gMath.Sin(mRobot->mBob/10)*.5f);
	float aGreen=0.0f;
	float aBlue=.5f;
	aP.mCorner[0]=Point(0,0);
	aP.mCorner[1]=Point(gG.Width(),0);
	aP.mCorner[2]=Point(0,320);
	aP.mCorner[3]=Point(gG.Width(),320);
	aC[0]=Color(aRed,aGreen,aBlue,0);
	aC[1]=Color(aRed,aGreen,aBlue,0);
	aC[2]=Color(aRed,aGreen,aBlue,.5f);
	aC[3]=Color(aRed,aGreen,aBlue,.5f);
	gG.FillQuad(aP,aC[0],aC[1],aC[2],aC[3]);
	gG.SetColor();


	if (mMainTitleOffset>-SCREENSIZE) DrawMainTitle();
	if (mSetupOffset>-SCREENSIZE) {gG.Translate(mSetupOffset,0.0f);mSetup.Draw();gG.Translate(-mSetupOffset,0.0f);}
	if (mHighScoreOffset>-SCREENSIZE) DrawHighScores();
	if (mAddonsOffset>-SCREENSIZE) DrawAddons();
	if (mLevelPickerOffset>-SCREENSIZE) DrawLevelPicker();
	if (mResumeOffset>-SCREENSIZE) DrawResume();
	gG.SetColor();

	gG.SetColor(.65f,.75f,1);
	gG.Translate(-mForeStep,15.0f);
	for (float aX=0;aX<gG.Width()+40;aX+=40)
	{
		gBundle_Tiles->mTile_Paint[94+21]->Draw(aX,261);
		gBundle_Tiles->mTile_Paint[94+43]->Draw(aX,261+40);
	}
	gG.Translate(mForeStep,-15.0f);

	if (mFade)
	{
		gG.SetColor(0,0,0,mFade);
		gG.FillRect();
		gG.SetColor();
	}

	if (mFlash)
	{
		gG.SetColor(mFlash);
		gG.FillRect();
		gG.SetColor();
	}
}

float gDragAmount=0;
void MainMenu::TouchStart(int x, int y)
{
	if (mScrolling) return;
	if (mFadeOut) return;

	if (mMainTitleOffset>-SCREENSIZE) ProcessTouchMainTitle(x,y);
	else if (mSetupOffset>-SCREENSIZE) mSetup.TouchStart(x,y);
	else if (mHighScoreOffset>-SCREENSIZE) ProcessTouchHighScores(x,y);
	else if (mAddonsOffset>-SCREENSIZE) ProcessTouchAddons(x,y);
	else if (mLevelPickerOffset>-SCREENSIZE) {gDragAmount=0;ProcessTouchLevelPicker(x,y);}
	else if (mResumeOffset>-SCREENSIZE) ProcessTouchResume(x,y);
}

void MainMenu::TouchEnd(int x, int y)
{
	if (mFadeOut) return;
	else if (mLevelPickerOffset>-SCREENSIZE) if (gDragAmount<GAMESPACING/2) ProcessUnTouchLevelPicker(x,y);
	mHiliteLevel=-1;
}

void MainMenu::ProcessTouchMainTitle(int x, int y)
{
	Rect aRect=Rect(104,104);
	if (aRect.GetCenterAt(gMenu_PlayPos).ContainsPoint(x,y))
	{
		gSounds->mSelect.Play();
		Transition* aT=new Transition(gMainMenu,new PickGame);
	}

	if (aRect.GetCenterAt(gMenu_NewsPos).ContainsPoint(x,y))
	{
		gSounds->mSelect.Play();

		RComm::SetNewsRead();

		Browser* aB=new Browser;
		aB->mTitle="NEWS";
		aB->mData=GetNews();
		Transition* aT=new Transition(this,aB);
	}

	if (aRect.GetCenterAt(gMenu_SetupPos).ContainsPoint(x,y))
	{
		gSounds->mSelect.Play();
		mFlash=1.0f;
		GameSettingsDialog* aD=gApp.GoSettings();
		Transition* aT=new Transition(this,aD);
	}

	if (aRect.GetCenterAt(gMenu_ScoresPos).ContainsPoint(x,y))
	{
		gSounds->mSelect.PlayPitched(1.25f);
		Transition* aT=new Transition(gMainMenu,new Makermall);
	}
}

void MainMenu::Notify(void *theData)
{
	if (theData==&mSetup)
	{
		gSounds->mSelect.PlayPitched(1.25f);
		mFlash=1.0f;
		mScrolling=SCREENSIZE;
		mMainTitleOffset=SCREENSIZEF;
	}
	if (theData==&mDoneButton)
	{
		gSounds->mSelect.PlayPitched(1.2f);

		gApp.mHS[gApp.mGameType].mScoreList[gApp.mJustGotHS].mName=mHSEditBox.GetText();
		gApp.mRememberName=mHSEditBox.GetText();
		mHideFlashingScore=false;
		*this-=&mHSEditBox;
		*this-=&mDoneButton;
		mBlinkerScrollTo=180;
	}
}


SetupController::SetupController(void)
{
	mYOffset=0;

	CheckDefaulty();
}

void SetupController::CheckDefaulty()
{
	gCanDefault=false;

	if (Point(gApp.mControlPos[0]-Rect(2,252,191,66).Center()).Length()>.5f) gCanDefault=true;
	if (Point(gApp.mControlPos[1]-Rect(412,253,66,66).Center()).Length()>.5f) gCanDefault=true;
	if (Point(gApp.mControlPos[2]-Rect(342,253,66,66).Center()).Length()>.5f) gCanDefault=true;
	if (Point(gApp.mControlPos[3]-Rect(272,253,66,66).Center()).Length()>.5f) gCanDefault=true;
	if (Point(gApp.mControlPos[4]-Rect(412,183,66,66).Center()).Length()>.5f) gCanDefault=true;

}

SetupController::~SetupController()
{
}

void SetupController::Go()
{
	mLastPlay=gApp.Tick()+100;

	mLevel=0;

	mSoundVolume.SetValue(gAudio.mSoundVolume);
	mMusicVolume.SetValue(gAudio.mMusicVolume);

	mSoundVolume.SetNotify(this);
	mMusicVolume.SetNotify(this);
}

void SetupController::Draw()
{
	//mRim.Draw(3,2+mYOffset,475,258);//SPRITERECT &gBundle_Play->mSetuprim
	gG.Translate(0.0f,mYOffset);
	CENTERGLOW("DONE",gG.HalfWidthF(),212);
	//gBundle_Play->mDone.Center(240.0f,232.5f);

	if (mLevel==0) DrawMenuLevel0();
	if (mLevel==1) DrawMenuLevel1();
	if (mLevel==2) DrawMenuLevel2();
	if (mLevel==3) DrawMenuLevel3();
	if (mLevel==4) DrawMenuLevel4();
	gG.Translate(0.0f,-mYOffset);
}

void SetupController::MouseMove(int x, int y)
{
}

void SetupController::MouseUp(int x, int y, int theButton)
{
}

void SetupController::DrawMenuLevel0()
{
}

void SetupController::DrawMenuLevel1()
{
}

void SetupController::Notify(void *theData)
{
	if (theData==&mSoundVolume) 
	{
		gAudio.SetSoundVolume(mSoundVolume.GetValue());
		if (gApp.Tick()>mLastPlay) 
		{
			gSounds->mLaser.Play();
			mLastPlay=gApp.Tick()+100;
		}
	}
	if (theData==&mMusicVolume) 
	{
		gAudio.SetMusicVolume(mMusicVolume.GetValue());
	}
}

void SetupController::DrawMenuLevel2()
{
}

void SetupController::DrawMenuLevel3()
{
	CENTERGLOW("ABOUT KITTYENGINE",gG.HalfWidthF(),10.0f);


	float aY=40;
	CENTERUNDERGLOW("ORIGINAL GAME BY",gG.HalfWidthF(),aY);aY+=15;
	CENTERUNDERGLOW("MIKE HOMMEL",240.0f,aY);aY+=25;

	DRAWUNDERGLOW("MUSIC BY",50.0f,aY);aY+=15;
	DRAWUNDERGLOW("DR. PETTER",50.0f,aY);aY-=15;

	RIGHTUNDERGLOW("IPHONE GAME BY",gG.WidthF()-50,aY);aY+=15;
	RIGHTUNDERGLOW("JOHN RAPTIS",gG.WidthF()-50,aY);aY+=25;

	aY+=10;

	CENTERUNDERGLOW("BETA TESTERS",gG.HalfWidthF(),aY);aY+=20;
	DRAWUNDERGLOW("PHIL HASSEY",50.0f,aY);
	RIGHTUNDERGLOW("INFAMOUS BUTCHER",430.0f,aY);aY+=15;
	DRAWUNDERGLOW("PHOENIX GABRIEL",50.0f,aY);
	RIGHTUNDERGLOW("CHRISTIAN THOMPSON",430.0f,aY);aY+=15;
	DRAWUNDERGLOW("BARRY HOWEL",50.0f,aY);
	CENTERUNDERGLOW("MARK CRAIG",240.0f,aY);
	RIGHTUNDERGLOW("JUSTIN YEO",430.0f,aY);aY+=15;

	aY=40;
	CENTEROVERGLOW("ORIGINAL GAME BY",240.0f,aY);aY+=15;
	CENTEROVERGLOW("MIKE HOMMEL",240.0f,aY);aY+=25;

	DRAWOVERGLOW("MUSIC BY",50.0f,aY);aY+=15;
	DRAWOVERGLOW("DR. PETTER",50.0f,aY);aY-=15;

	RIGHTOVERGLOW("IPHONE GAME BY",430.0f,aY);aY+=15;
	RIGHTOVERGLOW("JOHN RAPTIS",430.0f,aY);aY+=25;

	aY+=10;

	CENTEROVERGLOW("BETA TESTERS",240.0f,aY);aY+=20;
	DRAWOVERGLOW("PHIL HASSEY",50.0f,aY);
	RIGHTOVERGLOW("INFAMOUS BUTCHER",430.0f,aY);aY+=15;
	DRAWOVERGLOW("PHOENIX GABRIEL",50.0f,aY);
	RIGHTOVERGLOW("CHRISTIAN THOMPSON",430.0f,aY);aY+=15;
	DRAWOVERGLOW("BARRY HOWEL",50.0f,aY);
	CENTEROVERGLOW("MARK CRAIG",240.0f,aY);
	RIGHTOVERGLOW("JUSTIN YEO",430.0f,aY);aY+=15;

/*
		Christian Thompson
*/
}

void SetupController::DrawMenuLevel4()
{
}

void SetupController::MouseDown(int x, int y, int theButton)
{
	y-=(int)mYOffset;
	if (mLevel==0)
	{
		if (Rect(175,208,131,68).ContainsPoint(x,y)) {if (mNotify) mNotify->Notify(this);gApp.SaveGameData();mLevel=0;}
		else if (Rect(41,66,100,100).ContainsPoint(x,y)) 
		{
			mLevel=1;
			gSounds->mSelect.PlayPitched(1.35f);
			*mNotify+=&mSoundVolume;
			*mNotify+=&mMusicVolume;
		}
		else if (Rect(41+100,66,100,100).ContainsPoint(x,y))
		{
			gSounds->mSelect.PlayPitched(1.35f);
		}
		else if (Rect(41+100+100,66,100,100).ContainsPoint(x,y))
		{
			mLevel=3;
			gSounds->mSelect.PlayPitched(1.35f);
		}
		else if (Rect(41+100+100+100,66,100,100).ContainsPoint(x,y))
		{
			mLevel=4;
			gSounds->mSelect.PlayPitched(1.35f);
		}
	}
	else if (mLevel==1)
	{
		if (Rect(175,208,131,68).ContainsPoint(x,y)) 
		{
			*mNotify-=&mSoundVolume;
			*mNotify-=&mMusicVolume;
			mLevel=0;
			gSounds->mSelect.PlayPitched(1.0f);
		}
	}
	else if (mLevel==3)
	{
		if (Rect(175,208,131,68).ContainsPoint(x,y)) {mLevel=0;gSounds->mSelect.PlayPitched(1.0f);}
	}
	else if (mLevel==4)
	{
		if (Rect(175,208,131,68).ContainsPoint(x,y)) {mLevel=0;gSounds->mSelect.PlayPitched(1.0f);}
		if (Rect(70-5,78-5,93,94).ContainsPoint(x,y))
		{
			if (gCanDefault) 
			{
				gSounds->mSelect.PlayPitched(1.6f);
				//
				// Return all controls to default!
				//
				// FIXME Screen Resolution/Location of buttons
				gApp.mControlPos[0]=Rect(2,252,191,66).Center();
				gApp.mControlPos[1]=Rect(412,253,66,66).Center();
				gApp.mControlPos[2]=Rect(342,253,66,66).Center();
				gApp.mControlPos[3]=Rect(272,253,66,66).Center();
				gApp.mControlPos[4]=Rect(412,183,66,66).Center();
				
				gCanDefault=false;
			}

		

		}


		if (Rect(322,72,93,94).ContainsPoint(x,y))
		{
			gSounds->mSelect.PlayPitched(1.6f);
			//
			// Need to start up the customize controls gadget!
			//
			ControlGadget *aCG=new ControlGadget();
			gApp.AddCPU(aCG);
		}
	}
}


ControlGadget::ControlGadget()
{
	gControlGadget=this;
	Size();

	mSelected=-1;
	mControlSize[0]=Point(gBundle_Play->mControl_Move.mWidth,gBundle_Play->mControl_Move.mHeight)/2;
	mControlSize[1]=Point(gBundle_Play->mControl_Jump.mWidth,gBundle_Play->mControl_Jump.mHeight)/2;
	mControlSize[2]=Point(gBundle_Play->mControl_Shoot.mWidth,gBundle_Play->mControl_Shoot.mHeight)/2;
	mControlSize[3]=Point(gBundle_Play->mControl_Rocket.mWidth,gBundle_Play->mControl_Rocket.mHeight)/2;
	mControlSize[4]=Point(gBundle_Play->mControl_Rocketup.mWidth,gBundle_Play->mControl_Rocketup.mHeight)/2;
}

ControlGadget::~ControlGadget()
{
	if (gControlGadget==this) gControlGadget=NULL;
	if (gMainMenu) 
	{
		gMainMenu->mSetup.CheckDefaulty();
	}
}

void ControlGadget::Initialize()
{
	mX+=gReserveLeft;
	mWidth-=gReserveRight+gReserveLeft;
}

void ControlGadget::Update()
{
}

void ControlGadget::Draw()
{
	gG.Clear(0,0,0,1);
	gBundle_Play->mScreenshot.DrawScaled(mWidth/2,mHeight/2,2.0f);

	gG.Translate(0,45);
	CENTERUNDERGLOW("TOUCH AND DRAG TO",mWidth/2,80);
	CENTERUNDERGLOW("MOVE CONTROLS",mWidth/2,80+16);
	CENTEROVERGLOW("TOUCH AND DRAG TO",mWidth/2,80);
	CENTEROVERGLOW("MOVE CONTROLS",mWidth/2,80+16);

	CENTERUNDERGLOW("TOUCH HERE",mWidth/2,120);
	CENTERUNDERGLOW("WHEN FINISHED",mWidth/2,120+16);
	CENTEROVERGLOW("TOUCH HERE",mWidth/2,120);
	CENTEROVERGLOW("WHEN FINISHED",mWidth/2,120+16);
	gG.Translate(0,-35);

	if (gApp.mUseJoypad)
	{
		gG.SetColor(.75f);if (mSelected==0) gG.SetColor();
		gBundle_Play->mControl_Move.Center(gApp.mControlPos[0]);
	}

	gG.SetColor(.75f);if (mSelected==1) gG.SetColor();
	gBundle_Play->mControl_Jump.Center(gApp.mControlPos[1]);

	gG.SetColor(.75f);if (mSelected==2) gG.SetColor();
	gBundle_Play->mControl_Shoot.Center(gApp.mControlPos[2]);

	gG.SetColor(.75f);if (mSelected==3) gG.SetColor();
	gBundle_Play->mControl_Rocket.Center(gApp.mControlPos[3]);

	gG.SetColor(.75f);if (mSelected==4) gG.SetColor();
	gBundle_Play->mControl_Rocketup.Center(gApp.mControlPos[4]);
}

void ControlGadget::TouchStart(int x, int y)
{

	mSelected=-1;
	int aStart;
	if (gApp.mUseJoypad) aStart=0;
	else aStart=1;
	for (int aCount=aStart;aCount<5;aCount++)
	{
		Point aPos=gApp.mControlPos[aCount];
		Point aSize=mControlSize[aCount];
		if (Rect(aPos.mX-aSize.mX,aPos.mY-aSize.mY,aSize.mX*2,aSize.mY*2).ContainsPoint(x,y))
		{
			mDragOffset=aPos-Point(x,y);
			mSelected=aCount;
		}
	}
}

void ControlGadget::TouchMove(int x, int y)
{
	if (mSelected>=0 && mSelected<5)
	{
		gApp.mControlPos[mSelected]=Point(x,y)+mDragOffset;
	}
}

void ControlGadget::TouchEnd(int x, int y)
{
	if (mSelected==-1 && Rect(168,38).GetCenterAt(gG.Center()).ContainsPoint(x,y)) 
	{
		gApp.SaveGameData();
		Kill();
		return;
	}


	mSelected=-1;
}


void MainMenu::ProcessTouchHighScores(int x, int y)
{
	if (mHideFlashingScore) return;

	if (Rect(175,208,131,68).ContainsPoint(x,y))
	{
		gSounds->mSelect.PlayPitched(1.25f);
		mFlash=1.0f;
		mScrolling=SCREENSIZE;
		mMainTitleOffset=SCREENSIZEF;

		mManualScroll=true;
		gApp.mJustGotHS=-1;
		mHideFlashingScore=false;
		*this-=&mHSEditBox;
		*this-=&mDoneButton;
		//iTools_ShowKeyboard(false);
		mScoreTouchyRect=Rect(-1,-1,-1,-1);

		ComputeMaxScroll();

	}
	if (mScoreTouchyRect.ContainsPoint(x,y))
	{
		//
		// Okay, add an edit box, notify this...
		//
		gOut.Out("Enter name!");
		mHideFlashingScore=true;

		mHSEditBox.Size(mScoreTouchyRect);
		mHSEditBox.mX+=30;
		mHSEditBox.mWidth-=30;
		mHSEditBox.SetFont(&gBundle_Play->mFont_Commodore64Angled18);
		mHSEditBox.AllowCharacters(" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~`!#$%&*()-_=+;:'\",.?");
		mHSEditBox.FocusKeyboard();
		mHSEditBox.SetBackgroundColor(Color(0));
		mHSEditBox.SetTextColor(Color(0));
		mHSEditBox.SetTextOffset(Point(0,-14));

		mHSEditBox.SetMaxTextWidth((mScoreTouchyRect.mWidth-85));
		mHSEditBox.SetText(gApp.mHS[gLevelOrder[gHighScoreListNumber]].GetName(gApp.mJustGotHS));
		*this+=&mHSEditBox;

//		mDoneButton.Size(Rect(382,mScoreTouchyRect.mY-2,145,mScoreTouchyRect.mHeight));
		mDoneButton.Size(Rect(382,(float)((int)mScoreTouchyRect.mY-5),75,mScoreTouchyRect.mHeight));
		mDoneButton.SetNotify(this);
		mDoneButton.NotifyOnPress();
		*this+=&mDoneButton;

		//
		// SHOWKEYBOARD
		//
		//iTools_ShowKeyboard(true);
		mBlinkerScrollTo=80;
		mManualScroll=false;
	}

	if (gApp.mJustGotHS==-1)
	{
		//
		// Allow scrolling...
		//
		if (Rect(305,0,175,48).ContainsPoint(x,y))
		{
			//
			// Scroll Next
			//
			gSounds->mSelect.PlayPitched(1.2f);
			gHighScoreListNumber++;
			if (gHighScoreListNumber>=MyApp::GameType_Max) gHighScoreListNumber=0;
			mManualScroll=true;
			mScrollSpeed=0;
			ComputeMaxScroll();
		}
		if (Rect(0,0,175,48).ContainsPoint(x,y))
		{
			//
			// Scroll Previous
			//
			gSounds->mSelect.PlayPitched(1.2f);
			gHighScoreListNumber--;
			if (gHighScoreListNumber<0) gHighScoreListNumber=MyApp::GameType_Max-1;
			mManualScroll=true;
			mScrollSpeed=0;
			ComputeMaxScroll();
		}
		
	}
	
}

Point gDragFocus;
void MainMenu::StartDrag(int x, int y)
{
	gDragFocus=Point(x,y);
	//
	// FIXME:
	// Handling the abort drag?
	//
	//if (mHideFlashingScore) Manual_AbortDrag();
}

void MainMenu::Drag(int x, int y)
{
	if (mHighScoreOffset>=0)
	{
		float aStep=gDragFocus.mY-y;
		gDragFocus=Point(x,y);
		mManualScroll=true;
		mScrollSpeed=aStep;
	}

	if (mLevelPickerOffset>=0)
	{
		float aStep=gDragFocus.mX-x;
		gDragAmount+=(float)fabs(aStep);
		if (gDragAmount>GAMESPACING/2) mHiliteLevel=-1;

		gDragFocus=Point(x,y);
		gScrollLevels.mX+=aStep;
		if (gScrollLevels.mX<0) gScrollLevels.mX=0;
		float aMax=(GAMESPACING*(MyApp::GameType_Max-2))-(40);
		if (gScrollLevels.mX>aMax) gScrollLevels.mX=aMax;
		
	}
	
}


void MainMenu::UpdateHighScores()
{
	if (!mManualScroll)
	{
		if (mScoreTouchyRect.mY!=mBlinkerScrollTo)
		{
			float aStep=mScoreTouchyRect.mY-mBlinkerScrollTo;

			if (fabs(aStep)>.1f)
			{
				if (mHideFlashingScore) aStep*=.2f;
				else aStep*=.1f;
			}
			float aOld=mHSScroll;
			mHSScroll+=aStep;
			if (mHSScroll<0) mHSScroll=0;
			if (mHSScroll>mMaxScroll) mHSScroll=mMaxScroll;

			mHSEditBox.mY-=mHSScroll-aOld;
			mDoneButton.mY-=mHSScroll-aOld;
		}
	}
	else
	{
		mHSScroll+=mScrollSpeed;
		mScrollSpeed*=.9f;

		if (mHSScroll<0) mHSScroll=0;
		if (mHSScroll>mMaxScroll) mHSScroll=mMaxScroll;
	}

}

void MainMenu::DrawHighScores()
{
	gG.Translate(mHighScoreOffset,0.0f);

	if (gApp.mJustGotHS!=-1)
	{
		CENTERGLOW("YOU GOT A HIGH SCORE!",240,5);
		CENTERGLOW("TAP THE BLINKING SCORE TO RENAME",240,5+15);
	}
	else 
	{
		float aWidth=(float)gBundle_Play->mFont_Commodore64Angled11.Width(gLevelNames[gLevelOrder[gHighScoreListNumber]]);
		DRAWGLOW(gLevelNames[gLevelOrder[gHighScoreListNumber]],240-(aWidth/2),12);
		gBundle_Play->mScrollcaret.Center(240-((aWidth/2)+20),34);
		gBundle_Play->mScrollcaret.DrawRotated(240+((aWidth/2)+20),34,180);
	}
	CENTERGLOW("DONE",240,212);

	float aY;
	if (gApp.mJustGotHS==-1) 
	{
		aY=41-15+10;
		gG.Clip(31,41+10,418,182-10);
	}
	else
	{
		gG.Clip(31,41+20,418,182-20);
		aY=41-15+20;
	}

	aY-=mHSScroll;
	mScoreTouchyRect=Rect(-1,-1,-1,-1);

	bool aGotAScore=false;
	for (int aCount=0;aCount<gApp.mHS[gLevelOrder[gHighScoreListNumber]].mScoreCount;aCount++)
	{
		String aName=gApp.mHS[gLevelOrder[gHighScoreListNumber]].GetName(aCount);
		int aSeconds=gApp.mHS[gLevelOrder[gHighScoreListNumber]].GetScore(aCount);
		if (aSeconds==0) break;
		aSeconds/=50;

		String aTimeString;
		if (aSeconds>3600) aTimeString=Sprintf("%d:%.2d:%.2d",aSeconds/60/60,(aSeconds/60)%60,(aSeconds%60));
		else aTimeString=Sprintf("%d:%.2d",aSeconds/60,(aSeconds%60));

		bool aDraw=true;

		if (gLevelOrder[gHighScoreListNumber]==gApp.mGameType && gApp.mJustGotHS==aCount) 
		{
			if (((gApp.AppTime()/5)%2)==0 || mHideFlashingScore) aDraw=false;
			mScoreTouchyRect=Rect(29,aY+12,345,40);
		}

		gG.ClipColor(.5f);
		DRAWGLOW(Sprintf("%d.",aCount+1),31,aY+7);
		gG.ClipColor();

		if (aDraw)
		{
			if (mHideFlashingScore) if (gLevelOrder[gHighScoreListNumber]==gApp.mGameType && gApp.mJustGotHS!=aCount) 
			{
				gG.ClipColor(.5f);
			}
			DRAWGLOWBIG(aName,31+30,aY);
			RIGHTGLOWBIG(aTimeString,449,aY);
		}
		gG.ClipColor();
		aY+=25;
		aGotAScore=true;
	}

	if (!aGotAScore)
	{
		CENTERGLOW("NO SCORES YET!",240,115);
	}

	if (mHideFlashingScore)
	{
		//
		// Draw glowy edit box...
		//
		// FINISHME FIXME: Draw glowy edit box???
		/*
		gG.PushClip();
		gG.Translate(mHSEditBox.UpperLeft());
		gG.Clip(Rect((float)mHSEditBox.mBorderSize,(float)mHSEditBox.mBorderSize,mHSEditBox.mRect.mWidth-(mHSEditBox.mBorderSize*2),mHSEditBox.mRect.mHeight-(mHSEditBox.mBorderSize*2)));
		if (mHSEditBox.mStringUpToCursorWidth-mHSEditBox.mScroll>(mRect.mWidth-(mHSEditBox.mBorderSize*2))) mHSEditBox.mScroll=(int)(mHSEditBox.mStringUpToCursorWidth-(mRect.mWidth-(mHSEditBox.mBorderSize*2))+mHSEditBox.mCursorSize.mX);
		if (mHSEditBox.mStringUpToCursorWidth-mHSEditBox.mScroll<mHSEditBox.mBorderSize) mHSEditBox.mScroll=mHSEditBox.mStringUpToCursorWidth;
		float aXPos=(float)mHSEditBox.mBorderSize+mHSEditBox.mTextOffset.mX-mHSEditBox.mScroll;
		if (mHSEditBox.mCenter) aXPos=(mRect.mWidth/2)-(mHSEditBox.mFont->StringWidth(mHSEditBox.mText)/2);
		gG.PushClip();
		gG.Clip();
		if (gAppPtr->mKeyboardFocusGadget==&mHSEditBox) if (mHSEditBox.mCursorBlink) {gBundle_Play->mEditcursor.DrawScaled(Rect((float)mHSEditBox.mStringUpToCursorWidth+aXPos+mHSEditBox.mCursorOffset.mX,(float)mHSEditBox.mBorderSize+mHSEditBox.mCursorOffset.mY,mHSEditBox.mCursorSize.mX,mHSEditBox.mCursorSize.mY).UpperLeft()+Point(mHSEditBox.mCursorSize.mX/2,14.0f),1.1f);}
		gG.PopClip();
		DRAWGLOWBIG(mHSEditBox.mText,aXPos,(float)mHSEditBox.mBorderSize+mHSEditBox.mTextOffset.mY);
		gG.Translate();
		gG.PopClip();

		gG.Clip();
		Rect aRect=mDoneButton.mRect;
		aRect.mY=(float)((int)aRect.mY);
		//mSmallRim.Draw(aRect.Expand((float)(int)(15+gMath.Sin((float)gApp.mAppTime*4)*3))); //SPRITERECT &gBundle_Play->mSmallrim
		CENTERGLOW("SAVE",mDoneButton.mRect.Center().mX,mDoneButton.mRect.UpperLeft().mY-4);
		*/
	}

	gG.Clip();
	gG.Translate(-mHighScoreOffset,0.0f);
}

void MainMenu::ProcessTouchAddons(int x, int y)
{
	if (Rect(175,208,131,68).ContainsPoint(x,y))
	{
		gSounds->mSelect.PlayPitched(1.25f);
		mFlash=1.0f;
		mScrolling=SCREENSIZE;
		mMainTitleOffset=SCREENSIZEF;
	}
}

void MainMenu::DrawAddons()
{
	gG.Translate(mAddonsOffset,0.0f);

	//mSetup.mRim.Draw(3,2,475,258); SPRITERECT &gBundle_Play->mSetuprim
	CENTERGLOW("DONE",240,212);

	CENTERGLOW("COMING SOON",240,100);

	gG.Translate(-mAddonsOffset,0.0f);
}

void MainMenu::ProcessTouchLevelPicker(int x, int y)
{
	if (Rect(391,227,88,40).ContainsPoint(x,y))
	{
		gSounds->mSelect.PlayPitched(1.25f);
		mFlash=1.0f;
		mScrolling=SCREENSIZE;
		mMainTitleOffset=SCREENSIZEF;
	}

	int aFixX=x+(int)gScrollLevels.mX;	// Add scrolling
	int aFixY=y;

	if (Rect(75,70,GAMESPACING,152).ContainsPoint(aFixX,aFixY))
	{
		mHiliteLevel=100;
//		gSounds->mSelect.PlayPitched(.8f);
//		mFlash=1.0f;
	}
	else for (int aCount=0;aCount<MyApp::GameType_Max;aCount++)
	{

		if (Rect((float)(75+GAMESPACING)+(GAMESPACING*aCount),70,GAMESPACING,152).ContainsPoint(aFixX,aFixY))
		{
			mHiliteLevel=aCount;
//			gSounds->mSelect.PlayPitched(.8f);
			//mFlash=1.0f;
			break;
		}
	}

}

void MainMenu::ProcessUnTouchLevelPicker(int x, int y)
{
	int aFixX=x+(int)gScrollLevels.mX;	// Add scrolling
	int aFixY=y;

	if (Rect(75,70,GAMESPACING,152).ContainsPoint(aFixX,aFixY))
	{
		//
		// Kitty Connect!
		//
		gSounds->mSelect.PlayPitched(1.25f);
		mFlash=1.0f;

		//gApp.mMusic.FadeOut(.5f);
		mFadeOut=true;
		mFadeAction=2;
	}
	else for (int aCount=0;aCount<MyApp::GameType_Max;aCount++)
	{
		if (Rect((float)(75+GAMESPACING)+(GAMESPACING*aCount),70,GAMESPACING,152).ContainsPoint(aFixX,aFixY))
		{
			StartAGame(gLevelOrder[aCount]);
			break;
		}
	}
}


void MainMenu::StartAGame(int theLevel)
{
	gSounds->mSelect.PlayPitched(1.25f);
	mFlash=1.0f;
	mWantLevel=theLevel;

	String aLookForLevel=Sprintf("sandbox://tempsave64%d.sav",mWantLevel);
	if (DoesFileExist(aLookForLevel))
	{
		mResumeOffset=0;
		mLevelPickerOffset=-1000;
		return;
	}

	gApp.FadeAllMusicExcept(NULL,.5f);
	//gApp.mMusic.FadeOut(.5f);
	mFadeOut=true;
	mFadeAction=1;
}

void MainMenu::DrawResume()
{
	gG.PushTranslate();
	gG.Translate(mResumeOffset,45.0f);

	CENTERUNDERGLOWBIG("A SAVED GAME FOR",gG.HalfWidthF(),0);
	CENTERUNDERGLOWBIG("THIS LEVEL EXISTS!",gG.HalfWidthF(),22);
	CENTEROVERGLOWBIG("A SAVED GAME FOR",gG.HalfWidthF(),0);
	CENTEROVERGLOWBIG("THIS LEVEL EXISTS!",gG.HalfWidthF(),22);
	CENTERUNDERGLOW("WOULD YOU LIKE TO CONTINUE",gG.HalfWidthF(),100-45);
	CENTERUNDERGLOW("YOUR SAVED GAME?",gG.HalfWidthF(),114-45);
	CENTEROVERGLOW("WOULD YOU LIKE TO CONTINUE",gG.HalfWidthF(),100-45);
	CENTEROVERGLOW("YOUR SAVED GAME?",gG.HalfWidthF(),114-45);
	gG.PopTranslate();


	CENTERGLOWBIG("YES",gG.HalfWidthF()-95,172+30);
	CENTERGLOWBIG("NO",gG.HalfWidthF()+95,172+30);

}
	

void MainMenu::ProcessTouchResume(int x, int y)
{
	if (Rect(149+30,76+56).GetCenterAt(gG.HalfWidthF()-95,172+25).ContainsPoint(x,y))
	//if (Rect(70,172-10,149,76).Expand(15).ContainsPoint(x,y))
	{
		//
		// Yes
		//
		gSounds->mSelect.PlayPitched(1.25f);

		gApp.FadeAllMusicExcept(NULL,.5f);
		//gApp.mMusic.FadeOut(.5f);
		mFadeOut=true;
		mFadeAction=1;
	}
	if (Rect(149+30,76+56).GetCenterAt(gG.HalfWidthF()+95,172+25).ContainsPoint(x,y))
	//if (Rect(261,172-10,149,76).Expand(15).ContainsPoint(x,y))
	{
		//
		// No
		//
		gSounds->mSelect.PlayPitched(1.25f);

		gApp.FadeAllMusicExcept(NULL,.5f);
		mFadeOut=true;
		mFadeAction=1;
		DeleteFile(Sprintf("sandbox://tempsave64%d.sav",mWantLevel));
	}
}

void MainMenu::DrawLevelPicker()
{
}

KittyConnect::KittyConnect()
{
	gKittyConnect=this;
	Size();
	mStep=0;
	mGlow=0;

	mFade=1.0f;
	mFadeSpeed=-.1f;
}

KittyConnect::~KittyConnect()
{
	if (gKittyConnect==this) gKittyConnect=NULL;
}

void KittyConnect::Update()
{
	mStep+=.25f;
	if (mStep>=256) mStep-=256;

	mGlow++;
	if (mGlow>=360) mGlow-=360;

	if (mFadeSpeed)
	{
		mFade+=mFadeSpeed;
		if (mFade<=0.0f)
		{
			mFade=0.0f;
			mFadeSpeed=0;
		}
		if (mFade>=1.0f)
		{
			//
			//Back to main menu
			//
			if (mFadeResult==0)
			{
				gApp.GoMainMenu();
				gMainMenu->mFade=1.0f;
				gMainMenu->mMainTitleOffset=-1000;
				gMainMenu->mSetupOffset=-1000;
				gMainMenu->mHighScoreOffset=-1000;
				gMainMenu->mAddonsOffset=-1000;
				gMainMenu->mLevelPickerOffset=0;
			}
			if (mFadeResult==1)
			{
				//gApp.GoEditor();
			}
		}
	}
}

void KittyConnect::Draw()
{
	gG.Translate(0.0f,-mStep);
	for (int aSpanX=0;aSpanX<2;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<3;aSpanY++)
		{
			DrawBKGTile(aSpanX*256.0f,aSpanY*256.0f);
		}
	}
	gG.Translate(0.0f,mStep);
	gG.SetColor(.99f);
	Quad aP;
	Color aC[4];
	float aRed=0;
	float aGreen=.5f+(float)fabs(gMath.Sin(mGlow)*.5f);
	float aBlue=.5f;
	aP.mCorner[0]=Point(0,0);
	aP.mCorner[1]=Point(gG.WidthF(),0.0f);
	aP.mCorner[2]=Point(0,320);
	aP.mCorner[3]=Point(gG.WidthF(),320.0f);
	aC[0]=Color(aRed,aGreen,aBlue,0);
	aC[1]=Color(aRed,aGreen,aBlue,0);
	aC[2]=Color(aRed,aGreen,aBlue,.5f);
	aC[3]=Color(aRed,aGreen,aBlue,.5f);
	gG.FillQuad(aP,aC[0],aC[1],aC[2],aC[3]);
	gG.SetColor();

	CENTERGLOW("KITTY CONNECT",240,-10);
	DRAWGLOW("EDITOR",10,280);
	gG.ClipColor(.5f);
	CENTERGLOW("PLAY",240,280);
	gG.ClipColor();

	gBundle_Play->mBack.Draw(392,284);

	//mRim.Draw(-4+5,12+5,489-10,284-10); SPRITRECT &gBundle_Play->mSetuprim

	CENTERGLOW("KITTY CONNECT IS NOT LIVE YET...",240,100);

	CENTERGLOW("IN THE MEANTIME, PLEASE ENJOY\nTHE KITTY EDITOR!",240,150);

	if (mFade)
	{
		gG.SetColor(0,0,0,mFade);
		gG.FillRect();
		gG.SetColor();
	}
}

void KittyConnect::MouseDown(int x, int y, int theButton)
{
	if (mFadeSpeed!=0) return;

	if (Rect(373,287,107,33).ContainsPoint(x,y))
	{
		gSounds->mSelect.PlayPitched(1.0f);
		mFadeSpeed=.1f;
		mFadeResult=0;
	}
	if (Rect(0,287,107,33).ContainsPoint(x,y))
	{
		gSounds->mSelect.PlayPitched(1.5f);
		mFadeSpeed=.025f;
		gApp.FadeAllMusicExcept(NULL,1);
		mFadeResult=1;
	}
}

InGameMenu::InGameMenu()
{
	gInGameMenu=this;
	mFade=0;
	mFadeSpeed=.075f;

	gAdOffset_Keys=0;
	gAdOffset_Resume=0;	// Ad offset for resume screen

	mMapScroll=Point(0,0);

	mSetup.mYOffset=28;
	mSetup.Go();
	mSetup.mNotify=this;
	mIsSetup=false;
	mIsMap=false;
	mFadeResult=0;

	if (gWorld)
	{
		if (gWorld->mRobot)
		{
			for (int aCount=0;aCount<gWorld->mRobot->mPowerupCount;aCount++)
			{
				Powerup *aP=new Powerup;
				aP->mID=gWorld->mRobot->mPowerupOrder[aCount];
				aP->mStep=40;
				mPowerupList+=aP;
			}

			if (gWorld->mRobot->mCrystals) {Powerup *aP=new Powerup;aP->mID=28;mPowerupList+=aP;aP->mStep=35;aP->mCount=gWorld->mRobot->mCrystals;}
			if (gWorld->mRobot->mHasKey[0]) {Powerup *aP=new Powerup;aP->mID=11;mPowerupList+=aP;aP->mStep=20;}
			if (gWorld->mRobot->mHasKey[1]) {Powerup *aP=new Powerup;aP->mID=12;mPowerupList+=aP;aP->mStep=20;}
			if (gWorld->mRobot->mHasKey[2]) {Powerup *aP=new Powerup;aP->mID=13;mPowerupList+=aP;aP->mStep=20;}

			float aWidth=0;
			if (mPowerupList.GetCount()>1)
			EnumList(Powerup,aP,mPowerupList) 
			{
				if (aP==mPowerupList.FetchFirst()) aWidth+=aP->mStep/2;
				else if (aP==mPowerupList.FetchLast()) aWidth+=aP->mStep/2;
				else aWidth+=aP->mStep;
			}
			mPowerupPos=gG.HalfWidthF()-(aWidth/2);
		}
	}

}

InGameMenu::~InGameMenu()
{
	_FreeList(struct Powerup,mPowerupList);
	if (gInGameMenu==this) gInGameMenu=NULL;
}

void InGameMenu::Initialize()
{
}

void InGameMenu::Update()
{
	if (mFadeSpeed)
	{
		mFade+=mFadeSpeed;
		if (mFade>1.0f)
		{
			mFade=1.0f;
			mFadeSpeed=0;
		}
		if (mFade<=0.0f)
		{
			mFade=0.0f;
			mFadeSpeed=0;
			Kill();

			if (mFadeResult==0) if (gWorld) gWorld->Pause(false);
			if (mFadeResult==1)
			{
				if (gMakermall) 
				{
					ReturnToMakermall(gGame);
					//Transition* aT=new Transition(gGame,gMakermall);
					//gMakermall->mPlayingID=-1;
				}
				else 
				{
					Transition* aT=new Transition(gGame,new PickGame);
					mKill=false;
				}

			}
		}
	}
	else
	{
		if (!gApp.mTouchControls && mIsMap && IsKeyPressed(gApp.mMap))
		{
			gSounds->mSelect.PlayPitched(1.0);
			mFadeSpeed=-1.0f;
			mFadeResult=0;
		}
	}
}

void InGameMenu::DrawMap()
{
	if (!gWorld) return;

	float aMapSize=11;

	gG.SetColor(0,0,0,.5f);
	gG.FillRect();
	gG.SetColor();

	gG.PushTranslate();
	gG.Translate(mWidth/2,mHeight/2);
	gG.Translate(mMapScroll);

#define LINE 4
	Rect aSideRect[]=
	{
		Rect(0,0,LINE,LINE),
		Rect(0,0,aMapSize,LINE),
		Rect(aMapSize-LINE,0,LINE,LINE),
		Rect(0,0,LINE,aMapSize),
		Rect(aMapSize-LINE,0,LINE,aMapSize),
		Rect(0,aMapSize-LINE,LINE,LINE),
		Rect(0,aMapSize-LINE,aMapSize,LINE),
		Rect(aMapSize-LINE,aMapSize-LINE,LINE,LINE),
	};

	Point aStartWorld=gWorld->mRobot->mPos-mMapScroll*(aMapSize/3);
	IPoint aStartPos=gWorld->WorldToGrid(aStartWorld);

	IPoint aEndPos;

	aStartPos.mX-=(int)(gG.HalfWidthF()/aMapSize);
	aStartPos.mY-=(int)(160.0f/aMapSize);
	aEndPos.mX=aStartPos.mX+(int)(gG.WidthF()/aMapSize);
	aEndPos.mY=aStartPos.mY+(int)(320.0f/aMapSize);

	aStartPos.mX-=2;
	aEndPos.mX+=2;
	aStartPos.mX=_max(0,aStartPos.mX);
	aStartPos.mY=_max(0,aStartPos.mY);
	aEndPos.mX=_min(aEndPos.mX,gWorld->mGridWidth);
	aEndPos.mY=_min(aEndPos.mY,gWorld->mGridHeight);

	gG.Translate(-(gWorld->mRobot->mGridPos.mX*aMapSize),-(gWorld->mRobot->mGridPos.mY*aMapSize));
	for (int aSpanY=aStartPos.mY;aSpanY<aEndPos.mY;aSpanY++)
	{
		for (int aSpanX=aStartPos.mX;aSpanX<aEndPos.mX;aSpanX++)
		{
			MAPTYPE *aM=gWorld->GetMapPtr(aSpanX,aSpanY);
			if (!aM) continue;

			if (*aM>0 && *aM!=99 && *aM!=COSMIC_BLOCK)
			{
				Color aColor=Color(0,.8f,0);
				if (*aM==2) aColor=Color(1,0,0);
				if (*aM==98) aColor=Color(.6f,1,.25f);
				if (*aM==32 || *aM==37) aColor=Color(1,.8f,0);
				if (*aM==18) aColor=Color(.25f,.65f,1.0f);
				if (*aM==14) aColor=Color(1.0f,.75f,.25f);
				if (*aM==16) aColor=Color(.25f,1,.25f);
				if (*aM==CONVEYOR_LEFT || *aM==CONVEYOR_RIGHT) aColor=Color(1,0,1);

				gG.SetColor(aColor,.5f);
				gG.FillRect((aSpanX*aMapSize),(aSpanY*aMapSize),aMapSize,aMapSize);
				gG.SetColor(aColor);

				for (int aCount=0;aCount<8;aCount++)
				{
					IPoint aDir=gMath.GetCardinal8Direction(aCount);
					MAPTYPE *aMM=gWorld->GetMapPtr(aSpanX+aDir.mX,aSpanY+aDir.mY);
					if (!aMM || (aMM && *aMM!=*aM))// && *aMM!=0) // Commented this out because it seems to work better this way?
					{
						Rect aRect=Rect((aSpanX*aMapSize)+aSideRect[aCount].mX,(aSpanY*aMapSize)+aSideRect[aCount].mY,aSideRect[aCount].mWidth,aSideRect[aCount].mHeight);
						gG.FillRect(aRect);
					}
				}

				if (gWorld->mOverGrid)
				{
					gG.RenderWhite(true);
					gG.SetColor(.5f,1,.5f);
					char aType=gWorld->mOverGrid[(aSpanX)+((aSpanY)*gWorld->mGridWidth)];
					if (aType>0 && aType<64) gBundle_Tiles->mPipes[aType-1].DrawScaled((float)(aSpanX*aMapSize)+(aMapSize/2),(float)(aSpanY*aMapSize)+(aMapSize/2),.25f);
					gG.RenderWhite(false);
				}
				
			}
		}
	}

	EnumList(Player,aP,gWorld->mPlayerList)
	{
		IPoint aGridPos=gWorld->WorldToGrid(aP->mPos);
		MAPTYPE *aM=gWorld->GetMapPtr(aGridPos.mX,aGridPos.mY);
		if (!aM) continue;

		if (*aM!=0)
		{
			int aSpanX=aGridPos.mX;
			int aSpanY=aGridPos.mY;
			switch (aP->mID)
			{
			case ID_ROBOT:
				{
					gG.SetColor(1);
					if (((gAppPtr->AppTime()/3)%2)==0) gG.SetColor(0);
					gG.FillRect(aSpanX*aMapSize,aSpanY*aMapSize,aMapSize,aMapSize);
					break;
				}
			case ID_KITTY:
				{
					gG.SetColor(1,.5f,0,(float)fabs(gMath.Sin((float)gAppPtr->AppTime()*3)));
					gG.FillRect(aSpanX*aMapSize,aSpanY*aMapSize,aMapSize,aMapSize);
					break;
				}
			case ID_COMPUTRON:
			case ID_TELEMATIC:
				{
					gG.SetColor(.5f,.5f,.5f);
					gG.FillRect(aSpanX*aMapSize,aSpanY*aMapSize,aMapSize,aMapSize);
					break;
				}
			case ID_TELEPAD:
				{
					Telepad *aTP=(Telepad*)aP;
					Rect aRect;
					switch (aTP->mFacing)
					{
					case 0:aRect=Rect(0,aMapSize-2,aMapSize,2);break;
					case 1:aRect=Rect(0,0,aMapSize,2);break;
					case 2:aRect=Rect(aMapSize-2,0,2,aMapSize);break;
					case 3:aRect=Rect(0,0,2,aMapSize);break;
					
					}
					gG.SetColor(.5f,1,.5f);
					gG.FillRect((aSpanX*aMapSize)+aRect.mX,(aSpanY*aMapSize)+aRect.mY,aRect.mWidth,aRect.mHeight);
					break;
				}
			case ID_TELEMATIC_SLOT:
				{
					gG.SetColor(.75f,.5f,.75f);
					gG.FillRect(aSpanX*aMapSize,aSpanY*aMapSize,aMapSize,aMapSize);
					break;
				}
			case ID_RADIO:
			case ID_LEVER:
				{
					gG.SetColor(.75f,0,.75f);
					gG.FillRect(aSpanX*aMapSize,aSpanY*aMapSize,aMapSize,aMapSize);
					break;
				}
			case ID_COMPUTER:
				{
					gG.SetColor(.75f,.75f,.75f);
					gG.FillRect(aSpanX*aMapSize,aSpanY*aMapSize,aMapSize,aMapSize);
					break;
				}
			case ID_PICKUP:
				{
					Pickup *aPick=(Pickup*)aP;
					switch (aPick->mType)
					{
					case Pickup::Pickup_Greenkey:gG.SetColor(.25f,1.0f,.25f,(float)fabs(gMath.Sin((float)gAppPtr->AppTime()*3)));break;
					case Pickup::Pickup_Redkey:gG.SetColor(1,.75f,.25f,(float)fabs(gMath.Sin((float)gAppPtr->AppTime()*3)));break;
					case Pickup::Pickup_Bluekey:gG.SetColor(.25f,.65f,1.0f,(float)fabs(gMath.Sin((float)gAppPtr->AppTime()*3)));break;
					default:gG.SetColor(1,1,0,(float)fabs(gMath.Sin((float)gAppPtr->AppTime()*3)));break;
					}
					
					gG.FillRect(aSpanX*aMapSize,aSpanY*aMapSize,aMapSize,aMapSize);
					break;
				}
			}
		}
	}
	gG.PopTranslate();

	gG.SetColor(0,.3f,0);
	gG.FillRect(0,0,gG.WidthF(),30);
	gG.FillRect(0,320-30,gG.WidthF(),30);
	gG.SetColor(0,.8f,0);
	gG.FillRect(0,30,gG.WidthF(),2);
	gG.FillRect(0,320-32,gG.WidthF(),2);
	gG.SetColor();
	CENTERGLOW("SWIPE TO MOVE MAP",mWidth/2,20);
	CENTERGLOW("TAP TO RESUME GAME",mWidth/2,320-7);

}
void InGameMenu::Draw()
{
	if (!gWorld) return;

	gG.SetColor(0,0,0,.75f*mFade);
	gG.FillRect();
	gG.SetColor();
	if (mIsSetup) 
	{
		mSetup.Draw();
	}
	else if (mIsMap)
	{
		DrawMap();
	}
	else
	{
		gG.ClipColor(mFade);
		gG.PushTranslate();
		gG.Translate(0.0f,gAdOffset_Keys);

		float aX=mPowerupPos;
		if (gWorld->mRobot->mGoldKeyCount) aX-=(20);
		EnumList(Powerup,aP,mPowerupList)
		{
			int aID=aP->mID;
			if (aID==34) aID=65;
			if (aID==35) aID=72;
			// NEW POWERUP
			
			gBundle_Tiles->mBlock[aID].Center(aX,25);
			if (aID==72)
			{
				String aStr=Sprintf("+%d",gWorld->mRobot->mLazorLevel);
				gG.SetColor(0,0,0,1);
				gBundle_Play->mFont_Commodore64Angled11Outline.Center(aStr,aX,32+15);
				gG.SetColor();
				gBundle_Play->mFont_Commodore64Angled11.Center(aStr,aX,32+15);
				gG.SetColor();
			}

			if (aP->mCount>1)
			{
				String aStr=Sprintf("%d",aP->mCount);
				gG.SetColor(0,0,0,1);
				gBundle_Play->mFont_Commodore64Angled11Outline.Center(aStr,aX,32);
				gG.SetColor();
				gBundle_Play->mFont_Commodore64Angled11.Center(aStr,aX,32);
				gG.SetColor();
			}


			aX+=aP->mStep;
		}
		if (gWorld->mRobot->mGoldKeyCount)
		{
			gBundle_Tiles->mBlock[50].Center(aX,25);
			String aStr=Sprintf("%d",gWorld->mRobot->mGoldKeyCount);
			gG.SetColor(0,0,0,1);
			gBundle_Play->mFont_Commodore64Angled11Outline.Center(aStr,aX,32);
			gG.SetColor(1,.85f,0);
			gBundle_Play->mFont_Commodore64Angled11.Center(aStr,aX,32);
			gG.SetColor();
		}

		gG.PopTranslate();
		gG.PushTranslate();
		gG.Translate((gG.WidthF()-480)/2,0.0f);

		gG.PushTranslate();
		gG.Translate(0.0f,(-50*gMath.Cos(mFade*90))+gAdOffset_Resume);
		gBundle_Play->mResume_Ingame.Center(240,150-30);
		CENTERUNDERGLOW("RESUME",240,160-50-30);
		CENTERUNDERGLOW("GAME",240,160+14-50-30);
		CENTEROVERGLOW("RESUME",240,160-50-30);
		CENTEROVERGLOW("GAME",240,160+14-50-30);
		gG.PopTranslate();

		gG.PushTranslate();
		gG.Translate(-100*gMath.Cos(mFade*90),0.0f);
		gBundle_Play->mRestart_Ingame.Center(100,150);
		CENTERUNDERGLOW("RESTART",100-65,160-12);
		CENTERUNDERGLOW("LEVEL",100-65,160+14-12);
		CENTEROVERGLOW("RESTART",100-65,160-12);
		CENTEROVERGLOW("LEVEL",100-65,160+14-12);
		gG.PopTranslate();

		gG.PushTranslate();
		gG.Translate(100*gMath.Cos(mFade*90),0.0f);
		gBundle_Play->mQuit_Ingame.Center(380,150);
		CENTERUNDERGLOW("QUIT",380+55,160-12);
		CENTERUNDERGLOW("LEVEL",380+55,160+14-12);
		CENTEROVERGLOW("QUIT",380+55,160-12);
		CENTEROVERGLOW("LEVEL",380+55,160+14-12);
		gG.PopTranslate();

		gG.PushTranslate();
		gG.Translate(-70.0f*gMath.Sin(mFade*90),50*gMath.Sin(mFade*90));
		gBundle_Play->mSetup_Ingame.Center(240,200);
		CENTERGLOW("SETUP",240,210+32);
		gG.PopTranslate();

		gG.PushTranslate();
		gG.Translate(70.0f*gMath.Sin(mFade*90),50*gMath.Sin(mFade*90));
		gBundle_Play->mMap_Ingame.Center(240,200);
		CENTERGLOW("MAP",240,210+32);
		gG.PopTranslate();

		gG.ClipColor();


		gG.PopTranslate();
	}
}

float gScrollAccum;
Point gScrollSpot;
bool gScrolling=false;
void InGameMenu::TouchStart(int x, int y)
{
	if (mIsSetup)
	{
	}
	else if (mIsMap)
	{
		{
			gScrollAccum=0;
			gScrollSpot=Point(x,y);
			gScrolling=true;
		}
	}
	else
	{
		x-=(int)(gG.WidthF()-480)/2;

		if (Rect(110,110).GetCenterAt(240,150-30+gAdOffset_Resume).ExpandUp(30).ContainsPoint(x,y))
		{
			gSounds->mSelect.PlayPitched(1.25f);
			mFadeSpeed=-.1f;
			mFadeResult=0;
			return;
		}


		if (Rect(110,110).GetCenterAt(240-70,260).ExpandDown(30).ContainsPoint(x,y))
		{
			gSounds->mSelect.PlayPitched(1.5f);
			GameSettingsDialog* aD=gApp.GoSettings();
			aD->mReturnTo=this;
			Transition* aT=new Transition(NULL,aD);
			return;
			//mIsSetup=true;
		}

		if (Rect(110,110).GetCenterAt(240+70,260).ExpandDown(30).ContainsPoint(x,y))
		{
			gSounds->mSelect.PlayPitched(1.5f);
			mIsMap=true;
			gScrolling=false;
			gScrollAccum=1000;
		}

		// NEW COMMUNITY - Original
		/*
		if (Rect(110,110).GetCenterAt(100,150).ExpandLeft(70).ContainsPoint(x,y))
		{
			//
			// Restart Level
			//
			gSounds->mSelect.PlayPitched(1.5f);
			gWorld->mPlayMusic=false;
			gGame->RestartLevel();
			mFadeSpeed=-.1f;
			return;
		}
		*/

		// NEW COMMUNITY - restart confirmation (toggleable)
		if (Rect(110,110).GetCenterAt(100,150).ExpandLeft(70).ContainsPoint(x,y))
		{
			gSounds->mSelect.PlayPitched(1.5f);

			auto DoRestart = [&]()
			{
				gWorld->mPlayMusic = false;
				gGame->RestartLevel();

				// close the menu
				mFadeSpeed = -0.1f;
				mFadeResult = 0;
			};

			if (!gApp.mEnableRestartConfirm)
			{
				DoRestart();
				return;
			}

			MsgBox* aBox = new MsgBox;
			aBox->GoX(
				"ARE YOU SURE YOU WANT TO RESTART THE LEVEL?!",
				gGCenter(),
				1,
				MBHOOK(
				{
					if (theResult == "YES")
					{
						DoRestart();
					}
					// NO / cancel → do nothing
				})
			);

			return;
		}

		if (Rect(110,110).GetCenterAt(380,150).ExpandRight(50).ContainsPoint(x,y))
		{
			//
			// Quit Level
 			//
			gSounds->mSelect.PlayPitched(1.5f);
			gWorld->mPlayMusic=false;
			gWorld->mNeverPlayMusic=true;
			gApp.FadeInMusic(gApp.mMusic_Title);
			mFadeSpeed=-.05f;
			mFadeResult=1;
		}
	}
}

void InGameMenu::TouchMove(int x, int y)
{
	if (mIsMap)
	if (gScrolling)
	{
		Point aVec=Point(x,y)-gScrollSpot;
		gScrollAccum+=aVec.Length();
		mMapScroll+=aVec;
		gScrollSpot=Point(x,y);
	}
}

void InGameMenu::TouchEnd(int x, int y)
{
	if (gScrollAccum<10 && mIsMap)
	{
		gSounds->mSelect.PlayPitched(1.0);
		mIsMap=false;
		mFadeSpeed=-1.0f;
		mFadeResult=0;
		return;
	}
	gScrolling=false;
}


void InGameMenu::Notify(void *theData)
{
	if (theData==&mSetup)
	{
		gSounds->mSelect.PlayPitched(1.25f);
		mIsSetup=false;
	}
}

WinGame::WinGame()
{
	gWin=this;
	mFade=-2.0f;
	mBumpScale=1.0f;
	mFadeOut=false;
	mWait=0;
}

WinGame::~WinGame()
{
	if (gWin==this) gWin=NULL;
}

void WinGame::Update()
{
	mWait++;
	mBumpScale=_max(1.0f,mBumpScale-.05f);
	float aHold=mFade;

	if (mFadeOut)
	{
		mFade=_max(0.0f,mFade-.05f);
		if (mFade<=0) Kill();
		return;
	}

	mFade=_min(1.0f,mFade+.1f);
	if (mFade==1.0f && aHold<1.0f) 
	{
		mBumpScale=1.25f;
		gSounds->mWin.Play();
		gApp.FadeInMusic(gApp.mMusic_Title,1.0f);
		gWorld->mPlayMusic=false;

		if (gGame) gGame->Kill();
		//gApp->mMusic.FadeIn("title",1.0f);
	}


	if (mWait<100) return;
	if (mFadeOut) return;
	if (!gApp.mTouchControls)
	{
		if (
			IsKeyPressed(gApp.mLeft) ||
			IsKeyPressed(gApp.mRight) ||
			IsKeyPressed(gApp.mJump) ||
			IsKeyPressed(gApp.mShoot) ||
			IsKeyPressed(gApp.mRocket) ||
			IsKeyPressed(gApp.mMenu) ||
			IsKeyPressed(gApp.mRocketUp) ||
			IsKeyPressed(KB_SPACE) ||
			IsKeyPressed(KB_ESCAPE) ||
			IsKeyPressed(KB_TAB) ||
			IsKeyPressed(KB_ENTER)
			)
		{
			TouchStart(1,1);
		}
	}

	if (gMakermall && gMakermall->mWasInstaplay)
	{
		gMakermall->UpdatePendingQueryList();
	}
}

void WinGame::Draw()
{
	if (mFade>0)
	{
		gG.SetColor(0,0,0,mFade);
		gG.FillRect();
		gG.SetColor(mFade);
		gBundle_Play->mWin.DrawScaled(gG.WidthF()/2,320/2,mBumpScale);
		gG.SetColor();
	}
}

void WinGame::InstaPlayReponse()
{
	PickMenu* aPM=new PickMenu;
	aPM->mFont=&gBundle_Play->mFont_Commodore64Angled18;
	aPM->mRimBorder*=1.5f;
	aPM->mDarken=true;


	aPM->AddButton("PLAY AGAIN");
	aPM->AddButton("MORE FROM THIS CREATOR");
	aPM->GoX(gG.Center(),false,0,MBHOOK(
		{
			if (theResult.StartsWith("MORE FROM")) ReturnToMakermall(gWin);
			if (theResult.StartsWith("PLAY")) 
			{
				if (gMakermall)
				{
					gMakermall->StartPlaying(gMakermall->mPlayingID);
				}
			}
		}
	));
}


void WinGame::TouchStart(int x, int y)
{
	if (mWait<100) return;
	if (mFadeOut) return;
	DisableTouch();

	if (gMakermall) 
	{
		if (gMakermall->mWasInstaplay)
		{
			// Rate level first?
			if (gMakermall->mRatedID!=gMakermall->mPlayingID)
			{
				RatingScreen* aRS=new RatingScreen;
				aRS->mLevelID=gMakermall->mPlayingID;
				gAppPtr->AddCPU(aRS);
				aRS->Throttle(CPUHOOK(
					{
						if (gMakermall) gMakermall->mRatedID=gMakermall->mPlayingID;
						if (gWin) gWin->InstaPlayReponse();
					}
				));
			}
			else
			{
				if (gWin) gWin->InstaPlayReponse();
			}

		}
		else ReturnToMakermall(this);
	}
	else Transition* aT=new Transition(this,new PickGame);
}


#define PIXELSIZE 10
Transition::Transition(CPU* theOldCPU, CPU* theNewCPU)
{
	mOldCPU=theOldCPU;
	mNewCPU=theNewCPU;

	mProgress=0;
	
	gAppPtr->AddCPU(this);

	int aW=(gG.Width()/PIXELSIZE)+1;
	int aH=(gG.Height()/PIXELSIZE)+1;

	for (int aX=0;aX<aW;aX++)
	{
		for (int aY=0;aY<aH;aY++)
		{
			mList+=new Point(aX*PIXELSIZE,aY*PIXELSIZE);
		}
	}
	mList.Shuffle();
}

Transition::~Transition()
{
}

extern TitleBox* gTitleBox;
void Transition::Update()
{
	float aOldProgress=mProgress;
	if (mProgress>=1.0f && mProgress<1.05) mProgress=_min(2.0f,mProgress+.01f);
	else mProgress=_min(2.0f,mProgress+.075f);


	if (gMath.CrossTheshold(aOldProgress,mProgress,1.0f))
	{
		if (gTitleBox) gTitleBox->Kill();
		if (mOldCPU) mOldCPU->Kill();
		else if (gMakermall) 
		{
			gMakermall->DisableFeature(CPU_DRAW|CPU_TOUCH|CPU_UPDATE);
			gMakermall->LeaveMakermall();
		}
		gAppPtr->AddCPU(mNewCPU);
		gAppPtr->BringChildToTop(this);
		if (mNewCPU==gMakermall) 
		{
			gMakermall->ReturnToMakermall();
			gMakermall->EnableFeature(CPU_DRAW|CPU_TOUCH|CPU_UPDATE);
			gMakermall->BackFromPlaying();
		}
	}

	if (mProgress>=2.0f) Kill();
}

void Transition::Draw()
{
	if (mProgress<=1.0f)
	{
		int aEnd=(int)(gMath.Sin(mProgress*mProgress*mProgress*mProgress*90)*mList.GetCount());
		for (int aCount=0;aCount<aEnd;aCount++) gBundle_Play->mPixel_Glow.DrawScaled(mList[aCount]->mX+(PIXELSIZE/2),mList[aCount]->mY+(PIXELSIZE/2),(float)(PIXELSIZE)/20.0f);
		for (int aCount=0;aCount<aEnd;aCount++)
		{
			float aFraction=(.25f*QuickRandFraction(aCount))+.75f;
			gG.SetColor(aFraction,1,1);
			gBundle_Play->mPixel.DrawScaled(mList[aCount]->mX+(PIXELSIZE/2),mList[aCount]->mY+(PIXELSIZE/2),(float)(PIXELSIZE)/20.0f);
		}
	}
	else
	{
		int aStart=(int)((mProgress-1.0f)*mList.GetCount());
		int aEnd=mList.GetCount();
		for (int aCount=aStart;aCount<aEnd;aCount++) gBundle_Play->mPixel_Glow.DrawScaled(mList[aCount]->mX+(PIXELSIZE/2),mList[aCount]->mY+(PIXELSIZE/2),(float)(PIXELSIZE)/20.0f);
		for (int aCount=aStart;aCount<aEnd;aCount++) 
		{
			float aFraction=(.25f*QuickRandFraction(aCount))+.75f;
			gG.SetColor(aFraction,1,1);
			gBundle_Play->mPixel.DrawScaled(mList[aCount]->mX+(PIXELSIZE/2),mList[aCount]->mY+(PIXELSIZE/2),(float)(PIXELSIZE)/20.0f);
		}
	}

	gG.SetColor();
}



/////////////////////

PickGame* gPickGame=NULL;
PickGame::PickGame()
{
	HookTo(&gPickGame);

	mStep=0;
	mDidSwipe=false;
	mPickLevel=gApp.mHiliteLevel;
	mPulse=gRand.GetF(360);
	mPickScroll=0;
	mNextPickLevel=mPickLevel;
	mDown=false;

	FixLevelName(mPickLevel);

	mCheckAgain=5;

	FocusBackButton();
}

PickGame::~PickGame()
{
	if (gAppPtr)
	{
		gApp.mHiliteLevel=(int)mPickLevel;
		gApp.SaveGameData();
	}
}

void PickGame::Initialize()
{
	mX+=gReserveLeft;mWidth-=gReserveRight+gReserveLeft;

	mBack.Size(57,36);
	mBack.CenterAt(gG.UpperLeft()+Point(57.0f/2,36.0f/2)+Point(10,0));
	mBack.SetNotify(this);
	mBack.SetSounds(&gSounds->mSelect);
	*this+=&mBack;

	mPlay.Size(78,78);
	mPlay.CenterAt(Point(mWidth,mHeight)+Point(-(78/2)-gNotched,-(78.0f/2)));
	mPlay.SetNotify(this);
	mPlay.SetSounds(&gSounds->mSelect);
	*this+=&mPlay;

	mReset.Size(78,81);
	mReset.CenterAt(Point(0.0f,mHeight)+Point((78.0f/2),-(81.0f/2)));
	mReset.SetNotify(this);
	mReset.SetSounds(&gSounds->mSelect);
	*this+=&mReset;
}

void PickGame::Update()
{
	if (--mCheckAgain==0) FixLevelName(mPickLevel);

	mPulse+=2;
	mStep+=.25f;
	if (mStep>256) mStep-=256;

	if (mPickScroll)
	{
		mPickScroll+=mPickScrollStep;

		if (mPickScroll>1.0f || mPickScroll<-1.0f) 
		{
			FinishScroll();
		}
	}
}

void PickGame::FinishScroll()
{
	if (mPickScroll)
	{
		mPickScroll=0;
		mPickLevel=mNextPickLevel;
		gApp.mHiliteLevel=mPickLevel;
		FixLevelName(mPickLevel);
	}
}

void PickGame::Draw()
{
	gG.PushTranslate();
	gG.Translate(0.0f,-mStep);
	int aW=(gG.Width()/256)+1;
	int aH=(gG.Height()/256)+2;
	for (int aSpanX=0;aSpanX<aW;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<aH;aSpanY++)
		{
			DrawBKGTile(aSpanX*256.0f,aSpanY*256.0f);
		}
	}
	gG.PopTranslate();

	gG.SetColor(.99f);
	Quad aP;
	Color aC[4];
	float aRed=.25f;
	float aGreen=0.0f;
	float aBlue=1.0f;
	aP.mCorner[0]=Point(0,0);
	aP.mCorner[1]=Point(gG.Width(),0);
	aP.mCorner[2]=Point(0,320);
	aP.mCorner[3]=Point(gG.Width(),320);
	aC[0]=Color(aRed,aGreen,aBlue,0);
	aC[1]=Color(aRed,aGreen,aBlue,0);
	aC[2]=Color(aRed,aGreen,aBlue,.5f);
	aC[3]=Color(aRed,aGreen,aBlue,.5f);
	gG.FillQuad(aP,aC[0],aC[1],aC[2],aC[3]);
	gG.SetColor();

	gG.PushClip();
	Rect aRect=Rect(243,222);
	aRect.CenterAt(mWidth/2,142);
	//gG.Clip(163.0f,31.0f,243.0f,222.0f);
	gG.Clip(aRect);
	gBundle_Play->mLevelIcon[(int)mPickLevel].Center(HalfSize()+Point((mPickScroll)*gBundle_Play->mLevelIcon[(int)mPickLevel].mWidth,-16.0f));
	if (mPickScroll)
	{
		gBundle_Play->mLevelIcon[(int)mNextPickLevel].Center(HalfSize()+Point((mPickScroll)*gBundle_Play->mLevelIcon[(int)mPickLevel].mWidth+(gMath.Sign(-mPickScrollStep)*(gBundle_Play->mLevelIcon[(int)mPickLevel].mWidth)),-16.0f));
	}
	if (mLevelName.Len())
	{
		gG.ScaleView(1.25f+gMath.Sin(gAppPtr->AppTimeF()*6)*.025f,Point(CenterX(),100.0f));
		gBundle_Play->mFont_Commodore64Angled18.SetExtraSpacing(-3);
		gBundle_Play->mFont_Commodore64Angled18Outline.SetExtraSpacing(-3);
		gG.ClipColor(0,0,0,1);
		CENTERGLOWBIG(mLevelName,mWidth/2,100);
		CENTERGLOWBIG(mLevelName,mWidth/2,100);
		gG.ClipColor();
		CENTERGLOWBIG(mLevelName,mWidth/2,100);
		gG.ScaleView();

		float aY=185;
		for (int aCount=0;aCount<3;aCount++)
		{
			String aName=gApp.mHS[mLevelID].GetName(aCount);
			int aSeconds=gApp.mHS[mLevelID].GetScore(aCount);
			if (aSeconds==0) break;
			aSeconds/=50;

			if (aCount==0)
			{
				gG.ClipColor(0,0,0,1);
				CENTERGLOW("BEST TIMES",mWidth/2,160);
				CENTERGLOW("BEST TIMES",mWidth/2,160);
				gG.ClipColor();
				CENTERGLOW("BEST TIMES",mWidth/2,160);
			}

			Color aC=Color(1);
			if (aCount==gApp.mJustGotHS && mLevelID==gApp.mJustGotHSGameType) 
			{
				if ((gApp.AppTime()/10)%2) aC=Color(0,1,0);
			}

			String aTimeString;
			if (aSeconds>3600) aTimeString=Sprintf("%d:%.2d:%.2d",aSeconds/60/60,(aSeconds/60)%60,(aSeconds%60));
			else aTimeString=Sprintf("%d:%.2d",aSeconds/60,(aSeconds%60));

			gG.ClipColor(0,0,0,1);
			CENTERGLOW(aTimeString,mWidth/2,aY);
			CENTERGLOW(aTimeString,mWidth/2,aY);
			gG.ClipColor(aC);
			CENTERGLOW(aTimeString,mWidth/2,aY);

			gG.ClipColor();

			aY+=18;

		}
		
	}

	gG.PopClip();
	gBundle_Play->mPickerComputer.Center(HalfSize());

	if (!mDidSwipe)
	{
		if ((gAppPtr->AppTime()/15)%2)
		gBundle_Play->mSwipeformore.Center(mWidth/2,mHeight-27);
	}

	gG.SetColor(.75f+(gMath.Sin(mPulse)*.2f));
	gBundle_Play->mBack.Center(mBack.Center()+Point(0,mBack.IsDown()*2));

	gG.SetColor(.75f+(gMath.Sin(mPulse+30)*.2f));
	gBundle_Play->mPlay.Center(mPlay.Center());
	gG.SetColor();

	if (mReset.mFeatures&CPU_TOUCH)
	{
		gG.SetColor(.75f+(gMath.Sin(mPulse+90)*.2f));
		gBundle_Play->mResetLevel.Center(mReset.Center());
		gG.SetColor();
	}

}

void PickGame::Notify(void* theData)
{
	Button* aB=(Button*)theData;
	aB->DisableTouch();

	if (theData==&mBack)
	{
		Transition* aT=new Transition(this, new MainMenu);
	}

	if (theData==&mPlay) 
	{
		FinishScroll();
		StartAGame();
	}
	if (theData==&mReset)
	{
		FinishScroll();
		MsgBox* aBox=new MsgBox;
		aBox->GoX("Really reset this level's progress?",gGCenter(),1,MBHOOK(
			{
				if (theResult=="YES")
				{
					//
					// Delete savegame!
					//
					String aWork="sandbox://";aWork+=gTempSaveName;
					DeleteFile(aWork);
				}
				FixLevelName(mPickLevel);
			}
		));

		
	}
}

void PickGame::TouchStart(int x, int y)
{
	mDown=true;
}

void PickGame::TouchMove(int x, int y)
{
	if (mDown)
	{
		Point aVec=gApp.mMessageData_TouchPosition-gApp.mMessageData_TouchStartPosition;
		aVec.mY=0;
		float aLen=aVec.Length();
		if (aLen>5)
		{
			if (aLen<10) aVec.SetLength(10);
			if (aLen>20) aVec.SetLength(20);
			
			mPickScroll=mPickScrollStep=aVec.mX/(float)gBundle_Play->mLevelIcon[(int)mPickLevel].mWidth;
			mDown=false;

			mNextPickLevel=mPickLevel-(int)gMath.Sign(mPickScroll);
			int aMax=gBundle_Play->mLevelIcon.Size();
			if (mNextPickLevel<0) mNextPickLevel+=aMax;
			if (mNextPickLevel>=aMax) mNextPickLevel-=aMax;

			mDidSwipe=true;
			mLevelName="";
			mReset.DisableTouch();
		}
	}
}

void PickGame::TouchEnd(int x, int y)
{
	mDown=false;
}

void PickGame::StartAGame()
{
	gSounds->mSelect.PlayPitched(1.25f);
		{
			gApp.FadeAllMusicExcept(NULL,.5f);

			gApp.mGameOrder=mPickLevel;
			CPU* aCPU=NULL;
			if (mPickLevel==0) {gApp.mGameType=MyApp::GameType_Kid;aCPU=gApp.GoNewGame("data://NOVICELEVEL.kitty");}
			if (mPickLevel==1) {gApp.mGameType=MyApp::GameType_Expert;aCPU=gApp.GoNewGame("data://EXPERTLEVEL.kitty");}
			if (mPickLevel==2) {gApp.mGameType=MyApp::GameType_Bossy;aCPU=gApp.GoNewGame("data://SHORTBOSSY.kitty");}
			if (mPickLevel==3) {gApp.mGameType=MyApp::GameType_Mayhem;aCPU=gApp.GoNewGame("data://ELECTRICMAYHEM.kitty");}
			if (mPickLevel==4) {gApp.mGameType=MyApp::GameType_Master;aCPU=gApp.GoNewGame("data://MASTERLEVEL.kitty");}
			if (mPickLevel==5) {gApp.mGameType=MyApp::GameType_Flash;aCPU=gApp.GoNewGame("data://FLASHLEVEL.kitty");}
			if (mPickLevel==6) {gApp.mGameType=MyApp::GameType_Disarmed;aCPU=gApp.GoNewGame("data://DISARMED.kitty");}
			if (mPickLevel==7) {gApp.mGameType=MyApp::GameType_GirderLand;aCPU=gApp.GoNewGame("data://GIRDERLAND.kitty");}
			if (mPickLevel==8) {gApp.mGameType=MyApp::GameType_LoveNRockets;aCPU=gApp.GoNewGame("data://LOVENROCKETS.kitty");}
			if (mPickLevel==9) {gApp.mGameType=MyApp::GameType_Telefort;aCPU=gApp.GoNewGame("data://TELEFORT.kitty");}
			if (mPickLevel==10) {gApp.mGameType=MyApp::GameType_SoCloseSoFar;aCPU=gApp.GoNewGame("data://SOCLOSE.kitty");}

			if (aCPU) Transition* aT=new Transition(gPickGame,aCPU);
		}
}

void PickGame::FixLevelName(int theLevel)
{
	mIsSaveGame=false;
	mLevelID=0;
	switch (theLevel)
	{
	case 0:mLevelName="Novice\nLevel";mLevelFN="NOVICELEVEL";mLevelID=MyApp::GameType_Kid;break;
	case 1:mLevelName="Expert\nLevel";mLevelFN="EXPERTLEVEL";mLevelID=MyApp::GameType_Expert;break;
	case 2:mLevelName="Short\n& Bossy";mLevelFN="SHORTBOSSY";mLevelID=MyApp::GameType_Bossy;break;
	case 3:mLevelName="Electric\nMayhem";mLevelFN="ELECTRICMAYHEM";mLevelID=MyApp::GameType_Mayhem;break;
	case 4:mLevelName="Master\nLevel";mLevelFN="MASTERLEVEL";mLevelID=MyApp::GameType_Master;break;
	case 5:mLevelName="Classic\nFlash Game";mLevelFN="FLASHLEVEL";mLevelID=MyApp::GameType_Flash;break;
	case 6:mLevelName="Disarmed";mLevelFN="DISARMED";mLevelID=MyApp::GameType_Disarmed;break;
	case 7:mLevelName="Girder\nLand";mLevelFN="GIRDERLAND";mLevelID=MyApp::GameType_GirderLand;break;
	case 8:mLevelName="Love n'\nRockets";mLevelFN="LOVENROCKETS";mLevelID=MyApp::GameType_LoveNRockets;break;
	case 9:mLevelName="Telefort";mLevelFN="TELEFORT";mLevelID=MyApp::GameType_Telefort;break;
	case 10:mLevelName="So Close\nSo Far";mLevelFN="SOCLOSE";mLevelID=MyApp::GameType_SoCloseSoFar;break;
	}
	mLevelName.Uppercase();
	gTempSaveName="RESUME_";
	gTempSaveName+=mLevelFN;
	gTempSaveName+=".sav";

	mReset.DisableTouch();
	String aLookForLevel="sandbox://";aLookForLevel+=gTempSaveName;
	if (DoesFileExist(aLookForLevel)) 
	{
		mReset.EnableTouch();
		mIsSaveGame=true;
	}

	mScores=&gApp.mHS[mLevelID];
}

