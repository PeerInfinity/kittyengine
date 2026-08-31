#include "Makermall.h"
#include "MainMenu.h"
#include "Game.h"
extern float gNotched;

#define MAX_RECENT_LEVELS 25

float gTransferProgress=0;
char gTransferType=0;

String gTransferStatus;

bool gNarrowScreen=false;
int gAvatarStubCount=0;
SmartList(AvatarSprite) gAvatarList;
struct PendingAvatar
{
	Smart(Sprite) mSprite;
	String mAvatar;
};


inline longlong FixFilter(longlong theFilter)
{
	longlong aResult=theFilter;
	return aResult;
}


MyMLBox* gNotifyBox;
void SetNotifyMLBox(MyMLBox* theBox) {gNotifyBox=theBox;}
MyMLBox* GetNotifyMLBox() {return gNotifyBox;}
int gOldFeatures=0;


Makermall* gMakermall=NULL;
bool gValidLogin=false;
int gLocalTicker=1;

char* aCountryToFlagIcon[]={"--","ad","ae","af","ag","ai","al","am","an","ao","ar","as","at","au","aw","ax","az","ba","bb","bd","be","bf","bg","bh","bi","bj","bm","bn","bo","br","bs","bt","bv","bw","by","bz","ca","catalonia","cc","cd","cf","cg","ch","ci","ck","cl","cm","cn","co","cr","cs","cu","cv","cx","cy","cz","de","dj","dk","dm","do","dz","ec","ee","eg","eh","england","er","es","et","europeanunion","fam","fi","fj","fk","fm","fo","fr","ga","gb","gd","ge","gf","gh","gi","gl","gm","gn","gp","gq","gr","gs","gt","gu","gw","gy","hk","hm","hn","hr","ht","hu","id","ie","il","in","io","iq","ir","is","it","jm","jo","jp","ke","kg","kh","ki","km","kn","kp","kr","kw","ky","kz","la","lb","lc","li","lk","lr","ls","lt","lu","lv","ly","ma","mc","md","me","mg","mh","mk","ml","mm","mn","mo","mp","mq","mr","ms","mt","mu","mv","mw","mx","my","mz","na","nc","ne","nf","ng","ni","nl","no","np","nr","nu","nz","om","pa","pe","pf","pg","ph","pk","pl","pm","pn","pr","ps","pt","pw","py","qa","re","ro","rs","ru","rw","sa","sb","sc","scotland","sd","se","sg","sh","si","sj","sk","sl","sm","sn","so","sr","st","sv","sy","sz","tc","td","tf","tg","th","tj","tk","tl","tm","tn","to","tr","tt","tv","tw","tz","ua","ug","um","us","uy","uz","va","vc","ve","vg","vi","vn","vu","wales","wf","ws","ye","yt","za","zm","zw","??"};
int GetFlagIcon(String theCode)
{
	int aSearch=0;
	for (;;)
	{
		if (aCountryToFlagIcon[aSearch][0]=='?') break;
		if (theCode==aCountryToFlagIcon[aSearch]) return aSearch;
		aSearch++;
	}
	return 0;
}

bool IsLoginValid() {return gValidLogin;}

void BuildLocalLevels(void* theArg)
{
	RComm::RaptisoftQuery* aRQ=(RComm::RaptisoftQuery*)theArg;

	aRQ->mResult.WriteLine("[RESULT]OK[/RESULT][STARTCURSOR]0[/STARTCURSOR][ENDCURSOR]0[/ENDCURSOR]");
	aRQ->mResult.WriteLine("[LEVELS][LEVEL][COMMAND]NEW[/COMMAND][NAME]CREATE NEW LEVEL[/NAME][/LEVEL]");

	Array<String> aOrder;
	Utils::CSVToArray(gAppPtr->mSettings.GetString("LEVELORDER"),aOrder);

	Array<String> aFList;
	EnumDirectoryFiles("sandbox://EXTRALEVELS64\\",aFList);

#ifdef _DEBUG
	gOut.Out("Order 1");
	for (int aCount=0;aCount<aOrder.Size();aCount++) gOut.Out("  %s",aOrder[aCount].c());
#endif

	for (int aCount=0;aCount<aFList.Size();aCount++)
	{
		bool aSkip=false;
		for (int aCount2=0;aCount2<aOrder.Size();aCount2++)
		{
			if (aOrder[aCount2]==aFList[aCount]) {aSkip=true;break;}
		}
		if (!aSkip) aOrder+=aFList[aCount];
	}
#ifdef _DEBUG
	gOut.Out("Order 2");
	for (int aCount=0;aCount<aOrder.Size();aCount++) gOut.Out("  %s",aOrder[aCount].c());
#endif

	//
	// Okay, let's get all the levels...
	//
	Array<int> aIDList;
	String aSBox="sandbox://EXTRALEVELS64\\";
	String aLStr;
	for (int aCount=0;aCount<aOrder.Size();aCount++)
	{
		if (aOrder[aCount].Len()==0) continue;

		IOBuffer aBuffer;
		String aFN=aSBox;
		aFN+=aOrder[aCount];

		if (DoesFileExist(aFN))
		{
			aBuffer.Load(aFN);
			if (aBuffer.Len())
			{
				if (aBuffer.ReadInt()!=SAVEGAME_VERSION) 
				{
					// Do we do anything?
				}
				SaveGame aSave;
				aSave.mVersion=SAVEGAME_VERSION;
				aSave.Load(aBuffer);

				int aUploadID;
				String aLevelName;
				int aTags;

				aSave.StartChunk();
				aSave.Sync(&aUploadID);
				aSave.Sync(&aLevelName);
				aSave.Sync(&aTags);

				if (aUploadID!=-1) aIDList+=aUploadID;

				aLStr+="[LEVEL][LOCAL]1[/LOCAL]";
				aLStr+="[NAME]";aLStr+=aLevelName;aLStr+="[/NAME]";
				aLStr+="[ID]";aLStr+=aUploadID;aLStr+="[/ID]";
				aLStr+="[TAGS]";aLStr+=aTags;aLStr+="[/TAGS]";
				aLStr+="[/LEVEL]";

				// Server sends back...
/*
				[LEVEL]
				[AUTHOR]ch3a7er[/AUTHOR]
				[AUTHORLEVEL]18[/AUTHORLEVEL]
				[AVATAR]hoggy_065[/AVATAR]
				[COUNTRY]KZ[/COUNTRY]
				[RATING]10[/RATING]
				[DATE]2017-12-27 18:59:29[/DATE]
				[TAGS]132[/TAGS]
				[PLAYCOUNT]30[/PLAYCOUNT]
				[WINCOUNT]15[/WINCOUNT]
				[STARS]0[/STARS]
				[MISCFLAG]0[/MISCFLAG]
				[COMMENTS]1[/COMMENTS]
				[/LEVEL]
*/
			}
		}

	}



	aRQ->mResult.WriteLine(aLStr);
	aRQ->mResult.WriteLine("[/LEVELS]");

	aRQ->mDone=true;
}

void BuildPlayingLevels(void* theArg)
{
	RComm::RaptisoftQuery* aRQ=(RComm::RaptisoftQuery*)theArg;

	gMakermall->LoadRecents();

	aRQ->mResult.WriteLine("[RESULT]OK[/RESULT][STARTCURSOR]0[/STARTCURSOR][ENDCURSOR]0[/ENDCURSOR][LEVELS]");
	for (int aCount=0;aCount<gMakermall->mRecentLevels.Size();aCount++)
	{
		aRQ->mResult.WriteLine("[LEVEL]");
		aRQ->mResult.WriteLine(gMakermall->mRecentLevels[aCount]);
		aRQ->mResult.WriteLine("[/LEVEL]");
	}
	aRQ->mResult.WriteLine("[/LEVELS]");
	aRQ->mDone=true;
}


Makermall::Makermall()
{
	gNotifyBox=NULL;

	if (gApp.mAlwaysShowGlitchyLevels) RComm::SetIgnoreFilter(0);
	else RComm::SetIgnoreFilter(8192);

	mTags+="EASY";
	mTags+="HARD";

	mAdmin=false;
	gApp.FadeInMusic(NULL);
	gValidLogin=false;
	mPlayingID=-1;
	mRatedID=-1;
	mNotifyLevelup=false;

	if (gG.Width()<565) gNarrowScreen=true;


	gMakermall=this;
	mStep=0;
	mNextViewing=&mMyLevelsButton;
	mViewing=NULL;
	mFilter=gAppPtr->mSettings.GetInt(Sprintf("Filter_%s",gAppPtr->mUserName.c()));
	mNow=gAppPtr->mSettings.GetString(Sprintf("NewDate_%s",gAppPtr->mUserName.c()));

	mPendingQueryList+=RComm::LevelSharing_GetTags();
	CheckLogin();
	//mPendingQueryList+=RComm::MakerMall_Hello(gAppPtr->mUserName,gAppPtr->mUserPassword,gApp.mVersion);
	mListWidget=NULL;

	FillTags(gAppPtr->mSettings.GetString("TAGS"));

	String aSet="viewing";
	String aVW=gApp.mSettings.GetString(aSet);
	if (aVW=="new") mNextViewing=&mNewLevelsButton;
	if (aVW=="hot") mNextViewing=&mHotLevelsButton;
	if (aVW=="star") mNextViewing=&mStarLevelsButton;
	if (aVW=="random") mNextViewing=&mRandomLevelsButton;
	if (aVW=="playing") mNextViewing=&mPlayingLevelsButton;

	String aMsgName=Sprintf("Messages_%s",gAppPtr->mUserName.c());
	String aStr=gApp.mSettings.GetString(aMsgName);
	Utils::CSVToArray(aStr,mMsgList,127);
	for (int aCount=0;aCount<mMsgList.Size();aCount++) mMsgIDList[aCount]=gLocalTicker++;

	mUnreadMsg=gApp.mSettings.GetBool(Sprintf("Unread_%s",gAppPtr->mUserName.c()));
	mSearchAgain=false;

	mX+=gReserveLeft;mWidth-=gReserveRight+gReserveLeft;

}

Makermall::~Makermall()
{
	if (gMakermall==this) gMakermall=NULL;
	gNotifyBox=NULL;

	gAvatarList.Clear();
}

void Makermall::CheckLogin() 
{
	gOldFeatures=mFeatures;
	EnableUpdate();
	mPendingQueryList+=RComm::LevelSharing_Hello(gAppPtr->mUserName,gAppPtr->mUserPassword,gApp.mVersion);
}


void Makermall::Initialize()
{
	FocusSpins();
	FillTags(gApp.mSettings.GetString("TAGS"));

	FocusBackButton();
	gOldFeatures=mFeatures;

	mMainMenuButton.Size(0,0,gBundle_Play->mBack.WidthF(),gBundle_Play->mBack.HeightF());
	mMainMenuButton.SetNotify(this);
	mMainMenuButton.SetSounds(&gSounds->mSelect);
	*this+=&mMainMenuButton;

	mTagsButton.Size(21,28);
	mTagsButton.CenterAt(16.5f,62.0f);
	mTagsButton.SetNotify(this);
	mTagsButton.SetSounds(&gSounds->mSelect);
	*this+=&mTagsButton;

	#define MAINBUTTONWIDTH 126.0f

	mMyLevelsButton.Size(MAINBUTTONWIDTH,31.0f);
	mMyLevelsButton.CenterAt(101.0f,61.0f);
	mMyLevelsButton.SetNotify(this);
	mMyLevelsButton.SetSounds(&gSounds->mSelect);
	mMyLevelsButton.NotifyOnPress();
	*this+=&mMyLevelsButton;

	mNewLevelsButton.Size(MAINBUTTONWIDTH,31.0f);
	mNewLevelsButton.CenterAt(101.0f+(MAINBUTTONWIDTH+5.0f),61.0f);
	if (gNarrowScreen) 
	{
		mNewLevelsButton.mWidth=77;
		mNewLevelsButton.CenterAt(101.0f+(MAINBUTTONWIDTH+5.0f)-35+5,61.0f);
	}
	mNewLevelsButton.SetNotify(this);
	mNewLevelsButton.SetSounds(&gSounds->mSelect);
	mNewLevelsButton.NotifyOnPress();
	*this+=&mNewLevelsButton;

	mHotLevelsButton.Size(MAINBUTTONWIDTH,31.0f);
	mHotLevelsButton.CenterAt(101.0f+(MAINBUTTONWIDTH+5.0f)+(MAINBUTTONWIDTH+5.0f),61.0f);
	if (gNarrowScreen) 
	{
		mHotLevelsButton.mWidth=77;
		mHotLevelsButton.CenterAt(101.0f+(MAINBUTTONWIDTH+5.0f)+(47.0f+4.0f),61.0f);
	}
	mHotLevelsButton.SetNotify(this);
	mHotLevelsButton.SetSounds(&gSounds->mSelect);
	mHotLevelsButton.NotifyOnPress();
	*this+=&mHotLevelsButton;

	float aX=(mWidth-31)+10-gNotched;
	mSearchButton.Size(29.0f,31.0f);
	mSearchButton.CenterAt(aX,61.0f);
	mSearchButton.SetNotify(this);
	mSearchButton.SetSounds(&gSounds->mSelect);
	mSearchButton.NotifyOnPress();
	*this+=&mSearchButton;

	aX-=(29+5);
	mStarLevelsButton.Size(29.0f,31.0f);
	mStarLevelsButton.CenterAt(aX,61.0f);
	mStarLevelsButton.SetNotify(this);
	mStarLevelsButton.SetSounds(&gSounds->mSelect);
	mStarLevelsButton.NotifyOnPress();
	*this+=&mStarLevelsButton;

	aX-=(29+5);
	mRandomLevelsButton.Size(29.0f,31.0f);
	mRandomLevelsButton.CenterAt(aX,61.0f);
	mRandomLevelsButton.SetNotify(this);
	mRandomLevelsButton.SetSounds(&gSounds->mSelect);
	mRandomLevelsButton.NotifyOnPress();
	*this+=&mRandomLevelsButton;

	aX-=(29+5);
	mPlayingLevelsButton.Size(29.0f,31.0f);
	mPlayingLevelsButton.CenterAt(aX,61.0f);
	mPlayingLevelsButton.SetNotify(this);
	mPlayingLevelsButton.SetSounds(&gSounds->mSelect);
	mPlayingLevelsButton.NotifyOnPress();
	*this+=&mPlayingLevelsButton;

	FixBottomOfScreen();	
}

void Makermall::FixBottomOfScreen()
{
	*this-=&mLevelupButton;
	*this-=&mMailButton;

	mSignInButton.Size(0,285,mWidth,mHeight-285);
	mSignInButton.SetNotify(this);
	mSignInButton.SetSounds(&gSounds->mSelect);
	*this+=&mSignInButton;

	float aX=10;
	if (mNotifyLevelup) 
	{
		mLevelupButton.Size(0,285,40,mHeight-285);
		mLevelupButton.SetNotify(this);
		mLevelupButton.SetSounds(&gSounds->mSelect);
		*this+=&mLevelupButton;
		aX+=40;
		mSignInButton.mX+=40;
	}
	if (mUnreadMsg) 
	{
		//	gBundle_Play->mYougotmail.DrawScaled(aX+17-5,295+13,.8f);
		mMailButton.Size(aX-5,285,35,mHeight-285);
		mMailButton.SetNotify(this);
		mMailButton.SetSounds(&gSounds->mSelect);
		*this+=&mMailButton;
	
		aX+=35;
		mSignInButton.mX+=35;
	}
}

