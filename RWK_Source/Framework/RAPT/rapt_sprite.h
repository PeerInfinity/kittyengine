#pragma once

#include "util_core.h"
#include "rapt_string.h"
#include "rapt_point.h"
#include "rapt_color.h"
#include "rapt_rect.h"
#include "rapt_matrix.h"
#include "rapt_object.h"

#ifdef _DEBUG
//#define DEBUGBUNDLES
#endif

//
// Billboard levels is how many transparency levels to pre-cache for billboards.
// You want this to be as low as you can!  The only time you'd need higher would be
// if you had something that fades away reaaaaaally slowly...
//

class IOBuffer;
class Image;
class Sprite;
class SpriteBundle 
{
public:
	SpriteBundle();
	~SpriteBundle();

	virtual void			Load() {}
	bool					Load(String theBundleName, bool setIsLoaded=true);
	bool					LoadManual(String theFilename);
	void					Unload(bool doReset=false);
	bool					LoadDyna(String theDynaName, bool setIsLoaded=true);
	bool					LoadDyna(IOBuffer& theDynaBuffer, Image& theImage, bool setIsLoaded=true);
	bool					LoadDynaDataOnly(String theDynaName);
	void					UnloadDynaDataOnly();
    
	void					OverrideTextureSize(int theX, int theY);
	void					OverrideTextureSize();

	void					DumpDyna();
	bool					DoesDynaExist(char* theDyna);
	IOBuffer&				PointAtDyna(char* theDyna);
	struct DynaData
	{
		String				mName;
		int					mSeek;
	};
	SmartList(DynaData)		mDynaList;
	bool					GetDynaFile(char* theDyna, IOBuffer& theBuffer);

	//
	// In some cases (put in for Chuzzle 2), we want the names of all our sprites, with a pointer to
	// their direct sprite, for in-game fetching.  This handles that.
	//
	struct NameToSprite
	{
		String				mName;
		Sprite*				mSprite;
	};
	SmartList(NameToSprite)	mNameToSpriteList;
	Sprite*					GetSpriteByName(String theName);
	void					AddSpriteName(String theName, Sprite* theSprite) {Smart(NameToSprite) aNTS=new NameToSprite;aNTS->mName=theName;aNTS->mSprite=theSprite;mNameToSpriteList+=aNTS;}
	void					ClearSpriteNames() {mNameToSpriteList.Clear();}
    
    //
    // Lets us know if a retina version is around (for Apple
    // compatibility)
    //
    bool                    IsRetina(String theName);

	//
	// For loading threaded
	//
	void					LoadThreaded();
	static void				LoadThreadedStub(void* theArg);


public:
	int						mLoadCount;

	Array<int>				mTextureList;
	Array<IPoint>			mTextureSizeList;
	inline int				GetTextureRef(int theNumber) {return mTextureList[theNumber];}
	inline int				GetTexture(int theNumber=0) {return mTextureList[theNumber];}
	IPoint					GetSize(int theTexNumber=0);
	Point					GetSizeF(int theTexNumber=0) {return GetSize(theTexNumber);}

	IOBuffer*				mData;
	void					LoadData(String theBundleName);
	void					LoadDataManual(String theFileName);
	void					UnloadData();

	

	bool					mIsLoaded;
	inline bool				IsLoaded() {return mIsLoaded;}

	bool					mIsInLoadingThread;
	bool					mIsThreadLoading;
	inline bool				IsThreadLoading() {return mIsThreadLoading;}
			

#ifdef DEBUGBUNDLES
	String					mBundleName;
#endif
};

class Sprite
{
public:
	Sprite();
	virtual ~Sprite();

	//
	// Loads a sprite manually
	//
	//void					ManualLoad(int theTexture, Rect theRect, int theWidth, int theHeight, float theXMove, float theYMove);
	void					ManualLoad(int theTexture, Rect theRect, int theWidth, int theHeight, float theDrawWidth, float theDrawHeight, float theXMove, float theYMove, bool doRotate=false);
	void					ManualLoad(int theTexture, IOBuffer& theBuffer);	// Loads from a stream, with data in same order as call above
	void					ManualLoad(Sprite& theSprite);

