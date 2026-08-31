#include "MLRender.h"
#include "MyApp.h"
#include "os_core.h"

List gMLBoxList;
int gMLBoxListLock=-1;

MLBox::MLBox()
{
	if (gMLBoxListLock==-1) gMLBoxListLock=CreateThreadLock();

	mThreadLock=CreateThreadLock();
	mCurrentFont=mDefaultFont=NULL;
	mSkipOSTags=false;
	mDefaultIMG=NULL;
	mReadyToDisplay=true;
	mAbort=false;
	mBKGColor=Color(0,0,0,1);
	mChanged=false;
	mStubPending=false;

	ThreadLock(gMLBoxListLock);
	gMLBoxList+=this;
	ThreadUnlock(gMLBoxListLock);

	mLinkNotify=NULL;
}

MLBox::~MLBox()
{
	ThreadLock(gMLBoxListLock);
	gMLBoxList-=this;
	ThreadUnlock(gMLBoxListLock);

	mAbort=true;
	while (mStubPending) {Sleep(5);}
	ReleaseThreadLock(mThreadLock);
	ClearContents();
}

void MLBox::LoadStub(void* theArg)
{
	gG.Threading();
	MLBox* aBox=(MLBox*) theArg;

	ThreadLock(gMLBoxListLock);
	if (gMLBoxList.Exists(aBox) && aBox->mLayoutData.Len()==0)
	{
		String aLayoutURL=aBox->mLayoutURL;
		ThreadUnlock(gMLBoxListLock);
		IOBuffer aBuffer;

		if (aLayoutURL.StartsWith("http:\\\\") || aLayoutURL.StartsWith("http://") || aLayoutURL.StartsWith("www.")) aBuffer.Download(aLayoutURL);
		else aBuffer.Load(aLayoutURL);
		ThreadLock(gMLBoxListLock);
		if (gMLBoxList.Exists(aBox)) 
		{
			ThreadUnlock(gMLBoxListLock);
			aBox->mStubPending=true;
			if (!aBox->mAbort) aBox->Format(aBuffer.ToString());
			aBox->mStubPending=false;
		}
		else ThreadUnlock(gMLBoxListLock);
	}
	else
	{
		//OS_Core::Sleep(200); // Why was this sleep here?  Is there a reason?  Was it just a thing for CC?
		if (gMLBoxList.Exists(aBox)) 
		{
			aBox->mStubPending=true;
			ThreadUnlock(gMLBoxListLock);
			if (!aBox->mAbort) aBox->Format(aBox->mLayoutData);
			aBox->mStubPending=false;
		}
		else ThreadUnlock(gMLBoxListLock);
	}
}

void MLBox::Load(String theURL)
{
	mSetup.Reset();
	Reset();
	mReadyToDisplay=false;

	mLayoutURL=theURL;
	mLayoutData="";

#ifndef NO_THREADS
	Thread(LoadStub,this);
#else
	#ifdef FORCE_THREADING
	LoadStub(this);
	#else
	// Won't work on non-threaded... we can't download synchronously on WASM
	// Bail out so we can detect it...
	gOut.Out("CAN'T DO THIS OPERATION THREADED!  CALLING EXIT(0)  (Define FORCE_THREADING to enable this... but you will hang for threads then!)");
	exit(0);
	#endif
#endif
}

void MLBox::LoadFromString(String theString)
{
	mSetup.Reset();
	Reset();
	mReadyToDisplay=false;

	mLayoutURL="";
	mLayoutData=theString;

#ifndef NO_THREADS
	Thread(LoadStub,this);
#else
	// Check for needed downloads first...
	Smart(Preloader) aPL=Preload(theString);
	if (aPL->mPending==0) LoadStub(this);
	else 
	{
		mPreloader=aPL;
		mStubPending=true;
	}
#endif
}

Smart(Preloader) gPreloader=NULL;
Smart(Preloader) MLBox::Preload(String theString)
{
	Smart(Preloader) aPL=new Preloader;
	aPL->mSelf=aPL;
	aPL->mReformatString=theString;

	gPreloader=aPL;
	Format(theString);
	gPreloader=NULL;

	return aPL;
}


