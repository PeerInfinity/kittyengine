#include "rapt_image.h"
#include "rapt.h"
#include "rapt_iobuffer.h"

#include "graphics_core.h"

RGBA gNullPixel=RGBA(1,1,1,0);
unsigned int gNullColorBit=0;

bool DoesImageExist(String theFilename)
{
	//	theFilename.Lowercase();

	if (DoesFileExist(Sprintf("%s.cached",theFilename.c()))) return true;
	if (DoesFileExist(Sprintf("%s.png",theFilename.c()))) return true;
	if (DoesFileExist(Sprintf("%s.thumb",theFilename.c()))) return true;
	if (DoesFileExist(Sprintf("%s.gif",theFilename.c()))) return true;
	if (DoesFileExist(Sprintf("%s.jpg",theFilename.c()))) return true;
	if (DoesFileExist(Sprintf("%s.raw",theFilename.c()))) return true;
	if (DoesFileExist(Sprintf("%s.miximage",theFilename.c()))) return true;

	if (theFilename.Find("_alpha")!=String::mNotFound) return false;
	return DoesImageExist(Sprintf("%s_alpha",theFilename.c()));
}

bool DoesImageExist(String theFilename, String theVersion)
{
	bool aExists=false;
	String aCacheVersionName=theFilename;aCacheVersionName+=".";aCacheVersionName+=".cached.";aCacheVersionName+=theVersion;if (DoesFileExist(aCacheVersionName)) aExists=true;
	if (!aExists) {aCacheVersionName=theFilename;aCacheVersionName+=".png.";aCacheVersionName+=theVersion;if (DoesFileExist(aCacheVersionName)) aExists=true;}
	if (!aExists) {aCacheVersionName=theFilename;aCacheVersionName+=".thumb.";aCacheVersionName+=theVersion;if (DoesFileExist(aCacheVersionName)) aExists=true;}
	if (!aExists) {aCacheVersionName=theFilename;aCacheVersionName+=".png.";aCacheVersionName+=theVersion;if (DoesFileExist(aCacheVersionName)) aExists=true;}
	if (!aExists) {aCacheVersionName=theFilename;aCacheVersionName+=".gif.";aCacheVersionName+=theVersion;if (DoesFileExist(aCacheVersionName)) aExists=true;}
	if (!aExists) {aCacheVersionName=theFilename;aCacheVersionName+=".jpg.";aCacheVersionName+=theVersion;if (DoesFileExist(aCacheVersionName)) aExists=true;}
	if (!aExists) {aCacheVersionName=theFilename;aCacheVersionName+=".miximage.";aCacheVersionName+=theVersion;if (DoesFileExist(aCacheVersionName)) aExists=true;}

	if (aExists) return DoesImageExist(theFilename);

	if (theFilename.Find("_alpha")!=String::mNotFound) return false;
	return DoesImageExist(Sprintf("%s_alpha",theFilename.c()),theVersion);
}

/*
int gPendingImageLock=-1;
struct PendingImage
{
	String		mURL;
	String		mFN;
	String		mTail;
	Image*		mImage;
};
List gPendingImageList;
*/

Image::Image()
{
	mWidth=mHeight=0;
	mData=NULL;
}

Image::~Image()
{
	/*
	if (mPending)
	{
		if (gPendingImageLock==-1) gPendingImageLock=CreateThreadLock();
		ThreadLock(gPendingImageLock);
		EnumList(PendingImage,aPS,gPendingImageList) if (aPS->mImage==this) aPS->mImage=NULL;
		ThreadUnlock(gPendingImageLock);
	}
	*/
	Reset();
}

void Image::Reset()
{
	_DeleteArray(mData);
	mData=NULL;

	mWidth=0;
	mHeight=0;
}


void Image::MakeBlank(int theWidth, int theHeight)
{
	Reset();
	mWidth=theWidth;
	mHeight=theHeight;

	mData=new RGBA[mWidth*mHeight];
}


bool Image::Load(String theFilename)
{
	Reset();
	//
	// See what exists... PNG, GIF, etc...
	//
	String aLookFor;

	String aTail=theFilename.GetTail('.');
	if (aTail.Len() && DoesFileExist(theFilename))
	{

		aLookFor=theFilename;
		if (aTail=="thumb") LoadThumb(aLookFor.c());
		if (aTail=="png") Graphics_Core::LoadImage_PNG(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);
		if (aTail=="jpg") Graphics_Core::LoadImage_JPG(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);
		if (aTail=="gif") Graphics_Core::LoadImage_GIF(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);
		//if (aTail=="raw") Graphics_Core::LoadImage_RAW(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);
		theFilename=theFilename.RemoveTail('.');
	}
	else
	{
		aLookFor=Sprintf("%s.miximage",theFilename.c());if (DoesFileExist(aLookFor.c())) 
		{
			//
			// Mixed image... unpack it!
			//
			IOBuffer aBuffer;
			aBuffer.Load(aLookFor);

			IOBuffer aAlpha,aRGB;
			String aAlphaTail=aBuffer.ReadString();
			aBuffer.ReadBuffer(aAlpha);
			String aRGBTail=aBuffer.ReadString();
			aBuffer.ReadBuffer(aRGB);

			String aAlphaFN=Sprintf("%s_alpha.%s",theFilename.c(),aAlphaTail.c());
			String aRGBFN=Sprintf("%s.%s",theFilename.c(),aRGBTail.c());
			aAlpha.CommitFile(aAlphaFN);
			aRGB.CommitFile(aRGBFN);
			RemoveFile(aLookFor); // Remove original, we don't need it any more...
		}

		//aLookFor=Sprintf("%s.png",theFilename.c());if (DoesFileExist(aLookFor.c())) Graphics_Core::LoadImage_PNG(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);

	//gOut.Out("Load: %s",theFilename.c());

		aLookFor=Sprintf("%s.thumb",theFilename.c());if (DoesFileExist(aLookFor.c())) LoadThumb(aLookFor.c());
		if (!mData) {aLookFor=Sprintf("%s.png",theFilename.c());if (DoesFileExist(aLookFor.c())) Graphics_Core::LoadImage_PNG(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);}
		if (!mData) {aLookFor=Sprintf("%s.jpg",theFilename.c());if (DoesFileExist(aLookFor.c())) Graphics_Core::LoadImage_JPG(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);}
		if (!mData) {aLookFor=Sprintf("%s.gif",theFilename.c());if (DoesFileExist(aLookFor.c())) Graphics_Core::LoadImage_GIF(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);}
		//if (!mData) {aLookFor=Sprintf("%s.raw",theFilename.c());if (DoesFileExist(aLookFor.c())) Graphics_Core::LoadImage_RAW(aLookFor.c(),(unsigned char**)&mData,&mWidth,&mHeight);}
	}

	//
	// Now see if an alpha exists... PNG, GIF, etc...
	// If alpha exists, apply it to the alpha of this image.
	//

	RGBA *aAlphaBits=NULL;
	int aAlphaWidth=0;
	int aAlphaHeight=0;
	aLookFor=Sprintf("%s_alpha.png",theFilename.c());if (DoesFileExist(aLookFor.c())) Graphics_Core::LoadImage_PNG(aLookFor.c(),(unsigned char**)&aAlphaBits,&aAlphaWidth,&aAlphaHeight);
	if (!aAlphaBits) {aLookFor=Sprintf("%s_alpha.jpg",theFilename.c());if (DoesFileExist(aLookFor.c())) Graphics_Core::LoadImage_JPG(aLookFor.c(),(unsigned char**)&aAlphaBits,&aAlphaWidth,&aAlphaHeight);}
	if (!aAlphaBits) {aLookFor=Sprintf("%s_alpha.gif",theFilename.c());if (DoesFileExist(aLookFor.c())) Graphics_Core::LoadImage_GIF(aLookFor.c(),(unsigned char**)&aAlphaBits,&aAlphaWidth,&aAlphaHeight);}

	if (aAlphaBits)
	{
		if (!mData) {MakeBlank(aAlphaWidth,aAlphaHeight);memset(mData,255,aAlphaWidth*aAlphaHeight*4);}
		else if (aAlphaWidth!=mWidth || aAlphaHeight!=mHeight) {_DeleteArray(aAlphaBits);aAlphaBits=NULL;}

		if (aAlphaBits)
		{
			RGBA *aAlphaPtr=(RGBA*)aAlphaBits;
			RGBA *aDataPtr=(RGBA*)GetBits();
			for (int aCount=0;aCount<aAlphaWidth*aAlphaHeight;aCount++)
			{
				aDataPtr->mA=aAlphaPtr->mR;
				aAlphaPtr++;
				aDataPtr++;
			}

/*			
			unsigned char* aAlphaPtr=(unsigned char*)aAlphaBits;
			unsigned char* aDataPtr=(unsigned char*)GetBits();
			for (int aCount=0;aCount<aAlphaWidth*aAlphaHeight;aCount++)
			{
				*(aDataPtr)=*(aAlphaPtr+1);
				aAlphaPtr+=4;
				aDataPtr+=4;
			}
*/ 
		}
	}

	_DeleteArray(aAlphaBits);
	if (mData) return true;
	return false;
}

void Image::ExportPNG(String theFilename)
{
	if (!theFilename.ContainsI(".png")) theFilename+=".png";
	Graphics_Core::SaveImage_PNG(theFilename,(unsigned char*)mData,mWidth,mHeight);
}

void Image::ExportRAW(String theFilename)
{
	theFilename.RemoveTail('.');
	theFilename+=Sprintf("_%d_%d.raw",mWidth,mHeight);
	IOBuffer aBuffer;
	aBuffer.WriteRaw(mData,mWidth*mHeight*4);
	aBuffer.CommitFile(theFilename);

	//
	// To import into photoshop:
	// Use dimensions
	// 4 channels
	// Check interleaved
	// Check last channel is transparency
	//

	//Graphics_Core::SaveImage_RAW(theFilename,(unsigned char*)mData,mWidth,mHeight);
}

void Image::CopyFrom(Image &theSource)
{
	Reset();
	mWidth=theSource.mWidth;
	mHeight=theSource.mHeight;
	mData=new RGBA[theSource.mWidth*theSource.mHeight];
	memcpy(GetBits(),theSource.GetBits(),sizeof(RGBA)*theSource.mWidth*theSource.mHeight);
}

void Image::CopyTo(Image &theDestination)
{
	theDestination.Reset();
	theDestination.mWidth=mWidth;
	theDestination.mHeight=mHeight;
	theDestination.mData=new RGBA[mWidth*mHeight];
	memcpy(theDestination.GetBits(),GetBits(),sizeof(RGBA)*mWidth*mHeight);
}

void Image::CopyFrom(Image &theSource, int theX, int theY, int theWidth, int theHeight)
{
	AABBRect aWorkRect=AABBRect((float)theX,(float)theY,(float)(theX+theWidth-1),(float)(theY+theHeight-1));
	AABBRect aCrop=theSource.GetAABB().GetIntersection(aWorkRect);
	aCrop=aCrop.GetIntersection(aCrop);
	theX=(int)aCrop.mX1;
	theY=(int)aCrop.mY1;
	theWidth=(int)aCrop.Width();
	theHeight=(int)aCrop.Height();

	Image &aDest=*this;
	Image &aSrc=theSource;

	aDest.Reset();
	aDest.mWidth=theWidth;
	aDest.mHeight=theHeight;
	aDest.mData=new RGBA[mWidth*mHeight];

	RGBA *aSourcePtr=aSrc.GetPixelPtr(theX,theY);
	RGBA *aDestPtr=aDest.mData;

	for (int aRow=0;aRow<theHeight;aRow++)
	{
		memcpy(aDestPtr,aSourcePtr,theWidth*sizeof(RGBA));
		aSourcePtr+=aSrc.mWidth;
		aDestPtr+=aDest.mWidth;
	}
}