	//
	// Next version bundle load...
	//
	void					BundleLoad(short theVersion, Array<int>& theTextureList, IOBuffer& theBuffer);	// Loads from a stream, with data in same order as call above

	virtual void			Load(String theFilename, int theImageMode=8888, bool setIniitalized=true);
	virtual void			Load(Image& theImage, int theImageMode=8888, bool setIniitalized=true);
	virtual void			Unload();
	virtual bool			GoLive(bool doThread=true);
		
	//
	// Draw calls
	//
	virtual void			DrawCore(Quad& theXY, Quad& theUV);
	virtual void			DrawCore(Quad3D &theXY, Quad &theUV);

	void					Draw(float theX, float theY);
	inline void				Draw(Point thePos) {Draw(thePos.mX,thePos.mY);}
	void					Draw(Matrix &theMatrix);
	void					Draw(float theX, float theY, Quad theQuad);
	void					Draw(Quad &theDrawQuad, Quad &theTextureQuad);
	void					Draw(Quad &theDrawQuad, Quad &theTextureQuad, Color aC1, Color aC2, Color aC3, Color aC4);
	inline void				Draw(Quad &theDrawQuad) {Draw(theDrawQuad,mTextureQuad);}
	void					Draw(Quad3D &theDrawQuad);
	void					Draw(Rect theRect);
	void					Center(float theX, float theY);
	inline void				Center(Point thePos) {Center(thePos.mX,thePos.mY);}
	inline void				Center(Rect theRect) {Center(theRect.CenterX(),theRect.CenterY());}
	inline void				Center(Rect* theRect) {Center(theRect->CenterX(),theRect->CenterY());}
	void					Draw(Matrix &theMatrix, Matrix &theTextureMatrix);

	void					Center(Vector thePos);
	void					DrawScaled(Vector thePos, float theScale);


	//
	// BillBoard is accurate for color, etc...
	// BillBoardFast should be used only if not colorized, and it will cache gG.mBillboardLevels
	// of transparency shapes.  Use for white particles with transparency.
	//
	/*
	void					BillBoard(Vector thePos, float theRotation=0.0f, Point theScale=Point(1,1));
	void					BillBoardMT(Vector thePos, float theRotation=0.0f, Point theScale=Point(1,1));
	//void					BillBoardFast(Vector thePos, float theRotation=0.0f, float theScale=1.0f);
	void					BillBoardFast(Vector thePos, float theRotation=0.0f, Point theScale=Point(1,1));
	void					BillBoardFastMT(Vector thePos, float theRotation=0.0f, Point theScale=Point(1,1));

	inline void				BillBoard(Vector thePos, float theRotation, float theScale) {BillBoard(thePos,theRotation,Point(theScale,theScale));}
	inline void				BillBoardFast(Vector thePos, float theRotation, float theScale) {BillBoardFast(thePos,theRotation,Point(theScale,theScale));}
	inline void				BillBoardFastMT(Vector thePos, float theRotation, float theScale) {BillBoardFastMT(thePos,theRotation,Point(theScale,theScale));}
	*/

