#include "rapt_sprite.h"
#include "rapt.h"

//#define DEBUG_DYNAMIC

IPoint	gOverrideTextureSize;

#include "os_core.h"

Array<Vertex2D> gBillboardV;
Array<Vertex2DMT> gBillboardVMT;
Array<vbindex> gBillboardI;

SpriteBundle::SpriteBundle()
{
	mLoadCount=0;
	mIsLoaded=false;
	mIsThreadLoading=false;
	mIsInLoadingThread=false;
	mData=NULL;

	Array<char> aTest;
	aTest[3]=1;

	if (gBillboardI.Size()==0)
	{
		gBillboardI[0]=0;
		gBillboardI[1]=1;
		gBillboardI[2]=2;
		gBillboardI[3]=3;
		gBillboardI[4]=2;
		gBillboardI[5]=1;

		gBillboardV[3].mX=0;
	}
}

SpriteBundle::~SpriteBundle()
{
	Unload();
}

void SpriteBundle::OverrideTextureSize(int theX, int theY) 
{
	gOverrideTextureSize.mX=theX;
	gOverrideTextureSize.mY=theY;//+gG.KludgeMipMaps();
}

void SpriteBundle::OverrideTextureSize() 
{
	gOverrideTextureSize=IPoint(0,0);
}

bool DoesPrefixedImageExist(String theFilename)
{
	if (DoesImageExist(theFilename)) return true;
	String a4444=InsertFilePrefix(theFilename,"4444-");if (DoesImageExist(a4444)) return true;
	String a565=InsertFilePrefix(theFilename,"565-");if (DoesImageExist(a565)) return true;
	return false;
}

bool SpriteBundle::Load(String theBundleName, bool setIsLoaded)
{
	if (++mLoadCount>1) return false;


	if (mIsThreadLoading && !mIsInLoadingThread)	// If we're threading, just let it finish.
	{
		while (mIsThreadLoading) Sleep(25);
		return mIsLoaded;
	}

	//
	// First, we look for a Dyna... we'll actually look in sandbox/dyna for this.   
	//

	//String aSearchDir=PointAtImages("");

#ifdef DEBUGBUNDLES
	gOut.Out("LoadBundle: %s",theBundleName.c());
	mBundleName=theBundleName;
#endif

	int aCount=-1;
	for (;;)
	{
		String aFilename;
		//if (aCount==-1) {aFilename=Sprintf("%s\\%s",aSearchDir.c(),theBundleName.c());aCount=0;}
		//else aFilename=Sprintf("%s\\%s-%d",aSearchDir.c(),theBundleName.c(),aCount);
		if (aCount==-1) {aFilename="images://";aFilename+=theBundleName;aCount=0;}
		else {aFilename=Sprintf("images://%s-%d",theBundleName.c(),aCount);}

		if (!DoesPrefixedImageExist(aFilename) && aCount>=0) break;
        
#ifdef DEBUGBUNDLES
		gOut.Out("LoadTexture (Start)");
#endif

		mTextureList[aCount]=gG.LoadTexture(aFilename);
		//mTextureSizeList[aCount]=gG.GetTextureDimensions(mTextureList[aCount]);

#ifdef DEBUGBUNDLES
		gOut.Out("LoadTexture (OK)");
#endif

		aCount++;
	}
	if (aCount<=0)
	{
        aCount=-1;
		//
		// Didn't load?  Check lowercase, for our case-sensitive systems...
		//
		theBundleName=theBundleName.ToLower();
		for (;;)
		{
			String aFilename;
			//if (aCount==-1) {aFilename=Sprintf("%s\\%s",aSearchDir.c(),theBundleName.c());aCount=0;}
			//else aFilename=Sprintf("%s\\%s-%d",aSearchDir.c(),theBundleName.c(),aCount);
			if (aCount==-1) {aFilename="images://";aFilename+=theBundleName;aCount=0;}
			else {aFilename=Sprintf("images://%s-%d",theBundleName.c(),aCount);}

			if (!DoesPrefixedImageExist(aFilename) && aCount>=0) break;

#ifdef DEBUGBUNDLES
			gOut.Out("LoadTexture (Start)");
#endif

			mTextureList[aCount]=gG.LoadTexture(aFilename);

#ifdef DEBUGBUNDLES
			gOut.Out("LoadTexture (OK)");
#endif

			aCount++;
		}
	}

	if (setIsLoaded) mIsLoaded=true;

#ifdef _DEBUGBUNDLES
    gOut.Out("Loaded: %s [%d]",theBundleName.c(),aCount);
#endif
    
	return (aCount>0);
}

IPoint SpriteBundle::GetSize(int theTexNumber)
{
	if (mTextureSizeList.Size()==0)
	{
		mTextureSizeList.GuaranteeSize(mTextureSizeList.Size());
		for (int aCount=0;aCount<mTextureList.Size();aCount++) mTextureSizeList[aCount]=gG.GetTextureDimensions(mTextureList[aCount]);
	}
	return mTextureSizeList[theTexNumber];
}




bool SpriteBundle::LoadManual(String theFilename)
{
	if (++mLoadCount>1) return false;
	if (mIsThreadLoading && !mIsInLoadingThread)	// If we're threading, just let it finish.
	{
		while (mIsThreadLoading) Sleep(25);
		return mIsLoaded;
	}

	mTextureList.Reset();
	mTextureList[0]=gG.LoadTexture(theFilename);
	mIsLoaded=(mTextureList[0]!=-1);
	return mIsLoaded;
}

bool SpriteBundle::LoadDynaDataOnly(String theDynaName)
{
	//
	// If we're already loaded with the texture, we don't need to do any of this.
	//
	if (mLoadCount>0) return false; // Didn't load-- already loaded!
	if (mIsThreadLoading && !mIsInLoadingThread)	// If we're threading, just let it finish.
	{
		while (mIsThreadLoading) Sleep(25);
		return mIsLoaded;
	}

	if (!theDynaName.ContainsI(".dyna")) theDynaName+=".dyna";
	if (DoesFileExist(theDynaName))
	{
		//
		// Read in dyna data... then the png... then we're done!
		//
		mData=new IOBuffer;
		mData->Load(theDynaName);

		while (!mData->IsEnd())
		{
			String aName=mData->ReadString();
			int aSkipTo=mData->ReadInt();
			int aSeek=mData->Tell();

			Smart(DynaData) aDS=new DynaData;
			aDS->mName=aName;
			aDS->mSeek=aSeek;
			mDynaList+=aDS;

			mData->Seek(aSkipTo);
		}
		return true;
	}
	return false;
}

void SpriteBundle::UnloadDynaDataOnly()
{
	// 
	// We only unload the data if our loadcount == 0, indicating that it was loaded with LoadDynaDataOnly.
	// Otherwise, loadcount would be increased.
	//
	if (mLoadCount==0) UnloadData();
}


bool SpriteBundle::LoadDyna(String theDynaName, bool setIsLoaded)
{
	if (++mLoadCount>1) return false;
	if (mIsThreadLoading && !mIsInLoadingThread)	// If we're threading, just let it finish.
	{
		while (mIsThreadLoading) Sleep(25);
		return mIsLoaded;
	}

	bool aLoadedIt=false;

	//
	// Load it...
	//
	if (!theDynaName.ContainsI(".dyna")) theDynaName+=".dyna";
	if (DoesFileExist(theDynaName))
	{
		//
		// Read in dyna data... then the png... then we're done!
		//
		mData=new IOBuffer;
		mData->Load(theDynaName);

		while (!mData->IsEnd())
		{
			String aName=mData->ReadString();
			int aSkipTo=mData->ReadInt();
			int aSeek=mData->Tell();

			Smart(DynaData) aDS=new DynaData;
			aDS->mName=aName;
			aDS->mSeek=aSeek;
			mDynaList+=aDS;

			mData->Seek(aSkipTo);

/*
			mData->ReadRect();
			mData->ReadInt();
			mData->ReadInt();
			mData->ReadFloat();
			mData->ReadFloat();
			mData->ReadFloat();
			mData->ReadFloat();
			mData->ReadBool();
			int aKeyCount=mData->ReadInt();
			for (int aCount=0;aCount<aKeyCount;aCount++) mData->ReadPoint();
*/
		}

		String aFName=theDynaName;
		aFName.DeleteI(".dyna");
		mTextureList[0]=gG.LoadTexture(aFName);
		aLoadedIt=true;
	}
	else
	{
#ifdef _DEBUG
		gOut.Out("Dyna Bundle not found: [ %s ]",theDynaName.c());
#endif
	}

	if (setIsLoaded) mIsLoaded=true;
	return aLoadedIt;
}


bool SpriteBundle::GetDynaFile(char* theDyna, IOBuffer& theBuffer)
{
	IOBuffer& aBuffer=PointAtDyna(theDyna);
	if (!aBuffer.IsEnd()) 
	{
		aBuffer.ReadBuffer(theBuffer);
		return true;
	}
	return false;
}

bool SpriteBundle::LoadDyna(IOBuffer& theDynaBuffer, Image& theImage, bool setIsLoaded)
{
	if (++mLoadCount>1) return false;
	if (mIsThreadLoading && !mIsInLoadingThread)	// If we're threading, just let it finish.
	{
		while (mIsThreadLoading) Sleep(25);
		return mIsLoaded;
	}

	bool aLoadedIt=false;

	//
	// Load it...
	//
	if (theDynaBuffer.Len())
	{
		//
		// Read in dyna data... then the png... then we're done!
		//
		mData=new IOBuffer;
		mData->Copy(&theDynaBuffer);

		while (!mData->IsEnd())
		{
			String aName=mData->ReadString();
			int aSkipTo=mData->ReadInt();
			int aSeek=mData->Tell();

			Smart(DynaData) aDS=new DynaData;
			aDS->mName=aName;
			aDS->mSeek=aSeek;
			mDynaList+=aDS;

			mData->Seek(aSkipTo);
		}

		mTextureList[0]=gG.LoadTexture(theImage);
		aLoadedIt=true;
	}

	if (setIsLoaded) mIsLoaded=true;
	return aLoadedIt;
}


void SpriteBundle::LoadThreadedStub(void* theArg)
{
	SpriteBundle* theBundle=(SpriteBundle*)theArg;
    
    gG.Threading();
	theBundle->mIsInLoadingThread=true;
	theBundle->mIsThreadLoading=true;
	theBundle->Load();
	theBundle->mIsThreadLoading=false;
	theBundle->mIsInLoadingThread=false;
    
}

void SpriteBundle::LoadThreaded()
{
//    Load();
	OS_Core::Thread(&LoadThreadedStub,this);
}


void SpriteBundle::LoadData(String theBundleName)
{
	if (mData) return;
    
    gG.ThreadLock(true);
	//String aSearchDir=PointAtImages("");
	//String aFilename=Sprintf("%s\\%s.bundle",aSearchDir.c(),theBundleName.c());
	//String aFilename=PointAtImages(Sprintf("%s.bundle",theBundleName.c()));
	String aFilename=Sprintf("images://%s.bundle",theBundleName.c());

	if (DoesFileExist(aFilename))
	{
		mData=new IOBuffer;
		mData->Load(aFilename);
	}
	else
	{
		theBundleName=theBundleName.ToLower();
		//String aFilename=Sprintf("%s\\%s.bundle",aSearchDir.c(),theBundleName.c());
		//String aFilename=PointAtImages(Sprintf("%s.bundle",theBundleName.c()));
		String aFilename=Sprintf("images://%s.bundle",theBundleName.c());

		if (DoesFileExist(aFilename))
		{
			mData=new IOBuffer;
			mData->Load(aFilename);
		}
	}

	// kittyengine asset existence guard: the generated Bundler code below reads
	// sprite metadata straight off *mData, so an absent .bundle used to be a
	// null dereference -- a silent crash at boot with nothing on stderr.  An
	// empty buffer instead leaves every sprite at its default and the game runs.
	if (!mData) {gOut.Out("SpriteBundle: no bundle data for %s -- running without it",theBundleName.c());mData=new IOBuffer;}
    gG.ThreadLock(false);
}

void SpriteBundle::LoadDataManual(String theFileName)
{
	if (mData) return;

	String aFilename=Sprintf("%s.bundle",theFileName.c());
	if (DoesFileExist(aFilename))
	{
		mData=new IOBuffer;
		mData->Load(aFilename);
	}
}

bool SpriteBundle::IsRetina(String theName)
{
    if (!gAppPtr->IsRetinaDevice()) return false;
	//String aSearchDir=PointAtImages("");
    //String aFilename=Sprintf("%s\\%s@2X.bundle",aSearchDir.c(),theName.c());
	String aFilename=Sprintf("images://%s@2X.bundle",theName.c());
    if (!DoesFileExist(aFilename)) return false;
    
    //aFilename=Sprintf("%s\\%s@2X",aSearchDir.c(),theName.c());
	aFilename=Sprintf("images://%s@2X",theName.c());
    if (!DoesImageExist(aFilename)) return false;
    return true;
}

void SpriteBundle::Unload(bool doReset)
{
	//
	// If the loading thread is happening, wait, so that we don't destroy anything...
	//
	if (!gAppPtr || gAppPtr->IsQuit()) return;
	if (mLoadCount<=0) return;
	while (mIsThreadLoading) OS_Core::Sleep(100);

	if (doReset) mLoadCount=1;
	if (--mLoadCount<=0)
	{
		mLoadCount=0;
		mIsLoaded=false;

		for (int aCount=0;aCount<mTextureList.Size();aCount++) gG.UnloadTexture(mTextureList[aCount]);
		mTextureList.Reset();

#ifdef DEBUGBUNDLES
		gOut.Out("  <<<UnloadBundle: %s",mBundleName.c());
#endif

	}
	UnloadData();
}

void SpriteBundle::UnloadData()
{
	mDynaList.Clear();
	if (mData) delete mData;
	mData=NULL;
}


Sprite::Sprite()
{
	mIsInitialized=false;
	mOwnTexture=false;
	mOwnShape=true;
	mTexture=-1;
	mPending=false;
	mWidth=mHeight=0;
}

Sprite::~Sprite()
{
	Unload();
}

void Sprite::Unload()
{
	if (!gAppPtr) return;

	mIsInitialized=false;
	if (mOwnShape)
	{
		for (int aCount=0;aCount<mShape.Size();aCount++) 
		{
			if (mShape[aCount]>0) gG.UnloadShape(mShape[aCount]);
			mShape[aCount]=-1;
		}
		for (int aCount=0;aCount<mShapeMT.Size();aCount++) 
		{
			if (mShapeMT[aCount]>0) gG.UnloadShape(mShapeMT[aCount]);
			mShapeMT[aCount]=-1;
		}
		mOwnShape=false;
	}

	if (mTexture>=0)
	{
		if (mOwnTexture) gG.UnloadTexture(mTexture);
		mTexture=-1;
		mOwnTexture=false;
		//if (mOwnTexture) gOut.Out("Unload Own");
	}

	if (mTextureRect) delete mTextureRect;
/*
	for (int aCount=0;aCount<gG.mFillrect_List.Size();aCount++)
	{
		if (gG.mFillrect_List[aCount]==this)
		{
			gOut.Out("OMG WE DELETED A FILLRECT SPRITE!");
		}
	}
*/

}


void Sprite::ManualLoad(int theTexture, IOBuffer& theBuffer)
{
	if (theBuffer.IsEnd()) return;	// kittyengine asset existence guard
	Rect aRect=theBuffer.ReadRect();
	int aWidth=theBuffer.ReadInt();
	int aHeight=theBuffer.ReadInt();
	float aDWidth=theBuffer.ReadFloat();
	float aDHeight=theBuffer.ReadFloat();
	float aXMove=theBuffer.ReadFloat();
	float aYMove=theBuffer.ReadFloat();
	bool aRotated=theBuffer.ReadBool();
	ManualLoad(theTexture,aRect,aWidth,aHeight,aDWidth,aDHeight,aXMove,aYMove,aRotated);

	int aKeyCount=theBuffer.ReadInt();
	for (int aCount=0;aCount<aKeyCount;aCount++)
	{
		Point aKey=theBuffer.ReadPoint();
		mKey[aCount]=aKey;
	}
}