void Makermall::Update()
{
	mStep+=.25f;
	if (mStep>256) mStep-=256;

	RComm::RCThreadLock();
	if (mPendingQueryList.GetCount()) UpdatePendingQueryList();
	RComm::RCThreadUnlock();

	if (mViewing!=mNextViewing)
	{
		mViewing=mNextViewing;
		SwapToNewView();
	}
}

void Makermall::Core_Draw()
{
	if (mInstaplay)
	{
		gG.Clear(0,0,.32f);
		return;
	}
	CPU::Core_Draw();
}

void Makermall::Draw()
{

	gG.PushTranslate();
	gG.Translate(0.0f,mStep-256);
	int aW=(int)(mWidth/256)+1;
	int aH=(int)(mHeight/256)+2;
	for (int aSpanX=0;aSpanX<aW;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<aH;aSpanY++)
		{
			DrawBKGTile(aSpanX*256.0f,aSpanY*256.0f);
			//gBundle_Tiles->mMoonbase.Draw(aSpanX*252.0f,aSpanY*252.0f);
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
	aP.mCorner[1]=Point(mWidth,0.0f);
	aP.mCorner[2]=Point(0,320);
	aP.mCorner[3]=Point(mWidth,320.0f);
	aC[0]=Color(aRed,aGreen,aBlue,0);
	aC[1]=Color(aRed,aGreen,aBlue,0);
	aC[2]=Color(aRed,aGreen,aBlue,.5f);
	aC[3]=Color(aRed,aGreen,aBlue,.5f);
	gG.FillQuad(aP,aC[0],aC[1],aC[2],aC[3]);
	gG.SetColor();

	gBundle_Play->mBack.Center(mMainMenuButton.Center()+Point(0,mMainMenuButton.IsDown()*2));
	gBundle_Play->mMM_Bar.DrawScaled(HalfWidth(),61,Point(300,1));
	gBundle_Play->mMM_Header.Center(HalfWidth(),(42/2));

	gBundle_Play->mMM_Tags.Center(mTagsButton.Center()+Point(0,2*mTagsButton.IsDown()));
	gG.PushClip();gG.Clip(mMyLevelsButton.Translate(Point(0,3*(mViewing==&mMyLevelsButton))));
	gBundle_Play->mMM_MyLevels.Center(mMyLevelsButton.Center()+Point(0,3*(mViewing==&mMyLevelsButton)));
	gG.PopClip();
	if (gNarrowScreen)
	{
		gBundle_Play->mMM_NewLevels_Small.Center(mNewLevelsButton.Center()+Point(0,3*(mViewing==&mNewLevelsButton)));
		gBundle_Play->mMM_HotLevels_Small.Center(mHotLevelsButton.Center()+Point(0,3*(mViewing==&mHotLevelsButton)));
	}
	else
	{
		gG.PushClip();gG.Clip(mNewLevelsButton.Translate(Point(0,3*(mViewing==&mNewLevelsButton))));
		gBundle_Play->mMM_NewLevels.Center(mNewLevelsButton.Center()+Point(0,3*(mViewing==&mNewLevelsButton)));
		gG.PopClip();

		gG.PushClip();gG.Clip(mHotLevelsButton.Translate(Point(0,3*(mViewing==&mHotLevelsButton))));
		gBundle_Play->mMM_HotLevels.Center(mHotLevelsButton.Center()+Point(0,3*(mViewing==&mHotLevelsButton)));
		gG.PopClip();
	}

	gBundle_Play->mMM_Search.Center(mSearchButton.Center()+Point(0,3*(mViewing==&mSearchButton)));
	gBundle_Play->mMM_Starred.Center(mStarLevelsButton.Center()+Point(0,3*(mViewing==&mStarLevelsButton)));
	gBundle_Play->mMM_Random.Center(mRandomLevelsButton.Center()+Point(0,3*(mViewing==&mRandomLevelsButton)));
	gBundle_Play->mMM_Playing.Center(mPlayingLevelsButton.Center()+Point(0,3*(mViewing==&mPlayingLevelsButton)));

	gG.RenderAdditive();
	if (mViewing==&mMyLevelsButton) 
	{
		gG.PushClip();
		gG.Clip(mMyLevelsButton.Translate(Point(0,3*(mViewing==&mMyLevelsButton))));
		gBundle_Play->mMM_MyLevels.Center(mMyLevelsButton.Center()+Point(0,3*(mViewing==&mMyLevelsButton)));
		gG.PopClip();
	}
	if (mViewing==&mNewLevelsButton) 
	{
		gG.PushClip();gG.Clip(mNewLevelsButton.Translate(Point(0,3*(mViewing==&mNewLevelsButton))));
		if (gNarrowScreen) gBundle_Play->mMM_NewLevels_Small.Center(mNewLevelsButton.Center()+Point(0,3*(mViewing==&mNewLevelsButton)));
		else gBundle_Play->mMM_NewLevels.Center(mNewLevelsButton.Center()+Point(0,3*(mViewing==&mNewLevelsButton)));
		gG.PopClip();
	}
	if (mViewing==&mHotLevelsButton) 
	{
		gG.PushClip();gG.Clip(mHotLevelsButton.Translate(Point(0,3*(mViewing==&mHotLevelsButton))));
		if (gNarrowScreen) gBundle_Play->mMM_HotLevels_Small.Center(mHotLevelsButton.Center()+Point(0,3*(mViewing==&mHotLevelsButton)));
		else gBundle_Play->mMM_HotLevels.Center(mHotLevelsButton.Center()+Point(0,3*(mViewing==&mHotLevelsButton)));
		gG.PopClip();
	}
	if (mViewing==&mPlayingLevelsButton) gBundle_Play->mMM_Playing.Center(mPlayingLevelsButton.Center()+Point(0,3*(mViewing==&mPlayingLevelsButton)));
	if (mViewing==&mRandomLevelsButton) gBundle_Play->mMM_Random.Center(mRandomLevelsButton.Center()+Point(0,3*(mViewing==&mRandomLevelsButton)));
	if (mViewing==&mStarLevelsButton) gBundle_Play->mMM_Starred.Center(mStarLevelsButton.Center()+Point(0,3*(mViewing==&mStarLevelsButton)));
	if (mViewing==&mSearchButton) gBundle_Play->mMM_Search.Center(mSearchButton.Center()+Point(0,3*(mViewing==&mSearchButton)));
	gG.RenderNormal();

	gBundle_Play->mMM_Bar.DrawScaled(HalfWidth(),317,Point(300,1));
	if (IsLoginValid())
	{
		float aX=10;
		if (mNotifyLevelup) 
		{
			gBundle_Play->mLevelup.Draw(aX-10,295);

			if ((gAppPtr->AppTime()/20)%2)
			{
				gG.RenderWhite(true);
				gG.SetColor(.75f);
				gBundle_Play->mLevelup.Draw(aX-10,295);
				gG.SetColor();
				gG.RenderWhite(false);
			}
			else gBundle_Play->mFlashLevelUp.Draw(aX-10,295-55);
			aX+=40;
		}
		if (mUnreadMsg) 
		{
			if ((gAppPtr->AppTime()/15)%2)
			{
				gBundle_Play->mYougotmail.DrawScaled(aX+17-5,295+13,.8f);
			}
			else
			{
				gG.SetColor(0,1,0);
				gBundle_Play->mYougotmail.DrawScaled(aX+17-5,295+13,1.0f);
				gG.SetColor();
			}
			aX+=35;
		}
		DRAWGLOWSMALL("WELCOME,",aX,314);
		aX+=75;
		String aWelcome=gAppPtr->mUserName.ToUpper();
		DRAWGLOW(aWelcome,aX,314);

		aX=mWidth-mKarmaWidth-32;
		gBundle_Play->mYourKarma.Draw(aX,mHeight-32);
		DRAWGLOWSMALL(mKarma,aX+22,mHeight-8);

		int aStarCount=_min(5,mStarCount);

		float aSX=aX-30;
		for (int aCount=0;aCount<aStarCount;aCount++)
		{
			gBundle_Play->mYourStars.Draw(aSX,mHeight-32);
			aSX-=8;
		}
		aSX=aX-30;
		for (int aCount=0;aCount<aStarCount;aCount++)
		{
			gBundle_Play->mYourStarsTop.Draw(aSX+7,mHeight-32+7);
			aSX-=8;
		}
		if (mStarCount>5) 
		{
			//gOut.Out("Seriously, WTF: %d",mStarCount);
			RIGHTGLOWSMALL(mStars,aSX+29,mHeight-8);
		}

		
	}
	else
	{
		CENTERGLOWSMALL("TOUCH HERE TO SIGN IN",HalfWidth(),312);
	}

}
void Makermall::Notify(void* theData)
{
	if (theData==&mMainMenuButton)
	{
		DisableTouch();
		Transition* aT=new Transition(this, new MainMenu);
	}
	if (theData==&mTagsButton) ShowFilterDialog();
	if (theData==&mMyLevelsButton) View(&mMyLevelsButton);
	if (theData==&mNewLevelsButton) View(&mNewLevelsButton);
	if (theData==&mHotLevelsButton) View(&mHotLevelsButton);
	if (theData==&mRandomLevelsButton) View(&mRandomLevelsButton);
	if (theData==&mStarLevelsButton) View(&mStarLevelsButton);
	if (theData==&mPlayingLevelsButton) View(&mPlayingLevelsButton);
	if (theData==&mSearchButton) 
	{
		mSearchName="";
		mSearchAuthor="";
		mSearchNew=true;
		View(&mSearchButton);
	}
	if (theData==&mSignInButton || theData==&mLevelupButton)
	{
		//
		// Okay, we need to open the profile page...
		// If we're not signed in, though, we just open a sign-in page...
		// If we're signed in, it's a SERVE situation, yes?
		//
		MMBrowser* aM=new MMBrowser;
		Rect aRect=Rect(0,0,mWidth,mHeight);
		aM->Size(aRect);
		aM->CenterAt(gG.Center()+Point(0,-5));
		aM->mMLBox.SetLinkNotify(aM);
		aM->mTitle="SIGN IN";

		if (IsLoginValid())
		{
			aM->mMLBox.GoPending("GETTING PROFILE...");
			SetNotifyMLBox(&aM->mMLBox);
			mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"main","",&aM->mMLBox);
			//IOBuffer aBuffer;
			//aBuffer.Load(PointAtDesktop("test.ml"));
			//aM->mData=aBuffer.ToString();
		}
		else
		{
			IOBuffer aBuffer;
			aBuffer.Load("data://signin.ml");
			aM->mData=aBuffer.ToString();
		}
		aM->Initialize();
		aM->mFirstUpdate=false;

		Closer* aC=new Closer;
		aC->mCloseWhom=aM;
		gAppPtr->AddCPU(aC);
		gAppPtr->AddCPU(aM);

		gApp.SetOnscreenKeyboardExtraSpace(50);				// Extra space for onscreen keyboard (so you can make sure extra buttons are visible)
	}
	if (theData==&mMailButton)
	{
		ShowMail();
	}
}

void Makermall::View(void* theViewing)
{
	mNextViewing=theViewing;
	String aSet="viewing";
	if (mNextViewing==&mMyLevelsButton) gApp.mSettings.SetString(aSet,"mine");
	if (mNextViewing==&mNewLevelsButton) gApp.mSettings.SetString(aSet,"new");
	if (mNextViewing==&mHotLevelsButton) gApp.mSettings.SetString(aSet,"hot");
	if (mNextViewing==&mPlayingLevelsButton) gApp.mSettings.SetString(aSet,"playing");
	if (mNextViewing==&mStarLevelsButton) gApp.mSettings.SetString(aSet,"new");
	if (mNextViewing==&mRandomLevelsButton) gApp.mSettings.SetString(aSet,"new");
	if (mNextViewing==&mSearchButton) 
	{
		mSearchAgain=false;
		gApp.mSettings.SetString(aSet,"new");
	}
}

void Makermall::SwapToNewView()
{
	if (mListWidget) mListWidget->Kill();

	mListWidget=new ListWidget();
	mListWidget->Size(0,93,Width()+1,193);
	*this+=mListWidget;

	mPage=0;
	if (mViewing==&mMyLevelsButton)
	{
		RComm::RQuery aQ=RComm::Custom(BuildLocalLevels,NULL);
		mPendingQueryList+=aQ;
	}

	longlong aFilter=FixFilter(mFilter);
	if (mViewing==&mPlayingLevelsButton)
	{
		RComm::RQuery aQ=RComm::Custom(BuildPlayingLevels,NULL);
		mPendingQueryList+=aQ;
	}
	if (mViewing==&mNewLevelsButton)
	{
		RComm::RQuery aQ=RComm::LevelSharing_GetNewLevels(mNow,mPage,LEVELFETCHCOUNT,aFilter);
		mPendingQueryList+=aQ;
	}
	if (mViewing==&mHotLevelsButton)
	{
		RComm::RQuery aQ=RComm::LevelSharing_GetHotLevels(mPage,LEVELFETCHCOUNT,aFilter);
		mPendingQueryList+=aQ;
	}
	if (mViewing==&mRandomLevelsButton)
	{
		RComm::RQuery aQ=RComm::LevelSharing_GetRandomLevels(mPage,1,aFilter);
		mPendingQueryList+=aQ;
	}
	if (mViewing==&mStarLevelsButton)
	{
		RComm::RQuery aQ=RComm::LevelSharing_GetStarLevels(mPage,LEVELFETCHCOUNT,aFilter);
		mPendingQueryList+=aQ;
	}
	if (mViewing==&mSearchButton)
	{
		if (mSearchAgain)
		{
			RComm::RQuery aQ=RComm::LevelSharing_SearchLevels(gMakermall->mSearchName,gMakermall->mSearchAuthor,!gMakermall->mSearchNew,gMakermall->mPage,LEVELFETCHCOUNT,aFilter);
			mPendingQueryList+=aQ;
		}
		else
		{
			mLocalLevels.Reset();
			SearchWidget* aLW=new SearchWidget;
			mListWidget->AddCPU(aLW);
			mListWidget->Rehup();
		}
	}
}

ListWidget::ListWidget()
{
	mHasLevels=false;
}

ListWidget::~ListWidget()
{
}

void ListWidget::Draw()
{
	if (!mCPUManager)
	{
		gG.RenderAdditive();
		gBundle_Play->mLoading.DrawRotated(gG.HalfWidthF(),HalfHeight(),gAppPtr->AppTimeF()*4);
		gG.RenderNormal();
	}

	if (mMessage.Len())
	{
		CENTERGLOW(mMessage,HalfWidth(),HalfHeight());
	}
}

void ListWidget::Rehup()
{
	if (!mCPUManager) mCPUManager=new CPUManager;
	BindSpinsV();

	float aY=0;
	if (mCPUManager)
	{
		aY=10;
		EnumList(Widget,aCPU,*mCPUManager)
		{
			if (aCPU->mKill) continue;
			aCPU->mY=aY-GetPos().mY;
			aCPU->mX=(mWidth/2)-(aCPU->mWidth/2);
			int aMoveX=(QuickRand((int)aCPU->mName.GetHash())%6)-3;
			aCPU->mX+=aMoveX;//gRand.GetSpanF(-3,3);
			aY+=aCPU->mHeight+5;
		}
		aY+=5;
	}
	SetSwipeHeight(aY);
}

