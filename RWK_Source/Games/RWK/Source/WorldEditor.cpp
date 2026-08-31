
#include "WorldEditor.h"
#include "SaveGame.h"
#include "MainMenu.h"
#include "Game.h"
#include "Beepbox.h"
//#include "iToolsInterface.h"

WorldEditor *gWorldEditor=NULL;
extern TransDialog* gTransDialog;
int gToolBackupType=0;
int gToolBackup=0;
bool gErasing=false;

int gLayoutRef[]={	
	2,3,4,30,48,31,34,35,36,21,
	15,17,19,52,12,13,14,51,5,6,
	7,8,9,10,11,43,73,26,27,28,66,
	22,23,24,54,57, 72, 37,25,42,38,
	32,	33,29,39,44,40,41,45,46,47,
	49,	50,55,56,65,58,59,60,61,63,
	62,	64,67, 68,69,70,71,74,1,1,
	1,1,1,1,1,1,1,1,1,1,
	// Add Item Additem
};


#ifndef NO_THREADS
void UploadStub(void* theArg);
#endif

bool gWantTutorDialog=false;

void TutorDialog()
{
	if (gApp.mShowTutorDialog) 
	{
		gSounds->mMenu.Play();
		MsgBrowser* aMB=new MsgBrowser;
		IOBuffer aBuffer;
		aBuffer.Load("data://tutor.ml");
		aMB->mData=aBuffer.ToString();
		gApp.AddCPU(aMB);
	}

	gWantTutorDialog=false;
}

WorldEditor::WorldEditor(void)
{
	gWorldEditor=this;
	gErasing=false;
	mName="";
	mTestedOK=false;
	mTestedNoDying=false;
	mFlagBits=0;
	mUploadID=-1;
	mTestWorld=NULL;

	//gApp.mToolVersion=0;
	if (gApp.mToolVersion<1) 
	{
		gLayoutRef[26]=1;	// Gun
		gLayoutRef[35+1]=1; // Phage
		gLayoutRef[62+2]=1;
		gLayoutRef[63+2]=1;
		gLayoutRef[64+2]=1;
		gLayoutRef[65+2]=1;	// Radio
		gLayoutRef[66+2]=1; // Monster Energy
	}

	mIsEditor=true;
	//mCurrentUndo=NULL;


	mZoom=.9f;
	//gApp.mBundle_Play.mFont_Commodore64Angled8.SetExtraSpacing(-2);
	//gApp.mBundle_Play.mFont_Commodore64Angled8Outline.SetExtraSpacing(-2);

	mScrolling=false;
	mPlayMusic=false;
	mPaintID=0;
	mWantNewID=false;
	mMouseIsDown=false;
	mPaintMask=-1;

	mToolbox=NULL;
	mToolType=0;

	mDraggingRobot=false;
	mDraggingKitty=false;
	
	mZoom=1.0f;


	//Load(PointAtSandbox("TEMP64.KIT"));
}

WorldEditor::~WorldEditor(void)
{
	if (mToolbox) mToolbox->Kill();
	if (gWorldEditor==this) gWorldEditor=NULL;
	if (gAppPtr) gApp.mBundle_ETools.Unload();

	Clean();
}

void WorldEditor::Initialize()
{
	mX+=gReserveLeft;mWidth-=gReserveRight+gReserveLeft;

	gApp.mBundle_ETools.Load();

	mThumbPadCenter=Point(73,247);

	float aWidth=67;
	float aXPos=Width()-(aWidth);
	
	mBackButton.Size(aXPos,-1.0f,aWidth,39.0f);
	mBackButton.SetSounds(&gSounds->mClick);
	mBackButton.SetNotify(this);
	*this+=&mBackButton;

	mSettingsButton.Size(aXPos,33,aWidth,46.0f);
	mSettingsButton.SetSounds(&gSounds->mClick);
	mSettingsButton.SetNotify(this);
	*this+=&mSettingsButton;

	mShareButton.Size(aXPos,74.0f,aWidth,39.0f);
	mShareButton.SetSounds(&gSounds->mClick);
	mShareButton.SetNotify(this);
	*this+=&mShareButton;

	mLayoutButton.Size(aXPos,136.0f,aWidth,45.0f);
	mLayoutButton.SetSounds(&gSounds->mClick);
	mLayoutButton.SetNotify(this);
	*this+=&mLayoutButton;

	mPaintButton.Size(aXPos,175.0f,aWidth,48.0f);
	mPaintButton.SetSounds(&gSounds->mClick);
	mPaintButton.SetNotify(this);
	*this+=&mPaintButton;

	mEraseButton.Size(aXPos,217.0f,aWidth,37.0f);
	mEraseButton.SetSounds(&gSounds->mClick);
	mEraseButton.SetNotify(this);
	*this+=&mEraseButton;

	mTrashButton.Size(aXPos,Height()-45.0f,aWidth,45.0f);
	mTrashButton.SetSounds(&gSounds->mClick);
	mTrashButton.SetNotify(this);
	*this+=&mTrashButton;

	mSaveButton.Size(0,271-9999,56,49);
	mTestButton.Size(0,0,51,50);
	mTestButton.SetSounds(&gSounds->mClick);
	mTestButton.SetNotify(this);
	*this+=&mTestButton;

	if (gWantTutorDialog) TutorDialog();
}

String WorldEditor::QuickSave()
{
	if (mName.Len()>0) 
	{
		mLevelname=mName;

		String aDir="sandbox://EXTRALEVELS64\\";
		MakeDirectory(aDir);

		String aSaveFile=Sprintf("sandbox://EXTRALEVELS64\\%s.kitty",mName.c());
		Save(aSaveFile);

		return aSaveFile;
	}
	return "";
}


void WorldEditor::Update()
{
	if (mName.Len()==0) 
	{
		//
		// Force them to name the level!
		//
		//if (!gInfoDialog) new InfoDialog();
	}

#define BORDERX 2
#define BORDERY	2

#ifdef _WINX32
#define STEPLENGTH	40
	Point aVec;

#ifdef _DEBUG
	if (IsKeyPressed(KB_W)) 
	{
		mTestedNoDying=true;
		gSounds->mSelect.Play();
	}
#endif

	if (gWorld==this)
	{
		if (IsKeyDown(KB_UP)) aVec.mY=STEPLENGTH;
		if (IsKeyDown(KB_DOWN)) aVec.mY=-STEPLENGTH;
		if (IsKeyDown(KB_LEFT)) aVec.mX=STEPLENGTH;
		if (IsKeyDown(KB_RIGHT)) aVec.mX=-STEPLENGTH;
		if (IsKeyDown(KB_LEFTSHIFT)) aVec*=2;
	}
	mScrollCenter-=(aVec/mZoom)/5;

	if (mScrollCenter.mX<=-(BORDERX*mGridSize)) mScrollCenter.mX=-BORDERX*mGridSize;
	if (mScrollCenter.mY<=-(BORDERY*mGridSize)) mScrollCenter.mY=-BORDERY*mGridSize;
	if (mScrollCenter.mX>=(mGridWidth+BORDERX)*mGridSize) mScrollCenter.mX=(float)(mGridWidth+BORDERX)*mGridSize;
	if (mScrollCenter.mY>=(mGridHeight+BORDERY)*mGridSize) mScrollCenter.mY=(float)(mGridHeight+BORDERY)*mGridSize;

	if (IsKeyPressed(KB_G))
	{
		Grid *aG=GetGridPtr(mMouseGrid.mX,mMouseGrid.mY);
		if (aG)
		{
			if (mToolType==1) mTool=aG->mLayout;
			if (mToolType==2) {

				mTool=PaintToTool(aG->mPaint);
				mPaintID++;
				if (mPaintID>500) mPaintID=0;

			}
		}
	}

#else
	if (mScrolling)
	{
		Point aVec=mThumbPadCenter-gApp.mMessageData_TouchPosition;
		mScrollCenter-=(aVec/mZoom)/5;


		if (mScrollCenter.mX<=-(BORDERX*mGridSize)) mScrollCenter.mX=-BORDERX*mGridSize;
		if (mScrollCenter.mY<=-(BORDERY*mGridSize)) mScrollCenter.mY=-BORDERY*mGridSize;
		if (mScrollCenter.mX>=(mGridWidth+BORDERX)*mGridSize) mScrollCenter.mX=(float)(mGridWidth+BORDERX)*mGridSize;
		if (mScrollCenter.mY>=(mGridHeight+BORDERY)*mGridSize) mScrollCenter.mY=(float)(mGridHeight+BORDERY)*mGridSize;
	}
#endif
}

void WorldEditor::Clean()
{
	mPlayerList.Free();
	//FreeList(Undo,mUndoStack);

	mRobot=NULL;
	mKitty=NULL;
	delete [] mGrid;
	mGrid=NULL;
	mPaintID=0;

}

void WorldEditor::MakeBlank()
{
	mUploadID=-1;
	mTags=0;

	InitializeGrid(10,7);

	mRobot=new Robot();
	mPlayerList+=mRobot;
	mRobot->mPos=GridToWorld((mGridWidth/2)-1,mGridHeight-2);
	mRobot->mPos.mY-=2;
	mScrollCenter=mRobot->mPos;

	mKitty=new Kitty();
	mPlayerList+=mKitty;
	mKitty->mPos=GridToWorld((mGridWidth/2)+1,mGridHeight-2);
	mKitty->mPos.mY-=4;
}

void WorldEditor::Draw()
{
	DrawBackground();

	mRobot->mRed=0.0f;
	if (gWorld->Collide(Rect(mRobot->mPos.mX+(mRobot->mCollide.mX),mRobot->mPos.mY+(mRobot->mCollide.mY),mRobot->mCollide.mWidth,mRobot->mCollide.mHeight-1))) mRobot->mRed=1.0f;

	gG.PushTranslate();
	Point aRealCenter=-mScrollCenter+mScrollOffset;
//	gG.Translate(aRealCenter+(Point(gG.WidthF()/2,320.0f/2)));
	gG.Translate(aRealCenter+(Point(mWidth/2,320.0f/2)));
	DrawWorld();

	if (mMouseIsDown)
	{
		if (mZoom!=1.0f) gG.ScaleView(mZoom);

		Point aWorld=GridToWorld(mMouseGrid);
		gG.SetColor(.5f);
		gG.FillRect(aWorld.mX-2,aWorld.mY-2,4,4);
		gG.FillRect(aWorld.mX-2,aWorld.mY-500,4,500-30);
		gG.FillRect(aWorld.mX-2,aWorld.mY+30,4,500-30);
		gG.FillRect(aWorld.mX-500,aWorld.mY-2,500-30,4);
		gG.FillRect(aWorld.mX+30,aWorld.mY-2,500-30,4);
		gG.SetColor();

		if (mZoom!=1.0f) gG.ScaleView(1.0f);

	}
	gG.PopTranslate();
	DrawUI();
}

void WorldEditor::DrawOverlay()
{
	gG.PushTranslate();
	gG.Translate();
	gG.Translate(mX,mY);
	if (gWorld!=gWorldEditor) DrawUI();
	gG.PopTranslate();
}