	void					DrawFlipped(float theX, float theY, float flipX, float flipY);
	inline void				DrawFlipped(Point thePos,float flipX, float flipY) {DrawFlipped(thePos.mX,thePos.mY,flipX,flipY);}
	inline void				CenterFlipped(float theX, float theY, float flipX, float flipY) {DrawFlipped(theX,theY,flipX,flipY);}
	inline void				CenterFlipped(Point thePos,float flipX, float flipY) {DrawFlipped(thePos.mX,thePos.mY,flipX,flipY);}

	
	void					DrawScaled(float theX, float theY, Point theScale);
	void					DrawScaled(float theX, float theY, float theScale);
	inline void				DrawScaled(Point thePos, Point theScale) {DrawScaled(thePos.mX,thePos.mY,theScale);}
	inline void				DrawScaled(Point thePos, float theScale) {DrawScaled(thePos.mX,thePos.mY,theScale);}
	void					DrawRotated(float theX, float theY, float theRotation);
	inline void				DrawRotated(Point thePos, float theRotation) {DrawRotated(thePos.mX,thePos.mY,theRotation);}
	void					DrawRotatedScaled(float theX, float theY, float theRotation, Point theScale);
	void					DrawRotatedScaled(float theX, float theY, float theRotation, float theScale);
	inline void				DrawRotatedScaled(Point thePos, float theRotation, Point theScale) {DrawRotatedScaled(thePos.mX,thePos.mY,theRotation,theScale);}
	inline void				DrawRotatedScaled(Point thePos, float theRotation, float theScale) {DrawRotatedScaled(thePos.mX,thePos.mY,theRotation,theScale);}

	//
	// Partial draw calls
	//
	void					DrawSegment(float theX, float theY, float theStartX, float theStartY, float theEndX, float theEndY);
	void					DrawSegment(float theX, float theY, Quad &theDrawQuad, Quad &theTextureQuad);
	inline void				DrawSegment(float theX, float theY, QuadPair &theQuadPair) {DrawSegment(theX,theY,theQuadPair.mQuad[0],theQuadPair.mQuad[1]);}
	void					DrawSegmentH(float theX, float theY, float theStart, float theEnd);
	void					DrawSegmentV(float theX, float theY, float theStart, float theEnd);

	//
	// Lets you get a segment of a sprite...
	//
	QuadPair				GetSegment(float theStartX, float theStartY, float theEndX, float theEndY);


	//
	// Telescoped calls... draws the sprite gridded, repeating the middle
	//
	void					DrawTelescoped(float theX, float theY, float theWidth, float theHeight, bool ignoreCenter=false);
	void					DrawTelescopedWithFill(float theX, float theY, float theWidth, float theHeight, Color theCenterFillColor);
	void					DrawTelescopedH(float theX, float theY, float theWidth);
	void					DrawTelescopedV(float theX, float theY, float theHeight);
	inline void				DrawTelescopedH(Point thePos, float theWidth) {DrawTelescopedH(thePos.mX,thePos.mY,theWidth);}
	inline void				DrawTelescopedV(Point thePos, float theHeight) {DrawTelescopedH(thePos.mX,thePos.mY,theHeight);}
	inline void				DrawTelescoped(Point thePos, float theWidth, float theHeight, bool ignoreCenter=false) {DrawTelescoped(thePos.mX,thePos.mY,theWidth,theHeight,ignoreCenter);}
	inline void				DrawTelescoped(Rect theRect, bool ignoreCenter=false) {DrawTelescoped(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight,ignoreCenter);}
	void					DrawTelescopedCorners(float theX, float theY, float theWidth, float theHeight, Color theCenterFillColor);

	//
	// Magic Border... this assumes the sprite is the upper left corner of something, for which the middle is simply the
	// edges of it, repeated.
	//
	void					DrawMagicBorder(float theX, float theY, float theWidth, float theHeight, bool doFill=false);
	inline void				DrawMagicBorder(Rect theRect, bool doFill=false) {DrawMagicBorder(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight,doFill);}
	void					DrawMagicBorderH(float theX, float theY, float theWidth);
	void					DrawMagicBorderH(Point thePos, float theWidth) {DrawMagicBorderH(thePos.mX,thePos.mY,theWidth);}
	inline void				DrawMagicBorderH(Rect theRect) {DrawMagicBorderH(theRect.mX,theRect.mY,theRect.mWidth);}
	void					DrawMagicBorderH2(float theX, float theY, float theWidth);
	void					DrawMagicBorderH2(Point thePos, float theWidth) {DrawMagicBorderH2(thePos.mX,thePos.mY,theWidth);}
	inline void				DrawMagicBorderH2(Rect theRect) {DrawMagicBorderH2(theRect.mX,theRect.mY,theRect.mWidth);}