void Sprite::BundleLoad(short theVersion, Array<int>& theTextureList, IOBuffer& theBuffer)
{
	if (theBuffer.IsEnd()) return;	// kittyengine asset existence guard
	switch (theVersion)
	{
	case 0:
		{
			char aTex=theBuffer.ReadChar();
			ManualLoad(theTextureList[aTex],theBuffer);
			break;
		}
	}
}



void Sprite::ManualLoad(int theTexture, Rect theRect, int theWidth, int theHeight, float theDrawWidth, float theDrawHeight, float theXMove, float theYMove, bool doRotate)
{
	if (theWidth<=0 || theHeight<=0) return;

	//
	// We might be using a sprite size override.
	//
	IPoint aP=gOverrideTextureSize;
	if (aP.mX==0 && aP.mY==0) aP=gG.GetTextureDimensions(theTexture);

	Point aTSize=Point(aP.mX,aP.mY);

	mTexelSize=1.0f/aTSize;
	mTexelMove=mTexelSize*gG.KludgeTexels();

	//mTexelSize*=gG.KludgeTexels();	// FIXME: Do I need this?
	
	mTexture=theTexture;

    //
    // This appears to be redundant with the call below that
    // pulls in the corners of the texture quad.
    //
	theRect.mY+=gG.KludgeTexels();
	theRect.mX+=gG.KludgeTexels();
	theRect.mWidth-=gG.KludgeTexels()/2;		// Hm... not sure about subtracting the kludge from the
	theRect.mHeight-=gG.KludgeTexels()/2;		// width and height...

	mWidth=theWidth;
	mHeight=theHeight;

	if (!doRotate)
	{
		//
		// This seems to produce an overlarge image, but when unfiltered, it draws exactly
		// correctly...
		//
		Point aDrawDims=Point(theDrawWidth,theDrawHeight)/2;
		mDrawQuad.UpperLeft()=aDrawDims*Point(-1,-1);
		mDrawQuad.UpperRight()=aDrawDims*Point(1,-1);
		mDrawQuad.LowerLeft()=aDrawDims*Point(-1,1);
		mDrawQuad.LowerRight()=aDrawDims*Point(1,1);
		mDrawQuad+=Point(theXMove,theYMove);

		mTextureQuad.UpperLeft()=Point(theRect.mX,theRect.mY)/aTSize;
		mTextureQuad.UpperRight()=Point(theRect.mX+theRect.mWidth,theRect.mY)/aTSize;
		mTextureQuad.LowerLeft()=Point(theRect.mX,theRect.mY+theRect.mHeight)/aTSize;
		mTextureQuad.LowerRight()=Point(theRect.mX+theRect.mWidth,theRect.mY+theRect.mHeight)/aTSize;
	}
	else
	{
		mDrawQuad.UpperLeft().mX=-(theDrawHeight/2);
		mDrawQuad.UpperLeft().mY=-(theDrawWidth/2);
		mDrawQuad.UpperRight().mX=(theDrawHeight/2);
		mDrawQuad.UpperRight().mY=-(theDrawWidth/2);
		mDrawQuad.LowerLeft().mX=-(theDrawHeight/2);
		mDrawQuad.LowerLeft().mY=(theDrawWidth/2);
		mDrawQuad.LowerRight().mX=(theDrawHeight/2);
		mDrawQuad.LowerRight().mY=(theDrawWidth/2);
		mDrawQuad+=Point(theYMove,theXMove-gG.KludgePixels());

		mTextureQuad.UpperLeft()=Point(theRect.mX+theRect.mWidth,theRect.mY)/aTSize;
		mTextureQuad.UpperRight()=Point(theRect.mX+theRect.mWidth,theRect.mY+theRect.mHeight)/aTSize;
		mTextureQuad.LowerLeft()=Point(theRect.mX,theRect.mY)/aTSize;
		mTextureQuad.LowerRight()=Point(theRect.mX,theRect.mY+theRect.mHeight)/aTSize;
	}

	/*
	mDrawQuad.mCorner[0]-=gG.KludgePixels();
	mDrawQuad.mCorner[1].mX+=gG.KludgePixels();
	mDrawQuad.mCorner[1].mY-=gG.KludgePixels();
	mDrawQuad.mCorner[2].mX-=gG.KludgePixels();
	mDrawQuad.mCorner[2].mY+=gG.KludgePixels();
	mDrawQuad.mCorner[3]+=gG.KludgePixels();
	/**/	
	/*
	for (int aCount=0;aCount<4;aCount++)
	{
		mDrawQuad.mCorner[aCount].mX=floorf(mDrawQuad.mCorner[aCount].mX);
		mDrawQuad.mCorner[aCount].mY=floorf(mDrawQuad.mCorner[aCount].mY);
	}
	/**/



	mDrawQuadWidth=mDrawQuad.UpperRight().mX-mDrawQuad.UpperLeft().mX;
	mDrawQuadHeight=mDrawQuad.LowerRight().mY-mDrawQuad.UpperRight().mY;
	mDrawQuadCenter=mDrawQuad.GetBounds().Center();
	mBounds=mDrawQuad.GetBounds();

	mOriginalBounds=Rect(WidthF(),HeightF());
	mOriginalBounds.CenterAt(mDrawQuadCenter-Point(theXMove,theYMove));

	mIsInitialized=true;
}

void Sprite::ManualLoad(Sprite& theSprite)
{
	mIsInitialized=true;
	mTexture=theSprite.mTexture;

	mDrawQuad=theSprite.mDrawQuad;
	mTextureQuad=theSprite.mTextureQuad;
	mDrawQuadCenter=theSprite.mDrawQuadCenter;
	mBounds=theSprite.mBounds;
	mWidth=theSprite.mWidth;
	mHeight=theSprite.mHeight;
	mTexelSize=theSprite.mTexelSize;
	mTexelMove=theSprite.mTexelMove;
	mKey.Copy(theSprite.mKey);
	mDrawQuadWidth=theSprite.mDrawQuadWidth;
	mDrawQuadHeight=theSprite.mDrawQuadHeight;
}

bool Sprite::GoLive(bool doThread)
{
	//
	// doThread was put in because DynamicSprites can be unthreaded sometimes if you need to (like if you only need to load them once)
	//
	if (!mIsInitialized) return false;
	gG.SetTexture(mTexture);
	return true;
}

void Sprite::DrawCore(Quad& theXY, Quad& theUV)
{
#ifndef LEGACY_GL
	gG.mPipeline_Normal->StartQuad();
	gG.mPipeline_Normal->AddVertex().Set(gG.Fix2D(theXY.mCorner[0]),gG.mColorInt,theUV.mCorner[0]);
	gG.mPipeline_Normal->AddVertex().Set(gG.Fix2D(theXY.mCorner[1]),gG.mColorInt,theUV.mCorner[1]);
	gG.mPipeline_Normal->AddVertex().Set(gG.Fix2D(theXY.mCorner[2]),gG.mColorInt,theUV.mCorner[2]);
	gG.mPipeline_Normal->AddVertex().Set(gG.Fix2D(theXY.mCorner[3]),gG.mColorInt,theUV.mCorner[3]);
	gG.mPipeline_Normal->FinishQuad();
#else
	gG.DrawTexturedQuad(theXY,theUV);
#endif
}

void Sprite::DrawCore(Quad3D& theXY, Quad& theUV)
{
#ifndef LEGACY_GL
	gG.mPipeline_Normal->StartQuad();
	gG.mPipeline_Normal->AddVertex().Set(gG.Fix2D(theXY.mCorner[0].ToPoint()),gG.mColorInt,theUV.mCorner[0]);
	gG.mPipeline_Normal->AddVertex().Set(gG.Fix2D(theXY.mCorner[1].ToPoint()),gG.mColorInt,theUV.mCorner[1]);
	gG.mPipeline_Normal->AddVertex().Set(gG.Fix2D(theXY.mCorner[2].ToPoint()),gG.mColorInt,theUV.mCorner[2]);
	gG.mPipeline_Normal->AddVertex().Set(gG.Fix2D(theXY.mCorner[3].ToPoint()),gG.mColorInt,theUV.mCorner[3]);
	gG.mPipeline_Normal->FinishQuad();
#else
	gG.DrawTexturedQuad(theXY,theUV);
#endif
}


void Sprite::Draw(float theX, float theY)
{
	if (!GoLive()) return;

	Point aTranslate=Point(theX,theY)+GetHalfSize();
	gG.Translate(aTranslate);
	DrawCore(mDrawQuad,mTextureQuad);
	gG.Translate(-aTranslate);
}

void Sprite::Center(float theX, float theY)
{
	if (!GoLive()) return;

	gG.Translate(theX,theY);
	DrawCore(mDrawQuad,mTextureQuad);
	gG.Translate(-theX,-theY);
}

void Sprite::Center(Vector thePos)
{
	if (!GoLive()) return;

	Quad3D aBounds=GetDrawQuad3D();
	aBounds.CenterAt(thePos);
	DrawCore(aBounds,GetTextureQuad());
}

void Sprite::DrawScaled(Vector thePos, float theScale)
{
	if (!GoLive()) return;

	Quad3D aBounds=GetDrawQuad3D();
	aBounds.Scale(theScale);
	aBounds.CenterAt(thePos);
	DrawCore(aBounds,GetTextureQuad());
}


void Sprite::DrawFlipped(float theX, float theY, float flipX, float flipY)
{
	Matrix aMat;
	aMat.Scale(flipX,flipY,1);
	aMat.Translate(theX,theY);
	Draw(aMat);
}


void Sprite::Draw(Matrix &theMatrix)
{
	if (!GoLive()) return;

	Quad aDrawQuad;
	aDrawQuad.UpperLeft()=theMatrix.ProcessPoint(mDrawQuad.UpperLeft());
	aDrawQuad.UpperRight()=theMatrix.ProcessPoint(mDrawQuad.UpperRight());
	aDrawQuad.LowerLeft()=theMatrix.ProcessPoint(mDrawQuad.LowerLeft());
	aDrawQuad.LowerRight()=theMatrix.ProcessPoint(mDrawQuad.LowerRight());
	/**/
	DrawCore(aDrawQuad,mTextureQuad);
}

void Sprite::Draw(Matrix &theMatrix, Matrix &theTextureMatrix)
{
	if (!GoLive()) return;

	Quad aDrawQuad;
	aDrawQuad.UpperLeft()=theMatrix.ProcessPoint(mDrawQuad.UpperLeft());
	aDrawQuad.UpperRight()=theMatrix.ProcessPoint(mDrawQuad.UpperRight());
	aDrawQuad.LowerLeft()=theMatrix.ProcessPoint(mDrawQuad.LowerLeft());
	aDrawQuad.LowerRight()=theMatrix.ProcessPoint(mDrawQuad.LowerRight());

	//Quad aDrawQuad=mDrawQuad;
	Quad aTQ=mTextureQuad;
	//aDrawQuad.Process(theMatrix);
	aTQ.Process(theTextureMatrix);
	DrawCore(aDrawQuad,aTQ);
}


void Sprite::Draw(Quad &theDrawQuad, Quad &theTextureQuad)
{
	if (!GoLive()) return;
	DrawCore(theDrawQuad,theTextureQuad);
}

void Sprite::Draw(Quad3D &theDrawQuad)
{
	if (!GoLive()) return;
	gG.DrawTexturedQuad(theDrawQuad,GetTextureQuad());
}

void Sprite::Draw(Quad &theDrawQuad, Quad &theTextureQuad, Color aC1, Color aC2, Color aC3, Color aC4)
{
	if (!GoLive()) return;
	gG.DrawTexturedQuadGradient(theDrawQuad,theTextureQuad,aC1,aC2,aC3,aC4);
}


void Sprite::Draw(Rect theRect)
{
	static Quad aQ;
	aQ.mCorner[0]=theRect.UpperLeft();
	aQ.mCorner[1]=theRect.UpperRight();
	aQ.mCorner[2]=theRect.LowerLeft();
	aQ.mCorner[3]=theRect.LowerRight();
	Draw(aQ);
}


void Sprite::DrawSegment(float theX, float theY, float theStartX, float theStartY, float theEndX, float theEndY)
{
	if (!GoLive()) return;

	Quad aDQ=GetDrawQuad().GetSubQuad(theStartX,theStartY,theEndX,theEndY);
	Quad aTQ=GetTextureQuad().GetSubQuad(theStartX,theStartY,theEndX,theEndY);

	Point aTranslate=Point(theX,theY)+GetHalfSize();
	gG.PushTranslate();
	gG.Translate(aTranslate);
	DrawCore(aDQ,aTQ);
	gG.PopTranslate();
}

void Sprite::DrawSegment(float theX, float theY, Quad &theDrawQuad, Quad &theTextureQuad)
{
	if (!GoLive()) return;

	Point aTranslate=Point(theX,theY)+GetHalfSize();
	gG.PushTranslate();
	gG.Translate(aTranslate);
	DrawCore(theDrawQuad,theTextureQuad);
	gG.PopTranslate();
}

QuadPair Sprite::GetSegment(float theStartX, float theStartY, float theEndX, float theEndY)
{
	QuadPair aQP;
	aQP.mQuad[0]=GetDrawQuad().GetSubQuad(theStartX,theStartY,theEndX,theEndY);
	aQP.mQuad[1]=GetTextureQuad().GetSubQuad(theStartX,theStartY,theEndX,theEndY);
	return aQP;
}


void Sprite::DrawSegmentH(float theX, float theY, float theStart, float theEnd)
{
	if (!GoLive()) return;

	Quad aDQ=GetDrawQuad().GetSubQuad(theStart,0,theEnd,1);
	Quad aTQ=GetTextureQuad().GetSubQuad(theStart,0,theEnd,1);

	Point aTranslate=Point(theX,theY)+GetHalfSize();
	gG.PushTranslate();
	gG.Translate(aTranslate);
	DrawCore(aDQ,aTQ);
	gG.PopTranslate();
}

void Sprite::DrawSegmentV(float theX, float theY, float theStart, float theEnd)
{
	if (!GoLive()) return;
	Quad aDQ=GetDrawQuad().GetSubQuad(0,theStart,1,theEnd);
	Quad aTQ=GetTextureQuad().GetSubQuad(0,theStart,1,theEnd);

	Point aTranslate=Point(theX,theY)+GetHalfSize();
	gG.PushTranslate();
	gG.Translate(aTranslate);
	DrawCore(aDQ,aTQ);
	gG.PopTranslate();
	
}

void Sprite::DrawScaled(float theX, float theY, float theScale)
{
	Matrix aMat;
	aMat.Scale(theScale);
	aMat.Translate(theX,theY);
	Draw(aMat);
}

void Sprite::DrawScaled(float theX, float theY, Point theScale)
{
	Matrix aMat;
	aMat.Scale(theScale.mX,theScale.mY,1);
	aMat.Translate(theX,theY);
	Draw(aMat);
}

void Sprite::DrawRotated(float theX, float theY, float theRotation)
{
	Matrix aMat;
	aMat.Rotate2D(theRotation);
	aMat.Translate(theX,theY);
	Draw(aMat);
}

void Sprite::DrawRotatedScaled(float theX, float theY, float theRotation, Point theScale)
{
	Matrix aMat;
	aMat.Rotate2D(theRotation);
	aMat.Scale(theScale.mX,theScale.mY,1);
	aMat.Translate(theX,theY);
	Draw(aMat);
}

void Sprite::DrawRotatedScaled(float theX, float theY, float theRotation, float theScale)
{
	Matrix aMat;
	aMat.Rotate2D(theRotation);
	aMat.Scale(theScale);
	aMat.Translate(theX,theY);
	Draw(aMat);
}

void Sprite::DrawTelescopedH(float theX, float theY, float theWidth)
{
	float aChopWidth=1.0f/3.0f;
	float aStepWidth=(float)mWidth/3.0f;
	float aWorkWidth=theWidth-(aStepWidth*2);

	DrawSegmentH(theX,theY,0.0f,aChopWidth);
	int aSteps=(int)(aWorkWidth/aStepWidth);
	float aFraction=(aWorkWidth/aStepWidth)-aSteps;

	QuadPair aRepeater=GetSegment(aChopWidth,0.0f,aChopWidth+aChopWidth,1.0f);

	float aX=0;
	for (int aCount=0;aCount<aSteps;aCount++)
	{
		DrawSegment(theX+aX,theY,aRepeater);
		aX+=aStepWidth;
	}
	DrawSegmentH(theX+aX,theY,aChopWidth,aChopWidth+(aChopWidth*aFraction));
	DrawSegmentH(theX+theWidth-mWidth,theY,aChopWidth+aChopWidth,1.0f);
}