void MLBox::Format(String theString)
{
	mSkipOSTags=false;

	mLinkList.Clear();
	mLinkFromRecordList.Clear();
	mRecordList.Clear();
	mFlagList.Clear();

	if (mDefineList.GetCount())
	EnumSmartList(DefineStruct,aDS,mDefineList) theString.Replace(aDS->mFrom,aDS->mTo);
	mSetup.Reset();

	mCurrentFont=mDefaultFont;
	SmartList(MLObject)	aObjectList;

	if (!mCurrentFont)
	{
		gOut.Out("MLBox ERROR: No default font set for MLBox...");
		mAbort=true;
	}
	else
	{
		String aResult;

		while (theString.StepXML(aResult))
		{
			if (mAbort) break;
			//gOut.Out("XMLStep:[%s]",aResult.c());

			if (aResult[0]=='<')
			{
				aResult=aResult.GetSegment(1,aResult.Len()-2);
				FormatTag(aResult,aObjectList);
			}
			else
			{
				int aWordStart=-1;
				for (int aCount=0;aCount<aResult.Len();aCount++)
				{
					if (aResult[aCount]<=32 && aResult[aCount]>0)
					{
						if (aWordStart>=0)
						{
							String aGet=aResult.GetSegment(aWordStart,aCount-aWordStart);

							aGet.RemoveLeadingSpaces();
							aGet.RemoveTrailingSpaces();
							if (aGet.Len())
							{
								float aPageW=mWidth-mSetup.mPadLeft-mSetup.mPadRight;
								while (mCurrentFont->Width(aGet)>aPageW)
								{
									for (int aCount=0;aCount<aGet.Len();aCount++)
									{
										if (mCurrentFont->Width(aGet.GetSegment(0,aCount))>aPageW)
										{
											aCount--;

											Smart(MLText) aTxt=new MLText;aObjectList+=aTxt;
											aTxt->mFont=mCurrentFont;
											aTxt->mText=aGet.GetSegment(0,aCount);
											aTxt->mWidth=mCurrentFont->Width(aGet);
											aTxt->mHeight=mCurrentFont->mPointSize;
											aTxt->mLink=mCurrentLink;

											aGet=aGet.GetSegment(aCount,999999);
											break;
										}
									}
								}

								Smart(MLText) aTxt=new MLText;
								if (!mSkipOSTags) aObjectList+=aTxt;
								aTxt->mFont=mCurrentFont;
								aTxt->mText=aGet;

								aTxt->mWidth=mCurrentFont->Width(aGet);
								aTxt->mHeight=mCurrentFont->mPointSize;
								aTxt->mLink=mCurrentLink;
							}
							aWordStart=-1;
						}
					}
					else if (aWordStart==-1) aWordStart=aCount;
				}
				if (aWordStart>=0)
				{
					String aGet=aResult.GetSegment(aWordStart,aResult.Len()-aWordStart);
					aGet.RemoveLeadingSpaces();
					aGet.RemoveTrailingSpaces();
					if (aGet.Len())
					{
						Smart(MLText) aTxt=new MLText;
						if (!mSkipOSTags) aObjectList+=aTxt;
						aTxt->mFont=mCurrentFont;
						aTxt->mText=aGet;
						aTxt->mWidth=mCurrentFont->Width(aGet);
						aTxt->mHeight=mCurrentFont->mPointSize;
						aTxt->mLink=mCurrentLink;
					}
				}
			}
		}
	}

	EnumSmartList(MLObject,aO,aObjectList) aO->mBox=this;

	ThreadLock(mThreadLock);
	if (!mAbort) ClearContents();
	if (!mAbort) mObjectList+=aObjectList;
	if (!mAbort) Layout();

	EnumSmartList(MLImage,aMI,mCustomFromRecordList)
	{
		Rect aRect=GetRecord(aMI->mName);
		aMI->mX=aRect.mX;
		aMI->mY=aRect.mY;
		aMI->mWidth=aRect.mWidth;
		aMI->mHeight=aRect.mHeight;
	}
	mCustomFromRecordList.Clear();

	EnumSmartList(MLLink,aLink,mLinkFromRecordList)
	{
		Rect aRect=GetRecord(aLink->mRef);

		Button* aB=new Button;
		aB->Size(aRect);
		aB->SetNotify(this);
		aB->DragThru();
		aB->mDragThruIgnoreTouchEnd=false;
		aB->ExtraData()=aLink.GetPointer();
		aLink->mButton=aB;
		mLinkButtonList+=aB;
		mLinkList+=aLink;
		*this+=aB;

//		gOut.Out("Added Link From Record[%s]: %s",aLink->mRef.c(),aRect.ToString());
	}
	mLinkFromRecordList.Clear();

	//
	// A kludge... I don't want to make a tag to make my textbox do uppercase, so
	// I'm kludging it here for that purpose.
	//
	Smart(MLTextBox) aUName=GetObjectByID("secretcode");
	if (aUName) aUName->mTextBox.ForceUppercase();

	ThreadUnlock(mThreadLock);
	mReadyToDisplay=true;
	aObjectList.Clear();

	mChanged=true;
}

void MLBox::Update()
{
	if (mChanged)
	{
		mChanged=false;
		Changed();
	}
}