	void					DrawMagicBorderV(float theX, float theY, float theWidth);
	inline void				DrawMagicBorderV(Point thePos, float theHeight) {DrawMagicBorderV(thePos.mX,thePos.mY,theHeight);}


	//
	// Extended MagicBorder draws that allow you to provide different sprite formats or tweakings...
	//
	void					DrawMagicBorderEX_LeftRight(float theX, float theY, float theWidth, float theHeight, bool doFill=false);
	void					DrawMagicBorderEX_LeftRight_TopOnly(float theX, float theY, float theWidth, float theHeight, bool doFill=false);
	void					DrawMagicBorderEX_LeftRight_BottomOnly(float theX, float theY, float theWidth, float theHeight, bool doFill=false);
	void					DrawMagicBorderEX_AllCorners(float theX, float theY, float theWidth, float theHeight, bool doFill=false);
	inline void				DrawMagicBorderEX_AllCorners(Rect theRect, bool doFill=false) {DrawMagicBorderEX_AllCorners(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight,doFill);}
		

	//
	// Chain calls... just draws n sprites repeated.  
	//
	void					DrawTiledH(float theX, float theY, int theRepeats);
	inline void				DrawTiledH(Point thePos, int theRepeats) {DrawTiledH(thePos.mX,thePos.mY,theRepeats);}
	inline void				DrawTiledH(IPoint thePos, int theRepeats) {DrawTiledH((float)thePos.mX,(float)thePos.mY,theRepeats);}
	void					DrawTiledV(float theX, float theY, int theRepeats);
	inline void				DrawTiledV(Point thePos, int theRepeats) {DrawTiledV(thePos.mX,thePos.mY,theRepeats);}
	inline void				DrawTiledV(IPoint thePos, int theRepeats) {DrawTiledV((float)thePos.mX,(float)thePos.mY,theRepeats);}
	void					DrawTiled(float theX, float theY, int theRepeatX, int theRepeatY);
	inline void				DrawTiled(Point thePos, int theRepeatX, int theRepeatY) {DrawTiled(thePos.mX,thePos.mY,theRepeatX,theRepeatY);}
	void					DrawTiledScaled(float theX, float theY, int theRepeatX, int theRepeatY, float theScale);
	inline void				DrawTiledScaled(Point thePos, int theRepeatX, int theRepeatY, float theScale) {DrawTiledScaled(thePos.mX,thePos.mY,theRepeatX,theRepeatY,theScale);}

	void					FillRect(float theX, float theY, float theWidth, float theHeight,float theScale=1.0f);
	inline void				FillRect(Rect theRect, float theScale=1.0f) {FillRect(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight,theScale);}

	//
	// Draw Radial: Both values must be between -360 and +720
	//
	void					DrawRadial(float theX, float theY, float theStartAngle, float theEndAngle);
	inline void				DrawRadial(Point theCenter, float theStartAngle, float theEndAngle) {DrawRadial(theCenter.mX,theCenter.mY,theStartAngle,theEndAngle);}
	void					DrawRadialScaled(float theX, float theY, float theStartAngle, float theEndAngle, float theScale);
	inline void				DrawRadialScaled(Point theCenter, float theStartAngle, float theEndAngle, float theScale) {DrawRadialScaled(theCenter.mX,theCenter.mY,theStartAngle,theEndAngle, theScale);}

	//
	// This only works in 45-degree slices-- i.e. it won't handle corners...
	//
	void					DrawPieSlice45(float theX, float theY, float theStartAngle, float theEndAngle);
	inline void				DrawPieSlice45(Point theCenter, float theStartAngle, float theEndAngle) {DrawPieSlice45(theCenter.mX,theCenter.mY,theStartAngle,theEndAngle);}
	void					DrawPieSlice45Scaled(float theX, float theY, float theStartAngle, float theEndAngle, float theScale);
	inline void				DrawPieSlice45Scaled(Point theCenter, float theStartAngle, float theEndAngle, float theScale) {DrawPieSlice45Scaled(theCenter.mX,theCenter.mY,theStartAngle,theEndAngle,theScale);}