void Sprite::DrawTelescopedV(float theX, float theY, float theHeight)
{
	float aChopHeight=1.0f/3.0f;
	float aStepHeight=(float)mHeight/3.0f;
	float aWorkHeight=theHeight-(aStepHeight*2);
	
//	aChopHeight+=mTexelSize.mY;
	aChopHeight+=mTexelMove.mY;

	DrawSegmentV(theX,theY,0.0f,aChopHeight);
	int aSteps=(int)(aWorkHeight/aStepHeight);
	float aFraction=(aWorkHeight/aStepHeight)-aSteps;

	QuadPair aRepeater=GetSegment(0,aChopHeight,1,aChopHeight+aChopHeight);

	float aY=0;
	for (int aCount=0;aCount<aSteps;aCount++)
	{
		DrawSegment(theX,theY+aY,aRepeater);
		aY+=aStepHeight;
	}
	DrawSegmentV(theX,theY+aY,aChopHeight,aChopHeight+(aChopHeight*aFraction));
	DrawSegmentV(theX,theY+theHeight-mHeight,aChopHeight+aChopHeight,1.0f);
}

void Sprite::DrawTelescoped(float theX, float theY, float theWidth, float theHeight, bool ignoreCenter)
{
	float aChopWidth=1.0f/3.0f;
	float aStepWidth=(float)mWidth/3.0f;
	float aWorkWidth=theWidth-(aStepWidth*2);
//	aChopWidth+=mTexelSize.mX;
	aChopWidth+=mTexelMove.mX;
	
	float aChopHeight=1.0f/3.0f;
	float aStepHeight=(float)mHeight/3.0f;
	float aWorkHeight=theHeight-(aStepHeight*2);
//	aChopHeight+=mTexelSize.mY;
	aChopHeight+=mTexelMove.mY;

	int aStepsX=(int)(aWorkWidth/aStepWidth);
	float aFractionX=(aWorkWidth/aStepWidth)-aStepsX;
	int aStepsY=(int)(aWorkHeight/aStepHeight);
	float aFractionY=(aWorkHeight/aStepHeight)-aStepsY;

	QuadPair aBlock[3][3];
	QuadPair aFractionalRow=GetSegment(aChopWidth,aChopHeight,aChopWidth+aChopWidth,aChopHeight+(aChopHeight*aFractionY));
	aBlock[0][0]=GetSegment(0.0f,0.0f,aChopWidth,aChopHeight);
	aBlock[1][0]=GetSegment(aChopWidth,0.0f,aChopWidth+aChopWidth,aChopHeight);
	aBlock[2][0]=GetSegment(aChopWidth+aChopWidth,0,1.0f,aChopHeight);
	aBlock[0][1]=GetSegment(0.0f,aChopHeight,aChopWidth,aChopHeight+aChopHeight);
	aBlock[1][1]=GetSegment(aChopWidth,aChopHeight,aChopWidth+aChopWidth,aChopHeight+aChopHeight);
	aBlock[2][1]=GetSegment(aChopWidth+aChopWidth,aChopHeight,1.0f,aChopHeight+aChopHeight);
	aBlock[0][2]=GetSegment(0.0f,aChopHeight+aChopHeight,aChopWidth,1.0f);
	aBlock[1][2]=GetSegment(aChopWidth,aChopHeight+aChopHeight,aChopWidth+aChopWidth,1.0f);
	aBlock[2][2]=GetSegment(aChopWidth+aChopWidth,aChopHeight+aChopHeight,1.0f,1.0f);

	//
	// Top row...
	//
	DrawSegment(theX,theY,aBlock[0][0]);
	float aX=0;
	for (int aCount=0;aCount<aStepsX;aCount++)
	{
		DrawSegment(theX+aX,theY,aBlock[1][0]);
		aX+=aStepWidth;
	}
	DrawSegment(theX+aX,theY,aChopWidth,0,aChopWidth+(aChopWidth*aFractionX),aChopHeight); // Must Stay!
	DrawSegment(theX+theWidth-mWidth,theY,aBlock[2][0]);

	//
	// Middle Row...
	//
	float aY=0;
	for (int aCountY=0;aCountY<aStepsY;aCountY++)
	{
		DrawSegment(theX,theY+aY,aBlock[0][1]);

		if (!ignoreCenter)
		{
			float aX=0;
			for (int aCountX=0;aCountX<aStepsX;aCountX++)
			{
				DrawSegment(theX+aX,theY+aY,aBlock[1][1]);
				aX+=aStepWidth;
			}
			DrawSegment(theX+aX,theY+aY,aChopWidth,aChopHeight,aChopWidth+(aChopWidth*aFractionX),aChopHeight+aChopHeight); // Fractional must stay!
		}
		DrawSegment(theX+theWidth-mWidth,theY+aY,aBlock[2][1]);
		aY+=aStepHeight;
	}
	//
	// Fractional Row...
	//
	DrawSegment(theX,theY+aY,0.0f,aChopHeight,aChopWidth,aChopHeight+(aChopHeight*aFractionY));

	if (!ignoreCenter)
	{
		aX=0;
		for (int aCountX=0;aCountX<aStepsX;aCountX++)
		{
			DrawSegment(theX+aX,theY+aY,aFractionalRow);
			aX+=aStepWidth;
		}
		DrawSegment(theX+aX,theY+aY,aChopWidth,aChopHeight,aChopWidth+(aChopWidth*aFractionX),aChopHeight+(aChopHeight*aFractionY));
	}
	DrawSegment(theX+theWidth-mWidth,theY+aY,aChopWidth+aChopWidth,aChopHeight,1.0f,aChopHeight+(aChopHeight*aFractionY));
	//
	// Bottom Row...
	//
	aY=theHeight-mHeight;
	DrawSegment(theX,theY+aY,aBlock[0][2]);
	aX=0;
	for (int aCount=0;aCount<aStepsX;aCount++)
	{
		DrawSegment(theX+aX,theY+aY,aBlock[1][2]);
		aX+=aStepWidth;
	}
	DrawSegment(theX+aX,theY+aY,aChopWidth,aChopHeight+aChopHeight,aChopWidth+(aChopWidth*aFractionX),1.0f); // Fractional, leave it!
	DrawSegment(theX+theWidth-mWidth,theY+aY,aBlock[2][2]);
}

void Sprite::DrawTelescopedWithFill(float theX, float theY, float theWidth, float theHeight, Color theCenterFillColor)
{
	gG.PushColor();
	gG.SetColor(theCenterFillColor*gG.mColor);
	gG.FillRect(theX+(mWidth/3)-1,theY+(mHeight/3)-1,theWidth+2-(((mWidth/3)*2)-1),theHeight+2-(((mHeight/3)*2)-1));
	gG.PopColor();

	DrawTelescoped(theX,theY,theWidth,theHeight,true);
}

void Sprite::DrawTelescopedCorners(float theX, float theY, float theWidth, float theHeight, Color theCenterFillColor)
{

	float aChopWidth=1.0f/3.0f;
	float aStepWidth=(float)mWidth/3.0f;
	float aWorkWidth=theWidth-(aStepWidth*2);
//	aChopWidth+=mTexelSize.mX;
	aChopWidth+=mTexelMove.mX;

	float aChopHeight=1.0f/3.0f;
	float aStepHeight=(float)mHeight/3.0f;
	float aWorkHeight=theHeight-(aStepHeight*2);
//	aChopHeight+=mTexelSize.mY;
	aChopHeight+=mTexelMove.mY;

	int aStepsX=(int)(aWorkWidth/aStepWidth);
	float aFractionX=(aWorkWidth/aStepWidth)-aStepsX;
	int aStepsY=(int)(aWorkHeight/aStepHeight);
	float aFractionY=(aWorkHeight/aStepHeight)-aStepsY;

	QuadPair aBlock[3][3];
	QuadPair aFractionalRow=GetSegment(aChopWidth,aChopHeight,aChopWidth+aChopWidth,aChopHeight+(aChopHeight*aFractionY));
	aBlock[0][0]=GetSegment(0.0f,0.0f,aChopWidth,aChopHeight);
	aBlock[1][0]=GetSegment(aChopWidth,0.0f,aChopWidth+aChopWidth,aChopHeight);
	aBlock[2][0]=GetSegment(aChopWidth+aChopWidth,0,1.0f,aChopHeight);
	aBlock[0][1]=GetSegment(0.0f,aChopHeight,aChopWidth,aChopHeight+aChopHeight);
	aBlock[1][1]=GetSegment(aChopWidth,aChopHeight,aChopWidth+aChopWidth,aChopHeight+aChopHeight);
	aBlock[2][1]=GetSegment(aChopWidth+aChopWidth,aChopHeight,1.0f,aChopHeight+aChopHeight);
	aBlock[0][2]=GetSegment(0.0f,aChopHeight+aChopHeight,aChopWidth,1.0f);
	aBlock[1][2]=GetSegment(aChopWidth,aChopHeight+aChopHeight,aChopWidth+aChopWidth,1.0f);
	aBlock[2][2]=GetSegment(aChopWidth+aChopWidth,aChopHeight+aChopHeight,1.0f,1.0f);

	if (theCenterFillColor.mA>0)
	{
		gG.PushColor();
		gG.SetColor(theCenterFillColor*gG.mColor);
		gG.FillRect(theX+(mWidth/3)-1,theY,theWidth+2-(((mWidth/3)*2)-1),theHeight);
		gG.FillRect(theX,theY+((float)mHeight/3)-.5f,(float)(mWidth/3)+.5f,theHeight-(((float)mHeight/3)*2)+1);
		gG.FillRect(theX+theWidth-(((float)mWidth/3)+.5f),theY+((float)mHeight/3)-.5f,(float)(mWidth/3)+.5f,theHeight-(((float)mHeight/3)*2)+1);
		gG.PopColor();
	}


	//
	// Top row...
	//
	DrawSegment(theX,theY,aBlock[0][0]);
/*
	float aX=0;
	for (int aCount=0;aCount<aStepsX;aCount++)
	{
		DrawSegment(theX+aX,theY,aBlock[1][0]);
		aX+=aStepWidth;
	}
	DrawSegment(theX+aX,theY,aChopWidth,0,aChopWidth+(aChopWidth*aFractionX),aChopHeight); // Must Stay!
*/
	DrawSegment(theX+theWidth-mWidth,theY,aBlock[2][0]);

	//
	// Middle Row...
	//
/*
	float aY=0;
	for (int aCountY=0;aCountY<aStepsY;aCountY++)
	{
		DrawSegment(theX,theY+aY,aBlock[0][1]);

		if (!ignoreCenter)
		{
			float aX=0;
			for (int aCountX=0;aCountX<aStepsX;aCountX++)
			{
				DrawSegment(theX+aX,theY+aY,aBlock[1][1]);
				aX+=aStepWidth;
			}
			DrawSegment(theX+aX,theY+aY,aChopWidth,aChopHeight,aChopWidth+(aChopWidth*aFractionX),aChopHeight+aChopHeight); // Fractional must stay!
		}
		DrawSegment(theX+theWidth-mWidth,theY+aY,aBlock[2][1]);
		aY+=aStepHeight;
	}
	//
	// Fractional Row...
	//
	DrawSegment(theX,theY+aY,0.0f,aChopHeight,aChopWidth,aChopHeight+(aChopHeight*aFractionY));
*/

	//
	// Bottom Row...
	//
	float aY=theHeight-mHeight;
	DrawSegment(theX,theY+aY,aBlock[0][2]);
	DrawSegment(theX+theWidth-mWidth,theY+aY,aBlock[2][2]);
}

void Sprite::DrawTiledH(float theX, float theY, int theRepeats)
{
	for (int aCount=0;aCount<theRepeats;aCount++)
	{
		Draw(theX,theY);
		theX+=GetWidthF();
	}
}

void Sprite::DrawTiledV(float theX, float theY, int theRepeats)
{
	for (int aCount=0;aCount<theRepeats;aCount++)
	{
		Draw(theX,theY);
		theY+=GetHeightF();
	}
}

void Sprite::DrawTiled(float theX, float theY, int theRepeatX, int theRepeatY)
{
	for (int aSpanX=0;aSpanX<theRepeatX;aSpanX++)
	{
		float aY=theY;
		for (int aSpanY=0;aSpanY<theRepeatY;aSpanY++)
		{
			Draw(theX,aY);
			aY+=GetHeightF();
		}
		theX+=GetWidthF();
	}
}

void Sprite::DrawTiledScaled(float theX, float theY, int theRepeatX, int theRepeatY,float theScale)
{
	gG.PushTranslate();
	gG.Translate(HalfWidthF()*theScale,HalfHeightF()*theScale);
	for (int aSpanX=0;aSpanX<theRepeatX;aSpanX++)
	{
		float aY=theY;
		for (int aSpanY=0;aSpanY<theRepeatY;aSpanY++)
		{
			DrawScaled(theX,aY,theScale);
			aY+=GetHeightF()*theScale;
		}
		theX+=GetWidthF()*theScale;
	}
	gG.PopTranslate();
}

void Sprite::DrawRadial(float theX, float theY, float theStartAngle, float theEndAngle)
{
	float aSAngle=theStartAngle;
	float aEAngle=theEndAngle;


	//
	// Positive...
	//
	int aStartMult=(int)(aSAngle/45)-1;
	//if (aStartMult<0) aStartMult--;
	int aEndMult=(int)(aEAngle/45)+1;
	//if (aEndMult<0) aEndMult--;


	for (float aMult=(float)aStartMult;aMult<=(float)aEndMult;aMult++)
	{
		if (gMath.RangeIntersect(Point(theStartAngle,theEndAngle),Point((45)*(aMult),(45)*(aMult+1))))
		{
			float aLowCap=_max(aSAngle,(45)*(aMult));
			float aHighCap=_min(aEAngle,(45)*(aMult+1));
			DrawPieSlice45(theX,theY,aLowCap,aHighCap);
		}
	}
}

void Sprite::DrawRadialScaled(float theX, float theY, float theStartAngle, float theEndAngle, float theScale)
{
	float aSAngle=theStartAngle;
	float aEAngle=theEndAngle;


	//
	// Positive...
	//
	int aStartMult=(int)(aSAngle/45)-1;
	//if (aStartMult<0) aStartMult--;
	int aEndMult=(int)(aEAngle/45)+1;
	//if (aEndMult<0) aEndMult--;


	for (float aMult=(float)aStartMult;aMult<=(float)aEndMult;aMult++)
	{
		if (gMath.RangeIntersect(Point(theStartAngle,theEndAngle),Point((45)*(aMult),(45)*(aMult+1))))
		{
			float aLowCap=_max(aSAngle,(45)*(aMult));
			float aHighCap=_min(aEAngle,(45)*(aMult+1));
			DrawPieSlice45Scaled(theX,theY,aLowCap,aHighCap,theScale);
		}
	}
}