void MLBox::FormatTag(String theTag, SmartList(MLObject)& theObjectList)
{
	String aTag;
	Array<String> aParams;
	theTag.ParseXMLTag(aTag,&aParams);

	if (aTag=="/os") mSkipOSTags=false;
	if (mSkipOSTags) return;

	if (aTag=="BR") {Smart(MLObject) aBr=new MLObject;aBr->mType=ML_LINEBREAK;theObjectList+=aBr;}
	else if (aTag=="CR")  {Smart(MLObject) aBr=new MLObject;aBr->mType=ML_CR;theObjectList+=aBr;}
	else if (aTag=="LF")  {Smart(MLObject) aBr=new MLObject;aBr->mType=ML_LF;theObjectList+=aBr;}
	else if (aTag=="EXE")  {Exe(aParams);}
	else if (aTag=="LINK")
	{
		Smart(MLLink) aLink=new MLLink;

		for (int aCount=0;aCount<aParams.Size();aCount++)
		{
			String aParam=aParams[aCount].GetSegmentBefore('=');
			String aValue=aParams[aCount].GetSegmentAfter('=');
			if (aParam[0]=='\"') aParam=aParam.GetBracedToken('\"','\"');
			if (aValue[0]=='\"') aValue=aValue.GetBracedToken('\"','\"');

			if (aParam=="ID") aLink->mID=aValue.c_copy();
			if (aParam=="CMD") aLink->mCommand=aValue;
		}
		aLink->mIsWebLink=false;
		mLinkList+=aLink;
		mCurrentLink=aLink;
	}
	else if (aTag=="/LINK") mCurrentLink=NULL;
	else if (aTag=="LINKFROMRECORD")
	{
		Smart(MLLink) aLink=new MLLink;
		aLink->mCommand=aParams[1];
		aLink->mRef=aParams[0];
		aLink->mIsWebLink=false;
		mLinkFromRecordList+=aLink;

		for (int aCount=1;aCount<aParams.Size();aCount++)
		{
			String aParam=aParams[aCount].GetSegmentBefore('=');
			String aValue=aParams[aCount].GetSegmentAfter('=');
			if (aParam[0]=='\"') aParam=aParam.GetBracedToken('\"','\"');
			if (aValue[0]=='\"') aValue=aValue.GetBracedToken('\"','\"');

			if (aParam=="ID") aLink->mID=aValue.c_copy();
			if (aParam=="CMD") aLink->mCommand=aValue;
		}
	}
	else if (aTag=="click")
	{
		Smart(MLText) aTxt=new MLText;theObjectList+=aTxt;
		aTxt->mFont=mCurrentFont;

		if (!gApp.IsTouchDevice()) aTxt->mText=aTag;
		else
		{
			char* aLower="touch";
			char* aUpper="TOUCH";
			for (int aCount=0;aCount<5;aCount++)
			{
				if (aTag[aCount]<='Z') aTxt->mText+=aUpper[aCount];
				else aTxt->mText+=aLower[aCount];
			}
		}

		//aTxt->mText="click";
		aTxt->mWidth=mCurrentFont->Width(aTxt->mText);
		aTxt->mHeight=mCurrentFont->mPointSize;
		aTxt->mLink=mCurrentLink;
	}
	else if (aTag=="double-click")
	{
		Smart(MLText) aTxt=new MLText;theObjectList+=aTxt;
		aTxt->mFont=mCurrentFont;

		char *aLower="double-click";
		char *aUpper="DOUBLE-CLICK";

		if (gApp.IsTouchDevice())
		{
			char* aLower="double-tap";
			char* aUpper="DOUBLE-TAP";
		}

		for (int aCount=0;aCount<aTag.Len();aCount++)
		{
			if (aTag[aCount]<='Z') aTxt->mText+=aUpper[aCount];
			else aTxt->mText+=aLower[aCount];
		}

		//aTxt->mText="click";
		aTxt->mWidth=mCurrentFont->Width(aTxt->mText);
		aTxt->mHeight=mCurrentFont->mPointSize;
		aTxt->mLink=mCurrentLink;
	}
	else if (aTag=="WEBLINK")
	{
		Smart(MLLink) aLink=new MLLink;
		aLink->mCommand=aParams[0];
		aLink->mIsWebLink=true;
		mLinkList+=aLink;
		mCurrentLink=aLink;
	}
	else if (aTag=="/WEBLINK") mCurrentLink=NULL;
	else if (aTag=="VERSION") mVersion=aParams[0];
	else if (aTag=="COLOR")
	{
		Smart(MLColor) aC=new MLColor;
		if (aParams[0].Len()==0) aC->mColor=mDefaultColor;
		else if (aParams[0][0]=='#') aC->mColor.FromWeb(aParams[0]);
		else aC->mColor.FromText(aParams[0]);
		theObjectList+=aC;
	}
	else if (aTag=="BKGCOLOR")
	{
		if (aParams[0].Len()==0) mBKGColor=mDefaultColor;
		else if (aParams[0][0]=='#') mBKGColor.FromWeb(aParams[0]);
		else mBKGColor.FromText(aParams[0]);
	}
	else if (aTag=="LEFT")
	{
		Smart(MLAlign) aA=new MLAlign;
		aA->mAlign=-1;
		theObjectList+=aA;
	}
	else if (aTag=="CENTER")
	{
		Smart(MLAlign) aA=new MLAlign;
		aA->mAlign=0;
		theObjectList+=aA;
	}
	else if (aTag=="RIGHT")
	{
		Smart(MLAlign) aA=new MLAlign;
		aA->mAlign=1;
		theObjectList+=aA;
	}
	else if (aTag=="FONT")
	{
		EnumSmartList(FontStruct,aFS,mFontList)	{if (aFS->mName==aParams[0]) {mCurrentFont=aFS->mFont;break;}}
	}
	else if (aTag=="RECORD")
	{
		Smart(MLRecord) aR=new MLRecord;
		aR->mName=aParams[0];
		theObjectList+=aR;

	}
	else if (aTag=="/RECORD")
	{
		Smart(MLRecord) aR=new MLRecord;
		aR->mStop=true;
		theObjectList+=aR;
		//mRecordStack.Pop();
	}
	else if (aTag=="SETUP")
	{
		for (int aCount=0;aCount<aParams.Size();aCount++)
		{
			String aParam=aParams[aCount].GetSegmentBefore('=');
			String aValue=aParams[aCount].GetSegmentAfter('=');

			if (aParam=="padtop") mSetup.mPadTop=gMath.Eval(aValue);
			if (aParam=="padbottom") mSetup.mPadBottom=gMath.Eval(aValue);
			if (aParam=="padleft") mSetup.mPadLeft=gMath.Eval(aValue);
			if (aParam=="padright") mSetup.mPadRight=gMath.Eval(aValue);
			if (aParam=="borders") mSetup.mPadTop=mSetup.mPadBottom=mSetup.mPadLeft=mSetup.mPadRight=gMath.Eval(aValue);
			if (aParam=="vborders") mSetup.mPadTop=mSetup.mPadBottom=gMath.Eval(aValue);
			if (aParam=="hborders") mSetup.mPadLeft=mSetup.mPadRight=gMath.Eval(aValue);
			if (aParam=="caption") mSetup.mCaption=aValue;
		}
	}
	else if (aTag=="CUSTOM")
	{
		Smart(MLImage) aI=new MLImage;
		aI->mLink=mCurrentLink;
		Point aP=aParams[0].ToPointEval();
		aI->mWidth=aP.mX;
		aI->mHeight=aP.mY;
		aI->mName=aParams[1];
		aI->mName.RemoveCharacters("\"");
		if (aI->mName.Len()) aI->mID=aI->mName.c_copy();
		theObjectList+=aI;
	}
	else if (aTag=="CUSTOMFROMRECORD")
	{
		Smart(MLImage) aI=new MLImage;
		aI->mLink=mCurrentLink;
		Rect aRect=GetRecord(aParams[0]);
		if (aRect.mX<=-9999)
		{
			aRect.mX=0;
			aRect.mY=0;
			aRect.mWidth=0;
			aRect.mHeight=0;
			mCustomFromRecordList+=aI;
		}
		
		aI->mWidth=aRect.mWidth;
		aI->mHeight=aRect.mHeight;
		aI->mName=aParams[0];
		theObjectList+=aI;

		for (int aCount=0;aCount<aParams.Size();aCount++)
		{
			String aParam=aParams[aCount].GetSegmentBefore('=');
			String aValue=aParams[aCount].GetSegmentAfter('=');

			if (aParam=="expand") aI->mExtra=aValue.ToFloat();
		}
	}
	else if (aTag=="NULL")
	{
		Smart(MLImage) aI=new MLImage;
		aI->mLink=mCurrentLink;
		aI->mIsNull=true;
		aI->mWidth=.0001f;
		aI->mHeight=.0001f;
		theObjectList+=aI;
	}
	else if (aTag=="IMG")
	{
		Smart(MLImage) aI=new MLImage;
		aI->mLink=mCurrentLink;
		for (int aCount=0;aCount<aParams.Size();aCount++)
		{
			String aParam=aParams[aCount].GetSegmentBefore('=');
			String aValue=aParams[aCount].GetSegmentAfter('=');
			if (aParam[0]=='\"') aParam=aParam.GetBracedToken('\"','\"');
			if (aValue[0]=='\"') aValue=aValue.GetBracedToken('\"','\"');
			if (aValue.Len()==0) 
			{
				EnumSmartList(IMGStruct,aIMG,mIMGList)	if (aIMG->mName==aParams[0]) 
				{
					aI->mSprite=aIMG->mSprite;
					aI->mWidth=aI->mSprite->WidthF();
					aI->mHeight=aI->mSprite->HeightF();
					break;
				}
				if (!aI->mSprite)
				{
					if (aParam.StartsWith("www.")) aParam.Insert("http://",0);
					if (aParam.StartsWith("http://") || aParam.StartsWith("http:\\\\"))
					{
						aI->mSprite=GetWebImage(aParam);
						aI->mWidth=aI->mSprite->WidthF();
						aI->mHeight=aI->mSprite->HeightF();
					}
					else if (aParam.StartsWith("::")) 
					{
						aI->mSprite=GetLocalImage(aParam);
						aI->mWidth=0;//aI->mSprite->WidthF();
						aI->mHeight=0;//aI->mSprite->HeightF();
					}
					else
					{
						String aBName=aParam.GetSegmentBefore(':');
						String aIName=aParam.GetSegmentAfter(':');
						aI->mSprite=NULL;
						EnumSmartList(WebBundleStruct,aWB,mWebBundleList)
						{
							if (aWB->mName==aBName)
							{
								Smart(IMGStruct) aIMG=new IMGStruct;
								aIMG->mName=aParams[0];
								aIMG->mOwnSprite=true;
								aIMG->mSprite=new Sprite;
								aIMG->mSprite->ManualLoad(aWB->mBundle.GetTextureRef(0),aWB->mBundle.PointAtDyna(aIName));
								mIMGList+=aIMG;

								aI->mSprite=aIMG->mSprite;
								aI->mWidth=aI->mSprite->WidthF()*aI->mScale;
								aI->mHeight=aI->mSprite->HeightF()*aI->mScale;
								break;
							}
						}
					}
				}
			}
			else
			{
				if (aParam=="offset" || aParam=="off") aI->mOffset=aValue.ToPointEval();
				if (aParam=="scale") 
				{
					aI->mScale=aValue.ToFloat();
					aI->mWidth=aI->mSprite->WidthF()*aI->mScale;
					aI->mHeight=aI->mSprite->HeightF()*aI->mScale;
				}
				//
				// Params for image?
				//
			}
		}
		if (aI->mSprite)
		{
			theObjectList+=aI;
		}
	}
	else if (aTag=="bundle")
	{
		String aImage=aParams[1];
		if (aImage[0]=='\"') aImage=aImage.GetBracedToken('\"','\"');

		DownloadFile(aImage);
		String aDyna=aParams[2];
		if (aDyna.Len()==0)
		{
			aDyna=aImage;
			aDyna=aDyna.RemoveTail('.');
			aDyna+=".dyna";
		}
		String aDynaName=DownloadFile(aDyna);
		Smart(WebBundleStruct) aBundle=new WebBundleStruct;
		aBundle->mBundle.OverrideTextureSize();
		aBundle->mBundle.LoadDyna(aDynaName);
		aBundle->mName=aParams[0];
		mWebBundleList+=aBundle;
	}
	else if (aTag=="CHECKBOX")
	{
		Smart(MLCheckBox) aCB=new MLCheckBox;

		Sprite* aOn=NULL;
		Sprite* aOff=NULL;
		EnumSmartList(IMGStruct,aIMG,mIMGList) 
		{
			if (aIMG->mName==aParams[0]) aOff=aIMG->mSprite;
			if (aIMG->mName==aParams[1]) aOn=aIMG->mSprite;
		}
		if (aOn && aOff)
		{
			aCB->mCheckBox.SetSprites(*aOff,*aOn);
			aCB->mWidth=aOn->WidthF();
			aCB->mHeight=aOn->HeightF();
			for (int aCount=2;aCount<aParams.Size();aCount++)
			{
				String aParam=aParams[aCount].GetSegmentBefore('=');
				String aValue=aParams[aCount].GetSegmentAfter('=');
				if (aParam[0]=='\"') aParam=aParam.GetBracedToken('\"','\"');
				if (aValue[0]=='\"') aValue=aValue.GetBracedToken('\"','\"');
				if (aParam=="ID") aCB->mID=aValue.c_copy();
				if (aParam=="offset") aCB->mOffset=aValue.ToPoint();
			}
			theObjectList+=aCB;
		}
	}
	else if (aTag=="TEXTBOX")
	{
		Smart(MLTextBox) aTB=new MLTextBox;

		aTB->mWidth=-1;
		for (int aCount=0;aCount<aParams.Size();aCount++)
		{
			String aParam=aParams[aCount].GetSegmentBefore('=');
			String aValue=aParams[aCount].GetSegmentAfter('=');
			if (aParam[0]=='\"') aParam=aParam.GetBracedToken('\"','\"');
			if (aValue[0]=='\"') aValue=aValue.GetBracedToken('\"','\"');

			if (aParam=="ID") aTB->mID=aValue.c_copy();
			if (aParam=="DEFAULT") aTB->mTextBox.SetText(aValue);
			if (aParam=="WIDTH") aTB->mWidth=gMath.Eval(aValue);
			if (aParam=="ALLOW") aTB->mAllow=aValue;
			if (aParam=="MULTILINE") if (aValue.ToBool()) aTB->mTextBox.SetMultiLine();
		}
		aTB->mFont=mCurrentFont;
		aTB->mHeight=mCurrentFont->mPointSize;
		
		theObjectList+=aTB;
	}
	else if (aTag=="FILLLINE")
	{
		Smart(MLFillLine) aML=new MLFillLine;
		String aH=aParams[0];
		aML->mHeight=gMath.Eval(aH);
		theObjectList+=aML;
	}
	else if (aTag=="FLAG")
	{
		for (int aCount=0;aCount<aParams.Size();aCount++)
		{
			String aParam=aParams[aCount].GetSegmentBefore('=');
			String aValue=aParams[aCount].GetSegmentAfter('=');

			FlagStruct* aFS=GetFlagStruct(aParam);
			aFS->mValue=aValue;
		}
	}
	else if (aTag=="CONFINE") 
	{
		Smart(MLMoveCursor) aML=new MLMoveCursor;
		aML->mType=ML_CONFINE;

		String aXString=aParams[0];
		String aYString=aParams[1];
		if (aYString.Len()==0) aYString="100%";

		if (aXString.Contains((char)37)) aML->mFlags|=MLFLAG_XISPERCENT;
		if (aYString.Contains((char)37)) aML->mFlags|=MLFLAG_YISPERCENT;
		aXString.RemoveCharacters("%%");
		aYString.RemoveCharacters("%%");
		aML->mMove.mX=gMath.Eval(aXString);
		aML->mMove.mY=gMath.Eval(aYString);
		theObjectList+=aML;
	}
	else if (aTag=="/CONFINE")  {Smart(MLObject) aBr=new MLObject;aBr->mType=ML_ENDCONFINE;theObjectList+=aBr;}
	else if (aTag=="PUSHCURSOR")  {Smart(MLObject) aBr=new MLObject;aBr->mType=ML_PUSHCURSOR;theObjectList+=aBr;}
	else if (aTag=="POPCURSOR")  {Smart(MLObject) aBr=new MLObject;aBr->mType=ML_POPCURSOR;theObjectList+=aBr;}
	else if (aTag=="SPACE" || aTag=="SP")
	{
		Smart(MLSpace) aML=new MLSpace;
		aML->mSpace=gMath.Eval(aParams[0]);
		theObjectList+=aML;
	}
	else if (aTag=="MOVECURSOR")
	{
		Smart(MLMoveCursor) aML=new MLMoveCursor;

		if (aParams[0]=="end") aML->mFlags|=MLFLAG_END;
		else if (aParams[0]=="bottom") 
		{
			aML->mMove.mX=0;
			aML->mMove.mY=100;
			aML->mFlags|=MLFLAG_YISPERCENT;
		}
		else
		{
			String aXString=aParams[0].GetToken(',');
			String aYString=aParams[0].GetNextToken(',');

			if (aXString.Contains((char)37)) aML->mFlags|=MLFLAG_XISPERCENT;
			if (aYString.Contains((char)37)) aML->mFlags|=MLFLAG_YISPERCENT;
			if (aXString.StartsWith("-") || aXString.StartsWith("+")) aML->mFlags|=MLFLAG_XISRELATIVE;
			if (aYString.StartsWith("-") || aYString.StartsWith("+")) aML->mFlags|=MLFLAG_YISRELATIVE;
			if (aXString.StartsWith("+")) aXString=aXString.GetSegment(1,aXString.Len());
			aXString.RemoveCharacters("%%");
			aYString.RemoveCharacters("%%");
			aML->mMove.mX=gMath.Eval(aXString);
			aML->mMove.mY=gMath.Eval(aYString);
		}
		theObjectList+=aML;
	}
	else if (aTag=="os")
	{
		//if (aParams[0]=="ios") aParams[0]=gAppPtr->GetOSName();
		if (aParams[0]!=gAppPtr->GetOSName())
		{
			mSkipOSTags=true;
		}
	}
	else if (aTag=="/os") 
	{
		//
		// Just repeated here so we don't get the gOut saying we don't know this tag!
		//
		mSkipOSTags=false;
	}
	else
	{
		gOut.Out("??? Tag: %s -> %s",aTag.c(),aParams[0].c());
	}
}