void Image::CopyTo(Image &theDestination, int theX, int theY, int theWidth, int theHeight)
{
	AABBRect aWorkRect=AABBRect((float)theX,(float)theY,(float)(theX+theWidth-1),(float)(theY+theHeight-1));
	AABBRect aCrop=GetAABB().GetIntersection(aWorkRect);
	theX=(int)aCrop.mX1;
	theY=(int)aCrop.mY1;
	theWidth=(int)aCrop.Width();
	theHeight=(int)aCrop.Height();

	Image &aDest=theDestination;
	Image &aSrc=*this;

	aDest.Reset();
	aDest.mWidth=theWidth;
	aDest.mHeight=theHeight;
	aDest.mData=new RGBA[mWidth*mHeight];

	RGBA *aSourcePtr=aSrc.GetPixelPtr(theX,theY);
	RGBA *aDestPtr=aDest.mData;

	for (int aRow=0;aRow<theHeight;aRow++)
	{
		memcpy(aDestPtr,aSourcePtr,theWidth*sizeof(RGBA));
		aSourcePtr+=aSrc.mWidth;
		aDestPtr+=aDest.mWidth;
	}
}


void Image::Clear(float theR, float theG, float theB, float theA)
{
	if (theR==theG && theG==theB && theA==theB)
	{
		int aValue=(int)(theR*255.0f);
		memset(GetBits(),aValue,mWidth*mHeight*sizeof(RGBA));
	}
	else
	{
		RGBA *aPtr=GetBits();
		int aR=(int)(theR*255.0f);
		int aG=(int)(theG*255.0f);
		int aB=(int)(theB*255.0f);
		int aA=(int)(theA*255.0f);
		for (int aCount=0;aCount<mWidth*mHeight;aCount++)
		{
			aPtr->mR=aB;
			aPtr->mG=aG;
			aPtr->mB=aR;
			aPtr->mA=aA;
			aPtr++;
		}
	}
}

void Image::StampImage(Image &theImage, int theXPos, int theYPos, Rect theRect, Color theColor)
{
	if (theXPos+theImage.mWidth<0) return;
	if (theYPos+theImage.mHeight<0) return;
	if (theXPos>=mWidth) return;
	if (theYPos>=mHeight) return;

	bool aClipColor=false;
	if (theColor.mR!=1.0f || theColor.mG!=1.0f || theColor.mB!=1.0f || theColor.mA!=1.0f) aClipColor=true;

	int aYPos=theYPos;
	for (int aSpanY=(int)theRect.mY;aSpanY<(int)theRect.mHeight+(int)theRect.mY;aSpanY++)
	{
		if (aYPos>=0 && aYPos<mHeight && aSpanY>=0 && aSpanY<theImage.mHeight)
		{

			int aXPos=theXPos;
			for (int aSpanX=(int)theRect.mX;aSpanX<(int)theRect.mX+(int)theRect.mWidth;aSpanX++)
			{
				if (aXPos>=mWidth) break;
				if (aSpanX>=theImage.mWidth) break;
				if (aXPos>=0 && aSpanX>=0)
				{

					//
					// Plot away!
					//
					RGBA *aDest=GetPixelPtr(aXPos,aYPos);
					RGBA *aSrc=theImage.GetPixelPtr(aSpanX,aSpanY);

					if (aDest && aSrc) if (aSrc->mA>2)
					{
						if (!aClipColor)
						{
							if (aSrc->mA>=255 || aDest->mA==0)
							{
								aDest->mR=aSrc->mR;
								aDest->mG=aSrc->mG;
								aDest->mB=aSrc->mB;
								aDest->mA=aSrc->mA;
							}
							else
							{
								float aSrcAlpha=(float)aSrc->mA/255.0f;
								float aDestAlpha=1.0f-((float)aSrc->mA/255.0f);

								float aSrcRed=(float)aSrc->mR*aSrcAlpha;
								float aSrcGreen=(float)aSrc->mG*aSrcAlpha;
								float aSrcBlue=(float)aSrc->mB*aSrcAlpha;

								float aDestRed=(float)aDest->mR*aDestAlpha;
								float aDestGreen=(float)aDest->mG*aDestAlpha;
								float aDestBlue=(float)aDest->mB*aDestAlpha;

								int aResultRed=(int)_min(255,(aSrcRed+.5f)+(aDestRed+.5f));
								int aResultGreen=(int)_min(255,(aSrcGreen+.5f)+(aDestGreen+.5f));
								int aResultBlue=(int)_min(255,(aSrcBlue+.5f)+(aDestBlue+.5f));

								aDest->mR=aResultRed;
								aDest->mG=aResultGreen;
								aDest->mB=aResultBlue;
								aDest->mA=_max(aDest->mA,aSrc->mA);
							}
						}
						else
						{
							if (aSrc->mA*theColor.mA>=250 || aDest->mA==0)
							{
								aDest->mR=(int)(aSrc->mR*theColor.mR);
								aDest->mG=(int)(aSrc->mG*theColor.mG);
								aDest->mB=(int)(aSrc->mB*theColor.mB);
								aDest->mA=(int)(aSrc->mA*theColor.mA);
							}
							else
							{

								float aSrcAlpha=(float)aSrc->mA/255.0f;
								aSrcAlpha*=theColor.mA;
								float aDestAlpha=1.0f-((float)aSrcAlpha);

								float aSrcRed=(float)aSrc->mR*theColor.mR*aSrcAlpha;
								float aSrcGreen=(float)aSrc->mG*theColor.mG*aSrcAlpha;
								float aSrcBlue=(float)aSrc->mB*theColor.mB*aSrcAlpha;

								float aDestRed=(float)aDest->mR*aDestAlpha;
								float aDestGreen=(float)aDest->mG*aDestAlpha;
								float aDestBlue=(float)aDest->mB*aDestAlpha;

								int aResultRed=(int)_min(255,(aSrcRed+.5f)+(aDestRed+.5f));
								int aResultGreen=(int)_min(255,(aSrcGreen+.5f)+(aDestGreen+.5f));
								int aResultBlue=(int)_min(255,(aSrcBlue+.5f)+(aDestBlue+.5f));

								aDest->mR=aResultRed;
								aDest->mG=aResultGreen;
								aDest->mB=aResultBlue;
								aDest->mA=_max(aDest->mA,aSrc->mA);
							}
						}
					}
				}
				aXPos++;
			}
		}
		aYPos++;
		if (aYPos>=mHeight) break;
		if (aSpanY>=theImage.mHeight) break;
	}
}

Rect Image::GetBounds()
{
	int aLeft=mWidth;
	int aTop=mHeight;
	int aBottom=0;
	int aRight=0;

	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA *aPix=GetPixelPtr(aSpanX,aSpanY);
			if (aPix->mA)
			{
				aLeft=_min(aLeft,aSpanX);
				aRight=_max(aRight,aSpanX);
				aTop=_min(aTop,aSpanY);
				aBottom=_max(aBottom,aSpanY);
			}
		}
	}

	if (aRight<aLeft) {aLeft=0;aRight=0;}
	if (aBottom<aTop) {aTop=0;aBottom=0;}

	return Rect((float)aLeft,(float)aTop,(float)(aRight-aLeft+1),(float)(aBottom-aTop+1));
}

Rect Image::GetBoundsFromColor(Color theBackgroundColor)
{
	int aLeft=mWidth;
	int aTop=mHeight;
	int aBottom=0;
	int aRight=0;

	int aRBkg=(int)(theBackgroundColor.mR*255);
	int aGBkg=(int)(theBackgroundColor.mG*255);
	int aBBkg=(int)(theBackgroundColor.mB*255);

	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA *aPix=GetPixelPtr(aSpanX,aSpanY);
			if (aPix->mR!=aRBkg || aPix->mG!=aGBkg || aPix->mB!=aBBkg)
			{
				aLeft=_min(aLeft,aSpanX);
				aRight=_max(aRight,aSpanX);
				aTop=_min(aTop,aSpanY);
				aBottom=_max(aBottom,aSpanY);
			}
		}
	}

	if (aRight<aLeft) {aLeft=0;aRight=0;}
	if (aBottom<aTop) {aTop=0;aBottom=0;}

	return Rect((float)aLeft,(float)aTop,(float)(aRight-aLeft+1),(float)(aBottom-aTop+1));
}

void Image::EraseColor(Color theColor)
{
	int aRBkg=(int)(theColor.mR*255);
	int aGBkg=(int)(theColor.mG*255);
	int aBBkg=(int)(theColor.mB*255);

	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA *aPix=GetPixelPtr(aSpanX,aSpanY);
			if (aPix->mR==aRBkg && aPix->mG==aGBkg && aPix->mB==aBBkg) aPix->mA=0;
		}
	}
}



void Image::Crop(int theX, int theY, int theWidth, int theHeight)
{
	AABBRect aWorkRect=AABBRect((float)theX,(float)theY,(float)theX+theWidth-1,(float)theY+theHeight-1);
	AABBRect aCrop=GetAABB().GetIntersection(aWorkRect);

	Image aTemp;
	CopyTo(aTemp,(int)aCrop.mX1,(int)aCrop.mY1,(int)aCrop.Width(),(int)aCrop.Height());
	CopyFrom(aTemp);
}

void Image::AddBorder(int theSize)
{
	Image aOImage;
	aOImage.CopyFrom(*this);

	MakeBlank(mWidth+(theSize*2),mHeight+(theSize*2));
	Clear(0,0,0,0);

	StampImage(aOImage,theSize,theSize);
}

void Image::MakeTilingBorder()
{
	AddBorder(1);

	//
	// Now go round the border edges and copy pixels.
	// Top edge first!
	//
	RGBA *aPlotPtr;
	RGBA *aReadPtr;

	aPlotPtr=GetPixelPtr(1,0);
	aReadPtr=GetPixelPtr(1,1);
	for (int aCount=0;aCount<mWidth-2;aCount++)
	{
		aPlotPtr->mR=aReadPtr->mR;
		aPlotPtr->mG=aReadPtr->mG;
		aPlotPtr->mB=aReadPtr->mB;
		aPlotPtr->mA=aReadPtr->mA;

		aPlotPtr++;
		aReadPtr++;
	}

	//
	// Bottom edge
	//
	aPlotPtr=GetPixelPtr(1,mHeight-1);
	aReadPtr=GetPixelPtr(1,mHeight-2);
	for (int aCount=0;aCount<mWidth-2;aCount++)
	{
		aPlotPtr->mR=aReadPtr->mR;
		aPlotPtr->mG=aReadPtr->mG;
		aPlotPtr->mB=aReadPtr->mB;
		aPlotPtr->mA=aReadPtr->mA;

		aPlotPtr++;
		aReadPtr++;
	}

	//
	// Left edge
	//
	aPlotPtr=GetPixelPtr(0,1);
	aReadPtr=GetPixelPtr(1,1);
	for (int aCount=0;aCount<mHeight-2;aCount++)
	{
		aPlotPtr->mR=aReadPtr->mR;
		aPlotPtr->mG=aReadPtr->mG;
		aPlotPtr->mB=aReadPtr->mB;
		aPlotPtr->mA=aReadPtr->mA;

		aPlotPtr+=mWidth;
		aReadPtr+=mWidth;
	}

	//
	// Right edge
	//
	aPlotPtr=GetPixelPtr(mWidth-1,1);
	aReadPtr=GetPixelPtr(mWidth-2,1);
	for (int aCount=0;aCount<mHeight-2;aCount++)
	{
		aPlotPtr->mR=aReadPtr->mR;
		aPlotPtr->mG=aReadPtr->mG;
		aPlotPtr->mB=aReadPtr->mB;
		aPlotPtr->mA=aReadPtr->mA;

		aPlotPtr+=mWidth;
		aReadPtr+=mWidth;
	}

	aPlotPtr=GetPixelPtr(0,0);
	aReadPtr=GetPixelPtr(1,1);
	aPlotPtr->mR=aReadPtr->mR;
	aPlotPtr->mG=aReadPtr->mG;
	aPlotPtr->mB=aReadPtr->mB;
	aPlotPtr->mA=aReadPtr->mA;

	aPlotPtr=GetPixelPtr(mWidth-1,0);
	aReadPtr=GetPixelPtr(mWidth-2,1);
	aPlotPtr->mR=aReadPtr->mR;
	aPlotPtr->mG=aReadPtr->mG;
	aPlotPtr->mB=aReadPtr->mB;
	aPlotPtr->mA=aReadPtr->mA;

	aPlotPtr=GetPixelPtr(0,mHeight-1);
	aReadPtr=GetPixelPtr(1,mHeight-2);
	aPlotPtr->mR=aReadPtr->mR;
	aPlotPtr->mG=aReadPtr->mG;
	aPlotPtr->mB=aReadPtr->mB;
	aPlotPtr->mA=aReadPtr->mA;

	aPlotPtr=GetPixelPtr(mWidth-1,mHeight-1);
	aReadPtr=GetPixelPtr(mWidth-2,mHeight-2);
	aPlotPtr->mR=aReadPtr->mR;
	aPlotPtr->mG=aReadPtr->mG;
	aPlotPtr->mB=aReadPtr->mB;
	aPlotPtr->mA=aReadPtr->mA;
}