void Makermall::FillTags(String theString)
{
	if (!theString.Len()) return;

	mTags.Reset();
	String aToken=theString.GetToken('|');
	while (aToken.Len()) 
	{
		_DLOG("Got Tag: %s",aToken.c());
		mTags+=aToken;
		aToken=theString.GetNextToken('|');
	}

	if (gApp.mSettings.GetString("TAGS")!=theString)
	{
		gApp.mSettings.SetString("TAGS",theString);
		gApp.SaveSettings();
	}
}

void Makermall::UpdatePendingQueryList()
{
	EnumSmartList(RComm::RaptisoftQuery,aRQ,mPendingQueryList)
	{
		if (aRQ->IsDone())
		{
			bool aNeverMind=false;
			if (aRQ->mFunctionPointer==&RComm::Custom && mViewing!=&mMyLevelsButton && mViewing!=&mPlayingLevelsButton) aNeverMind=true;
			if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetNewLevels && mViewing!=&mNewLevelsButton) aNeverMind=true;
			if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetHotLevels && mViewing!=&mHotLevelsButton) aNeverMind=true;
			if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetRandomLevels && mViewing!=&mRandomLevelsButton) aNeverMind=true;
			if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetStarLevels && mViewing!=&mStarLevelsButton) aNeverMind=true;
			if (aRQ->mFunctionPointer==&RComm::LevelSharing_SearchLevels && mViewing!=&mSearchButton) aNeverMind=true;

			if (!aNeverMind)
			{
				if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetTags)
				{
					String aResult=aRQ->GetResult().ToString();
					String aGotTags=aResult.GetRML("TAGS");
					gAppPtr->mSettings.SetString("TAGS",aGotTags);
					gAppPtr->SaveSettings();
					
					FillTags(aGotTags);
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_Serve)
				{
					if (aRQ->mExtraData==GetNotifyMLBox())
					{
						MLBox* aBox=GetNotifyMLBox();
						aBox->LoadFromString(aRQ->mResult.ToString());
					}
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_Boost)
				{
					String aResult=aRQ->GetResult().ToString();
					String aTag;
					String aValue;

					String aMessage="";
					while (aResult.ParseRML(aTag,aValue)) if (aTag=="BOOST") aMessage=aValue;
					if (aMessage.Len()) {MsgBox* aBox=new MsgBox;aBox->GoNoThrottle(aMessage,gGCenter(),0);}
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_Hello)
				{
					SetFeatures(gOldFeatures);
					gValidLogin=false;
					String aResult=aRQ->GetResult().ToString();

					String aV=aResult.GetRML("AVATARVERSION");
					if (aV!=gApp.mAvatarVersion)
					{
						gApp.mAvatarVersion=aV;
						String aFolder="temp://avatars\\";
						DeleteDirectory(aFolder);
						MakeDirectory(aFolder);
						UnloadAvatars();
					}

					if (aResult.GetRML("RESULT")=="OK") 
					{
						gValidLogin=true;
						FillProfileInfo(aResult.GetRML("PROFILE"));

						if (IsLoginValid() && mLocalLevels.Size()) gMakermall->GetLocalLevelStats(mLocalLevels);
						MyMLBox* aBox=(GetNotifyMLBox());
						if (aBox)
						{
							gSounds->mBell.Play();
							if (aBox->GetDefine("#resume_to")=="comments")
							{
								RComm::RQuery aRQ=RComm::LevelSharing_GetComments(mPlayingID,aBox);
								gMakermall->mPendingQueryList+=aRQ;
								aBox->GoPending("LOADING COMMENTS...");
							}
							if (aBox->GetDefine("#resume_to")=="share")
							{
								aBox->LoadAsString("<setup borders=50 caption=\"LEVEL UP!\"><BKGCOLOR #004770><center>YOU CAN SHARE YOUR LEVEL NOW!");
							}
							else 
							{
								aBox->GoPending("GETTING PROFILE...");
								mPendingQueryList+=RComm::LevelSharing_Serve(gAppPtr->mUserName,gAppPtr->mUserPassword,"main","",aBox);
							}
							
						}
					}
					else
					{
						if (aResult.GetRML("FAILREASON")=="BADVERSION")
						{
							BadVersion();
							return;
						}
						else if (aResult.GetRML("FAILREASON")=="BADLOGIN")
						{
							MyMLBox* aBox=(GetNotifyMLBox());
							if (aBox)
							{

								IOBuffer aBuffer;
								aBuffer.Load("data://signinfail.ml");
								gSounds->mDie.Play();
								aBox->LoadFromString(aBuffer.ToString());
							}
						}
						else if (aResult.GetRML("FAILREASON")=="NOTACTIVE")
						{
							MyMLBox* aBox=(GetNotifyMLBox());
							if (aBox)
							{

								IOBuffer aBuffer;
								aBuffer.Load("data://signinnotactive.ml");
								gSounds->mDie.Play();
								aBox->LoadFromString(aBuffer.ToString());
							}
						}
						else gOut.Out("Fail Reason: %s",aResult.GetRML("FAILREASON").c());
					}
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_Comment)
				{
					MyMLBox* aBox=GetNotifyMLBox();
					if (aBox==aRQ->mExtraData)
					{
						RComm::RQuery aRQ=RComm::LevelSharing_GetComments(mPlayingID,aBox);
						gMakermall->mPendingQueryList+=aRQ;
					}
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetComments)
				{

					MyMLBox* aBox=GetNotifyMLBox();
					if (aBox==aRQ->mExtraData)
					{
						aBox->Define("#resume_to","comments");
						String aMoreComments;
						
						if (aRQ->IsRML())
						{
							bool aFirst=true;
							String aResults=aRQ->GetResult().ToString();
							String aTag,aValue;
							int aToggle=1;
							while (aResults.ParseRML(aTag,aValue))
							{
								if (aTag=="COMMENT")
								{
									if (aFirst)
									{
										aMoreComments+="<confine 25 #width-65>";
										aFirst=false;
									}

									String aComment=aValue.GetRML("TEXT");
									//aComment="FUCK YOU YOU FUCKING ASSHOLE!"; //Test swear filter
									String aText=PFilter(gBundle_Play->mFont_Commodore64Angled11,aComment,false);
									String aAuthor=aValue.GetRML("AUTHOR");

									if (aToggle%2) 
									{
										aMoreComments+=Sprintf("<record _cb%d>",aToggle);
									}
									aMoreComments+="<BR>";
									aMoreComments+="<font normal>";
									aMoreComments+=aText;
									aMoreComments+="<font small><BR><right><BR>- ";
									aMoreComments+=aAuthor;
									aMoreComments+=Sprintf("<SP %d>",gRand.Get(20));
									if (aToggle%2)
									{
										aMoreComments+="</record>";
										aMoreComments+=Sprintf("<customfromrecord _cb%d>",aToggle);
									}
									aMoreComments+="<br> <font normal>";
									aMoreComments+="<BR><left>";
									if (gMakermall->mAdmin) aMoreComments+="</LINK>";


									aToggle++;
								}
							}
						}


						String aComments="<SETUP hborders=20><BKGCOLOR #004770><BR>";
						String aStr;
						if (IsLoginValid()) aStr="ADD A COMMENT";
						else aStr="SIGN IN TO ADD COMMENTS";
						aComments+=Sprintf("<link cmd=addcomment><custom #width-40,40 x:\"%s\"></link><BR>",aStr.c());
						aComments+=aMoreComments;
						aComments+="<BR><BR>";

						aBox->LoadFromString(aComments);
					}
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_DeleteLevel)
				{
					gSounds->mModem.Stop();
					TransferPacket* aTP=(TransferPacket*)aRQ->mExtraData;
					aRQ->mExtraData=NULL;

					if (aTP->mDialog==gTransDialog)
					{
						String aRString=aRQ->mResult.ToString();
						_DLOG("Un-Share Result: [%s]",aRString.c());
						String aResult=aRString.GetRML("RESULT");
						if (aResult=="OK")
						{
							gSounds->mBell.Play();
							aTP->mDialog->SetStatus("YOUR LEVEL HAS BEEN REMOVED FROM MAKERMALL!");
							gWorldEditor->mUploadID=-1;
						}
						else
						{
							gSounds->mDie.Play();
							aTP->mDialog->SetStatus("SOMETHING WENT WRONG!  YOUR LEVEL HAS NOT BEEN UN-SHARED!  PLEASE TRY AGAIN IN A FEW MINUTES!");
						}
						aTP->mDialog->mQuery=NULL;
						aTP->mDialog->mDone=true;
					}
					delete aTP;
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_UploadLevel)
				{
					gSounds->mModem.Stop();

					TransferPacket* aTP=(TransferPacket*)aRQ->mExtraData;
					aRQ->mExtraData=NULL;

					if (aTP->mDialog==gTransDialog)
					{
						String aRString=aRQ->mResult.ToString();
						String aResult=aRString.GetRML("RESULT");
						if (aResult=="OK")
						{
							gSounds->mBell.Play();
							String aStatus=aRString.GetRML("STATUS");
							if (aStatus=="UPDATE")
							{
								aTP->mDialog->SetStatus("YOUR LEVEL HAS BEEN UPDATED ON MAKERMALL!  YOU CAN SEE HOW YOUR LEVEL IS DOING ON THE 'MY LEVELS' TAB!");
							}
							else
							{
								aTP->mDialog->SetStatus("YOUR LEVEL HAS BEEN SHARED ON MAKERMALL!  YOU CAN SEE HOW YOUR LEVEL IS DOING ON THE 'MY LEVELS' TAB!");
							}
							if (gWorldEditor) 
							{
								gWorldEditor->mUploadID=aRString.GetRML("LEVELID").ToInt();
								gWorldEditor->QuickSave();
							}
						}
						else
						{
							gSounds->mDie.Play();
							String aFailReason=aRString.GetRML("FAILREASON");
							if (aFailReason.Len())
							{
								if (aFailReason=="BADCHARS") aFailReason="YOU HAVE INVALID CHARACTERS IN YOUR LEVEL NAME!";
								else if (aFailReason=="SHAREFAIL" || aFailReason=="UPDATEFAIL") aFailReason="SOMETHING WENT WRONG ON THE SERVER, PLEASE TRY AGAIN IN A FEW MINUTES!";
								String aMessage="OOPS!  YOUR LEVEL HAS NOT BEEN SHARED YET!  ";
								aMessage+=aFailReason.ToUpper();
								aTP->mDialog->SetStatus(aMessage);
							}
							else aTP->mDialog->SetStatus("SOMETHING WENT WRONG!  YOUR LEVEL HAS NOT BEEN SHARED ON MAKERMALL!  PLEASE TRY AGAIN IN A FEW MINUTES!");
						}

						aTP->mDialog->mQuery=NULL;
						aTP->mDialog->mDone=true;
					}

					delete aTP;
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_Comment)
				{
					_LOG("Made a new comment (refresh comments!)...");
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_GiveStar)
				{
					String aResult=aRQ->GetResult().ToString();
					String aTag;
					String aValue;
					while (aResult.ParseRML(aTag,aValue))
					{ 
						if (aTag=="MOREINFO")
						{
							if (aValue=="STAROWNLEVEL")
							{
								AddMessage("Hey, it looks like you tried to give a star to your own level!  Pretty Sneaky!  This isn't allowed, so your star has been returned to you.");
							}
							if (aValue=="MAXSTARS")
							{
								AddMessage("You gave a star to a level that already has three stars!  That's generous, but three's the max, so your star has been returned to you.");
							}
						}
					}
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_DownloadLevel)
				{
					gSounds->mModem.Stop();
					if (gTransDialog) gTransDialog->Kill();

					TransferPacket* aTP=(TransferPacket*) aRQ->mExtraData;
					int aLevelID=aTP->mUploadID;
					delete aTP;

					if (aRQ->IsRML())
					{
						MsgBox *aBox=new MsgBox();
						aBox->GoNoThrottle("THIS LEVEL HAS BEEN REMOVED FROM MAKERMALL.  IT WILL BE REMOVED FROM YOUR RECENTS LIST.",gGCenter(),0);
						RemoveFromRecentList(aLevelID);
					}
					else
					{
						aRQ->mResult.CommitFile("sandbox://downloaded.kitty");
						gTempSaveName=Sprintf("MMRESUME-%d.sav",aLevelID);

						gSounds->mBell.PlayPitched(.8f);
						//
						// Does savegame exist?  If so we ask about continue...
						//
						String aSaveGame="sandbox://";aSaveGame+=gTempSaveName;
						bool aContinued=false;
						if (DoesFileExist(aSaveGame))
						{
							mPendingQueryList-=aRQ;
							MsgBox *aBox=new MsgBox();
							aBox->GoX("YOU PLAYED THIS LEVEL EARLIER.\n\nDO YOU WANT TO CONTINUE?",gGCenter(),1,MBHOOK(
								{
									if (theResult!="YES") DeleteFile(aSaveGame);
									else RComm::LevelSharing_PlayedLevel(aLevelID);
									StartPlaying(aLevelID);
								}
							));
						}
						else
						{
							if (!aContinued) RComm::LevelSharing_PlayedLevel(aLevelID);
							StartPlaying(aLevelID);
						}
					}
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetLocalStats)
				{
					if (mViewing==&mMyLevelsButton) FillLocalStats(aRQ->GetResult().ToString());
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetLevelStats)
				{
					if (mViewing==&mPlayingLevelsButton) FillLevelStats(aRQ->GetResult().ToString());
				}
				else if (aRQ->mFunctionPointer==&RComm::LevelSharing_GetNewLevels || aRQ->mFunctionPointer==&RComm::LevelSharing_GetHotLevels ||
						 aRQ->mFunctionPointer==&RComm::LevelSharing_GetRandomLevels || aRQ->mFunctionPointer==&RComm::LevelSharing_GetStarLevels ||
						 aRQ->mFunctionPointer==&RComm::Custom || aRQ->mFunctionPointer==&RComm::LevelSharing_SearchLevels)
				{
					String aRML=aRQ->GetResult().ToString();
					if (aRML.GetRML("RESULT")=="OK")
					{
						int aStartCursor=aRML.GetRML("STARTCURSOR").ToInt();
						mPage=aRML.GetRML("ENDCURSOR").ToInt();

						String aNow=aRML.GetRML("NOW");
						if (aNow.Len()) 
						{
							mSavedNow=aNow;
							gAppPtr->mSettings.SetString(Sprintf("NewDate_%s",gAppPtr->mUserName.c()),mSavedNow);
							gAppPtr->SaveSettings();

							_DLOG("New Now (Temporary): %s",mSavedNow.c());
						}

						String aLevels=aRML.GetRML("LEVELS");
						BuildLevels(aLevels);

						if (aStartCursor!=mPage) 
						{
							// Add a fetch more widget...
							Widget* aW=new Widget;
							aW->Load("[COMMAND]MORE[/COMMAND][NAME]SHOW MORE[/NAME]");
							aW->mCleanMe=true;
							mListWidget->AddCPU(aW);
						}
						mListWidget->Rehup();
						if (aStartCursor>0) 
						{
							mListWidget->mSwipeSpeed=Point(0,5);
						}
					}
				}
			}

			mPendingQueryList-=aRQ;
			EnumSmartListRewind(RComm::RaptisoftQuery);
		}
	}
}