void MLBox::DrawBackground()
{
	gG.PushTranslate();
	gG.Translate(mScrollX,mScrollY);
	gG.SetColor(mBKGColor);
	gG.FillRect(0,0,mWidth,mHeight);
	gG.SetColor();
	gG.PopTranslate();
}

void MLBox::Draw()
{
	ThreadLock(mThreadLock);
	DrawBackground();

	if (mReadyToDisplay)
	{
		gG.SetColor(mDefaultColor);
		Rect aVisible=GetVisibleArea();
		EnumSmartList(MLObject,aO,mObjectList)
		{
			if (aO->IsInArea(aVisible) || !aO->IsVisible()) 
			{
				aO->Draw();
			}
		}
	}
	else 
	{
		DrawPending();
	}

	gG.SetColor();
	ThreadUnlock(mThreadLock);
}

void MLText::Draw()
{
	mFont->Draw(mText,mX,mY+mFont->mAscent);
}

void MLImage::Draw()
{
	if (mIsNull) return;
	gG.PushColor();

	if (mSprite) 
	{
		if (mScale==1.0f) mSprite->Draw(mX+mOffset.mX,mY+mOffset.mY);
		else
		{
			mSprite->DrawScaled(mX+(mSprite->HalfWidthF()*mScale)+mOffset.mX,mY+(mSprite->HalfHeightF()*mScale)+mOffset.mY,mScale);
		}
	}
	else
	{
		//
		// Custom Image
		//
		gG.PushTranslate();
		gG.PushClip();
		gG.Translate(mX,mY);
		gG.ClipInto(mX,mY,mWidth,mHeight);
		Point aHold=gG.mTranslate;
		if (mBox) mBox->DrawCustom(mName,this);
		Point aDiff=gG.mTranslate-aHold;
		gG.PopClip();
		gG.PopTranslate();
		gG.Translate(aDiff);
	}
	gG.PopColor();
}