void Image::Resize(int theNewWidth, int theNewHeight, Image *theImage)
{
	//
	// First, create a new buffer for us...
	//
	RGBA *aNewData=new RGBA[theNewWidth*theNewHeight];

	//memset(aNewData,255,(theNewWidth*theNewHeight*4));
	int aNewWidth=theNewWidth;
	int aNewHeight=theNewHeight;

	//
	// Resize into it....
	//
//*
	struct Rect
	{
		float mX;
		float mY;
		float mWidth;
		float mHeight;
	} 
	aSrcRect,aDestRect;

	aSrcRect.mX=0;
	aSrcRect.mY=0;
	aSrcRect.mWidth=(float)mWidth;
	aSrcRect.mHeight=(float)mHeight;

	aDestRect.mX=0;
	aDestRect.mY=0;
	aDestRect.mWidth=(float)aNewWidth;
	aDestRect.mHeight=(float)aNewHeight;

	unsigned int* aSrcBits=(unsigned int*)mData;
	unsigned int* aDestBits=(unsigned int*)aNewData;

	#define SRC_TYPE unsigned int
	#define READ_COLOR(ptr) (*(ptr))

	{	
		int aSrcRowWidth = mWidth;
		int aSrcXI = (int) floor(aSrcRect.mX);
		int aSrcYI = (int) floor(aSrcRect.mY);
		int aSrcWidthI = (int) ceil(aSrcRect.mWidth + (aSrcRect.mX - aSrcXI));
		int aSrcHeightI = (int) ceil(aSrcRect.mHeight + (aSrcRect.mY - aSrcYI));

		if (aSrcXI<0) aSrcXI = 0;
		if (aSrcYI<0) aSrcYI = 0;
		if (aSrcXI + aSrcWidthI > mWidth) aSrcWidthI = mWidth - aSrcXI;
		if (aSrcYI + aSrcHeightI > mHeight) aSrcHeightI = mHeight - aSrcYI;
		if (aSrcWidthI<=0 || aSrcHeightI<=0) return;


		int aTempDestWidth =(int)(aDestRect.mWidth+4);
		int aTempDestHeight = (int)(aDestRect.mHeight+4);

		// For holding horizontally resized pixels not vertically (yet)

		unsigned int* aNewHorzPixels = new unsigned int[aTempDestWidth*aSrcHeightI*4];

		memset(aNewHorzPixels,0,aTempDestWidth*aSrcHeightI*4*4);

		if (aSrcRect.mWidth >= aDestRect.mWidth)
		{

			double aDestXFactor = aDestRect.mWidth / aSrcRect.mWidth;
			double aDestXOffset = 1.0 + (aSrcXI - aSrcRect.mX) * aDestXFactor;


			// Shrinking			


			for (int aSrcX = 0; aSrcX < aSrcWidthI; aSrcX++)			
			{
				double aDestX1 = aDestXFactor * aSrcX + aDestXOffset;
				double aDestX2 = aDestX1 + aDestXFactor;
				
				int aDestXI1 = (int) aDestX1;
				int aDestXI2 = (int) aDestX2;

				SRC_TYPE* s1 = &aSrcBits[aSrcYI*aSrcRowWidth + aSrcXI+aSrcX];

				if (aDestXI1 == aDestXI2)
				{

					unsigned int* d = &aNewHorzPixels[aDestXI1*4];					
					int aFactor = (int) (257 * aDestXFactor);
					
					for (int aSrcY = 0; aSrcY < aSrcHeightI; aSrcY++)
					{
						unsigned int pixel = READ_COLOR(s1);
						
						*d++ += aFactor * ((pixel      ) & 0xFF);
						*d++ += aFactor * ((pixel >>  8) & 0xFF);
						*d++ += aFactor * ((pixel >> 16) & 0xFF);
						*d++ += aFactor * ((pixel >> 24) & 0xFF);

						d += aTempDestWidth*4 - 4;
						s1 += aSrcRowWidth;
					}					
				}
				else
				{		
					int aFactor1 = (int) (257 * (aDestXI2 - aDestX1));
					int aFactor2 = (int) (257 * (aDestX2 - aDestXI2));
					
					unsigned int* d = &aNewHorzPixels[aDestXI1*4];					
					
					for (int aSrcY = 0; aSrcY < aSrcHeightI; aSrcY++)
					{
						unsigned int pixel = READ_COLOR(s1);

				
						*d++ += aFactor1 * ((pixel      ) & 0xFF);
						*d++ += aFactor1 * ((pixel >>  8) & 0xFF);
						*d++ += aFactor1 * ((pixel >> 16) & 0xFF);
						*d++ += aFactor1 * ((pixel >> 24) & 0xFF);
						
						*d++ += aFactor2 * ((pixel      ) & 0xFF);
						*d++ += aFactor2 * ((pixel >>  8) & 0xFF);
						*d++ += aFactor2 * ((pixel >> 16) & 0xFF);
						*d++ += aFactor2 * ((pixel >> 24) & 0xFF);


						d += aTempDestWidth*4 - 8;
						s1 += aSrcRowWidth;
					}					
				}
			}

		}
		else
		{

			double aSrcXFactor = (aSrcRect.mWidth - 1) / (aDestRect.mWidth - 1);

			for (int aDestX = 1; aDestX < aTempDestWidth-1; aDestX++)
			{
				unsigned int* d = &aNewHorzPixels[aDestX*4];

				double aSrcX = (aDestX - 1)*aSrcXFactor + aSrcRect.mX;
				int aSrcXI = (int) aSrcX;

				int aFactor1 = (int) (257 * (1.0 - (aSrcX - aSrcXI)));
				int aFactor2 = (int) (257 - aFactor1);
				
				SRC_TYPE* s = &aSrcBits[aSrcYI*aSrcRowWidth+aSrcXI];				

				for (int aDestY = 0; aDestY < aSrcHeightI; aDestY++)
				{
					unsigned int pixel1 = READ_COLOR(s++);
					unsigned int pixel2 = READ_COLOR(s);
					
					*d++ = (aFactor1 * ((pixel1      ) & 0xFF)) + (aFactor2 * ((pixel2      ) & 0xFF));
					*d++ = (aFactor1 * ((pixel1 >>  8) & 0xFF)) + (aFactor2 * ((pixel2 >>  8) & 0xFF));
					*d++ = (aFactor1 * ((pixel1 >> 16) & 0xFF)) + (aFactor2 * ((pixel2 >> 16) & 0xFF));
					*d++ = (aFactor1 * ((pixel1 >> 24) & 0xFF)) + (aFactor2 * ((pixel2 >> 24) & 0xFF));


					d += aTempDestWidth*4 - 4;
					s += aSrcRowWidth - 1;
				}				
			}
		}

		unsigned int* aNewPixels = new unsigned int[aTempDestWidth*aTempDestHeight*4];


		memset(aNewPixels,0,aTempDestWidth*aTempDestHeight*4*4);

		// Now resize vertically

		if (aSrcRect.mHeight >= aDestRect.mHeight)
		{			
			double aDestYFactor = aDestRect.mHeight / aSrcRect.mHeight;
			
			double aDestYOffset = 1.0 + (aSrcYI - aSrcRect.mY) * aDestYFactor;

			for (int aSrcY = 0; aSrcY < aSrcHeightI; aSrcY++)
			{
				double aDestY1 = aDestYFactor * aSrcY + aDestYOffset;
				double aDestY2 = aDestY1 + aDestYFactor;
				
				int aDestYI1 = (int) floor(aDestY1);
				int aDestYI2 = (int) floor(aDestY2);

				unsigned int* s = &aNewHorzPixels[aSrcY*aTempDestWidth*4];

				if (aDestYI1 == aDestYI2)
				{
					unsigned int* d = &aNewPixels[aDestYI1*aTempDestWidth*4];
					int aFactor = (int) (256 * aDestYFactor);
					
					for (int aSrcX = 0; aSrcX < aTempDestWidth; aSrcX++)
					{	
						*d++ += aFactor * *s++;
						*d++ += aFactor * *s++;
						*d++ += aFactor * *s++;
						*d++ += aFactor * *s++;	
					}
				}
				else
				{
					int aFactor1 = (int) (256 * (aDestYI2 - aDestY1));
					int aFactor2 = (int) (256 * (aDestY2 - aDestYI2));					
					
					unsigned int* d1 = &aNewPixels[aDestYI1*aTempDestWidth*4];
					unsigned int* d2 = &aNewPixels[aDestYI2*aTempDestWidth*4];					
					
					for (int aSrcX = 0; aSrcX < aTempDestWidth; aSrcX++)
					{
						*d1++ += aFactor1 * *s;
						*d2++ += aFactor2 * *s++;

						*d1++ += aFactor1 * *s;
						*d2++ += aFactor2 * *s++;

						*d1++ += aFactor1 * *s;
						*d2++ += aFactor2 * *s++;

						*d1++ += aFactor1 * *s;
						*d2++ += aFactor2 * *s++;
					}

				}
			}

		}
		else
		{

			double aSrcYFactor = (aSrcRect.mHeight - 1) / (aDestRect.mHeight - 1);

			for (int aDestY = 1; aDestY < aDestRect.mHeight + 1; aDestY++)			
			{
				unsigned int* d = &aNewPixels[(aDestY*aTempDestWidth+1)*4];

				double aSrcY = (aDestY-1)*aSrcYFactor + (aSrcRect.mY - ((int) aSrcRect.mY));
				int aSrcYI = (int) aSrcY;

				int aFactor1 = (int) (256 * (1.0 - (aSrcY - aSrcYI)));
				int aFactor2 = 256 - aFactor1;
				
				unsigned int* s1 = &aNewHorzPixels[(aSrcYI*aTempDestWidth+1)*4];
				unsigned int* s2;
				
				if (aSrcYI == aSrcHeightI - 1)
					s2 = s1;
				else
					s2 = &aNewHorzPixels[((aSrcYI+1)*aTempDestWidth+1)*4];

				for (int aDestX = 1; aDestX < aTempDestWidth-1; aDestX++)	
				{
					*d++ = (aFactor1 * *s1++) + (aFactor2 * *s2++);
					*d++ = (aFactor1 * *s1++) + (aFactor2 * *s2++);
					*d++ = (aFactor1 * *s1++) + (aFactor2 * *s2++);
					*d++ = (aFactor1 * *s1++) + (aFactor2 * *s2++);					
				}				
			}
		}
		
		for (int y = 0; y < aDestRect.mHeight; y++)
		{
			unsigned int* aDestPixels = &aDestBits[((int)aDestRect.mY+y)*(int)aDestRect.mWidth+(int)aDestRect.mX];

			for (int x = 0; x < aDestRect.mWidth; x++)				
			{
				unsigned int *p = &aNewPixels[((y+1)*aTempDestWidth+x+1)*4];
				
				int b = (*p++) >> 16;
				int g = (*p++) >> 16;
				int r = (*p++) >> 16;
				int a = (*p++) >> 16;

				*(aDestPixels++)= (b) | (g << 8) | (r << 16) | (a << 24);
			}
		}


		delete [] aNewPixels;
		delete [] aNewHorzPixels;
	}
/**/

	//
	// Now naturalize to theImage.  If theImage==NULL, then
	// we free up this image and stick it in there.
	//

	if (!theImage) 
	{
		theImage=this;
		_DeleteArray(mData);
		mData=NULL;
	}
	else theImage->Reset();

	theImage->mWidth=aNewWidth;
	theImage->mHeight=aNewHeight;
	theImage->mData=aNewData;

}