	//
	// Lets us draw warped, with an array of warped points
	// Warpsize is how many points in the grid.
	//
	void					DrawWarped(IPoint theWarpSize, Point *theXYPoints, Point* theUVPoints, Color *theColorPoints=NULL);
	inline void				DrawWarped(IPoint theWarpSize, Array<Point>& theXYPoints, Array<Point>& theUVPoints) {DrawWarped(theWarpSize,theXYPoints.mArray,theUVPoints.mArray);}
	inline void				DrawWarped(IPoint theWarpSize, Array<Point>& theXYPoints, Array<Point>& theUVPoints, Array<Color>& theColorPoints) {DrawWarped(theWarpSize,theXYPoints.mArray,theUVPoints.mArray,theColorPoints.mArray);}
	void					MakeWarpArray(IPoint theWarpSize, Array<Point>& theXYArray, Array<Point>& theUVArray);
	// REMMEBER: MakeWarpArray, you request number of POINTS, so you need at least 2 in a direction

	//
	// Skewed draws.  Pick % of skew.
	//
	void					DrawTweaked(Point thePos, Point tweakUL=Point(0,0), Point tweakUR=Point(0,0), Point tweakLL=Point(0,0), Point tweakLR=Point(0,0));

	//
	// Copy another sprite as a reference to our sprite
	//
	void					Copy(Sprite* theOtherSprite);

public:
	bool					mIsInitialized;

	int						mTexture;
	bool					mOwnTexture;
	bool					mOwnShape;
	CleanArray<int>			mShape;
	CleanArray<int>			mShapeMT;

	//
	// Info for drawing...
	//
	Quad					mDrawQuad;
	Quad					mTextureQuad;
	Rect*					mTextureRect=NULL;
	inline Quad&			GetDrawQuad() {return mDrawQuad;}
	inline Quad3D			GetDrawQuad3D()
	{
		Quad3D aResult;
		aResult.mCorner[0]=mDrawQuad.mCorner[0];
		aResult.mCorner[1]=mDrawQuad.mCorner[1];
		aResult.mCorner[2]=mDrawQuad.mCorner[2];
		aResult.mCorner[3]=mDrawQuad.mCorner[3];
		return aResult;
	}
	inline Quad&			GetTextureQuad() {return mTextureQuad;}
	inline Rect&			GetTextureRect() {if (!mTextureRect) mTextureRect=new Rect(mTextureQuad.GetBounds());return *mTextureRect;}
	inline int				GetTexture() {return mTexture;}

	Point					mDrawQuadCenter;
	inline Point&			GetDrawCenter() {return mDrawQuadCenter;}

	Rect					mBounds;
	inline Rect&			GetBounds() {return mBounds;}

	Rect					mOriginalBounds;	// Bounds of the uncropped sprite

	//
	// This is the width/height of the source
	// image that was put into the bundle
	//
	int						mWidth;
	int						mHeight;
	inline int				GetWidth() {return mWidth;}
	inline int				GetHeight() {return mHeight;}
	inline float			GetWidthF() {return (float)mWidth;}
	inline float			GetHeightF() {return (float)mHeight;}
	inline int				GetHalfWidth() {return mWidth/2;}
	inline int				GetHalfHeight() {return mHeight/2;}
	inline float			GetHalfWidthF() {return (float)mWidth/2;}
	inline float			GetHalfHeightF() {return (float)mHeight/2;}
	inline int				Width() {return mWidth;}
	inline int				Height() {return mHeight;}
	inline float			WidthF() {return (float)mWidth;}
	inline float			HeightF() {return (float)mHeight;}
	inline int				HalfWidth() {return mWidth/2;}
	inline int				HalfHeight() {return mHeight/2;}
	inline float			HalfWidthF() {return (float)mWidth/2;}
	inline float			HalfHeightF() {return (float)mHeight/2;}
	inline Point			GetSize() {return Point(GetWidth(),GetHeight());}
	inline Point			GetHalfSize() {return Point(GetWidth()/2,GetHeight()/2);}
	inline Point			HalfSize() {return Point(GetWidth()/2,GetHeight()/2);}