void Sprite::DrawPieSlice45(float theX, float theY, float theStartAngle, float theEndAngle)
{
	if (!GoLive()) return;
	Point aTranslate=Point(theX,theY);//-GetHalfSize();

	float aSAngle=theStartAngle;
	float aEAngle=theEndAngle;

	Point aStartVec=gMath.AngleToVector(aSAngle)*.7f;
	Point aEndVec=gMath.AngleToVector(aEAngle)*.7f;

	

	//
	// Hm... somehow we need to extend these vectors to the edge of the sprite's rect,
	// but clip them INTO the rect... you can't use Rect clamp...
	//
	Line aTop=Line(Point(-.5f,-.5f),Point(.5,-.5f));
	Line aBottom=Line(Point(-.5f,.5f),Point(.5,.5f));
	Line aRight=Line(Point(.5f,-.5f),Point(.5,.5f));
	Line aLeft=Line(Point(-.5f,-.5f),Point(-.5,.5f));

	enum CheckerSide
	{
		SIDE_TOP=0x01,
		SIDE_BOTTOM=0x02,
		SIDE_RIGHT=0x04,
		SIDE_LEFT=0x08
	};

	int aCheck=0;
	if (aStartVec.mX<-.5f) aCheck|=SIDE_LEFT;
	if (aStartVec.mX>.5f) aCheck|=SIDE_RIGHT;
	if (aStartVec.mY<-.5f) aCheck|=SIDE_TOP;
	if (aStartVec.mY>.5f) aCheck|=SIDE_BOTTOM;

	Line aTestVec=Line(Point(0,0),aStartVec);
	Point aNewPoint;
	//if (aCheck&SIDE_TOP) if (gMath.LineIntersectLine(aTop.mPos[0],aTop.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aStartVec=aNewPoint;
	//if (aCheck&SIDE_RIGHT) if (gMath.LineIntersectLine(aRight.mPos[0],aRight.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aStartVec=aNewPoint;
	//if (aCheck&SIDE_BOTTOM) if (gMath.LineIntersectLine(aBottom.mPos[0],aBottom.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aStartVec=aNewPoint;
	//if (aCheck&SIDE_LEFT) if (gMath.LineIntersectLine(aLeft.mPos[0],aLeft.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aStartVec=aNewPoint;
	if (aCheck&SIDE_TOP) aStartVec=gMath.LineIntersectY(aTestVec,-.5f);
	if (aCheck&SIDE_BOTTOM) aStartVec=gMath.LineIntersectY(aTestVec,.5f);
	if (aCheck&SIDE_LEFT) aStartVec=gMath.LineIntersectX(aTestVec,-.5f);
	if (aCheck&SIDE_RIGHT) aStartVec=gMath.LineIntersectX(aTestVec,.5f);

	aCheck=0;
	if (aEndVec.mX<-.5f) aCheck|=SIDE_LEFT;
	if (aEndVec.mX>.5f) aCheck|=SIDE_RIGHT;
	if (aEndVec.mY<-.5f) aCheck|=SIDE_TOP;
	if (aEndVec.mY>.5f) aCheck|=SIDE_BOTTOM;

	aTestVec=Line(Point(0,0),aEndVec);
	//if (aCheck&SIDE_TOP) if (gMath.LineIntersectLine(aTop.mPos[0],aTop.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aEndVec=aNewPoint;
	//if (aCheck&SIDE_RIGHT) if (gMath.LineIntersectLine(aRight.mPos[0],aRight.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aEndVec=aNewPoint;
	//if (aCheck&SIDE_BOTTOM) if (gMath.LineIntersectLine(aBottom.mPos[0],aBottom.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aEndVec=aNewPoint;
	//if (aCheck&SIDE_LEFT) if (gMath.LineIntersectLine(aLeft.mPos[0],aLeft.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aEndVec=aNewPoint;

	if (aCheck&SIDE_TOP) aEndVec=gMath.LineIntersectY(aTestVec,-.5f);
	if (aCheck&SIDE_BOTTOM) aEndVec=gMath.LineIntersectY(aTestVec,.5f);
	if (aCheck&SIDE_LEFT) aEndVec=gMath.LineIntersectX(aTestVec,-.5f);
	if (aCheck&SIDE_RIGHT) aEndVec=gMath.LineIntersectX(aTestVec,.5f);



	Point aCenter=mDrawQuad.GetSubPoint(.5f,.5f);
	Point aPos1=mDrawQuad.GetSubPoint(.5f+aStartVec.mX,.5f+aStartVec.mY);
	Point aPos2=mDrawQuad.GetSubPoint(.5f+aEndVec.mX,.5f+aEndVec.mY);

	Point aTCenter=mTextureQuad.GetSubPoint(.5f,.5f);
	Point aTPos1=mTextureQuad.GetSubPoint(.5f+aStartVec.mX,.5f+aStartVec.mY);
	Point aTPos2=mTextureQuad.GetSubPoint(.5f+aEndVec.mX,.5f+aEndVec.mY);


	gG.Translate(aTranslate);
	//gG.SetTexture(mTexture);
	//gG.SetTexture();

	gG.StartShape(1);
	gG.AddVertex(aCenter,gG.mColorInt,aTCenter);
	gG.AddVertex(aPos1,gG.mColorInt,aTPos1);
	gG.AddVertex(aPos2,gG.mColorInt,aTPos2);
	gG.FinishShape(3, 0,1,2);

	gG.Translate(-aTranslate);
}

void Sprite::DrawPieSlice45Scaled(float theX, float theY, float theStartAngle, float theEndAngle, float theScale)
{
	if (!GoLive()) return;
	Point aTranslate=Point(theX,theY);//-GetHalfSize();

	float aSAngle=theStartAngle;
	float aEAngle=theEndAngle;

	Point aStartVec=gMath.AngleToVector(aSAngle)*.7f;
	Point aEndVec=gMath.AngleToVector(aEAngle)*.7f;



	//
	// Hm... somehow we need to extend these vectors to the edge of the sprite's rect,
	// but clip them INTO the rect... you can't use Rect clamp...
	//
	Line aTop=Line(Point(-.5f,-.5f),Point(.5,-.5f));
	Line aBottom=Line(Point(-.5f,.5f),Point(.5,.5f));
	Line aRight=Line(Point(.5f,-.5f),Point(.5,.5f));
	Line aLeft=Line(Point(-.5f,-.5f),Point(-.5,.5f));

	enum CheckerSide
	{
		SIDE_TOP=0x01,
		SIDE_BOTTOM=0x02,
		SIDE_RIGHT=0x04,
		SIDE_LEFT=0x08
	};

	int aCheck=0;
	if (aStartVec.mX<-.5f) aCheck|=SIDE_LEFT;
	if (aStartVec.mX>.5f) aCheck|=SIDE_RIGHT;
	if (aStartVec.mY<-.5f) aCheck|=SIDE_TOP;
	if (aStartVec.mY>.5f) aCheck|=SIDE_BOTTOM;

	Line aTestVec=Line(Point(0,0),aStartVec);
	Point aNewPoint;
	//if (aCheck&SIDE_TOP) if (gMath.LineIntersectLine(aTop.mPos[0],aTop.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aStartVec=aNewPoint;
	//if (aCheck&SIDE_RIGHT) if (gMath.LineIntersectLine(aRight.mPos[0],aRight.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aStartVec=aNewPoint;
	//if (aCheck&SIDE_BOTTOM) if (gMath.LineIntersectLine(aBottom.mPos[0],aBottom.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aStartVec=aNewPoint;
	//if (aCheck&SIDE_LEFT) if (gMath.LineIntersectLine(aLeft.mPos[0],aLeft.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aStartVec=aNewPoint;
	if (aCheck&SIDE_TOP) aStartVec=gMath.LineIntersectY(aTestVec,-.5f);
	if (aCheck&SIDE_BOTTOM) aStartVec=gMath.LineIntersectY(aTestVec,.5f);
	if (aCheck&SIDE_LEFT) aStartVec=gMath.LineIntersectX(aTestVec,-.5f);
	if (aCheck&SIDE_RIGHT) aStartVec=gMath.LineIntersectX(aTestVec,.5f);

	aCheck=0;
	if (aEndVec.mX<-.5f) aCheck|=SIDE_LEFT;
	if (aEndVec.mX>.5f) aCheck|=SIDE_RIGHT;
	if (aEndVec.mY<-.5f) aCheck|=SIDE_TOP;
	if (aEndVec.mY>.5f) aCheck|=SIDE_BOTTOM;

	aTestVec=Line(Point(0,0),aEndVec);
	//if (aCheck&SIDE_TOP) if (gMath.LineIntersectLine(aTop.mPos[0],aTop.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aEndVec=aNewPoint;
	//if (aCheck&SIDE_RIGHT) if (gMath.LineIntersectLine(aRight.mPos[0],aRight.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aEndVec=aNewPoint;
	//if (aCheck&SIDE_BOTTOM) if (gMath.LineIntersectLine(aBottom.mPos[0],aBottom.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aEndVec=aNewPoint;
	//if (aCheck&SIDE_LEFT) if (gMath.LineIntersectLine(aLeft.mPos[0],aLeft.mPos[1],aTestVec.mPos[0],aTestVec.mPos[1],&aNewPoint)) aEndVec=aNewPoint;

	if (aCheck&SIDE_TOP) aEndVec=gMath.LineIntersectY(aTestVec,-.5f);
	if (aCheck&SIDE_BOTTOM) aEndVec=gMath.LineIntersectY(aTestVec,.5f);
	if (aCheck&SIDE_LEFT) aEndVec=gMath.LineIntersectX(aTestVec,-.5f);
	if (aCheck&SIDE_RIGHT) aEndVec=gMath.LineIntersectX(aTestVec,.5f);



	Point aCenter=mDrawQuad.GetSubPoint(.5f,.5f)*theScale;
	Point aPos1=mDrawQuad.GetSubPoint(.5f+aStartVec.mX,.5f+aStartVec.mY)*theScale;
	Point aPos2=mDrawQuad.GetSubPoint(.5f+aEndVec.mX,.5f+aEndVec.mY)*theScale;

	Point aTCenter=mTextureQuad.GetSubPoint(.5f,.5f);
	Point aTPos1=mTextureQuad.GetSubPoint(.5f+aStartVec.mX,.5f+aStartVec.mY);
	Point aTPos2=mTextureQuad.GetSubPoint(.5f+aEndVec.mX,.5f+aEndVec.mY);


	gG.Translate(aTranslate);
	//gG.SetTexture(mTexture);
	//gG.SetTexture();
	
	gG.StartShape(1);
	gG.AddVertex(aCenter,gG.mColorInt,aTCenter);
	gG.AddVertex(aPos1,gG.mColorInt,aTPos1);
	gG.AddVertex(aPos2,gG.mColorInt,aTPos2);
	gG.FinishShape(3, 0,1,2);

	gG.Translate(-aTranslate);
}

void Sprite::DrawWarped(IPoint theWarpSize, Point *theXYPoints, Point* theUVPoints, Color *theColorPoints)
{
	if (!GoLive()) return;
	Point* aXYPtr=theXYPoints;
	Point* aUVPtr=theUVPoints;
	Color* aColorPtr=theColorPoints;

	//gG.SetTexture(mTexture);

	//
	// Compatibility note: Chuzzle used "StartIndexedShape" but I tossed that in the newer iteration because
	// ALL shapes are indexed.  So gG.StartIndexedShape is just an inline that goes to gG.StartShape, for backwards compatibility.
	//
	gG.StartIndexedShape((theWarpSize.mX-1)*(theWarpSize.mY-1)*2);

	int aIndexCounter=0;

	if (!aColorPtr)
	{
		int aVColor=gG.mColorInt;
		for (int aSpanY=0;aSpanY<theWarpSize.mY;aSpanY++)
		{
			for (int aSpanX=0;aSpanX<theWarpSize.mX;aSpanX++)
			{
				gG.AddVertex(*(aXYPtr),aVColor,*(aUVPtr));
				aXYPtr++;
				aUVPtr++;
			}
		}
	}
	else
	{
		for (int aSpanY=0;aSpanY<theWarpSize.mY;aSpanY++)
		{
			for (int aSpanX=0;aSpanX<theWarpSize.mX;aSpanX++)
			{
				gG.AddVertex(*(aXYPtr),(*aColorPtr).ToInt(),*(aUVPtr));
				aXYPtr++;
				aUVPtr++;
				aColorPtr++;
			}
		}
	}

	aIndexCounter=0;
	for (int aSpanY=0;aSpanY<theWarpSize.mY-1;aSpanY++)
	{
		for (int aSpanX=0;aSpanX<theWarpSize.mX-1;aSpanX++)
		{
			gG.AddVertexIndex(aIndexCounter);
			gG.AddVertexIndex(aIndexCounter+1);
			gG.AddVertexIndex(aIndexCounter+theWarpSize.mX);

			gG.AddVertexIndex(aIndexCounter+1);
			gG.AddVertexIndex(aIndexCounter+theWarpSize.mX);
			gG.AddVertexIndex(aIndexCounter+theWarpSize.mX+1);

			aIndexCounter++;
		}
		aIndexCounter++;
	}

/*
	if (!aColorPtr)
	{
		//
		// Old version, less optimal
		//
		{
			for (int aSpanY=0;aSpanY<theWarpSize.mY-1;aSpanY++)
			{
				for (int aSpanX=0;aSpanX<theWarpSize.mX-1;aSpanX++)
				{
					int aVColor=gG.mColorInt;

					gG.AddVertex(*(aXYPtr),aVColor,*(aUVPtr));
					gG.AddVertex(*(aXYPtr+1),aVColor,*(aUVPtr+1));
					gG.AddVertex(*(aXYPtr+theWarpSize.mX),aVColor,*(aUVPtr+theWarpSize.mX));
					gG.AddVertex(*(aXYPtr+theWarpSize.mX+1),aVColor,*(aUVPtr+1+theWarpSize.mX));

					gG.AddVertexIndex(aIndexCounter);
					gG.AddVertexIndex(aIndexCounter+1);
					gG.AddVertexIndex(aIndexCounter+2);
					gG.AddVertexIndex(aIndexCounter+1);
					gG.AddVertexIndex(aIndexCounter+2);
					gG.AddVertexIndex(aIndexCounter+3);

					aIndexCounter+=4;


					aXYPtr++;
					aUVPtr++;
				}
				aXYPtr++;
				aUVPtr++;
			}
		}
	}
	else
	{
		for (int aSpanY=0;aSpanY<theWarpSize.mY-1;aSpanY++)
		{
			for (int aSpanX=0;aSpanX<theWarpSize.mX-1;aSpanX++)
			{
				gG.AddVertex(*(aXYPtr),(*aColorPtr).ToInt(),*(aUVPtr));
				gG.AddVertex(*(aXYPtr+1),(*(aColorPtr+1)).ToInt(),*(aUVPtr+1));
				gG.AddVertex(*(aXYPtr+theWarpSize.mX),(*(aColorPtr+theWarpSize.mX)).ToInt(),*(aUVPtr+theWarpSize.mX));
				gG.AddVertex(*(aXYPtr+theWarpSize.mX+1),(*(aColorPtr+theWarpSize.mX+1)).ToInt(),*(aUVPtr+1+theWarpSize.mX));

				gG.AddVertexIndex(aIndexCounter);
				gG.AddVertexIndex(aIndexCounter+1);
				gG.AddVertexIndex(aIndexCounter+2);
				gG.AddVertexIndex(aIndexCounter+1);
				gG.AddVertexIndex(aIndexCounter+2);
				gG.AddVertexIndex(aIndexCounter+3);

				aIndexCounter+=4;


				aXYPtr++;
				aUVPtr++;
				aColorPtr++;
			}
			aXYPtr++;
			aUVPtr++;
			aColorPtr++;
		}

	}
	/**/

	gG.FinishShape();
}

void Sprite::DrawTweaked(Point thePos, Point tweakUL, Point tweakUR, Point tweakLL, Point tweakLR)
{
	if (!GoLive()) return;

	Quad aDQuad=mDrawQuad;
	aDQuad.UpperLeft()+=tweakUL;
	aDQuad.UpperRight()+=tweakUR;
	aDQuad.LowerLeft()+=tweakLL;
	aDQuad.LowerRight()+=tweakLR;

	Point aTranslate=thePos;//+GetHalfSize();
	gG.Translate(aTranslate);
	//gG.SetTexture(mTexture);
	DrawCore(aDQuad,mTextureQuad);
	gG.Translate(-aTranslate);
}



RenderToSprite::RenderToSprite()
{
}

RenderToSprite::~RenderToSprite()
{
	if (gAppPtr && !gAppPtr->IsQuit()) Stop();
}