void Image::SetNullPixelColor(Color theColor)
{
	gNullPixel.mR=(unsigned char)(theColor.mR*255);
	gNullPixel.mG=(unsigned char)(theColor.mG*255);
	gNullPixel.mB=(unsigned char)(theColor.mB*255);
	gNullPixel.mA=(unsigned char)(theColor.mA*255);
}

RGBA* Image::GetPixelPtr(int x, int y)
{
	if (x<0 || y<0) return &gNullPixel;
	if (x>=mWidth || y>=mHeight) return &gNullPixel;
	return &mData[(y*mWidth)+x];
}

unsigned char& Image::GetR(int x, int y) {if (x<0 || y<0) return gNullPixel.mR;if (x>=mWidth || y>=mHeight) return gNullPixel.mR;return mData[(y*mWidth)+x].mR;}
unsigned char& Image::GetG(int x, int y) {if (x<0 || y<0) return gNullPixel.mG;if (x>=mWidth || y>=mHeight) return gNullPixel.mG;return mData[(y*mWidth)+x].mG;}
unsigned char& Image::GetB(int x, int y) {if (x<0 || y<0) return gNullPixel.mB;if (x>=mWidth || y>=mHeight) return gNullPixel.mB;return mData[(y*mWidth)+x].mB;}
unsigned char& Image::GetA(int x, int y) {if (x<0 || y<0) return gNullPixel.mA;if (x>=mWidth || y>=mHeight) return gNullPixel.mA;return mData[(y*mWidth)+x].mA;}


void Image::ImageToAlpha(Image& theImage, bool clearZeroAlpha)
{
	if (theImage.mWidth!=mWidth || theImage.mHeight!=mHeight) return;

	RGBA *aAlphaPtr=theImage.GetBits();
	RGBA *aRGBPtr=GetBits();
	if (!clearZeroAlpha)
	{
		for (int aCount=0;aCount<mWidth*mHeight;aCount++)
		{
			aRGBPtr->mA=aAlphaPtr->mR;
			aAlphaPtr++;
			aRGBPtr++;
		}
	}
	else
	{
		for (int aCount=0;aCount<mWidth*mHeight;aCount++)
		{
			aRGBPtr->mA=aAlphaPtr->mR;
			if (aRGBPtr->mA==0) aRGBPtr->mR=aRGBPtr->mG=aRGBPtr->mB=0;
			aAlphaPtr++;
			aRGBPtr++;
		}
	}
}

void Image::ImageToAlpha(Image& theImage, Rect theRect, bool clearZeroAlpha)
{
	if (theImage.mWidth!=mWidth || theImage.mHeight!=mHeight) return;


	if (!clearZeroAlpha)
	{
		for (int aSpanY=(int)theRect.mY;aSpanY<(int)theRect.mY+theRect.mHeight;aSpanY++)
		{
			int aSpanX=(int)theRect.mX;
			RGBA *aAlphaPtr=theImage.GetPixelPtr(aSpanX,aSpanY);
			RGBA *aRGBPtr=GetPixelPtr(aSpanX,aSpanY);

			for (;aSpanX<(int)theRect.mX+theRect.mWidth;aSpanX++)
			{
				aRGBPtr->mA=aAlphaPtr->mR;
				aAlphaPtr++;
				aRGBPtr++;
			}
		}
	}
/*
	if (!clearZeroAlpha)
	{
		for (int aCount=0;aCount<mWidth*mHeight;aCount++)
		{
			aRGBPtr->mA=aAlphaPtr->mR;
			aAlphaPtr++;
			aRGBPtr++;
		}
	}
	else
	{
		for (int aCount=0;aCount<mWidth*mHeight;aCount++)
		{
			aRGBPtr->mA=aAlphaPtr->mR;
			if (aRGBPtr->mA==0) aRGBPtr->mR=aRGBPtr->mG=aRGBPtr->mB=0;
			aAlphaPtr++;
			aRGBPtr++;
		}
	}
*/
}

//
// Lancoz Resizing
//
#ifdef _WIN32
#pragma warning (push,1)
#endif

#define CACHE_PRECISION    (100.0) 
#define FILTER_SIZE        (1.0)

static double CalcLancozKernel(unsigned int filter_size, double x)
{
	int filterneg = filter_size*-1;
	if(x >= filter_size || x <= filterneg)
		return 0;
	if(x == 0)
		return 1;

	double xpi = x * gMath.mPI;
	return filter_size * sin(xpi) * sin(xpi / filter_size) / (xpi * xpi);
}

static void CreateLancozCache(unsigned int cachePrecision, unsigned int filterSize,Array<double>& cache)
{
	unsigned int max = filterSize * filterSize * cachePrecision;
	double iPrecision = 1.0f / cachePrecision;
	double value;

	cache.SetSize(max+128);
	for(unsigned int cacheKey=0; cacheKey < max; cacheKey++)
	{
		value = CalcLancozKernel(filterSize, sqrt(cacheKey * iPrecision));
		cache[cacheKey] = value < 0 ? 0 : value;
	}
}

void Image::Resize_Lancoz(int theNewWidth, int theNewHeight, Image *theImage)
{
	RGBA *aNewData=new RGBA[theNewWidth*theNewHeight];

	RGBA color;

	double total=0.0f; 
	double distanceY=0.0f;
	double value=0.0f;

	unsigned char a=0;
	unsigned char r=0;
	unsigned char g=0;
	unsigned char b=0;
	unsigned int i=0;
	unsigned int cacheKey=0;

	int x=0;
	unsigned int x1=0;
	int x1b=0;
	int x1e=0;

	int y=0;
	unsigned int y1=0;
	int y1b=0;
	int y1e=0;

	unsigned int y2=0;
	unsigned int y3=0;

	double y1et=0.0f;
	double x1et=0.0f;

	Array<double> values;

	double sx = theNewWidth/(double)mWidth;
	double sy = theNewHeight/(double)mHeight;
	unsigned int sw1 = mWidth - 1;
	unsigned int sh1 = mHeight - 1;
	double isx = 1/sx;
	double isy = 1/sy;
	double cw = 1/(double)theNewWidth;
	double ch = 1/(double)theNewHeight;
	double csx = _min(1, sx) * _min(1, sx);
	double csy = _min(1, sy) * _min(1, sy);
	double cx=0.0f;
	double cy=0.0f;

	double sourcePixelX=0.0f;
	double sourcePixelY=0.0f;

	unsigned int cachePrecision = CACHE_PRECISION;
	unsigned int filterSize = FILTER_SIZE;
	Array<double> cache;
	CreateLancozCache(cachePrecision,filterSize,cache);

	y = theNewHeight;
	while(y--)
	{
		sourcePixelY = (y + 0.5f) * isy;
		y1b = (int)(sourcePixelY - filterSize);
		if(y1b < 0)
			y1b = 0;
		y1e = y1et = (sourcePixelY + filterSize);
		if(y1e != y1et)
			y1e = y1et + 1;
		if(y1e > sh1)
			y1e = sh1;
		cy = y * ch - sourcePixelY;
		y3 = y * theNewWidth;

		x = theNewWidth;
		while(x--)
		{
			sourcePixelX = (x + 0.5) * isx;
			x1b = sourcePixelX - filterSize;
			if(x1b < 0)
				x1b = 0;
			x1e = x1et = sourcePixelX + filterSize;
			if(x1e != x1et)
				x1e = x1et + 1;
			if(x1e > sw1)
				x1e = sw1;
			cx = x * cw - sourcePixelX;

			values.Reset();
			values.SetSize(256);
			i=0;
			total=0;
			for(y1 = y1b; y1 <= y1e; y1++)
			{
				distanceY = (y1 + cy) * (y1 + cy) * csy;
				for(x1 = x1b; x1 <= x1e; x1++)
				{
					values[i] = cache[(unsigned int)(((x1 + cx) * (x1 + cx) * csx + distanceY) * cachePrecision)] || 0;
					total += values[i];
					i++;
				}
			}

			total = 1 / total;

			i = a = r = g = b = 0;
			for(y1 = y1b; y1 <= y1e; y1++)
			{
				y2 = y1 * mWidth;
				for(x1 = x1b; x1 <= x1e; x1++)
				{
					color = mData[(unsigned int)(y2 + x1)];
					value = values[i++] * total;
					a += (unsigned char)(color.mA * value);
					r += (unsigned char)(color.mR * value);
					g += (unsigned char)(color.mG * value);
					b += (unsigned char)(color.mB * value);
				}
			}
			aNewData[(unsigned int)(x + y3)].mA=a;
			aNewData[(unsigned int)(x + y3)].mR=r;
			aNewData[(unsigned int)(x + y3)].mG=g;
			aNewData[(unsigned int)(x + y3)].mB=b;
		}
	} // while y

	if (!theImage) 
	{
		theImage=this;
		_DeleteArray(mData);
		mData=NULL;
	}
	else 
	{
		theImage->Reset();
	}

	theImage->mWidth=theNewWidth;
	theImage->mHeight=theNewHeight;
	theImage->mData=aNewData;
}

void Image::RotateCW()
{
	Image aTempImage;
	aTempImage.MakeBlank(mHeight,mWidth);

	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA* aSrc=GetPixelPtr(aSpanX,aSpanY);
			RGBA* aDest=aTempImage.GetPixelPtr(aTempImage.mWidth-1-aSpanY,aSpanX);

			*aDest=*aSrc;
		}
	}

	CopyFrom(aTempImage);
}

void Image::FlipH()
{
	Image aTempImage;
	aTempImage.MakeBlank(mWidth,mHeight);

	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA* aSrc=GetPixelPtr(aSpanX,aSpanY);
			RGBA* aDest=aTempImage.GetPixelPtr(aTempImage.mWidth-1-aSpanX,aSpanY);
			*aDest=*aSrc;
		}
	}

	CopyFrom(aTempImage);
}

void Image::FlipV()
{
	Image aTempImage;
	aTempImage.MakeBlank(mWidth,mHeight);

	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA* aSrc=GetPixelPtr(aSpanX,aSpanY);
			RGBA* aDest=aTempImage.GetPixelPtr(aSpanX,aTempImage.mHeight-1-aSpanY);
			*aDest=*aSrc;
		}
	}

	CopyFrom(aTempImage);
}