void MLFillLine::Draw()
{
	gG.FillRect(mBox->mSetup.mPadLeft,mY,mBox->mWidth-(mBox->mSetup.mPadLeft+mBox->mSetup.mPadRight),mHeight);
}

void MLBox::Layout()
{
	Rect aRect=*this;
	aRect.mX=aRect.mY=0;
	aRect.mHeight=0;

	Stack<Point> aCursorStack;
	Stack<Rect> aClipStack;

	float aMaxY=0;

	Rect aClip=Rect(mSetup.mPadLeft,mSetup.mPadTop,mWidth-mSetup.mPadLeft-mSetup.mPadRight,999999);
	Point aCursor=aClip.UpperLeft();
	short aAlignment=-1;

	Smart(MLObject) aPrevious=new MLObject;

	Smart(MLObject) aLineBreak=new MLObject;aLineBreak->mType=ML_LINEBREAK;aLineBreak->mFlags|=MLFLAG_SETUP;
	mObjectList.Insert(aLineBreak,0);
	aLineBreak=new MLObject;aLineBreak->mType=ML_LINEBREAK;aLineBreak->mFlags|=MLFLAG_SETUP;
	mObjectList+=aLineBreak;

#define ADDBREAK {Smart(MLObject) aLineBreak=new MLObject;aLineBreak->mType=ML_LINEBREAK;mObjectList.Insert(aLineBreak,aCount);aCount++;}

	for (int aCount=1;aCount<mObjectList.GetCount();aCount++)
	{
		if (mAbort) break;
		Smart(MLObject) aO=mObjectList[aCount];

		switch (aO->mType)
		{
		case ML_RECORD:
			{
				Smart(MLRecord) aR=aO;
				if (aR->mStop) mRecordStack.Pop();
				else
				{
					Smart(RecordStruct) aRS=new RecordStruct;
					aRS->mName=aR->mName;
					aRS->mHash=aRS->mName.GetHash();
					aRS->mRect=Rect(0,0,-1,-1);
					mRecordList+=aRS;
					mRecordStack.Push(aRS);
				}
			}
			break;
		case ML_TEXT:
			{
				Smart(MLText) aOT=aO;
				mCurrentFont=aOT->mFont;
				if (!mCurrentFont) mCurrentFont=mDefaultFont;
				aOT=aPrevious;
				if (aPrevious->mType==ML_TEXT && aPrevious->mY==aCursor.mY) 
				{
					aCursor.mX+=aOT->mFont->GetCharacter(32)->mWidth;
					//aPrevious->mWidth+=aOT->mFont->mCharacter[32].mWidth;
				}
			}
			break;
		case ML_CHECKBOX:
			{
				Smart(MLCheckBox) aCB=aO;
				mHelperList+=&aCB->mCheckBox;
			}
			break;
		case ML_TEXTBOX:
			{
				Smart(MLTextBox) aTB=aO;
				mCurrentFont=aTB->mFont;
				if (!mCurrentFont) mCurrentFont=mDefaultFont;
				if (aTB->mWidth==-1)
				{
					float aEndPos=(aClip.mX+aClip.mWidth);
					aTB->mWidth=aEndPos-aCursor.mX;
				}
				aTB->mTextBox.Size(0,0,aTB->mWidth+10,aTB->mHeight+10);
				//aTB->mTextBox.SetBackgroundMagicSprite(&gBundle_UI->mCorner);
				//mTextBox.SetBackgroundMagicSprite(&CurrentBundle().mTextboxBKG);
				aTB->mTextBox.SetBackgroundColor(Color(1,1,1,0));
				aTB->mTextBox.SetFont(mCurrentFont);
				//aTB->mTextBox.SetTextColor(Color(0,0,0,1));
				aTB->mTextBox.SetBorder(5);
				aTB->mTextBox.SetTextOffset(Point(0,-5));
				aTB->mTextBox.SetCursorOffset(Point(0,5));
				aTB->mTextBox.SetCursorWidth(5);
				//aTB->mTextBox.mID=ML_TEXTBOX;	// So the CPU points back at a textbox...
				if (aTB->mAllow.Len()) 
				{
					aTB->mTextBox.AllowCharacters(aTB->mAllow);
					aTB->mAllow="";
				}
				mHelperList+=&aTB->mTextBox;
			}
			break;
		case ML_SPACE:
			{
				Smart(MLSpace) aSP=aO;
				aCursor.mX+=aSP->mSpace;
				if (aPrevious->mType!=ML_TEXTBOX) aPrevious->mWidth+=aSP->mSpace;
			}
			break;
		case ML_ALIGN:{Smart(MLAlign) aA=aO;aAlignment=aA->mAlign;}break;
		case ML_LINEBREAK:{if (!aO->IsSetup()) aO->mWidth=0;aO->mHeight=mCurrentFont->mPointSize;}break;
		case ML_CR:aCursor.mX=aClip.mX;break;
		case ML_LF:{aCursor.mY+=mCurrentFont->mPointSize;ADDBREAK;}break;
		case ML_MOVECURSOR:
			{
				Smart(MLMoveCursor) aML=aO;
				Point aMove=aML->mMove;
				if (aO->mFlags&MLFLAG_END) 
				{
					Point aEnd;
					EnumSmartList(MLObject,aCheckO,mObjectList)
					{
						//if (aCheckO->mType==ML_LINEBREAK || aCheckO->mType==ML_CR) aEnd.mX=aClip.mX;
						if (aCheckO->UpperRight().mY>=aEnd.mY)
						{
							aEnd.mX=_max(aEnd.mX,aCheckO->UpperRight().mX);
							aEnd.mY=_max(aEnd.mY,aCheckO->UpperRight().mY);
						}
					}
					aCursor=aEnd;
				}
				else
				{
					if (aO->mFlags&MLFLAG_XISPERCENT) aMove.mX=(aML->mMove.mX/100.0f)*aClip.mWidth;
					if (aO->mFlags&MLFLAG_YISPERCENT) aMove.mY=(aML->mMove.mY/100.0f)*aMaxY;
					if (aO->mFlags&MLFLAG_XISRELATIVE) aCursor.mX+=aMove.mX;
					else aCursor.mX=aClip.mX+aMove.mX;
					if (aO->mFlags&MLFLAG_YISRELATIVE) aCursor.mY+=aMove.mY;
					else aCursor.mY=aClip.mY+aMove.mY;
				}
				if (aMove.mX) {ADDBREAK;}
			}
			break;
		case ML_PUSHCURSOR:aCursorStack.Push(aCursor);break;
		case ML_POPCURSOR:if (!aCursorStack.IsEmpty()) aCursor=aCursorStack.Pop();ADDBREAK;break;
		case ML_CONFINE:
			{
				aClipStack.Push(aClip);Smart(MLMoveCursor) aML=aO;Point aMove=aML->mMove;float aLeft;float aRight;
				if (aO->mFlags&MLFLAG_XISPERCENT) aMove.mX=(aML->mMove.mX/100.0f)*aClip.mWidth;
				if (aO->mFlags&MLFLAG_YISPERCENT) aMove.mY=(aML->mMove.mY/100.0f)*aClip.mWidth;
				aLeft=aClip.mX+aMove.mX;aRight=aClip.mX+aMove.mY;ADDBREAK;aClip=Rect(aLeft,0,aRight-aLeft,999999);aCursor.mX=aLeft;
			}
			break;
		case ML_ENDCONFINE:
			{
				aClip=aClipStack.Pop();ADDBREAK;
				aCursor.mX=aClip.mX;aCursor.mY=aMaxY;
			}
			break;
		case ML_FILLLINE:
			{
				ADDBREAK;
			}
			break;
		}

		if ((aCursor.mX+aO->mWidth)>(aClip.mX+aClip.mWidth) || aO->mType==ML_LINEBREAK)
		{
//			aPrevious=new MLObject;
			float aMaxY=aCursor.mY+mCurrentFont->mPointSize;

			int aCountBack=aCount;
			int aCountTo=aCount;
			if (aO->mType==ML_LINEBREAK) 
			{
				aCountBack--;
				aCountTo--;
			}


			for (;aCountBack>=0;aCountBack--) {if (mObjectList[aCountBack]->mType==ML_LINEBREAK) break;aMaxY=_max(aMaxY,mObjectList[aCountBack]->LowerRight().mY);}
			for (int aCountForward=aCountBack;aCountForward<=aCountTo;aCountForward++) if (mObjectList[aCountForward]->mType!=ML_FILLLINE) mObjectList[aCountForward]->mY=aMaxY-mObjectList[aCountForward]->mHeight;

			if (aAlignment!=-1)
			{
				float aStartX=99999;
				float aEndX=-99999;
				//
				// FIXME: 
				// This next line messes up...
				// If I have only one thing in the group, it misses it with <aCountTo
				//
				for (int aCountForward=aCountBack;aCountForward<=aCountTo;aCountForward++)
				{
					if (mObjectList[aCountForward]->IsVisible() && mObjectList[aCountForward]->IsSetup())
					{
						aStartX=_min(aStartX,mObjectList[aCountForward]->mX);
						aEndX=_max(aEndX,mObjectList[aCountForward]->LowerRight().mX);

/*
						gOut.Out("Whut[%d]: %f,%f -> %f,%f",mObjectList[aCountForward]->mType,mObjectList[aCountForward]->mX,mObjectList[aCountForward]->LowerRight().mX,aStartX,aEndX);
						if (mObjectList[aCountForward]->mType==ML_TEXT)
						{
							Smart(MLText) aOT=mObjectList[aCountForward];
							gOut.Out(" Text: [%s]",aOT->mText.c());
						}
*/
					}
				}
				float aWidth=_max(0,aEndX-aStartX);

				if (aAlignment==0)
				{
					float aTargetX=aClip.CenterX()-(aWidth/2);
					float aMoveX=aTargetX-aStartX;
					for (int aCountForward=aCountBack;aCountForward<=aCountTo;aCountForward++) mObjectList[aCountForward]->mX+=aMoveX;
				}
				if (aAlignment==1)
				{
					float aTargetX=aClip.LowerRight().mX-(aWidth);
					float aMoveX=aTargetX-aStartX;
					for (int aCountForward=aCountBack;aCountForward<=aCountTo;aCountForward++) mObjectList[aCountForward]->mX+=aMoveX;
				}
			}

			//
			// If we're centering or right aligning, process that here, using CountForward!
			//

			aCursor.mX=aClip.mX;
			aCursor.mY=aMaxY; // Here's the problem, sigh... align center, our cursor.mY gets put at aMaxY...
			if (aO->mType!=ML_LINEBREAK)
			{
				ADDBREAK;
			}
		}

		aO->mX=aCursor.mX;
		aO->mY=aCursor.mY;
		/*
		if (aFixList.GetCount())
		{
			EnumList(CPU,aCPU,aFixList)
			{
				aCPU->mX+=aO->mX;
				aCPU->mY+=aO->mY;
				*this+=aCPU;
				mCPUList+=aCPU;
			}
		}
		*/

		if (!mRecordStack.IsEmpty())
		{
			for (int aCount=0;aCount<mRecordStack.mStackPos;aCount++)
			{
				mRecordStack[aCount]->mObjectList+=aO;
/*
				if (aO->mWidth>0 && aO->mHeight>0)
				{
					if (mRecordStack[aCount]->mRect.mWidth<0) mRecordStack[aCount]->mRect=Rect(aO->mX,aO->mY,1,1);
					mRecordStack[aCount]->mRect=mRecordStack[aCount]->mRect.Union(*aO);
				}
/**/
			}
		}

		aO->mFlags|=MLFLAG_SETUP;
		aMaxY=_max(aMaxY,aO->LowerLeft().mY);
		aCursor.mX+=_max(0,aO->mWidth);
		if (!(aO->mFlags&MLFLAG_INVISIBLE)) aPrevious=aO;
	}

	//
	// Possible Optimization:
	// Join up adjacent text thingies that have the same font and color.
	//

	EnumSmartList(MLObject,aO,mObjectList)
	{
		if (mAbort) break;
		if (aO->mLink) 
		{
			Button* aB=new Button;
			aB->Size(aO->Expand(3));
			aB->SetNotify(this);
			aB->DragThru();
			aB->mDragThruIgnoreTouchEnd=false;
			aB->ExtraData()=aO->mLink.GetPointer();
			aO->mLink->mButton=aB;
			mLinkButtonList+=aB;
			*this+=aB;
		}
		if (aO->IsVisible()) aRect=aRect.Union(*aO);

		CPU* aFixCPU=NULL;
		Point aFixOffset;
		switch (aO->mType)
		{
		case ML_LINEBREAK:
		case ML_CR:
		case ML_LF:
		case ML_ALIGN:
		case ML_MOVECURSOR:
		case ML_PUSHCURSOR:
		case ML_POPCURSOR:
		case ML_CONFINE:
		case ML_ENDCONFINE:
		case ML_RECORD:
			mObjectList-=aO;
			EnumSmartListRewind(MLObject);
			break;
		case ML_TEXTBOX:{Smart(MLTextBox) aOO=aO;aFixCPU=&aOO->mTextBox;}break;
		case ML_CHECKBOX:{Smart(MLCheckBox) aOO=aO;aFixCPU=&aOO->mCheckBox;aFixOffset=aOO->mOffset;}break;
		}
		if (aFixCPU)
		{
			aFixCPU->mX+=aO->mX+aFixOffset.mX;
			aFixCPU->mY+=aO->mY+aFixOffset.mY;
			*this+=aFixCPU;
			mCPUList+=aFixCPU;
		}
	}
//*
	EnumSmartList(RecordStruct,aRS,mRecordList)
	{
		aRS->mRect=Rect(-1,-1,-1,-1);
		EnumSmartList(MLObject,aO,aRS->mObjectList)
		{
			if (aO->mWidth<0) continue;
			if (aO->mType==ML_SPACE) continue;
			if (aRS->mRect.mWidth<0) aRS->mRect=Rect(aO->mX,aO->mY,1,1);
			aRS->mRect=aRS->mRect.Union(*aO);
		}
		aRS->mObjectList.Clear();
	}
/**/
	if (aRect.mHeight>mHeight) aRect.mHeight+=mSetup.mPadBottom;
	SetSwipeSize(aRect.mWidth,aRect.mHeight);
}