#ifdef LEGACY_GL
void RenderToSprite::Go(int theWidth, int theHeight, bool wantAlpha, bool wantReadable)
{
	if (theWidth<=0 || theHeight<=0) return;

	mTexture=gG.CreateRenderer(theWidth,theHeight,wantAlpha,wantReadable);

	mTextureQuad.UpperLeft()=Point(0.0f,0.0f);
	mTextureQuad.UpperRight()=Point(1.0f,0.0f);
	mTextureQuad.LowerLeft()=Point(0.0f,1.0f);
	mTextureQuad.LowerRight()=Point(1.0f,1.0f);

	mWidth=theWidth;
	mHeight=theHeight;
	Point aDrawDims=Point((float)theWidth,(float)theHeight)/2;

	mDrawQuad.UpperLeft()=aDrawDims*Point(-1,-1);
	mDrawQuad.UpperRight()=aDrawDims*Point(1,-1);
	mDrawQuad.LowerLeft()=aDrawDims*Point(-1,1);
	mDrawQuad.LowerRight()=aDrawDims*Point(1,1);

	//
	// The render target needs special tweaking to make it draw at
	// the correct position, because of the way textures actually draw
	// at .5,.5 when they're actually at 0,0.
	//

	//
	// Great: This shouldn't happen on OpenGL.
	//

	float aKludge=gG.KludgePixels();
	if (aKludge!=0)
	{
		mDrawQuad.UpperLeft()+=Point(-aKludge,-aKludge);
		mDrawQuad.UpperRight()+=Point(-aKludge,-aKludge);
		mDrawQuad.LowerLeft()+=Point(-aKludge,-aKludge);
		mDrawQuad.LowerRight()+=Point(-aKludge,-aKludge);
	}

	mIsInitialized=true;
	gAppPtr->CatchUp();
}
#else
void RenderToSprite::Go(int theWidth, int theHeight, char* theFormat, bool wantReadable, bool useFullTexture) {Go(theWidth,theHeight,Hash8(theFormat),wantReadable,useFullTexture);}
void RenderToSprite::Go(int theWidth, int theHeight, longlong theFormat, bool wantReadable, bool useFullTexture)
{
	if (theWidth<=0 || theHeight<=0) return;

#ifdef _DEBUG
	if (theFormat<16)
	{
		gOut.Out("!!!!!! CORRECT YOUR RENDERTOSPRITE:GO CALL IN VALUE TO SUPPLY PIXEL FORMAT INSTEAD OF WANTALPHA!");
	}
#endif

	int aAskWidth=theWidth;
	int aAskHeight=theHeight;

	if (gG.MustPow2Textures())
	{
		aAskWidth=gMath.NearestPowerOfTwo(theWidth);
		aAskHeight=gMath.NearestPowerOfTwo(theHeight);
		if (useFullTexture)
		{
			theWidth=aAskWidth;
			theHeight=aAskHeight;
		}
	}


	mTexture=gG.CreateRenderer(aAskWidth,aAskHeight,theFormat,wantReadable);

	Point aRatio=Point((float)theWidth/(float)aAskWidth,(float)theHeight/(float)aAskHeight);

	mTextureQuad.UpperLeft()=Point(0.0f,0.0f);
	mTextureQuad.UpperRight()=Point(aRatio.mX,0.0f);
	mTextureQuad.LowerLeft()=Point(0.0f,aRatio.mY);
	mTextureQuad.LowerRight()=aRatio;

	mWidth=theWidth;
	mHeight=theHeight;
	Point aDrawDims=Point((float)theWidth,(float)theHeight)/2;


	mDrawQuad.UpperLeft()=aDrawDims*Point(-1,-1);
	mDrawQuad.UpperRight()=aDrawDims*Point(1,-1);
	mDrawQuad.LowerLeft()=aDrawDims*Point(-1,1);
	mDrawQuad.LowerRight()=aDrawDims*Point(1,1);

	//
	// The render target needs special tweaking to make it draw at
	// the correct position, because of the way textures actually draw
	// at .5,.5 when they're actually at 0,0.
	//
	
	//
	// Great: This shouldn't happen on OpenGL.
	//
	
	float aKludge=gG.KludgePixels();
	if (aKludge!=0)
	{
		mDrawQuad.UpperLeft()+=Point(-aKludge,-aKludge);
		mDrawQuad.UpperRight()+=Point(-aKludge,-aKludge);
		mDrawQuad.LowerLeft()+=Point(-aKludge,-aKludge);
		mDrawQuad.LowerRight()+=Point(-aKludge,-aKludge);
	}

	mIsInitialized=true;
	gAppPtr->CatchUp();
}
#endif

void RenderToSprite::MakeTile()
{
	Point aKludge=Point(1.0f/WidthF(),1.0f/HeightF())*.5f;
	mTextureQuad.UpperLeft()+=Point(aKludge.mX,aKludge.mY);
	mTextureQuad.UpperRight()+=Point(-aKludge.mX,aKludge.mY);
	mTextureQuad.LowerLeft()+=Point(aKludge.mX,-aKludge.mY);
	mTextureQuad.LowerRight()+=Point(-aKludge.mX,-aKludge.mY);
}

void RenderToSprite::Export(Image& theImage, bool swapRB)
{
#ifndef LEGACY_GL
	gG.RenderTo(*this);
	gG.GetRenderToPixels(theImage);
	gG.RenderTo();

	if (swapRB) theImage.SwapRB();
#endif
}

void RenderToSprite::ExportGIF(String theFN, bool swapRB) {Image aImage;Export(aImage,swapRB);aImage.ExportGIF(theFN);}
void RenderToSprite::ExportJPG(String theFN, bool swapRB) {Image aImage;Export(aImage,swapRB);aImage.ExportJPG(theFN);}
void RenderToSprite::ExportPNG(String theFN, bool swapRB) {Image aImage;Export(aImage,swapRB);aImage.ExportPNG(theFN);}
void RenderToSprite::Import(String theFN) {Image aImage;aImage.Load(theFN);Import(aImage);}


void RenderToSprite::Import(Image& theImage)
{
	Sprite aSpr;
	aSpr.Load(theImage);

	gG.RenderTo(*this);
	gG.Clear(0,0,0,0);
	aSpr.Draw(0,0);
	gG.RenderTo();
}





void RenderToSprite::Stop()
{
	if (mIsInitialized) gG.UnloadTexture(mTexture);
	mIsInitialized=false;
}


bool RenderToSprite::GoLive(bool doThread)
{
	if (!mIsInitialized) return false;
	gG.SetTexture(mTexture,true);
	return true;
}

#ifndef LEGACY_GL

Vx2D ShaderSprite::mV[4];
ShaderSprite::~ShaderSprite() {if (mOwnShader) delete mShader;mShader=NULL;}
void ShaderSprite::Go(String theShader, float theWidth, float theHeight)
{
	mShader=new Shader();mShader->Load(theShader);mOwnShader=true;
	SetSize(theWidth,theHeight);
}
void ShaderSprite::DrawCore(Quad& theXY, Quad& theUV)
{
	mV[0]=Vx2D(gG.Fix2D(theXY.mCorner[0]),gG.mColorInt,theUV.mCorner[0]);
	mV[1]=Vx2D(gG.Fix2D(theXY.mCorner[1]),gG.mColorInt,theUV.mCorner[1]);
	mV[2]=Vx2D(gG.Fix2D(theXY.mCorner[2]),gG.mColorInt,theUV.mCorner[2]);
	mV[3]=Vx2D(gG.Fix2D(theXY.mCorner[3]),gG.mColorInt,theUV.mCorner[3]);

	mShader->DrawShape(mV,4,Index4Array());
}

void ShaderSprite::DrawCore(Quad3D& theXY, Quad& theUV)
{
	mV[0]=Vx2D(gG.Fix2D(theXY.mCorner[0]),gG.mColorInt,theUV.mCorner[0]);
	mV[1]=Vx2D(gG.Fix2D(theXY.mCorner[1]),gG.mColorInt,theUV.mCorner[1]);
	mV[2]=Vx2D(gG.Fix2D(theXY.mCorner[2]),gG.mColorInt,theUV.mCorner[2]);
	mV[3]=Vx2D(gG.Fix2D(theXY.mCorner[3]),gG.mColorInt,theUV.mCorner[3]);
	mShader->DrawShape(mV,4,Index4Array());
}



ShaderRenderToSprite::ShaderRenderToSprite() {}
ShaderRenderToSprite::~ShaderRenderToSprite() {if (mOwnShader) delete mShader;mShader=NULL;}

void ShaderRenderToSprite::Go(int theWidth, int theHeight, String theShader, longlong theFormat, bool wantReadable) {RenderToSprite::Go(theWidth,theHeight,theFormat,wantReadable);mShader=new Shader();mShader->Load(theShader);mOwnShader=true;}
void ShaderRenderToSprite::Go(int theWidth, int theHeight, String theShader, char* theFormat, bool wantReadable) {RenderToSprite::Go(theWidth,theHeight,theFormat,wantReadable);mShader=new Shader();mShader->Load(theShader);mOwnShader=true;}

bool ShaderRenderToSprite::GoLive(bool doThread)
{
	if (!mIsInitialized) return false;
	if (mAlwaysRender) Render();
	gG.SetTexture(mTexture,true);
	return true;
}

void ShaderRenderToSprite::Render()
{
	gG.RenderTo(*this);
	mShader->FillRect(0,0,(float)mWidth,(float)mHeight);
	gG.RenderTo();
}
#endif

void Sprite::MakeWarpArray(IPoint theWarpSize, Array<Point>& theXYArray, Array<Point>& theUVArray)
{
	if (theWarpSize.mX<2) theWarpSize.mX=2;
	if (theWarpSize.mY<2) theWarpSize.mY=2;

	int aMax=theWarpSize.mX*theWarpSize.mY;
	theXYArray[aMax-1]=0;
	theUVArray[aMax-1]=0;

	float aMultX=(float)theWarpSize.mX-1;
	float aMultY=(float)theWarpSize.mY-1;

	Point *aXYPtr=theXYArray.mArray;
	Point *aUVPtr=theUVArray.mArray;
	for (float aSpanY=0;aSpanY<theWarpSize.mY;aSpanY++)
	{
		for (float aSpanX=0;aSpanX<theWarpSize.mX;aSpanX++)
		{
			*aXYPtr=mDrawQuad.GetSubPoint(aSpanX/aMultX,aSpanY/aMultY);
			*aUVPtr=mTextureQuad.GetSubPoint(aSpanX/aMultX,aSpanY/aMultY);
			aXYPtr++;
			aUVPtr++;
		}
	}
}

void Sprite::DrawMagicBorder(float theX, float theY, float theWidth, float theHeight, bool doFill)
{
	Center(theX+GetHalfWidthF(),theY+GetHalfHeightF());
	CenterFlipped(theX+theWidth-GetHalfWidthF(),theY+GetHalfHeightF(),-1,1);
	CenterFlipped(theX+GetHalfWidthF(),theY+theHeight-GetHalfHeightF(),1,-1);
	CenterFlipped(theX+theWidth-GetHalfWidthF(),theY+theHeight-GetHalfHeightF(),-1,-1);

	Quad aDQ;
	Quad aTQ=GetTextureQuad().GetSubQuad(.95f,0,1.0f,1.0f);

	aDQ.mCorner[0]=Point(theX+GetWidthF(),theY);
	aDQ.mCorner[1]=Point(theX+theWidth-GetWidthF(),theY);
	aDQ.mCorner[2]=Point(theX+GetWidthF(),theY+GetHeightF());
	aDQ.mCorner[3]=Point(theX+theWidth-GetWidthF(),theY+GetHeightF());
	Draw(aDQ,aTQ);
	aTQ.FlipV();
	aDQ.Translate(Point(0.0f,theHeight-GetHeightF()));
	Draw(aDQ,aTQ);

	aTQ=GetTextureQuad().GetSubQuad(0,.95f,1,1);
	aDQ.mCorner[0]=Point(theX,theY+GetHeightF());
	aDQ.mCorner[1]=Point(theX+GetWidthF(),theY+GetHeightF());
	aDQ.mCorner[2]=Point(theX,theY+theHeight-GetHeightF());
	aDQ.mCorner[3]=Point(theX+GetWidthF(),theY+theHeight-GetHeightF());
	Draw(aDQ,aTQ);
	aTQ.FlipH();
	aDQ.Translate(Point(theWidth-GetWidthF(),0.0f));
	Draw(aDQ,aTQ);

	if (doFill)
	{
		aTQ=GetTextureQuad().GetSubQuad(.95f,.95f,1,1);
		aDQ.mCorner[0]=Point(theX+GetWidthF(),theY+GetHeightF());
		aDQ.mCorner[1]=Point(theX+theWidth-GetWidthF(),theY+GetHeightF());
		aDQ.mCorner[2]=Point(theX+GetWidthF(),theY+theHeight-GetHeightF());
		aDQ.mCorner[3]=Point(theX+theWidth-GetWidthF(),theY+theHeight-GetHeightF());
		Draw(aDQ,aTQ);
	}


}

void Sprite::DrawMagicBorderH(float theX, float theY, float theWidth)
{
	Center(theX+GetHalfWidthF(),theY+GetHalfHeightF());
	CenterFlipped(theX+theWidth-GetHalfWidthF(),theY+GetHalfHeightF(),-1,1);

	Quad aDQ;
	Quad aTQ=GetTextureQuad().GetSubQuad(.95f,0,1.0f,1.0f);

	aDQ.mCorner[0]=Point(theX+GetWidthF(),theY);
	aDQ.mCorner[1]=Point(theX+theWidth-GetWidthF(),theY);
	aDQ.mCorner[2]=Point(theX+GetWidthF(),theY+GetHeightF());
	aDQ.mCorner[3]=Point(theX+theWidth-GetWidthF(),theY+GetHeightF());
	Draw(aDQ,aTQ);
}

void Sprite::DrawMagicBorderV(float theX, float theY, float theHeight)
{
	Quad aDQ;
	Quad aTQ=GetTextureQuad().GetSubQuad(0,.95f,1.0f,1.0f);

	aDQ.mCorner[0]=Point(theX,theY);
	aDQ.mCorner[1]=Point(theX+GetDrawQuadWidth(),theY);
	aDQ.mCorner[2]=Point(theX,theY+GetDrawQuadHeight());
	aDQ.mCorner[3]=Point(theX+GetDrawQuadWidth(),theY+GetDrawQuadHeight());
	Draw(aDQ);
	aDQ.FlipV();
	aDQ.Translate(Point(0.0f,theHeight-GetDrawQuadHeight()));
	Draw(aDQ);

	aDQ.mCorner[0]=Point(theX,theY+GetDrawQuadHeight());
	aDQ.mCorner[1]=Point(theX+GetDrawQuadWidth(),theY+GetDrawQuadHeight());
	aDQ.mCorner[2]=Point(theX,theY+theHeight-GetDrawQuadHeight());
	aDQ.mCorner[3]=Point(theX+GetDrawQuadWidth(),theY+theHeight-GetDrawQuadHeight());

	Draw(aDQ,aTQ);
}


void Sprite::DrawMagicBorderH2(float theX, float theY, float theWidth)
{
	Quad aTQ1=GetTextureQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aTQ2=GetTextureQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	Quad aDQ1=GetDrawQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aDQ2=GetDrawQuad().GetSubQuad(.5f,0,1.0f,1.0f);

	aDQ1.MoveTo(theX,theY);
	Draw(aDQ1,aTQ1);
	aDQ2.MoveTo(theX+theWidth-GetWidthF()/2,theY);
	Draw(aDQ2,aTQ2);

	Quad aTQMid;
	Quad aDQMid;
	aTQMid.mCorner[0]=aTQ1.mCorner[1];
	aTQMid.mCorner[1]=aTQ2.mCorner[0];
	aTQMid.mCorner[2]=aTQ1.mCorner[3];
	aTQMid.mCorner[3]=aTQ2.mCorner[2];
	aDQMid.mCorner[0]=aDQ1.mCorner[1];
	aDQMid.mCorner[1]=aDQ2.mCorner[0];
	aDQMid.mCorner[2]=aDQ1.mCorner[3];
	aDQMid.mCorner[3]=aDQ2.mCorner[2];
	Draw(aDQMid,aTQMid);
}