void Makermall::StartPlaying(int theLevelID)
{
	//DoParentalGate(false,CPUHOOK(
		{
			gSounds->mEvaporate.PlayPitched(1.5f);
			gSounds->mEvaporate.PlayPitched(.75f);

			if (1)
			//if (gApp.IsAdult()) // For putting app into pure child mode...
			{
				CPU* aGame=gApp.GoNewGame("sandbox://downloaded.kitty");
				Transition* aT=new Transition(NULL,aGame);
			}
			else
			{
				gApp.mIsKittyConnect=false;
				MainMenu *aMM=new MainMenu();
				Transition* aT=new Transition(gMakermall,aMM);
			}
		}
	//));

	mPlayingID=theLevelID;
}

void Makermall::BuildLevels(String theLevelRML)
{
	if (mListWidget && mListWidget->mCPUManager) EnumList(Widget,aW,*mListWidget->mCPUManager) if (aW->mCleanMe) aW->Kill();

	String aTag,aValue;
	String aInfoTag;
	String aInfoValue;
	Color mColor=IColor(255,251,142);

	mLocalLevels.Reset();

	while (theLevelRML.ParseRML(aTag,aValue))
	{
		if (aTag=="LEVEL" && aValue.Len())
		{
			Widget* aLW=new Widget;
			aLW->Load(aValue);

			if (aLW->mLocal) mLocalLevels+=aLW->mID;
			mListWidget->AddCPU(aLW);
			mListWidget->mHasLevels=true;
		}
	}

	if (!mListWidget->mHasLevels) 
	{
		if (mViewing==&mNewLevelsButton) mListWidget->mMessage="NO NEW LEVELS RIGHT NOW!";
		if (mViewing==&mHotLevelsButton) mListWidget->mMessage="NO HOT LEVELS RIGHT NOW!";
		if (mViewing==&mSearchButton) mListWidget->mMessage="NO MATCHING LEVELS FOUND!";
		if (mViewing==&mStarLevelsButton) mListWidget->mMessage="NO STAR LEVELS FOUND!";
		if (mViewing==&mRandomLevelsButton) mListWidget->mMessage="NO LEVELS FOUND!";
	}
	else if (mViewing==&mMyLevelsButton)
	{
		if (IsLoginValid() && mLocalLevels.Size()) gMakermall->GetLocalLevelStats(mLocalLevels);
	}
	else
	{
		if (mViewing==&mSearchButton)
		{
			Widget* aW=new Widget;
			aW->Load("[COMMAND]REFINE[/COMMAND][NAME]REFINE SEARCH[/NAME]");
			mListWidget->mCPUManager->Insert(aW);
		}

		if (mFilter!=0)
		{
			FilterWidget* aLW=new FilterWidget;
			mListWidget->mCPUManager->Insert(aLW);
		}
	}

}

Widget::Widget()
{
	mIsDown=false;
	mAnimate=false;
	mCleanMe=false;
	mLocal=false;
	mNoDying=false;
	mClientVersion=0;
	mStarCount=0;
	mID=-1;
	mFlag=0;

	mCommentButton.Size(-1,-1,-1,-1);
}

Widget::~Widget()
{
}

void Widget::Load(String theRML)
{
	mType=0;
	mRML=theRML;

	mColor=IColor(157,194,186);

	String aCommand=theRML.GetRML("COMMAND");
	String aName=theRML.GetRML("NAME");
	if (aCommand.Len())
	{
		mCommand=aCommand;
		mName=aName;
		Size(0,0,mWidth-100,31);
		mCleanMe=true;
	}
	else
	{
		if (aName.Len())
		{
			mLocal=theRML.GetRML("LOCAL").ToBool();

			//mName=aName;
			mName=PFilter(gBundle_Play->mFont_Commodore64Angled11,aName,false);


			mAuthor=theRML.GetRML("AUTHOR").ToUpper();
			mAuthorLevel=theRML.GetRML("AUTHORLEVEL");
			mAuthorLevelColor=_min(9,mAuthorLevel.ToInt()/5);
			mAuthorLevelBound=gBundle_Play->mFont_Tiny_Outline.GetBoundingRect(mAuthorLevel,Point(3,12));

			mFlag=GetFlagIcon(theRML.GetRML("COUNTRY"));
			mID=theRML.GetRML("ID").ToInt();

			//if (mLocal && mID>=0) aLocalLevels+=mID;

			mRating=theRML.GetRML("RATING");
			longlong aTags=theRML.GetRML("TAGS").ToLongLong();
			if (aTags&NOKILLBIT)
			{
				//gOut.Out("Got no kill bit!");
				aTags^=NOKILLBIT;
				mNoDying=true;
			}
			mSourceTags=aTags;
			mTried=theRML.GetRML("PLAYCOUNT");
			mWon=theRML.GetRML("WINCOUNT");
			mClientVersion=theRML.GetRML("CLIENTVERSION").ToFloat();
			mStarCount=theRML.GetRML("STARS").ToInt();
			int aBoost=theRML.GetRML("MISCFLAG").ToInt();
			mCommentCount=theRML.GetRML("COMMENTS");
			mAvatar=GetAvatar(theRML.GetRML("AVATAR"),true);

			float aRand=QuickRandFraction((int)aName.GetHash());
			mColor=mColor.Darken(aRand*.25f);

			int aTMask=1;
			float aX=116+164+10+25;
			gBundle_Play->mFont_Tiny.SetExtraSpacing(-1);
			for (int aCount=0;aCount<gMakermall->mTags.Size();aCount++)
			{
				if (aTags&aTMask)
				{
					mTags+=gMakermall->mTags[aCount];
					mTagWidth+=gBundle_Play->mFont_Tiny.Width(gMakermall->mTags[aCount]);
				}
				aTMask*=2;
			}
			gBundle_Play->mFont_Tiny.SetExtraSpacing(0);

			Size(0,0,mWidth-100,58);
			if (mID<0) mHeight=31;

		}
	}
}

void Widget::Initialize()
{
	if (mID>=0)
	{
		mCommentButton.Size(mWidth-58,0,58,mHeight);
		mCommentButton.SetNotify(this);
		mCommentButton.SetSounds(&gSounds->mClick);
		*this+=&mCommentButton;
	}
}

void OpenComments(int theLevelID)
{
	if (!gMakermall) return;

	gMakermall->mPlayingID=theLevelID;
	MMBrowser* aM=new MMBrowser;
	Rect aRect=Rect(0,0,gMakermall->mWidth,gMakermall->mHeight);
	aM->Size(aRect);
	aM->CenterAt(gG.Center()+Point(0,-5));
	aM->mMLBox.SetLinkNotify(aM);
	aM->mTitle="COMMENTS";
	aM->mMLBox.GoPending("LOADING COMMENTS...");
	aM->mMLBox.BindSpinsV();
	Color aC;
	aC.FromWeb("#004770");
	aM->mMLBox.mBKGColor=aC;
	aM->mMLBox.Define("#playingID",Sprintf("%d",theLevelID));
	
	aM->Initialize();
	aM->mFirstUpdate=false;
	SetNotifyMLBox(&aM->mMLBox);	// The comment callback will hit this...

	//
	// Get the comments, and remember the MLBox!
	//
	RComm::RQuery aRQ=RComm::LevelSharing_GetComments(theLevelID,&aM->mMLBox);
	gMakermall->mPendingQueryList+=aRQ;

	gApp.AddCPU(aM);
}

void Widget::Notify(void* theData)
{
	if (theData==&mCommentButton) OpenComments(mID);
}

void Widget::Draw()
{
	Color aColor=mColor;
	if (mIsDown) aColor=mColor.Darken(.25f);
	if (mCommand.Len())
	{
		gG.SetColor(aColor);
		gG.FillRect(0,0,mWidth,mHeight);
		gG.SetColor();
		String aName=mName;
		if (mAnimate) aName.Append(".",((gAppPtr->AppTime()/10)%4));
		gBundle_Play->mFont_Commodore64Angled11.Draw(aName,10,mHeight-8);
	}
	else
	{
		float aEdge=117;
		if (mLocal) 
		{
			aEdge=70;
			if (mID<0) aEdge=31;
		}

		gG.SetColor(aColor);
		gG.FillRect(0,0,mWidth,mHeight);
		if (mID>=0)
		{
			gG.SetColor(aColor.Darken(.5f));
			gG.FillRect(aEdge+25+5,mHeight-6-25,154.0f-5-15,26.0f);
		}
		gG.SetColor(aColor.Darken(.15f));
		gG.FillRect(0,0,aEdge,mHeight);
		if (!mLocal) 
		{
			// Author Icon Area
			gG.SetColor();
			if (mAvatar) mAvatar->mSprite->Draw(3,3);


			gG.PushTranslate();
			gG.Translate(1,1);
			gG.SetColor(0,0,0,1);
			gG.FillRect(mAuthorLevelBound.Translate(0,1).ExpandRight(3).ExpandDown(2));
			gG.SetColor(aColor.Darken(.15f));
			gG.FillRect(mAuthorLevelBound.Translate(0,1).Expand(1));
			gG.SetColor(0,0,0,1);
			gBundle_Play->mFont_Tiny_Outline.Draw(mAuthorLevel,4,12);
			gG.SetColor();
			gBundle_Play->mFont_Tiny.Draw(mAuthorLevel,4,12);
			gG.PopTranslate();
		}
		else
		{
			gG.SetColor(.75f);
			if (mID>=0) gBundle_Play->mMm_Edit.Center(aEdge/2,mHeight/2);
			else gBundle_Play->mMm_Pencil.Center(aEdge/2,mHeight/2);
		}

		gG.SetColor();
		if (mID>=0)
		{
			gBundle_Play->mMM_Heart.Center(aEdge+25+2,mHeight-7-(12));
			if (mRating.Len()==0)
			{
				gBundle_Play->mFont_Commodore64Angled11.SetExtraSpacing(-8);
				if (mID==-1) gBundle_Play->mFont_Commodore64Angled11.Center("...",aEdge+25+2-3,mHeight-7-(5)-5);
				else
				{
					String aDotString;
					aDotString.Append(".",(gAppPtr->AppTime()/10)%4);
					gBundle_Play->mFont_Commodore64Angled11.Draw(aDotString,aEdge+25+2-3-10,mHeight-7-(5)-5);
				}
				gBundle_Play->mFont_Commodore64Angled11.SetExtraSpacing(0);
			}
			else
			{
				gG.SetColor(0,0,0,.5f);
				gBundle_Play->mFont_Commodore64Angled11OutlineHard.Draw(mRating,aEdge+25+2,mHeight-7-(5));
				gG.SetColor();
				gBundle_Play->mFont_Commodore64Angled11.Draw(mRating,aEdge+25+2,mHeight-7-(5));
			}

			gG.SetColor(aColor.Darken(.075f));
			gG.FillRect(mWidth-58,0,58,mHeight);
			gG.SetColor(0,0,0,1);
			gBundle_Play->mMM_Comment.Center(mWidth-29+3,20+3);
			gG.SetColor();
			gG.PushTranslate();
			float aMod=(3.0f*mCommentButton.IsDown());
			gG.Translate(aMod,aMod);
			gBundle_Play->mMM_Comment.Center(mWidth-29,20);
			gG.SetColor(0,0,0,1);
			if (mCommentCount.Len()==0)
			{
				gBundle_Play->mFont_Commodore64Angled11.SetExtraSpacing(-8);
				if (mID==-1) gBundle_Play->mFont_Commodore64Angled11.Center("...",mWidth-33,20);
				else
				{
					String aDotString;
					aDotString.Append(".",(gAppPtr->AppTime()/10)%4);
					gBundle_Play->mFont_Commodore64Angled11.Draw(aDotString,mWidth-33-8,20);
				}
				gBundle_Play->mFont_Commodore64Angled11.SetExtraSpacing(0);
			}
			else gBundle_Play->mFont_Commodore64Angled8.Center(mCommentCount,mWidth-28,21);
			gG.PopTranslate();
			{
				gG.SetColor(0,0,0,.25f);
				if (mStarCount>1) gG.SetColor(255.0f/255.0f,220.0f/255.0f,0.0f);
				gBundle_Play->mMM_Star.Center(mWidth-29,46);
			}
			{
				gG.SetColor(0,0,0,.25f);
				if (mStarCount>0) gG.SetColor(255.0f/255.0f,220.0f/255.0f,0.0f);
				gBundle_Play->mMM_Star.Center(mWidth-29-14,42);
			}
			{
				gG.SetColor(0,0,0,.25f);
				if (mStarCount>2) gG.SetColor(255.0f/255.0f,220.0f/255.0f,0.0f);
				gBundle_Play->mMM_Star.Center(mWidth-29+14,42);
			}
			gG.SetColor();
		}
	
		gG.PushClip();
		gG.Clip(0,0,mWidth-(58+2),mHeight);
		gBundle_Play->mFont_Commodore64Angled11.SetExtraSpacing(-2);
		float aNameIndent=7;
		if (mNoDying) 
		{
			gBundle_Play->m1up.Draw(aEdge+5,8);
			aNameIndent+=25;
		}
		gBundle_Play->mFont_Commodore64Angled11.Draw(mName,aEdge+aNameIndent,20);
		gBundle_Play->mFont_Commodore64Angled11.SetExtraSpacing(0);

		if (mID>=0)
		{
			float aTagX=aEdge+164+10+20-10-15;
			gG.SetColor(0,0,0,.1f);
			gBundle_Play->mFont_Tiny_Outline.Draw("#TAGS",aTagX,38);
			gG.SetColor();
			gBundle_Play->mFont_Tiny.Draw("#TAGS",aTagX,38);

			gBundle_Play->mFont_Tiny.SetExtraSpacing(-1);
			for (int aCount=0;aCount<mTags.Size();aCount++)
			{
				gG.SetColor(aColor.Darken(.25f));
				gG.FillRect(aTagX-2,38+12-10,mTagWidth[aCount]+3,12);
				gG.SetColor(.65f);
				gBundle_Play->mFont_Tiny.Draw(mTags[aCount],aTagX,38+12);
				aTagX+=mTagWidth[aCount]+5;
			}
			gBundle_Play->mFont_Tiny.SetExtraSpacing(0);
		}

		if (!mLocal)
		{
			// Author stuff...
			gG.Clip(0,0,116-3,mHeight);
			gG.SetColor(0,0,0,.25f);
			gBundle_Play->mFont_Tiny_Outline.Draw(mAuthor,16+3+3,mHeight-3);
			gG.SetColor();
			gBundle_Play->mFont_Tiny.Draw(mAuthor,16+3+3,mHeight-3);
		}
		gG.PopClip();
		if (!mLocal)
		{
			gBundle_Play->mFlag[mFlag].Draw(3,mHeight-3-11+2);
		}

		if (mID>=0)
		{
			gG.PushTranslate();
			String aTried=mTried;
			String aWon=mWon;

			if (aTried.Len()==0)
			{
				gG.Translate(-5,-1);
				{
					aTried.Append(".",(gAppPtr->AppTime()/10)%4);
					aWon.Append(".",(gAppPtr->AppTime()/10)%4);
					aTried+=" ";
				}
			}

			if (aTried.Len())
			{
				gG.SetColor(0,0,0,.5f);
				gG.Translate(-5,-1);
				gBundle_Play->mFont_Tiny_Outline.Right("TRIED:",aEdge+25+120-15,40);
				gBundle_Play->mFont_Tiny_Outline.Right("WON:",aEdge+25+120-15,40+10);
				gBundle_Play->mFont_Tiny_Outline.Draw(aTried,aEdge+25+120+5-15,40);
				gBundle_Play->mFont_Tiny_Outline.Draw(aWon,aEdge+25+120+5-15,40+10);
				gG.SetColor();
				gBundle_Play->mFont_Tiny.Right("TRIED:",aEdge+25+120-15,40);
				gBundle_Play->mFont_Tiny.Right("WON:",aEdge+25+120-15,40+10);
				gBundle_Play->mFont_Tiny.Draw(aTried,aEdge+25+120+5-15,40);
				gBundle_Play->mFont_Tiny.Draw(aWon,aEdge+25+120+5-15,40+10);
			}
			gG.PopTranslate();
		}
	}
}