String MLBox::DownloadFile(String theURL)
{
	String aWork=theURL;
	aWork.DeleteI("http://");
	aWork.DeleteI("http:\\\\");
	aWork.Replace('/','_');
	aWork.Replace(':','_');
	aWork.Insert(Sprintf("v%s_",mVersion.c()));
	String aLocalURL=Sprintf("temp://web\\%s",aWork.c());

	if (aLocalURL.GetTail('.')=="miximage")
	{
		String aFN=aLocalURL.RemoveTail('.');
		if (DoesImageExist(aFN)) return aLocalURL;
	}

	if (!DoesFileExist(aLocalURL))
	{
        if (!theURL.StartsWith("hhtp")) theURL.Insert("http://");
		IOBuffer aBuffer;
		aBuffer.Download(theURL);
		MakeDirectory("temp://web\\");
		aBuffer.CommitFile(aLocalURL);

#ifdef _DEBUG
		gOut.Out("Save Local File: %s [%d]",aLocalURL.c(),aBuffer.Len());
#endif
	}
	return aLocalURL;
}

Sprite* MLBox::GetWebImage(String theURL)
{
	String aLocalURL=DownloadFile(theURL);

	aLocalURL.DeleteI(".png");
	aLocalURL.DeleteI(".jpg");
	aLocalURL.DeleteI(".gif");

	Smart(WebImageStruct) aW=new WebImageStruct;
	mWebImageList+=aW;
	aW->mSprite=new Sprite;

	Image aImage;
	aImage.Load(aLocalURL);
	int aTex=gG.LoadTexture(aImage);
	aW->mSprite->ManualLoad(aTex,Rect(0,0,(float)aImage.mWidth,(float)aImage.mHeight),aImage.mWidth,aImage.mHeight,(float)aImage.mWidth,(float)aImage.mHeight,0,0);
	return aW->mSprite;
}