void WorldEditor::DrawUI()
{
	if (gWorldEditor==gWorld)
	{
		//
		// Thumbpad
		//
		gBundle_ETools->mThumbpad.Center(mThumbPadCenter);
		Point aVec;
		if (mScrolling) 
		{
			aVec=Point(mThumbPadCenter)-gApp.mMessageData_TouchPosition;
			if (aVec.Length()>48) aVec.SetLength(48);
			aVec*=-1;
		}
		aVec+=Point(mThumbPadCenter);
		gBundle_ETools->mThumb.Center(aVec);

		//
		// Icons
		//
		String aMessage="";

		gG.SetColor(0,0,0,.5f);
		gG.FillRect(mLayoutButton.mX-5,0,100,320);
		gG.SetColor();
		gBundle_ETools->mBack.Center(mBackButton.Center()+Point((float)(mBackButton.IsDown()*3)));
		gBundle_ETools->mSettings.Center(mSettingsButton.Center()+Point((float)(mSettingsButton.IsDown()*3)));
		gBundle_ETools->mShare.Center(mShareButton.Center()+Point((float)(mShareButton.IsDown()*3)));
		if (mToolType!=1 || GetRealTool()==0) gBundle_ETools->mLayout.Center(mLayoutButton.Center()+Point((float)(mLayoutButton.IsDown()*3)));
		if (mToolType!=2) gBundle_ETools->mPaint.Center(mPaintButton.Center()+Point((float)(mPaintButton.IsDown()*3)));
		gBundle_ETools->mTrash.Center(mTrashButton.Center()+Point((float)(mTrashButton.IsDown()*3)));
		//gBundle_ETools->mSave.Center(mSaveButton.Center()+Point(mSaveButton.IsDown()*3));
	
//		if (mTestWorld) gBundle_ETools->mEdit.Center(mTestButton.Center()+Point(mTestButton.IsDown()*3));
		gBundle_ETools->mTest.Center(mTestButton.Center()+Point((float)(mTestButton.IsDown()*3)));

		if (gErasing) 
		{
			gBundle_ETools->mEraser.DrawScaled(mEraseButton.Center()+Point((float)(mEraseButton.IsDown()*3)),1.0f+(gMath.Sin(gApp.AppTimeF()*10)*.2f));
			aMessage="ERASING...";
		}
		else gBundle_ETools->mEraser.Center(mEraseButton.Center()+Point((float)(mEraseButton.IsDown()*3)));

		if (aMessage.Len())
		{
			if ((gApp.AppTime()/10)%2) {CENTERGLOW(aMessage,mWidth/2,18);}
			else 
			{
				gG.ClipColor(.25f);
				CENTERGLOW(aMessage,mWidth/2,18);
				gG.ClipColor();
			}
		}

		if (mToolType>0)
		{
			char aFlicker=(gApp.AppTime()/25)%2;
			if (mToolType==1 && GetRealTool()!=0)
			{
				Point aToolPos=mLayoutButton.Center();//+Point(-5,0);
				
				if (aFlicker) gG.SetColor(.5f);
				else  {gG.SetColor(0,0,0,1);gBundle_ETools->mSelected_Layout.DrawScaled(aToolPos+Point(0,-5),.8f);gG.SetColor();}
				gBundle_ETools->mSelected_Layout.DrawScaled(aToolPos+Point(0,-5),.8f);
				gG.SetColor();
				gBundle_Tiles->mBlock[GetRealTool()].DrawScaled(aToolPos,.75f);
			}
			if (mToolType==2)
			{
				Point aToolPos=mPaintButton.Center()+Point(-5,0);;
				
				if (aFlicker) gG.SetColor(.5f);
				else {gG.SetColor(0,0,0,1);gBundle_ETools->mSelected_Paint.DrawScaled(aToolPos+Point(0,-5),.8f);gG.SetColor();}
				gBundle_ETools->mSelected_Paint.DrawScaled(aToolPos+Point(0,-5),.8f);
				gG.SetColor();
				gBundle_Tiles->mTile_Paint[GetPaintRef()]->DrawScaled(aToolPos,.75f);
			}
		}

	}
	else
	{
		if (mTestWorld) gBundle_ETools->mEdit.Center(mTestButton.Center()+Point((float)(mTestButton.IsDown()*3)));
		else gBundle_ETools->mTest.Center(mTestButton.Center()+Point((float)(mTestButton.IsDown()*3)));

		//gBundle_ETools->mTest.Center(mTestButton.Center()+Point(mTestButton.IsDown()*3));
	}

}

int gPlotTouchID=-1;
void WorldEditor::TouchStart(int x, int y)
{
	static int aTickDoubleClick=-1;

//#ifndef WIN32_LEAN_AND_MEAN
	if (gMath.Distance(Point((float)x,(float)y),mThumbPadCenter)<=68) 
	{
		int aTick=gAppPtr->AppTime();
		if (aTick-aTickDoubleClick<=25) 
		{
			aTickDoubleClick=aTick;
			MouseDoubleClick(x,y,-1);
		}
		else 
		{
			aTickDoubleClick=aTick;
			mScrolling=true;
		}
	}
	else
//#endif
	{
		mDraggingRobot=false;
		mDraggingKitty=false;
		SetMouseGrid(x,y);

		gPlotTouchID=gAppPtr->mMessageData_TouchID;
		//
		// See if we're over Robot or Kitty...
		//
		float aWorkX=(float)x;
		float aWorkY=(float)y;

//		aWorkX-=gG.WidthF()/2;
		aWorkX-=mWidth/2;
		aWorkY-=320/2;
		aWorkX/=mZoom;
		aWorkY/=mZoom;
		aWorkX+=mScrollCenter.mX;
		aWorkY+=mScrollCenter.mY;

		if (gMath.Distance(aWorkX,aWorkY,mRobot->mPos.mX,mRobot->mPos.mY)<=30) mDraggingRobot=true;
		if (gMath.Distance(aWorkX,aWorkY,mKitty->mPos.mX,mKitty->mPos.mY)<=30) mDraggingKitty=true;

		if (mDraggingRobot)
		{
			mDragOffset=mRobot->mPos-Point(aWorkX,aWorkY);
			TouchMove(x,y);
		}
		else if (mDraggingKitty)
		{
			mDragOffset=mKitty->mPos-Point(aWorkX,aWorkY);
			TouchMove(x,y);
		}
		else
		{
			switch (mToolType)
			{
			case 1:
				{
					//
					// Draw layout mode
					//
					if (aWorkX<0) aWorkX-=mGridSize;
					if (aWorkY<0) aWorkY-=mGridSize;

					aWorkX/=mGridSize;
					aWorkY/=mGridSize;

					int aHold=mTool;

					if (mTool==14 || mTool==16 || mTool==18 || mTool==51)
					{
						//
						// Plotting a doorway instead... try to do this smart...
						// No block below or above?  Put door right here (bottom)
						// Block below?  Put bottom of door here.
						// Block above?  Put door two down.
						//


						IPoint aDoorBottom;
						if (GetGrid((int)aWorkX,(int)aWorkY+1)==1) aDoorBottom=IPoint(aWorkX,aWorkY);
						else if (GetGrid((int)aWorkX,(int)aWorkY+2)==1) aDoorBottom=IPoint(aWorkX,aWorkY+1);
						else if (GetGrid((int)aWorkX,(int)aWorkY-1)==1) aDoorBottom=IPoint(aWorkX,aWorkY+1);
						else aDoorBottom=IPoint(aWorkX,aWorkY);


						int aT=mTool;
						//if (mTool==15) mTool=16;
						//else if (mTool==16) mTool=18;

						if (GetGrid(aDoorBottom.mX,aDoorBottom.mY)==0) {mTool++;Plot(aDoorBottom.mX,aDoorBottom.mY);mTool--;}
						if (GetGrid(aDoorBottom.mX,aDoorBottom.mY-1)==0) Plot(aDoorBottom.mX,aDoorBottom.mY-1);

					}
					else 
					{
						//if (mTool>=17) mTool+=3;
						Plot((int)aWorkX,(int)aWorkY);

						if (mTool<=2) mMouseIsDown=true;
						if (mTool==VELCRO_BLOCK || mTool==COSMIC_BLOCK) mMouseIsDown=true;
						if (mTool==VELCRO_BLOCK) 
						{
							TouchMove(x,y);
						}

					}
					mTool=aHold;

					break;
				}
			case 2:
				{
					if (!mMouseIsDown)
					{
						if (mPaintMask!=-1) 
						{
							mPaintID++;
							if (mPaintID>500) mPaintID=0;
						}
						aWorkX/=mGridSize;
						aWorkY/=mGridSize;
						mPaintMask=GetGrid((int)aWorkX,(int)aWorkY);

						if (mPaintMask==VELCRO_BLOCK) break;

						if (mPaintMask!=COMPUTER_DOOR && mPaintMask!=BOSS_BLOCK && mPaintMask!=VIRUS_DOOR && mPaintMask!=BITCOIN_DOOR) mPaintMask=-1;	// ADD MASK
						else 
						{
							mPaintID++;
							if (mPaintID>500) mPaintID=0;
						}
					}

					mMouseIsDown=true;
					Paint(x,y);
				}
			}
		}
	}
}

void WorldEditor::Paint(int x, int y)
{
	float aWorkX=(float)x;
	float aWorkY=(float)y;

	aWorkX-=mWidth/2;
	aWorkY-=320/2;
	aWorkX/=mZoom;
	aWorkY/=mZoom;
	aWorkX+=mScrollCenter.mX;
	aWorkY+=mScrollCenter.mY;

	if (aWorkX<0) aWorkX-=mGridSize;
	if (aWorkY<0) aWorkY-=mGridSize;

	aWorkX/=mGridSize;
	aWorkY/=mGridSize;
	PlotPaint((int)aWorkX,(int)aWorkY);
}


int WorldEditor::GetRealTool()
{
	int aT=mTool;
/*
	if (mTool==15) aT=16;
	else if (mTool==16) aT=18;
	else if (mTool>=17) aT+=3;
*/
	return aT;
}

void WorldEditor::SetMouseGrid(int theX, int theY)
{
	float aWorkX=(float)theX;
	float aWorkY=(float)theY;

	aWorkX-=mWidth/2;
	aWorkY-=320/2;
	aWorkX/=mZoom;
	aWorkY/=mZoom;
	aWorkX+=mScrollCenter.mX;
	aWorkY+=mScrollCenter.mY;

	if (aWorkX<0) aWorkX-=mGridSize;
	if (aWorkY<0) aWorkY-=mGridSize;

	aWorkX/=mGridSize;
	aWorkY/=mGridSize;

	mMouseGrid.mX=(int)aWorkX;
	mMouseGrid.mY=(int)aWorkY;
}

void WorldEditor::TouchMove(int x, int y)
{
	if (gAppPtr->mMessageData_TouchID!=gPlotTouchID) return;

	SetMouseGrid(x,y);


	float aWorkX=(float)x;
	float aWorkY=(float)y;

	aWorkX-=mWidth/2;
	aWorkY-=320/2;
	aWorkX/=mZoom;
	aWorkY/=mZoom;
	aWorkX+=mScrollCenter.mX;
	aWorkY+=mScrollCenter.mY;


	if (mDraggingRobot) {mRobot->mPos=Point(aWorkX,aWorkY)+mDragOffset;mTestedOK=false;mTestedNoDying=false;}
	else if (mDraggingKitty) {mKitty->mPos=Point(aWorkX,aWorkY)+mDragOffset;mTestedOK=false;mTestedNoDying=false;}
	else if (mToolType==1)
	{
		if (mMouseIsDown)
		if (mTool==1 || mTool==2 || mTool==0 || mTool==VELCRO_BLOCK || mTool==COSMIC_BLOCK)
		{
			Plot(mMouseGrid.mX,mMouseGrid.mY);
			if (mTool==VELCRO_BLOCK) Paint(x,y);
		}
	}
	else if (mToolType==2)
	{
		if (mMouseIsDown) Paint(x,y);
	}
}