void Image::GuassianBlur(int radius, bool bWrapped)
{
	// Calculate the guassian matrix
	int matrix_size=radius*2+1;
	float sigma=radius/3.0f;
	float sigma2=2*Squared(sigma);
	float r2PISigma = sqrt(2*gMath.mPI*sigma);
	float r2 = (float)Squared(radius);


	Array<float> gauss_matrix;; //NOTE : So why just flat line and not a matrix? Because the matrix is the same in both directions. So the math says I can ignore the angled bits if I add 
	//      everything up horizontally first and then vertically ("separable" as they say in magical math wizard land) So.. if you add the bits from horz and 
	//      vert you end up with the same result as you would if you'd added the bits altogether in one pass, but it's a lot faster to do it piecemeal
	gauss_matrix.SetSize(matrix_size);

	float gauss_sum=0;
	for(int i=0;i<matrix_size; i++)
	{
		int dist = abs(i-radius);
		gauss_matrix[i]=exp(-1*Squared(dist)/sigma2)/r2PISigma;
		gauss_sum+=gauss_matrix[i];
	}
	// makes the sum of the entries 1
	for(int i=0;i<matrix_size;i++)
	{
		float t = gauss_matrix[i];
		t/=gauss_sum;
		gauss_matrix[i]=t;
	}

	// ok now apply the matrix to the image
	// NOTE : if you wanted a fast guassian blur you could do a thing where you pre-calc the matrix and just keep it cached somewhere
	Image aBlurredImage;
	aBlurredImage.MakeBlank(mWidth,mHeight);

	// calc blurred pixels horizontally
	RGBA edge;
	edge.mA=0;
	for(int i=0;i<mWidth;i++)
	{
		for(int j=0;j<mHeight;j++)
		{
			RGBA* aDst=aBlurredImage.GetPixelPtr(i,j);
			RGBA blurred;
			blurred.mA=0; blurred.mB=0; blurred.mG=0; blurred.mR=0;
			for(int k=-radius;k<radius;k++)
			{
				int xpixel = i+k;
				RGBA* aSrc=GetPixelPtr(xpixel,j);
				if(aSrc==NULL)
				{
					if(bWrapped)
					{
						if(xpixel<0) xpixel+=mWidth;
						if(xpixel>mWidth-1) xpixel-=mWidth;
						aSrc=aBlurredImage.GetPixelPtr(xpixel,j);
					}
					else
					{
						if(xpixel<0) aSrc=GetPixelPtr(0,j); else aSrc=GetPixelPtr(mWidth-1,j);
						edge.mR=aSrc->mR; edge.mG=aSrc->mG; edge.mB=aSrc->mB;
						aSrc=&edge;
					}
				}

				blurred.mR+=aSrc->mR*gauss_matrix[k+radius];
				blurred.mG+=aSrc->mG*gauss_matrix[k+radius];
				blurred.mB+=aSrc->mB*gauss_matrix[k+radius];
				blurred.mA+=aSrc->mA*gauss_matrix[k+radius];
			}
			*aDst=blurred;
		}
	}

	// calc blurred pixels vertically
	for(int j=0;j<mHeight;j++)
	{
		for(int i=0;i<mWidth;i++)
		{
			RGBA* aDst=aBlurredImage.GetPixelPtr(i,j);
			RGBA blurred;
			blurred.mA=0; blurred.mB=0; blurred.mG=0; blurred.mR=0;
			for(int k=-radius;k<radius;k++)
			{
				int ypixel = j+k;
				RGBA* aSrc=aBlurredImage.GetPixelPtr(i,ypixel);
				if(aSrc==NULL)
				{
					if(bWrapped)
					{
						if(ypixel<0) ypixel+=mHeight;
						if(ypixel>mHeight-1) ypixel-=mHeight;
						aSrc=aBlurredImage.GetPixelPtr(i,ypixel);
					}
					else
					{
						if(ypixel<0) aSrc=GetPixelPtr(i,0); else aSrc=GetPixelPtr(i, mHeight-1);
						edge.mR=aSrc->mR; edge.mG=aSrc->mG; edge.mB=aSrc->mB;
						aSrc=&edge;
					}
				}

				blurred.mR+=aSrc->mR*gauss_matrix[k+radius];
				blurred.mG+=aSrc->mG*gauss_matrix[k+radius];
				blurred.mB+=aSrc->mB*gauss_matrix[k+radius];
				blurred.mA+=aSrc->mA*gauss_matrix[k+radius];
			}
			*aDst=blurred;
		}
	}

	// Copy TempImage back to this image
	CopyFrom(aBlurredImage);
}


void Image::Bleed(int theSize, char theAlpha)
{
	List aFixList;
	struct FixStruct
	{
		RGBA *mPtr;
		int mRed;
		int mGreen;
		int mBlue;
	};

	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA *aPtr=GetPixelPtr(aSpanX,aSpanY);
			if (aPtr->mA==0)
			{
				//
				// See if we're next to a color pixel
				//
				int aRedMix=0;
				int aGreenMix=0;
				int aBlueMix=0;
				int aMixCount=0;

				for (int aTestX=aSpanX-theSize;aTestX<=aSpanX+theSize;aTestX++)
				{
					if (aTestX<0 || aTestX>=mWidth) continue;
					for (int aTestY=aSpanY-theSize;aTestY<=aSpanY+theSize;aTestY++)
					{
						if (aTestY<0 || aTestY>=mHeight) continue;

						RGBA* aMixRGB=GetPixelPtr(aTestX,aTestY);
						if (aMixRGB->mA)
						{
							aRedMix+=aMixRGB->mR;
							aGreenMix+=aMixRGB->mG;
							aBlueMix+=aMixRGB->mB;
							aMixCount++;
						}
					}
				}
				if (aMixCount>0)
				{
					aRedMix/=aMixCount;
					aGreenMix/=aMixCount;
					aBlueMix/=aMixCount;

					FixStruct *aFix=new FixStruct;
					aFix->mPtr=aPtr;
					aFix->mRed=aRedMix;
					aFix->mGreen=aGreenMix;
					aFix->mBlue=aBlueMix;

					aFixList+=aFix;
				}

				aPtr->mR=0;
				aPtr->mG=0;
				aPtr->mB=0;
			}
			//	aPtr->mA=255;
		}
	}

	EnumList(FixStruct,aFix,aFixList)
	{
		aFix->mPtr->mR=aFix->mRed;
		aFix->mPtr->mG=aFix->mGreen;
		aFix->mPtr->mB=aFix->mBlue;
		aFix->mPtr->mA=theAlpha;
	}

	_FreeList(FixStruct,aFixList);
}


void Image::SetPixel(int theXPos, int theYPos, Color theColor)
{
	if (theXPos<0) return;
	if (theYPos<0) return;
	if (theXPos>=mWidth) return;
	if (theYPos>=mHeight) return;

	RGBA aSourcePixel;
	aSourcePixel.mR=(int)(theColor.mR*255);
	aSourcePixel.mG=(int)(theColor.mG*255);
	aSourcePixel.mB=(int)(theColor.mB*255);
	aSourcePixel.mA=(int)(theColor.mA*255);


	RGBA *aDest=GetPixelPtr(theXPos,theYPos);
	RGBA* aSrc=&aSourcePixel;

	if (aSrc->mA*theColor.mA>=250 || aDest->mA==0)
	{
		aDest->mR=(int)(aSrc->mR);//*theColor.mR);
		aDest->mG=(int)(aSrc->mG);//*theColor.mG);
		aDest->mB=(int)(aSrc->mB);//*theColor.mB);
		aDest->mA=(int)(aSrc->mA);//*theColor.mA);
	}
	else
	{

		float aSrcAlpha=(float)aSrc->mA/255.0f;
		aSrcAlpha*=theColor.mA;
		float aDestAlpha=1.0f-((float)aSrcAlpha);

		float aSrcRed=(float)aSrc->mR*theColor.mR*aSrcAlpha;
		float aSrcGreen=(float)aSrc->mG*theColor.mG*aSrcAlpha;
		float aSrcBlue=(float)aSrc->mB*theColor.mB*aSrcAlpha;

		float aDestRed=(float)aDest->mR*aDestAlpha;
		float aDestGreen=(float)aDest->mG*aDestAlpha;
		float aDestBlue=(float)aDest->mB*aDestAlpha;

		int aResultRed=(int)_min(255,(aSrcRed+.5f)+(aDestRed+.5f));
		int aResultGreen=(int)_min(255,(aSrcGreen+.5f)+(aDestGreen+.5f));
		int aResultBlue=(int)_min(255,(aSrcBlue+.5f)+(aDestBlue+.5f));

		aDest->mR=aResultRed;
		aDest->mG=aResultGreen;
		aDest->mB=aResultBlue;
		aDest->mA=_max(aDest->mA,aSrc->mA);
	}
}

void Image::BresenhamLine(int x1,int y1,int x2,int y2, Color theColor)
{
	int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
	dx=x2-x1;
	dy=y2-y1;
	dx1=gMath.Abs(dx);
	dy1=gMath.Abs(dy);
	px=2*dy1-dx1;
	py=2*dx1-dy1;
	if(dy1<=dx1)
	{
		if(dx>=0)
		{
			x=x1;
			y=y1;
			xe=x2;
		}
		else
		{
			x=x2;
			y=y2;
			xe=x1;
		}
		SetPixel(x,y,theColor);
		for(i=0;x<xe;i++)
		{
			x=x+1;
			if(px<0)
			{
				px=px+2*dy1;
			}
			else
			{
				if((dx<0 && dy<0) || (dx>0 && dy>0))
				{
					y=y+1;
				}
				else
				{
					y=y-1;
				}
				px=px+2*(dy1-dx1);
			}
			SetPixel(x,y,theColor);
		}
	}
	else
	{
		if(dy>=0)
		{
			x=x1;
			y=y1;
			ye=y2;
		}
		else
		{
			x=x2;
			y=y2;
			ye=y1;
		}
		SetPixel(x,y,theColor);
		for(i=0;y<ye;i++)
		{
			y=y+1;
			if(py<=0)
			{
				py=py+2*dx1;
			}
			else
			{
				if((dx<0 && dy<0) || (dx>0 && dy>0))
				{
					x=x+1;
				}
				else
				{
					x=x-1;
				}
				py=py+2*(dx1-dy1);
			}
			SetPixel(x,y,theColor);
		}
	}
}

void Image::FloodFill(int theXPos, int theYPos, Color theFillColor)
{
	RGBA* aRGBA=GetPixelPtr(theXPos,theYPos);
	if (!aRGBA) return;
	if (aRGBA->mA>0) return;

	SetPixel(theXPos,theYPos,theFillColor);
	FloodFill(theXPos-1,theYPos,theFillColor);
	FloodFill(theXPos+1,theYPos,theFillColor);
	FloodFill(theXPos,theYPos-1,theFillColor);
	FloodFill(theXPos,theYPos+1,theFillColor);

}

void Image::FillRect(int theXPos, int theYPos, int theWidth, int theHeight, Color theColor)
{
	int aR=(int)(theColor.mR*255.0f);
	int aG=(int)(theColor.mG*255.0f);
	int aB=(int)(theColor.mB*255.0f);
	int aA=(int)(theColor.mA*255.0f);

	for (int aSpanY=theYPos;aSpanY<theYPos+theHeight;aSpanY++)
	{
		for (int aSpanX=theXPos;aSpanX<theXPos+theWidth;aSpanX++)
		{
			RGBA* aPix=GetPixelPtr(aSpanX,aSpanY);
			if (aPix)
			{
				aPix->mR=aR;
				aPix->mG=aG;
				aPix->mB=aB;
				aPix->mA=aA;
			}
		}
	}


}

void Image::MakeTileSafe(Rect theRect)
{
	theRect=theRect.Expand(1);

	theRect=theRect.Translate(.5f,.5f);


	int aX1=(int)theRect.mX;
	int aX2=(int)(theRect.mX+theRect.mWidth);
	int aY1=(int)theRect.mY;
	int aY2=(int)(theRect.mY+theRect.mHeight);

	// Top
	for (int aSpanX=aX1;aSpanX<aX2;aSpanX++)
	{
		RGBA* aSrc=GetPixelPtr(aSpanX,aY1+1);
		RGBA* aDest=GetPixelPtr(aSpanX,aY1);
		if (aSrc && aDest) *aDest=*aSrc;
	}
	// Bottom
	for (int aSpanX=aX1;aSpanX<aX2;aSpanX++)
	{
		RGBA* aSrc=GetPixelPtr(aSpanX,aY2-2);
		RGBA* aDest=GetPixelPtr(aSpanX,aY2-1);
		if (aSrc && aDest) *aDest=*aSrc;
	}
	// Left
	for (int aSpanY=aY1;aSpanY<aY2;aSpanY++)
	{
		RGBA* aSrc=GetPixelPtr(aX1+1,aSpanY);
		RGBA* aDest=GetPixelPtr(aX1,aSpanY);
		if (aSrc && aDest) *aDest=*aSrc;
	}
	// Right
	for (int aSpanY=aY1;aSpanY<aY2;aSpanY++)
	{
		RGBA* aSrc=GetPixelPtr(aX2-2,aSpanY);
		RGBA* aDest=GetPixelPtr(aX2-1,aSpanY);
		if (aSrc && aDest) *aDest=*aSrc;
	}

}