void Widget::TouchMove(int x, int y)
{
	int aSize=1;
	if (mCommand.Len()) aSize=8;
	if (mIsDown && (gMath.Abs(gAppPtr->mMessageData_TouchPosition.mY-gAppPtr->mMessageData_TouchStartPosition.mY)>aSize))
	{
		mIsDown=false;
		Point aHoldPos=gAppPtr->mMessageData_TouchPosition;
		gAppPtr->mMessageData_TouchPosition=Point(-9999,-9999);
		//gAppPtr->ForceTouchEnd(true);
		gAppPtr->ForceTouchEnd(false);
		int aHold=mFeatures;
		gAppPtr->mMessageData_TouchPosition=aHoldPos;
		DisableFeature(CPU_TOUCH);
		gAppPtr->ForceTouchStart();
		mFeatures=aHold;
	}
}

void Widget::TouchStart(int x, int y)
{
	mIsDown=true;
}

void Widget::TouchEnd(int x, int y)
{
	if (mIsDown)
	{
		if (mCommand.Len())
		{
			gSounds->mClick.Play();
			if (mCommand=="MORE")
			{
				mName="FETCHING";
				mAnimate=true;

				RComm::RQuery aQ;

				longlong aFilter=FixFilter(gMakermall->mFilter);
				if (gMakermall->mViewing==&gMakermall->mNewLevelsButton) aQ=RComm::LevelSharing_GetNewLevels(gMakermall->mNow,gMakermall->mPage,LEVELFETCHCOUNT,aFilter);
				if (gMakermall->mViewing==&gMakermall->mHotLevelsButton) aQ=RComm::LevelSharing_GetHotLevels(gMakermall->mPage,LEVELFETCHCOUNT,aFilter);
				if (gMakermall->mViewing==&gMakermall->mRandomLevelsButton) aQ=RComm::LevelSharing_GetRandomLevels(gMakermall->mPage,1,aFilter);
				if (gMakermall->mViewing==&gMakermall->mStarLevelsButton) aQ=RComm::LevelSharing_GetStarLevels(gMakermall->mPage,LEVELFETCHCOUNT,aFilter);
				if (gMakermall->mViewing==&gMakermall->mSearchButton) aQ=RComm::LevelSharing_SearchLevels(gMakermall->mSearchName,gMakermall->mSearchAuthor,!gMakermall->mSearchNew,gMakermall->mPage,LEVELFETCHCOUNT,aFilter);

				gMakermall->mPendingQueryList+=aQ;
			}
			if (mCommand=="REFINE")
			{
				gMakermall->mSearchAgain=false;
				gMakermall->mViewing=NULL;
				gMakermall->mNextViewing=&gMakermall->mSearchButton;
			}
			if (mCommand=="NEW")
			{

				MMBrowser* aM=new MMBrowser;
				Rect aRect=Rect(0,0,400,235);
				aM->Size(aRect);
				aM->CenterAt(gG.Center()+Point(0,-5));
				aM->mMLBox.SetLinkNotify(aM);
				aM->mTitle="NEW LEVEL";
				aM->mData=
					"<setup padtop=25 padleft=25 padright=25 padbottom=25><BKGCOLOR #004770>"
					"<CENTER>ENTER THE LEVEL NAME:<BR><MOVECURSOR 0,+8>"
					"<TEXTBOX id=NEWNAME><BR><BR>"
					"<link cmd=MakeNewLevel><IMG OK_Button></link><sp 5><link cmd=Cancel><IMG Cancel_Button></link>"
					;
				aM->Initialize();
				aM->mFirstUpdate=false;

				Closer* aC=new Closer;
				aC->mCloseWhom=aM;
				gAppPtr->AddCPU(aC);
				gAppPtr->AddCPU(aM);

				gApp.SetOnscreenKeyboardExtraSpace(50);				// Extra space for onscreen keyboard (so you can make sure extra buttons are visible)

				Smart(MLTextBox) aTB=aM->mMLBox.GetObjectByID("NEWNAME");
				aTB->mTextBox.SetBackgroundColor(Color(1));
				aTB->mTextBox.SetTextColor(Color(0,0,0,1));
				aTB->mTextBox.SetTextOffset(3,-2);
				aTB->mTextBox.SetCursorOffset(Point(0,2));
				aTB->mTextBox.SetCursorWidth(4);
				aTB->mTextBox.FocusKeyboard();
				aTB->mTextBox.ForbidCharacters("/\\?%%*:|\"<>.[]");
			}
		}
		else
		{
			if (gMakermall->mViewing==&gMakermall->mMyLevelsButton)
			{
				gSounds->mSelect.Play();
				gMakermall->SetMostRecentLevel(mName);
	
				Kill();
				gApp.GoEditor(mName);
				gMakermall->LeaveMakermall();
			}
			else
			{
				//
				// Are we in the avatar zone?
				//
				float aEdge=117;
				if (x<aEdge)
				{
					gSounds->mSelect.Play();
					mIsDown=false;

					PickMenu* aPM=new PickMenu;
					aPM->AddButton(Sprintf("MORE LEVELS FROM %s",mAuthor.c()));
					if (gMakermall->mAdmin)
					{
						aPM->AddButton("BOOST THIS LEVEL");
						aPM->AddButton("UN-BOOST THIS LEVEL");
						aPM->AddButton("NEG THIS LEVEL");
					}
					//aPM->AddButton("UN-SHARE LEVEL");
					aPM->GoX(ConvertLocalToScreen(Point(x,y)),true,0,MBHOOK(
						{
							if (theResult.Len())
							{
								if (theResult.StartsWith("MORE LEVELS"))
								{
									gMakermall->mViewing=NULL;
									gMakermall->mNextViewing=&gMakermall->mSearchButton;
									gMakermall->mSearchAgain=true;
									gMakermall->mSearchName="";
									gMakermall->mSearchAuthor=mAuthor;
									gMakermall->mSearchNew=true;
									// We do a search...
								}
								if (theResult.StartsWith("BOOST"))
								{
									gMakermall->mPendingQueryList+=RComm::LevelSharing_Boost(gAppPtr->mUserName,gAppPtr->mUserPassword,mID,1);
								}
								if (theResult.StartsWith("UN-BOOST"))
								{
									gMakermall->mPendingQueryList+=RComm::LevelSharing_Boost(gAppPtr->mUserName,gAppPtr->mUserPassword,mID,0);
								}
								if (theResult.StartsWith("NEG"))
								{
									gMakermall->mPendingQueryList+=RComm::LevelSharing_Boost(gAppPtr->mUserName,gAppPtr->mUserPassword,mID,-1);
								}
							}

						}
					));//mShareButton.Center());
					return;
				}

				String aCurrentClient=VERSION;
				if (aCurrentClient.ToFloat()<mClientVersion)
				{
					MsgBox *aBox=new MsgBox();
					aBox->GoNoThrottle("PLEASE UPDATE YOUR APP!  THIS LEVEL REQUIRES A NEWER VERSION OF THE ENGINE!",gGCenter(),0);
				}
				else
				{
					//
					// Play a level!
					//
					gMakermall->mRatedID=-1;

					TransDialog* aTD=new TransDialog;
					aTD->mSpeedMod=5.0f;
					aTD->mAddQueryToPending=true;
					gSounds->mModem.Play();

					TransferPacket* aTP=new TransferPacket;
					aTP->mName=mName;
					aTP->mDialog=aTD;
					aTP->mUploadID=mID;
				
					aTD->SetStatus("GETTING LEVEL...");
					gAppPtr->AddCPU(aTD);

					RComm::RQuery aQ=RComm::LevelSharing_DownloadLevel(mID,aTP);
					aTD->mQuery=aQ;

					gMakermall->AddToRecents(mRML);
				}
			}
		}
	}
	mIsDown=false;
}

void MMBrowser::Notify(void* theData)
{
	if (theData==&mClose)
	{
		Kill();
	}
	if (theData==&mNotifyData)
	{
		if (mNotifyData.mParam==NOTIFY_LINK)
		{
			gSounds->mSelect.Play();

			String* aStr=(String*)mNotifyData.mExtraData;
			String& aLink=*aStr;

			if (aLink=="Cancel") Kill();
			if (aLink=="MakeNewLevel")
			{
				Smart(MLTextBox) aTB=mMLBox.GetObjectByID("NEWNAME");
				String aLName=aTB->mTextBox.GetText();

				if (aLName.Len())
				{
					gMakermall->SetMostRecentLevel(aLName);
					
					Kill();

					gWantTutorDialog=true;

					gApp.GoEditor(aLName);
					gMakermall->LeaveMakermall();
				}
				else
				{
					MsgBox *aBox=new MsgBox();
					aBox->GoNoThrottle("PLEASE ENTER A NAME FOR YOUR NEW LEVEL!",gGCenter(),0);
				}
			}
		}
	}
}

void Makermall::ReturnFromEditor(String theName)
{
	FocusBackButton();
	if (gGame) gGame->Kill();
	mPlayingID=-1;

	gApp.FadeInMusic(NULL);


	ReturnToMakermall();

	mFirstUpdate=true;

	//
	// We need to freshen levels so most recent is on top...
	//

	mViewing=NULL;
	mNextViewing=&mMyLevelsButton;
}

void Makermall::SetMostRecentLevel(String theLevel)
{
	if (!theLevel.ContainsI(".kitty")) theLevel+=".kitty";
	SmartList(String) aOrder;
	Utils::CSVToSmartList(gAppPtr->mSettings.GetString("LEVELORDER"),aOrder);
	bool aFound=false;
	EnumSmartList(String,aS,aOrder)
	{
		if (aS->CompareI(theLevel)==0)
		{
			aOrder-=aS;
			aOrder.Insert(aS);
			aFound=true;
			break;
		}
	}
	if (!aFound) aOrder.Insert(new String(theLevel));
	gAppPtr->mSettings.SetString("LEVELORDER",Utils::SmartListToCSV(aOrder));
	gApp.SaveSettings();
}

void Makermall::RemoveFromRecentLevels(String theLevel)
{
	if (!theLevel.ContainsI(".kitty")) theLevel+=".kitty";
	SmartList(String) aOrder;
	Utils::CSVToSmartList(gAppPtr->mSettings.GetString("LEVELORDER"),aOrder);
	EnumSmartList(String,aS,aOrder)
	{
		if (aS->CompareI(theLevel)==0)
		{
			aOrder-=aS;
			break;
		}
	}
	gAppPtr->mSettings.SetString("LEVELORDER",Utils::SmartListToCSV(aOrder));
	gApp.SaveSettings();
}

void Makermall::AddMessage(String theMessage)
{
	if (theMessage.Len()) 
	{
		mMsgList+=theMessage;
		mUnreadMsg=true;
		gApp.mSettings.SetBool(Sprintf("Unread_%s",gAppPtr->mUserName.c()),true);
	}

	String aStr=Utils::ArrayToCSV(mMsgList,127);

	String aMsgName=Sprintf("Messages_%s",gAppPtr->mUserName.c());
	gApp.mSettings.SetString(aMsgName,aStr);
	gApp.SaveSettings();

	gMakermall->FixBottomOfScreen();
}

void Makermall::ManualUnshareLevel(int theID)
{
	// FINISHME
}

void MarkUntested()
{
	_DLOG("!!!! Mark Levels Untested !!!!");

	Array<String> aFNList;
	EnumDirectoryFiles("sandbox://EXTRALEVELS64\\",aFNList);

	for (int aCount=0;aCount<aFNList.Size();aCount++)
	{
		String aFN=Sprintf("sandbox://EXTRALEVELS64\\%s",aFNList[aCount].c());
		WorldEditor aWE;
		aWE.Load(aFN);
		if (aWE.mTestedOK)
		{
			_DLOG(" Setting %s as Not Tested!",aFNList[aCount].c());
			aWE.mTestedOK=false;
			aWE.Save(aFN);
		}
	}
}

void Makermall::BadVersion()
{
	mPendingQueryList.Clear();
	MsgBox* aBox=new MsgBox;
	aBox->GoNoThrottle("Things have changed!\n\nYour version of the engine needs to be updated to access Makermall!  Please update!",gGCenter(),0);
	gApp.GoMainMenu();
}

void Makermall::UnloadAvatars()
{
	// Depreciated cuz of the auto-dynamic-sprites
}

void Makermall::FillProfileInfo(String theProfileInfo)
{
	String aInfoTag, aInfoValue;
	mStarCount=-1;	// Forces the rehup 
	while (theProfileInfo.ParseRML(aInfoTag,aInfoValue))
	{
		if (aInfoTag=="KARMA") {mKarma=aInfoValue;mKarmaWidth=gBundle_Play->mFont_Commodore64Angled8.Width(mKarma);}
		if (aInfoTag=="STARS") {mStars=aInfoValue;mStarCount=mStars.ToInt();}
		if (aInfoTag=="CANLEVELUP") mNotifyLevelup=aInfoValue.ToBool();
		if (aInfoTag=="LEVEL") mLevel=aInfoValue;
		if (aInfoTag=="AVATAR") mAvatar=aInfoValue;
		//if (aInfoTag=="BONUS") LoadBonus(aInfoValue);
	}
	if (mNotifyLevelup) gSounds->mStream_Youcanlevelup.Play();

}

void Makermall::SendIPlayed()
{
	// happens elsewhere now
}

void Makermall::SendIWon(int theGameTime)
{
	RComm::LevelSharing_WonLevel(mPlayingID);
}

void Makermall::GetLocalLevelStats(Array<int>& theLevels)
{
	if (!IsLoginValid()) return;
	mPendingQueryList+=RComm::LevelSharing_GetLocalStats(gAppPtr->mUserName,gAppPtr->mUserPassword,theLevels);
}

TransDialog* gTransDialog=NULL;

TransDialog::TransDialog()
{
	mSpeedMod=1.0f;
	mCanCancel=false;
	HookTo(&gTransDialog);
	mBox=Rect(0,0,mWidth,mHeight);
	mBox=mBox.ExpandH(-50);
	mBox=mBox.ExpandV(-50);
	mProgress=0;
	mStarting=true;
	mDone=false;
}

TransDialog::~TransDialog()
{
	gSounds->mModem.Stop();
}

void TransDialog::Initialize()
{
	if (mCanCancel)
	{
		mCancelRect=gBundle_Play->mFont_Commodore64Angled11.GetBoundingRect(mCancelText,mBox.CenterX(),mBox.LowerLeft().mY-25,0).Expand(10);
	}
}

void TransDialog::Update()
{
	if (mQuery.IsNotNull())
	{
		float aOldProgress=mProgress;
		if (mProgress<mQuery->mProgress) mProgress=_min(1.0f,mProgress+(.0033f*mSpeedMod));
		mStarting=false;
		if (mProgress>=1.0f && aOldProgress<1.0f)
		{
			if (mAddQueryToPending)
			{
				RComm::RCThreadLock();
				gMakermall->mPendingQueryList+=mQuery;
				RComm::RCThreadUnlock();

				gMakermall->UpdatePendingQueryList();
			}
		}
	}
}