void WorldEditor::TouchEnd(int x, int y)
{
	if (gPlotTouchID==gAppPtr->mMessageData_TouchID) gPlotTouchID=-1;

	mMouseIsDown=false;

	if (mDraggingRobot)
	{
		IPoint aGPos=WorldToGrid(mRobot->mPos);
		mRobot->mPos=GridToWorld(aGPos);
	}
	if (mDraggingKitty)
	{
		IPoint aGPos=WorldToGrid(mKitty->mPos);
		mKitty->mPos=GridToWorld(aGPos)+Point(0,-5);
	}


	mDraggingRobot=false;
	mDraggingKitty=false;
	mScrolling=false;
}

void WorldEditor::MouseDoubleClick(int x, int y, int theButton)
{
	if (gMath.Distance(Point(x,y),mThumbPadCenter)<=68)
	{
		mScrollCenter=mRobot->mPos;
	}
}

void WorldEditor::Notify(void *theData)
{
	if (mToolbox) mToolbox->Kill();

	if (theData==&mBackButton) 
	{
		QuickSave();
		Kill();
		if (gMakermall) gMakermall->ReturnFromEditor(mName);
	}

	if (theData==&gApp.mGlobalNotify)
	{
		if (gApp.mGlobalNotify=="quit:yes")
		{
			//
			// Need to go back to Kitty Connect...
			//

			gApp.FadeInMusic(gApp.mMusic_Title);
			gApp.GoKittyConnect();
		}
		if (gApp.mGlobalNotify=="new:yes")
		{
			Clean();
			MakeBlank();
		}
		if (gApp.mGlobalNotify.strnicmp("BROWSE::",8))
		{
			String aLoadName=Sprintf("sandbox://EXTRALEVELS64\\%s.kitty",gApp.mGlobalNotify.GetSegment(8,9999).c());
			Load(aLoadName);
		}
	}

	if (theData==&mTestButton)
	{
		if (gWorldEditor==gWorld)
		{
			QuickSave();
			Save("sandbox://TESTDRIVE64.KIT");

			gTempSaveName="";

			mTestWorld=new World();
			mTestWorld->LoadLevel("sandbox://TESTDRIVE64.KIT",false);
			mTestWorld->mRobot->FixPosition();
			mTestWorld->mScrollCenter=mTestWorld->mRobot->mPos;
			mTestWorld->mTesting=true;
			mTestWorld->HookTo(&mTestWorld);
			*this+=mTestWorld;

/*
			mTestButton.mX=-100;
			mTestButton.mY=-100;
*/
			*this-=&mTestButton;
			*this+=&mTestButton;
		}
		else
		{
			gWorld->Kill();
			mTestWorld=NULL;

			//mTestButton.Size(61,278,35,35);
			gWorld=this;

			if (!(mFlagBits&0x01))
			{
				if (mTestedOK && !mTestedNoDying)
				{
					mFlagBits|=0x01;
					MsgBox *aBox=new MsgBox();
					aBox->GoNoThrottle("REMEMBER TO TEST AND WIN YOUR LEVEL WITHOUT GETTING KILLED TO EARN THE 1UP LEVEL FLAIR!",gGCenter(),0);
					return;
				}
			}
		}
	}
	if (theData==&mSettingsButton)
	{
		OpenSettings();
	}
	if (theData==&mLayoutButton)
	{
		gErasing=false;
		gApp.FadeInMusic(NULL,.5f);
		if (mToolbox && mToolbox->mCaption=="Layout Tools") mToolbox->Kill();
		else {
			Toolbox_Layout *aT=new Toolbox_Layout();
			//aT->Size(7,7,377,78+46+46+46+46);
			aT->Size(-2-8,0,8+377+46+46,78+46+46+46+46+46);
			aT->mX=mWidth-90-aT->mWidth;
			if (aT->mX<-2) aT->mX=-2;
			aT->mCaption="Layout Tools";
			mToolbox=aT;
			*this+=mToolbox;
		}
	}
	if (theData==&mPaintButton)
	{
		gErasing=false;
		gApp.FadeInMusic(NULL,.5f);
		if (mToolbox && mToolbox->mCaption=="Paint Tools") mToolbox->Kill();
		else {
			Toolbox_Paint *aT=new Toolbox_Paint();
			aT->Size(7,7,377,78+25+45);
			aT->mX=mWidth-90-aT->mWidth;
			aT->mCaption="Paint Tools";
			mToolbox=aT;
			*this+=mToolbox;
		}
	}
/*
	if(theData==&mSaveButton)
	{
		if (mToolbox) mToolbox->Kill();
		else {
			Toolbox_File *aT=new Toolbox_File();
			//aT->Size(7,7+175,198+46,78);
			//aT->mCaption="SYSTEM OPTIONS";
			aT->Size(7,7+175,198+46+46,78);
			aT->mCaption="Level Options";

			mToolbox=aT;
			*this+=mToolbox;
		}
	}
*/
	if (theData==&mEraseButton)
	{
		if (!gErasing)
		{
			gToolBackup=mTool;
			gToolBackupType=mToolType;
			gErasing=true;
			gApp.FadeInMusic(gApp.mMusic_Erase,.5f);

			mTool=0;
			mToolType=1;
		}
		else
		{
			mTool=gToolBackup;
			mToolType=gToolBackupType;
			gErasing=false;
			gApp.FadeInMusic(NULL,.5f);
		}
	}
	if (theData==&mShareButton)
	{
		if (mUploadID>=0)
		{
			PickMenu* aPM=new PickMenu;
			aPM->AddButton("UPDATE LEVEL");
			aPM->AddButton("UN-SHARE LEVEL");
			aPM->GoX(mShareButton.Center(),true,0,MBHOOK(
				{
					if (theResult=="UPDATE LEVEL") gWorldEditor->FinishSharing();
					else if (theResult=="UN-SHARE LEVEL")
					{
						MsgBox *aBox=new MsgBox();
						aBox->GoX("UN-SHARING A LEVEL WILL REMOVE YOUR LEVEL FROM MAKERMALL AND ERASE ALL STATS AND COMMENTS!\n\nREALLY DO THIS?",gGCenter(),1,MBHOOK(
							{
								if (theResult=="YES") 
								{
									TransDialog* aTD=new TransDialog;
									aTD->SetStatus("UN-SHARING LEVEL...");
									//aTD->mCanCancel=true;
									//aTD->mCancelText="TOUCH HERE TO CANCEL";
									gAppPtr->AddCPU(aTD);

									TransferPacket* aTP=new TransferPacket;
									aTP->mName=gWorldEditor->mName;
									aTP->mDialog=aTD;
									aTP->mUploadID=gWorldEditor->mUploadID;

									gMakermall->UnshareLevel(aTP);
									//Thread(&UnshareStub,aTP);
								}
							}));
						return;
					}
				}
			));
			return;
		}
		FinishSharing();

		return;
	}
	if (theData==&mTrashButton)
	{
		MsgBox *aBox=new MsgBox();
		aBox->GoX("THIS LEVEL WILL BE REMOVED FROM YOUR DEVICE AND FROM SHARING!\n\nREALLY TRASH THIS LEVEL?",gGCenter(),1,MBHOOK(
			{
				if (theResult=="YES") 
				{
					MsgBox *aBox=new MsgBox();
					aBox->GoX("LAST CHANCE!  THIS LEVEL WILL BE DELETED COMPLETELY!\n\nREALLY TRASH THIS LEVEL?",gGCenter(),1,MBHOOK(
						{
							if (theResult=="YES")
							{
								//
								// Delete it!
								//
								gWorldEditor->DeleteLevel();
								gWorldEditor->Kill();
								if (gMakermall) gMakermall->ReturnFromEditor(mName);
							}
						}
					));
				}
			}
		));
	}

}

void WorldEditor::FinishSharing()
{
	if (!mTestedOK)
	{
		MsgBox *aBox=new MsgBox();
		aBox->GoX("YOU MUST WIN THIS LEVEL BEFORE YOU CAN SHARE IT!\n\nPLAY IT NOW?",gGCenter(),1,MBHOOK(
			{
				if (theResult=="YES" && gWorldEditor) gWorldEditor->Notify(&mTestButton);
			}
		));
	}
	else if (mTags==0)
	{
		MsgBox *aBox=new MsgBox();
		aBox->GoX("YOU MUST TAG THIS LEVEL BEFORE YOU CAN SHARE IT!\n\nDO THAT NOW?",gGCenter(),1,MBHOOK(
			{
				if (theResult=="YES" && gWorldEditor) gWorldEditor->Notify(&mSettingsButton);
			}
		));
	}
	else if (!IsLoginValid())
	{
		MsgBox *aBox=new MsgBox();
		aBox->GoX("YOU MUST SIGN IN OR CREATE AN ACCOUNT TO SHARE LEVELS!\n\nDO THAT NOW?",gGCenter(),1,MBHOOK(
			{
				if (theResult=="YES") 
				{
					MMBrowser* aM=new MMBrowser;
					Rect aRect=Rect(0,0,mWidth,mHeight);
					aM->Size(aRect);
					aM->CenterAt(gG.Center()+Point(0,-5));
					aM->mMLBox.SetLinkNotify(aM);
					aM->mTitle="SIGN IN";
					aM->mMLBox.Define("#resume_to","share");

					IOBuffer aBuffer;
					aBuffer.Load("data://signin.ml");
					aM->mData=aBuffer.ToString();

					aM->Initialize();
					aM->mFirstUpdate=false;

					Closer* aC=new Closer;
					aC->mCloseWhom=aM;
					gAppPtr->AddCPU(aC);
					gAppPtr->AddCPU(aM);

				}

			}
		));
	}
	else
	{
		String aFN=QuickSave();
		TransDialog* aTD=new TransDialog;
		aTD->SetStatus("PREPARING LEVEL FOR SHARING...");
		aTD->mCanCancel=true;
		aTD->mCancelText="TOUCH HERE TO CANCEL";
		aTD->mAddQueryToPending=true;
		gAppPtr->AddCPU(aTD);

		TransferPacket* aTP=new TransferPacket;
		aTP->mName=mName;
		aTP->mFN=aFN;
		aTP->mDialog=aTD;
		aTP->mTags=mTags;

		if (mTestedNoDying) aTP->mTags|=NOKILLBIT;
		//Thread(&UploadStub,aTP);

		RunAfterNextDraw(HOOKLOCAL({gMakermall->UploadLevel(aTP);}));
	}
}

void WorldEditor::DeleteLevel()
{
	if (mUploadID!=-1) RComm::LevelSharing_DeleteLevel(gAppPtr->mUserName,gAppPtr->mUserPassword,mUploadID);

	String aFN=Sprintf("sandbox://EXTRALEVELS64\\%s.kitty",mName.c());
	DeleteFile(aFN);

	if (gMakermall) gMakermall->RemoveFromRecentLevels(mName);
}

void WorldEditor::Save(String theFilename)
{
	SaveGame aSave;
	aSave.mVersion=SAVEGAME_VERSION;
	aSave.Saving(theFilename);

	//
	// Editor info chunk
	//
	aSave.StartChunk();
	aSave.Sync(&mUploadID);
	aSave.Sync(&mLevelname);
	aSave.Sync(&mTags);
	aSave.Sync(&mPaintID);
	aSave.Sync(&mTestedOK);
	aSave.Sync(&mTestedNoDying);
	aSave.Sync(&mFlagBits);

	aSave.EndChunk();

	World::Sync(aSave);

	aSave.Commit();
}