Sprite* MLBox::GetLocalImage(String theURL)
{
	String aLocalURL="package://";aLocalURL+=theURL.GetSegmentAfter("::");

	aLocalURL.DeleteI(".png");
	aLocalURL.DeleteI(".jpg");
	aLocalURL.DeleteI(".gif");

	Smart(WebImageStruct) aW=new WebImageStruct;
	mWebImageList+=aW;
	aW->mSprite=new Sprite;

	Image aImage;
	aImage.Load(aLocalURL);
	int aTex=gG.LoadTexture(aImage);
	aW->mSprite->ManualLoad(aTex,Rect(0,0,(float)aImage.mWidth,(float)aImage.mHeight),aImage.mWidth,aImage.mHeight,(float)aImage.mWidth,(float)aImage.mHeight,0,0);
	return aW->mSprite;
}

void MLBox::Notify(void* theData)
{
	EnumList(Button,aB,mLinkButtonList)
	{
		if (theData==aB)
		{
			MLLink* aLink=(MLLink*)aB->ExtraData();
			mClickedLink=aLink;
			String aCommand=aLink->mCommand;
			if (aCommand.StartsWith("\"")) aCommand=aCommand.GetBracedToken('\"','\"');

			if (aLink->mIsWebLink) {if (!ClickWebLink(aCommand)) 
			{
				OS_Core::Execute(aCommand);}
                gAppPtr->Minimize();
			}
			else ClickLink(aCommand);
			return;
		}
	}
}