void Image::PremultiplyAlpha()
{
	RGBA* aPix=GetBits();
	for (int aCount=0;aCount<mWidth*mHeight;aCount++)
	{
		float aR=(float)aPix->mR/255.0f;
		float aG=(float)aPix->mG/255.0f;
		float aB=(float)aPix->mB/255.0f;
		float aA=(float)aPix->mA/255.0f;
		aR*=aA;
		aG*=aA;
		aB*=aA;

		aPix->mR=(unsigned char)(aR*255.0f);
		aPix->mG=(unsigned char)(aG*255.0f);
		aPix->mB=(unsigned char)(aB*255.0f);
		aPix++;
	}
}

//
// Macho Format is to get around some restraints and outright bugs that Apple invokes onto PNGs.
// Macho files are big, but in later versions I'll do some compression. 
// Version 0 is just a raw file of transparency RLE
//

//
// Macho Format is to get around some restraints and outright bugs that Apple invokes onto PNGs.
// Macho files are big, but in later versions I'll do some compression. 
// Version 0 is just a raw file of transparency RLE
//

//
// Macho Format is to get around some restraints and outright bugs that Apple invokes onto PNGs.
// Macho files are big, but in later versions I'll do some compression. 
// Version 0 is just a raw file of transparency RLE
//

void Image::LoadThumb(String theFilename)
{
	IOBuffer aBuffer;
	aBuffer.Load(theFilename);
	LoadThumb(aBuffer);
}

void Image::LoadThumb(IOBuffer& theBuffer)
{
	int aVersion=theBuffer.ReadInt();
	if (aVersion!=0 && aVersion!=1) 
	{
		MakeBlank(1,1);
		return;
	}

	int aWidth=theBuffer.ReadInt();
	int aHeight=theBuffer.ReadInt();
	MakeBlank(aWidth,aHeight);

	switch (aVersion)
	{
	case 0:theBuffer.ReadRaw(GetBits(),aWidth*aHeight*4);break;
	case 1:
		{
			IOBuffer aMinibuffer;
			theBuffer.ReadBuffer(aMinibuffer);

			char *aRGB=new char[mWidth*mHeight*3];
			char *aA=new char[mWidth*mHeight];

			aMinibuffer.ReadRaw(aRGB,aWidth*aHeight*3);
			aMinibuffer.ReadRaw(aA,aWidth*aHeight);

			char *aRGBPtr=aRGB;
			char *aAPtr=aA;
            char *aBits=(char*)GetBits();
            for (int aCount=0;aCount<mWidth*mHeight;aCount++)
			{
                *aBits=*aRGBPtr;aRGBPtr++;aBits++;
				*aBits=*aRGBPtr;aRGBPtr++;aBits++;
				*aBits=*aRGBPtr;aRGBPtr++;aBits++;
				*aBits=*aAPtr;aAPtr++;aBits++;
			}
			/*
			else
			{
				for (int aCount=0;aCount<mWidth*mHeight;aCount++)
				{
					*aBits=*(aRGBPtr+2);aBits++;
					*aBits=*(aRGBPtr+1);aBits++;
					*aBits=*(aRGBPtr+0);aBits++;
					aRGBPtr+=3;
					*aBits=*aAPtr;aAPtr++;aBits++;
				}
			}
			 */
			delete [] aRGB;
			delete [] aA;
			break;
		}
	}

	//
	// Android has a crappy dual-load issue... using Java to load the graphics, it loads them in correctly formatted for OpenGL in BGRA
	// But our THUMB files will be in the opposite order (this is because Android is letting us know that images will be in brga but isn't swapping on image loads because the java is already correct)
	//
	if (Graphics_Core::Query("KludgeRGBA")) this->SwapRB();
}

void Image::ExportThumb(String theFilename,Array<Color> &thePalette)
{
	IOBuffer aBuffer;
	ExportThumb(aBuffer,thePalette);
	aBuffer.CommitFile(theFilename);
}

void Image::ExportThumb(IOBuffer& theBuffer,Array<Color> &thePalette)
{
	//
	// Serious Issue!
	// Okay, so Apple+Android do a SwapRB to get it into the correct format (i.e. it needs to go into RGBA so it's consistently interpreted across all platforms)...
	// BUT here's the issue... if you load a thumb, and then re-export it, you get this SwapRB twice.  There's no flag in the thumb to indicate that it's already been
	// swapped (maybe I need to add one?).  The reason is, when an image is loaded on iOS+Android, it gets its r+b swapped in the core.  But when we copy the pixels from
	// the RenderToBits into the thumb, they're in the wrong format, and need to be swapped.
	//
	// The solution?  The GetRenderToBits function on iOS and Android needs to reverse the pixels.  Right now we just get a pointer to them, but clearly it needs to be modified
	// so that we can dump it right to the &mImage->mData and &mImage->mWdith and &mImage->mHeight, yes?  Then the core can swap the pixels.
	//


	//
	//if (!gG.GetRGBOrder().StartsWith("r")) SwapRB();

	Paletteize(thePalette);

	//
	// First, remove color wherever it does not matter...
	//
	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA* aPix=GetPixelPtr(aSpanX,aSpanY);
			if (aPix->mA==0)
			{
				bool aMatters=false;
				for (int aCount=0;aCount<9;aCount++)
				{
					IPoint aDir=gMath.GetCardinalDirection(aCount);
					RGBA* aCheckPix=GetPixelPtr(aSpanX+aDir.mX,aSpanY+aDir.mY);
					if (aCheckPix && aCheckPix->mA>0) {aMatters=true;break;}
				}
				if (!aMatters) aPix->mR=aPix->mG=aPix->mB=0;
			}
		}
	}

	/*

	//
	// Compression experiment... half as many colors really saves quite a bit...
	//
	for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
		{
			RGBA* aPix=GetPixelPtr(aSpanX,aSpanY);
			aPix->mA/=2;aPix->mA*=2;
			aPix->mR/=2;aPix->mR*=2;
			aPix->mG/=2;aPix->mG*=2;
			aPix->mB/=2;aPix->mB*=2;
		}
	}
	*/


	int aVersion=1;
	theBuffer.WriteInt(aVersion);
	theBuffer.WriteInt(mWidth);
	theBuffer.WriteInt(mHeight);

	IOBuffer aMiniBuffer;

	switch (aVersion)
	{
	case 0:
		{
			//
			// Just pure raw data
			//
            char *aRGB=new char[mWidth*mHeight*4];
            char *aRGBPtr=aRGB;
            RGBA* aPix=GetBits();

            if (!Graphics_Core::Query("KludgeRGBA"))
            {
                for (int aCount=0;aCount<mWidth*mHeight;aCount++)
                {
                    *aRGBPtr=aPix->mB;aRGBPtr++;
                    *aRGBPtr=aPix->mG;aRGBPtr++;
                    *aRGBPtr=aPix->mR;aRGBPtr++;
                    *aRGBPtr=255;aRGBPtr++;
                    aPix++;
                }
            }
            else
            {
                for (int aCount=0;aCount<mWidth*mHeight;aCount++)
                {
                    *aRGBPtr=aPix->mR;aRGBPtr++;
                    *aRGBPtr=aPix->mG;aRGBPtr++;
                    *aRGBPtr=aPix->mB;aRGBPtr++;
                    *aRGBPtr=255;aRGBPtr++;
                    aPix++;
                }
            }

			theBuffer.WriteRaw(aRGB,mWidth*mHeight*4);
            delete [] aRGB;
			break;
		}
	case 1:
		{
			//
			// Simple Compression: All RGB in one span, all A in another,
			// then zipped. (Produces approx 1.5x-2x of PNGs for large ones,
			// can produce much smaller than png for small ones)
			//
			char *aRGB=new char[mWidth*mHeight*3];
			char *aA=new char[mWidth*mHeight];
			char *aRGBPtr=aRGB;
			char *aAPtr=aA;
			RGBA* aPix=GetBits();

			//
			// A little bit of a kludge here... between OpenGL and Windows there's different rgba order.
			// HOWEVER, there's funny locations where/when it's actually an issue.  And this is one of them.
			//
			if (!Graphics_Core::Query("KludgeRGBA"))
			{
				for (int aCount=0;aCount<mWidth*mHeight;aCount++)
				{
					*aRGBPtr=aPix->mB;aRGBPtr++;
					*aRGBPtr=aPix->mG;aRGBPtr++;
					*aRGBPtr=aPix->mR;aRGBPtr++;
					*aAPtr=aPix->mA;aAPtr++;
					aPix++;
				}
			}
			else
			{
				for (int aCount=0;aCount<mWidth*mHeight;aCount++)
				{
					*aRGBPtr=aPix->mR;aRGBPtr++;
					*aRGBPtr=aPix->mG;aRGBPtr++;
					*aRGBPtr=aPix->mB;aRGBPtr++;
					*aAPtr=aPix->mA;aAPtr++;
					aPix++;
				}
			}
			aMiniBuffer.WriteRaw(aRGB,mWidth*mHeight*3);
			aMiniBuffer.WriteRaw(aA,mWidth*mHeight);

			delete [] aRGB;
			delete [] aA;

			theBuffer.WriteBuffer(aMiniBuffer);
			break;
		}
	}
//	ExportPNG(PointAtDesktop("macho.png"));
}

void Image::MakePow2(bool blankEdges)
{
	int aHoldWidth=mWidth;
	int aHoldHeight=mHeight;
	RGBA *aHoldData=mData;

	mWidth=gMath.NearestPowerOfTwo(mWidth);
	mHeight=gMath.NearestPowerOfTwo(mHeight);
	mData=new RGBA[mWidth*mHeight];
	if (blankEdges) memset(mData,0,mWidth*mHeight*sizeof(RGBA));

	RGBA* aSrcPtr=aHoldData;
	RGBA* aDestPtr=mData;

	for (int aSpanY=0;aSpanY<aHoldHeight;aSpanY++)
	{
		memcpy(aDestPtr,aSrcPtr,aHoldWidth*sizeof(RGBA));
		aSrcPtr+=aHoldWidth;
		aDestPtr+=mWidth;
	}

	delete [] aHoldData;

}

/*
private static C3 findClosestPaletteColor(C3 c, C3[] palette) {
	C3 closest = palette[0];

	for (C3 n : palette) 
		if (n.diff(c) < closest.diff(c))
			closest = n;

	return closest;
}
*/

struct IntRGBA
{
	IntRGBA() {}
	IntRGBA(int r, int g, int b) {mR=r;mG=g;mB=b;}
	IntRGBA operator=(RGBA theParam) {mR=theParam.mR;mG=theParam.mG;mB=theParam.mB;return *this;}
	int mR,mG,mB;

	void Add(IntRGBA theColor) {mR+=theColor.mR;mG+=theColor.mG;mB+=theColor.mB;}
	IntRGBA Subtract(IntRGBA theColor) {return IntRGBA(mR-theColor.mR,mG-theColor.mG,mB-theColor.mB);}
	IntRGBA Multiply(float theMult) {return IntRGBA((int)(mR*theMult),(int)(mG*theMult),(int)(mB*theMult));}
};