void WorldEditor::Load(String theFilename)
{
	Clean();

	SaveGame aSave;
	aSave.mVersion=SAVEGAME_VERSION;
	if (aSave.Loading(theFilename))
	{
		//
		// Metadata chunk
		//
		aSave.StartChunk();
		aSave.Sync(&mUploadID);
		aSave.Sync(&mLevelname);
		aSave.Sync(&mTags);
		aSave.Sync(&mPaintID);
		aSave.Sync(&mTestedOK);
		aSave.Sync(&mTestedNoDying);
		aSave.Sync(&mFlagBits);

		aSave.EndChunk();

		World::Sync(aSave);
	}
	else 
	{
		MakeBlank();
	}
	mZoom=.9f;

	if (mName.Len()==0) mName=mLevelname;
}

#ifdef _WIN32
void WorldEditor::LoadOldVersion(String theFilename)
{
	Clean();

	SaveGame aSave;
	aSave.mVersion=0x0001;
	if (aSave.Loading(theFilename))
	{
		//
		// Metadata chunk
		//
		aSave.StartChunk();
		aSave.Sync(&mLevelname);
		aSave.EndChunk();

		World::Sync(aSave);
		//
		// Editor tool chunk...
		//
		aSave.StartChunk();
		aSave.Sync(&mPaintID);
		aSave.Sync(&mTestedOK);
		int aDifficulty;
		aSave.Sync(&aDifficulty);
		aSave.EndChunk();

		mTestedOK=false;
		mPaintID=0;
		mUploadID=-1;
		mTestedNoDying=false;
		mTags=0;
		mFlagBits=0;

		_DLOG("Load ok...[%s] [%d]",mLevelname.c(),mUploadID);
	}
	else 
	{
		MakeBlank();
		//		InfoDialog *aI=new InfoDialog();
	}
	mZoom=.9f;

	if (mName.Len()==0) mName=mLevelname;
}
#endif

Toolbox::Toolbox()
{
	gWorldEditor->mToolType=0;
	mToolWidth=46;
	//mRim.Load(&gBundle_Play->mSmallrim);
}

Toolbox::~Toolbox()
{
	if (gWorldEditor) if (gWorldEditor->mToolbox==this) gWorldEditor->mToolbox=NULL;
}

void Toolbox::Update()
{
}

void Toolbox::Draw()
{
	gG.Clip();
	gG.Translate(-mX,-mY);
	gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(mX-10,mY-10,mWidth+40,mHeight+30,true);
	gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(mX-10,mY-10,mWidth+40,mHeight+30,true);
	gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(mX-10,mY-10,mWidth+40,mHeight+30,true);
	//mRim.Draw(mX-10,mY-10,mWidth+40,mHeight+30);	// FIXRIM (&gBundle_Play->mSmallrim)
	CENTERGLOW(mCaption,Center().mX,mY+28);
	gG.Translate(mX,mY);
	gG.Translate(10,18+10);
	DrawTools();
	gG.Translate();

}

#define ITEMSCALE .92f

void Toolbox::TouchStart(int x, int y)
{
	x-=10;
	y-=(18+10);
	x+=10;
	y+=10;
	x-=(int)(mToolWidth/2);
	y-=(int)mToolWidth/2;
	x/=(int)(mToolWidth*ITEMSCALE);
	y/=(int)(46.0f*ITEMSCALE);

	//gBundle_Tiles->mBlock[gLayoutRef[aLCount++]-1].DrawScaled(4+(mToolWidth/2)+(float)(mToolWidth*aSpanX*ITEMSCALE),10+(mToolWidth/2)+(float)(46*aSpanY)*ITEMSCALE,ITEMSCALE);

//	if (x>=0 && y>=0) PickTool(x+(y*((int)(mWidth/mToolWidth*ITEMSCALE))));
	if (x>=0 && y>=0) PickTool(x+(y*12));
}

void Toolbox_Layout::DrawTools()
{
	mToolWidth=42;

	int aLCount=0;
	for (int aSpanY=0;aSpanY<6;aSpanY++)
	{
		for (int aSpanX=0;aSpanX<12;aSpanX++)
		{
			//gBundle_Tiles->mBlock[gLayoutRef[aLCount++]-1].Draw(6+(float)(mToolWidth*aSpanX),6+(float)(46*aSpanY));
			gBundle_Tiles->mBlock[gLayoutRef[aLCount++]-1].DrawScaled(4+(mToolWidth/2)+(float)(mToolWidth*aSpanX*ITEMSCALE),10+(mToolWidth/2)+(float)(46*aSpanY)*ITEMSCALE,ITEMSCALE);
		}
	}
}

void Toolbox_Layout::PickTool(int theNumber)
{
	gWorldEditor->mToolType=1;
	gWorldEditor->mTool=gLayoutRef[theNumber]-1;

	Kill();
}

void WorldEditor::Plot(int x, int y)
{
	if (x<0 || y<0 || x>=mGridWidth || y>=mGridHeight)
	{
		//
		// Need to dynamically resize our work area!
		// And maladjust x and y to reflect the changes!
		//
		IPoint aUpperLeft(_min(x,0),_min(y,0));
		IPoint aLowerRight(_max(x+1,mGridWidth),_max(y+1,mGridHeight));
		EnlargeWorld(aUpperLeft.mX,aUpperLeft.mY,aLowerRight.mX-aUpperLeft.mX,aLowerRight.mY-aUpperLeft.mY);

		x=_max(0,x);
		y=_max(0,y);
	}

	SetGrid(x,y,mTool);
	mTestedOK=false;mTestedNoDying=false;
	Grid *aG=GetGridPtr(x,y);
	if (aG) 
	{
		aG->mPaint=0;
		aG->mPaintID=0;
	}

	if (mTool==RADIO_BLOCK)
	{
		//
		// Open up an interface to get radio text...
		//
		MMBrowser* aM=new MMBrowser;
		Rect aRect=Rect(0,0,400,235);
		aM->Size(aRect);
		aM->CenterAt(gG.Center()+Point(0,-5));
		if (gAppPtr->IsSmallScreen()) aM->mY-=45;
		aM->mMLBox.SetLinkNotify(aM);
		aM->mTitle="RADIO";
		String aDefault=GetRadioMessage(IPoint(x,y));
		aM->mData=Sprintf(
			"<setup padtop=25 padleft=25 padright=25 padbottom=25><BKGCOLOR #004770>"
			"<CENTER>ENTER THE RADIO MESSAGE:<BR><MOVECURSOR 0,+8>"
			"<TEXTBOX id=RADIOMESSAGE default=\"%s\"><BR><BR>"
			"<link cmd=SetRadioMessage:%d,%d><IMG OK_Button></link><sp 5><link cmd=Cancel><IMG Cancel_Button></link>",
			aDefault.c(),x,y
			);
		aM->Initialize();
		aM->mFirstUpdate=false;

		Closer* aC=new Closer;
		aC->mCloseWhom=aM;
		//gAppPtr->AddCPU(aC);
		//gAppPtr->AddCPU(aM);
		AddCPU(aC);
		AddCPU(aM);

		gApp.SetOnscreenKeyboardExtraSpace(50);				// Extra space for onscreen keyboard (so you can make sure extra buttons are visible)

		Smart(MLTextBox) aTB=aM->mMLBox.GetObjectByID("RADIOMESSAGE");
		aTB->mTextBox.SetBackgroundColor(Color(1));
		aTB->mTextBox.SetTextColor(Color(0,0,0,1));
		aTB->mTextBox.SetTextOffset(3,-2);
		aTB->mTextBox.SetCursorOffset(Point(0,2));
		aTB->mTextBox.SetCursorWidth(4);
		aTB->mTextBox.FocusKeyboard();
		aTB->mTextBox.SetMaxCharCount(40);
		//aTB->mTextBox.ForbidCharacters("/\\?%%*:|\"<>.[]");
	}
}

void WorldEditor::EnlargeWorld(int newX, int newY, int newWidth, int newHeight)
{
	Grid *aOldGrid=mGrid;
	int aOldWidth=mGridWidth;
	int aOldHeight=mGridHeight;

	InitializeGrid(newWidth,newHeight);
	MemToGrid(aOldGrid,aOldWidth,Rect(0,0,(float)aOldWidth,(float)aOldHeight),Point(-(newX),-(newY)));
	delete [] aOldGrid;

	Point aMove=-Point(newX*mGridSize,newY*mGridSize);
	mRobot->mPos+=aMove;
	mKitty->mPos+=aMove;
	mScrollCenter+=aMove;

	EnumSmartList(RadioText,aRT,mRadioTextList) aRT->mPos-=IPoint(newX,newY);
}

void WorldEditor::MemToGrid(Grid *theSource, int theSourceStride, Rect theSourceRect, Point theDestUpperLeft)
{
	int aDestX=(int)theDestUpperLeft.mX;
	int aDestY=(int)theDestUpperLeft.mY;
	int aSrcX=(int)theSourceRect.mX;
	int aSrcY=(int)theSourceRect.mY;

	for (int aSpanX=0;aSpanX<theSourceRect.mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<theSourceRect.mHeight;aSpanY++)
		{
			int aSourcePos=(aSrcX+aSpanX)+((aSrcY+aSpanY)*theSourceStride);
			int aDestPos=(aDestX+aSpanX)+((aDestY+aSpanY)*mGridWidth);
			mGrid[aDestPos]=theSource[aSourcePos];
		}
	}
}

void Toolbox_Paint::DrawTools()
{
	gBundle_ETools->mAutofix.Draw(6,6+48+46);
	if (gApp.mAutoFix) gBundle_ETools->mAutofix_Check.Draw(6,6+48+46);

	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(0)+44]->Draw(6,6);
	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(1)+44]->Draw(6+46,6);
	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(2)+44]->Draw(6+46+46,6);
	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(3)+44]->Draw(6+46+46+46,6);
	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(4)+44]->Draw(6+46+46+46+46,6);
	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(5)+44]->Draw(6+46+46+46+46+46,6);
	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(6)+44]->Draw(6+46+46+46+46+46+46,6);
	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(7)+44]->Draw(6+46+46+46+46+46+46+46,6);

	gBundle_Tiles->mTile_Paint[gWorldEditor->GetPaintBase(8)+44]->Draw(6,6+46);
	//ADDTOOL
}

void Toolbox_Paint::TouchStart(int x, int y)
{
	if (y>=(6+48+18+10)-5+46) gApp.mAutoFix=!gApp.mAutoFix;
	else 
	{
		x-=10;
		y-=(18+10);
		x-=3;
		y-=3;

		x/=(int)mToolWidth;
		y/=46;

		if (x>=0 && y>=0) PickTool(x+(y*((int)(mWidth/mToolWidth))));
	}
		//Toolbox::TouchStart(x,y);


}

void Toolbox_Paint::PickTool(int theNumber)
{
	_DLOG("Tool: %d",theNumber);

	gWorldEditor->mWantNewID=true;
	gWorldEditor->mToolType=2;
	gWorldEditor->mTool=theNumber;
	Kill();
}

int WorldEditor::GetPaintRef()
{
	int aBase=GetPaintBase(mTool);
	return aBase+44;
}