void TransDialog::Draw()
{
	gG.SetColor(0,0,0,.5f);
	gG.FillRect();
	gG.SetColor();
	gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(mBox,true);
	gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(mBox,true);
	gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(mBox,true);

	CENTERGLOW(mStatus,mBox.CenterX(),mBox.mY+50);

	if (mDone)
	{
		Rect aDoneRect=Rect(200,90);
		aDoneRect.CenterAt(HalfWidth(),mBox.LowerLeft().mY-(aDoneRect.HalfHeight()+10));
		gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(aDoneRect,true);
		CENTERGLOW("OKAY",aDoneRect.Center().mX,aDoneRect.Center().mY+5);
	}
	else
	{
		if (mCanCancel) CENTERGLOWSMALL(mCancelText,mBox.CenterX(),mBox.LowerLeft().mY-25);
		if (mQuery.IsNull() && mStarting)
		{
			gBundle_Play->mLoading.DrawRotatedScaled(mBox.Center()+Point(0,15),gApp.AppTimeF()*5,.65f);
		}
		else if (mQuery.IsNotNull())
		{
			gG.PushClip();
			Rect aArea=Rect(250,41);
			aArea.CenterAt(mBox.Center());
			gBundle_Play->mSmallrim.DrawMagicBorderEX_AllCorners(aArea.Expand(25),true);
			aArea.mWidth*=mProgress;
			gG.Clip(aArea);


			int aOffset=(gAppPtr->AppTime()*2)%gBundle_ETools->mProcessing.mWidth;
			float aX=aArea.mX-gBundle_ETools->mProcessing.mWidth-aOffset;
			while (aX<(aArea.mX+aArea.mWidth+aOffset))
			{
				gBundle_ETools->mProcessing.Draw(aX,aArea.mY);
				aX+=gBundle_ETools->mProcessing.mWidth;
			}

			gG.PopClip();
		}
	}
}

void TransDialog::SetStatus(String theStatus)
{
	mStatus=gBundle_Play->mFont_Commodore64Angled11.Wrap(theStatus,mBox.mWidth-50);
}

void TransDialog::TouchStart(int x, int y)
{
	if (mCanCancel && !mDone)
	{
		if (mCancelRect.ContainsPoint(x,y)) Kill();
	}
	else if (mDone)
	{
		Rect aDoneRect=Rect(200,90);
		aDoneRect.CenterAt(HalfWidth(),mBox.LowerLeft().mY-(aDoneRect.HalfHeight()+10));
		if (aDoneRect.ContainsPoint(x,y))
		{
			gSounds->mSelect.Play();
			Kill();
		}
	}
}


void Makermall::FillLocalStats(String theString)
{
	if (!mListWidget) return;
	if (!mListWidget->mCPUManager) return;

	String aLevels=theString.GetRML("LEVELS");
	String aTag,aValue;

	while (aLevels.ParseRML(aTag,aValue))
	{
		if (aTag=="LEVEL")
		{
			String aT2,aV2;
			Widget* aW=NULL;
			aValue.ParseRML();
			while (aValue.ParseRML(aT2,aV2))
			{
				if (aT2=="ID")
				{
					int aFindID=aV2.ToInt();
					EnumList(Widget,aWidget,*mListWidget->mCPUManager) if (aWidget->mLocal && aWidget->mID==aFindID) {aW=aWidget;break;}
				}
				if (aW)
				{
					if (aT2=="RATING") aW->mRating=aV2;
					if (aT2=="PLAYCOUNT") aW->mTried=aV2;
					if (aT2=="WINCOUNT") aW->mWon=aV2;
					if (aT2=="COMMENTS") aW->mCommentCount=aV2;
					if (aT2=="STARS") aW->mStarCount=aV2.ToInt();
				}
			}
		}
	}

}

void Makermall::FillLevelStats(String theString)
{
	if (!mListWidget) return;
	if (!mListWidget->mCPUManager) return;

	String aLevels=theString.GetRML("LEVELS");
	String aTag,aValue;

	while (aLevels.ParseRML(aTag,aValue))
	{
		if (aTag=="LEVEL")
		{
			_DLOG("Local Stat: %s",aValue.c());
			//[ID]13321[/ID][RATING]1[/RATING][PLAYCOUNT]0[/PLAYCOUNT][WINCOUNT]0[/WINCOUNT][COMMENTS]0[/COMMENTS][STARS]0[/STARS]
			String aT2,aV2;
			Widget* aW=NULL;
			aValue.ParseRML();
			while (aValue.ParseRML(aT2,aV2))
			{
				if (aT2=="ID")
				{
					int aFindID=aV2.ToInt();
					EnumList(Widget,aWidget,*mListWidget->mCPUManager) if (aWidget->mID==aFindID) {aW=aWidget;break;}
				}
				if (aW)
				{
					if (aT2=="RATING") aW->mRating=aV2;
					if (aT2=="PLAYCOUNT") aW->mTried=aV2;
					if (aT2=="WINCOUNT") aW->mWon=aV2;
					if (aT2=="COMMENTS") aW->mCommentCount=aV2;
					if (aT2=="STARS") aW->mStarCount=aV2.ToInt();
				}
			}
		}
	}

}

void UploadStub(void* theArg)
{
	TransferPacket* aTP=(TransferPacket*)theArg;

	IOBuffer aBuffer;
	aBuffer.Load(aTP->mFN);

	if (aTP->mDialog==gTransDialog)
	{
		gSounds->mModem.Play();

		aTP->mDialog->SetStatus("SHARING LEVEL...");
		RComm::RQuery aQ=RComm::LevelSharing_UploadLevelNoPreview(gAppPtr->mUserName,gAppPtr->mUserPassword,aTP->mName,aTP->mTags,aBuffer,VERSION,NULL);
		aTP->mDialog->mQuery=aQ;
		while (!aQ->IsDone()) Sleep(100);
		while (aTP->mDialog==gTransDialog && aTP->mDialog->mProgress<1.0f) Sleep(100);
		gSounds->mModem.Stop();


		if (aTP->mDialog==gTransDialog)
		{
			String aRString=aQ->mResult.ToString();
			_DLOG("Upload Result: [%s]",aRString.c());
			String aResult=aRString.GetRML("RESULT");
			if (aResult=="OK")
			{
				gSounds->mBell.Play();
				String aStatus=aRString.GetRML("STATUS");
				if (aStatus=="UPDATE")
				{
					aTP->mDialog->SetStatus("YOUR LEVEL HAS BEEN UPDATED ON MAKERMALL!  YOU CAN SEE HOW YOUR LEVEL IS DOING ON THE 'MY LEVELS' TAB!");
				}
				else
				{
					aTP->mDialog->SetStatus("YOUR LEVEL HAS BEEN SHARED ON MAKERMALL!  YOU CAN SEE HOW YOUR LEVEL IS DOING ON THE 'MY LEVELS' TAB!");
				}
				if (gWorldEditor) 
				{
					gWorldEditor->mUploadID=aRString.GetRML("LEVELID").ToInt();
					gWorldEditor->QuickSave();
				}
			}
			else
			{
				gSounds->mDie.Play();
				String aFailReason=aRString.GetRML("FAILREASON");
				if (aFailReason.Len())
				{
					if (aFailReason=="BADCHARS") aFailReason="YOU HAVE INVALID CHARACTERS IN YOUR LEVEL NAME!";
					else if (aFailReason=="SHAREFAIL" || aFailReason=="UPDATEFAIL") aFailReason="SOMETHING WENT WRONG ON THE SERVER, PLEASE TRY AGAIN IN A FEW MINUTES!";
					String aMessage="OOPS!  YOUR LEVEL HAS NOT BEEN SHARED YET!  ";
					aMessage+=aFailReason.ToUpper();
					aTP->mDialog->SetStatus(aMessage);
				}
				else aTP->mDialog->SetStatus("SOMETHING WENT WRONG!  YOUR LEVEL HAS NOT BEEN SHARED ON MAKERMALL!  PLEASE TRY AGAIN IN A FEW MINUTES!");
			}

			aTP->mDialog->mQuery=NULL;
			aTP->mDialog->mDone=true;
		}
	}

	delete aTP;
}

CommentWidget* AddCommentWidget(int theLevelID)
{
	CommentWidget* aCW=new CommentWidget;
	aCW->mLevelID=theLevelID;
	gAppPtr->AddCPU(aCW);

	return aCW;
}

void CommentWidget::Initialize()
{
	mX+=gReserveLeft;mWidth-=gReserveRight+gReserveLeft;

	mTextBox.Size(mWidth-110,70);
	mTextBox.CenterAt(HalfSize()+Point(-10,0));
	mTextBox.mY=38-7;
	*this+=&mTextBox;

	mTextBox.SetFont(&gBundle_Play->mFont_Commodore64Angled11);
	mTextBox.SetBackgroundColor(Color(1));
	mTextBox.SetTextColor(Color(0,0,0,1));
	mTextBox.SetTextOffset(3,0);
	mTextBox.SetCursorOffset(Point(0,2));
	mTextBox.SetCursorWidth(4);
	mTextBox.SetMaxCharCount(200);
	mTextBox.SetMultiLine();
	mTextBox.FocusKeyboard();

	mClose.Size(40,40);
	mClose.CenterAt(Point(mWidth,0.0f)+Point(-28,21));
	mClose.SetNotify(this);
	mClose.SetSounds(&gSounds->mClick);
	*this+=&mClose;

	mSayIt.Size(211,71);
	mSayIt.mY=mTextBox.LowerRight().mY+5;
	mSayIt.mX=mTextBox.LowerRight().mX-mSayIt.mWidth;
	mSayIt.mX+=25;
	mSayIt.SetNotify(this);
	mSayIt.SetSounds(&gSounds->mClick);
	*this+=&mSayIt;

}

void CommentWidget::Update()
{
}

void CommentWidget::Draw()
{
	gG.SetColor(0,0,0,.85f);
	gG.FillRect();
	gG.SetColor();

	gBundle_Play->mTinyCorner.DrawMagicBorder(mTextBox.Expand(10));
	CENTERGLOW("TYPE YOUR COMMENT!",HalfWidth(),16);

	gBundle_Play->mClosecomment.Center(mClose.Center());
	gBundle_Play->mSayit.Draw(mSayIt.UpperLeft());
}

void CommentWidget::Notify(void* theData)
{
	if (theData==&mClose) Kill();
	if (theData==&mSayIt)
	{
		if (mTextBox.GetText().Len())
		{
			//
			// We need to send the comment AND re-fill the ml box...
			//
			if (gMakermall)
			{
				MyMLBox* aBox=GetNotifyMLBox();
				RComm::RQuery aQ=RComm::LevelSharing_Comment(gApp.mUserName,gApp.mUserPassword,mLevelID,mTextBox.GetText(),aBox);
				gMakermall->mPendingQueryList+=aQ;
				if (aBox) aBox->GoPending("Loading comments...");
			}
			
			Kill();
		}
	}
}

#define BOXHEIGHT (90*2)

RatingScreen::RatingScreen()
{
	mFade=0;
	mVote=0;
	mReportAbuse=false;
	mGiveStar=false;
	mStarFade=0;
	mAbuseFade=0;

	if (IsLoginValid() && gMakermall) mMyStars=gMakermall->mStarCount;
	else mMyStars=0;

	mOldReportAbuse=mReportAbuse;
}

RatingScreen::~RatingScreen()
{
	gSounds->mLoop_Reportabuse.Stop();
}

void RatingScreen::Initialize()
{
	FocusBackButton();
	gSounds->mAlert.Play();

	mComments.Size(88,83);
	mComments.CenterAt(HalfSize()+Point(0,10));
	mComments.SetSounds(&gSounds->mClick);
	mComments.SetNotify(this);
	*this+=&mComments;

	mLike.Size(122,122);
	mLike.CenterAt(HalfSize()+Point(0,5)+Point(-100,0));
	mLike.SetSounds(&gSounds->mClick);
	mLike.SetNotify(this);
	*this+=&mLike;

	mHate.Size(122,122);
	mHate.CenterAt(HalfSize()+Point(0,5)+Point(100,0));
	mHate.SetSounds(&gSounds->mClick);
	mHate.SetNotify(this);
	*this+=&mHate;

	mDone.Size(188,77);
	mDone.CenterAt(HalfWidth(),CenterY()+15+(BOXHEIGHT/2));
	mDone.SetSounds(&gSounds->mSelect);
	mDone.SetNotify(this);
	*this+=&mDone;

	mStar.Size(122,122);
	mStar.CenterAt(HalfSize()+Point(0,5)+Point(-220,0));
	mStar.SetSounds(&gSounds->mClick);
	mStar.SetNotify(this);
	*this+=&mStar;

	mReport.Size(122,122);
	mReport.CenterAt(Center()+Point(0,5)+Point(220,0));
	mReport.SetSounds(&gSounds->mClick);
	mReport.SetNotify(this);
	*this+=&mReport;

}

#define SHOVE 150
#define SHOVESPEED 10
void RatingScreen::Update()
{
	if (mOldReportAbuse!=mReportAbuse)
	{
		if (!mReportAbuse) gSounds->mLoop_Reportabuse.Stop();
		else gSounds->mLoop_Reportabuse.Go();
		mOldReportAbuse=mReportAbuse;
	}
	mFade=_min(1.0f,mFade+.1f);
	mStarFade=_max(0,mStarFade-.1f);
	mAbuseFade=_max(0,mAbuseFade-.1f);
	mStar.DisableTouch();
	mReport.DisableTouch();
	if (mVote==0)
	{
		if ((int)mComments.CenterX()!=(int)CenterX())
		{
			for (int aCount=0;aCount<SHOVESPEED;aCount++)
			{
				float aStep=gMath.Sign(CenterX()-mComments.CenterX());
				if ((int)mComments.CenterX()!=(int)CenterX())
				{
					mComments.mX+=aStep;
					mLike.mX+=aStep;
					mHate.mX+=aStep;
					mStar.mX+=aStep;
					mReport.mX+=aStep;
				}
			}
		}
	}
	if (mVote>0)
	{
		if (mMyStars>0) mStar.EnableTouch();
		mStarFade=_min(1.0f,mStarFade+.15f);
		if (mComments.CenterX()<CenterX()+SHOVE)
		{
			for (int aCount=0;aCount<SHOVESPEED;aCount++)
			{
				if (mComments.CenterX()<CenterX()+SHOVE)
				{
					mComments.mX++;
					mLike.mX++;
					mHate.mX++;
					mStar.mX++;
					mReport.mX++;
				}
			}
		}
	}
	if (mVote<0)
	{
		mReport.EnableTouch();
		mAbuseFade=_min(1.0f,mAbuseFade+.15f);
		if (mComments.CenterX()>CenterX()-SHOVE)
		{
			for (int aCount=0;aCount<SHOVESPEED;aCount++)
			{
				if (mComments.CenterX()>CenterX()-SHOVE)
				{
					mComments.mX--;
					mLike.mX--;
					mHate.mX--;
					mStar.mX--;
					mReport.mX--;
				}
			}
		}
	}
}

