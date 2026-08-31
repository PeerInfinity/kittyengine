#pragma once

#include "rapt_point.h"
#include "rapt_string.h"
#include "rapt_color.h"
#include "rapt_rect.h"
#include "rapt_smartpointer.h"

enum
{
	IMAGEFLAG_DOWNLOADING=0x01,		// It's downloading
	IMAGEFLAG_ERROR=0x02,			// Something's wrong!
};

class RGBA;


class IOBuffer;
class Image
{
public:
	Image();
	virtual ~Image();

	bool				Load(String theFilename);
	void				LoadFromWeb(String theURL, String theVersion="1");
	bool				LoadFromWebCache(String theURL, String theVersion="1");


//	bool				Download(String theURL, String theVersion="1", String* returnLocalFN=NULL);	// Returns true if it's already cached, false if not!
	void				Reset();
	void				MakeBlank(int theWidth, int theHeight);
	inline void			MakeBlank(int theWidth, int theHeight,Color theColor) {MakeBlank(theWidth,theHeight);Clear(theColor);}
	bool				IsBlank(float theThreshold=0.0f);
	bool				IsBlank(Rect theRect,float theThreshold=0.0f);



	void				ExportPNG(String theFilename);
	void				ExportRAW(String theFilename);
	void				ExportJPG(String theFilename, unsigned char theQuality=90);
	void				ExportGIF(String theFilename);
	static void			ExportGIF(String theFilename, Array<Image*> theGIFs, int theMillseconds);			// Export an animated gif... Image::ExportGIF("filename.gif",{&aImage1,&aImage2,&aImage3},100);
	static void			ExportGIF(String theFilename, Array<Image*> theGIFs, Array<int> theMillseconds);	// Export an animated gif... Image::ExportGIF("filename.gif",{&aImage1,&aImage2,&aImage3},{100,50,200});
	
	//static void			ExportGIF(String theFilename, List& theGIFs, int theMillseconds);
	//static void			ExportGIF(String theFilename, List& theGIFs, Array<int> theMillseconds);

	Image*				MakeCopy() {Image* aImage=new Image;aImage->CopyFrom(*this);return aImage;}
	void				CopyFrom(Image &theSource);
	void				CopyTo(Image &theDestination);
	void				CopyFrom(Image &theSource, int theX, int theY, int theWidth, int theHeight);
	void				CopyTo(Image &theDestination, int theX, int theY, int theWidth, int theHeight);

	void				CopyFrom(Image *theSource) {if (theSource) CopyFrom(*theSource);}
	void				CopyTo(Image *theDestination) {if (theDestination) CopyTo(*theDestination);}
	void				CopyFrom(Image *theSource, int theX, int theY, int theWidth, int theHeight) {if (theSource) CopyFrom(*theSource,theX,theY,theWidth,theHeight);}
	void				CopyTo(Image *theDestination, int theX, int theY, int theWidth, int theHeight) {if (theDestination) CopyTo(*theDestination,theX,theY,theWidth,theHeight);}
	void				CopyFrom(Image *theSource, Rect theRect) {CopyFrom(theSource,(int)theRect.mX,(int)theRect.mY,(int)theRect.mWidth,(int)theRect.mHeight);}
	void				CopyTo(Image *theDestination, Rect theRect) {CopyTo(theDestination,(int)theRect.mX,(int)theRect.mY,(int)theRect.mWidth,(int)theRect.mHeight);}
	void				CopyFrom(Image *theSource, AABBRect theRect) {CopyFrom(theSource,(int)theRect.mX1,(int)theRect.mY1,(int)theRect.Width(),(int)theRect.Height());}
	void				CopyTo(Image *theDestination, AABBRect theRect) {CopyTo(theDestination,(int)theRect.mX1,(int)theRect.mY1,(int)theRect.Width(),(int)theRect.Height());}
	void				CopyFrom(Image &theSource, Rect theRect) {CopyFrom(theSource,(int)theRect.mX,(int)theRect.mY,(int)theRect.mWidth,(int)theRect.mHeight);}
	void				CopyTo(Image &theDestination, Rect theRect) {CopyTo(theDestination,(int)theRect.mX,(int)theRect.mY,(int)theRect.mWidth,(int)theRect.mHeight);}
	void				CopyFrom(Image &theSource, AABBRect theRect) {CopyFrom(theSource,(int)theRect.mX1,(int)theRect.mY1,(int)theRect.Width(),(int)theRect.Height());}
	void				CopyTo(Image &theDestination, AABBRect theRect) {CopyTo(theDestination,(int)theRect.mX1,(int)theRect.mY1,(int)theRect.Width(),(int)theRect.Height());}


	AABBRect			GetAABB() {return AABBRect(0,0,(float)mWidth-1,(float)mHeight-1);}



	void				StampImage(Image &theImage, int theXPos, int theYPos, Rect theRect,Color theColor);
	inline void			StampImage(Image &theImage, int theXPos, int theYPos, Color theColor=Color(1)) {StampImage(theImage,theXPos,theYPos,Rect(0,0,(float)theImage.mWidth,(float)theImage.mHeight),theColor);}



	void				Clear(float theR, float theG, float theB, float theA=1.0f);
	inline void			Clear(Color theColor) {Clear(theColor.mR,theColor.mG,theColor.mB,theColor.mA);}