	//
	// This is the width/height of the actual drawing area (as determined by the draw quad)
	//
	float					mDrawQuadWidth;
	float					mDrawQuadHeight;
	inline float			DrawQuadWidth() {return mDrawQuadWidth;}
	inline float			DrawQuadHeight() {return mDrawQuadHeight;}
	inline float			HalfDrawQuadWidth() {return mDrawQuadWidth/2;}
	inline float			HalfDrawQuadHeight() {return mDrawQuadHeight/2;}
	inline float			GetDrawQuadWidth() {return mDrawQuadWidth;}
	inline float			GetDrawQuadHeight() {return mDrawQuadHeight;}
	inline float			GetHalfDrawQuadWidth() {return mDrawQuadWidth/2;}
	inline float			GetHalfDrawQuadHeight() {return mDrawQuadHeight/2;}


	//
	// These are image keys (if any).
	// They just reference important coordinates in the image.
	//
	Array<Point>			mKey;
	inline Point			GetKey(int theKey=0) {return mKey[theKey];}
	
	//
	// This is the amount to move a texel 
	// anything along the lines of tweaking UV
	//
	Point					mTexelSize;		// Actual size of a texel
	Point					mTexelMove;		// How much to MOVE a texel to get it off an edge (iOS Kludge)
	inline Point			GetTexelSize() {return mTexelSize;}
	//
	// This converts UV coordinates from the original texture size to the matching coordinate in cropped sprite space.
	// So if you had a sprite that had UV of 0,0-1,1 but got cropped, this will convert the UV to the new coordinates.
	// Note that this will NOT work if a UV set includes blank space-- that space has been cropped off and you'll get
	// bits from the rest of the texture atlas.
	//
	void					ConvertUVCoordinate(float& theU, float& theV);

	//
	// If this sprite is pending completion (i.e. from a download or whatever reason)
	//
	bool					mPending;

};

class RenderToSprite : public Sprite
{
public:
	RenderToSprite();
	virtual ~RenderToSprite();
	//
	// This is pulled out because on some systems, we need TWO
	// variables to do render to texture.  So in DirectX, mFrameBuffer and
	// mTexture will be the same.  In OpenGL systems, mFrameBuffer will
	// be the frame buffer.
	//
#ifdef LEGACY_GL
	void					Go(int theWidth, int theHeight, bool wantAlpha=true, bool wantReadable=false);
#else
	void					Go(int theWidth, int theHeight, longlong theFormat=8888, bool wantReadable=false, bool useFullTexture=false); // UseFullTexture means, if we ask for 125,125 and card gives us 128,128, it'll make that the width/height and not the requested info
	void					Go(int theWidth, int theHeight, char* theFormat, bool wantReadable=false, bool useFullTexture=false);
#endif
	void					Stop();
	bool					GoLive(bool doThread=true);

	void					MakeTile();

	void					Export(Image& theImage, bool swapRB=false);
	void					Import(Image& theImage);

	void					ExportGIF(String theFN, bool swapRB=false);
	void					ExportJPG(String theFN, bool swapRB=false);
	void					ExportPNG(String theFN, bool swapRB=false);
	void					Import(String theFN);

};

//
// A renderto sprite that has a simple shader applied to it.
// Good for application of stuff that has like one shader on it or something.
// Used this to make the animated caustics that get applied to the distance fog in hamsterball.
//
#ifndef LEGACY_GL
class Shader;
class Vx2D;
class ShaderSprite : public Sprite // Sprite that just draws as a shader in a square.
{
public:
	ShaderSprite() {mTextureQuad.FromRect(Rect(0,0,1,1));}
	virtual ~ShaderSprite();

	Shader*					mShader=NULL;
	bool					mOwnShader=false;