int FindClosestPaletteColor(IntRGBA theColor, Array<IntRGBA>& thePal)
{
	int aMatch=0;
	int aBestDiff=9999999;
	for (int aCount=0;aCount<thePal.Size();aCount++)
	{
		int aDiff=gMath.Abs(thePal[aCount].mR-theColor.mR)+gMath.Abs(thePal[aCount].mG-theColor.mG)+gMath.Abs(thePal[aCount].mB-theColor.mB);
		if (aDiff<aBestDiff)
		{
			aMatch=aCount;
			aBestDiff=aDiff;
		}
	}
	return aMatch;
}

void Image::Paletteize(Array<Color>& thePalette)
{
	Array<IntRGBA> aPal;
	aPal.GuaranteeSize(thePalette.Size());
	for (int aCount=0;aCount<aPal.Size();aCount++)
	{
		//
		// Note that RGB is reversed here... there's some funny business going on that I don't have a good hold of,
		// related to how PNGs get loaded.  The RGBA is correct for bits in memory, like render textures, but in the image itself, R&B are reversed!
		//
		aPal[aCount].mR=(int)((float)thePalette[aCount].mB*255.0f);
		aPal[aCount].mG=(int)((float)thePalette[aCount].mG*255.0f);
		aPal[aCount].mB=(int)((float)thePalette[aCount].mR*255.0f);
	}

	IntRGBA* aSource=new IntRGBA[mWidth*mHeight];
	IntRGBA* aSourcePtr=aSource;
	RGBA* aSrcPtr=GetBits();

	for (int aCount=0;aCount<mWidth*mHeight;aCount++) 
	{
		*aSourcePtr=*aSrcPtr;
		aSourcePtr++;
		aSrcPtr++;
	}

	aSourcePtr=aSource;
	RGBA* aDestPtr=GetBits();
	for (int aSpanY=0;aSpanY<mHeight;aSpanY++) 
	{
		for (int aSpanX=0;aSpanX<mWidth;aSpanX++) 
		{
			IntRGBA aOldColor=*aSourcePtr;
			IntRGBA aNewColor=aPal[FindClosestPaletteColor(aOldColor,aPal)];

			aDestPtr->mR=_clamp(0,aNewColor.mR,255);
			aDestPtr->mG=_clamp(0,aNewColor.mG,255);
			aDestPtr->mB=_clamp(0,aNewColor.mB,255);
			aDestPtr->mA=255;

			IntRGBA aError=aOldColor.Subtract(aNewColor);
			aError.mR=_clamp(0,aError.mR,255);
			aError.mG=_clamp(0,aError.mG,255);
			aError.mB=_clamp(0,aError.mB,255);

			if (aSpanX+1<mWidth) 
			{
				IntRGBA* aPtr=aSourcePtr+1;
				aPtr->Add(aError.Multiply(7.0f/16.0f));
			}
			if (aSpanX-1>=0 && aSpanY+1<mHeight)
			{
				IntRGBA* aPtr=aSourcePtr-1+mWidth;
				aPtr->Add(aError.Multiply(3.0f/16.0f));
			}
			if (aSpanY+1<mHeight)
			{
				IntRGBA* aPtr=aSourcePtr+mWidth;
				aPtr->Add(aError.Multiply(5.0f/16.0f));
			}
			if (aSpanX+1<mWidth && aSpanY+1<mHeight)
			{
				IntRGBA* aPtr=aSourcePtr+1+mWidth;
				aPtr->Add(aError.Multiply(1.0f/16.0f));
			}

			aSourcePtr++;
			aDestPtr++;
		}
	}
	delete [] aSource;
}

void Image::ExportPThumb(String theFilename,Array<Color>& thePalette)
{
	IOBuffer aBuffer;
	ExportPThumb(aBuffer,thePalette);
	aBuffer.CommitFile(theFilename);
}

void Image::ExportPThumb(IOBuffer& theBuffer,Array<Color>& thePalette)
{
	int aVersion=1;
	theBuffer.WriteInt(aVersion);
	theBuffer.WriteInt(mWidth);
	theBuffer.WriteInt(mHeight);

	unsigned char* aArray=new unsigned char[mWidth*mHeight];
	unsigned char* aArrayPtr=aArray;
    
    bool aReverseRGB=!gG.GetRGBOrder().StartsWith("r");

	//Paletteize...
	{
		Array<IntRGBA> aPal;
		aPal.GuaranteeSize(thePalette.Size());
        for (int aCount=0;aCount<aPal.Size();aCount++)
        {
            //
            // Note that RGB is reversed here... there's some funny business going on that I don't have a good hold of,
            // related to how PNGs get loaded.  The RGBA is correct for bits in memory, like render textures, but in the image itself, R&B are reversed!
            //
            aPal[aCount].mR=(int)((float)thePalette[aCount].mB*255.0f);
            aPal[aCount].mG=(int)((float)thePalette[aCount].mG*255.0f);
            aPal[aCount].mB=(int)((float)thePalette[aCount].mR*255.0f);
        }

		IntRGBA* aSource=new IntRGBA[mWidth*mHeight];
		IntRGBA* aSourcePtr=aSource;
		RGBA* aSrcPtr=GetBits();

		for (int aCount=0;aCount<mWidth*mHeight;aCount++) 
		{
			*aSourcePtr=*aSrcPtr;
			aSourcePtr++;
			aSrcPtr++;
		}

		aSourcePtr=aSource;
		for (int aSpanY=0;aSpanY<mHeight;aSpanY++) 
		{
			for (int aSpanX=0;aSpanX<mWidth;aSpanX++) 
			{
				IntRGBA aOldColor=*aSourcePtr;
				int aPick=FindClosestPaletteColor(aOldColor,aPal);
				*aArrayPtr=(unsigned char)aPick;aArrayPtr++;
				IntRGBA aNewColor=aPal[aPick];

				IntRGBA aError=aOldColor.Subtract(aNewColor);
				aError.mR=_clamp(0,aError.mR,255);
				aError.mG=_clamp(0,aError.mG,255);
				aError.mB=_clamp(0,aError.mB,255);

				if (aSpanX+1<mWidth) 
				{
					IntRGBA* aPtr=aSourcePtr+1;
					aPtr->Add(aError.Multiply(7.0f/16.0f));
				}
				if (aSpanX-1>=0 && aSpanY+1<mHeight)
				{
					IntRGBA* aPtr=aSourcePtr-1+mWidth;
					aPtr->Add(aError.Multiply(3.0f/16.0f));
				}
				if (aSpanY+1<mHeight)
				{
					IntRGBA* aPtr=aSourcePtr+mWidth;
					aPtr->Add(aError.Multiply(5.0f/16.0f));
				}
				if (aSpanX+1<mWidth && aSpanY+1<mHeight)
				{
					IntRGBA* aPtr=aSourcePtr+1+mWidth;
					aPtr->Add(aError.Multiply(1.0f/16.0f));
				}

				aSourcePtr++;
			}
		}
		delete [] aSource;
	}

	theBuffer.WriteRaw(aArray,mWidth*mHeight);

	RGBA* aDestPtr=GetBits();
	aArrayPtr=aArray;
    for (int aCount=0;aCount<mWidth*mHeight;aCount++)
	{
		Color& aC=thePalette[*aArrayPtr];
        aDestPtr->mR=(unsigned char)(aC.mB*255);
        aDestPtr->mG=(unsigned char)(aC.mG*255);
        aDestPtr->mB=(unsigned char)(aC.mR*255);
		aDestPtr->mA=255;
		aDestPtr++;
		aArrayPtr++;
	}
	delete [] aArray;
}

void Image::SwapRB()
{
	RGBA* aPtr=GetBits();
	for (int aCount=0;aCount<mWidth*mHeight;aCount++)
	{
		unsigned char aHold=aPtr->mR;
		aPtr->mR=aPtr->mB;
		aPtr->mB=aHold;
		aPtr++;
	}
}

bool Image::IsBlank(float theThreshold)
{
	int aT=theThreshold*255;
	RGBA* aPtr=GetBits();
	for (int aCount=0;aCount<mWidth*mHeight;aCount++)
	{
		if (aPtr->mA>aT) return false;
		aPtr++;
	}
	return true;
}

bool Image::IsBlank(Rect theRect,float theThreshold)
{
	int aT=theThreshold*255;
	for (int aSpanY=(int)theRect.mY;aSpanY<(int)(theRect.mY+theRect.mHeight);aSpanY++)
	{
		RGBA* aPtr=GetPixelPtr((int)theRect.mX,aSpanY);
		for (int aSpanX=(int)theRect.mX;aSpanX<(int)(theRect.mX+theRect.mWidth);aSpanX++)
		{
			if (aPtr->mA>aT) return false;
			aPtr++;
		}
	}
	return true;
}

/*

//
// Threaded download for images... maybe too risky, honestly!
//

int gPendingCount=0;	// To throttle how many images are coming down...
void PendingImageStub(void* theArg)
{
	PendingImage* aPS=(PendingImage*)theArg;
	String aFN=aPS->mFN;aFN+=".";aFN+=aPS->mTail;

	while (gPendingCount>=2) 
	{
		Sleep(50);
		if (DoesFileExist(aFN)) break;
	}

	gPendingCount++;
	bool aFail=false;
	if (!DoesFileExist(aFN))
	{
		IOBuffer aBuffer;
		aBuffer.Download(aPS->mURL);

		if (aBuffer.Len()==0) aFail=true;
		else if (aBuffer.Len()<64) aFail=true;
		else if (aBuffer.mData[0]=='<') 
		{
			String aCheck=aBuffer.ToString(100);
			if (aCheck.ContainsI("<html>")) aFail=true;
			if (aCheck.ContainsI("::RML")) aFail=true;
		}
			
		if (!aFail) aBuffer.CommitFile(aFN);
	}

	ThreadLock(gPendingImageLock);
	if (aPS->mImage) 
	{
		if (!aFail) aPS->mImage->Load(aPS->mFN);
		//Sleep(25);
		aPS->mImage->mPending=false;
	}
	gPendingImageList-=aPS;delete aPS;
	ThreadUnlock(gPendingImageLock);

	gPendingCount=_max(gPendingCount-1,0);
}

bool Image::Download(String theURL, String theVersion, String* returnLocalFN)
{
	String aTail=theURL.GetTail('.');
	String aFN=URLToFN(theURL);//.RemoveTail('.'));
	aFN+="_v";
	aFN+=theVersion;
	aFN=PointAtCache(aFN);

	if (DoesImageExist(aFN)) Load(aFN);
	if (returnLocalFN) *returnLocalFN=Sprintf("%s.%s",aFN.c(),aTail.c());
	if (mWidth>0) return true;

	//
	// Cached image was bad...
	//
	Reset();
	//
	// Need to download this image and load it later...
	//
	mPending=true;
	PendingImage* aPS=new PendingImage;
	aPS->mImage=this;
	aPS->mURL=theURL;
	aPS->mFN=aFN;
	aPS->mTail=aTail;
	ThreadLock(gPendingImageLock);
	gPendingImageList+=aPS;
	ThreadUnlock(gPendingImageLock);
	Thread(&PendingImageStub,aPS);
	return false;
}
*/

void Image::LoadFromWeb(String theURL, String theVersion)
{
	String aTail=theURL.GetTail('.');
	String aFN="cache://";
	aFN+=URLToFN(theURL);//.RemoveTail('.'));
	//aFN+="_v";
	//aFN+=theVersion;
	//aFN=PointAtCache(aFN);
	String aCacheVersionName=aFN;aCacheVersionName+=".";aCacheVersionName+=theVersion;

	if (DoesImageExist(aFN) && DoesFileExist(aCacheVersionName))
	{
		Load(aFN);
		return;
	}

#ifdef _DEBUG
	gOut.Out(">>>> Downloading Image: [%s]",theURL.c());
#endif
	{
		bool aFail=false;
		String aCacheName=aFN;aFN+=".";aFN+=aTail;

		IOBuffer aBuffer;
		aBuffer.Download(theURL);

		if (aBuffer.Len()==0) aFail=true;
		else if (aBuffer.Len()<64) aFail=true;
		else if (aBuffer.mData[0]=='<') 
		{
			String aCheck=aBuffer.ToString(100);
			if (aCheck.ContainsI("<html>")) aFail=true;
			if (aCheck.ContainsI("::RML")) aFail=true;
		}

		if (!aFail)
		{
			aBuffer.CommitFile(aFN);
			IOBuffer aBuffer;aBuffer.WriteRaw(theVersion,theVersion.Len());aBuffer.CommitFile(aCacheVersionName);
			Load(aCacheName);
		}
	}
}