/*
void Sprite::BillBoard(Vector thePos, float theRotation, Point theScale)
{
	if (!GoLive()) return;
	Matrix aMat;

	aMat.Rotate2D(theRotation);
	aMat.Scale(theScale.mX*gG.mBillboardScale,theScale.mY*gG.mBillboardScale,1);
	//aMat.Scale(gG.WidthF()/gG.HeightF()*theScale,theScale,theScale);
	//aMat.SetTranslate(0,0,0);
	aMat*=gG.GetBillboardMatrix();
	aMat.Translate(thePos);

	//aMat.DumpMakeCommand(PointAtDesktop("BILL.txt"));

	//gG.SetTexture(mTexture);
	int aC=gG.GetColor().ToInt();
	gBillboardV[0].mX=-mDrawQuad.mCorner[0].mX;
	gBillboardV[0].mY=mDrawQuad.mCorner[0].mY;
	gBillboardV[0].mZ=0;
	gBillboardV[0].mDiffuse=aC;
	gBillboardV[0].mTextureU=mTextureQuad.mCorner[0].mX;
	gBillboardV[0].mTextureV=mTextureQuad.mCorner[0].mY;

	gBillboardV[1].mX=-mDrawQuad.mCorner[1].mX;
	gBillboardV[1].mY=mDrawQuad.mCorner[1].mY;
	gBillboardV[1].mZ=0;
	gBillboardV[1].mDiffuse=aC;
	gBillboardV[1].mTextureU=mTextureQuad.mCorner[1].mX;
	gBillboardV[1].mTextureV=mTextureQuad.mCorner[1].mY;

	gBillboardV[2].mX=-mDrawQuad.mCorner[2].mX;
	gBillboardV[2].mY=mDrawQuad.mCorner[2].mY;
	gBillboardV[2].mZ=0;
	gBillboardV[2].mDiffuse=aC;
	gBillboardV[2].mTextureU=mTextureQuad.mCorner[2].mX;
	gBillboardV[2].mTextureV=mTextureQuad.mCorner[2].mY;

	gBillboardV[3].mX=-mDrawQuad.mCorner[3].mX;
	gBillboardV[3].mY=mDrawQuad.mCorner[3].mY;
	gBillboardV[3].mZ=0;
	gBillboardV[3].mDiffuse=aC;
	gBillboardV[3].mTextureU=mTextureQuad.mCorner[3].mX;
	gBillboardV[3].mTextureV=mTextureQuad.mCorner[3].mY;

	//gG.CullNone();
	gG.PushWorldMatrix();
	gG.SetWorldMatrix(aMat);
	gG.PumpIndexedShape(gBillboardV,gBillboardI);
	gG.PopWorldMatrix();
}

void Sprite::BillBoardMT(Vector thePos, float theRotation, Point theScale)
{
	if (!GoLive()) return;
	Matrix aMat;

	aMat.Rotate2D(theRotation);
	aMat.Scale(theScale.mX*gG.mBillboardScale,theScale.mY*gG.mBillboardScale,1);
	//aMat.Scale(gG.WidthF()/gG.HeightF()*theScale,theScale,theScale);
	//aMat.SetTranslate(0,0,0);
	aMat*=gG.GetBillboardMatrix();
	aMat.Translate(thePos);

	//gG.SetTexture(mTexture);
	int aC=gG.GetColor().ToInt();
	gBillboardVMT[0].mX=-mDrawQuad.mCorner[0].mX;
	gBillboardVMT[0].mY=mDrawQuad.mCorner[0].mY;
	gBillboardVMT[0].mZ=0;
	gBillboardVMT[0].mDiffuse=aC;
	gBillboardVMT[0].mTextureU=mTextureQuad.mCorner[0].mX;
	gBillboardVMT[0].mTextureV=mTextureQuad.mCorner[0].mY;
	gBillboardVMT[0].mTextureU2=0;
	gBillboardVMT[0].mTextureV2=0;

	gBillboardVMT[1].mX=-mDrawQuad.mCorner[1].mX;
	gBillboardVMT[1].mY=mDrawQuad.mCorner[1].mY;
	gBillboardVMT[1].mZ=0;
	gBillboardVMT[1].mDiffuse=aC;
	gBillboardVMT[1].mTextureU=mTextureQuad.mCorner[1].mX;
	gBillboardVMT[1].mTextureV=mTextureQuad.mCorner[1].mY;
	gBillboardVMT[1].mTextureU2=0;
	gBillboardVMT[1].mTextureV2=0;

	gBillboardVMT[2].mX=-mDrawQuad.mCorner[2].mX;
	gBillboardVMT[2].mY=mDrawQuad.mCorner[2].mY;
	gBillboardVMT[2].mZ=0;
	gBillboardVMT[2].mDiffuse=aC;
	gBillboardVMT[2].mTextureU=mTextureQuad.mCorner[2].mX;
	gBillboardVMT[2].mTextureV=mTextureQuad.mCorner[2].mY;
	gBillboardVMT[2].mTextureU2=0;
	gBillboardVMT[2].mTextureV2=0;

	gBillboardVMT[3].mX=-mDrawQuad.mCorner[3].mX;
	gBillboardVMT[3].mY=mDrawQuad.mCorner[3].mY;
	gBillboardVMT[3].mZ=0;
	gBillboardVMT[3].mDiffuse=aC;
	gBillboardVMT[3].mTextureU=mTextureQuad.mCorner[3].mX;
	gBillboardVMT[3].mTextureV=mTextureQuad.mCorner[3].mY;
	gBillboardVMT[3].mTextureU2=0;
	gBillboardVMT[3].mTextureV2=0;

	//gG.CullNone();
	gG.SetWorldMatrix(aMat);
	gG.PumpIndexedShape(gBillboardVMT,gBillboardI);
	gG.SetWorldMatrix();
}

void Sprite::BillBoardFast(Vector thePos, float theRotation, Point theScale)
{
	Color aCC=gG.GetColor();
	if (aCC.mR+aCC.mB+aCC.mG>2.95f)
	{
		if (!GoLive()) return;
		Matrix aMat;

		aMat.Rotate2D(theRotation);
		aMat.Scale(theScale.mX*gG.mBillboardScale,theScale.mY*gG.mBillboardScale,1);
		//aMat.Scale(gG.WidthF()/gG.HeightF()*theScale,theScale,theScale);
		//aMat.SetTranslate(0,0,0);
		aMat*=gG.GetBillboardMatrix();
		aMat.Translate(thePos);

		//
		// Fastest Billboards, for uncolorized... we just have a shape for each of the BILLBOARDLEVELS
		// created...
		//
		int aLevel=(int)((1.0f-aCC.mA)*256)/(256/(gG.GetBillboardLevels()));
		if (mShape[aLevel]<=0)
		{
			int aC=aCC.ToInt();
            
			gBillboardV[0].mX=-mDrawQuad.mCorner[0].mX;
			gBillboardV[0].mY=mDrawQuad.mCorner[0].mY;
			gBillboardV[0].mZ=0;
			gBillboardV[0].mDiffuse=aC;
			gBillboardV[0].mTextureU=mTextureQuad.mCorner[0].mX;
			gBillboardV[0].mTextureV=mTextureQuad.mCorner[0].mY;

			gBillboardV[1].mX=-mDrawQuad.mCorner[1].mX;
			gBillboardV[1].mY=mDrawQuad.mCorner[1].mY;
			gBillboardV[1].mZ=0;
			gBillboardV[1].mDiffuse=aC;
			gBillboardV[1].mTextureU=mTextureQuad.mCorner[1].mX;
			gBillboardV[1].mTextureV=mTextureQuad.mCorner[1].mY;

			gBillboardV[2].mX=-mDrawQuad.mCorner[2].mX;
			gBillboardV[2].mY=mDrawQuad.mCorner[2].mY;
			gBillboardV[2].mZ=0;
			gBillboardV[2].mDiffuse=aC;
			gBillboardV[2].mTextureU=mTextureQuad.mCorner[2].mX;
			gBillboardV[2].mTextureV=mTextureQuad.mCorner[2].mY;

			gBillboardV[3].mX=-mDrawQuad.mCorner[3].mX;
			gBillboardV[3].mY=mDrawQuad.mCorner[3].mY;
			gBillboardV[3].mZ=0;
			gBillboardV[3].mDiffuse=aC;
			gBillboardV[3].mTextureU=mTextureQuad.mCorner[3].mX;
			gBillboardV[3].mTextureV=mTextureQuad.mCorner[3].mY;

			mShape[aLevel]=gG.CreateShape(gBillboardV,gBillboardI);
		}

		//gG.SetTexture(mTexture);
		gG.SetWorldMatrix(aMat);
		gG.DrawShape(mShape[aLevel]);
		gG.SetWorldMatrix();
	}
	else BillBoard(thePos,theRotation,theScale);
}

void Sprite::BillBoardFastMT(Vector thePos, float theRotation, Point theScale)
{
	Color aCC=gG.GetColor();
	if (aCC.mR+aCC.mB+aCC.mG>2.95f)
	{
		if (!GoLive()) return;
		Matrix aMat;

		aMat.Rotate2D(theRotation);
		aMat.Scale(theScale.mX*gG.mBillboardScale,theScale.mY*gG.mBillboardScale,1);
		//aMat.Scale(gG.WidthF()/gG.HeightF()*theScale,theScale,theScale);
		//aMat.SetTranslate(0,0,0);
		aMat*=gG.GetBillboardMatrix();
		aMat.Translate(thePos);

		//
		// Fastest Billboards, for uncolorized... we just have a shape for each of the BILLBOARDLEVELS
		// created...
		//
		int aLevel=(int)((1.0f-aCC.mA)*256)/(256/(gG.GetBillboardLevels()));
		if (mShapeMT[aLevel]<=0)
		{
			int aC=aCC.ToInt();

			gBillboardVMT[0].mX=-mDrawQuad.mCorner[0].mX;
			gBillboardVMT[0].mY=mDrawQuad.mCorner[0].mY;
			gBillboardVMT[0].mZ=0;
			gBillboardVMT[0].mDiffuse=aC;
			gBillboardVMT[0].mTextureU=mTextureQuad.mCorner[0].mX;
			gBillboardVMT[0].mTextureV=mTextureQuad.mCorner[0].mY;
			gBillboardVMT[0].mTextureU2=0;
			gBillboardVMT[0].mTextureV2=0;

			gBillboardVMT[1].mX=-mDrawQuad.mCorner[1].mX;
			gBillboardVMT[1].mY=mDrawQuad.mCorner[1].mY;
			gBillboardVMT[1].mZ=0;
			gBillboardVMT[1].mDiffuse=aC;
			gBillboardVMT[1].mTextureU=mTextureQuad.mCorner[1].mX;
			gBillboardVMT[1].mTextureV=mTextureQuad.mCorner[1].mY;
			gBillboardVMT[1].mTextureU2=0;
			gBillboardVMT[1].mTextureV2=0;

			gBillboardVMT[2].mX=-mDrawQuad.mCorner[2].mX;
			gBillboardVMT[2].mY=mDrawQuad.mCorner[2].mY;
			gBillboardVMT[2].mZ=0;
			gBillboardVMT[2].mDiffuse=aC;
			gBillboardVMT[2].mTextureU=mTextureQuad.mCorner[2].mX;
			gBillboardVMT[2].mTextureV=mTextureQuad.mCorner[2].mY;
			gBillboardVMT[2].mTextureU2=0;
			gBillboardVMT[2].mTextureV2=0;

			gBillboardVMT[3].mX=-mDrawQuad.mCorner[3].mX;
			gBillboardVMT[3].mY=mDrawQuad.mCorner[3].mY;
			gBillboardVMT[3].mZ=0;
			gBillboardVMT[3].mDiffuse=aC;
			gBillboardVMT[3].mTextureU=mTextureQuad.mCorner[3].mX;
			gBillboardVMT[3].mTextureV=mTextureQuad.mCorner[3].mY;
			gBillboardVMT[3].mTextureU2=0;
			gBillboardVMT[3].mTextureV2=0;

			mShapeMT[aLevel]=gG.CreateShape(gBillboardVMT,gBillboardI);
		}

		//gG.SetTexture(mTexture);
		gG.SetWorldMatrix(aMat);
		gG.DrawShape(mShapeMT[aLevel]);
		gG.SetWorldMatrix();
	}
	else BillBoardMT(thePos,theRotation,theScale);
}
*/

void Sprite::DrawMagicBorderEX_LeftRight(float theX, float theY, float theWidth, float theHeight, bool doFill)
{
	Quad aTQ1=GetTextureQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aTQ2=GetTextureQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	aTQ1.FlipV();
	aTQ2.FlipV();

	Quad aDQ1=GetDrawQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aDQ2=GetDrawQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	Quad aDQ3=GetDrawQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aDQ4=GetDrawQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	aDQ3.FlipV();
	aDQ4.FlipV();

	aDQ1.MoveTo(theX,theY);
	Draw(aDQ1,aTQ1);
	aDQ2.MoveTo(theX+theWidth-GetWidthF()/2,theY);
	Draw(aDQ2,aTQ2);
	/*
	//
	// Fixed?
	// This was drawing left and right twice, which showed up in additive.
	// Is it needed somewhere else?
	//
	aDQ3.MoveTo(theX,theY+theHeight);//-GetHeightF());
	Draw(aDQ3,aTQ1);
	aDQ4.MoveTo(theX+theWidth-GetWidthF()/2,theY+theHeight);//-GetHeightF());
	Draw(aDQ4,aTQ2);
	*/

	Quad aTQMid;
	Quad aDQMid;
	aTQMid.mCorner[0]=aTQ1.mCorner[1];
	aTQMid.mCorner[1]=aTQ2.mCorner[0];
	aTQMid.mCorner[2]=aTQ1.mCorner[3];
	aTQMid.mCorner[3]=aTQ2.mCorner[2];
	aDQMid.mCorner[0]=aDQ1.mCorner[1];
	aDQMid.mCorner[1]=aDQ2.mCorner[0];
	aDQMid.mCorner[2]=aDQ1.mCorner[3];
	aDQMid.mCorner[3]=aDQ2.mCorner[2];
	//
	// With the above "fixed?" uncommented, we have to flip this (why?)
	// Not sure if this is an artifact of copying from somewhere or what.
	//
	//aDQMid.FlipV();
	Draw(aDQMid,aTQMid);

	/*
	aDQMid.mCorner[0]=aDQ3.mCorner[1];
	aDQMid.mCorner[1]=aDQ4.mCorner[0];
	aDQMid.mCorner[2]=aDQ3.mCorner[3];
	aDQMid.mCorner[3]=aDQ4.mCorner[2];
	Draw(aDQMid,aTQMid);


	aTQMid.mCorner[0]=aTQ1.mCorner[2];
	aTQMid.mCorner[1]=aTQ1.mCorner[3];
	aTQMid.mCorner[2]=aTQ1.mCorner[2];
	aTQMid.mCorner[3]=aTQ1.mCorner[3];
	aDQMid.mCorner[0]=aDQ1.mCorner[2];
	aDQMid.mCorner[1]=aDQ1.mCorner[3];
	aDQMid.mCorner[2]=aDQ3.mCorner[2];
	aDQMid.mCorner[3]=aDQ3.mCorner[3];
	Draw(aDQMid,aTQMid);

	aTQMid.mCorner[0]=aTQ2.mCorner[2];
	aTQMid.mCorner[1]=aTQ2.mCorner[3];
	aTQMid.mCorner[2]=aTQ2.mCorner[2];
	aTQMid.mCorner[3]=aTQ2.mCorner[3];
	aDQMid.mCorner[0]=aDQ2.mCorner[2];
	aDQMid.mCorner[1]=aDQ2.mCorner[3];
	aDQMid.mCorner[2]=aDQ4.mCorner[2];
	aDQMid.mCorner[3]=aDQ4.mCorner[3];
	Draw(aDQMid,aTQMid);

	/*
	if (doFill)
	{
		aTQMid.mCorner[0]=aTQ1.mCorner[3];
		aTQMid.mCorner[1]=aTQ2.mCorner[2];
		aTQMid.mCorner[2]=aTQ1.mCorner[3];
		aTQMid.mCorner[3]=aTQ2.mCorner[2];

		aDQMid.mCorner[0]=aDQ1.mCorner[3];
		aDQMid.mCorner[1]=aDQ2.mCorner[2];
		aDQMid.mCorner[2]=aDQ3.mCorner[3];
		aDQMid.mCorner[3]=aDQ4.mCorner[2];
		Draw(aDQMid,aTQMid);
	}
	/**/

}