int WorldEditor::GetPaintBase(int theTool)
{
	switch (theTool)
	{
	case 0:return 0;break;	//ADDTOOL
	case 1:return 47;break;	//ADDTOOL
	case 2:return 94;break;	//ADDTOOL
	case 3:return 94+47;break;	//ADDTOOL
	case 4:return 94+47+47;break;	//ADDTOOL
	case 5:return 94+47+47+47;break;	//ADDTOOL
	case 6:return 94+47+47+47+47;break;	//ADDTOOL
	case 7:return 94+47+47+47+47+47;break;	//ADDTOOL
	case 8:return 94+47+47+47+47+47+47;break;	//ADDTOOL
	}

	return 0;
}

void WorldEditor::PlotPaint(int x, int y)
{
	if (x<0 || y<0 || x>=mGridWidth || y>=mGridHeight) return;

	if (mWantNewID) 
	{
		mWantNewID=false;
		mPaintID++;
		if (mPaintID>500) mPaintID=0;
	}

	//
	// Paint types:
	// 1. Draws on 1, 47 block layout
	// 2. Draws on 1, 9 block layout
	// 3. Draws on 1, 3 block layout vertical
	// 4. Draws on 1, 3 block layout horizontal
	// 5. Draws on 1, "tube" layout
	//
	switch (mTool)
	{
	case VELCRO_BLOCK:
		mPaintMask=-1;
	case 0:	// ADDTOOL
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		{
			Grid *aGrid=GetGridPtr(x,y);
			int aOldID=aGrid->mPaintID;
			int aOldPaint=aGrid->mPaint;

			PlotPaint1(x,y,mTool,mPaintID,true);
			PaintFix(x,y,mTool,mPaintID);

			if (mTool==VELCRO_BLOCK)
			{
				int aHold=mPaintID;
				mPaintID=501;

				

				int aOldTool=mTool;
				aOldID=501;

				PaintFix(x-1,y-1,aOldTool,aOldID);
				PaintFix(x-0,y-1,aOldTool,aOldID);
				PaintFix(x+1,y-1,aOldTool,aOldID);

				PaintFix(x-1,y-0,aOldTool,aOldID);
				PaintFix(x+1,y-0,aOldTool,aOldID);

				PaintFix(x-1,y+1,aOldTool,aOldID);
				PaintFix(x-0,y+1,aOldTool,aOldID);
				PaintFix(x+1,y+1,aOldTool,aOldID);

				mPaintID=aHold;
			}
			else if ((aOldID!=aGrid->mPaintID && gApp.mAutoFix))
			{
				int aOldTool=PaintToTool(aOldPaint);
				PaintFix(x-1,y-1,aOldTool,aOldID);
				PaintFix(x-0,y-1,aOldTool,aOldID);
				PaintFix(x+1,y-1,aOldTool,aOldID);

				PaintFix(x-1,y-0,aOldTool,aOldID);
				PaintFix(x+1,y-0,aOldTool,aOldID);

				PaintFix(x-1,y+1,aOldTool,aOldID);
				PaintFix(x-0,y+1,aOldTool,aOldID);
				PaintFix(x+1,y+1,aOldTool,aOldID);
			}
			break;
		}
	}
}

bool WorldEditor::PlotPaint1(int x, int y, int theTool,int theID, bool noPaint)
{

	Grid *aG=GetGridPtr(x,y);
	if (!aG) return false;

	bool aResult=false;

	
	if (mPaintMask==-1)
	{
		if (aG->mLayout==1 || aG->mLayout==67) // LAYOUT HERE?
		{
			int aPaintBase=GetPaintBase(theTool);

			int aTile=0;
			if (theTool==0) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==1) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==2) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==3) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==4) return false;
			if (theTool==5) return false;
			if (theTool==6) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==7) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==8) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL

			int aNewPaint=aTile+aPaintBase;
			int aNewPaintID=theID;

			if (aNewPaint!=aG->mPaint || aNewPaintID!=aG->mPaintID) aResult=true;
			if (!noPaint) aG->mPaint=aNewPaint;
			//else aG->mPaintID=-1;
			aG->mPaintID=aNewPaintID;
			if (aG->mPaint==511) aG->mPaint=0;
		}
		else if (aG->mLayout==2)
		{
			int aPaintBase=GetPaintBase(theTool);

			int aTile=0;
			if (theTool==0) return false;
			if (theTool==1) return false;
			if (theTool==2) return false;
			if (theTool==3) return false;
			if (theTool==4) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==5) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==6) return false;
			if (theTool==7) return false;
			if (theTool==8) return false;

			int aNewPaint=aTile+aPaintBase;
			int aNewPaintID=theID;

			if (aNewPaint!=aG->mPaint || aNewPaintID!=aG->mPaintID) aResult=true;
			if (!noPaint) aG->mPaint=aNewPaint;
			//else aG->mPaint=-1;
			aG->mPaintID=aNewPaintID;
			if (aG->mPaint==511) aG->mPaint=0;
			//if (aG->mPaint>gBundle_Tiles->mTile_Paint.mSpriteCount) aG->mPaint=0;
		}
		else if (aG->mLayout==VELCRO_BLOCK)
		{
			int aPaintBase=94+47+47+47+47+47+47+47;//GetPaintBase(theTool);

			int aTile=0;
			theID=501;

			if (theTool==VELCRO_BLOCK) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			
			
			int aNewPaint=aTile+aPaintBase;
			int aNewPaintID=theID;

			if (aNewPaint!=aG->mPaint || aNewPaintID!=aG->mPaintID) aResult=true;
			aG->mPaint=aNewPaint;

			aG->mPaintID=aNewPaintID;
			if (aG->mPaint==511) aG->mPaint=0;

			//aG->mPaint=aPaintBase+gRand.Get(5)+1;

		}
	}
	else 
	{
		//
		// Special painting for robot doors and other things that aren't
		// regular paint, but can be painted on, like.
		//
		if (aG->mLayout==mPaintMask)
		{
			int aPaintBase=GetPaintBase(theTool);

			int aTile=0;
			if (theTool==0) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==1) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==2) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==3) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==4) return false;
			if (theTool==5) return false;
			if (theTool==6) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==7) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL
			if (theTool==8) aTile=GetTileMatch47(x,y,theID);	//ADDTOOL

			int aNewPaint=aTile+aPaintBase;
			int aNewPaintID=theID;

			if (aNewPaint!=aG->mPaint || aNewPaintID!=aG->mPaintID) aResult=true;
			if (!noPaint) aG->mPaint=aNewPaint;
			//else aG->mPaint=-1;
			aG->mPaintID=aNewPaintID;
			if (aG->mPaint==511) aG->mPaint=0;
			//if (aG->mPaint>gBundle_Tiles->mTile_Paint.mSpriteCount) aG->mPaint=0;
		}
	}
	return aResult;
}

void WorldEditor::PaintFix(int x, int y, int theTool, int theID)
{
	Grid *aG=GetGridPtr(x,y);
	if (!aG) return;

	if (aG->mPaintID==theID)
	{
		bool aFix=false;
		if (theTool==0) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==1) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==2) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==3) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==4) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==5) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==6) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==7) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==8) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL
		if (theTool==VELCRO_BLOCK) aFix=PlotPaint1(x,y,theTool,theID);	//ADDTOOL

		if (aFix)
		{
			PaintFix(x-1,y-1,theTool,theID);
			PaintFix(x-0,y-1,theTool,theID);
			PaintFix(x+1,y-1,theTool,theID);

			PaintFix(x-1,y-0,theTool,theID);
			PaintFix(x+1,y-0,theTool,theID);

			PaintFix(x-1,y+1,theTool,theID);
			PaintFix(x-0,y+1,theTool,theID);
			PaintFix(x+1,y+1,theTool,theID);
		}
	}
}

bool WorldEditor::IsTileMatch(int x, int y, int theID)
{
	Grid *aG=GetGridPtr(x,y);
	if (!aG) return false;

	return (aG->mPaintID==theID);
}


int WorldEditor::GetTileMatch47(int x, int y, int theID)
{
	int aResultTile=44;
	int aObjectType=theID;//mPaintID;

	#define STEP 1

	if(IsTileMatch(x,y-STEP,aObjectType))
	{
		aResultTile=0;
		if(IsTileMatch(x+STEP,y,aObjectType))
		{
			aResultTile=4;
			if(IsTileMatch(x,y+STEP,aObjectType))
			{
				aResultTile=12;
				if(IsTileMatch(x-STEP,y,aObjectType))
				{
					aResultTile=28;
					if(IsTileMatch(x+STEP,y-STEP,aObjectType))
					{
						aResultTile=29;
						if(IsTileMatch(x+STEP,y+STEP,aObjectType))
						{
							aResultTile=33;
							if(IsTileMatch(x-STEP,y+STEP,aObjectType))
							{
								aResultTile=39;
								if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=43;
							}
							else if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=40;
						}
						else if(IsTileMatch(x-STEP,y+STEP,aObjectType))
						{
							aResultTile=37;
							if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=41;
						}
						else if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=36;
					}
					else if(IsTileMatch(x+STEP,y+STEP,aObjectType))
					{
						aResultTile=30;
						if(IsTileMatch(x-STEP,y+STEP,aObjectType))
						{
							aResultTile=34;
							if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=42;
						}
						else if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=38;
					}
					else if(IsTileMatch(x-STEP,y+STEP,aObjectType))
					{
						aResultTile=31;
						if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=35;
					}
					else if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=STEP;
				}
				else if(IsTileMatch(x+STEP,y-STEP,aObjectType))
				{
					aResultTile=16;
					if(IsTileMatch(x+STEP,y+STEP,aObjectType))aResultTile=18;
				}
				else if(IsTileMatch(x+STEP,y+STEP,aObjectType))aResultTile=17;
			}
			else if(IsTileMatch(x-STEP,y,aObjectType))
			{
				aResultTile=15;
				if(IsTileMatch(x+STEP,y-STEP,aObjectType))
				{
					aResultTile=25;
					if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=27;
				}
				else if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=26;
			}
			else if(IsTileMatch(x+STEP,y-STEP,aObjectType))aResultTile=8;
		}
		else if(IsTileMatch(x,y+STEP,aObjectType))
		{
			aResultTile=45;
			if(IsTileMatch(x-STEP,y,aObjectType))
			{
				aResultTile=14;
				if(IsTileMatch(x-STEP,y+STEP,aObjectType))
				{
					aResultTile=22;
					if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=24;
				}
				else if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=23;
			}
		}
		else if(IsTileMatch(x-STEP,y,aObjectType))
		{
			aResultTile=7;
			if(IsTileMatch(x-STEP,y-STEP,aObjectType))aResultTile=11;
		}
	}
	else if(IsTileMatch(x+STEP,y,aObjectType))
	{
		aResultTile=1;
		if(IsTileMatch(x,y+STEP,aObjectType))
		{
			aResultTile=5;
			if(IsTileMatch(x-STEP,y,aObjectType))
			{
				aResultTile=13;
				if(IsTileMatch(x+STEP,y+STEP,aObjectType))
				{
					aResultTile=19;
					if(IsTileMatch(x-STEP,y+STEP,aObjectType))aResultTile=21;
				}
				else if(IsTileMatch(x-STEP,y+STEP,aObjectType))aResultTile=20;
			}
			else if(IsTileMatch(x+STEP,y+STEP,aObjectType))aResultTile=9;
		}
		else if(IsTileMatch(x-STEP,y,aObjectType))
		{
			aResultTile=46;
		}
	}
	else if(IsTileMatch(x,y+STEP,aObjectType))
	{
		aResultTile=2;
		if(IsTileMatch(x-STEP,y,aObjectType))
		{
			aResultTile=6;
			if(IsTileMatch(x-STEP,y+STEP,aObjectType))aResultTile=10;
		}
	}
	else if(IsTileMatch(x-STEP,y,aObjectType))
	{
		aResultTile=3;
	}
	return aResultTile;
}