	inline void				SetSize(float theWidth, float theHeight) {mWidth=(int)theWidth;mHeight=(int)theHeight;float aW2=theWidth/2;float aH2=theHeight/2;mDrawQuad.mCorner[0]=Point(-aW2,-aH2);mDrawQuad.mCorner[1]=Point(aW2,-aH2);mDrawQuad.mCorner[2]=Point(-aW2,aH2);mDrawQuad.mCorner[3]=Point(aW2,aH2);}
	inline void				SetSize(float theSize) {SetSize(theSize,theSize);}
	inline void				SetSize(Rect theRect) {mDrawQuad.FromRect(theRect);mWidth=(int)theRect.mW;mHeight=(int)theRect.mH;}

	inline void				Go(Shader* theShader, float theWidth=1.0f, float theHeight=1.0f) {mShader=theShader;SetSize(theWidth,theHeight);}
	void					Go(String theShader, float theWidth=1.0f, float theHeight=1.0f);

	virtual void			DrawCore(Quad& theXY, Quad& theUV);
	virtual void			DrawCore(Quad3D &theXY, Quad &theUV);

	static Vx2D				mV[4];

	bool					GoLive(bool doThread) {if (mShader) return true;return false;}
};

class ShaderRenderToSprite : public RenderToSprite // RenderToSprite that has a Render() option to fill the whole "page" with a shader.
{
public:
	ShaderRenderToSprite();
	virtual~ ShaderRenderToSprite();

	void					Go(int theWidth, int theHeight, Shader* theShader, longlong theFormat=8888, bool wantReadable=false) {RenderToSprite::Go(theWidth,theHeight,theFormat,wantReadable);mShader=theShader;}
	void					Go(int theWidth, int theHeight, Shader* theShader, char* theFormat, bool wantReadable=false) {RenderToSprite::Go(theWidth,theHeight,theFormat,wantReadable);mShader=theShader;}
	void					Go(int theWidth, int theHeight, String theShader, longlong theFormat=8888, bool wantReadable=false);
	void					Go(int theWidth, int theHeight, String theShader, char* theFormat, bool wantReadable=false);

	bool					GoLive(bool doThread=true);
	virtual void			Render();

	Shader*					mShader=NULL;
	bool					mOwnShader=false;
	bool					mAlwaysRender=true; // Set to false if you want to render only once, or at your own discretion.
};
#endif

class DynamicSprite : public Sprite
{
public:
	DynamicSprite()
	{
		mLiveCount=0;
		mLiveCountMax=100;
		mImageMode=8888;
	}

	virtual ~DynamicSprite()
	{
		if (mHookedTo) *mHookedTo=NULL;
		mHookedTo=NULL;
		while (mFlags&DYNAMIC_STUBBING) {OS_Core::Sleep(1);}
	}

	//
	// Just forces texture mode into Clamp and restores it...
	//
	void Prep();
	void Unprep();

	//
	// Bullshit... we have to duplicate these here or else the Microsoft Compiler loses its
	// mind and can't figure out that we want Sprite::Draw and not Object::Draw-- EVEN THOUGH OBJECT::DRAW TAKES NO PARAMS!
	//
	inline void				Draw(float theX, float theY) {Prep();Sprite::Draw(theX,theY);Unprep();}
	inline void				Draw(Point thePos) {Prep();Sprite::Draw(thePos.mX,thePos.mY);Unprep();}
	inline void				Draw(Matrix &theMatrix) {Prep();Sprite::Draw(theMatrix);Unprep();}
	inline void				Draw(float theX, float theY, Quad theQuad) {Prep();Sprite::Draw(theX,theY,theQuad);Unprep();}
	inline void				Draw(Quad &theDrawQuad, Quad &theTextureQuad) {Prep();Sprite::Draw(theDrawQuad,theTextureQuad);Unprep();}
	inline void				Draw(Quad &theDrawQuad, Quad &theTextureQuad, Color aC1, Color aC2, Color aC3, Color aC4) {Prep();Sprite::Draw(theDrawQuad,theTextureQuad,aC1,aC2,aC3,aC4);Unprep();}
	inline void				Draw(Quad &theDrawQuad) {Prep();Sprite::Draw(theDrawQuad);Unprep();}
	inline void				Draw(Rect& theRect) {Prep();Sprite::Draw(theRect);Unprep();}