bool Image::LoadFromWebCache(String theURL, String theVersion)
{
	String aTail=theURL.GetTail('.');
	String aFN="cache://";
	aFN+=URLToFN(theURL);//.RemoveTail('.'));
							   //aFN+="_v";
							   //aFN+=theVersion;
	//aFN=PointAtCache(aFN);
	String aCacheVersionName=aFN;aCacheVersionName+=".";aCacheVersionName+=theVersion;

	if (DoesImageExist(aFN) && DoesFileExist(aCacheVersionName))
	{
		Load(aFN);
		return true;
	}

	return false;
}


void Image::MakeEdgeMask(Image& theResult, float theSize, float theAlphaThreshold)
{
	int aThreshold=_max(1,(int)((theAlphaThreshold)*255.0f));

	theResult.MakeBlank(mWidth,mHeight);
	theResult.Clear(Color(0,0,0,1));

	// Find all the edges...
	SmartList(IPoint) aEdgeList;
	for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
	{
		RGBA* aPix=GetPixelPtr(0,aSpanY);
		for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
		{
			if (aPix->mA<aThreshold)
			{
				if (
					GetPixelPtr(aSpanX-1,aSpanY-1)->mA>=aThreshold ||
					GetPixelPtr(aSpanX-0,aSpanY-1)->mA>=aThreshold ||
					GetPixelPtr(aSpanX+1,aSpanY-1)->mA>=aThreshold ||
					GetPixelPtr(aSpanX-1,aSpanY-0)->mA>=aThreshold ||
					GetPixelPtr(aSpanX+1,aSpanY-0)->mA>=aThreshold ||
					GetPixelPtr(aSpanX-1,aSpanY+1)->mA>=aThreshold ||
					GetPixelPtr(aSpanX-0,aSpanY+1)->mA>=aThreshold ||
					GetPixelPtr(aSpanX+1,aSpanY+1)->mA>=aThreshold
					) aEdgeList+=new IPoint(aSpanX,aSpanY);
			}
			aPix++;
		}
	}

	//
	// Distance to edge mask...
	//
	for (int aSpanY=0;aSpanY<mHeight;aSpanY++)
	{
		RGBA* aPix=GetPixelPtr(0,aSpanY);
		RGBA* aPlotPix=theResult.GetPixelPtr(0,aSpanY);
		for (int aSpanX=0;aSpanX<mWidth;aSpanX++)
		{
			if (aPix->mA>aThreshold)
			{
				float aBestDist=mWidth*mHeight*mWidth*mHeight;
				EnumSmartList(IPoint,aEdge,aEdgeList)
				{
					aBestDist=_min(aBestDist,gMath.DistanceSquared((float)aSpanX,(float)aSpanY,aEdge->mX,aEdge->mY));
					if (aBestDist<1*1) break;
				}

				
				aBestDist=_min(theSize,sqrt(aBestDist))/theSize;
				int aColor=(int)_min(255,(aBestDist*255));
				aPlotPix->mR=255;
				aPlotPix->mG=255;
				aPlotPix->mB=255;
				aPlotPix->mA=aColor;
			}

			aPix++;
			aPlotPix++;

		}
	}

}


int gPendingImageDownloads=0;
class ImageDownloader : public Object
{
public:
	IOBuffer mBuffer;
	Smart(Image) mImage;

	String mURL;
	String mCacheName_Full;
	String mCacheName;
	String mCacheVersionName;
	String mVersion;
	float* mProgress;
	bool mStarted=false;
	HOOKARGPTR mHook=NULL;
	void* mHookParam=NULL;

	ImageDownloader(String theURL, String theVersion, Smart(Image) theImage, float* theProgress, HOOKARGPTR theHook, void* theHookParam)
	{
		mImage=theImage;
		mImage->mFlag|=IMAGEFLAG_DOWNLOADING;
		mVersion=theVersion;
		mProgress=theProgress;
		mURL=theURL;
		mHook=theHook;
		mHookParam=theHookParam;

		String aTail=theURL.GetTail('.');
		String aFN="cache://";
		aFN+=URLToFN(theURL);
		//aFN=PointAtCache(aFN);
		mCacheVersionName=aFN;mCacheVersionName+=".";mCacheVersionName+=mVersion;

		mCacheName=aFN;aFN+=".";aFN+=aTail;
		mCacheName_Full=aFN;

#ifdef _DEBUG
		gOut.Out("Downloading Image = %s",theURL.c());
#endif
		//mBuffer.DownloadA(theURL,mProgress);
	}

	void Update()
	{
		if (!mStarted)
		{
			if (gPendingImageDownloads<3)
			{
				mBuffer.DownloadA(mURL,mProgress);
				gPendingImageDownloads++;
			}
			mStarted=true;
		}
		else if (!mBuffer.IsDownloading())
		{
			gPendingImageDownloads=_max(gPendingImageDownloads-1,0);

			mImage->mFlag|=(IMAGEFLAG_DOWNLOADING|IMAGEFLAG_ERROR);
			mImage->mFlag^=(IMAGEFLAG_DOWNLOADING|IMAGEFLAG_ERROR);

			if (!mBuffer.IsError())
			{
				bool aFail=false;
				if (mBuffer.Len()<64) aFail=true;
				else
				{
					if (mBuffer.mData[0]=='<')
					{
						String aCheck=mBuffer.ToString(100);
						if (aCheck.ContainsI("<html>")) aFail=true;
						if (aCheck.ContainsI("::RML")) aFail=true;
					}
				}

				if (!aFail)
				{
#ifdef _DEBUG
					gOut.Out("Commit Image: %s",mCacheName_Full.c());
					gOut.Out("Commit Version: %s -> [%s]",mCacheVersionName.c(),mVersion.c());
#endif
					mBuffer.CommitFile(mCacheName_Full);
					IOBuffer aBuffer;aBuffer.WriteString(mVersion);aBuffer.CommitFile(mCacheVersionName);
					mImage->Load(mCacheName);
				}
				else 
				{
					mImage->mFlag|=IMAGEFLAG_ERROR;
				}
			}

			if (mHook) mHook(mHookParam);
			Kill();
		}
	}
};

Smart(Image) DownloadImage(String theURL, String theVersion,float* theProgress, HOOKARGPTR theHook, void* theHookParam)
{
	Smart(Image) aImage=new Image;
	if (!aImage->LoadFromWebCache(theURL,theVersion)) gAppPtr->mBackgroundProcesses+=new ImageDownloader(theURL,theVersion,aImage,theProgress,theHook,theHookParam);
	return aImage;
}

#include "rapt_image_tinyjpg.h"

struct JPGStruct
{
	unsigned char* mPtr;
	int mLen=0;
}* gJPGStruct=NULL;

void JPGOut(unsigned char oneByte)
{
	*gJPGStruct->mPtr=oneByte;
	gJPGStruct->mPtr++;
	gJPGStruct->mLen++;
}

void Image::ExportJPG(String theFilename, unsigned char theQuality)
{
	unsigned char* aRGB=new unsigned char[mWidth*mHeight*3];
	unsigned char* aDPtr=aRGB;
	RGBA* aPix=GetBits();
	for (int aCount=0;aCount<mWidth*mHeight;aCount++)
	{
		*aDPtr=aPix->mR;aDPtr++;
		*aDPtr=aPix->mG;aDPtr++;
		*aDPtr=aPix->mB;aDPtr++;
		aPix++;
	}

	IOBuffer aBuffer;
	aBuffer.GuaranteeSize(mWidth*mHeight*10);

	JPGStruct aStruct;
	aStruct.mPtr=(unsigned char*)aBuffer.mData;
	gJPGStruct=&aStruct;

	TooJpeg::writeJpeg(JPGOut,aRGB,mWidth,mHeight,true,theQuality,false,NULL);
	
	delete [] aRGB;
	aBuffer.mDataLen=aStruct.mLen;
	aBuffer.CommitFile(theFilename);
}

#define MSF_GIF_IMPL
#include "rapt_image_tinygif.h"

void Image::ExportGIF(String theFilename)
{
	MsfGifState aGifState = {};
	msf_gif_begin(&aGifState, mWidth, mHeight);
	msf_gif_frame(&aGifState, (unsigned char*)mData, 0, 32, mWidth*4); // 0= milliseconds this frame, 32 = bit depth
	//msf_gif_frame(&aGifState, (unsigned char*)mData, 0, 32, mWidth*4); // Add another frame (animated gif)
	MsfGifResult aResult=msf_gif_end(&aGifState);

	IOBuffer aBuffer;
	aBuffer.WriteRaw(aResult.data,aResult.dataSize);
	aBuffer.CommitFile(theFilename);
	msf_gif_free(aResult);

/*
	int width = 480, height = 320, centisecondsPerFrame = 5, bitDepth = 16;
	MsfGifState gifState = {};
	// msf_gif_bgra_flag = true; //optionally, set this flag if your pixels are in BGRA format instead of RGBA
	// msf_gif_alpha_threshold = 128; //optionally, enable transparency (see documentation in header for details)
	msf_gif_begin(&gifState, width, height);
	msf_gif_frame(&gifState, ..., centisecondsPerFrame, bitDepth, width * 4); //frame 1
	msf_gif_frame(&gifState, ..., centisecondsPerFrame, bitDepth, width * 4); //frame 2
	msf_gif_frame(&gifState, ..., centisecondsPerFrame, bitDepth, width * 4); //frame 3, etc...
	MsfGifResult result = msf_gif_end(&gifState);
	if (result.data) {
		FILE * fp = fopen("MyGif.gif", "wb");
		fwrite(result.data, result.dataSize, 1, fp);
		fclose(fp);
	}
	msf_gif_free(result);
	*/
}

void Image::ExportGIF(String theFilename, Array<Image*> theGIFs, int theMillseconds)
{
	MsfGifState aGifState = {};
	msf_gif_begin(&aGifState, theGIFs[0]->mWidth, theGIFs[0]->mHeight);

	//
	// Crashing here?  All Images must be the same size!
	//

	for (int aCount=0;aCount<theGIFs.Size();aCount++) msf_gif_frame(&aGifState, (unsigned char*)theGIFs[aCount]->mData, theMillseconds, 32, theGIFs[0]->mWidth*4); // 0= milliseconds this frame, 32 = bit depth
	MsfGifResult aResult=msf_gif_end(&aGifState);

	IOBuffer aBuffer;
	aBuffer.WriteRaw(aResult.data,aResult.dataSize);
	aBuffer.CommitFile(theFilename);
	msf_gif_free(aResult);
}

void Image::ExportGIF(String theFilename, Array<Image*> theGIFs, Array<int> theMillseconds)
{
	MsfGifState aGifState = {};
	msf_gif_begin(&aGifState, theGIFs[0]->mWidth, theGIFs[0]->mHeight);

	//
	// Crashing here?  All Images must be the same size!
	//

	for (int aCount=0;aCount<theGIFs.Size();aCount++) msf_gif_frame(&aGifState, (unsigned char*)theGIFs[aCount]->mData, theMillseconds[aCount], 32, theGIFs[0]->mWidth*4); // 0= milliseconds this frame, 32 = bit depth
	MsfGifResult aResult=msf_gif_end(&aGifState);

	IOBuffer aBuffer;
	aBuffer.WriteRaw(aResult.data,aResult.dataSize);
	aBuffer.CommitFile(theFilename);
	msf_gif_free(aResult);
}

#ifdef _WIN32
#pragma warning(pop)
#endif
