void Sprite::DrawMagicBorderEX_LeftRight_TopOnly(float theX, float theY, float theWidth, float theHeight, bool doFill)
{
	Quad aTQ1=GetTextureQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aTQ2=GetTextureQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	Quad aDQ1=GetDrawQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aDQ2=GetDrawQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	Quad aDQ3=GetDrawQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aDQ4=GetDrawQuad().GetSubQuad(.5f,0,1.0f,1.0f);

	aDQ1.MoveTo(theX,theY);
	Draw(aDQ1,aTQ1);
	aDQ2.MoveTo(theX+theWidth-GetWidthF()/2,theY);
	Draw(aDQ2,aTQ2);
	aDQ3.MoveTo(theX,theY+theHeight);//-GetHeightF());
	aDQ4.MoveTo(theX+theWidth-GetWidthF()/2,theY+theHeight);//-GetHeightF());

	Quad aTQMid;
	Quad aDQMid;

	aTQMid.mCorner[0]=aTQ1.mCorner[1];
	aTQMid.mCorner[1]=aTQ2.mCorner[0];
	aTQMid.mCorner[2]=aTQ1.mCorner[3];
	aTQMid.mCorner[3]=aTQ2.mCorner[2];
	aDQMid.mCorner[0]=aDQ1.mCorner[1];
	aDQMid.mCorner[1]=aDQ2.mCorner[0];
	aDQMid.mCorner[2]=aDQ1.mCorner[3];
	aDQMid.mCorner[3]=aDQ2.mCorner[2];
	Draw(aDQMid,aTQMid);


	aTQMid.mCorner[0]=aTQ1.mCorner[2];
	aTQMid.mCorner[1]=aTQ1.mCorner[3];
	aTQMid.mCorner[2]=aTQ1.mCorner[2];
	aTQMid.mCorner[3]=aTQ1.mCorner[3];
	aDQMid.mCorner[0]=aDQ1.mCorner[2];
	aDQMid.mCorner[1]=aDQ1.mCorner[3];
	aDQMid.mCorner[2]=aDQ3.mCorner[0];
	aDQMid.mCorner[3]=aDQ3.mCorner[1];
	Draw(aDQMid,aTQMid);

	aTQMid.mCorner[0]=aTQ2.mCorner[2];
	aTQMid.mCorner[1]=aTQ2.mCorner[3];
	aTQMid.mCorner[2]=aTQ2.mCorner[2];
	aTQMid.mCorner[3]=aTQ2.mCorner[3];
	aDQMid.mCorner[0]=aDQ2.mCorner[2];
	aDQMid.mCorner[1]=aDQ2.mCorner[3];
	aDQMid.mCorner[2]=aDQ4.mCorner[0];
	aDQMid.mCorner[3]=aDQ4.mCorner[1];
	Draw(aDQMid,aTQMid);

	if (doFill)
	{
		aTQMid.mCorner[0]=aTQ1.mCorner[3];
		aTQMid.mCorner[1]=aTQ2.mCorner[2];
		aTQMid.mCorner[2]=aTQ1.mCorner[3];
		aTQMid.mCorner[3]=aTQ2.mCorner[2];

		aDQMid.mCorner[0]=aDQ1.mCorner[3];
		aDQMid.mCorner[1]=aDQ2.mCorner[2];
		aDQMid.mCorner[2]=aDQ3.mCorner[1];
		aDQMid.mCorner[3]=aDQ4.mCorner[0];
		Draw(aDQMid,aTQMid);
	}
}

void Sprite::DrawMagicBorderEX_LeftRight_BottomOnly(float theX, float theY, float theWidth, float theHeight, bool doFill)
{
	Quad aTQ1=GetTextureQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aTQ2=GetTextureQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	Quad aDQ1=GetDrawQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aDQ2=GetDrawQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	Quad aDQ3=GetDrawQuad().GetSubQuad(0,0,.5f,1.0f);
	Quad aDQ4=GetDrawQuad().GetSubQuad(.5f,0,1.0f,1.0f);
	aDQ3.FlipV();
	aDQ4.FlipV();

	aDQ1.MoveTo(theX,theY);
	//Draw(aDQ1,aTQ1);
	aDQ2.MoveTo(theX+theWidth-GetWidthF()/2,theY);
	//Draw(aDQ2,aTQ2);
	aDQ3.MoveTo(theX,theY+theHeight);//-GetHeightF());
	Draw(aDQ3,aTQ1);
	aDQ4.MoveTo(theX+theWidth-GetWidthF()/2,theY+theHeight);//-GetHeightF());
	Draw(aDQ4,aTQ2);


	Quad aTQMid;
	Quad aDQMid;
	aTQMid.mCorner[0]=aTQ1.mCorner[1];
	aTQMid.mCorner[1]=aTQ2.mCorner[0];
	aTQMid.mCorner[2]=aTQ1.mCorner[3];
	aTQMid.mCorner[3]=aTQ2.mCorner[2];
	aDQMid.mCorner[0]=aDQ3.mCorner[1];
	aDQMid.mCorner[1]=aDQ4.mCorner[0];
	aDQMid.mCorner[2]=aDQ3.mCorner[3];
	aDQMid.mCorner[3]=aDQ4.mCorner[2];
	Draw(aDQMid,aTQMid);


	aTQMid.mCorner[0]=aTQ1.mCorner[2];
	aTQMid.mCorner[1]=aTQ1.mCorner[3];
	aTQMid.mCorner[2]=aTQ1.mCorner[2];
	aTQMid.mCorner[3]=aTQ1.mCorner[3];
	aDQMid.mCorner[0]=aDQ1.mCorner[0];
	aDQMid.mCorner[1]=aDQ1.mCorner[1];
	aDQMid.mCorner[2]=aDQ3.mCorner[2];
	aDQMid.mCorner[3]=aDQ3.mCorner[3];
	Draw(aDQMid,aTQMid);

	aTQMid.mCorner[0]=aTQ2.mCorner[2];
	aTQMid.mCorner[1]=aTQ2.mCorner[3];
	aTQMid.mCorner[2]=aTQ2.mCorner[2];
	aTQMid.mCorner[3]=aTQ2.mCorner[3];
	aDQMid.mCorner[0]=aDQ2.mCorner[0];
	aDQMid.mCorner[1]=aDQ2.mCorner[1];
	aDQMid.mCorner[2]=aDQ4.mCorner[2];
	aDQMid.mCorner[3]=aDQ4.mCorner[3];
	Draw(aDQMid,aTQMid);

	if (doFill)
	{
		aTQMid.mCorner[0]=aTQ1.mCorner[3];
		aTQMid.mCorner[1]=aTQ2.mCorner[2];
		aTQMid.mCorner[2]=aTQ1.mCorner[3];
		aTQMid.mCorner[3]=aTQ2.mCorner[2];

		aDQMid.mCorner[0]=aDQ1.mCorner[1];
		aDQMid.mCorner[1]=aDQ2.mCorner[0];
		aDQMid.mCorner[2]=aDQ3.mCorner[3];
		aDQMid.mCorner[3]=aDQ4.mCorner[2];
		Draw(aDQMid,aTQMid);
	}

}

void Sprite::DrawMagicBorderEX_AllCorners(float theX, float theY, float theWidth, float theHeight, bool doFill)
{
	Quad aTQ1=GetTextureQuad().GetSubQuad(0,0,.5f,.5f);
	Quad aTQ2=GetTextureQuad().GetSubQuad(.5f,0,1.0f,.5f);
	Quad aTQ3=GetTextureQuad().GetSubQuad(0,.5f,.5f,1.0f);
	Quad aTQ4=GetTextureQuad().GetSubQuad(.5f,.5f,1.0f,1.0f);

	Quad aDQ1=GetDrawQuad().GetSubQuad(0,0,.5f,.5f);
	Quad aDQ2=GetDrawQuad().GetSubQuad(.5f,0,1.0f,.5f);
	Quad aDQ3=GetDrawQuad().GetSubQuad(0,.5f,.5f,1.0f);
	Quad aDQ4=GetDrawQuad().GetSubQuad(.5f,.5f,1.0f,1.0f);

	//gG.FillRect(theX,theY,theWidth,theHeight);

	aDQ1.MoveTo(theX,theY);
	Draw(aDQ1,aTQ1);
	aDQ2.MoveTo(theX+theWidth-GetWidthF()/2,theY);
	Draw(aDQ2,aTQ2);
	aDQ3.MoveTo(theX,theY+theHeight-GetHeightF()/2);
	Draw(aDQ3,aTQ3);
	aDQ4.MoveTo(theX+theWidth-GetWidthF()/2,theY+theHeight-GetHeightF()/2);
	Draw(aDQ4,aTQ4);

	Quad aDQMid1(aDQ1.mCorner[1],aDQ2.mCorner[0],aDQ1.mCorner[3],aDQ2.mCorner[2]);
	Quad aTQMid1(aTQ1.mCorner[1],aTQ2.mCorner[0],aTQ1.mCorner[3],aTQ2.mCorner[2]);
	Draw(aDQMid1,aTQMid1);

	Quad aDQMid2(aDQ3.mCorner[1],aDQ4.mCorner[0],aDQ3.mCorner[3],aDQ4.mCorner[2]);
	Quad aTQMid2(aTQ3.mCorner[1],aTQ4.mCorner[0],aTQ3.mCorner[3],aTQ4.mCorner[2]);
	Draw(aDQMid2,aTQMid2);

	Quad aDQMid3(aDQ1.mCorner[2],aDQ3.mCorner[0],aDQ1.mCorner[3],aDQ3.mCorner[1]);
	Quad aTQMid3(aTQ1.mCorner[2],aTQ3.mCorner[0],aTQ1.mCorner[3],aTQ3.mCorner[1]);
	Draw(aDQMid3,aTQMid3);

	Quad aDQMid4(aDQ2.mCorner[2],aDQ4.mCorner[0],aDQ2.mCorner[3],aDQ4.mCorner[1]);
	Quad aTQMid4(aTQ2.mCorner[2],aTQ4.mCorner[0],aTQ2.mCorner[3],aTQ4.mCorner[1]);
	Draw(aDQMid4,aTQMid4);

	if (doFill)
	{
		Quad aDQMid5(aDQ1.mCorner[3],aDQ2.mCorner[2],aDQ3.mCorner[1],aDQ4.mCorner[0]);
		Quad aTQMid5(aTQ1.mCorner[3],aTQ2.mCorner[2],aTQ3.mCorner[1],aTQ4.mCorner[0]);
		Draw(aDQMid5,aTQMid5);
	}
}

void Sprite::ConvertUVCoordinate(float& theU, float& theV)
{
	//
	// First thing is to scale it to the new tex size...
	//
	Rect aBound=mDrawQuad.GetBounds();
	if (aBound.mWidth==mOriginalBounds.mWidth && aBound.mHeight==mOriginalBounds.mHeight) return; // Don't do anything is not cropped...

	float aModX=aBound.mWidth/WidthF();
	float aModY=aBound.mHeight/HeightF();

	//
	// Now we need to subtract whatever our upper left offset was, in texture coordinates.
	// So pixel coord/Sprite Size...
	//
	Point aOffset=aBound.UpperLeft()-mOriginalBounds.UpperLeft();
	aOffset/=GetSize();
	theU-=aOffset.mX;
	theV-=aOffset.mY;
	//
	// The scaling is okay
	//
	theU/=aModX;
	theV/=aModY;

}

bool SpriteBundle::DoesDynaExist(char* theDyna)
{
	EnumSmartList(DynaData,aDS,mDynaList) if (aDS->mName==theDyna) return true;
	return false;
}

IOBuffer& SpriteBundle::PointAtDyna(char* theDyna)
{
	EnumSmartList(DynaData,aDS,mDynaList)
	{
		if (aDS->mName==theDyna)
		{
			mData->Seek(aDS->mSeek);
			return *mData;
		}
	}

	mData->SeekEnd();
	return *mData;
}

void SpriteBundle::DumpDyna()
{
	EnumSmartList(DynaData,aDS,mDynaList) gOut.Out("Dyna: [%s]",aDS->mName.c());
}

//#include "graphics_core.h"

void Sprite::Load(String theFilename, int theImageMode, bool setInitialized)
{
	mIsInitialized=false;

	Image aImage;
	aImage.Load(theFilename);
	Load(aImage);
	
	mWidth=aImage.mWidth;
	mHeight=aImage.mHeight;
	
	if (gG.MustPow2Textures()) if (!gMath.IsPowerOfTwo(mWidth) || !gMath.IsPowerOfTwo(mHeight)) aImage.MakePow2(true);


	//mTexture=gG.LoadTexture(theFilename);
	mTexture=gG.LoadTexture(aImage);
	if (mTexture==-1) return;


	/*
	IPoint aDims=gG.GetTextureDimensions(mTexture);
	mWidth=aDims.mX;
	mHeight=aDims.mY;

	if (gG.MustPow2Textures()) if (!gMath.IsPowerOfTwo(mWidth) || !gMath.IsPowerOfTwo(mHeight))
	{
		gOut.Out("!!!! Not a POW2 Texture: %s",theFilename.c());
	}
	 */

	float aModWidth=(float)mWidth;
	float aModHeight=(float)mHeight;
	mOwnTexture=true;

	Point aDrawDims=Point(mWidth,mHeight)/2;
	mDrawQuad.UpperLeft()=aDrawDims*Point(-1,-1);
	mDrawQuad.UpperRight()=aDrawDims*Point(1,-1);
	mDrawQuad.LowerLeft()=aDrawDims*Point(-1,1);
	mDrawQuad.LowerRight()=aDrawDims*Point(1,1);

	float aTWidth=WidthF()/aModWidth;
	float aTHeight=HeightF()/aModHeight;

	float aPixel=1/WidthF();
	aPixel*=gG.KludgeTexels();

	mTextureQuad.UpperLeft()=Point(aPixel,aPixel);
	mTextureQuad.UpperRight()=Point(aTWidth-aPixel,aPixel);
	mTextureQuad.LowerLeft()=Point(aPixel,aTHeight-aPixel);
	mTextureQuad.LowerRight()=Point(aTWidth-aPixel,aTHeight-aPixel);

	//
	// Finish up...
	//
	mDrawQuadWidth=mDrawQuad.UpperRight().mX-mDrawQuad.UpperLeft().mX;
	mDrawQuadHeight=mDrawQuad.LowerRight().mY-mDrawQuad.UpperRight().mY;
	mDrawQuadCenter=mDrawQuad.GetBounds().Center();
	mBounds=mDrawQuad.GetBounds();

	mOriginalBounds=Rect(WidthF(),HeightF());
	mOriginalBounds.CenterAt(mDrawQuadCenter);//-Point(theXMove,theYMove));

	if (setInitialized) mIsInitialized=true;
}

void Sprite::Load(Image& theImage, int theImageMode, bool setInitialized)
{
	mIsInitialized=false;
	Image& aImage=theImage;

	mWidth=aImage.mWidth;
	mHeight=aImage.mHeight;

	if (gG.MustPow2Textures()) aImage.MakePow2(true);

	float aModWidth=(float)aImage.mWidth;
	float aModHeight=(float)aImage.mHeight;

	mOwnTexture=true;
    
	switch (theImageMode)
	{
	case 8888:mTexture=gG.LoadTexture(aImage);break;
	case 565:mTexture=gG.LoadTexture565(aImage);break;
	case 4444:mTexture=gG.LoadTexture4444(aImage);break;
	}

	Point aDrawDims=Point(mWidth,mHeight)/2;
	mDrawQuad.UpperLeft()=aDrawDims*Point(-1,-1);
	mDrawQuad.UpperRight()=aDrawDims*Point(1,-1);
	mDrawQuad.LowerLeft()=aDrawDims*Point(-1,1);
	mDrawQuad.LowerRight()=aDrawDims*Point(1,1);

	float aTWidth=WidthF()/aModWidth;
	float aTHeight=HeightF()/aModHeight;

	float aPixel=1/WidthF();
	aPixel*=gG.KludgeTexels();

	mTextureQuad.UpperLeft()=Point(aPixel,aPixel);
	mTextureQuad.UpperRight()=Point(aTWidth-aPixel,aPixel);
	mTextureQuad.LowerLeft()=Point(aPixel,aTHeight-aPixel);
	mTextureQuad.LowerRight()=Point(aTWidth-aPixel,aTHeight-aPixel);

	//
	// Finish up...
	//
	mDrawQuadWidth=mDrawQuad.UpperRight().mX-mDrawQuad.UpperLeft().mX;
	mDrawQuadHeight=mDrawQuad.LowerRight().mY-mDrawQuad.UpperRight().mY;
	mDrawQuadCenter=mDrawQuad.GetBounds().Center();
	mBounds=mDrawQuad.GetBounds();

	mOriginalBounds=Rect(WidthF(),HeightF());
	mOriginalBounds.CenterAt(mDrawQuadCenter);//-Point(theXMove,theYMove));

	if (setInitialized) mIsInitialized=true;
}