int WorldEditor::PaintToTool(int thePaint)
{
	if (thePaint<=46) return 0;	//ADDTOOL
	else if (thePaint<=46+47) return 1;	//ADDTOOL
	else if (thePaint<=46+47+47) return 2;	//ADDTOOL
	else if (thePaint<=46+47+47+47) return 3;	//ADDTOOL
	else if (thePaint<=46+47+47+47+47) return 4;	//ADDTOOL
	else if (thePaint<=46+47+47+47+47+47) return 5;	//ADDTOOL
	else if (thePaint<=46+47+47+47+47+47+47) return 6;	//ADDTOOL
	else if (thePaint<=46+47+47+47+47+47+47+47) return 7;	//ADDTOOL
	else if (thePaint<=46+47+47+47+47+47+47+47+47) return 8;	//ADDTOOL

	return 0;
}

void Toolbox_File::DrawTools()
{
	gG.Translate(20,20);
	gBundle_ETools->mFiletool[0].Center(6,6);
	gBundle_ETools->mFiletool[1].Center(6+46+46,6);
	gBundle_ETools->mFiletool[3].Center(6+46+46+46,6);
	gG.SetColor(.5f);
	if (gWorldEditor) if (gWorldEditor->mTestedOK) gG.SetColor();
	gBundle_ETools->mFiletool[4].Center(6+46+46+46+46,6);
	if (gWorldEditor) if (gWorldEditor->mTestedOK) gG.SetColor();
	gBundle_ETools->mFiletool[5].Center(6+46+46+46+46+46,6);
	gG.SetColor();

	gG.Translate(-20,-20);

}

void Toolbox_File::PickTool(int theNumber)
{
	if (theNumber==1) return;

	gOut.Out("FINISHME: Picked Editor Options!");

/*
	gSounds->mSelect.PlayPitched(1.1f);
	switch (theNumber)
	{
	case 0:
		{
			MsgBoxOld *aBox=new MsgBoxOld("REALLY QUIT?","REALLY QUIT?  YOUR CURRENT LEVEL WILL BE HERE NEXT TIME YOU EDIT!",1,"quit:");
			aBox->Size(300,250,Point(170,(320/2)-15));
			aBox->mCallback=gWorldEditor;
			break;
		}
	case 1:
		{
			MsgBoxOld *aBox=new MsgBoxOld("START NEW LEVEL?","THIS WILL ERASE YOUR CURRENT LEVEL.  ARE YOU SURE?",1,"new:");
			aBox->Size(300,250,Point(170,(320/2)-15));
			aBox->mCallback=gWorldEditor;
			break;
		};
	case 2:
		{
			SaveDialog *aSD=new SaveDialog("");
			break;
		}
	case 3:
		{
			BrowseDialog *aBD=new BrowseDialog();
			aBD->mCallback=gWorldEditor;
			gApp.mMsgBox=aBD;
			gApp.AddCPU(aBD);
			break;
		}
	case 4:
		{
			//
			// UPLOAD!
			//
			//UploadDialog *aSh=new UploadDialog();
			break;
		}
	}

	Kill();
*/
}

SaveDialog::SaveDialog(String theFilename)
{
	Size();
	//mBorder.Load(&gBundle_Play->mSmallrim);//Play->mBorder);

	gApp.mMsgBox=this;
	gApp.AddCPU(this);

	//iTools_ShowKeyboard(true);

	mCancel.Size(302,79,79,46);
	mCancel.SetNotify(this);
	mCancel.NotifyOnPress();
	*this+=&mCancel;

	mSave.Size(215,79,79,46);
	mSave.SetNotify(this);
	mSave.NotifyOnPress();
	*this+=&mSave;

	mBrowse.Size(417,32,27,27);
	mBrowse.SetNotify(this);
	mBrowse.NotifyOnPress();
	*this+=&mBrowse;


	mEditBox.Size(141,38,255,23);
	mEditBox.SetFont(&gBundle_Play->mFont_Commodore64Angled11);
	mEditBox.mForceUppercase=true;
	mEditBox.AllowCharacters(" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~`!#$%&*()-_=+;:'\",.?");
	mEditBox.FocusKeyboard();
	mEditBox.SetBackgroundColor(Color(0));
	mEditBox.SetTextColor(Color(0));
	mEditBox.SetTextOffset(Point(0,-14));
	mEditBox.SetText(gWorldEditor->mLevelname);
	*this+=&mEditBox;
	mHide=false;
}

SaveDialog::~SaveDialog()
{
	if (gAppPtr) if (!gApp.IsQuit()) if (gApp.mMsgBox==this) 
	{
		gApp.mMsgBox=NULL;
		//iTools_ShowKeyboard(false);
	}
}

void SaveDialog::Update()
{
}

void SaveDialog::Draw()
{
	if (mHide) return;

	gG.SetColor(0,0,0,.65f);
	gG.FillRect();
	gG.SetColor();
	//mBorder.Draw(9,9,463,139); //SpriteRect &gBundle_Play->mSmallrim

	DRAWGLOW("FILENAME:",31,22+5);
	gG.SetColor();
	//gBundle_ETools->mSave_Levelname.Draw(31,35);
	//gBundle_ETools->mSave_Editbox.Draw(149,30);

	gG.PushTranslate();
	gG.Translate();
	//mBorder.Draw(127-5,32-15,285+10,29+35); //SpriteRect &gBundle_Play->mSmallrim

	gG.Translate(mEditBox.UpperLeft());

	//
	// Draw glowy edit box...
	//

	//FIXME... WTH is this?  I drew the textbox manually?  Look into this.
	//         I have no idea what I was doing here or why.  Ignore until it goes away!
	/*
	gG.PushClip();
	gG.Clip(Rect((float)mEditBox.mBorder.mX,(float)mEditBox.mBorder.mY,mEditBox.mWidth-(mEditBox.mBorder.mX*2),mEditBox.mHeight-(mEditBox.mBorder.mY*2)));
	if (mEditBox.mStringUpToCursorWidth-mEditBox.mScroll>(mRect.mWidth-(mEditBox.mBorderSize*2))) mEditBox.mScroll=(int)(mEditBox.mStringUpToCursorWidth-(mRect.mWidth-(mEditBox.mBorderSize*2))+mEditBox.mCursorSize.mX);
	if (mEditBox.mStringUpToCursorWidth-mEditBox.mScroll<mEditBox.mBorderSize) mEditBox.mScroll=mEditBox.mStringUpToCursorWidth;
	float aXPos=(float)mEditBox.mBorderSize+mEditBox.mTextOffset.mX-mEditBox.mScroll;
	if (mEditBox.mCenter) aXPos=(mRect.mWidth/2)-(mEditBox.mFont->StringWidth(mEditBox.mText)/2);
	gG.PushClip();
	gG.Clip();
	if (gAppPtr->mKeyboardFocusGadget==&mEditBox) if (mEditBox.mCursorBlink) gBundle_ETools->mEditcursor.Center(Rect((float)mEditBox.mStringUpToCursorWidth+aXPos+mEditBox.mCursorOffset.mX,(float)mEditBox.mBorderSize+mEditBox.mCursorOffset.mY,mEditBox.mCursorSize.mX,mEditBox.mCursorSize.mY).UpperLeft()+Point(mEditBox.mCursorSize.mX/2,19.0f/2));
	gG.PopClip();
	DRAWGLOW(mEditBox.mText,aXPos,(float)mEditBox.mBorderSize+mEditBox.mTextOffset.mY);
	gG.PopClip();
	*/

	gG.PopTranslate();
	gG.Translate();

	CENTERGLOW("SAVE",mSave.Center().mX,mSave.Center().mY-22);
	CENTERGLOW("CANCEL",mCancel.Center().mX,mCancel.Center().mY-22);


	gBundle_ETools->mBrowse.Center(mBrowse.Center()+Point(0,1));
	if (mEditBox.GetText().Len()<=0) gG.SetColor(.5f);
	gG.SetColor();
}

void SaveDialog::Notify(void *theData)
{
	if (theData==&mSave)
	{
		if (mEditBox.GetText().Len()>0)
		{
			String aDir="sandbox://EXTRALEVELS64\\";
			MakeDirectory(aDir);

			String aSaveName=Sprintf("sandbox://EXTRALEVELS64\\%s.kitty",mEditBox.GetText().c());
			gWorldEditor->mLevelname=mEditBox.mText;
			gWorldEditor->Save(aSaveName);
			gSounds->mBell.Play();

			Kill();
		}
	}
	if (theData==&mCancel)
	{
		Kill();
	}
	if (theData==&mBrowse)
	{
		//
		// Open a browser...
		//
		BrowseDialog *aBD=new BrowseDialog();
		aBD->mSelectText="SELECT";
		aBD->mCallback=this;
		gApp.AddCPU(aBD);
		mHide=true;

		//iTools_ShowKeyboard(false);
	}
	if (theData==&gApp.mGlobalNotify)
	{
		mHide=false;
		if (gApp.mGlobalNotify.strnicmp("BROWSE::",8))
		{
			mEditBox.SetText(gApp.mGlobalNotify.GetSegment(8,9999));
		}
		//iTools_ShowKeyboard(true);

	}
	
		
}

BrowseDialog::BrowseDialog()
{
	Size();
//	mBorder.Load(&gBundle_Play->mSmallrim);
	mCallback=NULL;

	Freshen();
	

	mCancel.Size(376,265,75,25);
	mCancel.NotifyOnPress();
	mCancel.SetNotify(this);
	*this+=&mCancel;

	mDelete.Size(35,265,75,25);
	mDelete.NotifyOnPress();
	mDelete.SetNotify(this);
	*this+=&mDelete;

	mSelect.Size(202,265,75,25);
	mSelect.NotifyOnPress();
	mSelect.SetNotify(this);
	*this+=&mSelect;

	mScroll=0;

	mSelectText="LOAD";
	mCaption="YOUR KITTY LEVELS";
}

void BrowseDialog::Freshen()
{
	_FreeList(KittyFile,mDirList);
	Array<String> aTempList;
	EnumDirectoryFiles("sandbox://EXTRALEVELS64\\",aTempList);

	for (int aCount=0;aCount<aTempList.Size();aCount++)
	{
		KittyFile *aKF=new KittyFile;
		aKF->mDifficulty=1;
		aKF->mName=aTempList[aCount];
		aKF->mName.Delete(".kitty");

		//FIXME
		//aKF->mName=gApp.StringDisplayFit(aKF->mName,&gBundle_Play->mFont_Commodore64Angled8,800);
		mDirList+=aKF;
	}

	mSelected=-1;
	mDragSpeed=0;

	mScrollMax=0;
	mScrollMin=(float)(mDirList.GetCount()/2);
	if ((mDirList.GetCount()%2)!=0) mScrollMin++;
	mScrollMin*=90;
	mScrollMin+=15;
	mScrollMin-=422;
	mScrollMin*=-1;

}

BrowseDialog::~BrowseDialog()
{
	if (gAppPtr) if (!gApp.IsQuit()) if (gApp.mMsgBox==this) gApp.mMsgBox=NULL;
	_FreeList(String,mDirList);
}

void BrowseDialog::Update()
{
	mScroll+=mDragSpeed;
	mScroll=_max(mScroll,mScrollMin);
	mScroll=_min(mScroll,mScrollMax);
	mDragSpeed*=.8f;
}