void RatingScreen::Draw()
{
	gG.SetColor(0,0,0,mFade*.75f);
	gG.FillRect();
	gG.SetColor(0,0,0,mFade);
	gG.FillRect(0,CenterY()-(BOXHEIGHT/2),mWidth,BOXHEIGHT);
	gG.SetColor();

	gBundle_Play->mRate_Border.DrawScaled(CenterX(),CenterY()-(BOXHEIGHT/2),Point(mFade*50,1.0f));
	gBundle_Play->mRate_Border.DrawScaled(CenterX(),CenterY()+(BOXHEIGHT/2),Point(mFade*50,1.0f));
	
	gG.ClipColor(mFade);
	CENTERGLOW("RATE THIS LEVEL!",CenterX(),CenterY()-(BOXHEIGHT/2)+25);

	gG.SetColor(.75f+(gMath.Sin(gAppPtr->AppTimeF()*2)*.2f));
	gBundle_Play->mRate_Comment.Center(mComments.Translate(0.0f,mComments.IsDown()*2.0f));

	gG.SetColor(.75f+(gMath.Sin((gAppPtr->AppTimeF()+90)*2)*.2f));
	if (mVote==1) 
	{
		gG.SetColor();
		gBundle_Play->mRate_Hilitecircle.DrawScaled(mLike.Center()+Point(0.0f,mLike.IsDown()*2.0f),2.3f);
		gG.RenderAdditive();
		gG.SetColor(.5f+gMath.Sin(gAppPtr->AppTimeF()*3)*.5f);
		gBundle_Play->mRate_Hilitecircle.DrawScaled(mLike.Center()+Point(0.0f,mLike.IsDown()*2.0f),2.3f);
		gG.RenderNormal();
		gG.SetColor();
	}
	gBundle_Play->mRate_Like.Center(mLike.Translate(0.0f,mLike.IsDown()*2.0f));
	gG.SetColor(.75f+(gMath.Sin((gAppPtr->AppTimeF()+90+90)*2)*.2f));
	if (mVote==-1) 
	{
		gG.SetColor();
		gBundle_Play->mRate_Hilitecircle.DrawScaled(mHate.Center()+Point(0.0f,mHate.IsDown()*2.0f),2.3f);
		gG.RenderAdditive();
		gG.SetColor(.5f+gMath.Sin(gAppPtr->AppTimeF()*3)*.5f);
		gBundle_Play->mRate_Hilitecircle.DrawScaled(mHate.Center()+Point(0.0f,mHate.IsDown()*2.0f),2.3f);
		gG.RenderNormal();
		gG.SetColor();
	}
	gBundle_Play->mRate_Like.DrawScaled(mHate.Center()+Point(0.0f,mHate.IsDown()*2.0f),-1);
	if (mStarFade>0)
	{
		gG.SetColor(mStarFade*(.75f+(gMath.Sin((gAppPtr->AppTimeF()+90+90)*2)*.2f)));
		if (mGiveStar) gG.SetColor();
		if (mMyStars==0)
		{
			gG.RenderWhite(true);
			//gG.SetColor(mStarFade*.25f);
			gBundle_Play->mRate_Greystar.Center(mStar.Center()+Point(0.0f,mStar.IsDown()*2.0f));
			gG.RenderWhite(false);
			gG.SetColor();
			gBundle_Play->mRate_Nostars.Center(mStar.Center()+Point(-135-((1.0f-mStarFade)*50),3.0f));
		}
		else
		{
			if (mGiveStar)
			{
				gBundle_Play->mRate_Hilitestar.DrawScaled(mStar.Center()+Point(0.0f,mStar.IsDown()*2.0f),2.0f);
				gG.RenderAdditive();
				gG.SetColor(.25f+gMath.Sin((90+gAppPtr->AppTimeF())*3)*.25f);
				gBundle_Play->mRate_Hilitestar.DrawScaled(mStar.Center()+Point(0.0f,mStar.IsDown()*2.0f),2.0f);
				gG.RenderNormal();
				gG.SetColor();
			}
			gBundle_Play->mRate_Star.Center(mStar.Translate(0.0f,mStar.IsDown()*2.0f));
			gG.SetColor();
			gBundle_Play->mRate_ExtraLove.Center(mStar.Center()+Point(-150-((1.0f-mStarFade)*50),-7.0f));
			Point aPos=mStar.Center()+Point(-150,-7)+Point(2,60);
			DRAWGLOW(Sprintf("%d",mMyStars-mGiveStar),aPos.mX,aPos.mY);
		}
	}
	if (mAbuseFade>0)
	{
		gG.SetColor(mAbuseFade*(.75f+(gMath.Sin((gAppPtr->AppTimeF()+90+90)*2)*.2f)));
		if (mReportAbuse) gG.SetColor();

		if (mReportAbuse)
		{
			gBundle_Play->mRate_Hilitereport.DrawScaled(mReport.Center()+Point(0.0f,mReport.IsDown()*2.0f),2.0f);
			gG.RenderAdditive();
			gG.SetColor(.25f+gMath.Sin((90+gAppPtr->AppTimeF())*3)*.25f);
			gBundle_Play->mRate_Hilitereport.DrawScaled(mReport.Center()+Point(0.0f,mReport.IsDown()*2.0f),2.0f);
			gG.RenderNormal();
			gG.SetColor();
		}

		gBundle_Play->mRate_Report.Center(mReport.Translate(0.0f,mReport.IsDown()*2.0f));
		if (mReportAbuse)
		{
			gG.RenderAdditive();
			gG.SetColor(.5f+gMath.Sin((90+gAppPtr->AppTimeF())*5)*.5f);
			gBundle_Play->mRate_Report.Center(mReport.Translate(0.0f,mReport.IsDown()*2.0f));
			gG.RenderNormal();
		}

		gG.SetColor();
		gBundle_Play->mRate_ReportAbuse.Center(mReport.Center()+Point(130+((1.0f-mAbuseFade)*50),-0.0f));
	}

	gG.SetColor();
	gBundle_Play->mRate_Done.Center(mDone.Translate(0.0f,mDone.IsDown()*2.0f));

	gG.ClipColor();
}

void RatingScreen::Notify(void* theData)
{
	if (theData==&mDone)
	{
		if (mReportAbuse)
		{
			AbuseWidget* aCW=new AbuseWidget;
			aCW->mLevelID=mLevelID;
			gAppPtr->AddCPU(aCW);
		}

		if (mVote) RComm::LevelSharing_Vote(mLevelID,mVote);
		if (mGiveStar && gMakermall) gMakermall->mPendingQueryList+=RComm::LevelSharing_GiveStar(gAppPtr->mUserName,gAppPtr->mUserPassword,mLevelID);
		Kill();
	}

	if (theData==&mLike) 
	{
		if (mVote==1) mVote=0;
		else mVote=1;
	}
	if (theData==&mHate)
	{
		if (mVote==-1) mVote=0;
		else mVote=-1;
	}
	if (theData==&mStar)
	{
		mGiveStar=!mGiveStar;
	}
	if (theData==&mReport)
	{
		mReportAbuse=!mReportAbuse;
	}

	if (mVote==0) {mGiveStar=false;mReportAbuse=false;}
	if (mVote==1) mReportAbuse=false;
	if (mVote==-1) mGiveStar=false;

	if (!gMakermall) {_DLOG("NO MAKERMALL!");return;}
	if (theData==&mComments) OpenComments(mLevelID);
}

void AbuseWidget::Draw()
{
	gG.SetColor(0,0,0,.85f);
	gG.FillRect();
	gG.SetColor();

	gBundle_Play->mTinyCorner.DrawMagicBorder(mTextBox.Expand(10));
	CENTERGLOW("WHY IS THIS LEVEL ABUSIVE?",CenterX(),16);

	gBundle_Play->mClosecomment.Center(mClose.Center());
	gBundle_Play->mSayit.Draw(mSayIt.UpperLeft());
}

void AbuseWidget::Notify(void* theData)
{
	Kill();
	if (theData==&mSayIt) if (mTextBox.GetText().Len()) {RComm::LevelSharing_ReportAbuse(mLevelID,mTextBox.GetText());}
}

void Makermall::BackFromPlaying()
{
	FocusBackButton();
	if (mPlayingID!=-1 && mRatedID!=mPlayingID)
	{
		mRatedID=mPlayingID;

		RatingScreen* aRS=new RatingScreen;
		aRS->mLevelID=mPlayingID;
		gAppPtr->AddCPU(aRS);
	}
	
	mPlayingID=-1;
}

FilterWidget::FilterWidget()
{
	mCleanMe=true;
	Size(0,0,gG.WidthF()-100,21);

	Initialize();
	mFirstUpdate=false;
}

void FilterWidget::Initialize()
{
	mTags=gMakermall->mFilter;

	int aTMask=1;
	float aX=21+5;
	float aY=21/2;
	for (int aCount=0;aCount<gMakermall->mTags.Size();aCount++)
	{
		if (mTags&aTMask)
		{
			mTagName+=gMakermall->mTags[aCount].c();
			Rect aRect=Rect(aX,aY,gBundle_Play->mFont_Commodore64Angled8.Width(gMakermall->mTags[aCount]),gBundle_Play->mFont_Commodore64Angled8.mAscent);
			aRect.CenterYAt(aY);
			if (aX+aRect.mWidth>=mWidth)
			{
				aX=21+5;
				aY+=aRect.mHeight+10;
				mHeight+=aRect.mHeight+10;
				aRect.mX=aX;
			}
			aRect.CenterYAt(aY);
			mTagRect+=aRect;
			aX+=aRect.mWidth+10;
		}
		aTMask*=2;
	}
}

void FilterWidget::Draw()
{
	gBundle_Play->mCloseFilter.Center(21/2,mHeight/2);

	for (int aCount=0;aCount<mTagName.Size();aCount++)
	{
		gG.SetColor(.25f);
		gBundle_Play->mTeenyCorner.DrawMagicBorder(mTagRect[aCount].Expand(3),true);
		gG.SetColor(.75f,.75f,.75f,1);
		gBundle_Play->mFont_Commodore64Angled8.Draw(mTagName[aCount],mTagRect[aCount].LowerLeft()+Point(1,-1));
	}
}

void FilterWidget::TouchEnd(int x, int y)
{
	if (mIsDown) if (x<mHeight) 
	{
		gSounds->mSelect.Play();

		Kill();
		gMakermall->mFilter=0;
		gMakermall->mNextViewing=gMakermall->mViewing;
		gMakermall->mViewing=NULL;
	}
	mIsDown=false;
}

bool FilterDialog::Done()
{
	if (gMakermall) 
	{
		gMakermall->mFilter=mTagDialog->mTagBits;
		gMakermall->mNextViewing=gMakermall->mViewing;
		gMakermall->mViewing=NULL;
	}
	return true;
}


void ShowFilterDialog()
{
	FilterDialog* aDialog=new FilterDialog;
	aDialog->Size();
	FullscreenSmallScreen(aDialog,gG.Center());

//	aDialog->mWidth=_max(480,gG.WidthF()-50);
	aDialog->mWidth=_max(480,gG.WidthF());
	aDialog->mHeight=375;
	aDialog->CenterAt(gG.Center());

	aDialog->mX+=gReserveLeft;
	aDialog->mWidth-=gReserveRight+gReserveLeft;


	//aDialog->Size(-25,-20,mWidth-118,mHeight-40);
	aDialog->SetBundle(&gApp.mBundle_FunDialog);
	aDialog->SetTitle("FILTER TAGS");				// Sets the dialog title

	//
	// Build the dialog...
	//
	DialogPanel* aPanel=aDialog->MakeNewPanel("FILTER TAGS");

	aDialog->mTagDialog=new DialogPickTags(gMakermall->mFilter);
	aDialog->mTagDialog->mAllowInfiniteTagPicks=true;
	aPanel->Add(aDialog->mTagDialog);
	aDialog->mTagDialog->mName="SELECT TAGS TO VIEW:";

	Closer* aC=new Closer();
	aC->mCloseWhom=aDialog;
	gApp.AddCPU(aC);
	gApp.AddCPU(aDialog);
}

MMBrowser* gMailBrowser=NULL;
void Makermall::RemoveMail(int theID)
{
	for (int aCount=0;aCount<mMsgList.Size();aCount++)
	{
		if (mMsgIDList[aCount]==theID)
		{
			mMsgList.DeleteElement(aCount);
			mMsgIDList.DeleteElement(aCount);
			break;
		}
	}
	AddMessage("");
	if (gMailBrowser) ShowMail();
}

void ShowMail()
{
	String aML="<SETUP hborders=20><BKGCOLOR #004770><BR><BR>";
	bool aFound=false;

	for (int aCount=gMakermall->mMsgList.Size()-1;aCount>=0;aCount--)
	{
		if (gMakermall->mMsgList[aCount].Len())
		{
			aML+="<confine 30 100%><color .75><font small>MAKERMALL SAYS:</confine>";
			aML+=Sprintf("<pushcursor><confine 0 30><link cmd=trash:%d><img trash></link>",gMakermall->mMsgIDList[aCount]);
			aML+="</confine><popcursor><confine 30 100%><color 1><font normal>";
			aML+=gMakermall->mMsgList[aCount];
			aML+="</confine><BR><BR>";
			aFound=true;
		}
	}

	if (!aFound) aML+="<center><color1>NO MESSAGES!";

	if (!gMailBrowser) 
	{
		gMailBrowser=new MMBrowser;
		gMailBrowser->HookTo(&gMailBrowser);
		Rect aRect=Rect(0,0,gG.WidthF(),gG.HeightF());
		gMailBrowser->Size(aRect);
		gMailBrowser->CenterAt(gG.Center()+Point(0,-5));
		gMailBrowser->mMLBox.SetLinkNotify(gMailBrowser);
		gMailBrowser->mTitle="MAIL";
		//gMailBrowser->mMLBox.AddIMG(&gBundle_Play->mTrash,"trash");

		Closer* aC=new Closer;
		aC->mCloseWhom=gMailBrowser;
		gAppPtr->AddCPU(aC);
		gAppPtr->AddCPU(gMailBrowser);
	}

	MMBrowser* aM=gMailBrowser;
	aM->mMLBox.Reset();
	aM->mData=aML;
	aM->Initialize();
	aM->mFirstUpdate=false;

	gApp.SetOnscreenKeyboardExtraSpace(50);				// Extra space for onscreen keyboard (so you can make sure extra buttons are visible)

	gMakermall->mUnreadMsg=false;
	gApp.mSettings.SetBool(Sprintf("Unread_%s",gAppPtr->mUserName.c()),false);
	gMakermall->FixBottomOfScreen();

}

SearchWidget::SearchWidget()
{
	mCleanMe=true;
	Size(0,0,gG.WidthF()-100,175);
	mColor=IColor(157,194,186);

	Initialize();
	mFirstUpdate=false;
	mNewest=true;
}