void Sprite::Copy(Sprite* theSprite)
{
	mIsInitialized=theSprite->mIsInitialized;
	mOwnShape=false;
	mOwnTexture=false;
	mTexture=theSprite->mTexture;
	mShape.Copy(theSprite->mShape);
	mShapeMT.Copy(theSprite->mShape);
	mDrawQuad=theSprite->mDrawQuad;
	mTextureQuad=theSprite->mTextureQuad;
	mDrawQuadCenter=theSprite->mDrawQuadCenter;
	mBounds=theSprite->mBounds;
	mOriginalBounds=theSprite->mOriginalBounds;
	mWidth=theSprite->mWidth;
	mHeight=theSprite->mHeight;
	mDrawQuadWidth=theSprite->mDrawQuadWidth;
	mDrawQuadHeight=theSprite->mDrawQuadHeight;
	mKey.Copy(theSprite->mKey);
	mTexelSize=theSprite->mTexelSize;
	mTexelMove=theSprite->mTexelMove;
}

void Sprite::FillRect(float theX, float theY, float theWidth, float theHeight,float theScale)
{
	gG.PushClip();
	gG.Clip(theX,theY,theWidth,theHeight);
	float aW=mWidth*theScale;
	float aH=mHeight*theScale;

	for (float aSpanX=theX+(aW/2);aSpanX<theX+theWidth+(aW);aSpanX+=aW)
	{
		for (float aSpanY=theY+(aH/2);aSpanY<theY+theHeight+(aH);aSpanY+=aH)
		{
			DrawScaled(aSpanX,aSpanY,theScale);
		}
	}
	gG.PopClip();
}

Sprite* SpriteBundle::GetSpriteByName(String theName)
{
	EnumSmartList(NameToSprite,aN,mNameToSpriteList) if (aN->mName==theName) return aN->mSprite;
	return NULL;
}

void DynamicSprite::ManualLoad(IOBuffer& theBuffer)
{
	mFilename="images://";mFilename+=theBuffer.ReadString();

	//gOut.Out("Dynamic::ManulaLoad: [ %s ]",mFilename.c());
	mImageMode=8888;

	//gOut.Out("Sprite Filename: %s",mFilename.c());

	mNewDQ.mCorner[0]=theBuffer.ReadPoint();
	mNewDQ.mCorner[1]=theBuffer.ReadPoint();
	mNewDQ.mCorner[2]=theBuffer.ReadPoint();
	mNewDQ.mCorner[3]=theBuffer.ReadPoint();
	
	mNewTQ.mCorner[0]=theBuffer.ReadPoint();
	mNewTQ.mCorner[1]=theBuffer.ReadPoint();
	mNewTQ.mCorner[2]=theBuffer.ReadPoint();
	mNewTQ.mCorner[3]=theBuffer.ReadPoint();

	mWidth=theBuffer.ReadInt();
	mHeight=theBuffer.ReadInt();

	mDrawQuad=mNewDQ;
	mTextureQuad=mNewTQ;

	mDrawQuadWidth=mDrawQuad.UpperRight().mX-mDrawQuad.UpperLeft().mX;
	mDrawQuadHeight=mDrawQuad.LowerRight().mY-mDrawQuad.UpperRight().mY;
	mDrawQuadCenter=mDrawQuad.GetBounds().Center();
	mBounds=mDrawQuad.GetBounds();

	int aKeyCount=theBuffer.ReadInt();
	for (int aCount=0;aCount<aKeyCount;aCount++)
	{
		Point aKey=theBuffer.ReadPoint();
		mKey[aCount]=aKey;
	}
}


int DynamicSprite::mStubbingCount=0;
int DynamicSprite::mStubbingMax=10;
void DynamicSprite::LoadStub(void* theArg)
{
	gG.Threading();
	DynamicSprite* aDS=(DynamicSprite*)theArg;

	while (aDS->mStubbingCount>=aDS->mStubbingMax) Sleep(2);
	aDS->mStubbingCount++;

	/*
	#ifdef _DEBUG
	//Sleep(100*(gRand.GetSpan(1,5)));
	static char aData[100];
	memset(aData,0,100);
	strncpy(aData,aDS->mFilename.c(),80);
	gOut.Out("* -------------> DynamicSprite::LoadStub: [%s]",aData);
	#endif
	/**/

	if (DoesImageExist(aDS->mFilename)) 
	{
		Image aImage;
		aImage.Load(aDS->mFilename);
		Sprite* aS=(Sprite*)theArg;

		aS->Load(aImage,8888,false);

		if (aDS->mNewDQ.mCorner[3].LengthSquared()>0) aDS->mDrawQuad=aDS->mNewDQ;
		if (aDS->mNewTQ.mCorner[3].LengthSquared()>0) 
		{
			aDS->mTextureQuad=aDS->mNewTQ;
			if (gG.KludgeTexels()!=0)
			{
				float aPixelX=1.0f/aDS->WidthF();
				float aPixelY=1.0f/aDS->HeightF();
				aPixelX*=gG.KludgeTexels();
				aPixelY*=gG.KludgeTexels();
				aDS->mTextureQuad.Translate(Point(aPixelX,aPixelY));

				for (int aCount=0;aCount<4;aCount++) 
				{
					aDS->mTextureQuad.mCorner[aCount].mX=_clamp(aPixelX,aDS->mTextureQuad.mCorner[aCount].mX,1.0f-aPixelX);
					aDS->mTextureQuad.mCorner[aCount].mY=_clamp(aPixelY,aDS->mTextureQuad.mCorner[aCount].mY,1.0f-aPixelY);
				}
			}
		}

		aDS->mDrawQuadWidth=aDS->mDrawQuad.UpperRight().mX-aDS->mDrawQuad.UpperLeft().mX;
		aDS->mDrawQuadHeight=aDS->mDrawQuad.LowerRight().mY-aDS->mDrawQuad.UpperRight().mY;
		aDS->mDrawQuadCenter=aDS->mDrawQuad.GetBounds().Center();
		aDS->mBounds=aDS->mDrawQuad.GetBounds();

		aS->mIsInitialized=true;
	}
	aDS->mStubbingCount--;
	UNSETBITS(aDS->mFlags,DYNAMIC_STUBBING);
}

bool DynamicSprite::GoLive(bool doThread)
{
	FetchImage(doThread);
	if (mTexture<0) return false;
	return Sprite::GoLive();
}

class DynamicSpriteWatcher : public Object
{ 
public:
	DynamicSpriteWatcher(DynamicSprite* theSpr) {mSprite=theSpr;theSpr->mHookedTo=&mSprite;gAppPtr->mBackgroundProcesses+=this;}
	void Update() {if (!gAppPtr) return;if (mSprite) mSprite->Update();if (!mSprite) Kill();}
	DynamicSprite*		mSprite;
};

void DynamicSprite::FetchImage(bool doThread)
{
	if (mFilename.Length()==0)
	{
		//
		// Still downloading, or something...
		//
		mLiveCount=-1;
		return;
	}
	if (mLiveCount<=0)
	{
		if (gAppPtr)
		{
			if (!(mFlags&DYNAMIC_NOWATCH)) DynamicSpriteWatcher* aWatch=new DynamicSpriteWatcher(this);

			mIsInitialized=false;
			mTexture=-1;

			mDrawQuad.mCorner[0]=Point(-9999,-9999);
			mDrawQuad.mCorner[1]=Point(-9999,-9999);
			mDrawQuad.mCorner[2]=Point(-9999,-9999);
			mDrawQuad.mCorner[3]=Point(-9999,-9999);
			mTextureQuad.mCorner[0]=Point(0,0);
			mTextureQuad.mCorner[1]=Point(0,0);
			mTextureQuad.mCorner[2]=Point(0,0);
			mTextureQuad.mCorner[3]=Point(0,0);
#ifndef NO_THREADS
			if (doThread) 
			{
				mFlags|=DYNAMIC_STUBBING; // A dynamic sprite will not finish delete while stubbing!
				Thread(&LoadStub,this);
			}
			else LoadStub(this);
#else
			WARNING("Potentially a pause-o-matic here with dynamically loading sprites...");
			LoadStub(this);
#endif
#ifdef DEBUG_DYNAMIC
			gOut.Out("$(2)Load Dynamic Sprite: %s",mFilename.c());
#endif
		}
	}
	mLiveCount=mLiveCountMax;
}

void DynamicSprite::Update()
{
	if (mIsInitialized)
	if (--mLiveCount<=0)
	{
		if (gAppPtr)
		{
			if (mHookedTo) *mHookedTo=NULL;
			mHookedTo=NULL;
			Unload();
#ifdef DEBUG_DYNAMIC
			gOut.Out("+!Unload Dynamic Sprite: %s [%d]",mFilename.c(),mLiveCount);
#endif
		}
		mLiveCount=0;
	}
}

char gHoldTextureMode;
void DynamicSprite::Prep() {gHoldTextureMode=gG.mTextureMode;gG.ClampTextures();}
void DynamicSprite::Unprep() {gG.mTextureMode=gHoldTextureMode;}

class SpriteDownloader : public Object
{
public:
	Smart(Image) mImage;
	Smart(Sprite) mSprite;
	Smart(DynamicSprite) mDSprite;
	HOOKARGPTR mHook=NULL;
	void* mHookParam=NULL;
	String mCacheName;

	void Update()
	{
		if (!mImage->IsDownloading())
		{
			if (mSprite) mSprite->Load(*mImage);
			if (mDSprite) mDSprite->Load(mCacheName);
			if (mHook) mHook(mHookParam);
			Kill();
		}
	}
};

Smart(Sprite) DownloadSprite(String theURL, String theVersion, float* theProgress, HOOKARGPTR theHook, void* theHookParam)
{
	SpriteDownloader* aSD=new SpriteDownloader;
	aSD->mImage=DownloadImage(theURL,theVersion,theProgress);
	aSD->mSprite=new Sprite;
	aSD->mHook=theHook;
	aSD->mHookParam=theHookParam;
	aSD->Update();
	if (!aSD->mKill) gAppPtr->mBackgroundProcesses+=aSD;

	return aSD->mSprite;
}

Smart(DynamicSprite) DownloadDynamicSprite(String theURL, String theVersion, float* theProgress, HOOKARGPTR theHook, void* theHookParam)
{
	SpriteDownloader* aSD=new SpriteDownloader;
	aSD->mImage=DownloadImage(theURL,theVersion,theProgress);
	aSD->mDSprite=new DynamicSprite;
	aSD->mCacheName=URLtoWebCache(theURL,theVersion);
	aSD->mHook=theHook;
	aSD->mHookParam=theHookParam;
	aSD->Update();
	if (!aSD->mKill) gAppPtr->mBackgroundProcesses+=aSD;

	return aSD->mDSprite;
}

DynamicSpritePool::DynamicSpritePool() {if (gAppPtr) gAppPtr->mBackgroundProcesses+=this;}
DynamicSpritePool::~DynamicSpritePool() {if (gAppPtr) gAppPtr->mBackgroundProcesses-=this;}

struct RCommPacket
{
	//
	// Little packet struct to help us save Makermall thumbs into Cache...
	//
	Smart(DynamicSprite) mSprite;
	String mFN;
};

void DynamicSpritePool::Update()
{
	if (mSpriteList.GetCount())
	{
		if (mSpriteList[mCursor]->mSprite->mIsInitialized)
		{
			//
			// Okay... this line is intended to make it subtract as a timelapse... we step through the group one at a time, so
			// when we hit the end we act like SpriteCount updates have happened.  HOWEVER!  We can't do this if we're at max!
			//
			if (mSpriteList[mCursor]->mSprite->mLiveCount<mSpriteList[mCursor]->mSprite->mLiveCountMax) mSpriteList[mCursor]->mSprite->mLiveCount-=(mSpriteList.GetCount()-1);
			mSpriteList[mCursor]->mSprite->Update();
		}
		mCursor++;if (mCursor>=mSpriteList.GetCount()) mCursor-=mSpriteList.GetCount();
	}

	if (mPendingQueryList)
	{
		RComm::RCThreadLock();
		RComm::RQueryList& aList=*mPendingQueryList;
		EnumSmartList(RComm::RaptisoftQuery,aRQ,aList)
		{
			if (aRQ->IsDone())
			{
				RCommPacket* aPacket=(RCommPacket*)aRQ->mExtraData;
				if (!aRQ->IsRML())
				{
					aPacket->mFN+=".thumb";
					aRQ->mResult.CommitFile(aPacket->mFN);
					aPacket->mSprite->Load(aPacket->mFN);
				}
				delete aPacket;

				aList.Remove(aRQ);
				EnumSmartListRewind(RComm::RaptisoftQuery);
			}
		}
		if (aList.GetCount()==0) {delete mPendingQueryList;mPendingQueryList=NULL;}
		RComm::RCThreadUnlock();
	}
}

Smart(DynamicSprite) DynamicSpritePool::GetSprite(String theFN, String theVersion)
{
	String aOriginalFN=theFN;
	longlong aHash=theFN.GetHashI();
	EnumSmartList(DSInfo,aDS,mSpriteList) {if (aDS->mHash==aHash) if (aDS->mFN==theFN) {if (!aDS->mSprite->mIsInitialized) aDS->mSprite->mLiveCount=0;return aDS->mSprite;}}
	if (theFN.StartsWith("http://") || theFN.StartsWith("https://"))
	{
		String theNewFN=URLtoWebCache(theFN,theVersion);
		if (!DoesImageExist(theNewFN))
		{
			mSpriteList+=new DSInfo(theFN,DownloadDynamicSprite(theFN,theVersion));
			return mSpriteList.Last()->mSprite;
		}
		else theFN=theNewFN;
	}
	else if (theFN.StartsWith("mm://")) // Makermall preview (levelID)
	{
		String theNewFN=URLtoWebCache(theFN,theVersion);

		if (!DoesImageExist(theNewFN))
		{
			Smart(DynamicSprite) aD=new DynamicSprite;

			mSpriteList+=new DSInfo(theFN,aD);

			RCommPacket* aPacket=new RCommPacket;
			aPacket->mSprite=aD;
			aPacket->mFN=theNewFN;

			int aLevelID=theFN.GetSegmentAfter("mm://").ToInt();
			if (aLevelID!=0) 
			{
				RComm::RCThreadLock();
				if (!mPendingQueryList) mPendingQueryList=new RComm::RQueryList;
				mPendingQueryList->Add(RComm::LevelSharing_DownloadLevelPreview(aLevelID,aPacket));
				RComm::RCThreadUnlock();
			}

			return mSpriteList.Last()->mSprite;
		}
		else theFN=theNewFN;
	}

	#ifdef _DEBUG
	if (!DoesImageExist(theFN) && !DoesFileExist(theFN)) gOut.Out("!Sprite not there: %s",theFN.c());
	#endif

	Smart(DynamicSprite) aD=new DynamicSprite();
	mSpriteList+=new DSInfo(aOriginalFN,aD);
	aD->Load(theFN);
	return aD;
}

void LoadSpriteFolder(String theDir, Array<Sprite>& theArray)
{
	//
	// Hey, we should put them into a bundle really...
	//
	Array<String> aNList;EnumDirectory("images://NPCs/",aNList);theArray.GuaranteeSize(aNList.Size());
	for (int aCount=0;aCount<aNList.Size();aCount++) theArray[aCount].Load(aNList[aCount].RemoveTail('.'));
}