void BrowseDialog::Draw()
{
	gG.SetColor(0,0,0,.65f);
	gG.FillRect();
	gG.SetColor();
	//mBorder.Draw(10,10,480-20,320-20); //SPRITERECT &gBundle_Play->mSmallRim

	CENTERGLOW(mCaption,mWidth/2,20);
	if (mSelected==-1) {CENTERGLOW("SWIPE TO SCROLL, TAP TO SELECT",mWidth/2,255);}
	else
	{
		CENTERGLOW("CANCEL",mCancel.Center().mX,mCancel.Center().mY-20);
		CENTERGLOW("DELETE",mDelete.Center().mX,mDelete.Center().mY-20);
		CENTERGLOW(mSelectText,mSelect.Center().mX,mSelect.Center().mY-20);
	}

	gG.SetColor(0,0,0,1);
	gG.SetColor();

	//
	// Show files
	//
	gG.Clip(30,66,422,194);

	gG.Translate(mScroll,0.0f);
	Point aPos=Point(82,116);
	int aCounter=0;
	EnumList(KittyFile,aKF,mDirList)
	{
		//gBundle_Play->mItembox.Center(aPos);
		CENTERGLOWSMALL(aKF->mName,aPos.mX,aPos.mY+22);
		gG.SetColor();

		if (mSelected==aCounter)
		{
			gG.RenderAdditive();
			if (((gApp.AppTime()/10)%2)) gG.SetColor(0,1,0,1);
			//gBundle_Play->mItembox.Center(aPos);
			CENTEROVERGLOWSMALL(aKF->mName,aPos.mX,aPos.mY+22);
			gG.RenderNormal();
			gG.SetColor();
		}


		if (aPos.mY<120) aPos.mY+=90;
		else
		{
			aPos.mX+=90;
			aPos.mY=116;
		}
		aCounter++;
	}

	gG.Translate(-mScroll,0.0f);

	gG.Clip();
}

void BrowseDialog::Notify(void *theData)
{
	if (mSelected==-1) return;
	if (theData==&mSelect) 
	{
		if (mCallback)
		{
			KittyFile *aKF=(KittyFile*)mDirList[mSelected];
			if (aKF)
			{
				gApp.mGlobalNotify=Sprintf("BROWSE::%s",aKF->mName.c());
				mCallback->Notify(&gApp.mGlobalNotify);
			}
		}
		Kill();
	}
	if (theData==&mCancel) 
	{
		gApp.mGlobalNotify="BROWSECANCEL";
		mCallback->Notify(&gApp.mGlobalNotify);
		Kill();
	}
	if (theData==&gApp.mGlobalNotify)
	{
		if (gApp.mGlobalNotify.Contains(":yes"))
		{
			KittyFile *aKF=(KittyFile*)mDirList[mSelected];
			if (aKF)
			{
				String aDeleteName=Sprintf("sandbox://EXTRALEVELS64\\%s.kitty",aKF->mName.c());
				DeleteFile(aDeleteName.c());
				Freshen();
			}

		}
	}

	/*
	if (theData==&mDelete)
	{
		if (mSelected!=-1)
		{
			MsgBox *aBox=new MsgBox();
			bool aResult=aBox->Go("REALLY DELETE THIS LEVEL?",gGCenter(),1);
			if (aResult)
			{
				gOut.Out("FIXME: DELETE THE LEVEL!!!!");
			}
		}
	}
	*/
}

void BrowseDialog::MouseDown(int x, int y, int theButton)
{
	x-=(int)mScroll;
	Point aPos=Point(82,116);
	int aCounter=0;
	EnumList(KittyFile,aKF,mDirList)
	{
		if (Rect(aPos.mX-(75/2),aPos.mY-(75/2),75,75).ContainsPoint(x,y))
		{
/*
			if (mSelected==aCounter)
			{
				//
				// We selected it!
				// Notify whoever that we just selected a file!
				// So, the SaveDialog will use the filename... the WorldEditor will load it.
				// 
				if (mCallback)
				{
					gApp.mGlobalNotify=Sprintf("BROWSE::%s",aKF->mName.c());
					mCallback->Notify(&gApp.mGlobalNotify);
				}
				Kill();
			}
*/
			mSelected=aCounter;

			break;
		}

		if (aPos.mY<120) aPos.mY+=90;
		else
		{
			aPos.mX+=90;
			aPos.mY=116;
		}
		aCounter++;
	}
}

void BrowseDialog::StartDrag(int x, int y)
{
	mDragPos=Point(x,y);
}

void BrowseDialog::Drag(int x, int y)
{
	float aMod=x-mDragPos.mX;
	mDragSpeed=aMod;

	mDragPos=Point(x,y);
}

UploadDialog::UploadDialog()
{
	//mBorder.Load(&gBundle_Play->mBorder);
	
	gApp.mMsgBox=this;
	gApp.AddCPU(this);

	mFirstDraw=false;
	mWantUpload=true;

	mProgress=0.0f;

	mButton.Size(Rect(198,218,84,63).Expand(10));
	mButton.NotifyOnPress();
	mButton.SetNotify(this);
	*this+=&mButton;

	mSuccess=false;

				  
	
}

UploadDialog::~UploadDialog()
{
	if (gAppPtr) if (!gApp.IsQuit()) if (gApp.mMsgBox==this) gApp.mMsgBox=NULL;
}

void UploadDialog::Update()
{
	if (mWantUpload && mFirstDraw)
	{
		StartUpload();
		mWantUpload=false;
	}
}

void UploadDialog::Draw()
{
	mFirstDraw=true;

	gG.SetColor(0,0,0,.65f);
	gG.FillRect();
	gG.SetColor();
	//mBorder.Draw(10,10,480-20,320-20); //SPRITERECT: &gBundle_Play->mBorder

	//gG.SetColor(0,0,0,1);
	RectComplex aRect;
	aRect.Add(Rect(50,145,381,41).Expand(4));
	aRect.Subtract(Rect(50,145,381,41));
	for (int aCount=0;aCount<aRect.GetRectCount();aCount++) gG.FillRect(aRect.GetRect(aCount));

	//gG.FillRect(Rect(50,145,381,41).Expand(4));
	gG.SetColor(.5f);
	gG.FillRect(50,145,381,41);
	gG.SetColor();
}

void UploadDialog::Notify(void *theData)
{
	Kill();
}

void UploadDialog::StartUpload()
{
	gWorldEditor->Save("sandbox://SHARE64.kitty");
	IOBuffer aBuffer;
	aBuffer.SetFileSource("sandbox://SHARE64.kitty");

	gSounds->mModem.Play();
}

void WorldEditor::DrawBackground()
{
	gG.Clear(.25f,0,.25f);
}


/*******************************/

DownloadDialog::DownloadDialog()
{
	//mBorder.Load(&gBundle_Play->mBorder);

	gApp.mMsgBox=this;
	gApp.AddCPU(this);

	mFirstDraw=false;
	mWantDownload=true;

	mProgress=0.0f;

	mButton.Size(Rect(198,218,84,63).Expand(10));
	mButton.NotifyOnPress();
	mButton.SetNotify(this);
	*this+=&mButton;
	mSuccess=false;
}

DownloadDialog::~DownloadDialog()
{
	if (gAppPtr) if (!gApp.IsQuit()) if (gApp.mMsgBox==this) gApp.mMsgBox=NULL;
}

void DownloadDialog::Update()
{
	if (mWantDownload && mFirstDraw)
	{
		StartDownload();
		mWantDownload=false;
	}

}

void DownloadDialog::Draw()
{
	mFirstDraw=true;

	gG.SetColor(0,0,0,.65f);
	gG.FillRect();
	gG.SetColor();
	//mBorder.Draw(10,10,480-20,320-20); //SPRITERECT: &gBundle_Play->mBorder

	//gG.SetColor(0,0,0,1);
	RectComplex aRect;
	aRect.Add(Rect(50,145,381,41).Expand(4));
	aRect.Subtract(Rect(50,145,381,41));
	for (int aCount=0;aCount<aRect.GetRectCount();aCount++) gG.FillRect(aRect.GetRect(aCount));
	//gG.FillRect(Rect(50,145,381,41).Expand(4));
	gG.SetColor(.5f);
	gG.FillRect(50,145,381,41);
	gG.SetColor();
}

void DownloadDialog::Notify(void *theData)
{
	// FIXME MagicInternet
	//gApp.mMagic.Cancel();
	Kill();
}

void DownloadDialog::StartDownload()
{
	gSounds->mModem.Play();
}

LevelSettingsDialog* gLevelSettingsDialog=NULL;
void WorldEditor::OpenSettings()
{
	mTempConveyorSpeed=mConveyorSpeed-.1f;
	gWorldEditor->QuickSave();

	LevelSettingsDialog* aDialog=new LevelSettingsDialog;
	aDialog->HookTo(&gLevelSettingsDialog);
	aDialog->mOldName=gWorldEditor->mName;
	aDialog->Size();
	FullscreenSmallScreen(aDialog,gG.Center());
	aDialog->mX+=gReserveLeft;
	aDialog->mWidth-=gReserveRight+gReserveLeft;

	aDialog->SetBundle(&gApp.mBundle_FunDialog);
	aDialog->SetTitle("LEVEL SETTINGS");				// Sets the dialog title

	//
	// Build the dialog...
	//


	DialogPanel* aPanel=aDialog->MakeNewPanel("LEVEL SETTINGS");

	DialogTextbox* aTB=new DialogTextbox("Name:",&gWorldEditor->mName);
	aTB->mTextBox.ForbidCharacters("/\\?%%*:|\"<>.[]");
	aPanel->Add(aTB);
	aPanel->Add(new DialogSeperator(10));
	aDialog->mTagDialog=new DialogPickTags(gWorldEditor->mTags);
	aPanel->Add(aDialog->mTagDialog);

	aPanel->Add(new DialogSlider("BELT SPEED:",&mTempConveyorSpeed));

	if (gApp.mToolVersion>=1)
	{
		aDialog->RehupMusicPanel();
		aPanel->Add(new DialogNextPanel("CUSTOMIZE MUSIC","EDIT",aDialog->mMusicPanel));
	}

	gApp.AddCPU(aDialog);
}

void LevelSettingsDialog::RehupMusicPanel()
{
	if (!mMusicPanel) 
	{
		mMusicPanel=MakeNewPanel("CUSTOM MUSIC",false);

		mMusicPanel->Add(new DialogButton("OPEN COMPOSER (IN A WEB PAGE)"));
		mMusicPanel->Add(new DialogGroupSpacer(15));
#ifdef _WASM
		mMusicPanel->Add(new DialogGroupHeader("When you finish composing, copy the URL to your clipboard, and follow the instructions at the bottom of your web browser!"));
#else
		mMusicPanel->Add(new DialogGroupHeader("When you finish composing, copy the URL to your clipboard, and touch one of the song slots below to paste it."));
#endif
		mMusicPanel->Add(new DialogGroupSpacer(5));
		mSongButton[0]=mMusicPanel->Add(new DialogButton("REPLACE SONG #1"));
		mSongButton[1]=mMusicPanel->Add(new DialogButton("REPLACE SONG #2"));
		mSongButton[2]=mMusicPanel->Add(new DialogButton("REPLACE SONG #3"));
	}

	if (gWorldEditor->mCustomSong[0].Len()==0) mSongButton[0]->mName="REPLACE SONG #1";
	else mSongButton[0]->mName="ERASE CUSTOM SONG #1";

	if (gWorldEditor->mCustomSong[1].Len()==0) mSongButton[1]->mName="REPLACE SONG #2";
	else mSongButton[1]->mName="ERASE CUSTOM SONG #2";

	if (gWorldEditor->mCustomSong[2].Len()==0) mSongButton[2]->mName="REPLACE SONG #3";
	else mSongButton[2]->mName="ERASE CUSTOM SONG #3";
}