	Rect				GetBounds();
	Rect				GetBoundsFromColor(Color theBackgroundColor);
	void				Crop(int theX, int theY, int theWidth, int theHeight);
	inline void			Crop(Rect theRect) {Crop((int)theRect.mX,(int)theRect.mY,(int)theRect.mWidth,(int)theRect.mHeight);}
	inline void			Crop() {Rect aRect=GetBounds();Crop(aRect);}
	inline void			Trim() {Crop();}


	void				AddBorder(int theSize);
	void				MakeTilingBorder();

	void				Resize(int theNewWidth, int theNewHeight, Image *theImage=NULL);
	inline void			Resize(float thePercent, Image *theImage=NULL) {Resize((int)((mWidth*thePercent)+.5f),(int)((mHeight*thePercent)+.5f),theImage);}

	void				Resize_Lancoz(int theNewWidth, int theNewHeight, Image *theImage=NULL);
	inline void			Resize_Lancoz(float thePercent, Image *theImage=NULL) {Resize_Lancoz((int)((mWidth*thePercent)+.5f),(int)((mHeight*thePercent)+.5f),theImage);}

	void				MakePow2(bool blankEdges=false);	// Increases the image's canvas size to pow2

	//
	// If it encounters this color, the alpha of that pixel becomes zero.
	//
	void				EraseColor(Color theColor);

	//
	// Turns another image's rgb into our alpha...
	//
	void				ImageToAlpha(Image& theImage, bool clearZeroAlpha=false);
	void				ImageToAlpha(Image& theImage, Rect theRect, bool clearZeroAlpha=false);

	//
	// Rotates our image 90 degrees
	//
	void				RotateCW();
	void				FlipH();
	void				FlipV();

	//
	// Gaussian Blur
	//
	void				GuassianBlur(int theRadius, bool isWrapped=false);

	//
	// Bleeds pixel color into adjacent transparent pixels, so that
	// filtering/blurring is clean (Alpha does not bleed, only color)
	//
	void				Bleed(int theSize, char theAlpha);

	//
	// Premultiplies the alpha into the rgb...
	//
	void				PremultiplyAlpha();

	//
	// Export to MACHO format...
	//
	void				LoadThumb(String theFilename);
	void				LoadThumb(IOBuffer& theBuffer);
	void				ExportThumb(String theFilename,Array<Color> &thePalette);
	void				ExportThumb(IOBuffer& theBuffer,Array<Color> &thePalette);

	void				LoadPThumb(String theFilename,Array<Color>& thePalette);
	void				LoadPThumb(IOBuffer& theBuffer,Array<Color>& thePalette);
	void				ExportPThumb(String theFilename,Array<Color>& thePalette);
	void				ExportPThumb(IOBuffer& theBuffer,Array<Color>& thePalette);

	//
	// Turns our image into a 256-color palette
	//
	void				Paletteize(Array<Color>& thePalette);

	//
	// Swaps the R&B components of the image.  This is for kludging stuff on Mac.  I put it in because the Hoggy thumbnails
	//
	void				SwapRB();


	//
	// Sees if it's in the process of downloading...
	//
	inline bool			IsDownloading() {return ((mFlag&IMAGEFLAG_DOWNLOADING)!=0);}

public:

	int					mWidth;
	int					mHeight;
	char				mFlag=0;

	RGBA				*mData;
	inline RGBA			*GetBits() {return mData;}
	RGBA				*GetPixelPtr(int x=0, int y=0);
	unsigned char&		GetR(int x, int y);
	unsigned char&		GetG(int x, int y);
	unsigned char&		GetB(int x, int y);
	unsigned char&		GetA(int x, int y);

	void				SetNullPixelColor(Color theColor);

	inline void			ReplacePixels(int theWidth, int theHeight, void* theNew)	{Reset();mData=(RGBA*)theNew;mWidth=theWidth;mHeight=theHeight;}

public:

	//
	// Extra helper functions 
	// These are NOT fast.  They're utilitarian, for prototyping or for non-speed intensive work.
	//
	void				SetPixel(int theXPos, int theYPos, Color theColor);
	void				BresenhamLine(int x1,int y1,int x2,int y2, Color theColor);
	void				FloodFill(int theXPos, int theYPos, Color theFillColor);
	void				FillRect(int theXPos, int theYPos, int theWidth, int theHeight, Color theColor);
	void				FillRect(Rect theRect, Color theColor) {FillRect((int)theRect.mX,(int)theRect.mY,(int)theRect.mWidth,(int)theRect.mHeight,theColor);}
	void				MakeTileSafe(Rect theRect); // Bleeds a rect out so it's texture safe...

	void				MakeEdgeMask(Image& theResult, float theSize, float theAlphaThreshold=.5f);
};

#ifndef HOOKARG
#define HOOKARG(func) [&](void* theArg) func
#define HOOKARGG(func) [](void* theArg) func
#define HOOKARGPTR std::function<void(void* theArg)>
#endif

bool DoesImageExist(String theFilename);
bool DoesImageExist(String theFilename, String theVersion);
Smart(Image) DownloadImage(String theURL, String theVersion="1", float* theProgress=NULL, HOOKARGPTR theHook=NULL, void* theHookParam=NULL);