	inline float			DrawQuadWidth() {return mDrawQuadWidth;}
	inline float			DrawQuadHeight() {return mDrawQuadHeight;}
	inline float			HalfDrawQuadWidth() {return mDrawQuadWidth/2;}
	inline float			HalfDrawQuadHeight() {return mDrawQuadHeight/2;}
	inline float			GetDrawQuadWidth() {return mDrawQuadWidth;}
	inline float			GetDrawQuadHeight() {return mDrawQuadHeight;}
	inline float			GetHalfDrawQuadWidth() {return mDrawQuadWidth/2;}
	inline float			GetHalfDrawQuadHeight() {return mDrawQuadHeight/2;}

	void					ManualLoad(IOBuffer& theBuffer);
	void					Load(String theFilename, int theImageMode=8888) {mFilename=theFilename;mImageMode=theImageMode;}
	void					FetchImage(bool doThread=true);
	virtual bool			GoLive(bool doThread=true);
	void					Update();

	static void				LoadStub(void* theArg);

public:
	String					mFilename;
	int						mImageMode;
	char					mFlags=0;		// Various flags
	enum
	{
		DYNAMIC_NOWATCH=0x01,
		DYNAMIC_STUBBING=0x02,	// Means we are in the middle of a load stub...
	};

	inline void				NoWatch() {mFlags|=DYNAMIC_NOWATCH;}

	short					mLiveCount;			// When this hits 0, we unload
	short					mLiveCountMax;		// This is how long we keep it live...

	//Point					mDQOffset;			// Offset of the drawquad after load...
	Quad					mNewDQ;				// New Draw Quad after load
	Quad					mNewTQ;				// New texture quad after load

	DynamicSprite**			mHookedTo=NULL;		// Hooked to variable (this is the variable in the SpriteUnloadWatcher, in case the sprite goes away before the watcher does)

	static int				mStubbingCount;		// How many are stubbing simultaneously
	static int				mStubbingMax;		// How many are allowed to stub simultaneously (saw SDL croak on too many)
};

#ifndef HOOKARG
#define HOOKARG(func) [&](void* theArg) func
#define HOOKARGG(func) [](void* theArg) func
#define HOOKARGPTR std::function<void(void* theArg)>
#endif

Smart(Sprite) DownloadSprite(String theURL, String theVersion="1", float* theProgress=NULL, HOOKARGPTR theHook=NULL, void* theHookParam=NULL);
Smart(DynamicSprite) DownloadDynamicSprite(String theURL, String theVersion="1", float* theProgress=NULL, HOOKARGPTR theHook=NULL, void* theHookParam=NULL);

//
// A pool of dynamic sprites that you can just load from anywhere and forget about.  This was designed for accessing custom content, like
// Makermall's thumbs and other web graphics.  These will unload if not drawn, and provide a "blank" object that can be drawn until it gets loaded.
//
namespace RComm{class RaptisoftQuery;}
class DynamicSpritePool : public Object
{
public:
	DynamicSpritePool();
	virtual ~DynamicSpritePool();

	void						Update();
	Smart(DynamicSprite)		GetSprite(String theFN, String theVersion="");

	struct DSInfo
	{
		DSInfo(String theFN,Smart(DynamicSprite) theSprite)
		{
			mSprite=theSprite;
			mSprite->NoWatch();
			mFN=theFN;
			mHash=mFN.GetHashI();
		}

		operator Smart(DynamicSprite)() {return mSprite;}

		longlong mHash;
		String mFN;
		Smart(DynamicSprite) mSprite;
	};

	SmartList(DSInfo)					mSpriteList;
	SmartList(RComm::RaptisoftQuery)*	mPendingQueryList=NULL;

	inline void Reset() {mSpriteList.Clear();}

	int							mCursor=0;	// Walks through the sprite list...
};

void LoadSpriteFolder(String theDir, Array<Sprite>& theArray);