void SearchWidget::Initialize()
{
	//
	// Add the stuff...
	//
	float aOffset=28;
	TextBox* aTextBox=&mSearchName;
	aTextBox->Size(125+aOffset,10,mWidth-135-aOffset,gBundle_Play->mFont_Commodore64Angled11.mAscent+10);
	aTextBox->mFont=&gBundle_Play->mFont_Commodore64Angled11;
	aTextBox->SetBorder(5);
	aTextBox->SetBackgroundColor(Color(1));
	aTextBox->SetTextColor(Color(68.0f/255.0f,92.0f/255.0f,85.0f/255.0f,1));
	aTextBox->SetTextOffset(3,-2);
	aTextBox->SetCursorOffset(Point(0,2));
	aTextBox->SetCursorWidth(4);
//	aTextBox->FocusKeyboard();
	aTextBox->ForbidCharacters("/\\?%%*:|\"<>.[]");
	aTextBox->SetText(gMakermall->mSearchName);
	*this+=aTextBox;

	aTextBox=&mSearchAuthor;
	aTextBox->Size(125+aOffset,43,mWidth-135-aOffset,gBundle_Play->mFont_Commodore64Angled11.mAscent+10);
	aTextBox->mFont=&gBundle_Play->mFont_Commodore64Angled11;
	aTextBox->SetBorder(5);
	aTextBox->SetBackgroundColor(Color(1));
	aTextBox->SetTextColor(Color(68.0f/255.0f,92.0f/255.0f,85.0f/255.0f,1));
	aTextBox->SetTextOffset(3,-2);
	aTextBox->SetCursorOffset(Point(0,2));
	aTextBox->SetCursorWidth(4);
	aTextBox->ForbidCharacters("/\\?%%*:|\"<>.[]");
	aTextBox->SetText(gMakermall->mSearchAuthor);
	*this+=aTextBox;

	mNewest=gMakermall->mSearchNew;

	Point aUL=mSearchAuthor.LowerLeft()+Point(60-30,30-18-5);
	mOrderNew.Size(aUL.mX-20,aUL.mY,250,22);
	mOrderNew.SetSounds(&gSounds->mSelect);
	mOrderNew.SetNotify(this);
	mOrderNew.NotifyOnPress();
	*this+=&mOrderNew;

	aUL=mSearchAuthor.LowerLeft()+Point(60-30,55-18-5);
	mOrderOld.Size(aUL.mX-20,aUL.mY,250,22);
	mOrderOld.SetSounds(&gSounds->mSelect);
	mOrderOld.SetNotify(this);
	mOrderOld.NotifyOnPress();
	*this+=&mOrderOld;

	Rect aR=Rect(mWidth-100,30);
	aR.CenterAt(mWidth/2,mHeight-25);
	mSearch.Size(aR);
	mSearch.SetSounds(&gSounds->mSelect);
	mSearch.SetNotify(this);
	*this+=&mSearch;

}

void SearchWidget::Draw()
{
	gG.SetColor(mColor);
	gG.FillRect(0,0,mWidth,mHeight);
	gG.SetColor();
	gBundle_Play->mFont_Commodore64Angled11.Right("BY NAME:",mSearchName.LowerLeft()+Point(0,-7));
	gBundle_Play->mFont_Commodore64Angled11.Right("BY AUTHOR:",mSearchAuthor.LowerLeft()+Point(0,-7));

	
	gG.Translate(0,-5);
	gBundle_Play->mFont_Commodore64Angled11.Draw("NEWEST FIRST",mSearchAuthor.LowerLeft()+Point(60,30));
	gBundle_Play->mCheckbox[mNewest].Draw(mSearchAuthor.LowerLeft()+Point(60-30,30-18));

	gBundle_Play->mFont_Commodore64Angled11.Draw("OLDEST FIRST",mSearchAuthor.LowerLeft()+Point(60,55));
	gBundle_Play->mCheckbox[!mNewest].Draw(mSearchAuthor.LowerLeft()+Point(60-30,55-18));
	gG.Translate(0,5);

	gG.SetColor(68.0f/255.0f,92.0f/255.0f,85.0f/255.0f,.75f);
	if (mSearch.IsDown()) gG.SetColor(68.0f/255.0f,92.0f/255.0f,85.0f/255.0f,.5f);
	gG.FillRect(mSearch);
	gG.SetColor();
	gBundle_Play->mFont_Commodore64Angled11.Center("SEARCH NOW",mSearch.CenterX(),mSearch.LowerLeft().mY-9+mSearch.IsDown()*2);
}

void SearchWidget::Notify(void* theData)
{
	if (theData==&mOrderNew) {mNewest=true;}
	else if (theData==&mOrderOld) {mNewest=false;}
	else if (theData==&mSearch)
	{
		//
		// For Rasta, somehow this ends up loading a level... what the hell is that!
		//

		if (gMakermall->mListWidget) gMakermall->mListWidget->Kill();

		gMakermall->mListWidget=new ListWidget();
		gMakermall->mListWidget->Size(0,93,gMakermall->Width()+1,193);
		gMakermall->AddCPU(gMakermall->mListWidget);
		gMakermall->mPage=0;

		gMakermall->mSearchName=mSearchName.GetText();
		gMakermall->mSearchAuthor=mSearchAuthor.GetText();
		gMakermall->mSearchNew=mNewest;

		RComm::RQuery aQ=RComm::LevelSharing_SearchLevels(gMakermall->mSearchName,gMakermall->mSearchAuthor,!gMakermall->mSearchNew,gMakermall->mPage,LEVELFETCHCOUNT,FixFilter(gMakermall->mFilter));
		gMakermall->mPendingQueryList+=aQ;

		gMakermall->mSearchAgain=true;
	}
}


void UpdateAvatarList()
{
	//gOut.Out("Avatar Count: %d <- %d",gAvatarList.GetCount(),gAvatarStubCount);
	static int aStep=0;
	if (gAvatarList.GetCount())
	{
		if (aStep>=gAvatarList.GetCount()) aStep=0;

		Smart(AvatarSprite) aAS=gAvatarList[aStep];
		aAS->mCountdown--;
		if (aAS->mCountdown==0) gAvatarList-=aAS;

		aStep++;
	}
}

int gPendingAvatars=0;
Smart(AvatarSprite) GetAvatar(String theAvatar, bool isManaged)
{
	if (theAvatar.Len()==0) theAvatar="null";
	if (isManaged) EnumSmartList(AvatarSprite,aAS,gAvatarList) if (aAS->mName==theAvatar) return aAS;

	Smart(AvatarSprite) aAS=new AvatarSprite;
	aAS->mName=theAvatar;
	aAS->mSprite=new Sprite;
	gAppPtr->AddBackgroundProcess(aAS.GetPointer());
	aAS->Go();

	if (isManaged) gAvatarList+=aAS;
	return aAS;
}

void AvatarSprite::Go()
{
	String aSprFN="temp://avatars\\";
	aSprFN+=mName;

	if (DoesImageExist(aSprFN)) 
	{
		mSprite->Load(aSprFN);
		gAppPtr->RemoveBackgroundProcess(this);
	}
	else
	{
		String aAvatarURL=Sprintf("%s/your_avatar_page_name",RComm::GetURL().c());
		mBuffer=new IOBuffer;
		mBuffer->DownloadA(aAvatarURL);
		mFlag=0;
	}
}

void AvatarSprite::Update()
{
	if (mFlag==1) // Waiting to start...
	{
		if (gPendingAvatars<3) Go();
	}
	else if (!mBuffer->IsDownloading())
	{
		gPendingAvatars=_max(0,gPendingAvatars-1);
		//_DLOG("Finished Downloading Avatar = %s -> %d",mName.c(),mBuffer->Len());
		gAppPtr->RemoveBackgroundProcess(this);
		if (!mBuffer->IsRML() && mBuffer->Len()>0)
		{
			String aFolder="temp://avatars\\";
			MakeDirectory(aFolder);
			String aFind=aFolder;
			aFind+=mName;

			String aSave=aFind;
			aSave+=".jpg";
			mBuffer->CommitFile(aSave);
			mBuffer=NULL;

			mSprite->Load(aFind);
		}
		else 
		{
			//
			// Could not find it to download...
			//
			gOut.Debug("Failed to load AVATAR");
		}

	}
}



PickMenu::PickMenu()
{
	mFont=&gBundle_Play->mFont_Commodore64Angled11;
	mRimSprite=&gBundle_Play->mSmallrim;
	mRimBorder=25;
}

void PickMenu::Initialize()
{
}

void PickMenu::Update()
{
}

void PickMenu::Draw()
{
	if (mDarken)
	{
		gG.SetColor(0,0,0,.5f);
		gG.FillRect(-5000,-5000,15000,15000);
		gG.SetColor();
	}
	gG.PushClip();
	gG.Clip();
	mRimSprite->DrawMagicBorderEX_AllCorners(-mRimBorder,-mRimBorder,mWidth+mRimBorder*2,mHeight+mRimBorder*2,true);
	mRimSprite->DrawMagicBorderEX_AllCorners(-mRimBorder,-mRimBorder,mWidth+mRimBorder*2,mHeight+mRimBorder*2,true);
	mRimSprite->DrawMagicBorderEX_AllCorners(-mRimBorder,-mRimBorder,mWidth+mRimBorder*2,mHeight+mRimBorder*2,true);
	gG.PopClip();

	EnumList(Button,aB,mButtonList)
	{
		String* aS=(String*)aB->ExtraData();

		if (mFont==&gBundle_Play->mFont_Commodore64Angled18) {DRAWGLOWBIG(*aS,0,aB->mY+aB->mHeight-2);}
		else {DRAWGLOW(*aS,0,aB->mY+aB->mHeight-2);}
	}
}

void PickMenu::AddButton(String theText)
{
	mButtonText+=theText;
}

void PickMenu::Notify(void* theData)
{
	Button* aB=(Button*)theData;
	String* aS=(String*)aB->ExtraData();
	mResult=*aS;
	mHook(mResult);
	Kill();
}

void PickMenu::GoX(Point theCenter, bool doFit, float extraButtonSize,MBHOOKPTR theHook)
{
	mResult="";
	mHook=theHook;

	Rect aRect;
	float aMaxWidth=0;
	for (int aCount=0;aCount<mButtonText.Size();aCount++)
	{
		Rect aButtonRect=mFont->GetBoundingRect(mButtonText[aCount],Point(0,0));
		aMaxWidth=_max(aMaxWidth,aButtonRect.mWidth);

		Button* aButton=new Button;
		aButton->ExtraData()=&mButtonText[aCount];
		aButton->Size(0,0,aButtonRect.mWidth,aButtonRect.mHeight+extraButtonSize);
		aButton->SetNotify(this);
		aButton->SetSounds(&gSounds->mClick);

		mButtonList+=aButton;
		*this+=aButton;
	}

	float aY=0;
	EnumList(Button,aB,mButtonList)
	{
		aB->mY=aY;
		aY+=aB->mHeight+5;
		aRect=aRect.Union(*aB);
	}
	Size(0,0,aRect.mWidth+(extraButtonSize*2),aRect.mHeight+extraButtonSize);
	CenterAt(theCenter);

	if (doFit)
	{
		if (mY<mRimBorder) mY=mRimBorder;
		//	if (mX+mWidth>gG.mWidth-mRimBorder) mX-=(mX+mWidth)-(gG.mWidth-mRimBorder);
		if (mX+mWidth>theCenter.mX-mRimBorder) mX-=(mX+mWidth)-(theCenter.mX-mRimBorder);
		if (mX<mRimBorder) mX=mRimBorder;
		if (mY+mHeight>gG.HeightF()-mRimBorder) mY-=(mY+mHeight)-(gG.HeightF()-mRimBorder);
	}

	Closer* aC=new Closer;
	aC->mCloseWhom=this;

	gAppPtr->AddCPU(aC);
	gAppPtr->AddCPU(this);
}

void Makermall::AddToRecents(String theRML)
{
	int aLevelID=theRML.GetRML("ID").ToInt();
	//
	// Scan existing list, remove anyone with the same ID (this forces it up to the top!)
	//
	for (int aCount=0;aCount<mRecentLevels.Size();aCount++)
	{
		int aPreviousID=mRecentLevels[aCount].GetRML("ID").ToInt();
		if (aPreviousID==aLevelID)
		{
			mRecentLevels.DeleteElement(aCount);
			aCount--;
		}
	}

	String aTag,aValue;
	String aNewString="";

	theRML.ParseRML();
	while (theRML.ParseRML(aTag,aValue))
	{
		if (aTag=="RATING" || aTag=="PLAYCOUNT" || aTag=="WINCOUNT" || aTag=="COMMENTS" || aTag=="STARS") aValue="";
		aNewString+="[";
		aNewString+=aTag;
		aNewString+="]";
		aNewString+=aValue;
		aNewString+="[/";
		aNewString+=aTag;
		aNewString+="]";
	}

	mRecentLevels.InsertElement(aNewString);

	SaveRecents();
}

void Makermall::SaveRecents()
{
	IOBuffer aBuffer;
	if (mRecentLevels.Size()>=MAX_RECENT_LEVELS)
	{
		//
		// Remove the continue files for savegames > 25
		//
		for (int aCount=MAX_RECENT_LEVELS;aCount<mRecentLevels.Size();aCount++)
		{
			int aLevelID=mRecentLevels[aCount].GetRML("ID").ToInt();
			String aFN=Sprintf("sandbox://MMRESUME-%d.sav",aLevelID);
			DeleteFile(aFN);
		}
	}

	for (int aCount=0;aCount<_min(MAX_RECENT_LEVELS,mRecentLevels.Size());aCount++) aBuffer.WriteString(mRecentLevels[aCount]);
	aBuffer.CommitFile("sandbox://recent.levels");
	mRecentLevels.Clip(MAX_RECENT_LEVELS);
}

void Makermall::RemoveFromRecentList(int theID)
{
	for (int aCount=0;aCount<mRecentLevels.Size();aCount++)
	{
		int aLevelID=mRecentLevels[aCount].GetRML("ID").ToInt();
		if (aLevelID==theID)
		{
			String aFN=Sprintf("sandbox://MMRESUME-%d.sav",aLevelID);
			DeleteFile(aFN);
			mRecentLevels.DeleteElement(aCount);
			aCount--;
		}
	}
	SaveRecents();
}

void Makermall::LoadRecents()
{
	IOBuffer aBuffer;
	mRecentLevels.Reset();
	aBuffer.Load("sandbox://recent.levels");

	Array<int> aLevelIDList;

	while (!aBuffer.IsEnd()) 
	{
		String aRecent=aBuffer.ReadString();
		int aLevelID=aRecent.GetRML("ID").ToInt();

		if (aLevelIDList.FindValue(aLevelID)>=0) continue;
		//String aFN=PointAtSandbox(Sprintf("MMRESUME-%d.sav",aLevelID));
		//if (!DoesFileExist(aFN)) continue;

		aLevelIDList+=aLevelID;
		mRecentLevels+=aRecent;
	}

	mPendingQueryList+=RComm::LevelSharing_GetLevelStats(aLevelIDList);
}


void Makermall::UploadLevel(TransferPacket* theTP)
{
	IOBuffer aBuffer;
	aBuffer.Load(theTP->mFN);

	gSounds->mModem.Play();

	theTP->mDialog->SetStatus("SHARING LEVEL...");
	RComm::RQuery aQ=RComm::LevelSharing_UploadLevelNoPreview(gAppPtr->mUserName,gAppPtr->mUserPassword,theTP->mName,theTP->mTags,aBuffer,VERSION,NULL);
	theTP->mDialog->mQuery=aQ;
	aQ->mExtraData=theTP;
	theTP->mDialog->mAddQueryToPending=true;
}

void Makermall::UnshareLevel(TransferPacket* theTP)
{
	gSounds->mModem.Play();

	RComm::RQuery aQ=RComm::LevelSharing_DeleteLevel(gAppPtr->mUserName,gAppPtr->mUserPassword,theTP->mUploadID);
	theTP->mDialog->mQuery=aQ;
	theTP->mDialog->mQuery=aQ;
	theTP->mDialog->mAddQueryToPending=true;
	aQ->mExtraData=theTP;
}

void Makermall::LeaveMakermall()
{
	DisableDraw();
	DisableUpdate();
	DisableTouch();
	if (mListWidget) mListWidget->UnbindSpins();
}

void Makermall::ReturnToMakermall()
{
	EnableTouch();
	EnableUpdate();
	EnableDraw();
	if (mListWidget) mListWidget->BindSpinsV();
	mWasInstaplay=false;
}