LevelSettingsDialog::LevelSettingsDialog()
{
	mMusicPanel=NULL;
}

void LevelSettingsDialog::NextPanel(DialogPanel* thePanel)
{
	if (thePanel==mMusicPanel) OS_Core::Query("PASTE:ON");
	else OS_Core::Query("PASTE:OFF");

	Dialog::NextPanel(thePanel);
}

void LevelSettingsDialog::PreviousPanel()
{
	OS_Core::Query("PASTE:OFF");
	Dialog::PreviousPanel();
}

void LevelSettingsDialog::ChangedControl(DialogWidget* theWidget)
{
	if (theWidget->mName.StartsWith("OPEN COMPOSER")) OpenURL("http://www.yoururl.com/compose.html");
	else if (theWidget->mName.StartsWith("REPLACE SONG"))
	{
		int aNumber=theWidget->mName.GetSegmentAfter('#').ToInt()-1;
		String aSong=PasteFromClipboard();
		if (aSong.Contains("#"))
		{
			BeepBox* aTest=new BeepBox;
			aTest->Load(aSong);
			aTest->Play();

			MsgBox *aBox=new MsgBox();
			aBox->GoX("YOUR SONG SHOULD NOW BE PLAYING.  DOES IT SOUND CORRECT?",gGCenter(),1,MBHOOK(
				{
					if (aTest)
					{
						aTest->Stop();
						delete aTest;
					}
					if (theResult=="YES")
					{
						gWorldEditor->mCustomSong[aNumber]=aSong;
						if (gLevelSettingsDialog) gLevelSettingsDialog->RehupMusicPanel();
					}
					else
					{
						MsgBox *aBox=new MsgBox();
						aBox->GoNoThrottle("PLEASE TRY TO IMPORT YOUR SONG AGAIN.  MAKE SURE YOU COPIED THE URL CORRECTLY FROM THE COMPOSER'S WEB PAGE!",gGCenter(),0);
					}
				}
			));
		}
		else
		{
			MsgBox *aBox=new MsgBox();
			aBox->GoNoThrottle("A SONG HAS NOT BEEN COPIED ONTO YOUR CLIPBOARD, OR WE CANNOT RECOGNIZE IT.  PLEASE COPY THE URL AGAIN!",gGCenter(),0);
		}
	}
	else if (theWidget->mName.StartsWith("ERASE CUSTOM SONG"))
	{
		int aNumber=theWidget->mName.GetSegmentAfter('#').ToInt()-1;

		MsgBox *aBox=new MsgBox();
		aBox->GoX("REALLY ERASE THIS CUSTOM SONG? (THE GAME WILL REVERT BACK TO THE DEFAULT SONG FOR THIS SLOT)",gGCenter(),1,MBHOOK(
			{
				if (theResult=="YES")
				{
					gWorldEditor->mCustomSong[aNumber]="";
					if (gLevelSettingsDialog) gLevelSettingsDialog->RehupMusicPanel();
				}
			}
		));
	}
}

bool LevelSettingsDialog::Done()
{
	OS_Core::Query("PASTE:OFF");

	bool aDidSave=false;
	if (mOldName!=gWorldEditor->mName)
	{
		// Un-Share?
		if (gWorldEditor->mUploadID!=-1) 
		{
			MsgBox *aBox=new MsgBox();
			static String aNewName;
			aNewName=gWorldEditor->mName;
			gWorldEditor->mName=mOldName;

			aBox->GoX("TO RENAME THIS LEVEL, IT MUST BE REMOVED FROM SHARING!\nDO THAT NOW?",gGCenter(),1,MBHOOK(
				{
					if (theResult=="YES") 
					{
						RComm::LevelSharing_DeleteLevel(gAppPtr->mUserName,gAppPtr->mUserPassword,gWorldEditor->mUploadID);
						gWorldEditor->mUploadID=-1;
						gWorldEditor->mName=aNewName;
						if (gLevelSettingsDialog) gLevelSettingsDialog->FinishRename();
					}
					aNewName="";
					if (gLevelSettingsDialog) gLevelSettingsDialog->Kill();
				}
			));
			return false;
		}
		else {FinishRename();return true;}
	}

	float aHold=gWorldEditor->mWantConveyorSpeed;
	gWorldEditor->mWantConveyorSpeed=gWorldEditor->mConveyorSpeed=gWorldEditor->mTempConveyorSpeed+.1f;
	if (gWorldEditor->mWantConveyorSpeed!=aHold) {gWorldEditor->mTestedOK=false;gWorldEditor->mTestedNoDying=false;}
	gWorldEditor->mTags=mTagDialog->mTagBits;
	gWorldEditor->QuickSave();
	return true;
}

void LevelSettingsDialog::FinishRename()
{
	gMakermall->RemoveFromRecentLevels(mOldName);
	//
	String aFN=Sprintf("sandbox://EXTRALEVELS64\\%s.kitty",mOldName.c());

	gWorldEditor->mWantConveyorSpeed=gWorldEditor->mConveyorSpeed=gWorldEditor->mTempConveyorSpeed+.1f;
	gWorldEditor->mTags=mTagDialog->mTagBits;
	gWorldEditor->QuickSave();

	DeleteFile(aFN);
	gMakermall->SetMostRecentLevel(gWorldEditor->mName);
}

DialogPickTags::DialogPickTags(longlong theTags)
{
	mAllowInfiniteTagPicks=false;
	longlong aTMask=1;
	mName="Tags: (Choose up to 3)";

	for (int aCount=0;aCount<gMakermall->mTags.Size();aCount++)
	{
		if (theTags&aTMask) 
		{
			mTags+=aCount;
		}
		aTMask*=2;
	}
	mTagBits=theTags;
}

void DialogPickTags::Initialize()
{

#define TAGHEIGHT 30
#define TAGSPACING 5
#define TAGSLEFT 10

	float aX=TAGSLEFT;
	float aY=40;
	float aBottom=0;
	for (int aCount=0;aCount<gMakermall->mTags.Size();aCount++)
	{
		String& aText=gMakermall->mTags[aCount];
		CheckBox* aB=new CheckBox;
		aB->ExtraData()=(void*)aCount;
		aB->mWidth=gBundle_Play->mFont_Commodore64Angled11.Width(aText);
		aB->mWidth+=26;
		aB->mHeight=TAGHEIGHT;
		aB->SetSounds(&gSounds->mClick);
		aB->SetNotify(this);
		mButtonList+=aB;

		if (aX+aB->mWidth>mWidth-20)
		{
			aX=TAGSLEFT;
			aY+=TAGHEIGHT;
			aY+=TAGSPACING;
		}

		*this+=aB;

		aBottom=aY+TAGHEIGHT;

		aB->mX=aX;
		aB->mY=aY;

		aX+=aB->mWidth+TAGSPACING;
	}

	mHeight=aBottom+TAGSPACING;
	UpdateButtons();

}

void DialogPickTags::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor();

	if (mCPUManager)
	{
		gG.PushTranslate();
		gG.Translate(-CurrentDialog()->mWidgetBorder);
		EnumList(Button,aB,*mCPUManager)
		{
			Color aTColor=Color(1);
			Rect aRect=Rect(aB->mX,aB->mY,aB->mWidth,aB->mHeight);
			if (!aB->IsDown()) gG.SetColor(.5f,.5f,.5f,.5f);
			else 
			{
				//gG.SetColor(1,.75f,0);
				//gG.SetColor(75.0f/255.0f,107.0f/255.0f,178.0f/255.0f);
				gG.SetColor(1);
				aTColor=Color(80.0f/255.0f,101.0f/255.0f,108.0f/255.0f,1);
				aRect=aRect.Expand(2);
			}
			gBundle_ETools->mTagCorner.DrawMagicBorder(aRect,true);
			//gG.FillRect(aB->mX,aB->mY,aB->mWidth,aB->mHeight);
			gG.SetColor(aTColor);
			int aPick=(uintptr_t)aB->ExtraData();
			String& aStr=gMakermall->mTags[aPick];
			gBundle_Play->mFont_Commodore64Angled11.Center(aStr,aB->mX+aB->mWidth/2,aB->mY+gBundle_Play->mFont_Commodore64Angled11.mAscent+3);
			gG.SetColor();
		}
		gG.PopTranslate();
	}
}

void DialogPickTags::Notify(void* theData)
{
	bool aChanged=false;
	EnumList(Button,aB,*mCPUManager)
	{
		if (aB==theData)
		{
			int aPick=(uintptr_t)aB->ExtraData();
			if (aB->IsDown())
			{
				mTags+=aPick;
				aChanged=true;
				if (!mAllowInfiniteTagPicks) if (mTags.Size()>3)
				{
					//
					// Remove bottom one!
					//
					mTags.DeleteElement(0);
				}
			}
			else
			{
				for (int aCount=0;aCount<mTags.Size();aCount++)
				{
					if (mTags[aCount]==aPick)
					{
						mTags.DeleteElement(aCount);
						aChanged=true;
						break;
					}
				}
			}
		}
	}

	if (aChanged)
	{
		mTagBits=0;
		longlong aTMask=1;

		for (int aCount=0;aCount<gMakermall->mTags.Size();aCount++)
		{
			if (mTags.FindValue(aCount)>=0) 
			{
				//gOut.Out("    --> Found Value %d -> Tags|= %d",aCount,aTMask);
				mTagBits|=aTMask;
			}
			aTMask*=2;
		}
		_DLOG("New Tags Bitmask: %d",mTagBits);
		UpdateButtons();
	}

}

void DialogPickTags::UpdateButtons()
{
	EnumList(Button,aB,*mCPUManager)
	{
		*aB->mIsDown=false;
		int aPick=(uintptr_t)aB->ExtraData();

		for (int aCount=0;aCount<mTags.Size();aCount++)
		{
			if (mTags[aCount]==aPick)
			{
				*aB->mIsDown=true;
				break;
			}
		}
	}

}

void WorldEditor::SetRadioMessage(IPoint thePos, String theText)
{
	Smart(RadioText) aRadioText;
	EnumSmartList(RadioText,aRT,mRadioTextList) 
	{
		_DLOG("Radio@[%s] = %s",aRT->mPos.ToString(),aRT->mText.c());
		if (aRT->mPos==thePos) aRadioText=aRT;
	}
	if (aRadioText.IsNull()) {aRadioText=new RadioText;mRadioTextList+=aRadioText;}
	aRadioText->mPos=thePos;
	aRadioText->mText=theText;
}

void WorldEditor::SetGrid(int theX, int theY, char theValue)
{
	if (theX<0 || theY<0) return;
	if (theX>=mGridWidth || theY>=mGridHeight) return;

	Grid* aGridPtr=&mGrid[theX+(theY*mGridWidth)];
	if (theValue!=RADIO_BLOCK && aGridPtr->mLayout==RADIO_BLOCK)
	{
		EnumSmartList(RadioText,aRT,mRadioTextList) 
		{
			if (aRT->mPos==IPoint(theX,theY))
			{
				_DLOG("Remove Radio Record...");
				mRadioTextList-=aRT;
				break;
			}
		}
	}
	aGridPtr->mLayout=theValue;
}