void MLBox::ClearContents()
{
	EnumList(Button,aB,mLinkButtonList) aB->Kill();
	EnumList(Button,aT,mHelperList) *this-=aT;
	mLinkButtonList.Clear();
	mHelperList.Clear();
	mCPUList.Clear();
	mObjectList.Clear();
	mRecordList.Clear();

	mCurrentLink=NULL;
	SetSwipeSize(0,0);
}

bool MLBox::ClickLink(String theCommand)
{
	if (theCommand.StartsWith("http:\\\\") || theCommand.StartsWith("http://") || theCommand.StartsWith("www."))
	{
		ClearContents();
		Load(theCommand);
		return true;
	}
	else if (mLinkNotify)
	{
		mLinkNotify->mNotifyData.mParam=NOTIFY_LINK;
		mLinkNotify->mNotifyData.mExtraData=&theCommand;
		mLinkNotify->Notify(&mLinkNotify->mNotifyData);
	}
	return false;
}

Rect MLBox::GetRecord(String theName)
{
	EnumSmartList(RecordStruct,aRS,mRecordList) if (aRS->mName==theName) return aRS->mRect;
	return Rect(-9999,-9999,-9999,-9999);
}

Rect MLBox::GetRecord(int theHash)
{
	EnumSmartList(RecordStruct,aRS,mRecordList) if (aRS->mHash==theHash) return aRS->mRect;
	return Rect(-9999,-9999,-9999,-9999);
}

bool MLBox::IsFlagSet(String theName)
{
	EnumSmartList(FlagStruct,aFS,mFlagList) if (aFS->mName==theName) return true;
	return false;
}

String  MLBox::GetFlagValue(String theName)
{
	EnumSmartList(FlagStruct,aFS,mFlagList) if (aFS->mName==theName) return aFS->mValue;
	return "";
}

MLBox::FlagStruct* MLBox::GetFlagStruct(String theName)
{
	EnumSmartList(FlagStruct,aFS,mFlagList) if (aFS->mName==theName) return aFS;
	Smart(FlagStruct) aFS=new FlagStruct;
	mFlagList+=aFS;

	aFS->mName=theName;
	return aFS;
}

void MLBox::Define(String theOriginal, String theReplace)
{
	Smart(DefineStruct) aDS;
	EnumSmartList(DefineStruct,aDSCheck,mDefineList) if (aDSCheck->mFrom==theOriginal) {aDS=aDSCheck;break;}
	if (!aDS) 
	{
		aDS=new DefineStruct;
		mDefineList+=aDS;
	}
	aDS->mFrom=theOriginal;
	aDS->mTo=theReplace;
}

String MLBox::GetDefine(String theDefine)
{
	EnumSmartList(DefineStruct,aDSCheck,mDefineList) if (aDSCheck->mFrom==theDefine) {return aDSCheck->mTo;}
	return "";
}


MLTextBox::~MLTextBox() 
{
	//
	// Textbox got removed in ClearContents
	//
}

void MLTextBox::Draw()
{
}

Smart(MLObject) MLBox::GetObjectByID(String theID)
{
	EnumSmartList(MLObject,aO,mObjectList) if (aO->mID) if (theID==aO->mID) return aO;
	return NULL;
}

