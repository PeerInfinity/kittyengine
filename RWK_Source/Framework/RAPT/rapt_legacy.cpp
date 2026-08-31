#ifdef RAPT_LEGACY
#ifdef RAPT_GRAPHICS_H
#ifdef GL30
#define USEPIPELINE
#endif


enum
{
	DRAWINFO_WORLDMATRIX=0x00000001,
	DRAWINFO_VIEWMATRIX=0x00000002,
	DRAWINFO_PROJECTIONMATRIX=0x00000004,
	DRAWINFO_VIEWPORT=0x00000008,

	DRAWINFO_COMBOMATRIX=0x00000010,
	DRAWINFO_DRAWMATRIX=0x00000010,
	DRAWINFO_WORLDVIEWMATRIX=0x00000020,
	DRAWINFO_BILLBOARDMATRIX=0x00000040,

	DRAWINFO_TEXTURES=0x00000080,

	DRAWINFO_WORLDMATRIXN=0x00000100,
	DRAWINFO_VIEWMATRIXN=0x00000200,
	DRAWINFO_PROJECTIONMATRIXN=0x00000400,

	DRAWINFO_COLOR=0x00000800,
	DRAWINFO_ZBIAS=0x00001000,
	DRAWINFO_ALPHATHRESHOLD=0x00002000,
	DRAWINFO_SHADERDATA=	0x00004000,
	DRAWINFO_LOOKAT=	0x00008000,
	DRAWINFO_TIME=			0x00010000,
	DRAWINFO_SHADERDATA2=	0x00020000, // Extra shaderdata, so I don't have to do so many uniforms...
	DRAWINFO_SHADERDATA3=	0x00040000,
	DRAWINFO_REZ=			0x00080000, // Pagesize in xy, Pagesize/2 in zw
	DRAWINFO_FLAGS=			0x00100000, // gG.flags in shader...
	DRAWINFO_SATURATION=	0x00200000, // gG.saturation in shader...

	// NEWSVAR NewShaderVariable New Shader Variable


	DRAWINFO_ALLMATRICES=DRAWINFO_WORLDMATRIX|DRAWINFO_VIEWMATRIX|DRAWINFO_PROJECTIONMATRIX|DRAWINFO_WORLDMATRIXN|DRAWINFO_VIEWMATRIXN|DRAWINFO_PROJECTIONMATRIXN,
	DRAWINFO_DEPENDENCIES=DRAWINFO_COMBOMATRIX|DRAWINFO_WORLDVIEWMATRIX|DRAWINFO_BILLBOARDMATRIX,
	DRAWINFO_ALL=0xFFFFFFFF
};

#define MAXTEXTURES	5

class RGBA
{
public:
	RGBA() {}
	RGBA(int r, int g, int b, int a)
	{
		mB=b;
		mR=r;
		mG=g;
		mA=a;
	}
	RGBA(int a)
	{
		mB=255;
		mR=255;
		mG=255;
		mA=a;
	}

	inline bool operator==(RGBA &thePixel) {return ((mR==thePixel.mR)&&(mG==thePixel.mG)&&(mB==thePixel.mB)&&(mA==thePixel.mA));}
	inline bool operator!=(RGBA &thePixel) {return ((mR!=thePixel.mR)||(mG!=thePixel.mG)||(mB!=thePixel.mB)||(mA!=thePixel.mA));}

	unsigned char mB;
	unsigned char mG;
	unsigned char mR;
	unsigned char mA;

	inline int ToInt() {return *(int*)this;}

	RGBA Lerp(RGBA theOther, float x)
	{
		int aR=_clamp(0,gMath.Lerp(mR,theOther.mR,x),255);
		int aG=_clamp(0,gMath.Lerp(mG,theOther.mG,x),255);
		int aB=_clamp(0,gMath.Lerp(mB,theOther.mB,x),255);
		int aA=_clamp(0,gMath.Lerp(mA,theOther.mA,x),255);
		return RGBA(aR,aG,aB,aA);
	}
};

inline RGBA LerpRGBA(int rgba1, int rgba2, float x)
{
	RGBA* aLow=(RGBA*)&rgba1;
	RGBA* aHigh=(RGBA*)&rgba1;
	int aR=_clamp(0,gMath.Lerp(aLow->mR,aHigh->mR,x),255);
	int aG=_clamp(0,gMath.Lerp(aLow->mG,aHigh->mG,x),255);
	int aB=_clamp(0,gMath.Lerp(aLow->mB,aHigh->mB,x),255);
	int aA=_clamp(0,gMath.Lerp(aLow->mA,aHigh->mA,x),255);
	return RGBA(aR,aG,aB,aA);
}

template <typename var_type>
void VertsToIndexedVerts(int theVCount, var_type* theVertices, Array<var_type>& theResultVerts, Array<vbindex>& theResultIndices)
{
	//
	// Find all unique vertices, put them in a list.
	//
	theResultIndices.Reset();
	theResultVerts.Reset();

	PointerTree3D<vbindex> aTree;	// Tree of all added indicia, for speed

	theResultIndices.GuaranteeSize(theVCount);
	theResultVerts.GuaranteeSize(theVCount);

	int aICount = 0;
	int aVCount = 0;
	var_type* aVertex=(theVertices);
	for (int aCount=0;aCount<theVCount;aCount++)
	{
		bool aFound=false;
		Cube aCube(.01f,.01f,.01f);
		aCube.CenterAt(aVertex->ToVector());
		auto aTList=aTree.Get(aCube);

		if (aTList.Size()) for (int aCompareI=0;aCompareI<aTList.Size();aCompareI++)
		{
			if (aVertex->PrettyClose(theResultVerts[aTList[aCompareI]]))
			{
				theResultIndices[aICount++]=aTList[aCompareI]; // Note: I had a pretty bad glitch here, where I was making theResultIndicies=aCompareI (which then was just the index into the fetched list)
				aFound=true;
				break;
			}
		}

		if (!aFound)
		{
			theResultVerts[aVCount++]=*aVertex;
			theResultIndices[aICount++]=aVCount-1;
			aTree.Add(aVCount-1,aCube);
		}
		aVertex++;
	}

	theResultIndices.Clip(aICount);
	theResultVerts.Clip(aVCount);
}

#ifdef USEPIPELINE
#include "rapt_vertexformats.h"

class Shader;
class PipelineBase
{
public:
	virtual void Flush() {}
};

template <typename vtype>
class Pipeline : public PipelineBase
{
public:
	Pipeline(Shader* theShader, int theMaxSize=1000) 
	{
		mShader=theShader;
		Presize(theMaxSize);
	}
	Pipeline(Shader* theShader, Pipeline<vtype>* sharePipeline) 
	{
		mShader=theShader;
		ShareKit(sharePipeline);
	}

	inline void ShareKit(Pipeline<vtype>* thePipeline) {mKit=thePipeline->mKit;}
	//
	// BIZARRE C++ Template declaration example syntax for this if you declare it inside another class.
	// 
	//         Pipeline<Vx2D> mButtonPipeline {&gShaderBlur};
	// 
	// Why is it not in parenthesis?  Why is it in braces?  Probably the non-ObjOriented C++ designer guy knows.
	// Anyway, the constructor gets called.  It's all the exact same, just... braces.
	//
	Shader* mShader=NULL;
	struct Kit
	{
		Array<vbindex> mI;
		Array<vtype> mV;
		int mVCount=0;
		int mICount=0;
		int mMaxV; 
		int mIStartShape=0;

		inline void Reset() {mVCount=mICount=0;}
	};

	Smart(Kit) mKit=new Kit;

	//
	// To presize it...
	//
	inline void	Presize(int theSize=10000) {mKit->mMaxV=theSize;mKit->mI.GuaranteeSize(mKit->mMaxV*3);mKit->mV.GuaranteeSize(mKit->mMaxV);}

	// Flush has to be set up to not allow nested flushes... shader changes can force a flush, so we'd end up call stacking.
	void					Flush() {static bool aFlushing=false;if (aFlushing) return;aFlushing=true;if (mKit->mICount>0) mShader->DrawShape(mKit->mV.mArray,mKit->mVCount,mKit->mI.mArray,mKit->mICount);mKit->Reset();aFlushing=false;}

	inline void				CheckMaxV(int theTriangles) {if (mKit->mVCount+(theTriangles*3)>=mKit->mMaxV) Flush();}
	inline void				StartShape(int theTriangles) {CheckMaxV(theTriangles);mKit->mIStartShape=mKit->mVCount;}

	// SVERO : (FinishShape va_list&) Changed the va_arg param from short to int... Using short there causes android to segfault Maybe something to do with how gcc compiles this vs vc... (FUCKING hard to find this bug!)
	inline void FinishShape(int theIndices, ...)  {va_list aIndexPtr;va_start(aIndexPtr,theIndices);FinishShape(theIndices,aIndexPtr);va_end(aIndexPtr);}
	inline void FinishShape(int theIndices, va_list& theIndexPtr) {int* aPtr=&theIndices;for (int aCount=1;aCount<=theIndices;aCount++){vbindex aIndex=(vbindex)va_arg(theIndexPtr,int);mKit->mI[mKit->mICount++]=aIndex+mKit->mIStartShape;}}
	inline void FinishShape(Array<vbindex>& theIndexArray) {for (int aCount=0;aCount<theIndexArray.Size();aCount++) mKit->mI[mKit->mICount++]=theIndexArray[aCount]+mKit->mIStartShape;}


	inline void				AddVertex(vtype theV);
	inline vtype&			AddVertex() {return mKit->mV[mKit->mVCount++];} // Remember, must manually handle gG.Translate, etc, for this!
	void					AddVertexIndex(int theIndex) {mKit->mI[mKit->mICount++]=theIndex+mKit->mIStartShape;}

	inline void				StartQuad() {StartShape(2);}
	inline void				FinishQuad() {AddVertexIndex(0);AddVertexIndex(1);AddVertexIndex(2);AddVertexIndex(2);AddVertexIndex(1);AddVertexIndex(3);}
};
#endif


//
// Helpers...
//
typedef struct Vertex2D
{
public:
	Vertex2D() {}
	Vertex2D(float x,float y, float z, unsigned int diffuse, float tu, float tv) {mX=x;mY=y;mZ=z;mDiffuse=diffuse;mTextureU=tu;mTextureV=tv;}
	Vertex2D(float x,float y, float z, unsigned int diffuse, Point uv) {mX=x;mY=y;mZ=z;mDiffuse=diffuse;mTextureU=uv.mX;mTextureV=uv.mY;}
	Vertex2D(Vector v, unsigned int diffuse, float tu, float tv) {mX=v.mX;mY=v.mY;mZ=v.mZ;mDiffuse=diffuse;mTextureU=tu;mTextureV=tv;}
	Vertex2D(Vector v, unsigned int diffuse, Point t) {mX=v.mX;mY=v.mY;mZ=v.mZ;mDiffuse=diffuse;mTextureU=t.mX;mTextureV=t.mY;}
	Vertex2D(int x,int y, int z, unsigned int diffuse, float tu, float tv) {mX=(float)x;mY=(float)y;mZ=(float)z;mDiffuse=diffuse;mTextureU=tu;mTextureV=tv;}
	Vertex2D(float x,float y, float z) {mX=x;mY=y;mZ=z;mDiffuse=0xFFFFFFFF;mTextureV=0;mTextureV=0;}
	Vertex2D(float x,float y, float z, unsigned int diffuse) {mX=x;mY=y;mZ=z;mDiffuse=diffuse;mTextureV=0;mTextureV=0;}

	float			mX;
	float			mY;
	float			mZ;

	unsigned int	mDiffuse;

	float			mTextureU;
	float			mTextureV;

	inline Vector*  XYZ() {return (Vector*)&mX;}
	inline Point*	UV() {return (Point*)&mTextureU;}


	inline bool		operator==(const Vertex2D &theVertex) {return !(mX!=theVertex.mX || mY!=theVertex.mY || mZ!=theVertex.mZ || mDiffuse!=theVertex.mDiffuse || mTextureU!=theVertex.mTextureU || mTextureV!=theVertex.mTextureV);}
	inline bool		PrettyClose(const Vertex2D &theVertex) {return !(!::PrettyClose(mX,theVertex.mX) || !::PrettyClose(mY,theVertex.mY) || !::PrettyClose(mZ,theVertex.mZ) || mDiffuse!=theVertex.mDiffuse || !::PrettyClose(mTextureU,theVertex.mTextureU) || !::PrettyClose(mTextureV,theVertex.mTextureV));}
	inline Vector	ToVector() {return Vector(mX,mY,mZ);}
	inline Point	ToUV() {return Point(mTextureU,mTextureV);}
	inline Vector	ToTranslatedVector(float theTranslate) {return Vector(mX+theTranslate,mY+theTranslate,mZ+theTranslate);}
	inline Vector	ToTranslatedVector(Vector theTranslate) {return Vector(mX+theTranslate.mX,mY+theTranslate.mY,mZ+theTranslate.mZ);}

	inline void		SetUV(float theU, float theV) {mTextureU=theU;mTextureV=theV;}
	inline void		SetUV(Point theUV) {SetUV(theUV.mX,theUV.mY);}
	inline void		SetPos(float x, float y, float z) {mX=x;mY=y;mZ=z;}
	inline void		SetPos(Vector theXYZ) {mX=theXYZ.mX;mY=theXYZ.mY;mZ=theXYZ.mZ;}
	inline void		Set(Vector theXYZ, Point theUV, unsigned int theDiffuse) {SetPos(theXYZ);SetUV(theUV);mDiffuse=theDiffuse;}

} Vertex2D;


typedef struct Vertex2DN
{
public:
	Vertex2DN() {}
	Vertex2DN(float x,float y, float z, float nx, float ny, float nz, float tu, float tv) {mX=x;mY=y;mZ=z;mNX=nx;mNY=ny;mNZ=nz;mTextureU=tu;mTextureV=tv;}
	Vertex2DN(float x,float y, float z, float nx, float ny, float nz, Point uv)  {mX=x;mY=y;mZ=z;mNX=nx;mNY=ny;mNZ=nz;mTextureU=uv.mX;mTextureV=uv.mY;}
	Vertex2DN(Vector v, Vector n, float tu, float tv)  {mX=v.mX;mY=v.mY;mZ=v.mZ;mNX=n.mX;mNY=n.mY;mNZ=n.mZ;mTextureU=tu;mTextureV=tv;}
	Vertex2DN(Vector v, Vector n, Point uv)  {mX=v.mX;mY=v.mY;mZ=v.mZ;mNX=n.mX;mNY=n.mY;mNZ=n.mZ;mTextureU=uv.mX;mTextureV=uv.mY;}
	Vertex2DN(Vector v, Vector n, Point uv, int theExtra)  {mX=v.mX;mY=v.mY;mZ=v.mZ;mNX=n.mX;mNY=n.mY;mNZ=n.mZ;mTextureU=uv.mX;mTextureV=uv.mY;}
	Vertex2DN(int x,int y, int z, Vector n, float tu, float tv)  {mX=(float)x;mY=(float)y;mZ=(float)z;mNX=n.mX;mNY=n.mY;mNZ=n.mZ;mTextureU=tu;mTextureV=tv;}
	Vertex2DN(int x,int y, int z, Vector n, Point uv)  {mX=(float)x;mY=(float)y;mZ=(float)z;mNX=n.mX;mNY=n.mY;mNZ=n.mZ;mTextureU=uv.mX;mTextureV=uv.mY;}
	Vertex2DN(float x,float y, float z, Vector n, Point uv)  {mX=(float)x;mY=(float)y;mZ=(float)z;mNX=n.mX;mNY=n.mY;mNZ=n.mZ;mTextureU=uv.mX;mTextureV=uv.mY;}
	Vertex2DN(float x,float y, float z, Vector n, float tu, float tv)  {mX=(float)x;mY=(float)y;mZ=(float)z;mNX=n.mX;mNY=n.mY;mNZ=n.mZ;mTextureU=tu;mTextureV=tv;}
	Vertex2DN(float x,float y, float z)  {mX=x;mY=y;mZ=z;mTextureV=0;mTextureV=0;}
	Vertex2DN(float x,float y, float z, float nx, float ny, float nz)  {mX=x;mY=y;mZ=z;mNX=nx;mNY=ny;mNZ=nz;mTextureV=0;mTextureV=0;}

	float			mX;
	float			mY;
	float			mZ;

	float			mNX;
	float			mNY;
	float			mNZ;
	
	float			mTextureU;
	float			mTextureV;

	inline Vector*  XYZ() {return (Vector*)&mX;}
	inline Point*	UV() {return (Point*)&mTextureU;}
	inline Vector*	Normal() {return (Vector*)&mNX;}

	inline bool		operator==(const Vertex2DN &theVertex) {return !(mX!=theVertex.mX || mY!=theVertex.mY || mZ!=theVertex.mZ || mNX!=theVertex.mNX || mNY!=theVertex.mNY || mNZ!=theVertex.mNZ || mTextureU!=theVertex.mTextureU || mTextureV!=theVertex.mTextureV);}
	inline bool		PrettyClose(const Vertex2DN &theVertex) {return !(!::PrettyClose(mX,theVertex.mX) || !::PrettyClose(mY,theVertex.mY) || !::PrettyClose(mZ,theVertex.mZ) || !::PrettyClose(mNX,theVertex.mNX) || !::PrettyClose(mNY,theVertex.mNY) || !::PrettyClose(mNZ,theVertex.mNZ) || !::PrettyClose(mTextureU,theVertex.mTextureU) || !::PrettyClose(mTextureV,theVertex.mTextureV));}

	inline Vector	ToVector() {return Vector(mX,mY,mZ);}
	inline Vector	ToNormal() {return Vector(mNX,mNY,mNZ);}
	inline Point	ToUV() {return Point(mTextureU,mTextureV);}
	inline Vector	ToTranslatedVector(float theTranslate) {return Vector(mX+theTranslate,mY+theTranslate,mZ+theTranslate);}
	inline Vector	ToTranslatedVector(Vector theTranslate) {return Vector(mX+theTranslate.mX,mY+theTranslate.mY,mZ+theTranslate.mZ);}

	inline void		SetUV(float theU, float theV) {mTextureU=theU;mTextureV=theV;}
	inline void		SetUV(Point theUV) {SetUV(theUV.mX,theUV.mY);}
	inline void		SetPos(float x, float y, float z) {mX=x;mY=y;mZ=z;}
	inline void		SetNormal(float x, float y, float z) {mNX=x;mNY=y;mNZ=z;}
	inline void		SetNormal(Vector theN) {SetNormal(theN.mX,theN.mY,theN.mZ);}
	inline void		SetPos(Vector theXYZ) {mX=theXYZ.mX;mY=theXYZ.mY;mZ=theXYZ.mZ;}
	inline void		Set(Vector theXYZ, Vector theNormal, Point theUV) {SetPos(theXYZ);SetUV(theUV);SetNormal(theNormal);}

} Vertex2DN;

typedef struct Vertex2DMT : public Vertex2D
{
public:
	Vertex2DMT() {}
	Vertex2DMT(Vertex2D mV) {mX=mV.mX;mY=mV.mY;mZ=mV.mZ;mDiffuse=mV.mDiffuse;mTextureU=mV.mTextureU;mTextureV=mV.mTextureV;mTextureU2=0;mTextureV2=0;}
	Vertex2DMT(float x,float y, float z, unsigned int diffuse, float tu, float tv, float tu2, float tv2) {mX=x;mY=y;mZ=z;mDiffuse=diffuse;mTextureU=tu;mTextureV=tv;mTextureU2=tu2;mTextureV2=tv2;}
	Vertex2DMT(float x,float y, float z, unsigned int diffuse, Point uv, Point uv2)  {mX=x;mY=y;mZ=z;mDiffuse=diffuse;mTextureU=uv.mX;mTextureV=uv.mY;mTextureU2=uv2.mX;mTextureV2=uv2.mY;}
	Vertex2DMT(Vector v, unsigned int diffuse, float tu, float tv)  {mX=v.mX;mY=v.mY;mZ=v.mZ;mDiffuse=diffuse;mTextureU=tu;mTextureV=tv;}
	Vertex2DMT(Vector v, unsigned int diffuse, float tu, float tv, float tu2, float tv2)  {mX=v.mX;mY=v.mY;mZ=v.mZ;mDiffuse=diffuse;mTextureU=tu;mTextureV=tv;mTextureU2=tu2;mTextureV2=tv2;}
	Vertex2DMT(int x,int y, int z, unsigned int diffuse, float tu, float tv, float tu2, float tv2)  {mX=(float)x;mY=(float)y;mZ=(float)z;mDiffuse=diffuse;mTextureU=tu;mTextureV=tv;mTextureU2=tu2;mTextureV2=tv2;}
	Vertex2DMT(float x,float y, float z)  {mX=x;mY=y;mZ=z;mDiffuse=0xFFFFFFFF;mTextureV=0;mTextureV=0;}

	float			mTextureU2;
	float			mTextureV2;
	inline Point*	UV2() {return (Point*)&mTextureU2;}

	inline bool		operator==(const Vertex2DMT &theVertex) {return !(mX!=theVertex.mX || mY!=theVertex.mY || mZ!=theVertex.mZ || mDiffuse!=theVertex.mDiffuse || mTextureU!=theVertex.mTextureU || mTextureV!=theVertex.mTextureV || mTextureU2!=theVertex.mTextureU2 || mTextureV2!=theVertex.mTextureV2);}
	inline bool		PrettyClose(const Vertex2DMT &theVertex) {return !(!::PrettyClose(mX,theVertex.mX) || !::PrettyClose(mY,theVertex.mY) || !::PrettyClose(mZ,theVertex.mZ) || mDiffuse!=theVertex.mDiffuse || !::PrettyClose(mTextureU,theVertex.mTextureU) || !::PrettyClose(mTextureV,theVertex.mTextureV) || !::PrettyClose(mTextureU2,theVertex.mTextureU2) || !::PrettyClose(mTextureV2,theVertex.mTextureV2));}

} Vertex2DMT;


class VB
{
public:
	VB() {Reset();}

	Array<Vertex2D>		mV;
	Array<vbindex>		mI;
	int					mVCount;
	int					mICount;
	char				mFlag;

	void Reset() {mV.Reset();mI.Reset();mVCount=0;mICount=0;mFlag=0;}
	void ResetQuick() {mVCount=0;mICount=0;mFlag=0;}
	void Clip() {mV.Clip(mVCount);mI.Clip(mICount);}

	int					CurrentV() {return mVCount;}
	int					CurrentI() {return mICount;}
	Vertex2D&			NextV() {return mV[mVCount++];}
	vbindex&			NextI() {return mI[mICount++];}
	
};

class VBMT
{
public:
	VBMT() {Reset();}

	Array<Vertex2DMT>	mV;
	Array<vbindex>		mI;
	int					mVCount;
	int					mICount;
	char				mFlag;

	void Reset() {mV.Reset();mI.Reset();mVCount=0;mICount=0;mFlag=0;}
	void ResetQuick() {mVCount=0;mICount=0;mFlag=0;}
	void Clip() {mV.Clip(mVCount);mI.Clip(mICount);}

	int					CurrentV() {return mVCount;}
	int					CurrentI() {return mICount;}
	Vertex2DMT&			NextV() {return mV[mVCount++];}
	vbindex&			NextI() {return mI[mICount++];}
};

class VBN
{
public:
	VBN() {Reset();}

	Array<Vertex2DN>	mV;
	Array<vbindex>		mI;
	int					mVCount;
	int					mICount;
	char				mFlag;

	void Reset() {mV.Reset();mI.Reset();mVCount=0;mICount=0;mFlag=0;}
	void ResetQuick() {mVCount=0;mICount=0;mFlag=0;}
	void Clip() {mV.Clip(mVCount);mI.Clip(mICount);}

	int					CurrentV() {return mVCount;}
	int					CurrentI() {return mICount;}
	Vertex2DN&			NextV() {return mV[mVCount++];}
	vbindex&			NextI() {return mI[mICount++];}
};




typedef struct Key3D
{
	Vector			mPos;
	Vector			mRotation;
} Key3D;


inline unsigned int TexMem(int w, int h) {return (w*h*4);}

Vertex2D MakeVertex2D(Vector theVector, int theDiffuse, Point theUV);
Vertex2DMT MakeVertex2DMT(Vector theVector, int theDiffuse, Point theUV, Point theUV2);
inline Vertex2DMT MakeVertex2DMT(Vector theVector, int theDiffuse, Point theUV) {return MakeVertex2DMT(theVector,theDiffuse,theUV,Point(0,0));}

typedef struct Vertex2D Vertex3D;

class Shader;
class Model;

typedef struct Viewport
{
	float			mX;
	float			mY;
	float			mWidth;
	float			mHeight;
	float			mMinZ;
	float			mMaxZ;
} Viewport;

class Graphics
{
public:

	//
	// Call these in App's constructor--
	// They're pre-game stuff to set up things.
	//
	void					SetDefaultResolution(int theWidth, int theHeight, bool isFullscreen);	// Sets the default game resolution on first run
	void					SetDefaultResolution(bool isFullscreen);								// Sets the default to highest possible (if not fullscreen, will fit it in the window)
	IPoint					GetHighestResolution(bool isWindowed);									// Gets the largest possible resolution
	void					SetAutoPageSizeMultiplier(String theResolution, float theMultiplier);	// Sets it up so that certain resolutions will automatically scale
	inline void				ClampHeight(int theY) {mClampHeight=theY;}								// Makes sure the Y page size never exceeds this (and adjusts X accordingly)
	void					RequestZ(float theZDepth=1000);											// Requests that ZBuffering be enabled
	void					RequestZ(float theZNear, float theZFar);								// Requests that ZBuffering be enabled, specifying near and far

#ifndef USEPIPELINE
	void					RequestMaxVertices(int theNumber, int theIndices=-1);					// Max vertices before we flush the buffer.
#endif
	Point					mZRange;	// Depth of Z

	int						Time(); // Just a helper for accessing AppTime from gG

	int						mFlags=0;		// gG.flags in shaders
	int						mLastFlags=0;	// Last flag (for tracking state changes)
	enum
	{
		FLAG_RENDERTO=			0x00000001,
		FLAG_RENDERWHITE=		0x00000002,
		FLAG_RENDERGREY=		0x00000004,

		FLAG_RENDERMODES=		FLAG_RENDERWHITE|FLAG_RENDERGREY,
		// NEWGFLAG
	};

	inline bool				IsRenderTo() {return (mFlags&FLAG_RENDERTO)!=0;}


public:
	Graphics();

	void					Go();
	void					Stop();
	void					Multitasking(bool isForeground);
	void					SystemNotify(String theNotify);

	void					BeginRender();
	void					EndRender();
	void					Show();

#ifdef USEPIPELINE
	//
	// Vertex Pipelines...
	//
	void					InitializePipelines();
	Pipeline<Vx2D>*			mPipeline_Normal;
	Pipeline<Vx2D>*			mPipeline_RenderWhite;
	Pipeline<Vx2D>*			mPipeline_Grey;
	PipelineBase*			mCurrentPipeline=mPipeline_Normal;
	void					SetPipeline(PipelineBase* thePipeline);
	void					FlushDrawBuffer();
#endif

public:
	void					SetResolution(int theWidth, int theHeight, bool isFullscreen);
	void					GetAvailableResolutions(Array<IPoint>& theArray);

	int						mResolutionWidth;
	int						mResolutionHeight;
	bool					mFullscreen;
	inline bool				IsFullscreen() {return mFullscreen;}
	bool					CanDoWindowed();
	inline int				GetResolutionWidth() {return mResolutionWidth;}
	inline int				GetResolutionHeight() {return mResolutionHeight;}

	void					SetPageSize(int theWidth, int theHeight);

	Font*					TempFont(); // DEBUG MODE ONLY-- gives a generic font for quick display of stuff

	//
	// These helper functions set the page size based on one desired dimension, keeping
	// the aspect ratio.
	//
	void					ForcePageSizeByWidth(int theDesiredWidth);
	void					ForcePageSizeByHeight(int theDesiredHeight);

	int						mWidth;
	int						mHeight;
	inline Rect				GetPageRect() {return Rect(0,0,(float)GetWidth(),(float)GetHeight());}
	inline IPoint			GetDimensions() {return IPoint(GetWidth(),GetHeight());}
	inline IPoint			GetHalfDimensions() {return IPoint(GetWidth()/2,GetHeight()/2);}
	inline Point			GetDimensionsF() {return Point(GetWidthF(),GetHeightF());}
	inline Point			GetHalfDimensionsF() {return Point(GetWidthF(),GetHeightF())/2;}
	inline int				GetWidth() {return mWidth-(int)(mLetterboxSize.mX);}
	inline int				GetHeight() {return mHeight-(int)(mLetterboxSize.mY);}
	inline float			GetWidthF() {return (float)mWidth-(mLetterboxSize.mX);}
	inline float			GetHeightF() {return (float)mHeight-(mLetterboxSize.mY);}
	inline int				Width() {return GetWidth();}
	inline int				Height() {return GetHeight();}
	inline float			WidthF() {return GetWidthF();}
	inline float			HeightF() {return GetHeightF();}
	bool					MustPow2Textures();



	inline int				GetHalfWidth() {return GetWidth()/2;}
	inline int				GetHalfHeight() {return GetHeight()/2;}
	inline float			GetHalfWidthF() {return GetWidthF()/2;}
	inline float			GetHalfHeightF() {return GetHeightF()/2;}
	inline int				HalfWidth() {return GetWidth()/2;}
	inline int				HalfHeight() {return GetHeight()/2;}
	inline float			HalfWidthF() {return GetWidthF()/2;}
	inline float			HalfHeightF() {return GetHeightF()/2;}

	inline Point			Center() {return Point(GetWidth(),GetHeight())/2;}
	inline Point			UpperLeft() {return Point(0,0);}
	inline Point			UpperRight() {return Point(GetWidth(),0);}
	inline Point			LowerLeft() {return Point(0,GetHeight());}
	inline Point			LowerRight() {return Point(GetWidth(),GetHeight());}

	//
	// For automatic page sizing... this is on by default, and makes page size ALWAYS equal to the
	// resolution.  If you ever call SetPageSize, it turns off.
	//
	bool					mAutoPageSize;
    float                   mAutoPageSizeMultiplier;
    inline float            GetAutoPageSizeMultiplier() {return mAutoPageSizeMultiplier;}
	struct AutoPageSize
	{
		IPoint				mResolution;
		float				mMultiplier;
	};
	Array<AutoPageSize>		mAutoPageSizeList;
	int						mClampHeight;

public:
	//
	// Fills and Clears
	//
	void					Clear(float theRed, float theGreen, float theBlue, float theAlpha=1.0f);
	inline void				Clear(Color theColor) {Clear(theColor.mR,theColor.mG,theColor.mB, theColor.mA);}
	inline void				Clear() {Clear(mColor.mR,mColor.mG,mColor.mB, mColor.mA);}
	inline void				Clear(float theAlpha) {Clear(1,1,1,theAlpha);}
	void					ClearZ(float theValue=1.0f);
#ifdef USEPIPELINE
	int						mBlankTexture;

	inline void				FillRect(float theX, float theY, float theWidth, float theHeight) 
	{
		mPipeline_Normal->StartQuad();
		mPipeline_Normal->AddVertex().Set(Fix2D(theX,theY),mColorInt,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth,theY),mColorInt,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX,theY+theHeight),mColorInt,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth,theY+theHeight),mColorInt,mClampUVCoordsPos);
		mPipeline_Normal->FinishQuad();
	}
	inline void				FillRectGradientV(float theX, float theY, float theWidth, float theHeight, Color theTop, Color theBottom)
	{
		theTop*=mColor;
		theBottom*=mColor;
		int aTop=theTop.ToInt();
		int aBottom=theBottom.ToInt();
		mPipeline_Normal->StartQuad();
		mPipeline_Normal->AddVertex().Set(Fix2D(theX,theY),aTop,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth,theY),aTop,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX,theY+theHeight),aBottom,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth,theY+theHeight),aBottom,mClampUVCoordsPos);
		mPipeline_Normal->FinishQuad();
	}
	void					FillRectGradientH(float theX, float theY, float theWidth, float theHeight, Color theLeft, Color theRight)
	{
		theLeft*=mColor;
		theRight*=mColor;
		int aLeft=theLeft.ToInt();
		int aRight=theRight.ToInt();
		mPipeline_Normal->StartQuad();
		mPipeline_Normal->AddVertex().Set(Fix2D(theX,theY),aLeft,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth,theY),aRight,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX,theY+theHeight),aLeft,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth,theY+theHeight),aRight,mClampUVCoordsPos);
		mPipeline_Normal->FinishQuad();
	}
	void					FillRectGradientBurst(float theX, float theY, float theWidth, float theHeight, Color theInside, Color theOutside, float theBorderSize)
	{
		theInside*=mColor;
		theOutside*=mColor;
		int aIColor=theInside.ToInt();
		int aOColor=theOutside.ToInt();

		mPipeline_Normal->StartShape(6);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX-theBorderSize,theY-theBorderSize),aOColor,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth+theBorderSize,theY-theBorderSize),aOColor,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX,theY),aIColor,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth,theY),aIColor,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX-theBorderSize,theY+theHeight+theBorderSize),aOColor,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX,theY+theHeight),aIColor,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth+theBorderSize,theY+theHeight+theBorderSize),aOColor,mClampUVCoordsPos);
		mPipeline_Normal->AddVertex().Set(Fix2D(theX+theWidth,theY+theHeight),aIColor,mClampUVCoordsPos);									
		mPipeline_Normal->FinishShape(24, 0,1,2,2,1,3, 0,2,4,4,2,5, 4,5,6,5,6,7, 1,3,6,6,3,7);
	}
#else
	void					FillRect(float theX, float theY, float theWidth, float theHeight);
	void					FillRect(Rect theRect, float theBevel) {FillRect(theRect);}
	void					FillRectGradientV(float theX, float theY, float theWidth, float theHeight, Color theTop, Color theBottom);
	void					FillRectGradientH(float theX, float theY, float theWidth, float theHeight, Color theLeft, Color theRight);
	void					FillRectGradientBurst(float theX, float theY, float theWidth, float theHeight, Color theInside, Color theOutside, float theBorderSize);
	
	inline void				FillRect(float theX, float theY, float theWidth, float theHeight, float theBevel) {FillRect(theX,theY,theWidth,theHeight);}
	
#endif

	inline void				FillRect() {FillRect(0.0f,0.0f,(float)mWidth,(float)mHeight);}
	inline void				FillRect(Rect theRect) {FillRect(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	inline void				FillRect(Point theUpperLeft, Point theLowerRight) {FillRect(theUpperLeft.mX,theUpperLeft.mY,theLowerRight.mX-theUpperLeft.mX,theLowerRight.mY-theUpperLeft.mY);}
	inline void				DrawPoint(Point thePos, float theSize=3) {FillRect(thePos.mX-(theSize/2),thePos.mY-(theSize/2),theSize,theSize);}

	//
	// Clip the view area, taking into account translations and whatnot... so these are in "current" coordinates.
	//
	void					Clip(float theX, float theY, float theWidth, float theHeight);
	inline void				Clip(Rect theRect) {Clip(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	inline void				Clip(Point theUpperLeft, Point theLowerRight) {Clip(theUpperLeft.mX,theUpperLeft.mY,theLowerRight.mX-theUpperLeft.mX,theLowerRight.mY-theUpperLeft.mY);}
	void					Clip();
	Rect					mClipRect;
	inline Rect				GetClipRect() 
	{
		if (mClipRect.mWidth==-1) return Rect(-100000,-100000,200000,200000);
		return mClipRect.Translate(-(float)(mLetterboxSize.mX/2),-(float)(mLetterboxSize.mY/2));
	}

	//
	// ClipInto clips your clip against the current clip!
	//
	void					ClipInto(float theX, float theY, float theWidth, float theHeight);
	inline void				ClipInto(Rect theRect) {ClipInto(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	inline void				ClipInto(Point theUpperLeft, Point theLowerRight) {ClipInto(theUpperLeft.mX,theUpperLeft.mY,theLowerRight.mX-theUpperLeft.mX,theLowerRight.mY-theUpperLeft.mY);}

	//
	// SetClip sets the clip directly, ignores world coordinates or translations and things!  These are in "screen" coordinates.
	//
	void					SetClip(float theX, float theY, float theWidth, float theHeight);
	inline void				SetClip(Rect theRect) {SetClip(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	inline void				SetClip(Point theUpperLeft, Point theLowerRight) {SetClip(theUpperLeft.mX,theUpperLeft.mY,theLowerRight.mX-theUpperLeft.mX,theLowerRight.mY-theUpperLeft.mY);}

	//
	// Freezes the state of our clipping (all subsequent clip calls are
	// actually ClipInto this)
	//
	inline void				FreezeClip(bool theState=true) {mFreezeClip.mState=theState;mFreezeClip.mRect=mClipRect;}
	struct FreezeClipState
	{
		bool				mState;
		Rect				mRect;
	} mFreezeClip;

	Stack<FreezeClipState>	mFreezeClipStack;
	inline void				PushFreezeClip() {mFreezeClipStack.Push(mFreezeClip);}
	inline void				PopFreezeClip() {mFreezeClip=mFreezeClipStack.Pop();}

public:
	//
	// For world translations, rotations, etc
	//
	Point					mBaseTranslate;	// Translation always starts here... you should NOT change this, because App changes it.
											// It's specifically put in to assist with moving the screen for the keyboard.
	Point					mTranslate;
	inline void				Translate() {mTranslate=mBaseTranslate;mTranslate+=(mLetterboxSize/2);}
	inline void				Translate(int x, int y) {mTranslate.mX+=x;mTranslate.mY+=y;}
	inline void				Translate(float x, float y) {mTranslate.mX+=x;mTranslate.mY+=y;}
	inline void				Translate(Point thePos) {mTranslate+=thePos;}
	inline void				Translate(IPoint thePos) {mTranslate+=Point(thePos.mX,thePos.mY);}
	inline void				TranslateToIntegers() {mTranslate.mX=(float)((int)mTranslate.mX);mTranslate.mY=(float)((int)mTranslate.mY);}
	//
	// Scale/Rotate (view matrix)
	// A note: This rotates the visible screen.  Your center always needs to be
	// something within the page view area.  Translates will not affect this!


	unsigned int			mRehupDrawInfo=0;	// Which matrices need to be rehupped...

	void					ScaleView(Matrix &theMatrix);
	void					ScaleView(float theScale, Point theCenter);
	inline void				ScaleView(float theScale=1.0f) {ScaleView(theScale,Center());}
	void					RotateView(float theRotation, Point theCenter);
//	inline void				RotateView(float theRotation=1.0f) {RotateView(theRotation,Center());}
	inline void				RotateView(float theRotation=0.0f) {RotateView(theRotation,Point(mWidth/2,mHeight/2));}
	void					SetViewMatrix(Matrix &theMatrix);
	void					SetViewMatrix();
	inline void				FreshenViewMatrix() {Graphics_Core::SetMatrix(1,&mViewMatrix.mData);}
	Matrix					mViewMatrix;
	Matrix					mViewMatrixN;

	//
	// Fixes a 2D position for translate/scale/rotate.
	//
	//WARNING("Finish gG.Fix2D()"); // ::Fix2D()
	inline Point			Fix2D(Point thePos) {return Fix2D(thePos.mX,thePos.mY);}
	inline Point			Fix2D(float theX, float theY) 
	{
		theX+=mTranslate.mX;
		theY+=mTranslate.mY;
		return Point(theX,theY);
	}

	Matrix					mProjectionMatrix;
	Matrix					mProjectionMatrixN;
	void					SetProjectionMatrix(Matrix &theMatrix);
	void					SetProjectionMatrix();
	//
	// World Matrix... for finer control of things, especially 3D things, we can just use a world matrix.
	//
	void					SetWorldMatrix(Matrix &theMatrix);
	void	  				SetWorldMatrix();
	inline void				FreshenWorldMatrix() {Graphics_Core::SetMatrix(0,&mWorldMatrix.mData);}
	Matrix					mWorldMatrix;
	Matrix					mWorldMatrixN;

	//
	// Lets us modify UV positions for moving things within a texture ATLAS
	//
	void					SetTextureMatrix(Matrix &theMatrix);
	inline void				FreshenTextureMatrix(Matrix &theMatrix) {Graphics_Core::SetMatrix(3,&mTextureMatrix.mData);}
	void	  				SetTextureMatrix();
	Matrix					mTextureMatrix;

	void					SetMultiTextureMatrix(Matrix &theMatrix);
	void	  				SetMultiTextureMatrix();
	inline void				FreshenMultiTextureMatrix(Matrix &theMatrix) {Graphics_Core::SetMatrix(4,&mMultiTextureMatrix.mData);}
	Matrix					mMultiTextureMatrix;



public:
	//
	// For culling...
	//
	void					CullNone();
	void					CullCCW();
	void					CullCW();
	char					mCullState;
	Stack<char>				mCullStack;
	inline void				PushCull() {mCullStack.Push(mCullState);}
	inline void				PopCull() {char aCull=mCullStack.Pop();switch (aCull) {case 0:CullNone();break;case 1:CullCCW();break;case 2:CullCW();break;}}

public:
	//
	// For pushing and popping states
	//
	void					PushColor() {mColorStack.Push(mColor);}
	void					PushClipColor() {mClipColorStack.Push(mClipColor);}
	void					PushTranslate() {mTranslateStack.Push(mTranslate);}
	void					PushClip() {mClipStack.Push(mClipRect);}
	void					PushViewMatrix() {mViewStack.Push(mViewMatrix);}
	void					PushWorldMatrix() {mWorldStack.Push(mWorldMatrix);}

	inline void				PushAndNoClip() {PushClip();Clip();}


	void					PopColor() {SetColor(mColorStack.Pop());}
	void					PopClipColor() {SetClipColor(mClipColorStack.Pop());SetColor(mColor);}
	void					PopTranslate() {mTranslate=mTranslateStack.Pop();}
	void					PopClip();
	void					PopViewMatrix() {mViewMatrix=mViewStack.Pop();SetViewMatrix(mViewMatrix);}
	void					PopWorldMatrix() {mWorldMatrix=mWorldStack.Pop();SetWorldMatrix(mWorldMatrix);}

	Stack<Color>			mColorStack;
	Stack<Color>			mClipColorStack;
	Stack<Point>			mTranslateStack;
	Stack<Rect>				mClipStack;
	Stack<Matrix>			mViewStack;
	Stack<Matrix>			mWorldStack;

public:
	//
	// Setting/Changing colors and draw states
	//
	void						SetColor(float theR, float theG, float theB, float theA=1.0f);
	inline void					SetColor(float theA=1.0f) {SetColor(1,1,1,theA);}
	inline void					SetColorI(float theR, float theG, float theB, float theA=255.0f) {SetColor(theR/255.0f,theG/255.0f,theB/255.0f,theA/255.0f);}
	inline void					SetIColor(int theR, int theG, int theB, int theA=255) {SetColor((float)theR/255.0f,(float)theG/255.0f,(float)theB/255.0f,(float)theA/255.0f);}
	inline void					SetColorGreyscale(float theIntensity=1.0f, float theA=1.0f) {SetColor(theIntensity,theIntensity,theIntensity,theA);}
	inline void					SetColorGrey(float theIntensity=1.0f, float theA=1.0f) {SetColor(theIntensity,theIntensity,theIntensity,theA);}
	inline void					SetColorBlack(float theA=1.0f) {SetColor(0,0,0,theA);}
	inline void					SetColor(Color theColor) {SetColor(theColor.mR,theColor.mG,theColor.mB,theColor.mA);}
	inline void					SetColor(Color theColor, float theA) {SetColor(theColor.mR,theColor.mG,theColor.mB,theA);}
	void						SetClipColor(float theR, float theG, float theB, float theA=1.0f);
	inline void					SetClipColor(float theA=1.0f) {SetClipColor(1,1,1,theA);}
	inline void					SetClipColor(Color theColor) {SetClipColor(theColor.mR,theColor.mG,theColor.mB,theColor.mA);}
	inline void					SetClipColor(Color theColor, float theA) {SetClipColor(theColor.mR,theColor.mG,theColor.mB,theA);}
	inline void					ClipColor(float theR, float theG, float theB, float theA=1.0f) {SetClipColor(theR,theG,theB,theA);}
	inline void					ClipColor(float theA=1.0f) {SetClipColor(1,1,1,theA);}
	inline void					ClipColor(Color theColor) {SetClipColor(theColor.mR,theColor.mG,theColor.mB,theColor.mA);}
	inline void					ClipColor(Color theColor, float theA) {SetClipColor(theColor.mR,theColor.mG,theColor.mB,theA);}
	inline void					ClipColorI(float theR, float theG, float theB, float theA=255.0f) {ClipColor(theR/255.0f,theG/255.0f,theB/255.0f,theA/255.0f);}
	inline void					ClipIColor(float theR, float theG, float theB, float theA=255.0f) {ClipColor(theR/255.0f,theG/255.0f,theB/255.0f,theA/255.0f);}
	Color						mColor;
	Color						mDrawColor;	// mColor with clip factored in
	Color						mClipColor;
	int							mColorInt;
	inline Color				GetColor() {return mColor;}
	inline Color				GetClipColor() {return mClipColor;}
	//
	// We can disable setcolors if we need to.  We needed to to make Planewalker work in Solomon Dark
	//
	bool						mDisableSetcolor;
	inline void					DisableSetColor() {mDisableSetcolor=true;}
	inline void					EnableSetColor() {mDisableSetcolor=false;}
	inline void					LockColor() {mDisableSetcolor=true;}
	inline void					UnlockColor() {mDisableSetcolor=false;}


	signed char					mRenderMode;
	signed char					mLastRenderMode;
	inline void					RenderNormal() {mRenderMode=0;FreshenRenderMode();}
	inline void					RenderAdditive() {mRenderMode=1;FreshenRenderMode();}
	inline void					RenderMultiply() {mRenderMode=2;FreshenRenderMode();}
	void						FreshenRenderMode();

#ifndef USEPIPELINE
	bool						mRenderWhite;
	inline bool					IsRenderWhite() {return mRenderWhite;}
	bool						mLastRenderWhite;
	inline void					RenderWhite(bool theState=true) {mRenderWhite=theState;FreshenRenderMode();}

	float						mSaturation;
	float						mLastSaturation;
	inline void					RenderSaturation(float theSaturation=1.0f) {mSaturation=theSaturation;FreshenRenderMode();}
	inline void					SetSaturation(float theSaturation=1.0f) {mSaturation=theSaturation;FreshenRenderMode();}

	float						mBlur;
	float						mLastBlur;
	inline void					RenderBlur(float theBlur=0.0f) {mBlur=theBlur;FreshenRenderMode();}
	inline void					SetBlur(float theBlur=0.0f) {mBlur=theBlur;FreshenRenderMode();}
#else
	inline void					RenderWhite(bool theState=true) {if (theState) mFlags|=FLAG_RENDERWHITE;else UNSETBITS(mFlags,FLAG_RENDERWHITE);FreshenRenderMode();}
	inline bool					IsRenderWhite() {return (mFlags&FLAG_RENDERWHITE)!=0;}
	inline void					RenderSaturation(float theSaturation=1.0f) {FlushDrawBuffer();mSaturation=theSaturation;if (mSaturation<.99999f) mFlags|=FLAG_RENDERGREY;else UNSETBITS(mFlags,FLAG_RENDERGREY);FreshenRenderMode();}
	float						mSaturation=1.0f;
#endif


	//
	// Turn filtering on/off
	//
	bool						mFilter;
	void						Filter(bool theState=true, bool noFilterIfZoomed=true);
	void						Sharpen(bool theState=false, bool noSharpenIfZoomed=true) {Filter(!theState,noSharpenIfZoomed);}

	//
	// Texture modes
	//
	char						mTextureMode;
	char						mLastTextureMode;
	void						ClampTextures();
	void						WrapTextures();

	//
	// For render to texture...
	//
	void						RenderTo(RenderToSprite& theSprite);
	inline void					SetRenderTarget(RenderToSprite& theSprite) {RenderTo(theSprite);}
	void						RenderTo();
	inline void					SetRenderTarget() {RenderTo();}
	void						SetRenderToCamera(int thePageWidth, int thePageHeight, int theResolutionWidth, int theResolutionHeight);

	RenderToSprite*				mRenderToSprite;
	Stack<RenderToSprite*>		mRenderToSpriteStack;
	void						PushRenderTo() {mRenderToSpriteStack.Push(mRenderToSprite);}
	void						PopRenderTo()
	{
		mRenderToSprite=mRenderToSpriteStack.Pop();
		if (mRenderToSprite) RenderTo(*mRenderToSprite);
		else RenderTo();
	}
    
    //
    // In extreme cases, lets us get the renderto pixels
    // You must call this WHILE rendering to something... it won't
    // work unless you are currently rendering to the thing in quesiton.
    // (Put in for the lightmaps in Solomon's)
    //
    void*                       GetRenderToPixels();
	IPoint						mRenderToDimensions;

public:
	//
	// Our shape mode:
	// 0 = pure triangle list
	// 1 = indexed
	//
#ifndef USEPIPELINE

	char					mShapeMode;

	//
	// For pumping vertices...
	//
	int						mMaxDrawBuffer;
	Array<Vertex2D>			mDrawBuffer;//[mMaxDrawBuffer];
	Vertex2D				*mDrawBufferPtr;
	int						mDrawBufferCounter;

	inline void				StartShape(int theTriangles) {if (mShapeMode!=0) FlushDrawBuffer(); FreshenRenderMode(); if ((mDrawBufferCounter+(theTriangles*3))>=mMaxDrawBuffer) FlushDrawBuffer(); mShapeMode=0;}
	inline void				FinishShape() {}
	void					FlushDrawBuffer();
	void					(*FlushDrawBuffer_PreProcess)(void* theArg);
	void*					mFlushDrawBuffer_PreProcessArg;
	
	void					SetFlushPreProcess(void (*theFunction)(void* theArg)=NULL, void* theArg=NULL) {FlushDrawBuffer();FlushDrawBuffer_PreProcess=theFunction;mFlushDrawBuffer_PreProcessArg=theArg;}

	//
	// For indexed primitives...
	//
	inline void				StartIndexedShape(int theTriangles) {if (mShapeMode!=1) FlushDrawBuffer(); FreshenRenderMode(); if ((mDrawBufferCounter+(theTriangles*3))>=mMaxDrawBuffer) FlushDrawBuffer();mIndexBufferStartPos=mDrawBufferCounter; mShapeMode=1;}
	inline void				FinishIndexedShape(int theIndices, ...)  {va_list aIndexPtr;va_start(aIndexPtr,theIndices);FinishIndexedShape(theIndices,aIndexPtr);va_end(aIndexPtr);}
	void					FinishIndexedShape(int theIndices, va_list& theIndexPtr);
	void					FinishIndexedShape(Array<int>& theIndexArray);
	void					FinishIndexedShape() {}

	inline void				FinishShape(int theIndices, ...)  {va_list aIndexPtr;va_start(aIndexPtr,theIndices);FinishIndexedShape(theIndices,aIndexPtr);va_end(aIndexPtr);}
	inline void				FinishShape(int theIndices, va_list& theIndexPtr) {FinishIndexedShape(theIndices,theIndexPtr);}
	inline void				FinishShape(Array<int>& theIndexArray) {FinishIndexedShape(theIndexArray);}

	inline void				AddVertexIndex(int theIndex) {mIndexBuffer[mIndexBufferCounter++]=theIndex+mIndexBufferStartPos;}
	Array<vbindex>			mIndexBuffer;
	vbindex*				mIndexBufferPtr;
	int						mIndexBufferCounter;
	int						mIndexBufferStartPos;


	//
	// Helpers
	//
	inline void				StartIndexedQuad() {StartIndexedShape(2);}
	inline void				StartQuad() {StartIndexedQuad();}
	inline void				FinishIndexedQuad() {AddVertexIndex(0);AddVertexIndex(1);AddVertexIndex(2);AddVertexIndex(2);AddVertexIndex(1);AddVertexIndex(3);}
	inline void				FinishQuad() {FinishIndexedQuad();}
    //
    // For indexed quint, first vertex is center.
    //
	inline void				StartIndexedQuint() {StartIndexedShape(4);}
	inline void				FinishIndexedQuint() {AddVertexIndex(0);AddVertexIndex(1);AddVertexIndex(2);AddVertexIndex(0);AddVertexIndex(3);AddVertexIndex(1);AddVertexIndex(0);AddVertexIndex(2);AddVertexIndex(4);AddVertexIndex(0);AddVertexIndex(4);AddVertexIndex(3);}
	//
	// This lets you just push your own buffer to the card, if you have one
	//
	void					DrawIndexedShape(int theTriangles, int theVertices, int theIndices, Vertex2D* theVertPtr, vbindex* theIndexPtr);
	inline void				DrawIndexedShape(Array<Vertex2D>& theVerts, Array<vbindex>& theIndices) {DrawIndexedShape(theIndices.Size()/3,theVerts.Size(),theIndices.Size(),theVerts.mArray,theIndices.mArray);}
	//
	// Same as above, except no translations, etc, are done.
	//
	void					DrawIndexedShapeTranslated(int theTriangles, int theVertices, int theIndices, Vertex2D* theVertPtr, vbindex* theIndexPtr);
	inline void				DrawIndexedShapeTranslated(Array<Vertex2D>& theVerts, Array<vbindex>& theIndices) {DrawIndexedShapeTranslated(theIndices.Size()/3,theVerts.Size(),theIndices.Size(),theVerts.mArray,theIndices.mArray);}
	//
	// This pushes a buffer directly to the card without any processing
	//
	void					PumpShape(Vertex2D* theVertex, int theVCount);
	void					PumpShape(Vertex2DN* theVertex, int theVCount);
	void					PumpShape(Vertex2DMT* theVertex, int theVCount);
	inline void				PumpShape(Array<Vertex2D>& theVertex) {PumpShape(theVertex.mArray,theVertex.Size());}
	inline void				PumpShape(Array<Vertex2DN>& theVertex) {PumpShape(theVertex.mArray,theVertex.Size());}
	inline void				PumpShape(Array<Vertex2DMT>& theVertex) {PumpShape(theVertex.mArray,theVertex.Size());}

	void					PumpCustomShape(void* theV, int theVCount, int theVStride, Array<vbindex>& theI);

	void					PumpIndexedShape(Array<Vertex2D>& theVertex, Array<vbindex>& theIndices);
	void					PumpIndexedShape(Array<Vertex2D>& theVertex, int theVCount, Array<vbindex>& theIndices, int theICount);
	void					PumpIndexedShape(Vertex2D* theVertex, int theVCount, vbindex* theIndices, int theICount);
	inline void				PumpIndexedShape(VB& theVB) {PumpIndexedShape(theVB.mV,theVB.mVCount,theVB.mI,theVB.mICount);}

	void					PumpIndexedShape(Array<Vertex2DN>& theVertex, Array<vbindex>& theIndices);
	void					PumpIndexedShape(Array<Vertex2DN>& theVertex, int theVCount, Array<vbindex>& theIndices, int theICount);
	void					PumpIndexedShape(Vertex2DN* theVertex, int theVCount, vbindex* theIndices, int theICount);
	inline void				PumpIndexedShape(VBN& theVB) {PumpIndexedShape(theVB.mV,theVB.mVCount,theVB.mI,theVB.mICount);}

	void					PumpIndexedShape(Array<Vertex2DMT>& theVertex, Array<vbindex>& theIndices);
	void					PumpIndexedShape(Array<Vertex2DMT>& theVertex, int theVCount, Array<vbindex>& theIndices, int theICount);
	void					PumpIndexedShape(Vertex2DMT* theVertex, int theVCount, vbindex* theIndices, int theICount);
	inline void				PumpIndexedShape(VBMT& theVB) {PumpIndexedShape(theVB.mV,theVB.mVCount,theVB.mI,theVB.mICount);}


	inline Vertex2D			*GetVertexPtr() {mDrawBufferCounter++;mDrawBufferPtr++;return (mDrawBufferPtr-1);}
	inline void				AddVertex(float x, float y, int theDiffuse, float theU, float theV) {if (mClampUVCoords) {theU=mClampUVCoordsPos.mX;theV=mClampUVCoordsPos.mY;} Vertex2D *aV=GetVertexPtr();aV->mX=x+mTranslate.mX;aV->mY=y+mTranslate.mY;aV->mZ=mDefaultZ;aV->mDiffuse=theDiffuse;aV->mTextureU=theU;aV->mTextureV=theV;}
	inline void				AddVertex(Point thePos, int theDiffuse, float theU, float theV) {AddVertex(thePos.mX,thePos.mY,theDiffuse,theU,theV);}
	inline void				AddVertex(Point thePos, int theDiffuse, Point theUV) {AddVertex(thePos.mX,thePos.mY,theDiffuse,theUV.mX,theUV.mY);}
	inline void				AddVertex(float x, float y, int theDiffuse) {AddVertex(x,y,theDiffuse,0.0f,0.0f);}
	inline void				AddVertex(Point thePos, int theDiffuse) {AddVertex(thePos.mX,thePos.mY,theDiffuse,0.0f,0.0f);}

	//
	// Translate Z?
	//
	inline void				AddVertex(float x, float y, float z, int theDiffuse, float theU, float theV) {if (mClampUVCoords) {theU=mClampUVCoordsPos.mX;theV=mClampUVCoordsPos.mY;} Vertex2D *aV=GetVertexPtr();aV->mX=x+mTranslate.mX;aV->mY=y+mTranslate.mY;aV->mZ=z;aV->mDiffuse=theDiffuse;aV->mTextureU=theU;aV->mTextureV=theV;}
	inline void				AddVertex(Vector thePos, int theDiffuse, float theU, float theV) {AddVertex(thePos.mX,thePos.mY,thePos.mZ,theDiffuse,theU,theV);}
	inline void				AddVertex(Vector thePos, int theDiffuse, Point theUV) {AddVertex(thePos.mX,thePos.mY,thePos.mZ,theDiffuse,theUV.mX,theUV.mY);}
	inline void				AddVertex(float x, float y, float z, int theDiffuse) {AddVertex(x,y,z,theDiffuse,0.0f,0.0f);}
	inline void				AddVertex(Vector thePos, int theDiffuse) {AddVertex(thePos.mX,thePos.mY,thePos.mZ,theDiffuse,0.0f,0.0f);}
	inline void				AddVertex(Vertex2D theV) {AddVertex(theV.mX,theV.mY,theV.mZ,theV.mDiffuse,theV.mTextureU,theV.mTextureV);}
	inline void				AddVertex(Vertex2D theV, int theDiffuse) {AddVertex(theV.mX,theV.mY,theV.mZ,theDiffuse,theV.mTextureU,theV.mTextureV);}
#endif

	//
	// Helpers to make pumping vertices easier...
	//
#ifdef USEPIPELINE
	inline void				DrawTexturedQuad(Quad &theXY, Quad &theUV) {mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[0]),mColorInt,theUV.mCorner[0]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[1]),mColorInt,theUV.mCorner[1]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[2]),mColorInt,theUV.mCorner[2]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[3]),mColorInt,theUV.mCorner[3]);mPipeline_Normal->FinishQuad();}
	void					DrawTexturedQuad(Quad3D &theXY, Quad &theUV) {mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[0].ToPoint()),mColorInt,theUV.mCorner[0]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[1].ToPoint()),mColorInt,theUV.mCorner[1]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[2].ToPoint()),mColorInt,theUV.mCorner[2]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[3].ToPoint()),mColorInt,theUV.mCorner[3]);mPipeline_Normal->FinishQuad();}
	void					DrawTexturedQuadGradientV(Quad &theXY, Quad &theUV, Color theTop, Color theBottom) {theTop*=mColor;theBottom*=mColor;int aTop=theTop.ToInt();int aBottom=theBottom.ToInt();mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[0]),aTop,theUV.mCorner[0]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[1]),aTop,theUV.mCorner[1]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[2]),aBottom,theUV.mCorner[2]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[3]),aBottom,theUV.mCorner[3]);mPipeline_Normal->FinishQuad();}
	void					DrawTexturedQuadGradientH(Quad &theXY, Quad &theUV, Color the0, Color the1) {the0*=mColor;the1*=mColor;int a0=the0.ToInt();int a1=the1.ToInt();mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[0]),a0,theUV.mCorner[0]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[1]),a1,theUV.mCorner[1]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[2]),a0,theUV.mCorner[2]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[3]),a1,theUV.mCorner[3]);mPipeline_Normal->FinishQuad();}
	void					DrawTexturedQuadGradient(Quad &theXY, Quad &theUV, int the0, int the1, int the2, int the3) {mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[0]),the0,theUV.mCorner[0]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[1]),the1,theUV.mCorner[1]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[2]),the2,theUV.mCorner[2]);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[3]),the3,theUV.mCorner[3]);mPipeline_Normal->FinishQuad();}
	void					FillQuad(Quad &theXY) {mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[0]),mColorInt,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[1]),mColorInt,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[2]),mColorInt,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[3]),mColorInt,mClampUVCoordsPos);mPipeline_Normal->FinishQuad();}
	void					FillQuad(Quad3D &theXY) {mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[0].ToPoint()),mColorInt,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[1].ToPoint()),mColorInt,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[2].ToPoint()),mColorInt,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[3].ToPoint()),mColorInt,mClampUVCoordsPos);mPipeline_Normal->FinishQuad();}
	void					FillQuad(Quad &theXY, Color theColor1, Color theColor2, Color theColor3, Color theColor4) {Color aC1=theColor1*mColor;Color aC2=theColor2*mColor;Color aC3=theColor3*mColor;Color aC4=theColor4*mColor;mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[0]),aC1.ToInt(),mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[1]),aC2.ToInt(),mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[2]),aC3.ToInt(),mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theXY.mCorner[3]),aC4.ToInt(),mClampUVCoordsPos);mPipeline_Normal->FinishQuad();}
	inline void				DrawTexturedQuadGradient(Quad &theXY, Quad &theUV, Color the0, Color the1, Color the2, Color the3) {Color aC1=the0*mColor;Color aC2=the1*mColor;Color aC3=the2*mColor;Color aC4=the3*mColor;DrawTexturedQuadGradient(theXY,theUV,aC1.ToInt(),aC2.ToInt(),aC3.ToInt(),aC4.ToInt());}

	inline void				DrawLine(float theX1, float theY1, float theX2, float theY2, float theWidth=1) {DrawLine(Point(theX1,theY1),Point(theX2,theY2),theWidth,mColorInt,mColorInt);}
	inline void				DrawLine(Line theLine, float theWidth=1) {DrawLine(theLine.mPos[0],theLine.mPos[1],theWidth,mColorInt,mColorInt);}
	inline void				DrawLine(Point theStart, Point theEnd, float theWidth=1) {DrawLine(theStart,theEnd,theWidth,mColorInt,mColorInt);}
	inline void				DrawLine(Point theStart, Point theEnd, float theWidth, Color theStartColor, Color theEndColor) {DrawLine(theStart,theEnd,theWidth,(theStartColor*mColor).ToInt(),(theEndColor*mColor).ToInt());}
	inline void				DrawLine(Line theLine, float theWidth, Color theStartColor, Color theEndColor) {DrawLine(theLine.mPos[0],theLine.mPos[1],theWidth,(theStartColor*mColor).ToInt(),(theEndColor*mColor).ToInt());}
	inline void				DrawLine(Point theStart, Point theEnd, float theWidth, int theStartColor, int theEndColor) {float aHalfWidth=theWidth/2.0f;Point aLineVec=theStart-theEnd;Point aPerp=aLineVec.Perp();aPerp.Normalize();mPipeline_Normal->StartQuad();mPipeline_Normal->AddVertex().Set(Fix2D(theStart-(aPerp*aHalfWidth)),theStartColor,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theStart+(aPerp*aHalfWidth)),theStartColor,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theEnd-(aPerp*aHalfWidth)),theEndColor,mClampUVCoordsPos);mPipeline_Normal->AddVertex().Set(Fix2D(theEnd+(aPerp*aHalfWidth)),theEndColor,mClampUVCoordsPos);mPipeline_Normal->FinishQuad();}

	inline void				StartShape(int theTriangles) {mPipeline_Normal->StartShape(theTriangles);}
	inline void				FinishShape(int theIndices, ...)  {va_list aIndexPtr;va_start(aIndexPtr,theIndices);FinishShape(theIndices,aIndexPtr);va_end(aIndexPtr);}
	inline void				FinishShape(int theIndices, va_list& theIndexPtr) {mPipeline_Normal->FinishShape(theIndices,theIndexPtr);}
	inline void				FinishShape(Array<vbindex>& theIndexArray) {mPipeline_Normal->FinishShape(theIndexArray);}
	inline void				FinishShape() {} // Does nothing, it's just here for consistency and clarity (i.e. every StartShape matches a FinishShape)

	inline void				StartQuad() {mPipeline_Normal->StartQuad();}
	inline void				FinishQuad() {mPipeline_Normal->FinishQuad();}

	inline void				AddVertexIndex(int theIndex) {mPipeline_Normal->AddVertexIndex(theIndex);}
	inline void				AddVertex(Vx2D theV) {mPipeline_Normal->AddVertex(theV);}
	inline void				AddVertex(Vx2D theV,int theDiffuse) {theV.mColor=theDiffuse;mPipeline_Normal->AddVertex(theV);}
	inline Vx2D&			AddVertex() {return mPipeline_Normal->AddVertex();} // Remember, must manually handle gG.Translate, etc, for this!
	inline void				AddVertex(float x, float y, int theDiffuse, float theU, float theV) {AddVertex().Set(x,y,theDiffuse,theU,theV);}
	inline void				AddVertex(Point thePos, int theDiffuse, float theU, float theV) {AddVertex().Set(thePos,theDiffuse,theU,theV);}
	inline void				AddVertex(Point thePos, int theDiffuse, Point theUV) {AddVertex().Set(thePos,theDiffuse,theUV);}
	inline void				AddVertex(float x, float y, int theDiffuse) {AddVertex().Set(x,y,theDiffuse,0.0f,0.0f);}
	inline void				AddVertex(Point thePos, int theDiffuse) {AddVertex().Set(thePos,theDiffuse,0.0f,0.0f);}



#else
	void					DrawTexturedQuad(Quad &theXY, Quad &theUV);
	void					DrawTexturedQuad(Quad3D &theXY, Quad &theUV);
	void					DrawTexturedQuadGradientV(Quad &theXY, Quad &theUV, Color theTop, Color theBottom);
	void					DrawTexturedQuadGradientH(Quad &theXY, Quad &theUV, Color the0, Color the1);
	void					DrawTexturedQuadGradient(Quad &theXY, Quad &theUV, int the0, int the1, int the2, int the3);
	void					FillQuad(Quad &theXY);
	void					FillQuad(Quad3D &theXY);
	void					FillQuad(Quad &theXY, Color theColor1, Color theColor2, Color theColor3, Color theColor4);
	inline void				DrawTexturedQuadGradient(Quad &theXY, Quad &theUV, Color the0, Color the1, Color the2, Color the3) {Color aC1=the0*mColor;Color aC2=the1*mColor;Color aC3=the2*mColor;Color aC4=the3*mColor;DrawTexturedQuadGradient(theXY,theUV,aC1.ToInt(),aC2.ToInt(),aC3.ToInt(),aC4.ToInt());}

	inline void				DrawLine(float theX1, float theY1, float theX2, float theY2, float theWidth=1) {DrawLine(Point(theX1,theY1),Point(theX2,theY2),theWidth);}
	inline void				DrawLine(Line theLine, float theWidth=1) {DrawLine(theLine.mPos[0],theLine.mPos[1],theWidth);}
	void					DrawLine(Point theStart, Point theEnd, float theWidth=1);
	void					DrawLine(Point theStart, Point theEnd, float theWidth, int theStartColor, int theEndColor);
	inline void				DrawLine(Point theStart, Point theEnd, float theWidth, Color theStartColor, Color theEndColor) {DrawLine(theStart,theEnd,theWidth,(theStartColor*mColor).ToInt(),(theEndColor*mColor).ToInt());}
	inline void				DrawLine(Line theLine, float theWidth, Color theStartColor, Color theEndColor) {DrawLine(theLine.mPos[0],theLine.mPos[1],theWidth,(theStartColor*mColor).ToInt(),(theEndColor*mColor).ToInt());}

	inline void				DrawLine(Line3D theLine, float theWidth=1) {DrawLine(theLine.mPos[0],theLine.mPos[1],Vector(.577350f,.577350f,.577350f),theWidth);}
	void					DrawLine(Vector theStart, Vector theEnd);
	void					DrawLine(Vector theStart, Vector theEnd, Color theStartColor, Color theEndColor);
	void					DrawLine(Vector theStart, Vector theEnd, Vector theWidthVector, float theWidth=1);
	void					DrawTexturedLine(Point theStart, Point theEnd, Quad& theUV, float theWidth=1);
	void					DrawTexturedLine(Point theStart, Point theEnd, Quad& theUV, Color theColor1, Color theColor2, float theWidth=1);

	void					Draw2DZLine(Vector theStart, Vector theEnd, float theWidth=1, float theWidth2=-1);	// Draws a regular 2D line but includes Z element for ZBuffer
	void					DrawTexturedZQuad(Quad &theXY, Quad &theUV, float theZ);
	void					DrawCircle(Point thePos, float theRadius, float theLineWidth);
#endif


	//
	// Crude circle draw
	//
	
	//
	// Depending on the system, we sometimes need to tweak drawing
	// coordinates to make things work correctly.  For instance, in
	// DirectX, drawing goes to the CENTER of the pixel, while in OpenGL,
	// it does not.  This simply gives us a number to add to pixels
	// to specify the rendering.
	//
	float					KludgePixels();
	float					KludgeTexels();
//    int                     KludgeMipMaps();


public:
	//
	// Textures and visual states...
	//
	int						LoadTexture(String theFilename);
	int						LoadTexture(Image &theImage);
	int						LoadTexture565(Image &theImage);
	int						LoadTexture4444(Image &theImage);
	int						LoadTexture8888(int theWidth, int theHeight, void* theBits);
	int						LoadTexture565(int theWidth, int theHeight, void* theBits);
	int						LoadTexture4444(int theWidth, int theHeight, void* theBits);
	int						CreateTexture(int theWidth, int theHeight);
	int						CreateRenderer(int theWidth, int theHeight, bool theFormat, bool wantReadable=false);
	void					UnloadTexture(int theNumber);
	IPoint					GetTextureDimensions(int theNumber);
	Point					GetTextureDimensionsF(int theNumber);
	void					SetTexture(int theTexture=-1, bool guaranteeSet=false);

	//
	// Sets a multitexture stage
	//
	void					SetMultiTexture(int theStage, int theTexture, bool guaranteeSet=false);
	inline void				SetMultiTexture(int theStage, void* theNULL, bool guaranteeSet=false) {SetMultiTexture(theStage,-1,guaranteeSet);}
	inline void				SetMultiTexture(int theTexture=-1) {SetMultiTexture(1,theTexture);}

	//
	// Sets a multitexture stage to the zbuffer of a texture...
	//
	void					SetMultiTextureZ(int theStage, int theTexture);

	int						mCurrentTexture;
	int						mCurrentMultiTexture[MAXTEXTURES];
	inline void				ForceNextTexture() {mCurrentTexture=-2;}
	inline void				ForceNextTexture(int theStage) {mCurrentMultiTexture[theStage]=-2;}
	//bool					mGuaranteeNextSet;

	int						mMaxTextureSize=1024;
	inline int				GetMaxTextureSize() {return mMaxTextureSize;}

#ifndef USEPIPELINE
	//
	// Shapes (vertex/index buffers)
    //
	int						CreateShape(Vertex2D* theVertexPtr, int theVertexCount, vbindex* theIndexPtr, int theIndexCount);
	inline int				CreateShape(Array<Vertex2D>& theVertices, Array<vbindex>& theIndexes) {return CreateShape(theVertices.mArray,theVertices.Size(),theIndexes.mArray,theIndexes.Size());}

	int						CreateShape(Vertex2DN* theVertexPtr, int theVertexCount, vbindex* theIndexPtr, int theIndexCount);
	inline int				CreateShape(Array<Vertex2DN>& theVertices, Array<vbindex>& theIndexes) {return CreateShape(theVertices.mArray,theVertices.Size(),theIndexes.mArray,theIndexes.Size());}

	int						CreateShape(Vertex2DMT* theVertexPtr, int theVertexCount, vbindex* theIndexPtr, int theIndexCount);
	inline int				CreateShape(Array<Vertex2DMT>& theVertices, Array<vbindex>& theIndexes) {return CreateShape(theVertices.mArray,theVertices.Size(),theIndexes.mArray,theIndexes.Size());}

	void					DrawShape(void* theVerts, int theVCount);
	void					DrawShape(void* theVerts, int theVCount, vbindex* theI, int theICount);
	inline void				DrawShape(void* theVerts, int theVCount, Array<vbindex>& theI) {DrawShape(theVerts,theVCount, theI.mArray,theI.Size());}
	void					DrawShape(int theShape);
#endif
	void					UnloadShape(int theShape);

	//
	// To reduce texture swaps, especially on slower machines, each of our spritebundles will (hopefully!) register
	// a solid white square texture called Fillrect.  When we SetTexture(NULL), if we find a Fillrect matching our
	// current texture, then what we'll do is force all texture coordinates to clamp into the fillrect of the current texture,
	// and not actually set the texture NULL.
	// 
	// We don't need to ever remove them from this list once registered, because if we have a texture set that's been unloaded,
	// we'll be crashing for other reasons than this.
	//
	bool					mUseTexturedFills;
	inline void				UseTexturedFills(bool theState=true) {mUseTexturedFills=theState;}

	CleanArray<Sprite*>		mFillrect_List;
	bool					mClampUVCoords;
	Point					mClampUVCoordsPos;

//	void					RegisterFillrect(Sprite* theSprite) {mFillrect_List[mFillrect_List.Size()]=theSprite;}
	void					RegisterFillrect(Sprite* theSprite);

	//
	// Set this to true if you want to save video memory by using any 16-bit versions of graphics you have
	// around.  It'll default to system normal, which will be different for each system (usually off for PCs,
	// but on for iDevices of low caliber, and android)
	//
	bool					mSaveVideoMemory;
	inline void				SaveVideoMemory(bool theState) {mSaveVideoMemory=theState;}
	bool					SuggestSaveVideoMemory(unsigned int theAmount);

	void					Go2D();

	//
	// Some information that lets us get info from the tech-end so we can do mystical operations
	//
	//bool					mNeedDrawInfo;
	Matrix					mDrawMatrix;
	Matrix					mBillboardMatrix;
	Matrix					mWorldViewMatrix;
	Viewport				mViewport;

	//
	// Billboard levels-- for pre-caching Billboards, this is how many transparency levels it will max out
	// at (default is 8)
	//
	int						mBillboardLevels;
	inline void				SetBillboardLevels(int theLevels) {mBillboardLevels=theLevels;}
	inline int				GetBillboardLevels() {return mBillboardLevels;}

	Matrix					mLast3DProjectionMatrix;
	Matrix					mLast3DViewMatrix;
	Matrix					mLast3DDrawMatrix;
	Matrix					mLast3DWorldMatrix;
	Viewport				mLast3DViewport;

	Matrix&					GetLast3DViewMatrix() {return mLast3DViewMatrix;}
	Matrix&					GetLast3DDrawMatrix() {return mLast3DDrawMatrix;}
	Matrix&					GetLast3DWorldMatrix() {return mLast3DWorldMatrix;}
	Matrix&					GetLast3DProjectionMatrix() {return mLast3DProjectionMatrix;}
	Viewport&				GetLast3DViewport() {return mLast3DViewport;}


	
	//void					GetDrawInfo();
	Vector					ManualTransform(Vector thePos);
	Vector					ManualTransform(Vector thePos, Matrix& theMatrix, Viewport& theViewport);

	void					ManualTransform(Vector* theVectors, int theVectorCount);
	inline void				ManualTransform(Array<Vector>& theVectors) {ManualTransform(&theVectors[0],theVectors.Size());}

	void					ManualTransform(Vector* theVectors, int theVectorCount, Matrix& theMatrix, Viewport& theViewport);
	inline void				ManualTransform(Array<Vector>& theVectors, Matrix& theMatrix, Viewport& theViewport) {ManualTransform(&theVectors[0],theVectors.Size(),theMatrix,theViewport);}

	void					ManualTransform(Array<Vector>& theVectorsIn, Array<Vector>& theVectorsOut);
	void					ManualTransform(Array<Vector>& theVectorsIn, Array<Vector>& theVectorsOut, Matrix& theMatrix, Viewport& theViewport);

	//
	// Helper function... for manual transforms, DirectX and OpenGL treat Z differently.
	// This lets you not have to think about it.  Technically should go in the core?
	//
	inline float			FixZ(float theZ)
	{
		#ifdef USING_DIRECTX
		return (theZ)*mZRange.mY;
		#else
		return (theZ*0.5f+0.5f)*mZRange.mY;
		#endif
	}

	Matrix&					GetDrawMatrix();
	Matrix&					GetBillboardMatrix();
	Matrix&					GetWorldViewMatrix();
	Viewport				GetViewport();

	Matrix&					GetWorldMatrix();
	Matrix&					GetViewMatrix();
	Matrix&					GetProjectionMatrix();

	Matrix&					GetWorldMatrixN();
	Matrix&					GetViewMatrixN();
	Matrix&					GetProjectionMatrixN();

	Matrix					GetTextureMatrix();

	Line3D					ScreenPosToRay(int x, int y);	// RAYCAST, CAST RAY, 
	inline Line3D			ScreenPosToRay(Point thePos) {return ScreenPosToRay((int)thePos.mX,(int)thePos.mY);}

	//
	// You can universally scale all billboards with this, to make them a uniform size.
	// For instance, if your "standard size" billboard is 64x64 pixels, then SetBillboardScale(1.0f/64.0f) is what gets that about 1:1
	//
	float					mBillboardScale;
	inline void				SetBillboardScale(float theScale) {mBillboardScale=theScale;}
	inline float			GetBillboardScale() {return mBillboardScale;}



public:
	//
	// New Stuff!
	// 3D Mode...
	//
	void					ZEnable();
	void					ZDisable();
	void					ZEnableWrite();
	void					ZDisableWrite();

	void					ZReverse();			// Reverses order of z-buffer (for drawing obscured things)
	void					ZNormal();			// Makes z-ordering normal (default when you enable Z)
	void					ZEqual();			// Draws only when z-order is equal
	void					ZLess();			// Draws only when z-order is less
	void					ZGreaterEqual();	// Draws only when z-order is >=
	void					ZLessEqual();	// Draws only when z-order is >=


	float					ZDepth();
	inline float			GetZDepth() {return ZDepth();}
	inline void				SetDefaultZ(float theZ) {mDefaultZ=theZ;}
	float					mDefaultZ;

	//
	// ZBias moves the viewport back plane forward by theValue... this number is as compared to the zvalue you set-- for instance,
	// if you made Z-Depth 200, you can bias it by 10 and it'll act like 190 is the high point.
	// 
	// Typically, you want this value as SMALL as you can get away with... something on the order of .001f
	//
    void					SetZBias(float theValue=1.0f);	// Divids w by this value... so make it like .95 to move things BEHIND other things, or 1.05 to move things in FRONT.
	float					mZBias;	// Last ZBias

	//
	// Puts it into 3D mode...
	//
	char					mDrawMode=0;	// 0 = 2D, 1 = 3D

	void					Go3D(Vector theCameraPos, Vector theCameraLookat, Vector theUpVector, float theFOV);
	Vector					mLookAt;
	//
	// Just pans the display, when in 3D mode.
	// Call this BEFORE calling Go3D
	//
	void					Set3DOffset(float theX, float theY);

	void					FogEnable(Color theColor, float theStart, float theEnd);
	void					FogDisable();

	//
	// Modifies the alpha threshold
	//
	void					SetAlphaThreshold(float theThreshold=.001f);
	float					mAlphaThreshold=.001f;

	//
	// Returns the RGBA order
	//
	String					GetRGBOrder();
    
    //
    // For Threading... this is only needed in OpenGL.  If you start a new thread that does graphics, you want
    // to call enable when you start the thread, disable when you finish it.  This must also be called in the main
    // thread in order for it to work.
    //
    void                    Threading();                // If you do graphics work within a thread, call this at the start of the thread.
    void		            ThreadLock(bool theState);	// This helps threaded graphics to not interfere with eachother.  Important for OpenGL, not for DirectX

	//
	// Returns whether the graphic screen qualifies as "small" (I.E. a phone)
	//
	bool					IsSmallScreen();

	//
	// Allows letterboxing...
	// NOTE:
	// Letterboxing is pretty kludged.  It's only in here for compatibility with older games that
	// can't work with resolution switches (like Solomon's Keep + Boneyard).  It can cause clipping
	// problems if you get fancy with the clipping, so avoid letterboxing except for legacy compatibility.
	// Any use of ClipInto, FreezeClip, and GetClipRect is affected.
	//
	void					Letterbox(int theLetterboxSize) {mLetterboxSize.mX=0;mLetterboxSize.mY=(float)theLetterboxSize;}
	void					LetterboxPortrait(int theLetterboxSize) {mLetterboxSize.mX=(float)theLetterboxSize;mLetterboxSize.mY=0;}
	Point					mLetterboxSize;
	Stack<Point>			mLetterboxSizeStack;

	//
	// Shaders...
	//
	bool					mNoShaders=false;
	void					SetShader(Shader* theShader=NULL);
	Shader*					mCurrentShader=NULL;
	Stack<Shader*>			mShaderStack;
	void					PushShader() {mShaderStack.Push(mCurrentShader);}
	void					PopShader() {if (mShaderStack.GetStackCount()>0) SetShader(mShaderStack.Pop());else SetShader(NULL);}

	//
	// Misc shaderdata so we don't need to declare things constantly!
	//
	float4					mShaderData[3];
	void					SetShaderData(int theSlot, float theD1, float theD2, float theD3, float theD4);

	void					SetShaderData(int theSlot, float theD1) {SetShaderData(theSlot,theD1,mShaderData[theSlot][1],mShaderData[theSlot][2],mShaderData[theSlot][3]);}
	void					SetShaderData(int theSlot, float theD1, float theD2) {SetShaderData(theSlot,theD1,theD2,mShaderData[theSlot][2],mShaderData[theSlot][3]);}
	void					SetShaderData(int theSlot, float theD1, float theD2, float theD3) {SetShaderData(theSlot,theD1,theD2,theD3,mShaderData[theSlot][3]);}

	inline void				SetShaderData(int theSlot, Point theD) {SetShaderData(theSlot,theD.mX,theD.mY);}
	inline void				SetShaderData(int theSlot, Vector theD) {SetShaderData(theSlot,theD.mX,theD.mY,theD.mZ);}
	inline void				SetShaderData(int theSlot, Rect theRect) {SetShaderData(theSlot,theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	inline void				SetShaderData(int theSlot, Vector theD, float theW) {SetShaderData(theSlot,theD.mX,theD.mY,theD.mZ,theW);}
	float4&					ShaderData(int theSlot) {return mShaderData[theSlot];}

	inline void				SetShaderData(float theD1) {SetShaderData(0,theD1);}
	inline void				SetShaderData(float theD1, float theD2) {SetShaderData(0,theD1,theD2);}
	inline void				SetShaderData(float theD1, float theD2, float theD3) {SetShaderData(0,theD1,theD2,theD3);}
	inline void				SetShaderData(float theD1, float theD2, float theD3, float theD4) {SetShaderData(0,theD1,theD2,theD3,theD4);}
	inline void				SetShaderData(Point theD) {SetShaderData(0,theD);}
	inline void				SetShaderData(Vector theD) {SetShaderData(0,theD);}
	inline void				SetShaderData(Rect theRect) {SetShaderData(0,theRect);}
	inline void				SetShaderData(Vector theD, float theW) {SetShaderData(0,theD,theW);}
	inline float4&			ShaderData() {return ShaderData(0);}

	inline void				SetShaderData1(float theD1) {SetShaderData(1,theD1);}
	inline void				SetShaderData1(float theD1, float theD2) {SetShaderData(1,theD1,theD2);}
	inline void				SetShaderData1(float theD1, float theD2, float theD3) {SetShaderData(1,theD1,theD2,theD3);}
	inline void				SetShaderData1(float theD1, float theD2, float theD3, float theD4) {SetShaderData(1,theD1,theD2,theD3,theD4);}
	inline void				SetShaderData1(Point theD) {SetShaderData(1,theD);}
	inline void				SetShaderData1(Vector theD) {SetShaderData(1,theD);}
	inline void				SetShaderData1(Rect theRect) {SetShaderData(1,theRect);}
	inline void				SetShaderData1(Vector theD, float theW) {SetShaderData(1,theD,theW);}
	inline float4&			ShaderData1() {return ShaderData(1);}

	inline void				SetShaderData2(float theD1) {SetShaderData(2,theD1);}
	inline void				SetShaderData2(float theD1, float theD2) {SetShaderData(2,theD1,theD2);}
	inline void				SetShaderData2(float theD1, float theD2, float theD3) {SetShaderData(2,theD1,theD2,theD3);}
	inline void				SetShaderData2(float theD1, float theD2, float theD3, float theD4) {SetShaderData(2,theD1,theD2,theD3,theD4);}
	inline void				SetShaderData2(Point theD) {SetShaderData(2,theD);}
	inline void				SetShaderData2(Vector theD) {SetShaderData(2,theD);}
	inline void				SetShaderData2(Rect theRect) {SetShaderData(2,theRect);}
	inline void				SetShaderData2(Vector theD, float theW) {SetShaderData(2,theD,theW);}
	inline float4&			ShaderData2() {return ShaderData(2);}

};

void SetVertex(Vertex2D& theVertex, float x, float y, int theDiffuse, float u, float v);
void SetVertex(Vertex2D& theVertex, Point thePos, int theDiffuse, float u, float v);
void SetVertex(Vertex2D& theVertex, Point thePos, int theDiffuse, Point theUV);
void SetVertex(Vertex2D& theVertex, Point thePos, int theDiffuse);


#define SHADERHOOK(func) [&](Shader* theShader) func
#define SHADERHOOKPTR std::function<void(Shader* theShader)>

class Shader
{
public:
	virtual void				Load() {}
	void						DoLoad();

	bool						mLoaded=false;
	inline bool					IsLoaded() {if (!mLoaded) DoLoad();return true;}

	int							mShader=-1;

	//
	// A static single light dir for all shaders... so if we ever need it, it's here.
	//
	static Vector				mLightDir;
	inline void					SetLightDir(Vector theDir) {mLightDir=theDir.Normal();}


	//
	// Override for if you want to set shaders in a custom, universal way...
	//
	virtual void				Customize(void* theData) {}

	class ShaderVariable
	{
	public:
		ShaderVariable(String theName, int theRef, int theType) {mName=theName;mRef=theRef;mType=theType;}
		String	mName;	// Name of variable
		int		mRef;	// Reference
		int		mType;	// Pixel or Vertex
	};

	SmartList(ShaderVariable)	mVariableList;
	Smart(ShaderVariable)		AddShaderVariable(String theName, int theRef) {Smart(ShaderVariable) aV=new ShaderVariable(theName,theRef,mCurrentShaderType);mVariableList+=aV;return aV;}

	void						Load(String theVShader, String thePShader);
	void						Load(String theRShader);
	inline void					Load(String theVShader, String thePShader, Array<char*> theVertexFormat) {mVertexFormat.Copy(theVertexFormat);Load(theVShader,thePShader);}

	String						MakeHLSL(String theStr);
	void						FixHLSLCodeBlock(String& theStr, bool killOutput);
	String						MakeGLSL(String theStr);
	void						FixGLSLCodeBlock(String& theStr);
	void						FixHLSLType(String& theType);
	String						GetHLSLTypeSuffix(String theType);
	int							GetHLSLTypeSize(String& theType);
	void						FixGLSLType(String& theType);
	char*						TypeToVertexFormat(String theType);

#ifdef _DEBUG
	String						mName;	// Shader's name (just for debugging)
#endif
	char						mCurrentShaderType=0;	// 0 = vertex, 1 = pixel (for converting to HLSL or GLSL)
	int							mColorCounter=0;
	int							mTextureCounter=0;
	int							mDepthCounter=0;
	Array<char*>				mVertexFormat;			// The vertex format

	struct DrawInfoStruct
	{
		int							mFlags=0;
		int							mWorldMatrix=-1;
		int							mViewMatrix=-1;
		int							mProjectionMatrix=-1;
		int							mWorldMatrixN=-1;
		int							mViewMatrixN=-1;
		int							mProjectionMatrixN=-1;
		int							mComboMatrix=-1;
		int							mBillboardMatrix=-1;
		int							mWorldViewMatrix=-1;
		int							mColor=-1;
		int							mZBias=-1;
		int							mAlphaThreshold=-1;
		int							mShaderData[3]={-1,-1,-1};
		int							mLookAt=-1;
		int							mTime=-1;
		int							mRez=-1;
		int							mGFlags=-1;
		int							mSaturation=-1;
		// NEWSVAR NewShaderVariable New Shader Variable
	} mDrawInfo[2];

	unsigned int					mRehup=0xFFFFFFFF;
	Array<int>						mTextureRef;		// Texture references (GL only)

	void						GoLive();
	void						Rehup();
	void						Rehup(int theShaderType);
	inline void					Freshen() {GoLive();}

	SHADERHOOKPTR				mSetShaderHook=NULL;
	inline void					SetShaderHook(SHADERHOOKPTR theHook) {mSetShaderHook=theHook;}

	virtual int					GetGlobalRef(String theVariable);
	virtual void				SetGlobal(int theVariable, float* theValue, int theCount=1);
	virtual void				SetGlobal(int theVariable, int* theValue, int theCount=1);
	virtual void				SetGlobal(int theVariable, Matrix* theValue);
	inline void					SetGlobal(int theVariable, float theValue) {SetGlobal(theVariable,&theValue);}
	inline void					SetGlobal(int theVariable, int theValue) {SetGlobal(theVariable,&theValue);}
	inline void					SetGlobal(int theVariable, Matrix& theValue) {SetGlobal(theVariable,&theValue);}

	virtual void				SetTexture(int theTexture);
	virtual void				SetTexture(Sprite* theSprite);
	virtual void				DrawShape(void* theVerts, int theVCount);
	virtual void				DrawShape(void* theVerts, int theVCount, vbindex* theI, int theICount);
	inline void					DrawShape(void* theVerts, int theVCount, Array<vbindex>& theI) {DrawShape(theVerts,theVCount, theI.mArray,theI.Size());}
	virtual void				DrawShape(int theShape);
	virtual void				DrawModel(Model& theModel, Matrix* theMatrix=NULL);
	inline void					Draw(Model& theModel, Matrix* theMatrix=NULL) {DrawModel(theModel,theMatrix);}

	void						Preload(Model& theModel); // Preloads a model with this shader (just so it doesn't happen during draw)

	virtual int					CreateShape(void* theVerts, int theVCount, vbindex* theI, int theICount);
	inline int					CreateShape(void* theVerts, int theVCount, Array<vbindex>& theI) {return CreateShape(theVerts,theVCount, theI.mArray,theI.Size());}

	//
	// For loading Models...
	//
	int							LoadModel(IOBuffer& theBuffer, Sprite* theSprite=NULL, Cube* receiveBounds=NULL, Array<Key3D>* aKeyList=NULL);
	virtual int					StreamShape(IOBuffer& theBuffer, Quad* theUVQuad=NULL) {return -1;}
	//
	// Helps us resolve standard shader globals (for GL, where they're from variable name...)
	//
	struct ResolveGlobal
	{
		ResolveGlobal() {mPtr=NULL;}
		ResolveGlobal(String theName, int* thePtr) {mName=theName;mPtr=thePtr;}
		String mName;
		int* mPtr;
	};
	Array<ResolveGlobal>		mResolveList;


#ifdef _DEBUG
	String						mVShaderString;
	String						mPShaderString;
#endif
};

//
// Quickie helper... indicia for a four-vertex square...
// We have six in there so it can be used for a cube if necessary...
// 
//
#define VERTEXINDEX4 {0,1,2,2,1,3}//, 0+4,1+4,2+4,2+4,1+4,3+4, 0+8,1+8,2+8,2+8,1+8,3+8, 0+12,1+12,2+12,2+12,1+12,3+12, 0+16,1+16,2+16,2+16,1+16,3+16, 0+20,1+20,2+20,2+20,1+20,3+20}	
#define VERTEXINDEX4CW {0,2,1,1,2,3}//, 0+4,1+4,2+4,2+4,1+4,3+4, 0+8,1+8,2+8,2+8,1+8,3+8, 0+12,1+12,2+12,2+12,1+12,3+12, 0+16,1+16,2+16,2+16,1+16,3+16, 0+20,1+20,2+20,2+20,1+20,3+20}	
#define VERTEXINDEX4CUBE {0,1,2,2,1,3, 0+4,1+4,2+4,2+4,1+4,3+4, 0+8,1+8,2+8,2+8,1+8,3+8, 0+12,1+12,2+12,2+12,1+12,3+12, 0+16,1+16,2+16,2+16,1+16,3+16, 0+20,1+20,2+20,2+20,1+20,3+20}
static const vbindex gIndex4[]=VERTEXINDEX4;
inline vbindex* Index4Helper() {return (vbindex*)gIndex4;}
inline Array<vbindex>& Index4Array() {static Array<vbindex>aIndex;return aIndex;}

#include "rapt_shaders.h"
#include "rapt_model.h"
//
// A universal "debug draw" helper that wec can derive from for any purpose...
//
class GXObj{public:virtual void Draw() {}};
class GXPoint : public GXObj {public:Vector mPos;Color mColor;float mSize;virtual void Draw();};
class GXPoint3D : public GXObj {public:Vector mPos;Color mColor;float mSize;virtual void Draw();};
class GXLine : public GXObj {public:Line mLine;Color mColor;float mSize;virtual void Draw();};
class GXLine3D : public GXObj {public:Line3D mLine;Color mColor;float mSize;virtual void Draw();};
class GXTri : public GXObj {public:Triangle mTri;Color mColor;float mSize;virtual void Draw();};
class GXTri3D : public GXObj {public:Triangle3D mTri;Color mColor;float mSize;virtual void Draw();};
class GXFilledTri : public GXObj {public:Triangle mTri;Color mColor;float mSize;virtual void Draw();};
class GXFilledTri3D : public GXObj {public:Triangle3D mTri;Color mColor;float mSize;virtual void Draw();};
class GXText : public GXObj {public:Vector mPos;Color mColor;Color mBKGColor;String mText;Rect mRect;virtual void Draw();};
class GXText3D : public GXObj {public:Vector mPos;Color mColor;Color mBKGColor;String mText;Rect mRect;virtual void Draw();};
class GXPlane3D : public GXObj {public:Plane mPlane;Vector mPlanePerp[2];Color mColor;float mSize;virtual void Draw();};
class GXDirLine3D : public GXObj {public:Line3D mLine;Color mColor[2];float mSize;virtual void Draw();};
class GX
{
public:
	SmartList(GXObj) mList;
	inline void Clear() {mList.Clear();}
	inline void Reset() {mList.Clear();}

	inline void Add(Smart(GXObj) theDD) {mList+=theDD;}
	Smart(GXObj) AddPoint(Point thePos,Color theColor=Color(1),float theSize=5);
	Smart(GXObj) AddPoint3D(Vector thePos,Color theColor=Color(1),float theSize=5);

	Smart(GXObj) AddLine(Point thePos1, Point thePos2,Color theColor=Color(1),float theSize=1);
	Smart(GXObj) AddLine3D(Vector thePos1, Vector thePos2,Color theColor=Color(1),float theSize=1);
	inline Smart(GXObj) AddLine(Line theLine,Color theColor=Color(1),float theSize=5) {return AddLine(theLine.mPos[0],theLine.mPos[1],theColor,theSize);}
	inline Smart(GXObj) AddLine3D(Line3D theLine,Color theColor=Color(1),float theSize=5) {return AddLine3D(theLine.mPos[0],theLine.mPos[1],theColor,theSize);}

	Smart(GXObj) AddDirLine3D(Vector thePos1, Vector thePos2,Color theColor1=Color(0,0,0,1),Color theColor2=Color(1),float theSize=1);
	inline Smart(GXObj) AddDirLine3D(Line3D theLine,Color theColor1=Color(0,0,0,1),Color theColor2=Color(1),float theSize=5) {return AddDirLine3D(theLine.mPos[0],theLine.mPos[1],theColor1,theColor2,theSize);}

	inline Smart(GXObj) AddRay(Point thePos1, Point theDir,Color theColor=Color(1),float theSize=1) {return AddLine(thePos1,thePos1+theDir,theColor,theSize);}
	inline Smart(GXObj) AddRay3D(Vector thePos1, Vector theDir,Color theColor=Color(1),float theSize=1) {return AddLine3D(thePos1,thePos1+theDir,theColor,theSize);}


	Smart(GXObj) AddTri(Point thePos1, Point thePos2, Point thePos3, Color theColor=Color(1),float theSize=1);
	Smart(GXObj) AddTri3D(Vector thePos1, Vector thePos2, Vector thePos3, Color theColor=Color(1),float theSize=1);
	inline Smart(GXObj) AddTri(Triangle theT, Color theColor=Color(1),float theSize=5) {return AddTri(theT.mPos[0],theT.mPos[1],theT.mPos[2],theColor,theSize);}
	inline Smart(GXObj) AddTri3D(Triangle3D theT, Color theColor=Color(1),float theSize=5) {return AddTri3D(theT.mPos[0],theT.mPos[1],theT.mPos[2],theColor,theSize);}
	inline Smart(GXObj) AddTri3D(Triangle3DPtr theT, Color theColor=Color(1),float theSize=5) {return AddTri3D(*theT.mPos[0],*theT.mPos[1],*theT.mPos[2],theColor,theSize);}

	Smart(GXObj) AddFilledTri(Point thePos1, Point thePos2, Point thePos3, Color theColor=Color(1),float theSize=1);
	Smart(GXObj) AddFilledTri3D(Vector thePos1, Vector thePos2, Vector thePos3, Color theColor=Color(1),float theSize=1);
	inline Smart(GXObj) AddFilledTri(Triangle theT, Color theColor=Color(1),float theSize=5) {return AddFilledTri(theT.mPos[0],theT.mPos[1],theT.mPos[2],theColor,theSize);}
	inline Smart(GXObj) AddFilledTri3D(Triangle3D theT, Color theColor=Color(1),float theSize=5) {return AddFilledTri3D(theT.mPos[0],theT.mPos[1],theT.mPos[2],theColor,theSize);}
	inline Smart(GXObj) AddFilledTri3D(Triangle3DPtr theT, Color theColor=Color(1),float theSize=5) {return AddFilledTri3D(*theT.mPos[0],*theT.mPos[1],*theT.mPos[2],theColor,theSize);}


	Smart(GXObj) AddPlane3D(Vector thePos, Vector theNormal, Color theColor=Color(1), float theSize=1);
	inline Smart(GXObj) AddPlane3D(Plane thePlane, Color theColor=Color(1), float theSize=1) {return AddPlane3D(thePlane.mPos,thePlane.mNormal,theColor,theSize);}

	void AddCube(Cube theCube, Color theColor=Color(1),float theSize=1);

	Smart(GXObj) AddText(Point thePos,String theString, Color theColor=Color(1), Color theBKGColor=Color(0));
	Smart(GXObj) AddText3D(Vector thePos,String theString, Color theColor=Color(1), Color theBKGColor=Color(0));

	void Draw();

	Vector mOffset;
	inline void SetOffset(Vector theVector=Vector(0,0,0)) {mOffset=theVector;}

	bool mEnabled=true;
	inline void Enable() {mEnabled=true;}
	inline void Disable() {mEnabled=false;}
};

GX& GetGX();
#define gGX GetGX()

//
// Pipeline: A buffer of vertices that go with a specific shader/vertex.  Intended to eventually replace the normal draw commands in gG.  Only valid if GL30 is defined.
// The idea is to pile a bunch of vertices in when you use a particular shader and then flush it.  Basically, this reproduces all the draw functionality of gG, and gG will
// eventually toss it.
//
#ifdef USEPIPELINE
template <typename vtype> void Pipeline<vtype>::AddVertex(vtype theV) {theV.mPos=gG.Fix2D(theV.mPos);mKit->mV[mKit->mVCount++]=theV;}
#endif
#endif //RAPT_GRAPHICS_H

#ifdef RAPT_GRAPHICS_CPP

#include "os_core.h"
#include "graphics_core.h"
#include "rapt_sprite.h"

#include "rapt_app.h"
#include "os_core.h"

//#define DEBUG(n) gOut.Out(n)
#define DEBUG(n) 
#ifdef USEPIPELINE
#define Flush() {if (mCurrentPipeline) mCurrentPipeline->Flush();}
#else
#define Flush() {if (mDrawBufferCounter>0) FlushDrawBuffer();}
#endif

//#define TRACKSTATECHANGES

#ifdef TRACKSTATECHANGES
int gStateChangeCount;
#endif

Graphics::Graphics()
{
	mResolutionWidth=800;
	mResolutionHeight=600;
	mAutoPageSize=true;
    mAutoPageSizeMultiplier=1.0f;
	mLetterboxSize=Point(0,0);
	mClampHeight=0;
	mSaveVideoMemory=false;
	mDefaultZ=1.0f;
	mCullState=0;
	mRehupDrawInfo=0xFFFFFFFF;
	mBillboardScale=1.0f;
	mTextureMode=0;
	mLastTextureMode=-1;
	mBillboardLevels=8;
#ifndef USEPIPELINE
	FlushDrawBuffer_PreProcess=NULL;
#endif
	
	Index4Array().GuaranteeSize(6);
	for (int aCount=0;aCount<6;aCount++) Index4Array()[aCount]=gIndex4[aCount];

//	SetPageSize(mResolutionWidth,mResolutionHeight);
	mClampUVCoords=false;
	mDisableSetcolor=false;

	mLastRenderMode=-1;

#ifndef USEPIPELINE
	mRenderWhite=false;mLastRenderWhite=false;
	mSaturation=1.0f;mLastSaturation=1.0f;
	mBlur=0.0f;mLastBlur=0.0f;
	mShapeMode=1;
#endif

	mFilter=true;

#ifndef USEPIPELINE
	RequestMaxVertices(10000);
	mDrawBufferCounter=0;
	mIndexBufferCounter=0;
	mDrawBuffer.GuaranteeSize(mMaxDrawBuffer);
	mIndexBuffer.GuaranteeSize(mMaxDrawBuffer);
#endif

	UseTexturedFills();
}

void Graphics::Go()
{
	IPoint aResolution=IPoint(-1,-1);
	bool aFullscreen=true;
	
	if (Graphics_Core::CanDoWindowed())
	{

		if (gAppPtr->mSettings.Exists("Graphics.Resolution")) aResolution=gAppPtr->mSettings.GetIPoint("Graphics.Resolution");
		aFullscreen=gAppPtr->mSettings.GetBool("Graphics.Fullscreen");

		//SetResolution(aResolution.mX,aResolution.mY,aFullscreen);

		//
		// If we don't have a resolution, load up a default resolution explicitely so that we can
		// get the highest possible resolution.
		//
		if (aResolution.mX==-1 || aResolution.mY==-1)
		{
			aResolution=Point(1024,768);
			SetResolution(1024,768,false);
		}
	}
	Graphics_Core::Startup();

	//
	// This indicates we want the BEST rez we can get...
	//
	if (aResolution.mX<=0 || aResolution.mY<=0) aResolution=GetHighestResolution(aFullscreen);
	SetResolution(aResolution.mX,aResolution.mY,aFullscreen);
	mMaxTextureSize=(uintptr_t)Graphics_Core::Query("maxtexturesize");

	//SaveVideoMemory(gAppPtr->mSettings.GetBool("SaveVideoMemory",Graphics_Core::SaveVideoMemory()));

	//
	// We don't register this here because we assume the out-core will be deciding which devices
	// should and shouldn't save video memory... for instance, and iPad2 would, but an iPad3 might not.
	//
#ifdef ALLOW_REGISTER_VARIABLES
	gAppPtr->mSettings.RegisterVariable("Graphics.SaveVideoMemory",mSaveVideoMemory,false);
#else
	mSaveVideoMemory=gAppPtr->mSettings.GetBool("Graphics.SaveVideoMemory",false);
#endif

	//gAppPtr->mSettings.Changed();  // Why did I have this?  It forces a save right on start, but it won't matter there... no?

	if (Graphics_Core::Query("NoShaders")) mNoShaders=true; // Legacy compatibility

#ifdef USING_DIRECTX
	//
	// Legacy stuff for Hoggy 2 to compile...
	//
	mNoShaders=true;
#endif

#ifndef LEGACY_GL
	if (!mNoShaders) gSh.Load();
#endif
#ifdef USEPIPELINE
	InitializePipelines();
#endif
}

void Graphics::Stop()
{
	Graphics_Core::SetTexture();
	Graphics_Core::Shutdown();
}

int Graphics::Time() {return AppTime();}


bool Graphics::SuggestSaveVideoMemory(unsigned int theAmount) {return (theAmount>=Graphics_Core::GetVRAM());}
void Graphics::Multitasking(bool isForeground) {Graphics_Core::Multitasking(isForeground);}
void Graphics::SystemNotify(String theNotify) {}

void Graphics::BeginRender()
{
	static bool aFirstTime=true;if (aFirstTime) {Graphics_Core::Initialize();aFirstTime=false;}

	UNSETBITS(mFlags,FLAG_RENDERTO);

	Graphics_Core::DisableZBuffer();
	Graphics_Core::Set2DCamera(mWidth,mHeight);
	mRehupDrawInfo|=DRAWINFO_ALL;
	Graphics_Core::BeginRender();

	mCurrentShader=NULL;
	mCurrentTexture=-1;
	for (int aCount=1;aCount<MAXTEXTURES;aCount++) mCurrentMultiTexture[aCount]=-1;
	mTextureMode=-1;
#ifndef USEPIPELINE
	mShapeMode=1;			// Default to indexed...
#endif

	EnableSetColor();
	SetTexture(-1,true);
	for (int aCount=1;aCount<MAXTEXTURES;aCount++) SetMultiTexture(aCount,-1,true);
	SetZBias();
	SetClipColor();
	SetColor();
	Filter();
	WrapTextures();

	SetViewMatrix();
	SetWorldMatrix();
	SetTextureMatrix();
	SetMultiTextureMatrix();

	Translate();
	Clip();
	FreezeClip(false);

	RenderNormal();
	RenderWhite(false);
	RenderSaturation();
	CullNone();
		
	DEBUG("Flush -> BeginRender()");
#ifdef USEPIPELINE
	Flush();
	SetPipeline(mPipeline_Normal);
#else
	FlushDrawBuffer();
#endif
	//FlushDrawBuffer();
	//mColorStack.Empty();
	//mTranslateStack.Empty();
	//mClipStack.Empty();

#ifdef TRACKSTATECHANGES
	gStateChangeCount=0;
#endif
}

void Graphics::EndRender()
{
	DEBUG("Flush -> EndRender()");

	Flush();
	SetTexture(-1,true);
	SetShader();
	Graphics_Core::EndRender();

#ifdef TRACKSTATECHANGES
	gOut.Out("State Changes: %d",gStateChangeCount);
#endif

}

void Graphics::Show()
{
	if (mLetterboxSize.LengthSquared())
	{
		BeginRender();
		gG.Translate(-mLetterboxSize/2);
		//gG.Clip(-1000,-1000,3000,3000);

		gG.SetColor(0,0,0,1);
		if (mLetterboxSize.mY)
		{
			gG.FillRect(0,0,(float)mWidth,(float)mLetterboxSize.mY/2);
			gG.FillRect(0,(float)mHeight-1-mLetterboxSize.mY/2,(float)mWidth,(float)(mLetterboxSize.mY/2)+1);
		}
		if (mLetterboxSize.mX)
		{
			gG.FillRect(0,0,(float)mLetterboxSize.mX/2,(float)mHeight);
			gG.FillRect((float)mWidth-1-mLetterboxSize.mX/2,0,(float)(mLetterboxSize.mX/2)+1,(float)mHeight);
		}

		gG.Translate();
		EndRender();
	}
	Graphics_Core::Show();
}

void Graphics::SetPageSize(int theWidth, int theHeight)
{
	mAutoPageSize=false;
	mWidth=theWidth;
	mHeight=theHeight;
	Graphics_Core::Set2DCamera(mWidth,mHeight);
	mRehupDrawInfo|=DRAWINFO_ALL;
}

void Graphics::ForcePageSizeByWidth(int theDesiredWidth)
{
	static float aRatio=(float)mResolutionHeight/(float)mResolutionWidth;
	gG.SetPageSize((int)(theDesiredWidth),(int)(theDesiredWidth*aRatio));
}

void Graphics::ForcePageSizeByHeight(int theDesiredHeight)
{
	static float aRatio=(float)mResolutionWidth/(float)mResolutionHeight;
	gG.SetPageSize((int)(theDesiredHeight*aRatio),(int)theDesiredHeight);
}

void Graphics::SetAutoPageSizeMultiplier(String theResolution, float theMultiplier)
{
	int aAddPos=mAutoPageSizeList.Size();

	Point aP=theResolution.ToPoint();
	mAutoPageSizeList[aAddPos].mResolution.mX=(int)aP.mX;
	mAutoPageSizeList[aAddPos].mResolution.mY=(int)aP.mY;
	mAutoPageSizeList[aAddPos].mMultiplier=theMultiplier;
}

IPoint Graphics::GetHighestResolution(bool isWindowed)
{
	Array<IPoint> aRezList;
	GetAvailableResolutions(aRezList);
	if (aRezList.Size() && !Graphics_Core::CanDoWindowed()) return aRezList[0];
    
    //
    // We can't count on the resolutions being in any order, so sort them from lowest at 0 to highest at max.
    //

    IPoint aResult=IPoint(800,600);

    int aBestArea=aResult.mX*aResult.mY;
    for (int aCount=0;aCount<aRezList.Size();aCount++)
    {
        if (!Graphics_Core::IsTooBigForWindowed(aRezList[aCount].mX,aRezList[aCount].mY))
        {
            int aArea=aRezList[aCount].mX*aRezList[aCount].mY;
            if (aArea>aBestArea)
            {
                aBestArea=aArea;
                aResult=aRezList[aCount];
            }
        }
    }
    
	return aResult;
}

void Graphics::SetResolution(int theWidth, int theHeight, bool isFullscreen)
{

	mFullscreen=isFullscreen;

	OS_Core::SetResolution(theWidth,theHeight,isFullscreen);
	Graphics_Core::SetResolution(theWidth,theHeight,isFullscreen);
	OS_Core::GetResolution(&mResolutionWidth,&mResolutionHeight);

	if (gAppPtr->mSettings.GetIPoint("Graphics.Resolution")!=IPoint(theWidth,theHeight)) gAppPtr->mSettings.SetIPoint("Graphics.Resolution",IPoint(theWidth,theHeight));
	if (gAppPtr->mSettings.GetBool("Graphics.Fullscreen")!=isFullscreen) gAppPtr->mSettings.SetBool("Graphics.Fullscreen",isFullscreen);

    mAutoPageSizeMultiplier=1.0f;
	if (mAutoPageSize) 
	{
		for (int aCount=0;aCount<mAutoPageSizeList.Size();aCount++) if (mAutoPageSizeList[aCount].mResolution.mX==mResolutionWidth && mAutoPageSizeList[aCount].mResolution.mY==mResolutionHeight) mAutoPageSizeMultiplier=mAutoPageSizeList[aCount].mMultiplier;

		float aWantPageWidth=(float)mResolutionWidth/mAutoPageSizeMultiplier;
		float aWantPageHeight=(float)mResolutionHeight/mAutoPageSizeMultiplier;

		//SetPageSize((int)((float)mResolutionWidth/mAutoPageSizeMultiplier),(int)((float)mResolutionHeight/mAutoPageSizeMultiplier));

		if (mClampHeight!=0)
		{
			//
			// We're clamping, so maladjust!
			//

			if (aWantPageHeight>mClampHeight)
			{
				aWantPageHeight=(float)mClampHeight;
				float aRatio=(float)mResolutionWidth/(float)mResolutionHeight;
				aWantPageWidth=((float)(aWantPageHeight*aRatio));
			}
		}

		SetPageSize((int)aWantPageWidth,(int)aWantPageHeight);
		mAutoPageSize=true;
	}
}

void Graphics::SetDefaultResolution(int theWidth, int theHeight, bool isFullscreen)
{
/*
	if (Graphics_Core::IsTooBigForWindowed(theWidth,theHeight))
	{
		//
		// Our desktop resolution is too small to run windowed.  So, we tweak our
		// default resolution to be more appropriate.
		//
		isFullscreen=true;
		int aWidth;
		int aHeight;
		Graphics_Core::GetDesktopSize(&aWidth,&aHeight);

		theWidth=_max(aWidth,theWidth);
		theHeight=_max(aHeight,theHeight);
	}
/**/

	if (!gAppPtr->mSettings.Exists("Graphics.Resolution")) gAppPtr->mSettings.SetIPoint("Graphics.Resolution",IPoint(theWidth,theHeight));
	if (!gAppPtr->mSettings.Exists("Graphics.Fullscreen")) gAppPtr->mSettings.SetBool("Graphics.Fullscreen",isFullscreen);

	//
	// I took this line out because it was causing glitchiness when starting in default fullscreen.
	// For some reason, started with defaulted fullscreen, going ito SetResolution glitched things up so
	// nothing would display.  However, there do not seem to be any side effects to leaving this out, soo....
	//
	//SetResolution(theWidth,theHeight,isFullscreen);	// Needs to stay so that page size can be matched up...
}

void Graphics::SetDefaultResolution(bool isFullscreen)
{
	int aWidth=-1;
	int aHeight=-1;

	if (!gAppPtr->mSettings.Exists("Graphics.Resolution")) gAppPtr->mSettings.SetIPoint("Graphics.Resolution",IPoint(aWidth,aHeight));
	if (!gAppPtr->mSettings.Exists("Graphics.Fullscreen")) gAppPtr->mSettings.SetBool("Graphics.Fullscreen",isFullscreen);
}


void Graphics::Clear(float theRed, float theGreen, float theBlue, float theAlpha)
{
	DEBUG("Flush -> Clear()");
	Flush();
	Graphics_Core::Clear(theRed,theGreen,theBlue,theAlpha);
}

void Graphics::ClearZ(float theValue)
{
	Flush();
	EndRender();
	Graphics_Core::ClearZ(theValue);
	BeginRender();
}

#ifndef USEPIPELINE
void Graphics::FinishIndexedShape(int theIndices, va_list& theIndexPtr)
{
//    va_list aIndexPtr;
//    va_start(aIndexPtr,theIndices);
    
	int* aPtr=&theIndices;
	for (int aCount=1;aCount<=theIndices;aCount++)
	{
      // SVERO : Changed the va_arg param from short to int... Using short there causes android to segfault
      //         Maybe something to do with how gcc compiles this vs vc... (FUCKING hard to find this bug!)
		vbindex aIndex=(vbindex)va_arg(theIndexPtr,int);//*(aPtr+aCount);
		mIndexBuffer[mIndexBufferCounter++]=aIndex+mIndexBufferStartPos;
	}
}
void Graphics::FinishIndexedShape(Array<int>& theIndexArray)
{
	for (int aCount=0;aCount<theIndexArray.Size();aCount++) mIndexBuffer[mIndexBufferCounter++]=theIndexArray[aCount]+mIndexBufferStartPos;
}
#endif

void Graphics::FlushDrawBuffer()
{
#ifdef USEPIPELINE
	Flush();
#else
	if (mDrawBufferCounter>0) 
	{
		if (FlushDrawBuffer_PreProcess!=NULL) FlushDrawBuffer_PreProcess(mFlushDrawBuffer_PreProcessArg);
		if (mShapeMode==0) Graphics_Core::DrawShape((void*)mDrawBuffer.mArray,mDrawBufferCounter/3,sizeof(Vertex2D));
		if (mShapeMode==1) Graphics_Core::DrawIndexedShape((void*)mDrawBuffer.mArray,(void*)mIndexBuffer.mArray,mDrawBufferCounter,mIndexBufferCounter/3,sizeof(Vertex2D));
	}
	mDrawBufferCounter=0;
	mIndexBufferCounter=0;
	mDrawBufferPtr=&mDrawBuffer[0];
#endif
}

#ifndef USEPIPELINE
void Graphics::PumpShape(Vertex2D* theVertex, int theVCount)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawShape(theVertex,theVCount,sizeof(Vertex2D));
}

void Graphics::PumpShape(Vertex2DN* theVertex, int theVCount)
{
	Flush();
	FreshenRenderMode();
	//Graphics_Core::DrawShapeN(theVertex,theVCount,sizeof(Vertex2DN));
}

void Graphics::PumpShape(Vertex2DMT* theVertex, int theVCount)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawShapeMT(theVertex,theVCount,sizeof(Vertex2DMT));
}


void Graphics::PumpIndexedShape(Array<Vertex2D>& theVertex, Array<vbindex>& theIndices)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShape(theVertex.mArray,theIndices.mArray,theVertex.Size(),theIndices.Size()/3,sizeof(Vertex2D));
}

void Graphics::PumpCustomShape(void* theV, int theVCount, int theVStride, Array<vbindex>& theI)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShape(theV,theI.mArray,theVCount,theI.Size()/3,theVStride);
}


void Graphics::PumpIndexedShape(Vertex2D* theVertex, int theVCount, vbindex* theIndices, int theICount)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShape(theVertex,theIndices,theVCount,theICount/3,sizeof(Vertex2D));
}


void Graphics::PumpIndexedShape(Array<Vertex2D>& theVertex, int theVCount, Array<vbindex>& theIndices, int theICount)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShape(theVertex.mArray,theIndices.mArray,theVCount,theICount/3,sizeof(Vertex2D));
}

void Graphics::PumpIndexedShape(Array<Vertex2DN>& theVertex, Array<vbindex>& theIndices)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShapeN(theVertex.mArray,theIndices.mArray,theVertex.Size(),theIndices.Size()/3,sizeof(Vertex2DN));
}

void Graphics::PumpIndexedShape(Vertex2DN* theVertex, int theVCount, vbindex* theIndices, int theICount)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShapeN(theVertex,theIndices,theVCount,theICount/3,sizeof(Vertex2DN));
}


void Graphics::PumpIndexedShape(Array<Vertex2DN>& theVertex, int theVCount, Array<vbindex>& theIndices, int theICount)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShapeN(theVertex.mArray,theIndices.mArray,theVCount,theICount/3,sizeof(Vertex2DN));
}


void Graphics::PumpIndexedShape(Array<Vertex2DMT>& theVertex, Array<vbindex>& theIndices)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShapeMT(theVertex.mArray,theIndices.mArray,theVertex.Size(),theIndices.Size()/3,sizeof(Vertex2DMT));
}

void Graphics::PumpIndexedShape(Vertex2DMT* theVertex, int theVCount, vbindex* theIndices, int theICount)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShapeMT(theVertex,theIndices,theVCount,theICount/3,sizeof(Vertex2DMT));
}


void Graphics::PumpIndexedShape(Array<Vertex2DMT>& theVertex, int theVCount, Array<vbindex>& theIndices, int theICount)
{
	Flush();
	FreshenRenderMode();
	Graphics_Core::DrawIndexedShapeMT(theVertex.mArray,theIndices.mArray,theVCount,theICount/3,sizeof(Vertex2DMT));
}

void Graphics::DrawIndexedShape(int theTriangles, int theVertices, int theIndices, Vertex2D* theVertPtr, vbindex* theIndexPtr)
{
	StartIndexedShape(theTriangles);
	Vertex2D* aVPtr=theVertPtr;
	vbindex* aIPtr=theIndexPtr;

	for (int aCount=0;aCount<theVertices;aCount++)
	{
		AddVertex(aVPtr->mX,aVPtr->mY,aVPtr->mZ,aVPtr->mDiffuse,aVPtr->mTextureU,aVPtr->mTextureV);
		aVPtr++;
	}
	for (int aCount=0;aCount<theIndices;aCount++)
	{
		AddVertexIndex(*aIPtr);
		aIPtr++;
	}
	FinishIndexedShape();

	//
	// Can't just do this because of Translate...
	//
	//if (mShapeMode!=-1) FlushDrawBuffer();
	//Graphics_Core::DrawIndexedShape((void*)theVertices,(void*)theIndices,theVertexCount,theTriangleCount,sizeof(Vertex2D));
	//mShapeMode=-1;
}

void Graphics::DrawIndexedShapeTranslated(int theTriangles, int theVertices, int theIndices, Vertex2D* theVertPtr, vbindex* theIndexPtr)
{
	if (mShapeMode!=-1) Flush();
	Graphics_Core::DrawIndexedShape((void*)theVertPtr,(void*)theIndexPtr,theIndices,theTriangles,sizeof(Vertex2D));
	mShapeMode=-1;
}

void Graphics::FillRect(float theX, float theY, float theWidth, float theHeight)
{
	SetTexture();

	int aVColor=mColorInt;

	StartIndexedQuad();
	AddVertex(theX,theY,aVColor);
	AddVertex(theX+theWidth,theY,aVColor);
	AddVertex(theX,theY+theHeight,aVColor);
	AddVertex(theX+theWidth,theY+theHeight,aVColor);
	FinishIndexedQuad();
}
void Graphics::FillRectGradientV(float theX, float theY, float theWidth, float theHeight, Color theTop, Color theBottom)
{
	SetTexture();

	theTop*=mColor;
	theBottom*=mColor;
	
	unsigned int aTColor=theTop.ToInt();
	unsigned int aBColor=theBottom.ToInt();

	StartIndexedQuad();
	AddVertex(theX,theY,aTColor);
	AddVertex(theX+theWidth,theY,aTColor);
	AddVertex(theX,theY+theHeight,aBColor);
	AddVertex(theX+theWidth,theY+theHeight,aBColor);
	FinishIndexedQuad();
}

void Graphics::FillRectGradientH(float theX, float theY, float theWidth, float theHeight, Color theLeft, Color theRight)
{
	SetTexture();

	theLeft*=mColor;
	theRight*=mColor;
	
	unsigned int aLColor=theLeft.ToInt();
	unsigned int aRColor=theRight.ToInt();

	StartIndexedQuad();
	AddVertex(theX,theY,aLColor);
	AddVertex(theX+theWidth,theY,aRColor);
	AddVertex(theX,theY+theHeight,aLColor);
	AddVertex(theX+theWidth,theY+theHeight,aRColor);
	FinishIndexedQuad();
}

void Graphics::FillRectGradientBurst(float theX, float theY, float theWidth, float theHeight, Color theInside, Color theOutside, float theBorderSize)
{
	SetTexture();

	theInside*=mColor;
	theOutside*=mColor;

	unsigned int aIColor=theInside.ToInt();
	unsigned int aOColor=theOutside.ToInt();

	StartIndexedShape(6);
	AddVertex(theX-theBorderSize,theY-theBorderSize,aOColor);				
	AddVertex(theX+theWidth+theBorderSize,theY-theBorderSize,aOColor);		
	AddVertex(theX,theY,aIColor);											
	AddVertex(theX+theWidth,theY,aIColor);									
	AddVertex(theX-theBorderSize,theY+theHeight+theBorderSize,aOColor);		
	AddVertex(theX,theY+theHeight,aIColor);									
	AddVertex(theX+theWidth+theBorderSize,theY+theHeight+theBorderSize,aOColor);		
	AddVertex(theX+theWidth,theY+theHeight,aIColor);									
	FinishIndexedShape(24, 0,1,2,2,1,3, 0,2,4,4,2,5, 4,5,6,5,6,7, 1,3,6,6,3,7);
}
void Graphics::DrawTexturedQuad(Quad &theXY, Quad &theUV)
{
	int aVColor=mColorInt;

	StartIndexedQuad();
	AddVertex(theXY.UpperLeft(),aVColor,theUV.UpperLeft());
	AddVertex(theXY.UpperRight(),aVColor,theUV.UpperRight());
	AddVertex(theXY.LowerLeft(),aVColor,theUV.LowerLeft());
	AddVertex(theXY.LowerRight(),aVColor,theUV.LowerRight());
	FinishIndexedQuad();
}
void Graphics::DrawTexturedQuad(Quad3D &theXY, Quad &theUV)
{
	int aVColor=mColorInt;

	StartIndexedQuad();
	AddVertex(theXY.UpperLeft(),aVColor,theUV.UpperLeft());
	AddVertex(theXY.UpperRight(),aVColor,theUV.UpperRight());
	AddVertex(theXY.LowerLeft(),aVColor,theUV.LowerLeft());
	AddVertex(theXY.LowerRight(),aVColor,theUV.LowerRight());
	FinishIndexedQuad();
}
void Graphics::DrawTexturedQuadGradient(Quad &theXY, Quad &theUV, int the0, int the1, int the2, int the3)
{
	StartIndexedQuad();
	AddVertex(theXY.UpperLeft(),the0,theUV.UpperLeft());
	AddVertex(theXY.UpperRight(),the1,theUV.UpperRight());
	AddVertex(theXY.LowerLeft(),the2,theUV.LowerLeft());
	AddVertex(theXY.LowerRight(),the3,theUV.LowerRight());
	FinishIndexedQuad();
}
void Graphics::DrawTexturedQuadGradientV(Quad &theXY, Quad &theUV, Color theTop, Color theBottom)
{
	Color aTop=theTop*mColor;
	Color aBottom=theBottom*mColor;
	int aTopColor=aTop.ToInt();
	int aBottomColor=aBottom.ToInt();

	StartIndexedQuad();
	AddVertex(theXY.UpperLeft(),aTopColor,theUV.UpperLeft());
	AddVertex(theXY.UpperRight(),aTopColor,theUV.UpperRight());
	AddVertex(theXY.LowerLeft(),aBottomColor,theUV.LowerLeft());
	AddVertex(theXY.LowerRight(),aBottomColor,theUV.LowerRight());
	FinishIndexedQuad();
}
void Graphics::DrawTexturedQuadGradientH(Quad &theXY, Quad &theUV, Color the0, Color the1)
{
	Color aTop=the0*mColor;
	Color aBottom=the1*mColor;
	int aTopColor=aTop.ToInt();
	int aBottomColor=aBottom.ToInt();

	StartIndexedQuad();
	AddVertex(theXY.UpperLeft(),aTopColor,theUV.UpperLeft());
	AddVertex(theXY.UpperRight(),aBottomColor,theUV.UpperRight());
	AddVertex(theXY.LowerLeft(),aTopColor,theUV.LowerLeft());
	AddVertex(theXY.LowerRight(),aBottomColor,theUV.LowerRight());
	FinishIndexedQuad();
}
void Graphics::FillQuad(Quad& theXY)
{
	SetTexture();

	int aVColor=mColorInt;

	StartIndexedQuad();
	AddVertex(theXY.UpperLeft(),aVColor);
	AddVertex(theXY.UpperRight(),aVColor);
	AddVertex(theXY.LowerLeft(),aVColor);
	AddVertex(theXY.LowerRight(),aVColor);
	FinishIndexedQuad();
}
void Graphics::FillQuad(Quad3D& theXY)
{
	SetTexture();

	int& aVColor=mColorInt;

	StartIndexedQuad();
	AddVertex(theXY.UpperLeft(),aVColor);
	AddVertex(theXY.UpperRight(),aVColor);
	AddVertex(theXY.LowerLeft(),aVColor);
	AddVertex(theXY.LowerRight(),aVColor);
	FinishIndexedQuad();
}
void Graphics::FillQuad(Quad &theXY, Color theColor1, Color theColor2, Color theColor3, Color theColor4)
{
	SetTexture();

	theColor1*=mColor;
	theColor2*=mColor;
	theColor3*=mColor;
	theColor4*=mColor;

	StartIndexedQuad();
	AddVertex(theXY.UpperLeft(),theColor1.ToInt());
	AddVertex(theXY.UpperRight(),theColor2.ToInt());
	AddVertex(theXY.LowerLeft(),theColor3.ToInt());
	AddVertex(theXY.LowerRight(),theColor4.ToInt());
	FinishIndexedQuad();
}

void Graphics::DrawLine(Point theStart, Point theEnd, float theWidth)
{
	SetTexture();

	int aVColor=mColorInt;
	float aHalfWidth=theWidth/2;

	Point aLineVec=theStart-theEnd;
	Point aPerp=aLineVec.Perp();
	aPerp.Normalize();

	Point aUpperLeft=theStart-(aPerp*aHalfWidth);
	Point aUpperRight=theStart+(aPerp*aHalfWidth);
	Point aLowerLeft=theEnd-(aPerp*aHalfWidth);
	Point aLowerRight=theEnd+(aPerp*aHalfWidth);

	StartIndexedQuad();
	AddVertex(aUpperLeft,aVColor);
	AddVertex(aUpperRight,aVColor);
	AddVertex(aLowerLeft,aVColor);
	AddVertex(aLowerRight,aVColor);
	FinishIndexedQuad();
}

void Graphics::Draw2DZLine(Vector theStart, Vector theEnd, float theWidth, float theWidth2)
{
	if (theWidth2==-1) theWidth2=theWidth;
	SetTexture();

	int aVColor=mColorInt;
	float aHalfWidth=theWidth/2;
	float aHalfWidth2=theWidth2/2;

	Vector aLineVec=theStart-theEnd;
	Point aPVec=aLineVec;
	Vector aPerp=aPVec.Perp();
	aPerp.Normalize();

	Vector aUpperLeft=theStart-(aPerp*aHalfWidth);
	Vector aUpperRight=theStart+(aPerp*aHalfWidth);
	Vector aLowerLeft=theEnd-(aPerp*aHalfWidth2);
	Vector aLowerRight=theEnd+(aPerp*aHalfWidth2);

	static Array<Vertex2D> aV;
	static Array<vbindex> aI;

	aV[0].Set(aUpperLeft,Point(0,0),aVColor);
	aV[1].Set(aUpperRight,Point(0,0),aVColor);
	aV[2].Set(aLowerLeft,Point(0,0),aVColor);
	aV[3].Set(aLowerRight,Point(0,0),aVColor);
	if (aI.Size()==0)
	{
		aI[0]=0;aI[1]=1;aI[2]=2;
		aI[3]=1;aI[4]=2;aI[5]=3;
	}

	gG.PumpIndexedShape(aV,aI);

	/*
	StartIndexedQuad();
	AddVertex(aUpperLeft,aVColor);
	AddVertex(aUpperRight,aVColor);
	AddVertex(aLowerLeft,aVColor);
	AddVertex(aLowerRight,aVColor);
	FinishIndexedQuad();
	*/
}

void Graphics::DrawTexturedZQuad(Quad &theXY, Quad &theUV, float theZ)
{
	int aVColor=mColorInt;
	static Array<Vertex2D> aV;
	static Array<vbindex> aI;
	aV[0].Set(Vector(theXY.mCorner[0].mX+mTranslate.mX,theXY.mCorner[0].mY+mTranslate.mY,theZ),theUV.mCorner[0],aVColor);
	aV[1].Set(Vector(theXY.mCorner[1].mX+mTranslate.mX,theXY.mCorner[1].mY+mTranslate.mY,theZ),theUV.mCorner[1],aVColor);
	aV[2].Set(Vector(theXY.mCorner[2].mX+mTranslate.mX,theXY.mCorner[2].mY+mTranslate.mY,theZ),theUV.mCorner[2],aVColor);
	aV[3].Set(Vector(theXY.mCorner[3].mX+mTranslate.mX,theXY.mCorner[3].mY+mTranslate.mY,theZ),theUV.mCorner[3],aVColor);
	if (aI.Size()==0) {aI[0]=0;aI[1]=1;aI[2]=2;aI[3]=1;aI[4]=2;aI[5]=3;}
	gG.PumpIndexedShape(aV,aI);
}

void Graphics::DrawLine(Vector theStart, Vector theEnd)
{
	SetTexture();

	int aVColor=mColorInt;

	Vertex2D aV[2];
	aV[0].mX=theStart.mX;
	aV[0].mY=theStart.mY;
	aV[0].mZ=theStart.mZ;
	aV[0].mDiffuse=aVColor;
	aV[1].mX=theEnd.mX;
	aV[1].mY=theEnd.mY;
	aV[1].mZ=theEnd.mZ;
	aV[1].mDiffuse=aVColor;

	Graphics_Core::DrawLine(aV,1,sizeof(Vertex2D));

/*
	Point aLineVec=theStart-theEnd;
	Point aPerpP=aLineVec.Perp();
	aPerpP.Normalize();

	Vector aPerp=Vector(aPerpP.mX,aPerpP.mY,0);

	Vector aUpperLeft=theStart-(aPerp*aHalfWidth);
	Vector aUpperRight=theStart+(aPerp*aHalfWidth);
	Vector aLowerLeft=theEnd-(aPerp*aHalfWidth);
	Vector aLowerRight=theEnd+(aPerp*aHalfWidth);

	StartIndexedQuad();
	AddVertex(aUpperLeft,aVColor);
	AddVertex(aUpperRight,aVColor);
	AddVertex(aLowerLeft,aVColor);
	AddVertex(aLowerRight,aVColor);
	FinishIndexedQuad();
*/
}

void Graphics::DrawLine(Vector theStart, Vector theEnd, Color theStartColor, Color theEndColor)
{
	SetTexture();

	int aVColor=mColorInt;

	Vertex2D aV[2];
	aV[0].mX=theStart.mX;
	aV[0].mY=theStart.mY;
	aV[0].mZ=theStart.mZ;
	aV[0].mDiffuse=theStartColor.ToInt();
	aV[1].mX=theEnd.mX;
	aV[1].mY=theEnd.mY;
	aV[1].mZ=theEnd.mZ;
	aV[1].mDiffuse=theEndColor.ToInt();

	Graphics_Core::DrawLine(aV,1,sizeof(Vertex2D));
}

void Graphics::DrawTexturedLine(Point theStart, Point theEnd, Quad& theUV, float theWidth)
{
	int aVColor=mColorInt;
	float aHalfWidth=theWidth/2;

	Point aLineVec=theStart-theEnd;
	Point aPerp=aLineVec.Perp();
	aPerp.Normalize();

	Point aUpperLeft=theStart-(aPerp*aHalfWidth);
	Point aUpperRight=theStart+(aPerp*aHalfWidth);
	Point aLowerLeft=theEnd-(aPerp*aHalfWidth);
	Point aLowerRight=theEnd+(aPerp*aHalfWidth);

	StartIndexedQuad();
	AddVertex(aUpperLeft,aVColor,theUV.UpperLeft());
	AddVertex(aUpperRight,aVColor,theUV.UpperRight());
	AddVertex(aLowerLeft,aVColor,theUV.LowerLeft());
	AddVertex(aLowerRight,aVColor,theUV.LowerRight());
	FinishIndexedQuad();
}

void Graphics::DrawTexturedLine(Point theStart, Point theEnd, Quad& theUV, Color theColor1, Color theColor2, float theWidth)
{
	float aHalfWidth=theWidth/2;

	Point aLineVec=theStart-theEnd;
	Point aPerp=aLineVec.Perp();
	aPerp.Normalize();

	Point aUpperLeft=theStart-(aPerp*aHalfWidth);
	Point aUpperRight=theStart+(aPerp*aHalfWidth);
	Point aLowerLeft=theEnd-(aPerp*aHalfWidth);
	Point aLowerRight=theEnd+(aPerp*aHalfWidth);

	int aTopColor=(theColor1*mColor).ToInt();
	int aBottomColor=(theColor2*mColor).ToInt();

	StartIndexedQuad();
	AddVertex(aUpperLeft,aTopColor,theUV.UpperLeft());
	AddVertex(aUpperRight,aTopColor,theUV.UpperRight());
	AddVertex(aLowerLeft,aBottomColor,theUV.LowerLeft());
	AddVertex(aLowerRight,aBottomColor,theUV.LowerRight());
	FinishIndexedQuad();
}


void Graphics::DrawLine(Vector theStart, Vector theEnd, Vector theWidthVector, float theWidth)
{
	SetTexture();

	int aVColor=mColorInt;
	float aHalfWidth=theWidth/2;

	Vector aPerp=theWidthVector.Normal()*aHalfWidth;

	Vector aUpperLeft=theStart-(aPerp);
	Vector aUpperRight=theStart+(aPerp);
	Vector aLowerLeft=theEnd-(aPerp);
	Vector aLowerRight=theEnd+(aPerp);

	StartIndexedQuad();
	AddVertex(aUpperLeft,aVColor);
	AddVertex(aUpperRight,aVColor);
	AddVertex(aLowerLeft,aVColor);
	AddVertex(aLowerRight,aVColor);
	FinishIndexedQuad();
}

void Graphics::DrawLine(Point theStart, Point theEnd, float theWidth, int theStartColor, int theEndColor)
{

	SetTexture();
	float aHalfWidth=theWidth/2;

	Point aLineVec=theStart-theEnd;
	Point aPerp=aLineVec.Perp();
	aPerp.Normalize();

	Point aUpperLeft=theStart-(aPerp*aHalfWidth);
	Point aUpperRight=theStart+(aPerp*aHalfWidth);
	Point aLowerLeft=theEnd-(aPerp*aHalfWidth);
	Point aLowerRight=theEnd+(aPerp*aHalfWidth);

	StartIndexedQuad();
	AddVertex(aUpperLeft,theStartColor);
	AddVertex(aUpperRight,theStartColor);
	AddVertex(aLowerLeft,theEndColor);
	AddVertex(aLowerRight,theEndColor);
	FinishIndexedQuad();
}
#endif

void Graphics::SetShaderData(int theSlot,float theD1, float theD2, float theD3, float theD4)
{
	static int aSlotToFlag[3]={DRAWINFO_SHADERDATA,DRAWINFO_SHADERDATA2,DRAWINFO_SHADERDATA3};
	mShaderData[theSlot][0]=theD1;
	mShaderData[theSlot][1]=theD2;
	mShaderData[theSlot][2]=theD3;
	mShaderData[theSlot][3]=theD4;
	int& aFlag=aSlotToFlag[theSlot];
	if (mCurrentShader)	{if (mCurrentShader->mDrawInfo[0].mFlags&aFlag) Graphics_Core::SetShaderGlobal_Float(mCurrentShader->mDrawInfo[0].mShaderData[theSlot],(float*)&gG.mShaderData[theSlot],4);if (mCurrentShader->mDrawInfo[1].mFlags&aFlag) Graphics_Core::SetShaderGlobal_Float(mCurrentShader->mDrawInfo[1].mShaderData[theSlot],(float*)&gG.mShaderData[theSlot],4);}
}

void Graphics::SetColor(float theR, float theG, float theB, float theA)
{
	if (mDisableSetcolor) return;
	if (mCurrentShader) mCurrentShader->mRehup|=DRAWINFO_COLOR;


	mColor.mR=theR;
	mColor.mG=theG;
	mColor.mB=theB;
	//mColor.mA=theA;
	mColor.mA=_clamp(0,theA,1);

	mDrawColor=mColor*mClipColor;
	mColorInt=mDrawColor.ToInt();
	
#ifndef USEPIPELINE
	if (mRenderWhite)
	{
		//
		// On OpenGL systems, RenderWhite needs an extra
		// call to work properly.  This handles that.
		//
		if (Graphics_Core::NeedSetRenderWhiteColor())
		{
			DEBUG("Flush -> RenderWhiteSwitch()");
			Flush();
			Graphics_Core::SetRenderWhiteColor(&mDrawColor.mR);
		}
	}
#endif
}

void Graphics::SetClipColor(float theR, float theG, float theB, float theA)
{
	mClipColor.mR=theR;
	mClipColor.mG=theG;
	mClipColor.mB=theB;
	mClipColor.mA=theA;

	SetColor(mColor);
}

void Graphics::RegisterFillrect(Sprite* theSprite) 
{
	if (theSprite->mTexture>=0) 
	{
		mFillrect_List[theSprite->mTexture]=theSprite;
		//gOut.Out("Register [%d ] = %d",theSprite->mTexture,theSprite);
	}
}


void Graphics::SetTexture(int theTexture, bool guaranteeSet)
{
	mClampUVCoords=false;
	if (mUseTexturedFills && theTexture==-1 && mCurrentTexture>=0 && !guaranteeSet)
	{
		//
		// Do we have a Fillrect registered with the current texture?
		// If so, don't switch, but clamp the UV coords into the Fillrect sprite.
		// New way just keeps the pointer in the array at exactly the texture's spot.
		//

		if (mFillrect_List[mCurrentTexture])
		{
			mClampUVCoords=true;
			mClampUVCoordsPos=mFillrect_List[mCurrentTexture]->GetTextureQuad().Center();
			return;
		}
	}
	if (mCurrentTexture!=theTexture || guaranteeSet) 
	{
		Flush();
#ifdef USEPIPELINE
		if (theTexture==-1) theTexture=mBlankTexture;
#endif
		Graphics_Core::SetTexture(theTexture);

#ifdef USEPIPELINE
		if (mCurrentShader) 
		{
			mCurrentShader->mRehup|=DRAWINFO_TEXTURES;
			mCurrentShader->Rehup();
		}
#endif
		//SetShader(NULL);
		//SetShader(mCurrentShader);

		if (theTexture>=0 && mFillrect_List[theTexture]) mClampUVCoordsPos=mFillrect_List[theTexture]->GetTextureQuad().Center();
		else mClampUVCoordsPos=0;
	}

	mCurrentTexture=theTexture;
}

void Graphics::SetMultiTexture(int theStage, int theTexture, bool guaranteeSet)
{
	mClampUVCoords=false;
	/*
	if (mUseTexturedFills)
	if (theTexture==-1 && mCurrentMultiTexture!=-1 && !guaranteeSet)
	{
		//
		// Do we have a Fillrect registered with the current texture?
		// If so, don't switch, but clamp the UV coords into the Fillrect sprite.
		// New way just keeps the pointer in the array at exactly the texture's spot.
		//

		if (mFillrect_List[mCurrentMultiTexture])
		{
			mClampUVCoords=true;
			mClampUVCoordsPos=mFillrect_List[mCurrentMultiTexture]->GetTextureQuad().Center();
			return;
		}
	}
	*/
	if (mCurrentMultiTexture[theStage]!=theTexture || guaranteeSet) 
	{
		DEBUG(Sprintf("Flush -> ChangeTexture() = %d",theTexture));
		Flush();
		Graphics_Core::SetMultiTexture(theStage,theTexture);
	}

	mCurrentMultiTexture[theStage]=theTexture;
}

void Graphics::SetMultiTextureZ(int theStage, int theTexture)
{
	mClampUVCoords=false;
	Flush();
	Graphics_Core::SetMultiTexture(theStage,theTexture,"z");
	mCurrentMultiTexture[theStage]=-1; // Guarantees if we set it again, it changes.
}

int Graphics::LoadTexture(String theFilename)
{
	//
	// Look for low-color versions...
	//
	if (mSaveVideoMemory || !DoesImageExist(theFilename))
	{
		String a4444=InsertFilePrefix(theFilename,"4444-");
		if (DoesImageExist(a4444)) 
		{
			Image aImage;
			aImage.Load(a4444);
			return LoadTexture4444(aImage.mWidth,aImage.mHeight,aImage.mData);
		}
		String a565=InsertFilePrefix(theFilename,"565-");
		if (DoesImageExist(a565))
		{
			Image aImage;
			aImage.Load(a565);
			return LoadTexture565(aImage.mWidth,aImage.mHeight,aImage.mData);
		}
	}
	
	if (DoesImageExist(Sprintf("%s_alpha",theFilename.c())))
	{
		Image aImage;
		aImage.Load(theFilename);
		return LoadTexture(aImage);
	}
    
    //
    // Removed quickloading because it's PNG only, and Apple has ensure we can't properly use PNGs... ever.
    //
    //    ThreadLock(true);
    //	  int aResult=Graphics_Core::QuickLoadTexture(Sprintf("%s.png",theFilename.c()));
    //    ThreadLock(false);
    //    if (aResult==-1)
    
	//
	// If a quick load didn't work, attempt a manual load...
	//

	Image aImage;
	aImage.Load(theFilename);

	int aResult=LoadTexture(aImage);
	return aResult;
	//return LoadTexture(aImage);
}

int Graphics::LoadTexture(Image &theImage)
{
    ThreadLock(true);
    int aResult=Graphics_Core::LoadTexture(theImage.mWidth,theImage.mHeight,(unsigned char*)theImage.GetBits(),0);
    ThreadLock(false);

	/*
	static int aCounter=1;
	{
		theImage.ExportRAW(PointAtDesktop(Sprintf("Test%d_",aCounter++)));
	}
	/**/
    return aResult;
}

int Graphics::LoadTexture565(Image &theImage)
{
	ThreadLock(true);
	int aResult=Graphics_Core::LoadTexture(theImage.mWidth,theImage.mHeight,(unsigned char*)theImage.GetBits(),1);
	ThreadLock(false);
	return aResult;
}

int Graphics::LoadTexture4444(Image &theImage)
{
	ThreadLock(true);
	int aResult=Graphics_Core::LoadTexture(theImage.mWidth,theImage.mHeight,(unsigned char*)theImage.GetBits(),2);
	ThreadLock(false);
	return aResult;
}

int Graphics::LoadTexture8888(int theWidth, int theHeight, void* theBits)
{
    ThreadLock(true);
	int aResult=Graphics_Core::LoadTexture(theWidth,theHeight,(unsigned char*)theBits,0);
    ThreadLock(false);
    return aResult;
}

int Graphics::LoadTexture565(int theWidth, int theHeight, void* theBits)
{
    ThreadLock(true);
    int aResult=Graphics_Core::LoadTexture(theWidth,theHeight,(unsigned char*)theBits,1);
    ThreadLock(false);
    return aResult;
}

int Graphics::LoadTexture4444(int theWidth, int theHeight, void* theBits)
{
    ThreadLock(true);
    int aResult=Graphics_Core::LoadTexture(theWidth,theHeight,(unsigned char*)theBits,2);
    ThreadLock(false);
    return aResult;
}

int Graphics::CreateTexture(int theWidth, int theHeight)
{
    ThreadLock(true);
    int aResult=Graphics_Core::CreateTexture(theWidth,theHeight);
    ThreadLock(false);
    return aResult;
}

int Graphics::CreateRenderer(int theWidth, int theHeight, bool theFormat, bool wantReadable)
{
    ThreadLock(true);
    int aResult=Graphics_Core::CreateRenderer(theWidth,theHeight, theFormat, wantReadable);
    ThreadLock(false);
    return aResult;
}

#ifndef USEPIPELINE
int Graphics::CreateShape(Vertex2D* theVertexPtr, int theVertexCount, vbindex* theIndexPtr, int theIndexCount)
//int Graphics::CreateShape(Array<Vertex2D>& theVertices, Array<vbindex>& theIndexes)
{
    ThreadLock(true);
//	int aResult=Graphics_Core::CreateShape(theVertices.mArray,theVertices.Size(),theIndexes.mArray,theIndexes.Size(),sizeof(Vertex2D));
	int aResult=Graphics_Core::CreateShape(theVertexPtr,theVertexCount,theIndexPtr,theIndexCount,sizeof(Vertex2D));
    ThreadLock(false);
    return aResult;
}

int Graphics::CreateShape(Vertex2DN* theVertexPtr, int theVertexCount, vbindex* theIndexPtr, int theIndexCount)
{
	ThreadLock(true);
	int aResult=Graphics_Core::CreateShapeN(theVertexPtr,theVertexCount,theIndexPtr,theIndexCount,sizeof(Vertex2DN));
	ThreadLock(false);
	return aResult;
}

int Graphics::CreateShape(Vertex2DMT* theVertexPtr, int theVertexCount, vbindex* theIndexPtr, int theIndexCount)
{
	ThreadLock(true);
	int aResult=Graphics_Core::CreateShapeMT(theVertexPtr,theVertexCount,theIndexPtr,theIndexCount,sizeof(Vertex2DMT));
	ThreadLock(false);
	return aResult;
}


void Graphics::DrawShape(int theShape)
{
	if (mCurrentShader) {mCurrentShader->DrawShape(theShape);}
	else
	{
		FreshenRenderMode();
		Graphics_Core::DrawShape(theShape);
	}
}

void Graphics::DrawShape(void* theVerts, int theVCount)
{
	if (mCurrentShader) mCurrentShader->DrawShape(theVerts,theVCount);
	else PumpShape((Vertex2D*)theVerts,theVCount);
}

void Graphics::DrawShape(void* theVerts, int theVCount, vbindex* theI, int theICount)
{
	if (mCurrentShader) mCurrentShader->DrawShape(theVerts,theVCount,theI,theICount);
	else PumpIndexedShape((Vertex2D*)theVerts,theICount,theI,theICount);
}
#endif

void Graphics::UnloadShape(int theShape)
{
    ThreadLock(true);
	Graphics_Core::UnloadShape(theShape);
    ThreadLock(false);
}

void Graphics::UnloadTexture(int theNumber)
{
	if (theNumber<0) return;

	ThreadLock(true);
	Graphics_Core::UnloadTexture(theNumber);
	ThreadLock(false);

	if (mFillrect_List.Size()>theNumber) mFillrect_List[theNumber]=NULL;
	if (mCurrentTexture==theNumber)
	{
		mCurrentTexture=-2; // To make sure we do a set and aren't pointed at an old texture...
		SetTexture(-1,true);
		mCurrentTexture=-2; // To make sure we do a set and aren't pointed at an old texture...
	}

	//
	// This just makes sure the fillrect associated with this texture is gone...
	//
}


IPoint Graphics::GetTextureDimensions(int theNumber)
{
	IPoint aP;
	Graphics_Core::GetTextureDimensions(theNumber,&aP.mX,&aP.mY);
	return aP;
}

Point Graphics::GetTextureDimensionsF(int theNumber)
{
	IPoint aP;
	Graphics_Core::GetTextureDimensions(theNumber,&aP.mX,&aP.mY);
	return Point(aP.mX,aP.mY);
}

void Graphics::FreshenRenderMode()
{
#ifdef TRACKSTATECHANGES
	bool aStateChanged=false;
#endif

	if (mTextureMode!=mLastTextureMode)
	{
		Flush();

		switch (mTextureMode)
		{
		case 0:Graphics_Core::Clamp();break;
		case 1:Graphics_Core::Wrap();break;
		}
		mLastTextureMode=mTextureMode;
	}

	if (mRenderMode!=mLastRenderMode) 
	{
		Flush();

		switch (mRenderMode)
		{
		case 0:	Graphics_Core::RenderNormal(); break;
		case 1:	Graphics_Core::RenderAdditive(); break;
		case 2:	Graphics_Core::RenderMultiply(); break;
		}

		mLastRenderMode=mRenderMode;

#ifdef TRACKSTATECHANGES
		aStateChanged=true;
#endif

	}
#ifndef USEPIPELINE
	if (mRenderWhite!=mLastRenderWhite)
	{
		Flush();
        Graphics_Core::SetRenderWhiteColor(&mDrawColor.mR);
		Graphics_Core::RenderWhite(mRenderWhite);
		mLastRenderWhite=mRenderWhite;

#ifdef TRACKSTATECHANGES
		aStateChanged=true;
#endif

	}
	if (mSaturation!=mLastSaturation)
	{
		Flush();
		Graphics_Core::RenderSaturation(mSaturation);
		mLastSaturation=mSaturation;
	}

	if (mBlur!=mLastBlur)
	{
		Flush();
		Graphics_Core::RenderBlurred(mBlur);
		mLastBlur=mBlur;
	}
#else
	if (mFlags!=mLastFlags)
	{
		if ((mFlags&FLAG_RENDERMODES)!=(mLastFlags&FLAG_RENDERMODES))
		{
			if (mFlags&FLAG_RENDERWHITE) SetPipeline(mPipeline_RenderWhite);
			else if (mFlags&FLAG_RENDERGREY) SetPipeline(mPipeline_Grey);
			else SetPipeline(mPipeline_Normal);
		}
		mLastFlags=mFlags;
	}
#endif

#ifdef TRACKSTATECHANGES
	if (aStateChanged) gStateChangeCount++;
#endif

}


/*
void Graphics::RenderAdditive(bool theState)
{
	if (theState)
	{
		if (mRenderAdditive!=1)
		{
			DEBUG("Flush -> RenderAdditiveSwitch1()");
			FlushDrawBuffer();
			mRenderAdditive=1;
			Graphics_Core::RenderAdditive(true);
		}
	}
	else
	{
		if (mRenderAdditive!=0)
		{
			DEBUG("Flush -> RenderAdditiveSwitch2()");
			FlushDrawBuffer();
			mRenderAdditive=0;
			Graphics_Core::RenderAdditive(false);
		}
	}
}

void Graphics::RenderMultiply(bool theState)
{
	if (theState)
	{
		if (mRenderMultiply!=1)
		{
			DEBUG("Flush -> RenderMultiply1()");
			FlushDrawBuffer();
			mRenderMultiply=1;
			Graphics_Core::RenderMultiply(true);
		}
	}
	else
	{
		if (mRenderMultiply!=0)
		{
			DEBUG("Flush -> RenderMultiply2()");
			FlushDrawBuffer();
			mRenderMultiply=0;
			Graphics_Core::RenderMultiply(false);
		}
	}
}


void Graphics::RenderWhite(bool theState)
{
	if (theState)
	{
		if (mRenderWhite!=1)
		{
			DEBUG("Flush -> RenderWhite1()");
			FlushDrawBuffer();
			mRenderWhite=1;
			Graphics_Core::RenderWhite(true);
			//
			// For OpenGL systems, RenderWhite also needs to know the current color...
			// 
			if (Graphics_Core::NeedSetRenderWhiteColor()) Graphics_Core::SetRenderWhiteColor(&mDrawColor.mR);
		}
	}
	else
	{
		if (mRenderWhite!=0)
		{
			DEBUG("Flush -> RenderWhite2()");
			FlushDrawBuffer();
			mRenderWhite=0;
			Graphics_Core::RenderWhite(false);
		}
	}
}


void Graphics::RenderGrayscale(bool theState)
{
	if (theState)
	{
		if (mRenderGrayscale!=1)
		{
			DEBUG("Flush -> RenderGrayScale1()");
			FlushDrawBuffer();
			mRenderGrayscale=1;
			Graphics_Core::RenderGrayscale(true);
		}
	}
	else
	{
		if (mRenderGrayscale!=0)
		{
			DEBUG("Flush -> RenderGrayScale2()");
			FlushDrawBuffer();
			mRenderGrayscale=0;
			Graphics_Core::RenderGrayscale(false);
		}
	}
}
*/

void Graphics::ScaleView(Matrix &theMatrix)
{
	DEBUG("Flush -> ScaleView()");
	Flush();

	SetViewMatrix(theMatrix);
}

void Graphics::ScaleView(float theScale, Point theCenter)
{
	DEBUG("Flush -> ScaleView()");
	Flush();
	
	Matrix aMat;
	aMat.Translate(-theCenter);
	aMat.Scale(theScale,theScale,1.0f);
	aMat.Translate(theCenter);

	SetViewMatrix(aMat);
}

void Graphics::RotateView(float theRotation, Point theCenter)
{
	DEBUG("Flush -> RotateView()");
	Flush();
	
	Matrix aMat;
	aMat.Translate(-theCenter);
	aMat.Rotate2D(theRotation);
	aMat.Translate(theCenter);

	SetViewMatrix(aMat);
}

void Graphics::SetViewMatrix(Matrix &theMatrix)
{
	DEBUG("Flush -> SetViewMatrixView()");
	Flush();

	mViewMatrix=theMatrix;
	mViewMatrixN=theMatrix;mViewMatrixN.Normalize();

	Graphics_Core::SetMatrix(1,&theMatrix.mData);
	mRehupDrawInfo|=DRAWINFO_DEPENDENCIES;
	if (mCurrentShader) mCurrentShader->mRehup|=(DRAWINFO_VIEWMATRIX|DRAWINFO_VIEWMATRIXN|DRAWINFO_COMBOMATRIX|DRAWINFO_BILLBOARDMATRIX|DRAWINFO_WORLDVIEWMATRIX);
}

void Graphics::SetViewMatrix()
{
	DEBUG("Flush -> SetViewMatrixView()");
	Flush();

	mViewMatrix.Reset();mViewMatrixN.Reset();
	Graphics_Core::SetMatrix(1,&mViewMatrix.mData);
	mRehupDrawInfo|=DRAWINFO_DEPENDENCIES;
	if (mCurrentShader) mCurrentShader->mRehup|=(DRAWINFO_VIEWMATRIX|DRAWINFO_VIEWMATRIXN|DRAWINFO_COMBOMATRIX|DRAWINFO_WORLDVIEWMATRIX|DRAWINFO_BILLBOARDMATRIX);
}

void Graphics::SetProjectionMatrix(Matrix &theMatrix)
{
	Flush();
	mProjectionMatrix=theMatrix;
	mProjectionMatrixN=theMatrix;mProjectionMatrixN.Normalize();

	Graphics_Core::SetMatrix(2,&mProjectionMatrix.mData);
	mRehupDrawInfo|=DRAWINFO_DEPENDENCIES;
	if (mCurrentShader) mCurrentShader->mRehup|=(DRAWINFO_PROJECTIONMATRIX|DRAWINFO_PROJECTIONMATRIXN|DRAWINFO_COMBOMATRIX|DRAWINFO_BILLBOARDMATRIX);
}

void Graphics::SetProjectionMatrix()
{
	Flush();
	mProjectionMatrix.Reset();mProjectionMatrixN.Reset();
	Graphics_Core::SetMatrix(1,&mProjectionMatrix.mData);
	mRehupDrawInfo|=DRAWINFO_DEPENDENCIES;
	if (mCurrentShader) mCurrentShader->mRehup|=(DRAWINFO_PROJECTIONMATRIX|DRAWINFO_PROJECTIONMATRIXN|DRAWINFO_COMBOMATRIX|DRAWINFO_BILLBOARDMATRIX);
}

void Graphics::SetWorldMatrix(Matrix &theMatrix)
{
	Flush();

	mWorldMatrix=theMatrix;
	mWorldMatrixN=theMatrix;mWorldMatrixN.Normalize();
	Graphics_Core::SetMatrix(0,&theMatrix.mData);
	mRehupDrawInfo|=DRAWINFO_DEPENDENCIES;
	if (mCurrentShader) mCurrentShader->mRehup|=(DRAWINFO_WORLDMATRIX|DRAWINFO_WORLDMATRIXN|DRAWINFO_COMBOMATRIX|DRAWINFO_WORLDVIEWMATRIX|DRAWINFO_BILLBOARDMATRIX);
}


void Graphics::SetWorldMatrix()
{
	Flush();
	mWorldMatrix.Reset();
	mWorldMatrixN.Reset();
	Graphics_Core::SetMatrix(0,&mWorldMatrix.mData);
	mRehupDrawInfo|=DRAWINFO_DEPENDENCIES;
	if (mCurrentShader) mCurrentShader->mRehup|=(DRAWINFO_WORLDMATRIX|DRAWINFO_WORLDMATRIXN|DRAWINFO_COMBOMATRIX|DRAWINFO_WORLDVIEWMATRIX|DRAWINFO_BILLBOARDMATRIX);
}

void Graphics::SetTextureMatrix(Matrix &theMatrix)
{
	Flush();

	mTextureMatrix=theMatrix;
	Graphics_Core::SetMatrix(3,&theMatrix.mData);
}


void Graphics::SetTextureMatrix()
{
	Flush();
	mTextureMatrix.Reset();
	//Graphics_Core::SetMatrix(3,&mTextureMatrix.mData);
	Graphics_Core::SetMatrix(3,NULL);
}

void Graphics::SetMultiTextureMatrix(Matrix &theMatrix)
{
	Flush();

	mMultiTextureMatrix=theMatrix;
	Graphics_Core::SetMatrix(4,&theMatrix.mData);
}


void Graphics::SetMultiTextureMatrix()
{
	Flush();
	mMultiTextureMatrix.Reset();
	Graphics_Core::SetMatrix(4,NULL);
}

void Graphics::Clip()
{
	DEBUG("Flush -> NoClipping()");
	Flush();

	mClipRect=Rect(0,0,-1,-1);
	if (mFreezeClip.mState) mClipRect=mFreezeClip.mRect;

	Graphics_Core::Clip();
}

void Graphics::Clip(float theX, float theY, float theWidth, float theHeight)
{
	DEBUG("Flush -> Clip()");
	Flush();

	if (theWidth<0 || theHeight<0) Clip();
	else
	{
		theX+=mTranslate.mX;
		theY+=mTranslate.mY;

		if (mFreezeClip.mState)
		{
			Rect aWorkRect=Rect(theX,theY,theWidth,theHeight).GetClipInto(mFreezeClip.mRect);

			theX=aWorkRect.mX;
			theY=aWorkRect.mY;
			theWidth=aWorkRect.mWidth;
			theHeight=aWorkRect.mHeight;
		}

		//
		// Apply clipping to the view matrix.  This will NOT work
		// with rotating, but it will with scaling.
		//

		Point aPoint[2];
		aPoint[0]=Point(theX,theY);
		aPoint[1]=aPoint[0]+Point(theWidth,theHeight);
		mViewMatrix.ProcessPoints(aPoint,2);
		Graphics_Core::Clip(aPoint[0].mX,aPoint[0].mY,aPoint[1].mX,aPoint[1].mY);


		aPoint[1]-=aPoint[0];
		mClipRect=Rect(aPoint[0].mX,aPoint[0].mY,aPoint[1].mX,aPoint[1].mY);

//		mClipRect=Rect(theX,theY,theWidth,theHeight);
//		Graphics_Core::Clip(theX,theY,(theX+theWidth),(theY+theHeight));
	}
}

void Graphics::ClipInto(float theX, float theY, float theWidth, float theHeight)
{
	if (GetClipRect().mWidth==-1) 
	{
		Clip(theX,theY,theWidth,theHeight);
		return;
	}
	DEBUG("Flush -> ClipInto()");
	Flush();

	if (theWidth>0 && theHeight>0)
	{
		theX+=mTranslate.mX;
		theY+=mTranslate.mY;

		Rect aCurrentClipRect;
		if (!gG.IsRenderTo()) aCurrentClipRect=GetClipRect().Translate(mLetterboxSize/2);
		else aCurrentClipRect=GetClipRect();
		Rect aMyRect=Rect(theX,theY,theWidth,theHeight).GetClipInto(aCurrentClipRect);

		if (mFreezeClip.mState)
		{
			aMyRect=aMyRect.GetClipInto(mFreezeClip.mRect);
		}

		//
		// Apply clipping to the view matrix.  This will NOT work
		// with rotating, but it will with scaling.
		//
		Point aPoint[2];
		aPoint[0]=Point(aMyRect.mX,aMyRect.mY);
		aPoint[1]=aPoint[0]+Point(aMyRect.mWidth,aMyRect.mHeight);
		mViewMatrix.ProcessPoints(aPoint,2);
		Graphics_Core::Clip(aPoint[0].mX,aPoint[0].mY,aPoint[1].mX,aPoint[1].mY);
		aPoint[1]-=aPoint[0];
		mClipRect=Rect(aPoint[0].mX,aPoint[0].mY,aPoint[1].mX,aPoint[1].mY);
	}
}

void Graphics::SetClip(float theX, float theY, float theWidth, float theHeight)
{
	DEBUG("Flush -> SetClip()");
	Flush();

	if (theWidth>0 && theHeight>0)
	{
		//
		// Apply clipping to the view matrix.  This will NOT work
		// with rotating, but it will with scaling.
		//
		Point aPoint[2];
		aPoint[0]=Point(theX,theY);
		aPoint[1]=aPoint[0]+Point(theWidth,theHeight);
		mViewMatrix.ProcessPoints(aPoint,2);
		Graphics_Core::Clip(aPoint[0].mX,aPoint[0].mY,aPoint[1].mX,aPoint[1].mY);
		aPoint[1]-=aPoint[0];
		mClipRect=Rect(aPoint[0].mX,aPoint[0].mY,aPoint[1].mX,aPoint[1].mY);
	}
}


void Graphics::PopClip()
{
	Rect aRect=mClipStack.Pop();
	aRect.mX-=mTranslate.mX;
	aRect.mY-=mTranslate.mY;
	Clip(aRect);
}

void Graphics::RenderTo()
{
	mRenderToSprite=NULL;

	EndRender();

	mLetterboxSize=mLetterboxSizeStack.Pop();

	Graphics_Core::SetRenderTarget(-1);
	Graphics_Core::Set2DCamera(mWidth,mHeight);
	mRehupDrawInfo|=DRAWINFO_ALL;
	Graphics_Core::BeginRender();
	Translate();

	UNSETBITS(mFlags,FLAG_RENDERTO);
}

void* Graphics::GetRenderToPixels()
{
    if (gG.IsRenderTo())
    {
        Flush();
        return Graphics_Core::GetRenderTargetPixels();
    }
    else gOut.Out("GetRenderToPixels must be used WHILE still rendering to texture!");
	return NULL;
}


void Graphics::RenderTo(RenderToSprite& theSprite)
{
	mLetterboxSizeStack.Push(mLetterboxSize);
	mLetterboxSize*=0;

	mRenderToSprite=&theSprite;

	EndRender();
	Graphics_Core::SetRenderTarget(theSprite.mTexture);
	IPoint aP=GetTextureDimensions(theSprite.mTexture);
	Graphics_Core::Set2DRenderToCamera(aP.mX,aP.mY,aP.mX,aP.mY);
	Translate();

	Graphics_Core::BeginRender();

	// 
	// OpenGL Kludge...
	// Some version of OpenGL mess up unless we do a hard set of texture after
	// switching to a render target, if the very first draw we do is untextured (like a fillrect).
	// This makes sure that the texture gets hard-set no matter what, as we enter into a renderto.
	//
	SetTexture(-1,true);

	mFlags|=FLAG_RENDERTO;
	mRenderToDimensions=aP;
}

void Graphics::SetRenderToCamera(int thePageWidth, int thePageHeight, int theResolutionWidth, int theResolutionHeight)
{
	EndRender();
	Graphics_Core::Set2DRenderToCamera(thePageWidth,thePageHeight,theResolutionWidth,theResolutionHeight);
	Translate();
	Graphics_Core::BeginRender();
}

void Graphics::Filter(bool theState, bool noFilterIfZoomed)
{
	if (noFilterIfZoomed) if (Graphics_Core::IsZoomed()) return;

	DEBUG("Flush -> Filter()");
	Flush();
	Graphics_Core::Filter(theState);

	mFilter=theState;
}

float Graphics::KludgePixels()
{
	return Graphics_Core::KludgePixels();
}


float Graphics::KludgeTexels()
{
	return Graphics_Core::KludgeTexels();
}

/*
int Graphics::KludgeMipMaps()
{
    return Graphics_Core::KludgeMipMaps();
}
*/

void Graphics::GetAvailableResolutions(Array<IPoint>& theArray)
{
	theArray.Reset();
	Array<Array<int> >& aArray=Graphics_Core::GetAvailableResolutions();
	for (int aCount=0;aCount<aArray.Size();aCount++)
	{
		theArray+=IPoint(aArray[aCount][0],aArray[aCount][1]);
	}
}

void Graphics::RequestZ(float theZDepth)
{
	mZRange.mX=1;
	mZRange.mY=theZDepth;
	Graphics_Core::WantZBuffer(1,theZDepth);
}

void Graphics::RequestZ(float theZNear, float theZFar)
{
	mZRange.mX=theZNear;
	mZRange.mY=theZFar;
	Graphics_Core::WantZBuffer(theZNear, theZFar);
}

#ifndef USEPIPELINE
void Graphics::RequestMaxVertices(int theNumber, int theIndices)
{
	mMaxDrawBuffer=theNumber;
	if (theIndices==-1) theIndices=theNumber*4;	// You need more indices than draw buffers, because indices represents triangles TOUCHING a vertex.  x4 is approximately okay for cubes.  

	mDrawBuffer.GuaranteeSize(mMaxDrawBuffer);
	mIndexBuffer.GuaranteeSize(theIndices);
}
#endif

void Graphics::ZEnable()
{
	if (Graphics_Core::GetZDepth()>0) 
	{
		Flush();
		Graphics_Core::EnableZBuffer();
		ZNormal();
	}
	else gOut.Out("Warning: You called Graphics::EnableZBuffer without requesting a ZBuffer before startup!");
}

void Graphics::ZDisable()
{
	Flush();
	Graphics_Core::DisableZBuffer();
	FogDisable();
}

void Graphics::ZNormal()
{
	Flush();
	Graphics_Core::SetZOrder(0);
}

void Graphics::ZReverse()
{
	Flush();
	Graphics_Core::SetZOrder(1);
}

void Graphics::ZEqual()
{
	Flush();
	Graphics_Core::SetZOrder(2);
}

void Graphics::ZLess()
{
	Flush();
	Graphics_Core::SetZOrder(3);
}

void Graphics::ZGreaterEqual()
{
	Flush();
	Graphics_Core::SetZOrder(4);
}

void Graphics::ZLessEqual()
{
	Flush();
	Graphics_Core::SetZOrder(5);
}


void Graphics::ZEnableWrite()
{
	Flush();
	Graphics_Core::EnableZWrites();
}

void Graphics::ZDisableWrite()
{
	Flush();
	Graphics_Core::DisableZWrites();
}


float Graphics::ZDepth()
{
	return Graphics_Core::GetZDepth();
}

void Graphics::Go2D()
{
	mDrawMode=0;
	//
	// 2D is defaulted in BeginRender -- so just flush everything and
	// reinitialize.
	//
	
	FreshenWorldMatrix();
	EndRender();
	BeginRender();

	//ClearLights();
	FogDisable();
	ZDisable();
	CullNone();
	SetTexture(-1,true);
	SetColor();
	SetClipColor();
	SetShader(NULL);

	mRehupDrawInfo|=DRAWINFO_ALL;
	if (mCurrentShader) mCurrentShader->mRehup|=(DRAWINFO_ALLMATRICES|DRAWINFO_COMBOMATRIX);
}

void Graphics::Set3DOffset(float theX, float theY)
{
	Graphics_Core::Set3DCameraOffset(theX,theY);
}

void Graphics::Go3D(Vector theCameraPos, Vector theCameraLookat, Vector theUpVector, float theFOV)
{
	mDrawMode=1;
	mLookAt=theCameraLookat;

	gG.FreshenWorldMatrix();
	EndRender();
	ZEnable();

	if (!mRenderToSprite) Graphics_Core::Set3DCamera(theCameraPos.mX,theCameraPos.mY,theCameraPos.mZ,theCameraLookat.mX,theCameraLookat.mY,theCameraLookat.mZ,theUpVector.mX,theUpVector.mY,theUpVector.mZ,theFOV);
	else 
	{
		IPoint aP=GetTextureDimensions(mRenderToSprite->mTexture);
		Graphics_Core::Set3DRenderToCamera(theCameraPos.mX,theCameraPos.mY,theCameraPos.mZ,theCameraLookat.mX,theCameraLookat.mY,theCameraLookat.mZ,theUpVector.mX,theUpVector.mY,theUpVector.mZ,theFOV,aP.mX,aP.mY,aP.mX,aP.mY);
	}

	mViewport=GetViewport();
	
//	gOut.Out("VP: %f,%f,%f,%f",mViewport.mX,mViewport.mY,mViewport.mWidth,mViewport.mHeight);

	mRehupDrawInfo|=DRAWINFO_ALL;

	mLast3DProjectionMatrix=GetProjectionMatrix();
	mLast3DViewMatrix=GetViewMatrix();
	mLast3DWorldMatrix=GetWorldMatrix();
	mLast3DViewport=GetViewport();

	//mLast3DDrawMatrix=mLast3DViewMatrix;
	//mLast3DDrawMatrix*=mLast3DProjectionMatrix;

	mLast3DDrawMatrix=GetDrawMatrix();

	mRehupDrawInfo|=DRAWINFO_ALL;
	if (mCurrentShader) mCurrentShader->mRehup|=(DRAWINFO_ALLMATRICES|DRAWINFO_COMBOMATRIX|DRAWINFO_LOOKAT);
}

#ifndef USEPIPELINE
void Graphics::DrawCircle(Point thePos, float theRadius, float theLineWidth)
{
	float aStep=5;
	for (float aAngle=0;aAngle<360;aAngle+=aStep)
	{
		Point aP1=thePos+gMath.AngleToVector(aAngle)*theRadius;
		Point aP2=thePos+gMath.AngleToVector(aAngle+aStep)*theRadius;
		gG.DrawLine(aP1,aP2,theLineWidth);
	}
}
#endif

void Graphics::Threading()
{
    Graphics_Core::EnableThreadGraphics();
}

void Graphics::ThreadLock(bool theState)
{
    if (theState) Graphics_Core::ThreadLock();
    else Graphics_Core::ThreadUnlock();
}

#ifndef USEPIPELINE
void SetVertex(Vertex2D& theVertex, float x, float y, int theDiffuse, float u, float v)
{
	theVertex.mX=x;
	theVertex.mY=y;
	theVertex.mZ=1;
	theVertex.mDiffuse=theDiffuse;
	theVertex.mTextureU=u;
	theVertex.mTextureV=v;
}

void SetVertex(Vertex2D& theVertex, Point thePos, int theDiffuse, float u, float v) 
{
	theVertex.mX=thePos.mX;
	theVertex.mY=thePos.mY;
	theVertex.mZ=1;
	theVertex.mDiffuse=theDiffuse;
	theVertex.mTextureU=u;
	theVertex.mTextureV=v;
}

void SetVertex(Vertex2D& theVertex, Point thePos, int theDiffuse, Point theUV) 
{
	theVertex.mX=thePos.mX;
	theVertex.mY=thePos.mY;
	theVertex.mZ=1;
	theVertex.mDiffuse=theDiffuse;
	theVertex.mTextureU=theUV.mX;
	theVertex.mTextureV=theUV.mY;
}

void SetVertex(Vertex2D& theVertex, Point thePos, int theDiffuse) 
{
	theVertex.mX=thePos.mX;
	theVertex.mY=thePos.mY;
	theVertex.mZ=1;
	theVertex.mDiffuse=theDiffuse;
	theVertex.mTextureU=gG.mClampUVCoordsPos.mX;
	theVertex.mTextureV=gG.mClampUVCoordsPos.mY;
}
#endif

void Graphics::CullNone()
{
	if (mCullState!=0) Flush();
	mCullState=0;
	Graphics_Core::CullNone();
}

void Graphics::CullCCW()
{
	if (mCullState!=1) Flush();
	mCullState=1;
	Graphics_Core::CullCCW();
}

void Graphics::CullCW()
{
	if (mCullState!=2) Flush();
	mCullState=2;
	Graphics_Core::CullCW();
}

/*
void Graphics::GetDrawInfo()
{
	Matrix aWorld,aView,aProjection;
	Graphics_Core::GetMatrix(0,&aWorld.mData);
	Graphics_Core::GetMatrix(1,&aView.mData);
	Graphics_Core::GetMatrix(2,&aProjection.mData);

	mDrawMatrix=aWorld;
	mDrawMatrix*=aView;
	mWorldViewMatrix=mDrawMatrix;
	mBillboardMatrix=mWorldViewMatrix.Billboard();

	mDrawMatrix*=aProjection;

	Graphics_Core::GetViewport((float*)&mViewport);
	//
	// Viewport has to get normalized to local rez...
	// The Camera function does this for display, but we use the viewport
	// to compute things...
	// 
	mViewport.mX=0;
	mViewport.mY=0;
	mViewport.mWidth=WidthF();
	mViewport.mHeight=HeightF();
	mNeedDrawInfo=false;
}
*/

Vector Graphics::ManualTransform(Vector thePos)
{
	//if (mNeedDrawInfo) GetDrawInfo();

	Vector aResult=GetDrawMatrix().ProcessPointNormalized(thePos);
    
	aResult.mY*=-1;
//	aResult.mX=(aResult.mX*0.5f+0.5f)*mViewport.mWidth;
//	aResult.mY=(aResult.mY*0.5f+0.5f)*mViewport.mHeight;
	aResult.mX=(aResult.mX*0.5f+0.5f)*mWidth;
	aResult.mY=(aResult.mY*0.5f+0.5f)*mHeight;
	//aResult.mZ*=mZRange.mY;
	#ifdef USING_DIRECTX
	aResult.mZ=(aResult.mZ)*mZRange.mY;
	#else
	aResult.mZ=(aResult.mZ*0.5f+0.5f)*mZRange.mY;
	#endif


	//aResult.mZ=(1.0f+aResult.mZ)*.05f;
	return aResult;
}

void Graphics::ManualTransform(Vector* theVectors, int theVectorCount)
{
	//if (mNeedDrawInfo) GetDrawInfo();

	Vector* aVPtr=theVectors;
	for (int aCount=0;aCount<theVectorCount;aCount++)
	{
		Vector aResult=mLast3DDrawMatrix.ProcessPointNormalized(*aVPtr);
		if (!gG.IsRenderTo()) aResult.mY*=-1;
//		aResult.mX=(aResult.mX*0.5f+0.5f)*mViewport.mWidth;
//		aResult.mY=(aResult.mY*0.5f+0.5f)*mViewport.mHeight;
		aResult.mX=(aResult.mX*0.5f+0.5f)*mWidth;
		aResult.mY=(aResult.mY*0.5f+0.5f)*mHeight;
		//aResult.mZ=(1.0f+(aResult.mZ*0.5f+0.5f)*(mViewport.mMaxZ-mViewport.mMinZ))*.05f;
		//aResult.mZ=(aResult.mZ*0.5f+0.5f)*mZRange.mY;
		//aResult.mZ*=mZRange.mY;

		//
		// Weird inconsistency between DX and OpenGL here...
		// Looks like DX's Z goes from 0-1 and GL's goes from -.5 to .5
		// Or something.  Either way, the follow produces the results we expect to see.
		//
		#ifdef USING_DIRECTX
		aResult.mZ=(aResult.mZ)*mZRange.mY;
		#else
		aResult.mZ=(aResult.mZ*0.5f+0.5f)*mZRange.mY;
		#endif


		//aResult.mZ=(1.0f+aResult.mZ)*.05f;

		*aVPtr=aResult;
		aVPtr++;
	}
}

void Graphics::ManualTransform(Array<Vector>& theVectorsIn, Array<Vector>& theVectorsOut)
{
	//if (mNeedDrawInfo) GetDrawInfo();

	theVectorsOut.GuaranteeSize(theVectorsIn.Size());

	Vector* aVPtrIn=theVectorsIn.mArray;
	Vector* aVPtrOut=theVectorsOut.mArray;
	for (int aCount=0;aCount<theVectorsIn.Size();aCount++)
	{
		*aVPtrOut=mLast3DDrawMatrix.ProcessPointNormalized(*aVPtrIn);
		if (!gG.IsRenderTo()) aVPtrOut->mY*=-1;
		aVPtrOut->mX=(aVPtrOut->mX*0.5f+0.5f)*mWidth;
		aVPtrOut->mY=(aVPtrOut->mY*0.5f+0.5f)*mHeight;
		#ifdef USING_DIRECTX
		aVPtrOut->mZ=(aVPtrOut->mZ)*mZRange.mY;
		#else
		aVPtrOut->mZ=(aVPtrOut->mZ*0.5f+0.5f)*mZRange.mY;
		#endif
		aVPtrIn++;		
		aVPtrOut++;
	}
}

void Graphics::ManualTransform(Array<Vector>& theVectorsIn, Array<Vector>& theVectorsOut, Matrix& theMatrix, Viewport& theViewport)
{
	theVectorsOut.GuaranteeSize(theVectorsIn.Size());

	Vector* aVPtrIn=theVectorsIn.mArray;
	Vector* aVPtrOut=theVectorsOut.mArray;
	for (int aCount=0;aCount<theVectorsIn.Size();aCount++)
	{
		*aVPtrOut=theMatrix.ProcessPointNormalized(*aVPtrIn);
		if (!gG.IsRenderTo()) aVPtrOut->mY*=-1;
		aVPtrOut->mX=(aVPtrOut->mX*0.5f+0.5f)*theViewport.mWidth;
		aVPtrOut->mY=(aVPtrOut->mY*0.5f+0.5f)*theViewport.mHeight;
		#ifdef USING_DIRECTX
		aVPtrOut->mZ=(aVPtrOut->mZ)*mZRange.mY;
		#else
		aVPtrOut->mZ=(aVPtrOut->mZ*0.5f+0.5f)*mZRange.mY;
		#endif
		aVPtrIn++;		
		aVPtrOut++;
	}
}


Vector Graphics::ManualTransform(Vector thePos, Matrix& theMatrix, Viewport& theViewport)
{
	Vector aResult=theMatrix.ProcessPointNormalized(thePos);

	if (!gG.IsRenderTo()) aResult.mY*=-1;
	aResult.mX=(aResult.mX*0.5f+0.5f)*theViewport.mWidth;
	aResult.mY=(aResult.mY*0.5f+0.5f)*theViewport.mHeight;


	#ifdef USING_DIRECTX
	aResult.mZ=(aResult.mZ)*mZRange.mY;
	#else
	aResult.mZ=(aResult.mZ*0.5f+0.5f)*mZRange.mY;
	#endif
	return aResult;
}

void Graphics::ManualTransform(Vector* theVectors, int theVectorCount, Matrix& theMatrix, Viewport& theViewport)
{
	/*
	//
	// Experiment in turning this into a simple matrix transform...
	// This seems to rotate, etc, properly, but not scale.
	//
	Matrix aWorkMat=theMatrix;
	aWorkMat.Scale(.5f,-.5f,0.0f);
	aWorkMat.Translate(.5f,.5f,0);
	aWorkMat.Scale(theViewport.mWidth,theViewport.mHeight,1.0f);
	aWorkMat.ProcessPoints(theVectors,theVectorCount);
	return;
	/**/

	Vector* aVPtr=theVectors;
	for (int aCount=0;aCount<theVectorCount;aCount++)
	{
		Vector aResult=theMatrix.ProcessPointNormalized(*aVPtr);
		if (!gG.IsRenderTo()) aResult.mY*=-1;
		aResult.mX=(aResult.mX*0.5f+0.5f)*theViewport.mWidth;
		aResult.mY=(aResult.mY*-0.5f+0.5f)*theViewport.mHeight;

		//
		// Weird inconsistency between DX and OpenGL here...
		// Looks like DX's Z goes from 0-1 and GL's goes from -.5 to .5 (like x,y do)
		// Or something.  Either way, the following produces the results we expect to see
		// when rendering 2d things transformed by the 3D matrices
		//
		#ifdef USING_DIRECTX
		aResult.mZ=(aResult.mZ)*mZRange.mY;
		#else
		aResult.mZ=(aResult.mZ*0.5f+0.5f)*mZRange.mY;
		#endif

		*aVPtr=aResult;
		aVPtr++;
	}
}



Matrix& Graphics::GetDrawMatrix()
{
	if (mRehupDrawInfo&DRAWINFO_DRAWMATRIX)
	{
		mDrawMatrix=GetWorldMatrix();mDrawMatrix*=GetViewMatrix();mDrawMatrix*=GetProjectionMatrix();
		mRehupDrawInfo&=~(DRAWINFO_DRAWMATRIX);
	}
	//if (mNeedDrawInfo) GetDrawInfo();
	return mDrawMatrix;
}

Matrix& Graphics::GetBillboardMatrix()
{
	if (mRehupDrawInfo&DRAWINFO_BILLBOARDMATRIX)
	{
		mBillboardMatrix=GetWorldViewMatrix().Billboard();
		//mBillboardMatrix*=GetProjectionMatrix();

		//
		// I believe I can't use this because the vertex's translate needs to happen
		// AFTER the .Billboard multiply... That's the issue-- we need to billboard it,
		// THEN translate it... then we move on.
		//
		/*
		mBillboardMatrix.Identity();
		mBillboardMatrix.Scale(gG.mBillboardScale,gG.mBillboardScale,1);
		mBillboardMatrix*=GetWorldViewMatrix().Billboard();
		// Translate of vertex position should happen here...
		mBillboardMatrix*=GetWorldMatrix();
		mBillboardMatrix*=GetViewMatrix();
		mBillboardMatrix*=GetProjectionMatrix();
		/**/

		mRehupDrawInfo&=~(DRAWINFO_BILLBOARDMATRIX);
	}
	//if (mNeedDrawInfo) GetDrawInfo();
	return mBillboardMatrix;
}

Matrix& Graphics::GetWorldViewMatrix()
{
	if (mRehupDrawInfo&DRAWINFO_WORLDVIEWMATRIX)
	{
		mWorldViewMatrix=GetWorldMatrix();
		mWorldViewMatrix*=GetViewMatrix();
		mRehupDrawInfo&=~(DRAWINFO_WORLDVIEWMATRIX);
	}
	//if (mNeedDrawInfo) GetDrawInfo();
	return mWorldViewMatrix;
}

Viewport Graphics::GetViewport()
{
	if (mRehupDrawInfo&DRAWINFO_VIEWPORT)
	{
		Graphics_Core::GetViewport((float*)&mViewport);
		//
		// Viewport has to get normalized to local rez...
		// The Camera function does this for display, but we use the viewport
		// to compute things...
		// 
		//*
		mViewport.mX=0;
		mViewport.mY=0;
		mViewport.mWidth=WidthF();
		mViewport.mHeight=HeightF();
		/**/
		mRehupDrawInfo&=~(DRAWINFO_VIEWPORT);
		
	}
	return mViewport;
}

Matrix& Graphics::GetWorldMatrix()
{
	if (mRehupDrawInfo&(DRAWINFO_WORLDMATRIX|DRAWINFO_WORLDMATRIXN)) 
	{
		Graphics_Core::GetMatrix(0,&mWorldMatrix.mData);
		mWorldMatrixN=mWorldMatrix;mWorldMatrixN.Normalize();
		mRehupDrawInfo&=~(DRAWINFO_WORLDMATRIX|DRAWINFO_WORLDMATRIXN);
	}
	return mWorldMatrix;
}

Matrix& Graphics::GetWorldMatrixN()
{
	if (mRehupDrawInfo&(DRAWINFO_WORLDMATRIX|DRAWINFO_WORLDMATRIXN)) 
	{
		Graphics_Core::GetMatrix(0,&mWorldMatrix.mData);
		mWorldMatrixN=mWorldMatrix;mWorldMatrixN.Normalize();
		mRehupDrawInfo&=~(DRAWINFO_WORLDMATRIX|DRAWINFO_WORLDMATRIXN);
	}
	return mWorldMatrixN;
}

Matrix& Graphics::GetViewMatrix()
{
	if (mRehupDrawInfo&(DRAWINFO_VIEWMATRIX|DRAWINFO_VIEWMATRIXN)) 
	{
		Graphics_Core::GetMatrix(1,&mViewMatrix.mData);
		mViewMatrixN=mViewMatrix;mViewMatrixN.Normalize();
		mRehupDrawInfo&=~(DRAWINFO_VIEWMATRIX|DRAWINFO_VIEWMATRIXN);
	}
	return mViewMatrix;
}

Matrix& Graphics::GetViewMatrixN()
{
	if (mRehupDrawInfo&(DRAWINFO_VIEWMATRIX|DRAWINFO_VIEWMATRIXN)) 
	{
		Graphics_Core::GetMatrix(1,&mViewMatrix.mData);
		mViewMatrixN=mViewMatrix;mViewMatrixN.Normalize();
		mRehupDrawInfo&=~(DRAWINFO_VIEWMATRIX|DRAWINFO_VIEWMATRIXN);
	}
	return mViewMatrixN;
}

Matrix& Graphics::GetProjectionMatrix()
{
	if (mRehupDrawInfo&(DRAWINFO_PROJECTIONMATRIX|DRAWINFO_PROJECTIONMATRIXN)) 
	{
		Graphics_Core::GetMatrix(2,&mProjectionMatrix.mData);
		mProjectionMatrixN=mProjectionMatrix;mProjectionMatrixN.Normalize();
		mRehupDrawInfo&=~(DRAWINFO_PROJECTIONMATRIX|DRAWINFO_PROJECTIONMATRIXN);
	}
	return mProjectionMatrix;
}

Matrix& Graphics::GetProjectionMatrixN()
{
	if (mRehupDrawInfo&(DRAWINFO_PROJECTIONMATRIX|DRAWINFO_PROJECTIONMATRIXN)) 
	{
		Graphics_Core::GetMatrix(2,&mProjectionMatrix.mData);
		mProjectionMatrixN=mProjectionMatrix;mProjectionMatrixN.Normalize();
		mRehupDrawInfo&=~(DRAWINFO_PROJECTIONMATRIX|DRAWINFO_PROJECTIONMATRIXN);
	}
	return mProjectionMatrixN;
}

Matrix Graphics::GetTextureMatrix()
{
	Matrix aWorld;
	Graphics_Core::GetMatrix(3,&aWorld.mData);
	return aWorld;
}

void Graphics::FogEnable(Color theColor, float theStart, float theEnd)
{
	Flush();
	Graphics_Core::EnableFog(true,theColor.ToInt(),theStart,theEnd);
}

void Graphics::FogDisable()
{
	Flush();
	Graphics_Core::DisableFog();
}

Vertex2D MakeVertex2D(Vector theVector, int theDiffuse, Point theUV)
{
	Vertex2D aV;
	aV.mX=theVector.mX;
	aV.mY=theVector.mY;
	aV.mZ=theVector.mZ;
	aV.mDiffuse=theDiffuse;
	aV.mTextureU=theUV.mX;
	aV.mTextureV=theUV.mY;
	return aV;
}

Vertex2DMT MakeVertex2DMT(Vector theVector, int theDiffuse, Point theUV, Point theUV2)
{
	Vertex2DMT aV;
	aV.mX=theVector.mX;
	aV.mY=theVector.mY;
	aV.mZ=theVector.mZ;
	aV.mDiffuse=theDiffuse;
	aV.mTextureU=theUV.mX;
	aV.mTextureV=theUV.mY;
	aV.mTextureU2=theUV2.mX;
	aV.mTextureV2=theUV2.mY;
	return aV;
}

void Graphics::ClampTextures()
{
	mTextureMode=0;
}

void Graphics::WrapTextures()
{
	mTextureMode=1;
}


void Graphics::SetZBias(float theValue)
{
	if (mZBias!=theValue)
	{
		mZBias=theValue;
		if (mCurrentShader) 
		{
			mCurrentShader->mRehup|=DRAWINFO_ZBIAS;
			Flush();
			//Graphics_Core::SetZBias(theValue);
		}
		//else gOut.Debug("! SetZBIAS: Depreciated except when using a shader that explicitely accounts for it!");
	}
}

Font* Graphics::TempFont()
{
	static Font* aTempFont=NULL;
	if (!aTempFont)
	{
		aTempFont=new Font;

#ifdef _WIN32
#ifdef _DEBUG
		const unsigned char aWorkFontPNG[5647]={137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,64,8,6,0,0,0,210,214,127,127,0,0,0,4,115,66,73,84,8,8,8,8,124,8,100,136,0,0,21,198,73,68,65,84,120,156,237,157,103,216,93,197,113,199,255,23,212,11,18,66,244,34,138,232,33,18,136,38,155,24,16,166,87,17,9,12,36,20,131,177,33,216,16,131,147,135,184,134,4,27,176,141,73,128,216,116,236,208,123,49,69,198,32,131,8,2,35,4,196,1,81,44,28,64,128,64,70,72,72,72,136,246,243,135,217,243,222,185,115,247,156,123,239,171,130,62,48,207,179,207,61,103,119,118,118,118,118,118,119,118,118,247,220,154,36,1,53,37,168,213,106,248,247,8,181,90,141,152,39,166,71,122,62,189,138,118,59,208,14,189,136,211,110,185,173,234,214,41,148,201,178,149,140,187,83,142,212,25,223,69,158,21,92,92,207,128,179,133,164,105,146,222,151,52,69,210,182,25,58,107,75,122,213,253,122,248,164,162,252,13,37,221,35,105,190,164,63,75,186,72,82,255,144,215,135,69,146,14,146,74,43,121,88,194,251,82,174,48,151,103,69,73,227,37,221,38,233,13,73,179,37,61,36,233,168,12,237,29,37,77,149,244,158,164,71,36,109,226,210,94,144,180,65,122,30,153,202,222,49,189,111,36,233,197,192,194,54,9,103,155,16,31,235,249,145,75,59,72,86,239,15,36,61,41,105,39,151,182,153,76,222,93,237,231,120,127,41,148,243,130,76,222,74,191,211,60,3,94,1,158,10,204,125,75,210,21,146,86,147,116,159,164,111,22,5,185,194,118,148,244,168,251,109,106,160,2,63,228,187,90,210,111,37,173,46,105,115,73,31,74,250,113,40,127,5,23,14,149,116,149,146,18,100,42,61,78,210,173,233,183,76,73,214,148,52,89,210,254,146,126,40,105,120,10,231,73,58,85,210,245,33,239,149,146,190,39,105,21,73,119,72,186,212,209,186,71,210,23,211,243,110,146,22,72,26,147,222,247,146,116,111,160,245,85,73,191,148,116,124,136,247,117,252,174,164,51,92,25,215,75,58,89,82,31,73,255,46,233,114,151,246,156,164,233,146,118,15,117,28,35,105,150,76,113,11,184,75,210,30,233,121,247,84,151,58,184,198,33,52,214,115,192,208,244,60,12,120,38,224,150,65,142,222,233,192,79,221,251,66,96,128,123,239,13,188,29,243,6,254,198,2,31,248,198,77,241,253,129,217,192,90,192,59,64,191,12,78,13,152,2,252,67,84,200,20,250,0,143,85,164,247,3,230,187,247,189,129,27,211,243,221,192,153,192,253,233,253,215,192,254,14,119,32,240,82,162,49,189,168,119,224,109,135,148,207,151,249,27,224,22,96,183,162,78,33,223,97,192,245,33,207,213,192,49,1,111,119,224,214,244,124,51,176,147,151,79,129,148,107,192,121,64,15,215,64,239,101,4,243,40,176,163,251,245,105,5,173,211,128,235,66,218,173,192,127,37,102,55,205,208,165,164,17,115,241,227,129,187,210,243,61,192,184,12,206,209,78,192,7,2,255,151,26,244,135,192,171,41,126,23,224,233,6,225,212,243,175,11,60,225,248,232,11,204,72,114,121,29,24,4,204,73,191,179,49,165,44,112,191,10,156,151,158,207,3,142,15,13,52,16,248,29,169,179,185,248,85,129,7,129,69,137,215,81,33,189,87,226,125,229,244,62,8,120,45,241,22,241,94,75,188,190,80,240,213,206,8,240,81,120,255,56,188,247,6,222,5,86,74,191,189,51,141,120,50,240,135,34,205,149,183,10,112,17,48,53,209,125,22,216,186,172,161,131,162,198,222,115,93,18,114,13,248,26,112,173,175,100,122,158,0,236,11,236,153,132,182,91,106,164,111,81,239,201,253,176,145,41,167,120,255,130,141,64,190,126,247,0,167,0,183,167,247,137,152,66,61,16,240,158,0,70,166,231,145,216,72,228,211,47,5,70,103,120,246,163,211,113,192,27,153,244,115,128,19,211,243,9,192,207,74,232,220,10,252,27,112,69,148,107,149,112,103,1,61,93,99,207,11,13,84,6,30,231,234,36,240,117,74,42,87,195,52,248,116,224,127,67,222,202,17,192,9,103,94,40,255,221,20,239,241,222,194,122,212,239,129,3,28,189,190,192,169,212,123,220,235,25,62,183,194,77,95,46,237,27,73,70,69,3,124,7,27,37,255,169,162,174,49,28,6,252,164,36,237,25,224,16,108,20,46,155,254,134,99,83,87,45,213,109,83,143,227,240,142,75,178,25,27,211,171,20,224,15,192,218,212,135,192,105,129,193,127,196,52,174,248,205,14,227,216,188,122,101,72,155,77,163,13,176,18,169,247,149,240,82,139,66,72,113,7,82,159,123,139,240,64,209,200,14,111,17,166,196,11,177,33,183,230,242,127,46,61,159,6,156,31,104,173,11,92,130,27,193,28,205,141,19,159,27,165,247,209,233,125,68,86,200,121,153,79,167,25,138,178,119,4,158,2,62,76,245,110,104,60,135,119,95,74,235,146,67,166,156,181,128,5,164,169,169,82,1,92,184,0,211,234,254,192,191,226,134,143,148,126,3,54,255,22,191,145,185,130,222,138,88,239,30,225,104,95,132,13,73,171,3,131,177,161,243,94,151,30,121,25,139,245,174,56,12,255,55,112,104,224,235,112,224,87,1,239,85,172,183,76,1,190,142,41,223,88,172,7,175,7,252,51,102,244,14,113,121,70,99,6,94,63,79,171,205,158,221,174,2,116,28,50,52,198,97,35,220,184,92,185,109,241,150,18,62,72,13,82,32,110,11,188,152,226,159,3,182,12,2,40,134,246,134,33,190,68,161,118,199,230,225,226,125,0,214,179,222,198,52,243,14,96,205,160,0,30,186,122,128,11,189,128,63,166,223,56,45,76,47,226,83,184,26,27,154,71,97,134,208,28,108,254,255,159,84,254,77,88,47,241,244,179,189,179,85,195,46,75,72,124,246,192,108,169,30,221,230,47,17,186,142,188,165,223,164,61,85,105,85,233,75,34,148,209,111,81,238,118,216,180,179,121,5,206,215,105,94,138,117,220,179,151,37,44,9,254,186,4,215,29,6,188,203,117,113,133,147,115,103,46,142,107,54,147,247,43,146,206,150,57,126,110,145,57,82,122,72,218,82,210,137,50,239,218,158,146,222,89,18,124,87,241,188,172,92,193,173,226,165,70,79,96,132,129,146,110,146,9,228,22,73,3,92,90,153,155,215,199,175,32,233,66,73,243,100,238,200,47,148,224,229,104,117,197,57,230,79,146,53,96,1,133,171,180,203,77,44,243,42,62,233,9,185,202,94,34,105,23,73,235,73,186,93,230,6,126,77,210,249,50,207,217,78,106,108,252,130,254,39,50,101,137,252,31,23,248,235,226,219,197,141,146,121,81,231,202,220,221,235,133,244,232,238,126,88,210,214,129,222,81,146,222,74,97,127,153,87,239,181,80,111,15,59,43,47,191,149,213,236,174,111,24,70,14,166,190,38,174,97,78,139,173,176,57,245,48,108,205,233,231,232,210,117,122,10,39,97,198,93,63,204,64,121,177,100,88,109,160,149,161,83,195,150,92,113,37,81,216,5,93,171,3,224,114,224,200,92,239,234,198,20,83,208,39,201,230,249,192,243,99,36,187,165,132,239,45,177,181,251,193,152,33,189,49,230,43,56,54,214,61,133,158,152,31,99,106,72,191,16,91,185,140,75,105,223,167,110,196,198,85,81,13,115,42,17,226,55,73,121,27,100,237,5,211,39,53,144,55,196,254,159,186,39,112,69,204,40,234,68,1,30,38,121,175,50,129,50,90,153,244,147,129,59,113,70,78,9,141,53,18,143,61,155,42,217,13,69,112,244,41,73,219,153,198,14,19,113,175,167,177,177,107,152,18,92,90,130,95,195,58,203,130,144,190,90,122,238,145,222,135,196,252,174,94,187,144,87,128,153,192,151,163,172,189,64,142,197,122,143,103,230,125,26,97,81,5,227,185,248,217,152,7,109,14,240,56,176,65,9,94,3,83,33,253,20,204,82,239,91,224,103,122,104,209,19,206,196,150,115,77,61,186,133,162,110,140,185,134,223,194,188,133,69,252,171,192,183,19,110,47,242,245,188,8,56,168,164,94,51,49,165,172,50,80,61,126,225,153,124,168,66,206,173,222,127,135,41,65,151,76,83,252,234,30,63,39,144,9,152,123,212,103,154,65,221,19,152,99,188,12,10,156,143,129,31,99,195,213,56,172,23,119,162,0,167,164,60,167,20,184,1,167,104,248,15,128,191,195,70,172,99,92,92,214,235,149,41,255,102,204,9,244,121,108,89,89,196,15,199,252,0,96,202,127,88,134,255,65,152,114,15,202,208,253,128,176,52,107,33,199,247,129,71,72,203,237,40,155,170,119,26,123,127,14,47,43,107,207,212,91,152,127,222,103,250,5,230,188,233,141,249,218,31,201,8,160,74,163,23,58,193,244,194,70,130,78,20,224,170,244,252,19,234,222,186,178,222,116,18,102,179,52,217,5,109,40,192,44,204,13,156,171,79,129,187,23,141,123,35,62,255,193,216,72,16,227,95,35,249,22,2,207,77,67,120,139,178,219,85,128,162,247,55,196,231,234,157,19,72,131,182,58,70,111,195,124,237,79,0,91,228,24,171,16,236,52,204,149,90,195,108,140,119,74,42,222,64,203,197,21,195,110,111,108,183,175,203,62,201,244,168,231,129,245,171,232,86,240,249,33,102,227,68,186,115,176,78,65,38,143,127,174,97,142,164,47,132,248,107,48,31,188,199,219,153,230,109,245,172,189,18,211,171,222,221,115,132,182,21,96,38,105,158,40,233,97,165,154,89,33,216,179,176,77,158,222,192,151,112,187,116,57,97,86,208,169,97,243,244,4,156,215,207,225,254,45,245,125,241,238,140,0,111,146,70,191,16,110,199,230,100,104,92,105,228,26,98,77,108,85,224,233,110,141,173,2,246,195,108,152,109,49,143,234,49,85,114,204,40,119,83,155,180,209,70,120,220,118,20,224,110,146,1,180,4,21,160,47,240,75,96,46,182,34,88,43,230,207,209,42,17,112,13,56,130,52,212,6,220,71,48,79,95,141,70,187,32,186,142,179,2,194,122,239,137,88,15,246,54,192,102,216,200,7,54,10,238,151,227,217,197,29,31,232,214,176,169,99,10,102,217,191,132,77,85,149,114,92,92,104,167,125,114,25,142,163,121,21,176,37,48,25,155,203,39,147,57,184,145,161,211,237,144,163,83,70,187,27,101,190,140,109,58,213,176,81,196,239,60,110,140,237,124,206,196,78,250,84,41,75,91,124,180,226,173,172,158,75,2,218,105,159,92,134,62,192,159,168,111,255,214,128,39,49,235,186,47,230,136,153,92,165,0,101,133,45,174,18,116,42,220,18,90,15,99,211,209,64,204,231,223,84,151,118,149,177,170,252,178,244,118,120,94,28,249,181,82,130,202,60,46,114,60,118,6,45,87,200,0,74,246,235,43,20,0,236,172,192,116,154,135,228,2,154,232,185,184,98,56,7,91,34,237,17,120,138,48,31,179,132,27,14,69,36,220,17,216,112,190,48,253,118,178,103,95,214,64,123,96,35,203,76,210,244,16,210,159,198,78,233,120,158,79,72,241,197,251,77,94,46,25,249,149,213,213,67,165,157,227,222,199,98,203,217,115,154,242,4,38,99,67,244,199,246,238,111,203,53,88,11,5,56,63,229,143,70,84,174,130,145,89,191,255,127,40,118,210,103,231,146,252,53,236,76,221,143,72,203,213,178,186,197,122,182,161,0,77,117,77,241,47,99,203,174,93,128,87,74,26,240,143,212,15,190,12,72,121,60,223,107,98,219,211,125,50,249,203,66,172,119,187,120,13,157,176,180,241,67,67,12,4,254,156,50,31,16,137,183,33,184,213,114,121,50,12,198,180,156,17,119,8,233,184,85,73,158,66,200,11,58,16,102,199,130,116,52,231,96,211,227,48,204,105,86,230,203,216,39,189,239,131,173,86,98,195,92,78,114,25,183,169,120,101,241,67,177,83,65,115,19,189,40,95,114,121,139,221,192,79,194,111,1,243,37,173,42,233,72,217,206,94,167,48,171,27,121,36,187,164,114,91,136,187,81,205,119,7,228,42,212,83,210,177,146,30,203,208,123,85,210,26,238,125,172,172,174,99,93,220,26,146,94,113,239,197,46,93,195,179,43,239,126,73,223,151,157,187,63,182,164,30,147,84,191,47,48,38,189,75,106,216,165,188,70,118,89,101,113,225,44,217,174,237,186,146,70,135,180,21,194,111,29,146,134,204,77,191,239,150,244,130,174,158,85,166,133,25,154,177,135,102,223,59,192,37,147,199,195,2,108,154,90,55,211,155,110,164,209,207,127,5,118,228,250,10,23,183,47,118,188,173,157,33,119,67,204,161,243,54,182,211,150,27,33,192,156,103,79,166,247,169,216,169,224,88,183,161,152,39,118,113,71,128,215,169,175,116,54,168,146,105,78,51,126,155,126,239,115,105,211,36,125,94,214,179,246,146,244,96,89,131,47,45,112,12,23,183,103,34,20,241,135,200,246,236,143,145,52,35,131,55,89,141,215,165,246,146,157,47,216,203,197,109,147,240,84,171,213,136,247,11,67,220,229,146,206,145,237,213,159,45,105,132,164,11,50,229,78,147,93,155,27,46,105,125,73,79,103,112,230,74,26,156,137,239,20,86,149,157,189,144,236,188,64,123,144,209,238,34,108,143,157,74,157,143,121,225,214,137,189,162,5,205,82,13,204,244,172,152,150,179,1,178,222,56,23,78,164,126,3,38,246,166,209,46,109,52,245,19,180,15,80,223,103,184,133,116,185,37,39,155,16,183,0,59,201,92,195,14,206,190,133,219,180,10,252,221,134,205,243,247,148,212,117,117,220,113,244,156,188,218,140,159,65,221,163,185,110,149,252,61,52,244,170,160,229,143,203,78,167,12,148,245,148,38,173,10,21,174,133,2,98,143,173,58,125,20,211,226,93,192,131,210,251,161,49,163,227,247,231,137,215,220,37,210,169,146,254,42,61,239,175,116,15,48,253,30,144,158,183,72,120,202,212,37,198,77,145,244,13,73,43,201,78,17,173,44,233,104,217,253,195,8,147,36,29,46,55,255,7,216,78,141,119,249,186,11,183,75,250,178,76,6,167,183,157,171,74,59,42,122,64,25,116,226,5,195,197,231,242,122,63,64,110,68,104,224,153,250,220,252,44,238,126,160,195,159,140,205,183,83,169,223,121,28,138,109,90,13,193,206,29,120,250,67,176,237,232,119,177,83,203,43,187,180,13,49,159,195,2,108,59,120,123,224,12,108,7,53,142,0,219,167,231,47,100,234,93,195,70,135,163,51,178,106,168,95,85,91,165,248,65,216,40,243,70,162,23,101,218,148,207,103,206,249,216,139,240,14,205,204,181,211,192,59,96,107,242,249,216,16,119,100,174,130,49,111,7,10,212,50,79,72,59,55,53,208,189,1,111,2,118,143,224,167,129,254,5,152,7,116,0,230,208,186,176,19,57,180,25,214,161,249,8,123,95,74,54,179,114,117,235,174,188,98,230,31,101,4,54,1,115,228,188,144,132,115,111,142,96,69,120,22,219,169,235,131,29,71,154,21,26,159,44,67,29,86,168,21,174,195,31,159,202,60,38,208,41,142,74,141,11,241,127,162,222,235,135,80,191,68,218,49,63,21,56,183,38,25,249,184,227,49,143,93,254,26,215,146,134,10,129,173,142,105,253,156,244,27,183,139,115,224,189,133,62,244,44,26,188,172,145,50,252,236,70,125,39,109,10,48,38,228,93,29,27,134,95,193,14,101,120,250,100,234,179,14,214,171,6,135,122,172,156,226,227,253,197,247,29,189,21,105,52,64,111,192,46,154,122,126,198,80,191,165,92,195,118,88,247,9,56,251,6,156,150,74,178,180,219,191,202,13,188,14,54,111,174,159,126,135,85,48,58,20,235,241,126,203,119,21,204,66,190,25,219,10,205,42,64,73,217,35,177,105,99,127,108,8,222,27,243,185,143,116,248,191,2,254,30,155,251,174,202,209,47,171,99,43,129,167,231,87,169,187,113,87,73,229,23,120,171,98,103,247,252,145,185,137,216,206,98,241,190,41,118,214,176,24,222,123,165,247,77,114,229,46,211,70,207,40,64,1,109,25,113,25,193,222,132,221,201,243,184,147,176,35,204,123,99,123,235,190,129,158,195,12,169,90,18,218,180,80,246,181,164,123,244,46,156,64,227,161,146,55,169,223,195,223,196,243,95,194,99,91,211,141,139,187,8,51,166,250,99,54,192,149,1,119,60,118,96,164,134,77,35,231,68,90,152,221,81,224,156,70,227,71,50,106,216,94,192,253,216,121,131,107,112,119,251,151,25,56,5,168,236,25,81,136,46,238,136,164,0,49,255,28,236,88,248,96,224,178,160,0,103,99,6,86,45,253,70,3,236,13,156,71,47,197,173,149,226,11,26,254,148,114,175,22,10,144,77,115,233,197,29,199,232,161,187,11,51,98,239,165,249,3,14,181,212,104,23,98,163,213,192,140,124,6,97,27,66,91,97,199,238,7,5,26,55,96,103,49,250,97,167,166,206,252,84,21,32,19,231,225,163,12,206,154,88,239,29,154,73,59,24,235,233,51,176,30,228,21,224,115,216,39,80,106,216,210,229,139,65,48,31,208,252,81,137,222,52,206,195,47,81,119,198,52,56,62,202,234,23,131,75,47,110,57,119,218,1,182,76,180,207,44,73,175,97,35,217,92,224,43,153,114,223,166,126,251,184,15,54,141,46,55,10,224,195,17,94,59,93,252,93,216,29,251,178,202,87,133,87,48,67,110,22,205,55,124,223,192,174,109,123,252,141,104,156,135,207,194,78,43,15,4,126,158,107,228,80,191,145,152,255,126,2,201,186,119,233,231,98,74,218,137,31,163,134,245,254,51,176,35,233,13,35,64,192,195,231,119,105,115,168,219,8,171,144,190,67,180,244,90,59,3,57,5,8,105,67,48,151,112,159,32,140,227,176,51,127,77,243,107,155,2,188,4,91,107,223,158,73,187,142,198,47,111,124,23,187,33,228,55,107,250,39,188,57,216,105,31,175,0,4,122,36,94,251,99,54,73,92,211,79,198,185,129,93,131,76,196,148,238,196,140,130,141,7,142,74,207,135,224,108,128,118,228,155,226,239,76,116,86,195,54,172,62,94,110,20,192,85,244,108,215,24,69,24,134,245,166,65,33,222,135,187,73,67,123,18,250,29,65,192,251,97,67,250,215,124,217,41,109,68,18,252,1,216,105,218,199,176,121,118,235,138,242,98,3,69,5,216,32,61,247,37,29,224,72,161,55,54,68,199,81,232,23,152,242,13,194,118,13,61,253,85,177,47,115,248,242,38,145,86,1,29,200,119,3,236,211,46,115,176,189,132,133,17,111,169,67,11,6,251,39,97,197,79,155,253,154,114,40,4,178,57,182,21,218,7,59,6,213,37,156,148,62,40,225,15,203,40,64,13,83,158,169,152,31,224,66,172,135,222,157,19,80,89,29,66,154,31,193,252,241,182,209,184,75,47,46,207,12,234,23,70,54,8,117,187,145,116,68,205,225,239,154,228,210,137,2,248,176,37,118,153,100,185,82,128,67,113,159,120,43,97,188,42,156,143,29,200,60,55,67,99,119,234,123,229,237,78,33,91,87,52,114,19,157,80,191,194,23,223,15,51,32,139,60,223,196,173,66,92,158,174,203,50,184,85,70,12,57,126,219,225,45,197,189,128,221,45,28,140,77,99,215,46,111,10,112,37,182,198,205,26,120,25,252,40,148,226,67,74,235,103,104,188,77,250,160,98,7,124,214,202,120,105,67,1,30,197,70,178,3,105,252,50,215,141,228,191,45,248,50,245,17,96,152,151,81,133,130,118,234,26,222,21,91,38,206,195,142,139,13,249,84,21,32,35,132,103,200,239,145,151,41,76,236,37,231,99,115,99,217,247,235,58,233,253,101,249,7,96,75,173,41,45,20,96,4,182,100,189,133,198,47,133,205,32,255,141,163,255,192,110,55,175,132,57,133,62,9,229,238,137,125,252,106,97,42,123,215,144,238,97,17,54,215,111,211,174,18,45,51,8,204,70,33,188,23,181,178,3,5,216,28,59,130,221,27,179,1,114,95,4,45,83,128,50,240,194,27,149,26,102,58,118,49,116,100,133,2,180,21,66,158,85,176,189,134,89,152,129,54,215,225,22,31,127,56,128,70,87,245,95,7,5,40,158,251,97,67,124,215,58,127,121,83,128,150,195,107,136,111,71,1,252,42,96,15,26,55,65,154,26,181,130,78,217,8,0,102,157,55,125,198,173,170,126,237,40,65,136,235,137,45,121,189,213,127,53,182,15,225,241,78,196,60,131,57,5,168,97,190,135,185,153,58,68,248,244,20,160,3,161,117,220,112,153,252,45,21,160,5,207,163,128,139,177,17,224,103,148,92,246,72,184,67,49,133,156,135,245,234,245,3,47,223,33,25,164,46,207,101,216,71,46,22,97,211,216,112,151,103,58,233,200,187,195,95,155,116,180,43,35,139,222,216,145,246,248,21,209,172,188,22,175,69,59,132,156,192,83,220,166,216,41,153,249,216,9,218,141,200,84,206,225,47,113,5,40,19,78,136,27,136,121,4,159,168,80,128,75,48,103,210,0,236,236,131,239,205,96,27,77,209,79,95,21,222,167,249,58,125,47,154,207,44,70,56,37,228,105,170,255,50,135,10,5,120,24,219,191,238,131,89,206,19,93,229,230,227,190,197,147,226,122,208,248,61,225,181,50,2,40,109,212,12,63,57,104,105,48,150,212,239,117,234,159,73,25,72,243,119,143,87,243,249,35,189,76,57,51,73,247,40,93,250,122,52,110,86,249,250,246,74,178,124,45,228,105,146,201,146,110,223,86,80,117,80,179,248,196,217,34,217,177,241,29,92,218,83,74,255,152,225,152,30,174,198,195,141,163,36,221,169,198,63,69,200,66,73,229,87,200,132,38,252,226,32,107,17,74,132,185,182,236,51,107,146,29,194,124,62,148,53,171,160,229,226,226,37,25,255,254,160,164,253,10,94,82,220,1,178,127,31,201,193,135,146,38,202,14,108,70,240,159,138,91,246,224,53,49,196,61,132,25,111,189,49,43,119,146,19,238,24,108,139,116,227,148,190,57,230,5,251,27,135,243,3,224,123,37,189,30,204,127,62,39,149,19,231,211,166,222,222,138,231,86,105,41,126,179,212,11,119,141,61,181,138,70,166,183,142,160,126,96,101,37,108,53,240,38,153,79,222,167,80,216,0,191,89,222,70,128,42,5,216,30,51,128,192,230,182,237,2,179,135,99,30,181,69,152,81,52,62,164,223,137,109,178,204,193,166,19,191,187,7,182,57,51,8,155,23,47,107,37,24,47,160,86,56,57,97,166,184,137,52,127,182,165,59,10,80,195,148,168,234,200,154,135,185,88,135,25,158,163,185,180,219,184,18,42,20,224,247,88,207,239,141,217,0,147,2,243,173,26,232,117,204,89,210,3,115,38,221,17,132,83,88,213,67,104,254,51,132,50,136,56,135,81,191,188,90,121,136,50,229,89,72,243,63,106,84,42,64,224,121,73,123,2,151,107,5,240,127,25,211,159,198,187,129,149,61,174,36,221,59,82,160,190,15,222,117,224,178,34,111,153,240,254,19,243,3,148,126,19,168,140,167,24,87,38,147,20,134,102,20,162,116,216,110,85,135,156,82,125,90,80,101,4,62,46,251,55,176,62,178,239,228,250,111,240,122,131,101,162,236,251,180,85,176,162,204,152,244,176,106,250,29,44,105,102,75,78,243,240,3,73,11,101,55,137,227,223,222,229,224,19,169,110,236,121,227,49,131,251,161,236,150,81,95,73,223,14,105,143,203,62,62,221,95,42,53,98,227,95,194,229,140,189,42,249,47,27,168,24,1,10,63,192,123,233,215,187,114,253,50,106,23,210,7,10,67,254,231,49,59,162,7,246,17,198,139,131,230,95,128,45,201,78,77,61,185,59,83,64,83,111,106,209,171,179,61,182,68,38,197,168,50,27,59,179,224,233,143,196,246,10,94,194,220,209,163,104,174,127,219,246,201,167,10,57,166,218,168,192,237,85,149,74,239,59,98,123,0,239,99,94,56,127,181,10,108,27,118,30,118,151,96,165,208,184,29,205,159,25,5,232,74,91,92,153,180,8,253,176,29,211,197,42,239,83,133,78,53,181,29,45,110,67,112,148,149,181,132,20,160,225,170,219,82,144,215,72,108,3,234,37,108,100,27,181,180,202,91,218,208,35,253,230,230,165,79,148,153,163,74,230,203,182,113,188,160,42,240,202,156,34,237,206,153,179,219,196,235,46,92,38,233,98,217,191,125,206,95,202,101,45,85,104,57,31,181,211,224,157,64,43,5,104,135,159,72,163,44,207,146,230,189,140,191,165,81,206,103,240,25,124,6,159,193,210,135,191,0,130,114,23,231,181,251,48,6,0,0,0,0,73,69,78,68,174,66,96,130};
		const unsigned char aWorkFontBUNDLE[5573]={0,0,0,232,65,0,0,60,66,0,0,64,64,0,0,64,64,3,0,0,0,3,0,0,0,0,0,64,64,0,0,64,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,65,0,0,64,64,0,0,232,65,70,0,44,0,0,0,128,191,70,0,46,0,0,0,128,191,80,0,44,0,0,0,128,191,80,0,46,0,0,0,128,191,84,0,44,0,0,0,128,191,84,0,46,0,0,0,128,191,84,0,58,0,0,0,128,191,84,0,59,0,0,0,128,191,84,0,97,0,0,0,128,191,84,0,99,0,0,0,128,191,84,0,101,0,0,0,128,191,84,0,111,0,0,0,128,191,84,0,115,0,0,0,128,191,86,0,44,0,0,0,128,191,86,0,46,0,0,0,128,191,89,0,44,0,0,0,128,191,89,0,45,0,0,0,128,191,89,0,46,0,0,0,128,191,89,0,101,0,0,0,128,191,89,0,111,0,0,0,128,191,89,0,113,0,0,0,128,191,0,0,0,0,33,0,0,0,64,64,0,0,128,63,0,0,160,192,0,0,0,0,66,0,0,176,65,0,0,64,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,64,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,34,0,0,0,128,64,0,0,128,63,0,0,0,193,0,0,0,0,0,0,0,200,65,0,0,160,64,0,0,160,64,24,0,0,0,24,0,0,0,0,0,160,64,0,0,160,64,0,0,0,63,0,0,0,63,0,0,0,0,0,35,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,64,65,0,0,0,0,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,36,0,0,0,224,64,0,0,64,64,0,0,128,192,0,0,0,138,66,0,0,48,65,0,0,224,64,0,0,64,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,64,65,0,0,0,63,0,0,0,0,0,0,0,0,0,37,0,0,0,48,65,0,0,160,64,0,0,160,192,0,0,0,52,66,0,0,48,65,0,0,48,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,48,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,38,0,0,0,0,65,0,0,128,64,0,0,160,192,0,0,0,152,66,0,0,176,65,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,39,0,0,0,0,64,0,0,128,63,0,0,0,193,0,0,0,64,65,0,0,92,66,0,0,64,64,0,0,160,64,24,0,0,0,24,0,0,0,0,0,64,64,0,0,160,64,0,0,0,63,0,0,0,63,0,0,0,0,0,40,0,0,0,128,64,0,0,0,64,0,0,128,192,0,0,0,0,0,0,0,64,65,0,0,160,64,0,0,80,65,24,0,0,0,24,0,0,0,0,0,160,64,0,0,80,65,0,0,0,63,0,0,0,63,0,0,0,0,0,41,0,0,0,128,64,0,0,128,63,0,0,128,192,0,0,0,188,66,0,0,176,65,0,0,160,64,0,0,80,65,24,0,0,0,24,0,0,0,0,0,160,64,0,0,80,65,0,0,0,63,0,0,0,63,0,0,0,0,0,42,0,0,0,160,64,0,0,0,64,0,0,224,192,0,0,0,160,64,0,0,248,65,0,0,224,64,0,0,192,64,24,0,0,0,24,0,0,0,0,0,224,64,0,0,192,64,0,0,0,63,0,0,0,0,0,0,0,0,0,43,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,138,66,0,0,184,65,0,0,224,64,0,0,224,64,24,0,0,0,24,0,0,0,0,0,224,64,0,0,224,64,0,0,0,63,0,0,0,63,0,0,0,0,0,44,0,0,0,64,64,0,0,128,63,0,0,0,0,0,0,0,0,0,0,0,240,65,0,0,64,64,0,0,160,64,24,0,0,0,24,0,0,0,0,0,64,64,0,0,160,64,0,0,0,63,0,0,0,63,0,0,0,0,0,45,0,0,0,128,64,0,0,128,63,0,0,128,192,0,0,0,0,0,0,0,108,66,0,0,160,64,0,0,64,64,24,0,0,0,24,0,0,0,0,0,160,64,0,0,64,64,0,0,0,63,0,0,0,63,0,0,0,0,0,46,0,0,0,64,64,0,0,128,63,0,0,128,191,0,0,0,160,64,0,0,108,66,0,0,64,64,0,0,64,64,24,0,0,0,24,0,0,0,0,0,64,64,0,0,64,64,0,0,0,63,0,0,0,63,0,0,0,0,0,47,0,0,0,64,64,0,0,128,63,0,0,160,192,0,0,0,124,66,0,0,16,66,0,0,160,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,160,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,48,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,160,64,0,0,0,0,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,49,0,0,0,224,64,0,0,0,64,0,0,160,192,0,0,0,240,66,0,0,48,65,0,0,160,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,160,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,50,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,138,66,0,0,0,0,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,51,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,160,64,0,0,160,65,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,52,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,226,66,0,0,48,65,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,53,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,168,65,0,0,68,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,54,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,168,65,0,0,48,65,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,55,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,160,64,0,0,20,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,56,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,160,64,0,0,64,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,57,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,188,66,0,0,12,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,58,0,0,0,64,64,0,0,128,63,0,0,128,192,0,0,0,80,66,0,0,4,66,0,0,64,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,64,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,59,0,0,0,64,64,0,0,128,63,0,0,64,192,0,0,0,80,66,0,0,76,66,0,0,64,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,64,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,60,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,52,66,0,0,48,66,0,0,224,64,0,0,224,64,24,0,0,0,24,0,0,0,0,0,224,64,0,0,224,64,0,0,0,63,0,0,0,63,0,0,0,0,0,61,0,0,0,224,64,0,0,64,64,0,0,128,192,0,0,0,16,66,0,0,96,66,0,0,0,65,0,0,192,64,24,0,0,0,24,0,0,0,0,0,0,65,0,0,192,64,0,0,0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,152,66,0,0,92,66,0,0,224,64,0,0,224,64,24,0,0,0,24,0,0,0,0,0,224,64,0,0,224,64,0,0,0,63,0,0,0,63,0,0,0,0,0,63,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,96,66,0,0,60,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,64,0,0,0,64,65,0,0,192,64,0,0,64,192,0,0,0,96,66,0,0,0,0,0,0,80,65,0,0,96,65,24,0,0,0,24,0,0,0,0,0,80,65,0,0,96,65,0,0,0,63,0,0,0,0,0,0,0,0,0,65,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,52,66,0,0,0,0,0,0,48,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,48,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,66,0,0,0,0,65,0,0,128,64,0,0,160,192,0,0,0,170,66,0,0,48,66,0,0,0,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,0,65,0,0,48,65,0,0,0,0,0,0,0,63,0,0,0,0,0,67,0,0,0,16,65,0,0,128,64,0,0,160,192,0,0,0,170,66,0,0,48,65,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,68,0,0,0,16,65,0,0,128,64,0,0,160,192,0,0,0,16,66,0,0,0,0,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,69,0,0,0,0,65,0,0,128,64,0,0,160,192,0,0,0,16,66,0,0,52,66,0,0,0,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,0,65,0,0,48,65,0,0,0,0,0,0,0,63,0,0,0,0,0,70,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,188,66,0,0,56,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,71,0,0,0,16,65,0,0,128,64,0,0,160,192,0,0,0,16,66,0,0,176,65,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,72,0,0,0,16,65,0,0,128,64,0,0,160,192,0,0,0,64,65,0,0,48,65,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,73,0,0,0,64,64,0,0,128,63,0,0,160,192,0,0,0,0,0,0,0,64,66,0,0,64,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,64,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,74,0,0,0,192,64,0,0,0,64,0,0,160,192,0,0,0,96,66,0,0,96,65,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,75,0,0,0,0,65,0,0,128,64,0,0,160,192,0,0,0,16,66,0,0,48,65,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,76,0,0,0,224,64,0,0,128,64,0,0,160,192,0,0,0,64,65,0,0,176,65,0,0,0,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,0,65,0,0,48,65,0,0,0,0,0,0,0,63,0,0,0,0,0,77,0,0,0,16,65,0,0,128,64,0,0,160,192,0,0,0,170,66,0,0,176,65,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,78,0,0,0,16,65,0,0,128,64,0,0,160,192,0,0,0,152,66,0,0,48,65,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,79,0,0,0,16,65,0,0,128,64,0,0,160,192,0,0,0,152,66,0,0,48,66,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,80,0,0,0,0,65,0,0,128,64,0,0,160,192,0,0,0,188,66,0,0,48,65,0,0,0,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,0,65,0,0,48,65,0,0,0,0,0,0,0,63,0,0,0,0,0,81,0,0,0,16,65,0,0,128,64,0,0,128,192,0,0,0,16,66,0,0,4,66,0,0,16,65,0,0,64,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,64,65,0,0,0,63,0,0,0,0,0,0,0,0,0,82,0,0,0,16,65,0,0,160,64,0,0,160,192,0,0,0,52,66,0,0,176,65,0,0,32,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,32,65,0,0,48,65,0,0,0,0,0,0,0,63,0,0,0,0,0,83,0,0,0,0,65,0,0,128,64,0,0,160,192,0,0,0,224,65,0,0,0,0,0,0,0,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,0,65,0,0,48,65,0,0,0,0,0,0,0,63,0,0,0,0,0,84,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,208,66,0,0,176,65,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,85,0,0,0,16,65,0,0,128,64,0,0,160,192,0,0,0,64,65,0,0,48,66,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,86,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,216,66,0,0,0,0,0,0,48,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,48,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,87,0,0,0,48,65,0,0,160,64,0,0,160,192,0,0,0,96,66,0,0,200,65,0,0,80,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,80,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,88,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,152,66,0,0,4,66,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,89,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,170,66,0,0,4,66,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,90,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,64,65,0,0,4,66,0,0,16,65,0,0,48,65,24,0,0,0,24,0,0,0,0,0,16,65,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,91,0,0,0,64,64,0,0,0,64,0,0,128,192,0,0,0,0,0,0,0,12,66,0,0,128,64,0,0,80,65,24,0,0,0,24,0,0,0,0,0,128,64,0,0,80,65,0,0,0,0,0,0,0,63,0,0,0,0,0,93,0,0,0,64,64,0,0,128,63,0,0,128,192,0,0,0,224,65,0,0,176,65,0,0,128,64,0,0,80,65,24,0,0,0,24,0,0,0,0,0,128,64,0,0,80,65,0,0,0,0,0,0,0,63,0,0,0,0,0,94,0,0,0,160,64,0,0,0,64,0,0,224,192,0,0,0,174,66,0,0,0,0,0,0,224,64,0,0,224,64,24,0,0,0,24,0,0,0,0,0,224,64,0,0,224,64,0,0,0,63,0,0,0,63,0,0,0,0,0,95,0,0,0,224,64,0,0,64,64,0,0,128,63,0,0,0,170,66,0,0,92,66,0,0,16,65,0,0,64,64,24,0,0,0,24,0,0,0,0,0,16,65,0,0,64,64,0,0,0,63,0,0,0,63,0,0,0,0,0,96,0,0,0,128,64,0,0,0,64,0,0,0,193,0,0,0,112,65,0,0,92,66,0,0,128,64,0,0,128,64,24,0,0,0,24,0,0,0,0,0,128,64,0,0,128,64,0,0,0,0,0,0,0,0,0,0,0,0,0,97,0,0,0,224,64,0,0,64,64,0,0,128,192,0,0,0,168,65,0,0,248,65,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,98,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,96,66,0,0,16,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,99,0,0,0,192,64,0,0,64,64,0,0,128,192,0,0,0,124,66,0,0,96,65,0,0,192,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,192,64,0,0,16,65,0,0,0,0,0,0,0,63,0,0,0,0,0,100,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,226,66,0,0,176,65,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,101,0,0,0,224,64,0,0,64,64,0,0,128,192,0,0,0,160,64,0,0,48,65,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,102,0,0,0,64,64,0,0,0,64,0,0,160,192,0,0,0,240,66,0,0,176,65,0,0,192,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,192,64,0,0,48,65,0,0,0,0,0,0,0,63,0,0,0,0,0,103,0,0,0,224,64,0,0,64,64,0,0,64,192,0,0,0,138,66,0,0,28,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,104,0,0,0,224,64,0,0,64,64,0,0,160,192,0,0,0,52,66,0,0,76,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,105,0,0,0,64,64,0,0,128,63,0,0,160,192,0,0,0,224,65,0,0,48,65,0,0,64,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,64,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,106,0,0,0,64,64,0,0,0,0,0,0,128,192,0,0,0,198,66,0,0,176,65,0,0,160,64,0,0,80,65,24,0,0,0,24,0,0,0,0,0,160,64,0,0,80,65,0,0,0,63,0,0,0,63,0,0,0,0,0,107,0,0,0,192,64,0,0,64,64,0,0,160,192,0,0,0,168,65,0,0,0,0,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,108,0,0,0,64,64,0,0,128,63,0,0,160,192,0,0,0,248,65,0,0,48,65,0,0,64,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,64,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,109,0,0,0,48,65,0,0,160,64,0,0,128,192,0,0,0,204,66,0,0,48,65,0,0,48,65,0,0,16,65,24,0,0,0,24,0,0,0,0,0,48,65,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,110,0,0,0,224,64,0,0,64,64,0,0,128,192,0,0,0,168,65,0,0,176,65,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,111,0,0,0,224,64,0,0,64,64,0,0,128,192,0,0,0,138,66,0,0,240,65,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,112,0,0,0,224,64,0,0,64,64,0,0,64,192,0,0,0,224,65,0,0,76,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,113,0,0,0,224,64,0,0,64,64,0,0,64,192,0,0,0,52,66,0,0,4,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,114,0,0,0,128,64,0,0,0,64,0,0,128,192,0,0,0,124,66,0,0,60,66,0,0,160,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,160,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,115,0,0,0,224,64,0,0,64,64,0,0,128,192,0,0,0,202,66,0,0,0,0,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,116,0,0,0,64,64,0,0,128,63,0,0,160,192,0,0,0,0,0,0,0,0,0,0,0,160,64,0,0,64,65,24,0,0,0,24,0,0,0,0,0,160,64,0,0,64,65,0,0,0,63,0,0,0,0,0,0,0,0,0,117,0,0,0,224,64,0,0,64,64,0,0,128,192,0,0,0,138,66,0,0,72,66,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,118,0,0,0,160,64,0,0,0,64,0,0,128,192,0,0,0,238,66,0,0,0,0,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,119,0,0,0,16,65,0,0,128,64,0,0,128,192,0,0,0,152,66,0,0,0,0,0,0,48,65,0,0,16,65,24,0,0,0,24,0,0,0,0,0,48,65,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,120,0,0,0,160,64,0,0,0,64,0,0,128,192,0,0,0,168,65,0,0,32,66,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,121,0,0,0,160,64,0,0,0,64,0,0,64,192,0,0,0,224,65,0,0,12,66,0,0,224,64,0,0,48,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,48,65,0,0,0,63,0,0,0,63,0,0,0,0,0,122,0,0,0,160,64,0,0,0,64,0,0,128,192,0,0,0,188,66,0,0,0,0,0,0,224,64,0,0,16,65,24,0,0,0,24,0,0,0,0,0,224,64,0,0,16,65,0,0,0,63,0,0,0,63,0,0,0,0,0,0,0};
		Array<int> aFontTex;
		Image aImage;
		Graphics_Core::LoadImageMem_PNG(aWorkFontPNG,5647,(unsigned char**)&aImage.mData,&aImage.mWidth,&aImage.mHeight);
		aFontTex[0]=gG.LoadTexture(aImage);

		IOBuffer aSprData;aSprData.SetMemoryPointerSource((void*)&aWorkFontBUNDLE,5573);

		Sprite aTemp;aTemp.BundleLoad(0,aFontTex,aSprData); // Get past the FILLRECT sprite
		aTempFont->BundleLoad(0,aFontTex,aSprData); // Load the font...
#endif
#endif
	}
	return aTempFont;
}


Line3D Graphics::ScreenPosToRay(int x, int y)
{
	//if (mNeedDrawInfo) GetDrawInfo();

	//*
	//theY=gG.HeightF()-theY;

	//
	// This code adapted from here:
	// http://www.gamedev.net/topic/547342-c-directx9-mouse-position-to-ray/
	//

	Matrix aProjection=mLast3DProjectionMatrix;
	//aProjection.Invert();		// Unnecessary?  It seems to work the same with this in or out.
	//aProjection.Transpose();	// Transposing seems to be incorrect because it forces the origin to 0,0,0 always... which would correspond to the fourth column of the matrix.

	Vector aV;
	aV.mX=(((2.0f*x)/mLast3DViewport.mWidth)-1)/aProjection.mData._11;
	aV.mY=-(((2.0f*y)/mLast3DViewport.mHeight)-1)/aProjection.mData._22;
	aV.mZ=1.0f;

	aV.mX*=-aProjection.mData._43;
	aV.mY*=-aProjection.mData._43;
	//aV.mX*=.99f;
	//aV.mY*=.99f;

	//
	// Note: Camera does NOT use WorldMatrix... it would need to be applied for individual rotated objects,
	// but not for the raw camera itself.  I.E. it doesn't belong in the core function, it would be applied
	// outside.  All we want here is the raw vector in camera space.
	//
	Matrix aView=mLast3DViewMatrix;
	aView.Invert();

	Vector aRayOrigin;
	Vector aRayDir;
	aRayDir.mX  = aV.mX*aView.mData._11 + aV.mY*aView.mData._21 + aV.mZ*aView.mData._31;
	aRayDir.mY  = aV.mX*aView.mData._12 + aV.mY*aView.mData._22 + aV.mZ*aView.mData._32;
	aRayDir.mZ  = aV.mX*aView.mData._13 + aV.mY*aView.mData._23 + aV.mZ*aView.mData._33;
	aRayOrigin.mX = aView.mData._41;
	aRayOrigin.mY = aView.mData._42;
	aRayOrigin.mZ = aView.mData._43;

	//
	// Raydir needs some transformation to viewport??
	//

	Line3D aLine;
	aLine.mPos[0]=aRayOrigin;
	aLine.mPos[1]=aRayOrigin+(aRayDir*Graphics_Core::GetZDepth());
	return aLine;
	/**/
}


void Graphics::SetAlphaThreshold(float theThreshold)
{
	if (theThreshold!=mAlphaThreshold)
	{
		if (mCurrentShader) mCurrentShader->mRehup|=DRAWINFO_ALPHATHRESHOLD;
		Graphics_Core::SetAlphaThreshold(theThreshold);
	}
}


bool Graphics::MustPow2Textures()
{
	return Graphics_Core::MustPow2Textures();
}

String Graphics::GetRGBOrder()
{
	return Graphics_Core::GetRGBOrder();
}

bool Graphics::CanDoWindowed()
{
	return Graphics_Core::CanDoWindowed();
}

bool Graphics::IsSmallScreen()
{
	return Graphics_Core::IsSmallScreen();
}

void Graphics::SetShader(Shader* theShader)
{
	if (theShader==NULL)
	{
		Flush();
		mCurrentShader=NULL;
		Graphics_Core::SetShader(-1);
	}
	else 
	{
		if (mCurrentShader!=theShader)
		{
			Flush();
			mCurrentShader=theShader;
			theShader->GoLive();
			Graphics_Core::SetShader(theShader->mShader);
		}
		else if(mCurrentShader->mRehup) theShader->GoLive();
	}
}

Vector Shader::mLightDir;
void Shader::Load(String theRShader)
{
	mVertexFormat.Reset();

	if (gG.mNoShaders) return;

	String aVShader="#RLSL";
	String aPShader="#RLSL";

	String aScratch;
	String aGlobal;
	String aInput;
	String aVertex;
	String aVSMain;
	String aPSMain;
	Array<String> aMoreFunctions;

	String aGet[2];
	bool aMore=theRShader.GetCodeBlock(aGet[0],aGet[1]);
	while (aMore)
	{
		if (aGet[0]=="global") aGlobal=aGet[1];
#ifdef _DEBUG
		else if (aGet[0]=="info") mName=aGet[1];
#else
		else if (aGet[0]=="info") {}
#endif

		else if (aGet[0]=="input") aInput=aGet[1];
		else if (aGet[0]=="vertex") aVertex=aGet[1];
		else if (aGet[0].StartsWith("vsmain")) aVSMain=aGet[1];
		else if (aGet[0].StartsWith("psmain")) aPSMain=aGet[1];
		//else if (aGet[0]=="scratch") aInput=aGet[1];
		else 
		{
			String aMore=aGet[0];aMore+="{";aMore+=aGet[1];aMore+="}";
			aMoreFunctions+=aMore;
		}
		aMore=theRShader.GetNextCodeBlock(aGet[0],aGet[1]);
	}

	aVShader+="global {";aVShader+=aGlobal;aVShader+="}";aVShader.AppendCRLF();
	//if (aScratch.Len()) {aVShader+="scratch {";aVShader+=aScratch;aVShader+="}";aVShader.AppendCRLF();}
	if (aInput.Len()) {aVShader+="input {";aVShader+=aInput;aVShader+="}";aVShader.AppendCRLF();}
	aVShader+="vertex {";aVShader+=aVertex;aVShader+="}";aVShader.AppendCRLF();
	aVShader+="main() {";aVShader+=aVSMain;aVShader+="}";aVShader.AppendCRLF();
	for (int aCount=0;aCount<aMoreFunctions.Size();aCount++) {aVShader+=aMoreFunctions[aCount];aVShader.AppendCRLF();}

	aPShader+="global {";aPShader+=aGlobal;aPShader+="}";aPShader.AppendCRLF();
	//if (aScratch.Len()) {aPShader+="scratch {";aPShader+=aScratch;aPShader+="}";aPShader.AppendCRLF();}
	aPShader+="vertex {";aPShader+=aVertex;aPShader+="}";aPShader.AppendCRLF();
	aPShader+="main() {";aPShader+=aPSMain;aPShader+="}";aPShader.AppendCRLF();
	for (int aCount=0;aCount<aMoreFunctions.Size();aCount++) {aPShader+=aMoreFunctions[aCount];aPShader.AppendCRLF();}

	Load(aVShader,aPShader);
}

void Shader::Load(String theVShader, String thePShader)
{
	String aStyle=Graphics_Core::GetShaderStyle();
	if (theVShader.StartsWith("#RLSL"))
	{
		theVShader.Delete(0,5);
		mCurrentShaderType=0;
		if (aStyle=="HLSL") theVShader=MakeHLSL(theVShader);
		else if (aStyle=="GLSL") theVShader=MakeGLSL(theVShader);
	}
	if (thePShader.StartsWith("#RLSL"))
	{
		thePShader.Delete(0,5);
		mCurrentShaderType=1;
		if (aStyle=="HLSL") thePShader=MakeHLSL(thePShader);
		else if (aStyle=="GLSL") thePShader=MakeGLSL(thePShader);
	}

#ifdef _DEBUG
	mVShaderString=theVShader;
	mPShaderString=thePShader;
#endif

	//ThreadLock(true);
	mShader=Graphics_Core::CreateShader(theVShader.c(),thePShader.c(),mVertexFormat);
	if (mShader>=0)
	{
		for (int aCount=0;aCount<mResolveList.Size();aCount++)
		{
			if (mResolveList[aCount].mPtr)
			{
				int aRef=Graphics_Core::GetShaderGlobalRef(mShader,0,mResolveList[aCount].mName.c());
				*mResolveList[aCount].mPtr=aRef;
				/*
				#ifdef _DEBUG
				if (aRef==-1)
				{
					gOut.Out("! ERROR: Could not resolve \"%s\" in shader",mResolveList[aCount].mName.c());
				}
				#endif
				*/
			}
		}
	}
	mLoaded=true;
	//ThreadLock(false);

#ifdef _DEBUG
	if (mShader==-1)
	{
		static int aBadShader=0;
		char* aError=(char*)Graphics_Core::Query("feedback");

		IOBuffer aBuffer;
		aBuffer.WriteLine(aError);
		if (mName.Len()==0) mName=Sprintf("badshader-%d",aBadShader++);
		String mName="badshader";
		mName+=".txt";
		aBuffer.CommitFile(Sprintf("desktop://%s",mName.c()));
		gOut.Out("! ERROR - Bad Shader Compile, aborting!");
		exit(0);

		/*
		static int aBadShader=0;
		IOBuffer aBuffer;
		aBuffer.WriteLine("--- VERTEX SHADER ------------------");
		{
			IOBuffer aBreakBuffer;
			aBreakBuffer.WriteLine(mVShaderString);
			aBreakBuffer.SeekBeginning();
			int aLineCount=1;
			while (!aBreakBuffer.IsEnd())
			{
				String aLine=Sprintf("%.3d\t%s",aLineCount,aBreakBuffer.ReadLine().c());
				aLineCount++;
				aBuffer.WriteLine(aLine);
			}
		}

		//aBuffer.WriteLine(mVShaderString);
		aBuffer.WriteLine("");
		aBuffer.WriteLine("--- PIXEL SHADER ------------------");
		{
			IOBuffer aBreakBuffer;
			aBreakBuffer.WriteLine(mPShaderString);
			aBreakBuffer.SeekBeginning();
			int aLineCount=1;
			while (!aBreakBuffer.IsEnd())
			{
				String aLine=Sprintf("%.3d\t%s",aLineCount,aBreakBuffer.ReadLine().c());
				aLineCount++;
				aBuffer.WriteLine(aLine);
			}
		}


		//aBuffer.WriteLine(mPShaderString);
		aBuffer.CommitFile(PointAtDesktop(Sprintf("badshader-%d.txt",aBadShader++)));
		exit(0);
		*/
	}
#endif
}

void Shader::Rehup(int theShaderType)
{
	Graphics_Core::SetShader(this->mShader);
	unsigned int aFlags=mDrawInfo[theShaderType].mFlags&mRehup;
	if (aFlags&DRAWINFO_WORLDMATRIX) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mWorldMatrix,gG.GetWorldMatrix().FloatPtr());}
	if (aFlags&DRAWINFO_WORLDMATRIXN) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mWorldMatrixN,gG.GetWorldMatrixN().FloatPtr());}
	if (aFlags&DRAWINFO_VIEWMATRIX) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mViewMatrix,gG.GetViewMatrix().FloatPtr());}
	if (aFlags&DRAWINFO_VIEWMATRIXN) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mViewMatrix,gG.GetViewMatrixN().FloatPtr());}
	if (aFlags&DRAWINFO_PROJECTIONMATRIX) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mProjectionMatrix,gG.GetProjectionMatrix().FloatPtr());}
	if (aFlags&DRAWINFO_PROJECTIONMATRIXN) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mProjectionMatrix,gG.GetProjectionMatrixN().FloatPtr());}
	if (aFlags&DRAWINFO_COMBOMATRIX) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mComboMatrix,gG.GetDrawMatrix().FloatPtr());}
	if (aFlags&DRAWINFO_BILLBOARDMATRIX) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mBillboardMatrix,gG.GetBillboardMatrix().FloatPtr());}
	if (aFlags&DRAWINFO_WORLDVIEWMATRIX) {Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mWorldViewMatrix,gG.GetWorldViewMatrix().FloatPtr());}
	if (aFlags&DRAWINFO_COLOR) {Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mColor,(float*)&gG.mDrawColor,4);}
	if (aFlags&DRAWINFO_ZBIAS) {Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mZBias,(float*)&gG.mZBias,1);}
	if (aFlags&DRAWINFO_SHADERDATA) {Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mShaderData[0],(float*)&gG.mShaderData[0],4);}
	if (aFlags&DRAWINFO_SHADERDATA2) {Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mShaderData[0],(float*)&gG.mShaderData[1],4);}
	if (aFlags&DRAWINFO_SHADERDATA3) {Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mShaderData[0],(float*)&gG.mShaderData[2],4);}
	if (aFlags&DRAWINFO_LOOKAT) {Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mLookAt,(float*)&gG.mLookAt,3);}
	if (aFlags&DRAWINFO_ALPHATHRESHOLD) {Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mAlphaThreshold,(float*)&gG.mAlphaThreshold,1);}
	if (aFlags&DRAWINFO_FLAGS) {Graphics_Core::SetShaderGlobal_Int(mDrawInfo[theShaderType].mGFlags,(int*)&gG.mFlags,1);}
	if (aFlags&DRAWINFO_SATURATION) {Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mSaturation,(float*)&gG.mSaturation,1);}
	// NEWSVAR NewShaderVariable New Shader Variable


	// Time and Rez are special, we ALWAYS rheup it if the flags have it
	// (Improvement?  I don't ALWAYS need to set it... but it will need to be set unless the shader was already set...)
	if (mDrawInfo[theShaderType].mFlags&DRAWINFO_TIME) {float aTime=(float)gG.Time();Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mTime,(float*)&aTime,1);}
	if (mDrawInfo[theShaderType].mFlags&DRAWINFO_REZ) {if (gG.IsRenderTo()) {float aRez[4]={(float)gG.mRenderToDimensions.mX,(float)gG.mRenderToDimensions.mY,(float)(gG.mRenderToDimensions.mX/2),(float)(gG.mRenderToDimensions.mY/2)};Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mRez,aRez,4);} else {float aRez[4]={(float)gG.mWidth,(float)gG.mHeight,(float)(gG.mWidth/2),(float)(gG.mHeight/2)};Graphics_Core::SetShaderGlobal_Float(mDrawInfo[theShaderType].mRez,aRez,4);}}

	/*
		Matrix aWorld=gG.GetWorldMatrix();
		Matrix aView=gG.GetViewMatrix();
		Matrix aProj=gG.GetProjectionMatrix();

		Matrix aMat=aWorld;
		aMat*=aView;
		aMat*=aProj;
		Graphics_Core::SetShaderGlobal_Matrix(mDrawInfo[theShaderType].mComboMatrix,aMat.FloatPtr());
		/**/
	if (aFlags&DRAWINFO_TEXTURES) {for (int aCount=0;aCount<mTextureRef.Size();aCount++) {Graphics_Core::SetShaderGlobal_Int(mTextureRef[aCount],&aCount,1);}}
}

void Shader::Rehup()
{
	if (mDrawInfo[0].mFlags&mRehup) Rehup(0);
	if (mDrawInfo[1].mFlags&mRehup) Rehup(1);
	mRehup=0;
}

void Shader::GoLive()
{
	IsLoaded();
	mRehup=0xFFFFFFFF;
	Rehup();
	if (mSetShaderHook) mSetShaderHook(this);
}

char* Shader::TypeToVertexFormat(String theType)
{
	if (theType=="xyz") return "xyz";
	if (theType=="xy") return "xy";
	if (theType=="normal") return "normal";
	if (theType=="rgba") return "rgba";
	if (theType=="rgba0") return "rgba";
	if (theType=="color") return "color";
	if (theType=="uv") return "uv";
	if (theType=="uint") return "uint";
	if (theType=="float") return "float";
	if (theType=="int") return "int";

#ifdef _DEBUG
	gOut.Out("!   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	gOut.Out("!   !! Mangled Vertex Input Type");
	gOut.Out("!   !! You must add this type here and in gl30.h");
	gOut.Out("!   !! you want to use it (search for VX_UINT in gl30.h)");
	gOut.Out("!   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
#endif
	return "?";
}

void Shader::FixHLSLType(String& theType)
{
	if (theType=="matrix") theType="float4x4";
	else if (theType=="worldmatrix") theType="float4x4";
	else if (theType=="viewmatrix") theType="float4x4";
	else if (theType=="projectionmatrix") theType="float4x4";
	else if (theType=="worldmatrixn") theType="float4x4";
	else if (theType=="viewmatrixn") theType="float4x4";
	else if (theType=="projectionmatrixn") theType="float4x4";
	else if (theType=="combomatrix") theType="float4x4";
	else if (theType=="billboardmatrix") theType="float4x4";
	else if (theType=="worldviewmatrix") theType="float4x4";
	else if (theType=="xyz") theType="float4";
	else if (theType=="xy") theType="float2";
	else if (theType=="normal") theType="float4";
	else if (theType=="rgba") theType="float4";
	else if (theType=="rgba0") theType="float4";
	else if (theType=="color") theType="float4";
	else if (theType=="uv") theType="float2";
	else if (theType=="tangent") theType="float3";
	else if (theType=="binormal") theType="float3";
	else if (theType=="screenpos") theType="float2";
	else if (theType=="depth") theType="float";
	else if (theType=="z_out") theType="float";
	else if (theType=="vector") theType="float3";
	else if (theType=="point") theType="float2";
	else if (theType=="gG.color") theType="float4";
	else if (theType=="gG.zbias") theType="float";
	else if (theType=="gG.time") theType="float";
	else if (theType=="gG.alphathreshold") theType="float";
	else if (theType=="gG.shaderdata") theType="float4";
	else if (theType=="gG.shaderdata0") theType="float4";
	else if (theType=="gG.shaderdata1") theType="float4";
	else if (theType=="gG.shaderdata2") theType="float4";
	else if (theType=="gG.lookat") theType="float3";
	else if (theType=="gG.resolution") theType="float4";
	else if (theType=="gG.flags") theType="int";
	else if (theType=="gG.saturation") theType="float";
	// NEWSVAR NewShaderVariable New Shader Variable

}

void Shader::FixGLSLType(String& theType)
{
	if (theType=="matrix") theType="mat4";
	else if (theType=="worldmatrix") theType="mat4";
	else if (theType=="viewmatrix") theType="mat4";
	else if (theType=="projectionmatrix") theType="mat4";
	else if (theType=="worldmatrixn") theType="mat4";
	else if (theType=="viewmatrixn") theType="mat4";
	else if (theType=="projectionmatrixn") theType="mat4";
	else if (theType=="combomatrix") theType="mat4";
	else if (theType=="billboardmatrix") theType="mat4";
	else if (theType=="worldviewmatrix") theType="mat4";
	else if (theType=="xyz") theType="vec4";
	else if (theType=="xy") theType="vec2";
	else if (theType=="normal") theType="vec4";
	else if (theType=="rgba") theType="uint";
	else if (theType=="rgba0") theType="uint";
	else if (theType=="color") theType="vec4";
	else if (theType=="uv") theType="vec2";
	else if (theType=="tangent") theType="vec3";
	else if (theType=="binormal") theType="vec3";
	else if (theType=="screenpos") theType="vec2";
	else if (theType=="depth") theType="float";
	else if (theType=="z_out") theType="float";
	else if (theType=="sampler") theType="sampler2D";
	else if (theType=="float2") theType="vec2";
	else if (theType=="float3") theType="vec3";
	else if (theType=="float4") theType="vec4";
	else if (theType=="vector") theType="vec3";
	else if (theType=="point") theType="vec2";
	else if (theType=="gG.color") theType="vec4";
	else if (theType=="gG.zbias") theType="float";
	else if (theType=="gG.time") theType="float";
	else if (theType=="gG.shaderdata") theType="vec4";
	else if (theType=="gG.shaderdata0") theType="vec4";
	else if (theType=="gG.shaderdata1") theType="vec4";
	else if (theType=="gG.shaderdata2") theType="vec4";
	else if (theType=="gG.alphathreshold") theType="float";
	else if (theType=="gG.lookat") theType="vec3";
	else if (theType=="gG.resolution") theType="vec4";
	else if (theType=="gG.flags") theType="int";
	else if (theType=="gG.saturation") theType="float";
	// NEWSVAR NewShaderVariable New Shader Variable
}

String Shader::GetHLSLTypeSuffix(String theType)
{
	if (theType=="xyz") return ": POSITION";
	else if (theType=="rgba") return Sprintf(": COLOR%d",mColorCounter++);
	else if (theType=="rgba0") return Sprintf(": COLOR%d",mColorCounter++);
	else if (theType=="color") return Sprintf(": COLOR%d",mColorCounter++);
	else if (theType=="uv") return Sprintf(": TEXCOORD%d",mTextureCounter++);
	else if (theType=="normal") return ": NORMAL";
	else if (theType=="tangent") return ": TANGENT";
	else if (theType=="binormal") return ": BINORMAL";
	else if (theType=="screenpos") return ": VPOS";
	else if (theType=="depth") return Sprintf(": DEPTH%d",mDepthCounter++);
	else if (theType=="z_out") return Sprintf(": DEPTH%d",mDepthCounter++);
	return "";
}

int Shader::GetHLSLTypeSize(String& theType)
{
	if (theType=="float4x4") return 4;
	if (theType=="sampler") return 0;
	return 1;
}

//#ifdef _DEBUG
#define ADDCRLF aResult+="\n";
//#else
//#define ADDCRLF
//#endif
String Shader::MakeHLSL(String theStr)
{
	int aTime=gAppPtr->Tick();
	String aResult;
	Array<String> aGlobals;
	Array<String> aInputs;
	//Array<String> aOutputs;
	Array<String> aVertex;
	Array<String> aMain;
	Array<String> aMoreFunctionsName;
	Array<String> aMoreFunctionsBody;

	String aGet[2];
	bool aMore=theStr.GetCodeBlock(aGet[0],aGet[1]);
	while (aMore)
	{
		if (aGet[0]=="global") aGet[1].TokenizeCode(';',aGlobals);
		else if (aGet[0]=="input") aGet[1].TokenizeCode(';',aInputs);
		//else if (aGet[0]=="output") aGet[1].TokenizeCode(';',aOutputs);
		else if (aGet[0]=="vertex") aGet[1].TokenizeCode(';',aVertex);
		else if (aGet[0].StartsWith("main")) aGet[1].TokenizeCode(';',aMain);
		else 
		{
			aMoreFunctionsName+=aGet[0];
			aMoreFunctionsBody+=aGet[1];
		}
		aMore=theStr.GetNextCodeBlock(aGet[0],aGet[1]);
	}

	/*
	theStr.GetCodeBlockByName("global").TokenizeCode(';',aGlobals);
	theStr.GetCodeBlockByName("input").TokenizeCode(';',aInputs);
	theStr.GetCodeBlockByName("output").TokenizeCode(';',aOutputs);
	theStr.GetCodeBlockByName("main").TokenizeCode(';',aMain);
	if (aMain.Size()==0) theStr.GetCodeBlockByName("main()").TokenizeCode(';',aMain);
	*/

	//
	// Start by converting globals...
	//
	int aRegister=0;
	for (int aCount=0;aCount<aGlobals.Size();aCount++)
	{
		String aType=aGlobals[aCount].GetSegmentBefore(' ');
		String aVar=aGlobals[aCount].GetSegmentAfter(' ');

		String aPreType=aType;

		int aRegisterSize=0;
		FixHLSLType(aType);

		//
		// Get the sizes...
		//
		aRegisterSize=GetHLSLTypeSize(aType);
		if (aVar.Contains('=')) 
		{
			aType.Insert("static const ",0);
			aRegisterSize=0;
		}

		//
		// Build the string
		//
		aResult+=aType;aResult+=" ";aResult+=aVar;
		if (aRegisterSize>0) {aResult+=" : register(c";aResult+=aRegister;aResult+=")";}
		aResult+=";";ADDCRLF;

		AddShaderVariable(aVar,aRegister);

		if (aPreType=="worldmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_WORLDMATRIX;mDrawInfo[mCurrentShaderType].mWorldMatrix=aRegister;}
		if (aPreType=="worldmatrixn") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_WORLDMATRIXN;mDrawInfo[mCurrentShaderType].mWorldMatrixN=aRegister;}
		if (aPreType=="viewmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_VIEWMATRIX;mDrawInfo[mCurrentShaderType].mViewMatrix=aRegister;}
		if (aPreType=="viewmatrixn") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_VIEWMATRIXN;mDrawInfo[mCurrentShaderType].mViewMatrixN=aRegister;}
		if (aPreType=="projectionmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_PROJECTIONMATRIX;mDrawInfo[mCurrentShaderType].mProjectionMatrix=aRegister;}
		if (aPreType=="projectionmatrixn") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_PROJECTIONMATRIXN;mDrawInfo[mCurrentShaderType].mProjectionMatrixN=aRegister;}
		if (aPreType=="combomatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_COMBOMATRIX;mDrawInfo[mCurrentShaderType].mComboMatrix=aRegister;}
		if (aPreType=="billboardmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_BILLBOARDMATRIX;mDrawInfo[mCurrentShaderType].mBillboardMatrix=aRegister;}
		if (aPreType=="worldviewmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_WORLDVIEWMATRIX;mDrawInfo[mCurrentShaderType].mWorldViewMatrix=aRegister;}
		if (aPreType=="gG.color") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_COLOR;mDrawInfo[mCurrentShaderType].mColor=aRegister;}
		if (aPreType=="gG.zbias") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_ZBIAS;mDrawInfo[mCurrentShaderType].mZBias=aRegister;}
		if (aPreType=="gG.time") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_TIME;mDrawInfo[mCurrentShaderType].mTime=aRegister;}
		if (aPreType=="gG.shaderdata") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SHADERDATA;mDrawInfo[mCurrentShaderType].mShaderData[0]=aRegister;}
		if (aPreType=="gG.shaderdata0") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SHADERDATA;mDrawInfo[mCurrentShaderType].mShaderData[0]=aRegister;}
		if (aPreType=="gG.shaderdata1") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SHADERDATA2;mDrawInfo[mCurrentShaderType].mShaderData[1]=aRegister;}
		if (aPreType=="gG.shaderdata2") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SHADERDATA3;mDrawInfo[mCurrentShaderType].mShaderData[2]=aRegister;}
		if (aPreType=="gG.lookat") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_LOOKAT;mDrawInfo[mCurrentShaderType].mLookAt=aRegister;}
		if (aPreType=="gG.alphathreshold") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_ALPHATHRESHOLD;mDrawInfo[mCurrentShaderType].mAlphaThreshold=aRegister;}
		if (aPreType=="gG.resolution") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_REZ;mDrawInfo[mCurrentShaderType].mRez=aRegister;}
		if (aPreType=="gG.flags") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_FLAGS;mDrawInfo[mCurrentShaderType].mGFlags=aRegister;}
		if (aPreType=="gG.saturation") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SATURATION;mDrawInfo[mCurrentShaderType].mSaturation=aRegister;}
		// NEWSVAR NewShaderVariable New Shader Variable

		aRegister+=aRegisterSize;
	}

	//
	// Now the output info...
	//
	mColorCounter=mTextureCounter=mDepthCounter=0;
	/*
	if (aOutputs.Size()>1) {aResult+="struct _OUTPUT";ADDCRLF;aResult+="{";ADDCRLF;}
	String aFirstType;
	String aFirstVar;
	String aFirstSuffix;
	for (int aCount=0;aCount<aOutputs.Size();aCount++)
	{
		String aType=aOutputs[aCount].GetSegmentBefore(' ');
		String aVar=aOutputs[aCount].GetSegmentAfter(' ');

		String aSuffix=GetHLSLTypeSuffix(aType);
		FixHLSLType(aType);

		if (aCount==0) {aFirstType=aType;aFirstVar=aVar;aFirstSuffix=aSuffix;}
		if (aOutputs.Size()==1) break;

		aResult+=aType;aResult+=" ";
		aResult+=aVar;aResult+=" ";
		aResult+=aSuffix;aResult+=";";
		ADDCRLF;
	}
	if (aOutputs.Size()>1) {aResult+="};";ADDCRLF;}
	*/

	if (mCurrentShaderType==0)
	if (aVertex.Size())
	{
		aResult+="struct _VERTEX";ADDCRLF;
		aResult+="{";ADDCRLF;
		int aTexCoordCounter=0;
		for (int aCount=0;aCount<aVertex.Size();aCount++)
		{
			String aType=aVertex[aCount].GetSegmentBefore(' ');
			String aVar=aVertex[aCount].GetSegmentAfter(' ');

			String aSuffix=GetHLSLTypeSuffix(aType);
			if (aSuffix.StartsWith(": TEXCOORD")) 
			{
				aTexCoordCounter=_max(aTexCoordCounter,((aSuffix[10]-48)+1));
			}
			if (aSuffix.Len()==0) {aSuffix=" : TEXCOORD";aSuffix+=aTexCoordCounter++;}

			FixHLSLType(aType);

			//if (aType=="int") aResult+="nointerpolation "; // Needs DX10...
			aResult+=aType;aResult+=" ";
			aResult+=aVar;aResult+=" ";
			aResult+=aSuffix;aResult+=";";
			ADDCRLF;
		}
		aResult+="};";ADDCRLF;
	}


	//
	// Add in any other functions...
	//
	for (int aCount=0;aCount<aMoreFunctionsName.Size();aCount++)
	{
		FixHLSLCodeBlock(aMoreFunctionsName[aCount],(mCurrentShaderType!=0));
		aResult+=aMoreFunctionsName[aCount];ADDCRLF;
		aResult+="{";ADDCRLF;
		Array<String> aCodeLines;aMoreFunctionsBody[aCount].TokenizeCode(';',aCodeLines);
		for (int aCount=0;aCount<aCodeLines.Size();aCount++) 
		{
			FixHLSLCodeBlock(aCodeLines[aCount],(mCurrentShaderType!=0));
			aResult+=aCodeLines[aCount];
			aResult+=";";ADDCRLF;
		}
		aResult+="}";ADDCRLF;
		//aResult+=aMoreFunctions[0];ADDCRLF;
	}


	//
	// Now the main...
	//
	mColorCounter=mTextureCounter=mDepthCounter=0;
	if (mCurrentShaderType==0) aResult+="_VERTEX ";
	else aResult+="float4 ";
	aResult+="main(";

	//
	// Inputs...
	//

	//Array<String> aRGBAInList;	// This is here because DirectX shaders have RGBA order like OpenGL shaders.  Because the rest of my system isn't using it,
								// I have to do a reverse of rgba ints for DX shaders ONLY.  NOTE: At some point if you move the whole works over to shaders,
								// you can remove this.  That will get rid of the troublesome BGRA/RGBA garbage too!
								// Okay, don't need it... was declaring my color in wrong...

	Array<String> aWork;
	if (mCurrentShaderType==0) aWork=aInputs;
	else aWork=aVertex;
	int aTexCoordCounter=0;
	for (int aCount=0;aCount<aWork.Size();aCount++)
	{
		if (aCount>0) aResult+=",";
		String aType=aWork[aCount].GetSegmentBefore(' ');
		String aVar=aWork[aCount].GetSegmentAfter(' ');
		String aSuffix=GetHLSLTypeSuffix(aType);

		if (mCurrentShaderType==0) mVertexFormat+=TypeToVertexFormat(aType);
		else
		{
			if (aSuffix.StartsWith(": TEXCOORD")) {aTexCoordCounter=_max(aTexCoordCounter,((aSuffix[10]-48)+1));}
			if (aSuffix.Len()==0) {aSuffix=": TEXCOORD";aSuffix+=aTexCoordCounter++;}
		}
		//if (mCurrentShaderType==0 && aType=="rgba") aRGBAInList+=aVar;
		FixHLSLType(aType);

		aResult+="in ";
		aResult+=aType;aResult+=" ";
		aResult+=aVar;aResult+=" ";
		aResult+=aSuffix;
	}

	aResult+=")";
	if (mCurrentShaderType==1) aResult+=": COLOR0";
	ADDCRLF;
	aResult+="{";ADDCRLF;
	bool aKillOutput=false;
	if (mCurrentShaderType==0)
	{
		aResult+="_VERTEX vertex;";ADDCRLF;
	}
	else
	{
		aResult+="float4 pixel;";ADDCRLF;
		aKillOutput=true;
	}
	//
	// Code...
	//
	/*
	for (int aCount=0;aCount<aRGBAInList.Size();aCount++)
	{
		String aStr=Sprintf("\tinput.%s.rgba=input.%s.bgra",aRGBAInList[aCount].c(),aRGBAInList[aCount].c());
		aMain.InsertElement(aStr,0);
	}
	*/


	for (int aCount=0;aCount<aMain.Size();aCount++)
	{
		FixHLSLCodeBlock(aMain[aCount],aKillOutput);
		aResult+=aMain[aCount];aResult+=";";ADDCRLF;
	}
	//
	// End of code
	//
	if (mCurrentShaderType==0) {aResult+="return vertex;";ADDCRLF;}
	else {aResult+="return pixel;";ADDCRLF;}

	aResult+="}";ADDCRLF;

#ifdef _DEBUG
	gOut.Out("$%s Shader Generate Time: %d",mCurrentShaderType?"Pixel":"Vertex",gAppPtr->Tick()-aTime);
#endif

	return aResult;
}

void Shader::FixHLSLCodeBlock(String& theStr, bool killOutput)
{
	if (killOutput) theStr.Replace("output.","output_");
	theStr.Replace("input.","");
	if (mCurrentShaderType==1) theStr.Replace("vertex.","");
	theStr.Replace("global.","");
	theStr.Replace("mix(","lerp(");
	theStr.Replace("discard","clip(-1)");
}

String Shader::MakeGLSL(String theStr)
{
	int aTime=gAppPtr->Tick();
	String aResult="#version 300 es\n";
	aResult+="precision mediump float;";ADDCRLF;

	Array<String> aGlobals;
	Array<String> aScratch;
	Array<String> aInputs;
	//Array<String> aOutputs;
	Array<String> aVertex;
	Array<String> aMain;

	Array<String> aMoreFunctionsName;
	Array<String> aMoreFunctionsBody;

	String aGet[2];
	String aPFormat;
	bool aMore=theStr.GetCodeBlock(aGet[0],aGet[1]);
	while (aMore)
	{
		if (aGet[0]=="global") aGet[1].TokenizeCode(';',aGlobals);
#ifdef _DEBUG
		else if (aGet[0]=="info") mName=aGet[1];
#else
		else if (aGet[0]=="info") {}
#endif
		else if (aGet[0]=="pixelformat") aPFormat=aGet[1];
		else if (aGet[0]=="input") aGet[1].TokenizeCode(';',aInputs);
		else if (aGet[0]=="vertex") aGet[1].TokenizeCode(';',aVertex);
		//else if (aGet[0]=="scratch") aGet[1].TokenizeCode(';',aScratch);
		//else if (aGet[0]=="output") aGet[1].TokenizeCode(';',aOutputs);
		else if (aGet[0].StartsWith("main")) aGet[1].TokenizeCode(';',aMain);
		else 
		{
			aMoreFunctionsName+=aGet[0];
			aMoreFunctionsBody+=aGet[1];
		}
		aMore=theStr.GetNextCodeBlock(aGet[0],aGet[1]);
	}

	Array<String> aSamplerList;
	for (int aCount=0;aCount<aGlobals.Size();aCount++)
	{
		String aType=aGlobals[aCount].GetSegmentBefore(' ');
		String aVar=aGlobals[aCount].GetSegmentAfter(' ');
		String aPreType=aType;
		FixGLSLType(aType);

		//
		// Convert c-style arrays to GLSL initializers...
		//
		if (aVar.Contains('{'))
		{
			String aInBrackets=aVar.GetBracedToken('[',']');
			String aInBraces=aVar.GetBracedToken('{','}');
			String aVarName=aVar.GetSegmentBefore('=');

			String aNew=aVarName;
			aNew+="=";
			aNew+=aType;
			aNew+="[";
			aNew+=aInBrackets;
			aNew+="](";
			aNew+=aInBraces;
			aNew+=")";
			aVar=aNew;
		}

		bool aIsUniform=false;
		if (aType.StartsWith("layout")) {}
		else if (!aVar.Contains('=')) {aResult+="uniform ";aIsUniform=true;}
		else aResult+="const ";
		aResult+=aType;
		aResult+=" ";
		aResult+=aVar;
		aResult+=";";ADDCRLF;

		Smart(ShaderVariable) aSV=AddShaderVariable(aVar,-1);
		//
		// Well shit... I need to resolve these "registers"...
		//
		if (aPreType=="worldmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_WORLDMATRIX;mDrawInfo[mCurrentShaderType].mWorldMatrix=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mWorldMatrix);}
		else if (aPreType=="worldmatrixn") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_WORLDMATRIXN;mDrawInfo[mCurrentShaderType].mWorldMatrixN=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mWorldMatrixN);}
		else if (aPreType=="viewmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_VIEWMATRIX;mDrawInfo[mCurrentShaderType].mViewMatrix=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mViewMatrix);}
		else if (aPreType=="viewmatrixn") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_VIEWMATRIXN;mDrawInfo[mCurrentShaderType].mViewMatrixN=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mViewMatrixN);}
		else if (aPreType=="projectionmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_PROJECTIONMATRIX;mDrawInfo[mCurrentShaderType].mProjectionMatrix=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mProjectionMatrix);}
		else if (aPreType=="projectionmatrixn") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_PROJECTIONMATRIXN;mDrawInfo[mCurrentShaderType].mProjectionMatrixN=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mProjectionMatrixN);}
		else if (aPreType=="combomatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_COMBOMATRIX;mDrawInfo[mCurrentShaderType].mComboMatrix=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mComboMatrix);}
		else if (aPreType=="billboardmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_BILLBOARDMATRIX;mDrawInfo[mCurrentShaderType].mBillboardMatrix=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mBillboardMatrix);}
		else if (aPreType=="worldviewmatrix") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_WORLDVIEWMATRIX;mDrawInfo[mCurrentShaderType].mWorldViewMatrix=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mWorldViewMatrix);}
		else if (aPreType=="gG.color") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_COLOR;mDrawInfo[mCurrentShaderType].mColor=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mColor);}
		else if (aPreType=="gG.zbias") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_ZBIAS;mDrawInfo[mCurrentShaderType].mZBias=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mZBias);}
		else if (aPreType=="gG.time") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_TIME;mDrawInfo[mCurrentShaderType].mTime=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mTime);}
		else if (aPreType=="gG.shaderdata") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SHADERDATA;mDrawInfo[mCurrentShaderType].mShaderData[0]=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mShaderData[0]);}
		else if (aPreType=="gG.shaderdata0") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SHADERDATA;mDrawInfo[mCurrentShaderType].mShaderData[0]=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mShaderData[0]);}
		else if (aPreType=="gG.shaderdata1") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SHADERDATA2;mDrawInfo[mCurrentShaderType].mShaderData[1]=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mShaderData[1]);}
		else if (aPreType=="gG.shaderdata2") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SHADERDATA3;mDrawInfo[mCurrentShaderType].mShaderData[2]=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mShaderData[2]);}
		else if (aPreType=="gG.lookat") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_LOOKAT;mDrawInfo[mCurrentShaderType].mLookAt=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mLookAt);}
		else if (aPreType=="gG.alphathreshold") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_ALPHATHRESHOLD;;mDrawInfo[mCurrentShaderType].mAlphaThreshold=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mAlphaThreshold);}
		else if (aPreType=="gG.resolution") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_REZ;mDrawInfo[mCurrentShaderType].mRez=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mRez);}
		else if (aPreType=="gG.flags") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_FLAGS;mDrawInfo[mCurrentShaderType].mGFlags=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mGFlags);}
		else if (aPreType=="gG.saturation") {mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_SATURATION;mDrawInfo[mCurrentShaderType].mSaturation=-1;mResolveList+=ResolveGlobal(aVar,&mDrawInfo[mCurrentShaderType].mSaturation);}
		// NEWSVAR NewShaderVariable New Shader Variable
		else 
		{
			if (aPreType!="sampler") {if (aIsUniform) mResolveList+=ResolveGlobal(aVar,&aSV->mRef);}
			else {aSamplerList+=aVar;mDrawInfo[mCurrentShaderType].mFlags|=DRAWINFO_TEXTURES;}
		}
	}

	for (int aCount=0;aCount<aScratch.Size();aCount++)
	{
		String aType=aGlobals[aCount].GetSegmentBefore(' ');
		String aVar=aGlobals[aCount].GetSegmentAfter(' ');
		String aPreType=aType;
		FixGLSLType(aType);

		//
		// Convert c-style arrays to GLSL initializers...
		//
		if (aVar.Contains('{'))
		{
			String aInBrackets=aVar.GetBracedToken('[',']');
			String aInBraces=aVar.GetBracedToken('{','}');
			String aVarName=aVar.GetSegmentBefore('=');

			String aNew=aVarName;
			aNew+="=";
			aNew+=aType;
			aNew+="[";
			aNew+=aInBrackets;
			aNew+="](";
			aNew+=aInBraces;
			aNew+=")";
			aVar=aNew;
		}

		bool aIsUniform=false;
		if (!aVar.Contains('=')) {aResult+="attribute ";}
		//else aResult+="const ";
		aResult+=aType;
		aResult+=" ";
		aResult+=aVar;
		aResult+=";";ADDCRLF;
	}


	//
	// Set up texture samplers to resolve...
	//
	mTextureRef.GuaranteeSize(aSamplerList.Size());
	for (int aCount=0;aCount<aSamplerList.Size();aCount++) mResolveList+=ResolveGlobal(aSamplerList[aCount],&mTextureRef[aCount]);

	Array<String> aRGBAInList;
	Array<String> aRGBA0InList;
	Array<String> aNormalInList;
	for (int aCount=0;aCount<aInputs.Size();aCount++)
	{
		String aType=aInputs[aCount].GetSegmentBefore(' ');
		String aVar=aInputs[aCount].GetSegmentAfter(' ');
		String aPreType=aType;

		if (mCurrentShaderType==0) 
		{
			String aForm=TypeToVertexFormat(aType);
			mVertexFormat+=TypeToVertexFormat(aType);
		}

		FixGLSLType(aType);

		aResult+="layout (location=";
		aResult+=aCount;
		aResult+=") in ";
		aResult+=aType;
		aResult+=" input_";
		if (aPreType=="rgba") {aResult+="rgba_";aRGBAInList+=aVar;}
		else if (aPreType=="rgba0") {aResult+="rgba_";aRGBA0InList+=aVar;}
		//if (aPreType=="normal") {aResult+="normal_";aNormalInList+=aVar;}
		aResult+=aVar;
		aResult+=";";ADDCRLF;
	}

	Array<int> aColorOutList;
	String aPositionVar;
	/*
	for (int aCount=0;aCount<aOutputs.Size();aCount++)
	{
		String aType=aOutputs[aCount].GetSegmentBefore(' ');
		String aVar=aOutputs[aCount].GetSegmentAfter(' ');
		String aPreType=aType;
		FixGLSLType(aType);

		aResult+="out ";
		aResult+=aType;
		aResult+=" output_";
		aResult+=aVar;
		aResult+=";";ADDCRLF;

		if (aPreType=="color") aColorOutList+=aCount;
		if (aPreType=="xyz" && aPositionVar.Len()==0) {aPositionVar="vxoutput_";aPositionVar+=aVar;}
	}
	*/
	String aOutIn;
	if (mCurrentShaderType==0) aOutIn="out ";
	else aOutIn="in ";
	for (int aCount=0;aCount<aVertex.Size();aCount++)
	{
		String aType=aVertex[aCount].GetSegmentBefore(' ');
		String aVar=aVertex[aCount].GetSegmentAfter(' ');
		String aPreType=aType;
		FixGLSLType(aType);

		String aLine;

		//aResult+="out ";
		if (aType=="int") aLine+="flat ";
		aLine+=aOutIn;
		aLine+=aType;
		aLine+=" vertex_";
		aLine+=aVar;
		aLine+=";";

		/*
		if (aLine.ContainsI("flatuv"))
		{
			aLine.Replace("flatuv","vec2");
			aLine.Insert("noperspective ");
		}
		*/

		if (aPreType=="color") aColorOutList+=aCount;
		if (mCurrentShaderType==0) if ((aPreType=="xyz" || aPreType=="xy") && aPositionVar.Len()==0) {aPositionVar="vertex_";aPositionVar+=aVar;}

		aResult+=aLine;
		ADDCRLF;
	}

	if (mCurrentShaderType==1)
	{
		if (aPFormat=="16") aResult+="out uvec4 pixel;";
		else aResult+="out vec4 pixel;";
		ADDCRLF;
	}

	/*
	"out vec4 FragColor;\n"
		"FragColor = outColor;\n"
	*/


	//
	// Add in any other functions...
	//
	for (int aCount=0;aCount<aMoreFunctionsName.Size();aCount++)
	{
		FixGLSLCodeBlock(aMoreFunctionsName[aCount]);
		aResult+=aMoreFunctionsName[aCount];ADDCRLF;
		aResult+="{";ADDCRLF;
		Array<String> aCodeLines;aMoreFunctionsBody[aCount].TokenizeCode(';',aCodeLines);
		for (int aCount=0;aCount<aCodeLines.Size();aCount++) 
		{
			FixGLSLCodeBlock(aCodeLines[aCount]);
			aResult+=aCodeLines[aCount];
			aResult+=";";ADDCRLF;
		}
		aResult+="}";ADDCRLF;
		//aResult+=aMoreFunctions[0];ADDCRLF;
	}

	//
	// Now Main...
	//
	aResult+="void main()";ADDCRLF;
	aResult+="{";ADDCRLF;
	//
	// One big deal here... if we are using an RGBA in for vertex shader, and a color in output, we need to convert it to four floats...
	//
	for (int aCount=0;aCount<aRGBAInList.Size();aCount++)
	{
		String aToken="input_rgba_";
		aToken+=aRGBAInList[aCount];

		aResult+="vec4 input_";
		aResult+=aRGBAInList[aCount];
		aResult+=Sprintf("=vec4(float(((%s)&0x000000FFu))/255.0f,float(((%s)&0x0000FF00u)>>8)/255.0f,float(((%s)&0x00FF0000u)>>16)/255.0f,float(((%s)&0xFF000000u)>>24)/255.0f);",aToken.c(),aToken.c(),aToken.c(),aToken.c());
		ADDCRLF;
	}

	for (int aCount=0;aCount<aRGBA0InList.Size();aCount++)
	{
		String aToken="input_rgba_";
		aToken+=aRGBA0InList[aCount];

		aResult+="vec4 input_";
		aResult+=aRGBA0InList[aCount];
		aResult+=";";ADDCRLF;

		aResult+="if (";
		aResult+=aToken;
		aResult+="==uint(0)) input_";
		aResult+=aRGBA0InList[aCount];
		aResult+="=vec4(0,0,0,0);";ADDCRLF;
		
		aResult+="else input_";
		aResult+=aRGBA0InList[aCount];

		aResult+=Sprintf("=vec4(float(((%s)&0x000000FFu))/255.0f,float(((%s)&0x0000FF00u)>>8)/255.0f,float(((%s)&0x00FF0000u)>>16)/255.0f,float(((%s)&0xFF000000u)>>24)/255.0f);",aToken.c(),aToken.c(),aToken.c(),aToken.c());
		ADDCRLF;
	}
	/*
	for (int aCount=0;aCount<aNormalInList.Size();aCount++)
	{
		aResult+="vec4 input_";
		aResult+=aNormalInList[aCount];
		String aToken="input_normal_";
		aToken+=aNormalInList[aCount];
		aResult+=Sprintf("=vec4(%s.x,%s.y,%s.z,1.0f);",aToken.c(),aToken.c(),aToken.c(),aToken.c());
		ADDCRLF;
	}
	*/

	//Array<String> aRGBAInList;

	int aColorConversion=0;
	for (int aCount=0;aCount<aMain.Size();aCount++)
	{
		FixGLSLCodeBlock(aMain[aCount]);
		/*
		int aFind=aMain[aCount].FindI("rgbatocolor");
		while (aFind>=0)
		{
			aMain[aCount].mCursor=aMain[aCount].mData+aFind;
			String aToken=aMain[aCount].GetNextBracedToken('(',')');
			aMain[aCount].Delete(aFind,11);
			//String aReplace=Sprintf("vec4(float(((%s)&0x00FF0000u)>>16)/255.0f,float(((%s)&0x0000FF00u)>>8)/255.0f,float(((%s)&0x000000FFu))/255.0f,float(((%s)&0xFF000000u)>>24)/255.0f)",aToken.c(),aToken.c(),aToken.c(),aToken.c());
			String aReplace=Sprintf("vec4(float(((%s)&0x000000FFu))/255.0f,float(((%s)&0x0000FF00u)>>8)/255.0f,float(((%s)&0x00FF0000u)>>16)/255.0f,float(((%s)&0xFF000000u)>>24)/255.0f)",aToken.c(),aToken.c(),aToken.c(),aToken.c());
			aMain[aCount].Delete(aFind+1,aToken.Len());
			aMain[aCount].Insert(aReplace,aFind+1);

			aFind=aMain[aCount].FindI("rgbatocolor",aFind+1);
			// Get inside parenthesis

			aColorConversion++;
		}
		*/

		aResult+=aMain[aCount];aResult+=";";ADDCRLF;
	}

	if (aPositionVar.Len())
	{
		aResult+="gl_Position=";
		aResult+=aPositionVar;
		aResult+=";";
		ADDCRLF;
	}
	aResult+="}";ADDCRLF;

	return aResult;
}

void Shader::FixGLSLCodeBlock(String& theStr)
{
	//
	// I need some kind of array'd replace in String for this...
	//
	theStr.Replace("output.","output_");
	theStr.Replace("input.","input_");
	theStr.Replace("vertex.","vertex_");
	theStr.Replace("global.","");
	theStr.Replace("pixel.zdepth","gl_FragCoord.z");	// Depth coordinate (for writing depth)
	theStr.Replace("pixel.pos2D","gl_FragCoord.xy");
	theStr.Replace("pixel.pos","gl_FragCoord.xyz");
	theStr.Replace("zdepth","gl_FragDepth");

	theStr.Replace("FLAG_RENDERTO",Sprintf("%d",Graphics::FLAG_RENDERTO));
	// NEWGFLAG
	//theStr.Replace("pixel.zbuffer","gl_FragCoord.w"); // For the actual number in the zbuffer (use this when trying to render it to color)

	//
	// HLSL to GLSL conversons...
	//
	theStr.Replace("tex2D(","texture(");
	theStr.Replace("lerp(","mix(");
	theStr.Replace("frac(","fract(");
	theStr.Replace("float4","vec4");
	theStr.Replace("float3","vec3");
	theStr.Replace("float2","vec2");
	theStr.Replace("matrix","mat4");

	//
	// What about "mul?"
	//
	int aFind=theStr.FindI("mul(");
	while (aFind>=0)
	{
		theStr.mCursor=theStr.mData+aFind;
		String aToken=theStr.GetNextBracedToken('(',')');
		String aFirst=aToken.CGetToken(',');
		String aSecond=aToken.CGetNextToken(',');
		theStr.Delete(aFind+4,aToken.Len());
		theStr.Insert(Sprintf("%s*%s",aFirst.c(),aSecond.c()),aFind+4);
		theStr.Delete(aFind,3);
		aFind=theStr.FindI("mul(",aFind+1);
	}
}

int Shader::GetGlobalRef(String theVariable)
{
	int aResult=Graphics_Core::GetShaderGlobalRef(mShader,0,theVariable.c());
	if (aResult==-1) 
	{
		aResult=Graphics_Core::GetShaderGlobalRef(mShader,1,theVariable.c());
	}
	if (aResult==-1)
	{
		EnumSmartList(ShaderVariable,aS,mVariableList)
		{
			if (aS->mName==theVariable)
			{
				aResult=aS->mRef;
				break;
			}
		}
	}
//#ifdef _DEBUG
	if (aResult==-1) gOut.Out("!GetGlobalRef(\"%s\") not found!",theVariable.c());
//#endif
	return aResult;
}

//void Shader::DoLoad() {gG.ThreadLock(true);Load();gG.ThreadLock(false);}
void Shader::DoLoad() {Load();}

void Shader::SetTexture(int theTexture) {gG.SetTexture(theTexture);}
void Shader::SetTexture(Sprite* theSprite) {gG.SetTexture(theSprite->mTexture);}
void Shader::SetGlobal(int theVariable, float* theValue, int theCount) {Graphics_Core::SetShaderGlobal_Float(theVariable,theValue,theCount);}
void Shader::SetGlobal(int theVariable, int* theValue, int theCount) {Graphics_Core::SetShaderGlobal_Int(theVariable,theValue,theCount);}
void Shader::SetGlobal(int theVariable, Matrix* theValue) {Graphics_Core::SetShaderGlobal_Matrix(theVariable,theValue->FloatPtr());}
void Shader::DrawShape(void* theVerts, int theVCount) {if (gG.mCurrentShader!=this) gG.SetShader(this);if (mRehup) Rehup();Graphics_Core::DrawShape(mShader,theVerts,theVCount);}
void Shader::DrawShape(void* theVerts, int theVCount, vbindex* theI, int theICount) {if (gG.mCurrentShader!=this) gG.SetShader(this);if (mRehup) Rehup();Graphics_Core::DrawIndexedShape(mShader,theVerts,theVCount,theI,theICount);}
void Shader::DrawShape(int theShape) {if (gG.mCurrentShader!=this) gG.SetShader(this);if (mRehup) Rehup();Graphics_Core::DrawShape(theShape);}
int	Shader::CreateShape(void* theVerts, int theVCount, vbindex* theI, int theICount) {IsLoaded();return Graphics_Core::CreateShape(mShader,theVerts,theVCount,theI,theICount);}

void Shader::DrawModel(Model& theModel, Matrix* theMatrix)
{
	/*
	if (theModel.mShader!=this) {IsLoaded();theModel.ConnectToShader(this);}
	if (theMatrix) gG.SetWorldMatrix(*theMatrix);
	if (theModel.mSprite) gG.SetTexture(theModel.mSprite->mTexture);
	DrawShape(theModel.mShape);
	*/
}
void Shader::Preload(Model& theModel) {}//{IsLoaded();if (theModel.mShader!=this) theModel.ConnectToShader(this);}


int Shader::LoadModel(IOBuffer& theBuffer, Sprite* theSprite, Cube* receiveBounds, Array<Key3D>* aKeyList)
{
	short aVersion=theBuffer.ReadShort();

	if (aVersion==1)
	{
		Cube aBounds=theBuffer.ReadCube();if (receiveBounds) *receiveBounds=aBounds;

		//
		// Keys...
		//
		int aKeys=theBuffer.ReadInt();
		if (aKeys>0)
		{
			bool aKillKeys=false;
			if (!aKeyList) {aKeyList=new Array<Key3D>;aKillKeys=true;}
			aKeyList->GuaranteeSize(theBuffer.ReadInt());for (int aCount=0;aCount<aKeyList->Size();aCount++) {aKeyList->Element(aCount).mPos=theBuffer.ReadVector();aKeyList->Element(aCount).mRotation=theBuffer.ReadVector();}
			if (aKillKeys) {delete aKeyList;aKeyList=NULL;}
		}

		//
		// Indicia
		//
		Quad* aQuad=NULL;if (theSprite) aQuad=&theSprite->GetTextureQuad();
		return StreamShape(theBuffer,aQuad);
		Array<vbindex> aI;aI.GuaranteeSize(theBuffer.ReadInt());
		for (int aCount=0;aCount<aI.Size();aCount++) {aI[aCount]=theBuffer.ReadShort();}
	}
	return -1;
}


GX& GetGX()
{
	static GX* aB=NULL;
	if (!aB) aB=new GX;
	return *aB;
}

void GXPoint::Draw() {gG.SetColor(mColor);gG.DrawPoint(mPos,mSize);}
void GXPoint3D::Draw() {gG.SetColor(mColor);gG.DrawPoint(gG.ManualTransform(mPos,gG.mLast3DDrawMatrix,gG.mLast3DViewport),mSize);}
void GXLine::Draw() {gG.SetColor(mColor);gG.DrawLine(mLine.mPos[0],mLine.mPos[1],mSize);}
void GXLine3D::Draw() {gG.SetColor(mColor);gG.DrawLine(gG.ManualTransform(mLine.mPos[0],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.ManualTransform(mLine.mPos[1],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),mSize);}
void GXDirLine3D::Draw() {gG.SetColor(1);gG.DrawLine(gG.ManualTransform(mLine.mPos[0],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.ManualTransform(mLine.mPos[1],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),mSize,mColor[0],mColor[1]);}
void GXTri::Draw() {gG.SetColor(mColor);gG.DrawLine(mTri.mPos[0],mTri.mPos[1],mSize);gG.DrawLine(mTri.mPos[1],mTri.mPos[2],mSize);gG.DrawLine(mTri.mPos[2],mTri.mPos[0],mSize);}
void GXTri3D::Draw() {gG.SetColor(mColor);gG.DrawLine(gG.ManualTransform(mTri.mPos[0],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.ManualTransform(mTri.mPos[1],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),mSize);gG.DrawLine(gG.ManualTransform(mTri.mPos[1],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.ManualTransform(mTri.mPos[2],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),mSize);gG.DrawLine(gG.ManualTransform(mTri.mPos[2],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.ManualTransform(mTri.mPos[0],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),mSize);}
void GXFilledTri::Draw() 
{
	gG.SetColor(mColor);

	gG.StartShape(1);
	gG.AddVertex(mTri.mPos[0],gG.mColorInt,0,0);
	gG.AddVertex(mTri.mPos[1],gG.mColorInt,0,0);
	gG.AddVertex(mTri.mPos[2],gG.mColorInt,0,0);
	gG.FinishShape();
}
void GXFilledTri3D::Draw() 
{
	gG.SetColor(mColor);

	gG.StartShape(1);
	gG.AddVertex(gG.ManualTransform(mTri.mPos[0],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.mColorInt,0,0);
	gG.AddVertex(gG.ManualTransform(mTri.mPos[1],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.mColorInt,0,0);
	gG.AddVertex(gG.ManualTransform(mTri.mPos[2],gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.mColorInt,0,0);
	gG.FinishShape();
}
void GXText::Draw() {gG.SetColor(mBKGColor);gG.FillRect(mRect);gG.SetColor(mColor);gG.TempFont()->Center(mText,mPos);}
void GXText3D::Draw() 
{
	Rect aR=mRect;
	Point aP=gG.ManualTransform(mPos,gG.mLast3DDrawMatrix,gG.mLast3DViewport);
	aR.CenterAt(aP);
	gG.SetColor(mBKGColor);gG.FillRect(aR);
	gG.SetColor(mColor);gG.TempFont()->Center(mText,gG.ManualTransform(mPos,gG.mLast3DDrawMatrix,gG.mLast3DViewport)+Point(0,5));
}
void GXPlane3D::Draw()
{
	gG.SetColor(mColor);
	gG.DrawLine(gG.ManualTransform(mPlane.mPos,gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),gG.ManualTransform(mPlane.mPos+mPlane.mNormal,gG.mLast3DDrawMatrix,gG.mLast3DViewport).ToPoint(),mSize);
	Vector aCorner1=gG.ManualTransform(mPlane.mPos-(mPlanePerp[0]/2)-(mPlanePerp[1]/2),gG.mLast3DDrawMatrix,gG.mLast3DViewport);
	Vector aCorner2=gG.ManualTransform(mPlane.mPos+(mPlanePerp[0]/2)-(mPlanePerp[1]/2),gG.mLast3DDrawMatrix,gG.mLast3DViewport);
	Vector aCorner3=gG.ManualTransform(mPlane.mPos-(mPlanePerp[0]/2)+(mPlanePerp[1]/2),gG.mLast3DDrawMatrix,gG.mLast3DViewport);
	Vector aCorner4=gG.ManualTransform(mPlane.mPos+(mPlanePerp[0]/2)+(mPlanePerp[1]/2),gG.mLast3DDrawMatrix,gG.mLast3DViewport);
	gG.SetColor(mColor,.5f);

	gG.StartQuad();
	gG.AddVertex(aCorner1,gG.mColorInt);
	gG.AddVertex(aCorner2,gG.mColorInt);
	gG.AddVertex(aCorner3,gG.mColorInt);
	gG.AddVertex(aCorner4,gG.mColorInt);
	gG.FinishQuad();

	gG.DrawLine(aCorner1.ToPoint(),aCorner2.ToPoint(),mSize);
	gG.DrawLine(aCorner1.ToPoint(),aCorner3.ToPoint(),mSize);
	gG.DrawLine(aCorner3.ToPoint(),aCorner4.ToPoint(),mSize);
	gG.DrawLine(aCorner2.ToPoint(),aCorner4.ToPoint(),mSize);

	gG.SetColor();
}

void GX::Draw() {gG.SetTexture();gG.SetShader();gG.PushColor();EnumSmartList(GXObj,aDD,mList) aDD->Draw();gG.PopColor();}
Smart(GXObj) GX::AddPoint(Point thePos,Color theColor,float theSize)
{
	thePos+=mOffset;
	Smart(GXPoint) aDD=new GXPoint;
	aDD->mPos=thePos;
	aDD->mColor=theColor;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}
Smart(GXObj) GX::AddPoint3D(Vector thePos,Color theColor,float theSize)
{
	thePos+=mOffset;
	Smart(GXPoint3D) aDD=new GXPoint3D;
	aDD->mPos=thePos;
	aDD->mColor=theColor;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}
Smart(GXObj) GX::AddLine(Point thePos1, Point thePos2,Color theColor,float theSize)
{
	thePos1+=mOffset;
	thePos2+=mOffset;
	Smart(GXLine) aDD=new GXLine;
	aDD->mLine=Line(thePos1,thePos2);
	aDD->mColor=theColor;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}
Smart(GXObj) GX::AddLine3D(Vector thePos1, Vector thePos2,Color theColor,float theSize)
{
	thePos1+=mOffset;
	thePos2+=mOffset;
	Smart(GXLine3D) aDD=new GXLine3D;
	aDD->mLine=Line3D(thePos1,thePos2);
	aDD->mColor=theColor;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}
Smart(GXObj) GX::AddDirLine3D(Vector thePos1, Vector thePos2,Color theColor1, Color theColor2,float theSize)
{
	thePos1+=mOffset;
	thePos2+=mOffset;
	Smart(GXDirLine3D) aDD=new GXDirLine3D;
	aDD->mLine=Line3D(thePos1,thePos2);
	aDD->mColor[0]=theColor1;
	aDD->mColor[1]=theColor2;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}
Smart(GXObj) GX::AddTri(Point thePos1, Point thePos2, Point thePos3, Color theColor,float theSize)
{
	thePos1+=mOffset;
	thePos2+=mOffset;
	thePos3+=mOffset;
	Smart(GXTri) aDD=new GXTri;
	aDD->mTri=Triangle(thePos1,thePos2,thePos3);
	aDD->mColor=theColor;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}

Smart(GXObj) GX::AddTri3D(Vector thePos1, Vector thePos2, Vector thePos3, Color theColor,float theSize)
{
	thePos1+=mOffset;
	thePos2+=mOffset;
	thePos3+=mOffset;
	Smart(GXTri3D) aDD=new GXTri3D;
	aDD->mTri=Triangle3D(thePos1,thePos2,thePos3);
	aDD->mColor=theColor;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}

Smart(GXObj) GX::AddFilledTri(Point thePos1, Point thePos2, Point thePos3, Color theColor,float theSize)
{
	thePos1+=mOffset;
	thePos2+=mOffset;
	thePos3+=mOffset;
	Smart(GXFilledTri) aDD=new GXFilledTri;
	aDD->mTri=Triangle(thePos1,thePos2,thePos3);
	aDD->mColor=theColor;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}

Smart(GXObj) GX::AddFilledTri3D(Vector thePos1, Vector thePos2, Vector thePos3, Color theColor,float theSize)
{
	thePos1+=mOffset;
	thePos2+=mOffset;
	thePos3+=mOffset;
	Smart(GXFilledTri3D) aDD=new GXFilledTri3D;
	aDD->mTri=Triangle3D(thePos1,thePos2,thePos3);
	aDD->mColor=theColor;
	aDD->mSize=theSize;
	if (mEnabled) mList+=aDD;
	return aDD;
}

void GX::AddCube(Cube theCube, Color theColor,float theSize)
{
	if (!mEnabled) return;
	AddLine3D(theCube.Corner(0),theCube.Corner(1),theColor,theSize);
	AddLine3D(theCube.Corner(0),theCube.Corner(2),theColor,theSize);
	AddLine3D(theCube.Corner(1),theCube.Corner(3),theColor,theSize);
	AddLine3D(theCube.Corner(2),theCube.Corner(3),theColor,theSize);
	AddLine3D(theCube.Corner(0+4),theCube.Corner(1+4),theColor,theSize);
	AddLine3D(theCube.Corner(0+4),theCube.Corner(2+4),theColor,theSize);
	AddLine3D(theCube.Corner(1+4),theCube.Corner(3+4),theColor,theSize);
	AddLine3D(theCube.Corner(2+4),theCube.Corner(3+4),theColor,theSize);
	AddLine3D(theCube.Corner(0),theCube.Corner(0+4),theColor,theSize);
	AddLine3D(theCube.Corner(1),theCube.Corner(1+4),theColor,theSize);
	AddLine3D(theCube.Corner(2),theCube.Corner(2+4),theColor,theSize);
	AddLine3D(theCube.Corner(3),theCube.Corner(3+4),theColor,theSize);
}

Smart(GXObj) GX::AddText(Point thePos,String theString, Color theColor,Color theBKGColor)
{
	Smart(GXText) aT=new GXText;
	thePos+=mOffset;
	aT->mColor=theColor;
	aT->mText=theString;
	aT->mPos=thePos;
	aT->mBKGColor=theBKGColor;
	aT->mRect=gG.TempFont()->GetBoundingRect(theString).Expand(5);
	aT->mRect.CenterAt(aT->mPos);
	if (mEnabled) mList+=aT;
	return aT;
}

Smart(GXObj) GX::AddText3D(Vector thePos,String theString, Color theColor,Color theBKGColor)
{
	Smart(GXText3D) aT=new GXText3D;
	thePos+=mOffset;
	aT->mColor=theColor;
	aT->mText=theString;
	aT->mPos=thePos;
	aT->mBKGColor=theBKGColor;
	aT->mRect=gG.TempFont()->GetBoundingRect(theString).Expand(5);
	aT->mRect.CenterAt(aT->mPos);
	if (mEnabled) mList+=aT;
	return aT;
}

Smart(GXObj) GX::AddPlane3D(Vector thePos, Vector theNormal, Color theColor, float theSize)
{
	Smart(GXPlane3D) aP=new GXPlane3D;
	thePos+=mOffset;
	aP->mPlane.Create(thePos,theNormal);
	aP->mColor=theColor;
	aP->mSize=theSize;

	thePos+=Vector(1,1,1);
	Vector aOther;
	gMath.DoesInfiniteLineIntersectPlane(thePos,thePos+theNormal,aP->mPlane,&aOther);

	aP->mPlanePerp[0]=aOther-aP->mPlane.mPos;
	aP->mPlanePerp[0].Normalize();

	Matrix aMat;
	aMat.RotateAroundAxis3D(theNormal,90);
	aP->mPlanePerp[1]=aMat.ProcessPoint(aP->mPlanePerp[0]);

	if (mEnabled) mList+=aP;
	return aP;
}

#ifdef USEPIPELINE
void Graphics::InitializePipelines()
{
	mPipeline_Normal=new Pipeline<Vx2D>(&gSh.mShader2D,10000);
	mPipeline_RenderWhite=new Pipeline<Vx2D>(&gSh.mShader2DWhite,mPipeline_Normal);
	mPipeline_Grey=new Pipeline<Vx2D>(&gSh.mShader2DGrey,mPipeline_Normal);

	Image aImage;
	aImage.MakeBlank(2,2,Color(1));
	mBlankTexture=LoadTexture565(aImage);
}
void Graphics::SetPipeline(PipelineBase* thePipeline)
{
	if (thePipeline!=mCurrentPipeline)
	{
		Flush();
		mCurrentPipeline=thePipeline;
	}
}
#endif
#endif //RAPT_GRAPHICS_CPP

#ifdef RAPT_ML_H

//
// Backwards compatibility... older games can conflict with this file.
// Older games should set NO_RAPT_ML in their defines.  This looks like it's only an issue for Apple (as usual)...
// looks like Apple is compiling it into the 
//

#ifndef NO_RAPT_ML

#define MLHOOK(func) [&](MLRender* theThis)->bool func
#define MLHOOKPTR std::function<auto(MLRender* theThis)->bool>
#define MLDRAWCUSTOMHOOK(func) [&](MLObject* theThis)->bool func
#define MLDRAWCUSTOMHOOKPTR std::function<auto(MLObject* theThis)->bool>

//
// ML:
//
// All tags support the following params:
// id="theID";			// Gives it a name
// width=n;				// Sets a specific width (can just use w)  Note: A number with a % will be replaced with the actual width times that percent.
// height=n;			// Sets a specific height (can just use h)
// size=w,h;			// Sets the w/h in one swoop
// zorder=n;			// Sets the zorder of the item (can just use z)... this is just a sort at the end of formatting to put them in order (-100 to 100). Lower numbers draw first.
// ext="theExt";		// Sets extended info for the option, for misc usage
//
// <-- -->						Comment (maybe switch to c++ comments?  Do you ever need /* in a web page?)
//
// <setup>						Sets up the current page's setup.
//
//								Custom parameters:
//								border=x;			// Sets the internal border (note: border is used for GENERATION-- when scrolling, it'll scroll complete top to bottom)
//								pagealign=x;		// Aligns the whole thing to top/center/middle/bottom
//								bkgcolor=color;		// Color defaults to transparent
//								clip=bool;			// If clipping is true or false (default to true)
//								minheight=x;		// Minimum height of the page
//
// <title whatever>				Sets the page's title.  This is never displayed, it's for reference.
//
// <custom>						Any custom object (calls DrawCustom)
//								Example: 
//											<custom size=100,100;id=CustomBox;>
//											<custom size=recordname;id=Whatever;> // Makes this from a record
//
// <center><left><right>		Align text (default left)
//
// <align center>				Aligns (same as center/left/right)
//								Example: <align center><align left><align right>
//
// <valign top>					Align all our objects on a line to eachother
//								Example: <valign top><valign bottom><valign middle/center>
//								(You can do this for individual objects)
//
// <fluff><nofluff>				Fluff the size of all objects on a single line so they're all the same vertical size.
//								(Note that this will invalidate all vertical alignment stuff, because container boxes will be the same size)
// 
// <br>							Break to the next line
// <br n>						Break to the next line and move down n pixels
//
// <space n>					Inserts some blank horizontal space between items
// <sp n>						(Just for spacing or moving the cursor around)
//
// <font fontname>				Set current font
//
// <null>						Creates a null object.  This was needed in my old renderer, so I am including it because situations might pop up where it is needed.
//								At the time of this creation, I can't think of any, but putting it in just in case.
//
// <os name></os>				Anything between these tags is ONLY seen if the OS name matches
//								Example: <os android>This is Android</os><os win32>This is win32</os><os ios>This is iOS!</os>
//
// <if expression>				Only sees the code between the tags if the expressione evaluates as true
//								Helper variables:
//								#hour, #min, #second
//								#month, #day, #year
// 
// <page></page>				Nests a page within the page (replacement for table)
//								If width/height is specified, it'll use it.  Otherwise width = _min(how wide the contents are,container width) and height=how high the contents are.
//								All the custom parameters from <setup> can be used here. (You can also put a <setup> inside the page to just do it that way)
//
// <record name></record>		Records all objects, for the purpose of forming a bounding box around them.
//								Example:	<record name>
//											<record name expand=3;> // Records, and expands the recording by 3 at the end...
//
//										Optional Params:
//											expand=n	- Expands the whole rect by n
//											expandup=n	- Expands upward (also down, left, right)
//											expandh=	- Expands horizontally (also expandv)
//											offset=x,y  - Offsets the record
//
// <color name>					Sets the drawing color
// <clipcolor name>				Sets the clipping color
//
// <blink color;color;time>		Sets the drawing color, blinking
//								Example:	<blink color=1,0,0;color=1,1,1;time=30;>
//
// <img url>					Creates an image.
//								Example:
//								<img http://www.url.com/image.jpg > // Specifically http: protocol
//								<img www.url.com/images.jpg> // Whichever protocol RComm connected with
//								//<img preloadname width=100;height=100;> // Image added with AddImage("preloadname",&mSprite), scaled to 100,100
//								//<img bundlename->sprite> // Image from a bundle added with AddBundle("bundlename",&mBundle)
//
// <bundle name url>			Downloads and loads a bundle (must be a .wad file)
//		
//								CAVEATES: Inside the wad, you should have images and a dyna.  Both should have the same filename as 'name' in the tag.
//								I.E. hoggy.png, hoggy.dyna (to match the example above)
//
//								To make a bundle wad, drop hoggy.png+hoggy.dyna+_wad.txt into a folder and drop in bundler2.
//
// <link where></link>			Creates a link out of a group of objects.
//								If you want the link to do special tricks, you need to do:
//
//									void NotifyFunction(MLRender *theRenderer)
//									{
//										String aParam=theRenderer->mCurrentLink->GetParam("ParamName");
//									}
//									mMLRender->SetCustomNotifyFunction(&NotifyFunction);
//
//								Then you can get any params, like so:
//								<link whatever notify=something;otherparam=somethingelse;>
//					
//
//								add "web" to open the link in a browser: 
//								<link http://www.url.com notify=something;web>Blah Blah</link>
//								...including http://, https://, or itms-apps:// will automatically direct it to web.
//								Just saying www.destination.com will send it to the web with whatever protocol RComm connected with (this way,
//								a web game will go http)
//
//								A few automatic links:
//								<link _close> ... closes the ML box (if the box is inside another CPU, that CPU should watch for the box to have mKill=true)
//
// <widget whatever>			Allows you to push a custom widget into the HTML.  We will size from the widget's height/width if our size is zero...
//								Happens at format time.  Override ::Widget(String theID) to return a widget.  Returning null simply makes this a null call.
//								SPECIAL NOTE: You will want to implement "dragthru" for any of these CPUs if they will be on a scrolling page!
//
// <exe theCommand>				Executes a command.  Happens at format time.
//								Override ::Exe(String theCommand)
//
// <setflag name=value>			Sets a flag within the MLRender.  Use this for directing things or whatever...
// <flag name=value>
//
// <offset x,y>					Offsets all subsequent draws.  If no x/y it goes to 0,0.  Also </offset> returns it to 0,0
//								Example: This is <offset 0,5>subscript<offset>
//
// <pushcursor>					Pushes the cursor location
// <popcursor>					Pops the cursor location
//
// <define x=y>					defines so that x gets converted to y
// <definetag x=y>				defines a tag-- example:
//								<definetag textcolor=color 1,1,1,.5>
//								...gets converted to <textcolor>=<color 1,1,1,.5>
//
// TO BE ADDED:
// shrinkwrap=true; for a page... this will shrink the width of the page IF the contents are not wider than what was requested with width=x
// <if eval> ... evals (variables need to be define'd in... this is literally just a raw eval>

enum
{
	ML_BKGCOLOR=0,		// A background color tag
	ML_ENDLINE,			// End of a line... exists only to help with vertical alignment
	ML_TEXT,			// A piece of text
	ML_WORDSPACE,		// Space between words (ignored at line ends and beginnings)
	ML_SPACE,			// User created space with <SP> (ignored at line ends only)
	ML_CUSTOM,			// Custom object
	ML_IMAGE,			// An image
	ML_BREAK,			// Linebreak... same as ML_ENDLINE but isn't auto-removed
	ML_NULL,			// A null object
	ML_ALIGN,			// Alignment object (Center/Left/Right)
	ML_COLOR,			// Color change object
	ML_PAGE,			// A whole nested render page
	ML_RECORD,			// An object recording
	ML_LINK,			// A link
	ML_WIDGET,			// A widget (mCPU is the widget)
	ML_FLAG,			// A flag.  Id=name, ext=value
	ML_OFFSET,			// Sets the draw offset
	ML_BLINK,			// Color, but a blinking object
	ML_PUSHCURSOR,		// Push cursor
	ML_POPCURSOR,		// Pop cursor
	ML_CLIPCOLOR,		// Sets gG.ClipColor (so that you can affect cascading colors)
};

enum
{
	MLFLAG_HIDDEN=0x01,			// This object is hidden
	MLFLAG_INSIGNIFICANT=0x02,	// If object is insignificant (example: Font/Color/Null are insignificant, images, test, etc, are)
	MLFLAG_TOUCHABLE=0x04,		// This object matters for touching
	MLFLAG_FLUFF=0x08,			// Fluff this object
	MLFLAG_UNORGANIZED=0x10,	// This object should NOT be organized into the setup (it will be sized from a record later)

	MLFLAG_XHIDDEN=0x20,		// Not real... just here so I can take hiddens out easily...
	MLFLAG_MAX
};

class MLRender;
class MLLink;
#ifdef _DEBUG
extern int gMLObjectCount;
#endif

class MLObject;
void SetGlobalMLSetupHook(MLHOOKPTR theHook=NULL);
void SetGlobalMLNotifyHook(MLHOOKPTR theHook=NULL);
void SetGlobalMLClickHook(HOOKSTRINGPTR theHook=NULL);
void SetGlobalMLDrawCustomHook(MLDRAWCUSTOMHOOKPTR theHook=NULL);

class MLObject : public virtual Rect
{
public:
	MLObject()
	{
		mType=-1;
		mExtraData=NULL;
		mFlag=0;
		mZOrder=0;
		mCPU=NULL;
		mMLRender=NULL;
		mVAlign=-1; // Align top by default
		mMLLink=NULL;
#ifdef _DEBUG
		gMLObjectCount++;
#endif
	}
	virtual ~MLObject();

	int				mType;				// Type of object (raw text, button, image, etc)
	String			mID;				// Text associated with this item (either raw text from the ML, or id="thetext" for non raw text)
	Array<String>	mExt;				// Extended info... each ext= adds one...
	String			mHookName;			// Any object name we're hooked to (like from size=name)
	char			mFlag;				// Any flags
	char			mZOrder;			// Zorder
	char			mVAlign;			// V alignment-- top/bottom/middle
	String			mParams;			// If Len()>0, process for basic attributes


	void*			mExtraData;			// Any kind of extra data
	CPU*			mCPU;				// If we add a CPU to the process, this is it (it needs to be removed)

	MLRender*		mMLRender;			// Parent
	MLLink*			mMLLink;			// The link we're a part of
	Point			mOffset;			// Draw offset

	virtual void	Draw() {}
	bool			IsDown();
	bool			IsOver();

	//
	// Only happens if it's in the update list...
	//
	virtual bool	Update() {return false;} // Return true to keep it in the list, return false to indicate "done"

};


class MLText : public MLObject
{
public:
	MLText() {mType=ML_TEXT;}
	String			mText;
	Font*			mFont;
	void			Draw();
};

class MLWordSpace : public MLObject {public:MLWordSpace() {mType=ML_WORDSPACE;mFlag|=MLFLAG_XHIDDEN;}}; // Space between words (auto generated)
class MLWhiteSpace : public MLObject {public:MLWhiteSpace() {mType=ML_SPACE;mFlag|=MLFLAG_XHIDDEN;}}; // Any whitespace, from <sp x>
class MLNull : public MLObject {public:MLNull() {mType=NULL;mFlag|=MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT;}}; // Null object, in case ever needed... <null>
class MLColor : public MLObject {public:MLColor() {mType=ML_COLOR;mFlag|=MLFLAG_INSIGNIFICANT;}Color mColor;void Draw() {gG.SetColor(mColor);}}; // Changes color
class MLClipColor : public MLObject {public:MLClipColor() {mType=ML_CLIPCOLOR;mFlag|=MLFLAG_INSIGNIFICANT;}Color mColor;void Draw() {gG.SetClipColor(mColor);}}; // Changes color
class MLBlink : public MLObject {public:MLBlink() {mType=ML_BLINK;mFlag|=MLFLAG_INSIGNIFICANT;mTime=30;}Color mColor[2];int mTime;void Draw() {if ((AppTime()%(mTime*2))<mTime) gG.SetColor(mColor[0]);else gG.SetColor(mColor[1]);}}; // Changes color
class MLAlign : public MLObject {public:MLAlign() {mType=ML_ALIGN;mFlag=MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT;mAlign=0;}char mAlign;};
class MLRecord : public MLObject {public:MLRecord() {mType=ML_RECORD;mFlag|=MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT;}SmartList(MLObject) mObjectList;};
class MLOffset : public MLObject {public:MLOffset() {mType=ML_OFFSET;mFlag|=MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT;}Point mOffset;};
class MLBreak : public MLObject {public:MLBreak() {mType=ML_BREAK;mExtraY=0;}float mExtraY;};
class MLPushCursor: public MLObject {public: MLPushCursor() {mType=ML_PUSHCURSOR;mFlag|=MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT;}};
class MLPopCursor: public MLObject {public: MLPopCursor() {mType=ML_POPCURSOR;mFlag|=MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT;}};

//
// Not an object...bundles are just loaded and filled.
//
class MLBundle
{
public:
	MLBundle() {mOwnBundle=false;mBundle=NULL;}
	virtual ~MLBundle() {if (mOwnBundle && mBundle) delete mBundle;mBundle=NULL;mOwnBundle=false;}

	String					mID;
	bool					mOwnBundle;
	SpriteBundle*			mBundle;
};

class MLRender : public SwipeBox
{
public:
	MLRender();
	virtual ~MLRender();

	//
	// Helper for sizing... if we ever size MLRender manually, then we turn on mFixedHeight, which locks it to that size.
	// If we don't (or height=0), then it is sized to the size of the contents.
	//
	// I put this in because I don't want to say "mFixedHeight=true" every time I size things.  Meanwhile, if a height is never specified, it'll be
	// auto-sized.
	//
	inline void			ManualSetSize() {if (mHeight>0) mFixedHeight=true;} // We manually set the size... so now mFixedHeight is true.

	inline void			Resize(float theX, float theY, float theWidth, float theHeight) {SwipeBox::Resize(theX,theY,theWidth,theHeight);ManualSetSize();}
	inline void			Resize(Point theP1, Point theP2) {SwipeBox::Resize(theP1,theP2);ManualSetSize();}
	inline void			SetSize(float theX, float theY, float theWidth, float theHeight) {SwipeBox::Resize(theX,theY,theWidth,theHeight);ManualSetSize();}
	inline void			Size(float theX, float theY, float theWidth, float theHeight) {SwipeBox::Resize(theX,theY,theWidth,theHeight);ManualSetSize();}
	inline void			Size(Rect theRect) {SwipeBox::Resize(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);ManualSetSize();}
	inline void			Size(float theWidth, float theHeight) {SwipeBox::Size(theWidth,theHeight);ManualSetSize();}
	inline void			Size(Point theSize) {SwipeBox::Size(theSize.mX,theSize.mY);ManualSetSize();}
	inline void			Size() {SwipeBox::Size();ManualSetSize();}

	static MLHOOKPTR			mGlobalSetupHook;
	static MLHOOKPTR			mGlobalNotifyHook;
	static HOOKSTRINGPTR		mGlobalClickHook;
	static MLDRAWCUSTOMHOOKPTR	mGlobalDrawCustomHook;

	//virtual void		HookTo(void* thePointer);
	void				LoadFromString(String theString); // Resets the ML and invokes processing of ML

	void				Load(String theFilename);
	inline void			Load(IOBuffer& theBuffer) {LoadFromString(theBuffer.ToString());}

	void				Format(String theString);	// Creates the objects from the string
	void				Rehup(); // Organizes our objects (can be called seperately because sizes might be changed!

	void				ThreadLock();
	void				ThreadUnlock();
	void				ThreadLock_Draw();
	void				ThreadUnlock_Draw();

	void				Setup(String theParams);	// Setup from a string of params

	String				ProcessTag(String theTag, String theAccumulate); // Processes a tag.  If you want to get all contents within the string, return the name of the tag (without params)
																		 // Example... if starting a <page> group, return "<page>"

	MLRender*			Root();	// Gets the root MLRender (for adding objects)
	MLRender*			mMLRenderParent;

	virtual CPU*		HitTest(float x, float y);

	void				Reset();
	void				FixBorders();

	void				Notify(void* theData);

	String				GetCurrentLinkID();

public:
	//
	// Key variables...
	// 
	String					mURL;	// Last loaded URL (if any)
	bool					mRehup;	// If true, we rehup (if you change the size of an item or anything like that, it's a rehup)
	void					DoRehup();
	inline void				NeedRehup() {DoRehup();}
	inline void				WantRehup() {DoRehup();}

	String					mTitle;	

	SmartList(MLObject)		mObjectList;
	SmartList(MLObject)		mUpdateObjectList; // These update (for instance, if downloading...)
	//SmartList(MLObject)		mTouchObjectList;
	SmartList(MLObject)		mDrawList;
	SmartList(MLObject)		mPageList;	 // Pages within our page...
	int						GetNextSignificantObjectType(int theSlotNumber, int theDir);

	//
	// Fonts
	//
	struct FontStruct		{String mName;Font* mFont;};
	struct ImageStruct		{String mName;Sprite* mSprite;};
	struct PendingObject	{String mInfo;Smart(MLObject) mMLObject;};
	struct DefineStruct		{String mDefine;String mReplaceWith;};
	struct FlagStruct		{String mName;String mValue;};
	//struct NotifyStruct		{String mName;CPU* mCPU;};

	struct Resources
	{
		Resources() 
		{
			mVersion="1";
			mBrokenImage=NULL;
			mLoadStubRunning=false;
			mCustomNotifyFunction=NULL;
			mLoading=0;
		}
		SmartList(FontStruct)		mFontList;
		SmartList(ImageStruct)		mImageList;
		SmartList(MLBundle)			mBundleList;
		SmartList(PendingObject)	mPendingObjectList;		// We reconcile these at the end, they're images that in some way need something else to be reconciled...
		SmartList(DefineStruct)		mDefineList;
		SmartList(FlagStruct)		mFlagList;
		//bool						(*mCustomNotifyFunction)(MLRender* theRenderer);
		MLHOOKPTR					mCustomNotifyFunction=NULL;
		HOOKSTRINGPTR				mCustomClickFunction=NULL;

		//SmartList(NotifyStruct)		mNotifyList;

		String					mVersion;		// ML version (for blasting the cache)
		Sprite					*mBrokenImage;	// If we have a broken link
		bool					mLoadStubRunning;	// If the load stub is running
		int						mLoading;			// If we're loading
	};
	void						AddFont(String theFontName, Font* theFont);
	//void						AddNotify(String theNotifyName, CPU* theCPU);
	void						AddImage(String theImageName, Sprite* theSprite);
	void						AddDefine(String theDefine, String theReplace);
	inline void					Define(String theDefine, String theReplace) {AddDefine(theDefine,theReplace);}

	Sprite*						GetImage(String theImageName) {GetResources();EnumSmartList(ImageStruct,aIS,mResources->mImageList) if (aIS->mName==theImageName) return aIS->mSprite;return NULL;}

	Font*						mCurrentFont;
	SmartList(MLRecord)			mPendingRecordList;
	SmartList(MLRecord)			mRecordList;
	SmartList(MLLink)			mLinkList;
	Smart(MLLink)				mCurrentLink;
	inline Smart(MLLink)		GetCurrentLink() {return mCurrentLink;}
	Rect						GetRecord(String theName);
	Smart(MLRecord)				mCurrentRecord;
	Smart(Resources)			mResources;
	inline Smart(Resources)		GetResources() {if (mResources.IsNull()) mResources=new Resources;return mResources;}

	SmartList(MLObject)			mPendingItemList;	// List of objects that we will size at the very end
	void						FinishPendingItems();

	void						SetFlag(String theName, String theValue);
	String						GetFlag(String theName);
	MLObject*					GetObjectByID(String theID);
	void						GetAllObjectsByID(String theID, List& theList);
	void						GetAllObjectsStartingWithID(String theID, List& theList);

	//
	// This is the size of the page we can work within...
	//
	Rect					mPageRect;
	short					mSkipDraw;
	float					mBorderTop;
	float					mBorderBottom;
	float					mBorderLeft;
	float					mBorderRight;

	//
	// Info about our core page drawing...
	//
	Color					mBkgColor;
	bool					mClip;

	//
	// We are skipping tags based on various reasons (bit mask)...
	// 0x01 = Skip because of OS
	//
	char					mSkipTags;

	//
	// Remembers our info so we can push and pop it...
	//
	String					mCurrentML;
	Stack<String>			mMLStack;
	inline void				Push() {mMLStack.Push(mCurrentML);}
	inline void				Pop() {mCurrentML=mMLStack.Pop();LoadFromString(mCurrentML);}

	//
	// If we should resize our page to our contents (i.e. from an internal page with no width specifier)
	//
	bool					mSizeWidthToContents;

	//
	// Current valign for tags...
	//
	char					mVAlign;
	char					mPageAlign;
	char					mFlags;	// Flags for tags
	String					mPendingPageParams;		
	String					mSetupParams;	

	//
	// Thread locks:
	//
	int						mThreadLock;
	int						mThreadLock_Draw;

	//
	// If the height is fixed (i.e. it was specified...)
	//
	bool					mFixedHeight;
	float					mMinHeight;

	//
	// Fixes a number string... fixes percents, arithmetic, etc...
	//
	float					MLStringToFloat(String theString, Rect* theRect=NULL);

public:

	//
	// Stuff you probably will want to call...
	//
	inline void				SetBrokenLinkImage(Sprite *theBroken) {GetResources()->mBrokenImage=theBroken;}

	//
	// Overridable functions for ease of use...
	//
	virtual void				Update();
	virtual void				DrawUnder() {}
	virtual void				Draw();
	virtual void				DrawOver() {}
	virtual MLObject*			ParseCustomTag(String theTag, String theParams) {return NULL;}	// A way to add tags if you want 'em... this will get called if the tag doesn't match anything else!
	virtual	bool				DrawCustom(MLObject* theObject) {return false;}
	virtual void				FinishedRehup() {} // Called when we finished rehupping everything (but before moving things into the draw list)
	virtual void				ClickLink(String theID) {}
	virtual CPU*				Widget(String theID) {return NULL;}		// For handling <widget>
	virtual void				Exe(String theCommand) {}				// For handling <exe>

	MLDRAWCUSTOMHOOKPTR			mDrawCustomHook=NULL;
	void						SetDrawCustomHook(MLDRAWCUSTOMHOOKPTR theHook) {mDrawCustomHook=theHook;}

	// Add the custom notify here!
	//CPU*						mCustomNotifyCPU;
	//inline void					SetCustomNotifyCPU(CPU* theCPU) {mCustomNotifyCPU=theCPU;}

//	void						SetCustomNotifyFunction(bool (*theFunction)(MLRender* theRenderer)) {GetResources()->mCustomNotifyFunction=theFunction;}
	void						SetCustomNotifyFunction(MLHOOKPTR theHook) {GetResources()->mCustomNotifyFunction=theHook;}
	void						SetCustomClickFunction(HOOKSTRINGPTR theHook) {GetResources()->mCustomClickFunction=theHook;}
};

class MLCustom : public MLObject
{
public:
	MLCustom() {mType=ML_CUSTOM;mFirstDraw=true;}
	void			Draw();
	bool			mFirstDraw;
};

class MLPage : public MLObject
{
public:
	MLPage() {mType=ML_PAGE;mFlag|=MLFLAG_TOUCHABLE;mCPU=&mMyMLRender;}
	MLRender		mMyMLRender;
	void			Draw();
};

class MLImage : public MLObject
{
public:
	MLImage() {mType=ML_IMAGE;mSprite=NULL;mOwnSprite=false;}
	virtual ~MLImage() {Reset();}

	void Reset() {if (mOwnSprite && mSprite) delete mSprite;mSprite=NULL;mOwnSprite=false;}
	void Draw();
	bool FixSize() {if (mSprite) {mWidth=(float)mSprite->mWidth;mHeight=(float)mSprite->mHeight;}return (mWidth>0 && mHeight>0);}

	bool Update() 
	{
		if (!(mDLImage->mFlag&IMAGEFLAG_DOWNLOADING))
		{
			if (!(mDLImage->mFlag&IMAGEFLAG_ERROR)) Load(mDLImage.GetPointer());
			mDLImage=NULL;
			return false;
		}
		return true;
	}


	Smart(Image)	mDLImage=NULL;
	float			mProgress=0; // For downloading...
	Sprite*			mSprite;
	bool			mOwnSprite;

	void Load(Sprite* theSprite) {Reset();mSprite=theSprite;mOwnSprite=false;FixSize();}
	void Load(Image* theImage) {Reset();if (theImage) {mSprite=new Sprite;mOwnSprite=true;mSprite->Load(*theImage);}FixSize();}
};
class MLLink : public MLObject
{
public:
	MLLink() {mType=ML_LINK;mCPU=&mButton;mBrowser=false;}
	SmartList(MLObject)			mObjectList;
	Button						mButton;

	Array<String>				mParamName;
	Array<String>				mParamValue;
	String						GetParam(String theName);

	bool						mBrowser;	// Opens this link in a browser
};

class MLBox : public CPU
{
public:

	void						Initialize();
	void						Core_Draw();
	void						Update();

	inline void					SetRenderRect(Rect theRect) {mRenderRect=theRect;}
	inline void					SetContents(String theContents) {mContents=theContents;}
	virtual void				Setup() {} // Override if yu want to do some kind of setup before MLRender gets loaded...

	Rect						mRenderRect;
	MLRender					mRender;
	String						mContents;
};

MLRender* GetCurrentML();	// Gets the current MLRender being worked with (for responding to clicks)
#endif
#endif //RAPT_ML_H

#ifdef RAPT_ML_CPP
#ifndef NO_RAPT_ML

#ifdef _DEBUG
int gMLObjectCount=0;
#endif

void MLRender::ThreadLock() {if (mThreadLock==-1) mThreadLock=CreateThreadLock();::ThreadLock(mThreadLock);}
void MLRender::ThreadUnlock() {::ThreadUnlock(mThreadLock);}
void MLRender::ThreadLock_Draw() {if (mThreadLock_Draw==-1) mThreadLock_Draw=CreateThreadLock();::ThreadLock(mThreadLock_Draw);}
void MLRender::ThreadUnlock_Draw() {::ThreadUnlock(mThreadLock_Draw);}

MLRender* gCurrentMLRender;
MLRender* GetCurrentML() {if (!gCurrentMLRender) {static MLRender gJunk;return &gJunk;}return gCurrentMLRender;}

MLHOOKPTR MLRender::mGlobalSetupHook=NULL;
MLHOOKPTR MLRender::mGlobalNotifyHook=NULL;
HOOKSTRINGPTR MLRender::mGlobalClickHook=NULL;
MLDRAWCUSTOMHOOKPTR MLRender::mGlobalDrawCustomHook=NULL;

void SetGlobalMLSetupHook(MLHOOKPTR theHook) {MLRender::mGlobalSetupHook=theHook;}
void SetGlobalMLNotifyHook(MLHOOKPTR theHook) {MLRender::mGlobalNotifyHook=theHook;}
void SetGlobalMLClickHook(HOOKSTRINGPTR theHook) {MLRender::mGlobalClickHook=theHook;}
void SetGlobalMLDrawCustomHook(MLDRAWCUSTOMHOOKPTR theHook) {MLRender::mGlobalDrawCustomHook=theHook;}

MLRender::MLRender()
{
	mCurrentFont=NULL;
	//mDLImageListStep=0;
	mRehup=false;
	mFixedHeight=false;
	mSkipDraw=0;
	mSkipTags=0;
	mMLRenderParent=NULL;
	mBorderTop=mBorderLeft=mBorderRight=mBorderBottom=0;
	mPageAlign=-1;
	mMinHeight=0;
	mThreadLock=-1;
	mSizeWidthToContents=false;
	mThreadLock_Draw=-1;
	mClip=true;
	//mCustomNotifyCPU=NULL;
	Reset();

	/*
	AddDefine("#os",String::GetHashI(OS_Core::GetOSName()));
	AddDefine("#win32",String::GetHashI("win32"));
	AddDefine("#android",String::GetHashI("android"));
	AddDefine("#ios",String::GetHashI("ios"));
	AddDefine("#wasm",String::GetHashI("wasm"));
	AddDefine("#mac",String::GetHashI("mac"));
	AddDefine("#linux",String::GetHashI("linux"));
	*/
}

MLRender::~MLRender()
{
	if (mThreadLock!=-1) ReleaseThreadLock(mThreadLock);
	if (mThreadLock_Draw!=-1) ReleaseThreadLock(mThreadLock_Draw);

	if (gCurrentMLRender==this) gCurrentMLRender=NULL;
}

/*
void MLRender::HookTo(void* thePointer)
{
	if (thePointer) 
	{
		CPU** aCPU=(CPU**)thePointer;
		*aCPU=this;
		mVariablePointer=thePointer;
	}
	else mVariablePointer=NULL;
}
*/


void MLRenderLoadStub(void* theArg)
{
#ifdef _DEBUG
	gOut.Out("MLRenderLoadStub...started");
#endif
	MLRender* aMLRender=(MLRender*)theArg;
	aMLRender->GetResources()->mLoadStubRunning=true;
	IOBuffer aBuffer;
	if (aMLRender->mURL.StartsWith("http://") || aMLRender->mURL.StartsWith("https://")) aBuffer.Download(aMLRender->mURL);
	else aBuffer.Load(aMLRender->mURL);
	aMLRender->LoadFromString(aBuffer.ToString());
	aMLRender->GetResources()->mLoadStubRunning=false;
#ifdef _DEBUG
	gOut.Out("MLRenderLoadStub...finished!");
#endif
}

void MLRender::Load(String theFilename)
{
	if (theFilename.StartsWith("www.")) theFilename.Insert(RComm::GetURLPrefix().c());

	mURL=theFilename;
#ifndef NO_THREADS
	Thread(&MLRenderLoadStub,this);
#else
	MLRenderLoadStub(this);
#endif
}

void MLRender::Draw()
{
	gCurrentMLRender=this;
	gG.PushColor();
	if (GetResources()->mLoading>0) return;

	if (mBkgColor.mA>0)
	{
		gG.SetColor(mBkgColor);
		gG.FillRect(mScrollX,mScrollY,mWidth,mHeight);
		gG.SetColor();
	}

	gG.PushTranslate();
	if (mSkipDraw>0)
	{
		mSkipDraw=_max(0,mSkipDraw-1);
		gG.Translate(0.0f,-mHeight*100);
	}
	ThreadLock_Draw();

	EnumSmartList(MLObject,aO,mDrawList) 
	{
		/*
		gG.PushColor();
		gG.SetColor(.25f);
		if (aO->mType==ML_WORDSPACE) gG.SetColor(1,0,0);
		gG.FillRect(aO->mX,aO->mY,aO->mWidth,1);
		gG.FillRect(aO->mX,aO->mY+aO->mHeight-1,aO->mWidth,1);
		gG.FillRect(aO->mX,aO->mY,1,aO->mHeight);
		gG.FillRect(aO->mX+aO->mWidth-1,aO->mY,1,aO->mHeight);
		gG.PopColor();
		/**/

		gG.PushTranslate();
		gG.Translate(aO->UpperLeft()+aO->mOffset);
		aO->Draw();
		gG.PopTranslate();
	}
	ThreadUnlock_Draw();
	gG.PopTranslate();
	gG.PopColor();

}

void MLRender::LoadFromString(String theString)
{
	mCurrentML=theString;
	EnumSmartList(DefineStruct,aD,GetResources()->mDefineList) 
	{
		theString.Replace(aD->mDefine,aD->mReplaceWith);
	}

	GetResources()->mLoading++;
	ThreadLock();
	if (mGlobalSetupHook) MLRender::mGlobalSetupHook(this);
	Format(theString);
	ThreadUnlock();
	GetResources()->mLoading=_max(0,GetResources()->mLoading--);
}

void MLRender::Reset()
{
	mBorderTop=mBorderLeft=mBorderRight=mBorderBottom=0;
	mBkgColor=Color(0);
	mObjectList.Clear();
	//mTouchObjectList.Clear();
	mPendingItemList.Clear();
	mRecordList.Clear();
	mPageList.Clear();
	mLinkList.Clear();
	mVAlign=-1;
	mFlags=0;
}

void MLRender::Format(String theString)
{
	Reset();
	GetResources();

	if (!mCurrentFont)
	{
		gOut.Out("!!!! MLRender::Format() - No default font!");
		return;
	}

	theString+="<>"; // Add this tag to the end as an easy way to make sure the last word gets processed without
					 // Having to do a cleanup after the loop is over...
	

	//mPageRect=Rect(0,0,mWidth,mHeight);

	int aPPos=0;
	int aStartTagPos=-1;
	String mAccum;		// Accumulates everything

	bool aInTag=false;
	String mTagAccum;	// Accumulates a tag

	String aFindEndTag="";	// An end tag we're looking for
	int aNestedTagCount=0;	// So that we can parse past nested tags, like <page><page></page></page>

	//
	// Okay, we walk the ML... we're just converting everything into tags now, including all text.
	//
	bool aInComment=false;
	char aInQuote=0;
	if (theString.Len())
	while (aPPos<=theString.Len())
	{
		int aDefineCount=GetResources()->mDefineList.GetCount();
		char aC=theString[aPPos];
		if (aC<32) {aPPos++;continue;}

		if (aInComment)
		{
			if (aC=='-' && theString[aPPos+1]=='-' && theString[aPPos+2]=='>') 
			{
				aInComment=false;
				aPPos+=3;aC=theString[aPPos];
			}
		}
		else
		{
			if (aC=='<' && theString[aPPos+1]=='-' && theString[aPPos+2]=='-') aInComment=true;
		}

		if (aInQuote) {if (aC==aInQuote) {aInQuote=0;}}
		else if (aC=='\"') aInQuote=aC;

		if (!aInComment)
		{
			if (aC==10) aC=0;
			if (aC==13) aC=' ';
			//gOut.Out("%c",aC);

			{
				if (aNestedTagCount==0) if (!aInTag) if (aC==' ' || aC==0) 
				{
					if (mAccum.Len()) ProcessTag("<txt>",mAccum);
					if (aC) ProcessTag("<wsp>"," ");
					mAccum="";
					aC=0;
				}

				if (!aInQuote && aC=='<')
				{
					if (aNestedTagCount==0) if (!aInTag) {if (mAccum.Len()) ProcessTag("<txt>",mAccum.GetSegment(0,mAccum.Len()));mAccum="";}
					mTagAccum="";
					aInTag=true;
				}
				if (!aInQuote && aC=='>' && aInTag)
				{
					mTagAccum+=aC;
					if (aFindEndTag.Len()) 
					{
						String aTest=mTagAccum.GetSegmentBefore(' ');
						if (aTest.LastChar()!='>') aTest+='>';
						if (aTest.StartsWith(aFindEndTag)) aNestedTagCount++;
					}

					if (mTagAccum[1]=='/')
					{
						String aCheck='<';aCheck+=mTagAccum.GetSegment(2,9999);
						if (aCheck==aFindEndTag) 
						{
							aNestedTagCount=_max(0,aNestedTagCount-1);
						}
					}

					if (aNestedTagCount==0) 
					{
						mAccum.Delete(mAccum.Len()-(mTagAccum.Len()-1));
						aFindEndTag=ProcessTag(mTagAccum,mAccum);
						if (aFindEndTag.Len()) aNestedTagCount++;

						mAccum="";
						aInTag=false;
						aC=0;
					}

				}

				if (aC)
				{
					if (aInTag) mTagAccum+=aC;

					//gOut.Out("%d -> %s",aNestedTagCount,mAccum.c());
					mAccum+=aC;
				}
			}
		}

		aPPos++;

		// Tweak any defines...
		if (GetResources()->mDefineList.GetCount()!=aDefineCount) {EnumSmartList(DefineStruct,aD,GetResources()->mDefineList) theString.Replace(aD->mDefine,aD->mReplaceWith,aPPos);}
	}

	//
	// Remove stray ML_WORDSPACE objects
	//
	for (int aCount=0;aCount<mObjectList.GetCount();aCount++)
	{
		Smart(MLObject) aO=mObjectList[aCount];
		if (aO->mType==ML_WORDSPACE)
		{
			if (GetNextSignificantObjectType(aCount,1)!=ML_TEXT || GetNextSignificantObjectType(aCount,-1)!=ML_TEXT)
			{
				mObjectList-=aO;
				mDrawList-=aO;
				aCount--;
			}
		}
	}


	//
	// Any pending objects need to be reconciled...
	//
	if (!mMLRenderParent)
	{
		EnumSmartList(PendingObject,aPO,GetResources()->mPendingObjectList)
		{
			switch (aPO->mMLObject->mType)
			{
			case ML_IMAGE:
				{
					MLImage* aMI=(MLImage*)aPO->mMLObject.GetPointer();
					String aBundle=aPO->mInfo.GetSegmentBefore("::");
					String aSprite=aPO->mInfo.GetSegmentAfter("::");
					EnumSmartList(MLBundle,aB,GetResources()->mBundleList)
					{
						if (aB->mID==aBundle)
						{
							Sprite* aS=new Sprite;
							aS->ManualLoad(aB->mBundle->GetTextureRef(0),aB->mBundle->PointAtDyna(aSprite));
							aMI->Load(aS);aMI->mOwnSprite=true;
						}
					}
				}
				break;
			}
		}
		GetResources()->mPendingObjectList.Clear();
	}

	//
	// We skip one draw, because this allows us to do custom item sizing in CustomDraw (good quarantine)
	//
	mSkipDraw=1;
	mRehup=true;
}

String MLRender::ProcessTag(String theTag, String theAccumulate)
{
	if (theTag=="<>") return "";

	String aContents=theTag.GetSegment(1,theTag.Len()-2);
	String aTag=aContents.GetSegmentBefore(' ');
	String aParams=aContents.GetSegmentAfter(' ');

	//gOut.Out("Tag: %s -> %s",aTag.c(),aParams.c());

	Smart(MLObject) aResultObject;

	if (aTag=="/os") {mSkipTags|=0x02;mSkipTags^=0x02;return "";}
	if (aTag=="/if") {mSkipTags|=0x01;mSkipTags^=0x01;return "";}

	if (mSkipTags) return "";

	if (aTag=="os") {if (gAppPtr->GetOSName()!=aParams) mSkipTags|=0x02;return "";}
	if (aTag=="if") {if (aParams.StartsWith("\"")) {String aNewParams=aParams.GetQuotes();if (aNewParams.Len()) aParams=aNewParams;}if (!MLStringToFloat(aParams)) mSkipTags|=0x01;return "";}

	if (aTag=="txt") // Ordinary text
	{
		Smart(MLText) aT=new MLText;
		aT->mText=theAccumulate;
		aT->mFont=mCurrentFont;
		aT->Size(mCurrentFont->GetBoundingRect(aT->mText));
		mObjectList+=aT;
		aResultObject=aT;
	}
	else if (aTag=="wsp") // Wordspace (current font text size)
	{
		Smart(MLWordSpace) aS=new MLWordSpace;
		aS->Size(mCurrentFont->GetBoundingRect(" "));
		mObjectList+=aS;
		aResultObject=aS;
	}
	else if (aTag=="click")
	{
		String aText;
		if (!gAppPtr->IsTouchDevice()) aText=aTag;
		else
		{
			char* aLower="touch";
			char* aUpper="TOUCH";
			for (int aCount=0;aCount<5;aCount++)
			{
				if (aTag[aCount]<='Z') aText+=aUpper[aCount];
				else aText+=aLower[aCount];
			}
		}

		Smart(MLText) aT=new MLText;
		aT->mText=aText;
		aT->mFont=mCurrentFont;
		aT->Size(mCurrentFont->GetBoundingRect(aT->mText));
		mObjectList+=aT;
		aResultObject=aT;
	}
	else if (aTag=="custom")
	{
		Smart(MLCustom) aO=new MLCustom;
		mObjectList+=aO;
		aResultObject=aO;
	}
	else if (aTag=="setup") mSetupParams=aParams;
	else if (aTag=="title") mTitle=aParams;
	else if (aTag=="page")
	{
		//
		// We remember these params, because they don't get invoked until we hit our end tag...
		//
		mPendingPageParams=aParams;
		return "<page>";
	}
	else if (aTag=="/page")
	{

		Smart(MLPage) aP=new MLPage;
		//aP->mMyMLRender.DisableDraw();
		aP->mMyMLRender.mFixedHeight=false;
		aP->mMyMLRender.mResources=mResources;
		aP->mMyMLRender.mCurrentFont=mCurrentFont;
		aP->mMyMLRender.mMLRenderParent=this;
		aP->mMyMLRender.mFlags=mFlags;
		//
		// We generate the actual page later, after we get the ID/height/width and everything...
		//
		mPageList+=aP;
		mObjectList+=aP;
		aResultObject=aP;
	}
	else if (aTag=="sp" || aTag=="space")
	{
		float aW=aParams.ToFloat();
		if (aW!=0)
		{
			Smart(MLWhiteSpace) aO=new MLWhiteSpace;
			aO->mHeight=1;
			aO->mWidth=aParams.ToFloat();
			mObjectList+=aO;
			aResultObject=aO;
		}
	}
	else if (aTag=="br")
	{
		Smart(MLBreak) aO=new MLBreak;
		aO->mHeight=mCurrentFont->mAscent;
		if (aParams.Len()) aO->mExtraY=aParams.ToFloat();
		mObjectList+=aO;
		aResultObject=aO;
	}
	else if (aTag=="center")
	{
		Smart(MLAlign) aA=new MLAlign;
		aA->mType=ML_ALIGN;
		aA->mFlag|=MLFLAG_HIDDEN;
		aA->mAlign=0;
		mObjectList+=aA;
	}
	else if (aTag=="align")
	{
		Smart(MLAlign) aA=new MLAlign;
		aA->mType=ML_ALIGN;
		aA->mFlag|=MLFLAG_HIDDEN;
		aA->mAlign=-1;
		if (aParams=="left") aA->mAlign=-1;
		if (aParams=="center") aA->mAlign=0;
		if (aParams=="right") aA->mAlign=1;
		mObjectList+=aA;
	}
	else if (aTag=="fluff")
	{
		mFlags|=MLFLAG_FLUFF;
	}
	else if (aTag=="nofluff")
	{
		mFlags|=MLFLAG_FLUFF;
		mFlags^=MLFLAG_FLUFF;
	}
	else if (aTag=="valign")
	{
		if (aParams=="top") mVAlign=-1;
		if (aParams=="middle" || aParams=="center") mVAlign=0;
		if (aParams=="bottom") mVAlign=1;
	}

	else if (aTag=="left")
	{
		Smart(MLAlign) aA=new MLAlign;
		aA->mType=ML_ALIGN;
		aA->mFlag|=MLFLAG_HIDDEN;
		aA->mAlign=-1;
		mObjectList+=aA;
	}
	else if (aTag=="right")
	{
		Smart(MLAlign) aA=new MLAlign;
		aA->mType=ML_ALIGN;
		aA->mFlag|=MLFLAG_HIDDEN;
		aA->mAlign=1;
		mObjectList+=aA;
	}
/*
	else if (aTag=="top")
	{
		Smart(MLVAlign) aA=new MLVAlign;
		aA->mType=ML_VALIGN;
		aA->mFlag|=MLFLAG_HIDDEN;
		aA->mAlign=-1;
		mObjectList+=aA;
	}
	else if (aTag=="bottom")
	{
		Smart(MLVAlign) aA=new MLVAlign;
		aA->mType=ML_VALIGN;
		aA->mFlag|=MLFLAG_HIDDEN;
		aA->mAlign=1;
		mObjectList+=aA;
	}
	else if (aTag=="middle")
	{
		Smart(MLVAlign) aA=new MLVAlign;
		aA->mType=ML_VALIGN;
		aA->mFlag|=MLFLAG_HIDDEN;
		aA->mAlign=0;
		mObjectList+=aA;
	}
*/
	else if (aTag=="font")
	{
		Font *aFont=NULL;
		String aFontName=aParams;//.GetSegmentBefore(' ');
		if (aFontName=="default") aFont=mResources->mFontList[0]->mFont;
		else EnumSmartList(FontStruct,aFS,mResources->mFontList) if (aFS->mName==aFontName) {aFont=aFS->mFont;break;}
		if (!aFont)	{/* FINISHME: We could check the bundles for fonts too...*/}
		if (aFont) mCurrentFont=aFont;
	}
	else if (aTag=="color")
	{
		Smart(MLColor) aC=new MLColor;
		aC->mColor.FromText(aParams);
		mObjectList+=aC;
	}
	else if (aTag=="clipcolor")
	{
		Smart(MLClipColor) aC=new MLClipColor;
		aC->mColor.FromText(aParams);
		mObjectList+=aC;
	}
	else if (aTag=="blink")
	{
		Smart(MLBlink) aBlink=new MLBlink;
		aBlink->mColor[0]=Color(1);
		aBlink->mColor[1]=Color(1,0,0);
		int aColorPtr=0;

		if (aParams.Len())
		{
			String aToken=aParams.GetToken(';');
			while (aToken.Len())
			{
				String aCommand=aToken.GetSegmentBefore('=');
				String aValue=aToken.GetSegmentAfter('=');
				if (aCommand=="color" && aColorPtr<=1) aBlink->mColor[aColorPtr++].FromText(aValue);
				if (aCommand=="time") aBlink->mTime=aValue.ToInt();
				aToken=aParams.GetNextToken(';');
			}
		}
		mObjectList+=aBlink;
	}
	else if (aTag=="record")
	{
		String aID=aParams.GetSegmentBefore(' ');
		aParams=aParams.GetSegmentAfter(' ');
		Smart(MLRecord) aR=new MLRecord;
		aR->mID=aID;
		aR->mParams=aParams;
		mPendingRecordList+=aR;
		mCurrentRecord=aR;
		//mObjectList+=aR;
		aResultObject=aR;
	}
	else if (aTag=="/record") {mCurrentRecord=NULL;}
	else if (aTag=="link")
	{
		mCurrentLink=new MLLink;
		mCurrentLink->mMLRender=this;
		String aLinkWhere=aParams.GetSegmentBefore(' ');
		String aLinkParams=aParams.GetSegmentAfter(' ');

		//
		// Helpers for simplicity...
		//
		if (aLinkWhere.StartsWith("www.")) aLinkWhere.Insert(RComm::GetURLPrefix());
		if (aLinkWhere.ContainsI("http://") || aLinkWhere.ContainsI("https://") || aLinkWhere.ContainsI("itms-apps://")) mCurrentLink->mBrowser=true;

		mCurrentLink->mID=aLinkWhere;

		if (aLinkParams.Len())
		{
			String aToken=aLinkParams.GetToken(';');
			while (aToken.Len())
			{
				String aCommand=aToken.GetSegmentBefore('=');
				String aValue=aToken.GetSegmentAfter('=');
				//if (aCommand=="notify")	{EnumSmartList(NotifyStruct,aNS,GetResources()->mNotifyList) {if (aNS->mName==aValue) {mCurrentLink->mNotify=aNS->mCPU;break;}}}
				if (aCommand=="web") mCurrentLink->mBrowser=true;
				else
				{
					mCurrentLink->mParamName+=aCommand;
					mCurrentLink->mParamValue+=aValue;
				}
				aToken=aLinkParams.GetNextToken(';');
			}
		}
	
		mLinkList+=mCurrentLink;
	}
	else if (aTag=="/link") mCurrentLink=NULL;
	else if (aTag=="version")
	{
		String aV=aParams.GetSegmentBefore(' ');
		aParams=aParams.GetSegmentAfter(' ');
		GetResources()->mVersion=aV;
	}
	else if (aTag=="bundle")
	{
		String aName=aParams.GetSegmentBefore(' ');

		String aURL=aParams.GetSegmentAfter(' ');
		aParams="";

		Smart(MLBundle) aBundle=new MLBundle;
		aBundle->mID=aName;
		aBundle->mBundle=new SpriteBundle;

		bool aFail=true;
		if (aURL.StartsWith("www.")) aURL.Insert(RComm::GetURLPrefix().c());
		if (aURL.StartsWith("http://") || aURL.StartsWith("https://"))
		{
			String aImageName="cache://";aImageName+=aName;
			String aDynaName="cache://";aDynaName+=aName;aDynaName+=".dyna";
			DownloadWad(aURL,GetResources()->mVersion);
			for (int aCount=0;aCount<2;aCount++)
			{
				if (DoesImageExist(aImageName)) 
				{
					Image aImage;
					aImage.Load(aImageName);
					if (aImage.mWidth>0) 
					{
						IOBuffer aDyna;
						aDyna.Load(aDynaName);
						if (aDyna.Len()>0)
						{
							aBundle->mBundle->LoadDyna(aDyna,aImage);
							//aBundle->mBundle->DumpDyna();
							aFail=false;
						}
					}
				}
			
				if (aCount==0) 
				{
					if (aFail) 
					{
						RemoveFile(aDynaName);
						DownloadWad(aURL,GetResources()->mVersion,true);
					}
					else break;
				}
			}
		}
		else
		{
			//
			// Local Dynabundle?  Is this worth bothering with?
			//
		}

		if (!aFail) GetResources()->mBundleList+=aBundle;
	}
	else if (aTag=="img" || aTag=="image")
	{
		String aURL=aParams.GetSegmentBefore(' ');
		aParams=aParams.GetSegmentAfter(' ');
		Smart(MLImage) aIm=new MLImage;

		if (aURL.StartsWith("www.")) aURL.Insert(RComm::GetURLPrefix());
		if (aURL.StartsWith("http://") || aURL.StartsWith("https://"))
		{
			Smart(Image) aDLImage=DownloadImage(aURL,GetResources()->mVersion,&aIm->mProgress);
			if (aIm->mWidth>0) aIm->Load(aDLImage);
			else 
			{
				aIm->mDLImage=aDLImage;
				mUpdateObjectList+=aIm;
			}

			/*
			Image aImage;
			aImage.LoadFromWeb(aURL,GetResources()->mVersion);
			if (aImage.mWidth>0) aIm->Load(&aImage);
			else aIm->Load(mResources->mBrokenImage);
			*/
		}
		else
		{
			if (aURL.Contains("::"))
			{
				//
				// Bundled...
				//
				Smart(PendingObject) aPO=new PendingObject;
				aPO->mInfo=aURL;
				aPO->mMLObject=aIm;
				GetResources()->mPendingObjectList+=aPO;
			}
			else aIm->Load(GetImage(aURL));
		}
		aIm->FixSize();

		mObjectList+=aIm;
		aResultObject=aIm;
	}
	else if (aTag=="offset" || aTag=="off")
	{
		Smart(MLOffset) aOff=new MLOffset;
		aOff->mOffset=aParams.ToPoint();
		mObjectList+=aOff;
		aResultObject=aOff;
	}
	else if (aTag=="/offset" || aTag=="/off")
	{
		Smart(MLOffset) aOff=new MLOffset;
		mObjectList+=aOff;
		aResultObject=aOff;
	}
	else if (aTag=="setflag" || aTag=="flag")
	{
		String aName=aParams.GetSegmentBefore('=');
		String aValue=aParams.GetSegmentAfter('=');
		SetFlag(aName,aValue);
	}
	else if (aTag=="pushcursor" || aTag=="pushc") 
	{
		Smart(MLPushCursor) aPush=new MLPushCursor;
		mObjectList+=aPush;
		aResultObject=aPush;
	}
	else if (aTag=="popcursor" || aTag=="popc") 
	{
		Smart(MLPopCursor) aPop=new MLPopCursor;
		mObjectList+=aPop;
		aResultObject=aPop;
	}
	else if (aTag=="define")
	{
		String aFrom=aParams.GetSegmentBefore('=');
		String aTo=aParams.GetSegmentAfter('=');
		Define(aFrom,aTo);
	}
	else if (aTag=="widget")
	{
		String aName=aParams.GetSegmentBefore(' ');
		aParams=aParams.GetSegmentAfter(' ');

		CPU* aCPU=Root()->Widget(aName);
		if (aCPU)
		{
			aCPU->mFirstUpdate=false;
			aCPU->Initialize();

			Smart(MLObject) aO=new MLObject;
			aO->mType=ML_WIDGET;
			aO->mFlag=0;
			aO->mWidth=aCPU->mWidth;
			aO->mHeight=aCPU->mHeight;
			aO->mCPU=aCPU;

			mObjectList+=aO;
			aResultObject=aO;
		}
	}
	else if (aTag=="exe")
	{
		Root()->Exe(aParams);
	}
	else if (aTag=="/if") {}
	else
	{
		aResultObject=ParseCustomTag(aTag,aParams);
		if (aResultObject.IsNull())
		{
#ifdef _DEBUG
			gOut.Out("No object returned for <%s %s>",aTag.c(),aParams.c());
#endif
		}
	}

	if (aResultObject)
	{
		aResultObject->mMLRender=this;
		//if (aResultObject->mFlag&MLFLAG_TOUCHABLE) mTouchObjectList+=aResultObject;
		if (mFlags&MLFLAG_FLUFF) aResultObject->mFlag|=MLFLAG_FLUFF;
		aResultObject->mVAlign=mVAlign;
		if (mCurrentRecord && !(aResultObject->mFlag&(MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT))) mCurrentRecord->mObjectList+=aResultObject;
		if (mCurrentLink) {if (!(aResultObject->mFlag&(MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT))) {mCurrentLink->mObjectList+=aResultObject;aResultObject->mMLLink=mCurrentLink;}}
		aResultObject->mParams=aParams;
	}

	/*
	if (aResultObject.IsNotNull() && aParams.Len())
	{
		String aToken=aParams.GetToken(';');
		while (aToken.Len())
		{
			String aCommand=aToken.GetSegmentBefore('=');
			String aValue=aToken.GetSegmentAfter('=');

			if (aCommand=="ID") {aResultObject->mID=aValue;}
			else if (aCommand=="SIZE") 
			{
				if (aValue.Contains(','))
				{
					Point aSize=aValue.ToPoint();
					aResultObject->mWidth=aSize.mX;
					aResultObject->mHeight=aSize.mY;
				}
				else
				{
					mObjectList-=aResultObject;
					mPendingItemList+=aResultObject;
					aResultObject->mHookName=aValue;
				}
			}
			else if (aCommand=="SRC")
			{
				mObjectList-=aResultObject;
				mPendingItemList+=aResultObject;
				aResultObject->mHookName=aValue;
			}
			else if (aCommand=="WIDTH" || aCommand=="W") {aResultObject->mWidth=aValue.ToFloat();}
			else if (aCommand=="HEIGHT" || aCommand=="H") {aResultObject->mHeight=aValue.ToFloat();}
			else if (aCommand=="ZORDER" || aCommand=="Z") {aResultObject->mZOrder=_clamp(-100,aValue.ToInt(),100);}
			else if (aCommand=="EXT") {aResultObject->mExt=aValue;}

			aToken=aParams.GetNextToken(';');
		}
	}
	*/

	if (aResultObject && aResultObject->mType==ML_PAGE)
	{
		//
		// Now set up the <page></page> with the params...
		//
		Smart(MLPage) aP=aResultObject;

		aP->mMyMLRender.mWidth=aResultObject->mWidth;
		aP->mMyMLRender.mHeight=aResultObject->mHeight;

		aP->mMyMLRender.Format(theAccumulate);
		aP->mMyMLRender.mSetupParams=mPendingPageParams;

		//aP->mMyMLRender.Setup(mPendingPageParams);

		/*
		if (aP->mMyMLRender.mWidth==0) 
		{
			aP->mWidth=mPageRect.mWidth;
			aP->mMyMLRender.mWidth=mPageRect.mWidth;
			aP->mMyMLRender.mSizeWidthToContents=true;
		}
		if (aP->mMyMLRender.mHeight==0) aP->mMyMLRender.mHeight=0;
		*/

		mPendingPageParams="";

		//if (aP->mWidth==0) aP->mWidth=aP->mMLRender.mWidth;
		//if (aP->mHeight==0) aP->mHeight=aP->mMLRender.mHeight;
	}

	return "";
}

void MLRender::AddFont(String theFontName, Font* theFont)
{
	GetResources();
	Smart(FontStruct) aFS=new FontStruct;
	aFS->mName=theFontName;
	aFS->mFont=theFont;
	mResources->mFontList+=aFS;

	if (!mCurrentFont) mCurrentFont=theFont;
}

/*
void MLRender::AddNotify(String theNotifyName, CPU* theCPU)
{
	GetResources();
	Smart(NotifyStruct) aNS=new NotifyStruct;
	aNS->mName=theNotifyName;
	aNS->mCPU=theCPU;
	mResources->mNotifyList+=aNS;
}
*/


void MLRender::AddImage(String theImageName, Sprite* theSprite)
{
	GetResources();
	Smart(ImageStruct) aIS=new ImageStruct;
	aIS->mName=theImageName;
	aIS->mSprite=theSprite;
	mResources->mImageList+=aIS;
}

void MLRender::Rehup()
{
	// if mHeight==0, we're computing the height from the contents (i.e. this is a "page" or sub-MLRender)
	// Otherwise we don't change that stuff.
	//if (mSetupParams.Contains("1234")) gOut.Out("Rehup? [%s] -> %f",mSetupParams.c(),mHeight);


	FixBorders();

	if (Root()!=this)
	{
		mSizeWidthToContents=true;
		if (mSetupParams.Contains("size=") || mSetupParams.Contains("w=") || mSetupParams.Contains("width=")) mSizeWidthToContents=false;
	}
	
	Setup(mSetupParams);

/*
#ifdef _DEBUG
	if (!mMLRenderParent) gOut.Out("Rehup MLRender... [%s]->%s",ToString(),mPageRect.ToString());
	else gOut.Out("Rehup INNER MLRender... [%s]->%s",ToString(),mPageRect.ToString());
	gOut.mIndent+=4;
#endif
*/

	Point aCursor=mPageRect.UpperLeft();
	bool aNeedSort=false;

	ThreadLock();
	//
	// Pass #1: cleanup and prep
	//
	mObjectList-=mPendingItemList;
	EnumSmartList(MLObject,aO,mObjectList)
	{
		if (aO->mType==ML_ENDLINE)
		{
			mObjectList-=aO;
			EnumSmartListRewind(MLObject);
		}

		//
		// Fix sizes...
		//
		if (aO->mParams.Len())
		{
			String aToken=aO->mParams.GetToken(';');
			while (aToken.Len())
			{
				String aCommand=aToken.GetSegmentBefore('=');
				String aValue=aToken.GetSegmentAfter('=');

				if (aCommand=="ID") {aO->mID=aValue;}
				else if (aCommand=="SIZE") 
				{
					if (aValue.Contains(','))
					{
						String aP1=aValue.GetSegmentBefore(',');
						String aP2=aValue.GetSegmentAfter(',');
						aO->mWidth=MLStringToFloat(aP1);
						aO->mHeight=MLStringToFloat(aP2);
					}
					else
					{
						mObjectList-=aO;
						mPendingItemList+=aO;
						aO->mHookName=aValue;
					}
				}
				else if (aCommand=="SRC")
				{
					mObjectList-=aO;
					mPendingItemList+=aO;
					aO->mHookName=aValue;
				}
				else if (aCommand=="WIDTH" || aCommand=="W") {aO->mWidth=MLStringToFloat(aValue);}
				else if (aCommand=="HEIGHT" || aCommand=="H") {aO->mHeight=MLStringToFloat(aValue);if (aO->mType==ML_PAGE) {MLPage* aP=(MLPage*)aO.GetPointer();aP->mMyMLRender.mFixedHeight=true;}}
				else if (aCommand=="ZORDER" || aCommand=="Z") {aO->mZOrder=_clamp(-100,(int)MLStringToFloat(aValue),100);}
				else if (aCommand=="EXT") {aO->mExt+=aValue;}
				else if (aCommand=="OFFSET" || aCommand=="OFF") {aO->mOffset=aValue.ToPoint();}

				aToken=aO->mParams.GetNextToken(';');
			}
		}
	}
	Smart(MLObject) aEnd=new MLObject;
	aEnd->mType=ML_ENDLINE;
	aEnd->mFlag=MLFLAG_HIDDEN;
	mObjectList+=aEnd;

	//
	// Pass #2: Rehup childen
	//
	EnumSmartList(MLPage,aP,mObjectList) if (aP->mType==ML_PAGE)
	{
		if (aP->mMyMLRender.mWidth==0) aP->mMyMLRender.mWidth=mPageRect.mWidth;
		aP->mMyMLRender.Rehup();
		aP->mWidth=aP->mMyMLRender.mWidth;
		aP->mHeight=aP->mMyMLRender.mHeight;
	}

	//
	// Pass #3: Horizontal alignment
	//
	int aZOrder=-9999;
	EnumSmartList(MLObject,aO,mPendingItemList)
	{
		if (aZOrder==-9999) aZOrder=aO->mZOrder;
		if (aO->mZOrder!=aZOrder) aNeedSort=true; // Pending items might force a ZSort too...
	}

	Stack<Point> mCursorStack;

	float aOffsetX=0;
	for (int aCount=0;aCount<mObjectList.GetCount();aCount++)
	{
		Smart(MLObject) aO=mObjectList[aCount];

		/*
		if (aO->mType==ML_TEXT)
		{
			MLText* aT=(MLText*) aO.GetPointer();
			gOut.Out("Obj = %d @ %s = [%s]",aO->mType,aCursor.ToString(),aT->mText.c());
		}
		else gOut.Out("Obj = %d @ %s",aO->mType,aCursor.ToString());
		*/

		if (aO->mType==ML_PUSHCURSOR) {mCursorStack.Push(aCursor);}
		else if (aO->mType==ML_POPCURSOR) 
		{
			if (!mCursorStack.IsEmpty()) aCursor=mCursorStack.Pop();
			//
			// Endline to force alignment...
			//
			Smart(MLObject) aEnd=new MLObject;
			aEnd->mType=ML_ENDLINE;
			aEnd->mFlag=MLFLAG_HIDDEN;
			mObjectList.Insert(aEnd,aCount);
			aCount++;
		}
		else
		{
			//
			// if ANY Zorder changes happen, then we know we need to sort.
			//
			if (aZOrder==-9999) aZOrder=aO->mZOrder;
			if (aO->mZOrder!=aZOrder) aNeedSort=true;

			if (aO->mType==ML_BREAK) aCursor.mX=mPageRect.mX;
			else if (aO->mType==ML_OFFSET) {Smart(MLOffset) aOff=aO;aOffsetX=aOff->mOffset.mX;}
			else if (aO->mType!=ML_WORDSPACE && aCursor.mX+aO->mWidth>mPageRect.mX+mPageRect.mWidth)
			{
				aCursor.mX=mPageRect.mX;

				Smart(MLObject) aEnd=new MLObject;
				aEnd->mType=ML_ENDLINE;
				aEnd->mFlag=MLFLAG_HIDDEN;
				mObjectList.Insert(aEnd,aCount);
				aCount++;
			}
			float aWidth=aO->mWidth;

			if (aO->mType==ML_WORDSPACE)
			{
	//			if (aCount==0 || mObjectList[aCount-1]->mType!=ML_TEXT) 
				if (aCount==0 || GetNextSignificantObjectType(aCount,-1)!=ML_TEXT) 
				{
					aWidth=0;
					//aO->mFlag|=MLFLAG_HIDDEN;
				}
				else 
				{
					//aO->mFlag|=MLFLAG_HIDDEN;
					//aO->mFlag^=MLFLAG_HIDDEN;
				}
			}

			aO->mX=aCursor.mX+aOffsetX;
			aCursor.mX=aO->mX+aWidth;
			//mCursor.mX+=aWidth;
		}
	}
	//
	// Pass #4: Vertical alignment... anything between ENDLINES gets aligned up, tho...
	//
	float aMaxHeight=0;
	float aMaxWidth=0;
	float aTotalHeight=0;
	float aOffsetY=0;
	SmartList(MLObject) aCurrentLine;
	CleanArray<float> aOffsetYList;
	
	mCursorStack.Reset();
	int aAlign=-1; // Left align
	for (int aCount=0;aCount<mObjectList.GetCount();aCount++)
	{
		Smart(MLObject) aO=mObjectList[aCount];
		if (aO->mType==ML_ALIGN) {MLAlign* aMA=(MLAlign*)aO.GetPointer();aAlign=aMA->mAlign;}
		else if (aO->mType==ML_OFFSET) {Smart(MLOffset) aOff=aO;aOffsetY=aOff->mOffset.mY;}
		else if (aO->mType==ML_WORDSPACE)
		{
			//if (mObjectList[aCount+1]->mType!=ML_TEXT)
			if (GetNextSignificantObjectType(aCount,1)!=ML_TEXT)
			{
				//aO->mFlag|=MLFLAG_HIDDEN;
			}
			else
			{
				//aO->mFlag|=MLFLAG_HIDDEN;
				//aO->mFlag^=MLFLAG_HIDDEN;
			}
		}
		else if (aO->mType==ML_PUSHCURSOR) {mCursorStack.Push(aCursor);}
		else if (aO->mType==ML_POPCURSOR) 
		{
			if (!mCursorStack.IsEmpty()) aCursor=mCursorStack.Pop();
		}


		if (aO->mType==ML_BREAK) 
		{
			bool aInsignificant=true;
			EnumSmartList(MLObject,aOO,aCurrentLine) if (!(aOO->mFlag&(MLFLAG_INSIGNIFICANT|MLFLAG_HIDDEN))) aInsignificant=false;
			aCurrentLine+=aO;
			if (aInsignificant) aMaxHeight=_max(aMaxHeight,aO->mHeight);
		} // Break needs to register a height (of the font) that we get the vertical space...
		if (aO->mType==ML_ENDLINE || aO->mType==ML_BREAK)
		{
			float aTop=aCursor.mY;
			float aBottom=aCursor.mY+aMaxHeight;
			aTotalHeight=_max(aTotalHeight,aBottom);
			float aWidth=0;

			//
			// Wordspaces are not allowed at the beginning or end of lines!
			//
			Smart(MLObject) aLastO=aCurrentLine.FetchLast();if (aLastO) if (aLastO->mType==ML_WORDSPACE) aCurrentLine-=aLastO;
			Smart(MLObject) aFirstO=aCurrentLine[0];if (aFirstO) if (aFirstO->mType==ML_WORDSPACE) aCurrentLine-=aFirstO;

			//
			// End of the line...
			//
			int aCLCount=0;
			EnumSmartList(MLObject,aOO,aCurrentLine)
			{
				if (aOO->mFlag&MLFLAG_FLUFF)
				{
					aOO->mY=aTop;
					aOO->mHeight=aBottom-aTop;
				}
				else
				{
					if (aOO->mVAlign==-1) aOO->mY=aTop;
					else if (aOO->mVAlign==0) aOO->mY=aTop+(((aBottom-aTop)/2)-aOO->mHeight/2);
					else if (aOO->mVAlign==1) aOO->mY=aBottom-aOO->mHeight;
				}
				aOO->mY+=aOffsetYList[aCLCount++];
				aWidth+=aOO->mWidth;
				//aWidth=_max(aOO->LowerRight().mX,aWidth);
				aMaxWidth=_max(aOO->LowerRight().mX,aMaxWidth);
			}

			if (aAlign!=-1)
			{
				float aMove=((mPageRect.mWidth)-aWidth);
				if (aAlign==0) aMove/=2;
				EnumSmartList(MLObject,aOO,aCurrentLine) aOO->mX+=aMove;
			}

			aCurrentLine.Clear();
			aMaxHeight=0;
			aCursor.mY=aBottom;
			if (aO->mType==ML_BREAK) 
			{
				Smart(MLBreak) aB=aO;
				aCursor.mY+=aB->mExtraY;
			}
		}
		else 
		{
			if (!(aO->mFlag&MLFLAG_HIDDEN)) 
			{
				aOffsetYList[aCurrentLine.GetCount()]=aOffsetY;
				aCurrentLine+=aO;
				aMaxHeight=_max(aMaxHeight,aO->mHeight);

				if (aO->mType==ML_TEXT)
				{
					MLText* aMT=(MLText*)aO.GetPointer();
					float aBottom=aCursor.mY+aMaxHeight;
					aTotalHeight=_max(aTotalHeight,aBottom);

					if (mSetupParams.Contains("1234")) gOut.Out("Text: [%s] -> %f",aMT->mText.c(),aTotalHeight);
				}

				//float aBottom=aCursor.mY+aMaxHeight;
				//aTotalHeight=_max(aTotalHeight,aBottom);

			}
		}
	}

	//
	// Now we set the swipe height...
	//
	aTotalHeight+=mBorderBottom;
	SetSwipeHeight(aTotalHeight);
	//if (mHeight==0) mHeight=aTotalHeight;
	if (!mFixedHeight) 
	{
		mHeight=aTotalHeight;
		mHeight=_max(mHeight,mMinHeight);
	}

	//
	// If we are supposed to size our width to our contents, then this is where we adjust the width (we only do this once)
	//
	if (mSizeWidthToContents)
	{
		mWidth=aMaxWidth+mBorderRight;
		mSizeWidthToContents=false;
		SetSwipeWidth(aMaxWidth-1);
	}
	else 
	{
		if (aMaxWidth>mWidth) SetSwipeWidth((aMaxWidth+mBorderRight));
	}

	//
	// Post processing cleanup!
	// Here we take PageAlign into consideration (not if it scrolls, since if it scrolls, it'll match up to the scrollness)
	//
	if (mPageAlign!=-1 && !IsScrollableY())
	{
		//
		// Align everything with our page...
		//

		// THE PAGE ALIGN IS BROKEN!
		// It's not taking the border into consideration...
		// it needs to be added at the end-- check that!

		float aTop=9999999;
		float aBottom=-9999999;
		EnumSmartList(MLObject,aO,mObjectList) 
		{
			if (aO->mFlag&(MLFLAG_XHIDDEN|MLFLAG_HIDDEN|MLFLAG_INSIGNIFICANT)) continue;

			aTop=_min(aTop,aO->mY);
			aBottom=_max(aBottom,aO->mY+aO->mHeight);
		}

		if (mPageAlign!=-1)
		{
			float aMove=(mHeight-(mBorderBottom+mBorderTop))-(aBottom-aTop);
			if (mPageAlign==0) aMove/=2;

			aMove+=mBorderTop;
			EnumSmartList(MLObject,aO,mObjectList) aO->mY+=aMove;
		}
	}

	//
	// Clean up our records...
	//
	EnumSmartList(MLRecord,aR,mPendingRecordList)
	{
		bool aFirst=true;
		Rect aRect;
		EnumSmartList(MLObject,aO,aR->mObjectList)
		{
			if (aFirst) aRect=*aO;
			else aRect=aRect.Union(*aO);
			aFirst=false;
		}
		aR->Size(aRect);
		if (aR->mParams.Len())
		{
			String aToken=aR->mParams.GetToken(';');
			while (!aToken.IsNull())
			{
				String aCommand=aToken.GetSegmentBefore('=');
				String aValue=aToken.GetSegmentAfter('=');
				if (aCommand=="expand") aR->Size(aR->Expand(aValue.ToFloat()));
				if (aCommand=="expandh") aR->Size(aR->ExpandH(aValue.ToFloat()));
				if (aCommand=="expandv") aR->Size(aR->ExpandV(aValue.ToFloat()));
				if (aCommand=="expandup") aR->Size(aR->ExpandUp(aValue.ToFloat()));
				if (aCommand=="expanddown") aR->Size(aR->ExpandDown(aValue.ToFloat()));
				if (aCommand=="expandleft") aR->Size(aR->ExpandLeft(aValue.ToFloat()));
				if (aCommand=="expandright") aR->Size(aR->ExpandRight(aValue.ToFloat()));
				if (aCommand=="offset") aR->Size(aR->Translate(aValue.ToPoint()));
				aToken=aR->mParams.GetNextToken(';');
			}
		}
		mRecordList+=aR;
	}
	mPendingRecordList.Clear();

	EnumSmartList(MLObject,aO,mObjectList) if (aO->mCPU) AddCPU(aO->mCPU);

	//
	// Size/resize our links...
	//
	EnumSmartList(MLLink,aLink,mLinkList)
	{
		bool aFirst=true;
		EnumSmartList(MLObject,aO,aLink->mObjectList)
		{
			if (aFirst) aLink->Size(*aO);
			else aLink->Size(aLink->Union(*aO));
			aFirst=false;
		}
		aLink->mButton.Size(*aLink);
		aLink->mButton.SetNotify(this);
		if (CanScroll()) aLink->mButton.DragThru();
		else if (CanScrollH()) aLink->mButton.DragThruH();
		else if (CanScrollV()) aLink->mButton.DragThruV();
		*this+=&aLink->mButton;
	}

	//
	// Finishing anything that's due to be sized by a <record>...
	//
	FinishPendingItems();
	FinishedRehup();
	
	//
	// Move everything into the draw zone
	//
	ThreadLock_Draw();
	mDrawList.Clear();
	EnumSmartList(MLObject,aO,mObjectList) 
	{
		//if (aO->mType==ML_CUSTOM) gOut.Out("!!!! Got a custom??");
		if (aO->mCPU)
		{
			aO->mCPU->mX=aO->mX;
			aO->mCPU->mY=aO->mY;
			aO->mCPU->mWidth=aO->mWidth;
			aO->mCPU->mHeight=aO->mHeight;
		}
		if (!(aO->mFlag&(MLFLAG_HIDDEN))) mDrawList+=aO;
	}
	if (aNeedSort)
	{

		bool aDoSort=true;
		while (aDoSort)
		{
			aDoSort=false;
			for (int aCount=1;aCount<mDrawList.GetCount();aCount++)
			{
				Smart(MLObject) aO1=mDrawList[aCount-1];
				Smart(MLObject) aO2=mDrawList[aCount];

				if (aO2->mZOrder<aO1->mZOrder)
				{
					mDrawList.Swap(aCount-1,aCount);
					aDoSort=true;
				}
			}
		}
	}

	ThreadUnlock_Draw();
	ThreadUnlock();
	mRehup=false;
/*
#ifdef _DEBUG
	if (!mMLRenderParent) gOut.Out("Done Rehup MLRender... [%s]",ToString());
	else gOut.Out("Done Rehup INNER MLRender... [%s]",ToString());
	gOut.mIndent-=4;
#endif
*/

}

void MLText::Draw()
{
	mFont->Draw(mText,0,mHeight-1);
}

void MLCustom::Draw() 
{
	if (mMLRender) 
	{
		gG.PushTranslate();
		gG.PushClip();
		gG.PushColor();
		gG.PushClipColor();

		bool aHandled=false;
		if (MLRender::mGlobalDrawCustomHook) aHandled=MLRender::mGlobalDrawCustomHook(this);
		if (!aHandled) if (!mMLRender->Root()->DrawCustom(this)) if (mMLRender->Root()->mDrawCustomHook) mMLRender->Root()->mDrawCustomHook(this);
		gG.PopClipColor();
		gG.PopColor();
		gG.PopClip();
		gG.PopTranslate();

		mFirstDraw=false;
	}
}

void MLPage::Draw()
{
	return;
	gG.PushClip();
//	mMyMLRender.mX=0;
//	mMyMLRender.mY=0;
	gG.ClipInto(mMyMLRender.Translate(-mMyMLRender.UpperLeft()));
	mMyMLRender.DrawUnder();
	mMyMLRender.Draw();
	mMyMLRender.DrawOver();
	gG.PopClip();
}

int MLRender::GetNextSignificantObjectType(int theSlotNumber, int theDir)
{
	for (;;)
	{
		theSlotNumber+=theDir;
		Smart(MLObject) aO=mObjectList[theSlotNumber];
		if (aO.IsNull()) break;
		if (!(aO->mFlag&MLFLAG_INSIGNIFICANT)) return aO->mType;
	}

	return -1;
}

MLObject::~MLObject()
{
	if (mMLRender && mCPU) mMLRender->RemoveCPU(mCPU);

#ifdef _DEBUG
	gMLObjectCount--;
#endif
}

MLRender* MLRender::Root()
{
	if (mMLRenderParent) return mMLRenderParent->Root();
	return this;
}

CPU* MLRender::HitTest(float x, float y)
{
	if (mMLRenderParent) 
	{
		if (!IsScrollable())
		{
			CPU* aResult=SwipeBox::HitTest(x,y);
			if (aResult==this) return NULL;
			return aResult;	// If one of our children, that's who we hit...
		}
	}

	return SwipeBox::HitTest(x,y);	// Root behaves normally
}

void MLRender::Setup(String theParams)
{
	//gOut.Out("Setup params: %s",theParams.c());
	String aToken=theParams.GetToken(';');

	MLRender* aParent=mMLRenderParent;
	if (!aParent) aParent=this;

	//gOut.Out("Current PageRect: %s",aParent->mPageRect.ToString());

	while (aToken.Len())
	{
		String aCommand=aToken.GetSegmentBefore('=');
		String aValue=aToken.GetSegmentAfter('=');

		if (aCommand=="SIZE") 
		{
			if (aValue.Contains(','))
			{
				String aP1=aValue.GetSegmentBefore(',');
				String aP2=aValue.GetSegmentAfter(',');
				mWidth=MLStringToFloat(aP1,&aParent->mPageRect);
				mHeight=MLStringToFloat(aP2,&aParent->mPageRect);
			}
		}
		else if (aCommand=="WIDTH" || aCommand=="W") {mWidth=MLStringToFloat(aValue,&aParent->mPageRect);}
		else if (aCommand=="HEIGHT" || aCommand=="H") {mHeight=MLStringToFloat(aValue,&aParent->mPageRect);}
		else if (aCommand=="MINHEIGHT" || aCommand=="MINH") {mMinHeight=MLStringToFloat(aValue,&aParent->mPageRect);}
		else if (aCommand=="border") {mBorderTop=mBorderBottom=mBorderLeft=mBorderRight=aValue.ToFloat();FixBorders();}
		else if (aCommand=="vborder") {mBorderTop=mBorderBottom=aValue.ToFloat();FixBorders();}
		else if (aCommand=="hborder") {mBorderLeft=mBorderRight=aValue.ToFloat();FixBorders();}
		else if (aCommand=="pagealign" || aCommand=="valign") 
		{
			if (aValue=="top") mPageAlign=-1;
			if (aValue=="middle" || aValue=="center") mPageAlign=0;
			if (aValue=="bottom") mPageAlign=1;
		}
		else if (aCommand=="bkgcolor") mBkgColor.FromText(aValue);
		else if (aCommand=="clip") 
		{
			mClip=aValue.ToBool();
			if (!mClip) DisableFeature(CPU_CLIP);
		}

		aToken=theParams.GetNextToken(';');
	}

	mPageRect=Rect(mBorderLeft,mBorderTop,mWidth-(mBorderLeft+mBorderRight),mHeight-(mBorderTop+mBorderBottom));
}

void MLRender::FixBorders()
{
	AABBRect aPRect=AABBRect(0,0,mWidth,mHeight);

	aPRect.mX1+=mBorderLeft;
	aPRect.mX2-=mBorderRight;
	aPRect.mY1+=mBorderTop;
	aPRect.mY2-=mBorderBottom;
	mPageRect=aPRect.ToRect();
}

Rect MLRender::GetRecord(String theName)
{
	EnumSmartList(MLRecord,aR,mRecordList)
	{
		if (aR->mID==theName)
		{
			return *aR;
		}
	}

	EnumSmartList(MLPage,aP,mPageList)
	{
		Rect aResult=aP->mMyMLRender.GetRecord(theName);
		if (aResult.mWidth) return aResult.Translate(aP->UpperLeft());
	}

	return Rect(0,0,0,0);
}

void MLRender::FinishPendingItems()
{
	//EnumSmartList(MLRecord,aR,mRecordList) gOut.Out("Records: %s = %s",aR->mID.c(),aR->ToString());
	EnumSmartList(MLObject,aO,mPendingItemList)
	{
		if (aO->mHookName.Len())
		{
			Rect aR=GetRecord(aO->mHookName);
			if (aR.mWidth) 
			{
				aO->Size(aR);
				mObjectList+=aO;
			}
		}
	}
	//mPendingItemList.Clear(); // We don't clear it, we need it for rehup!
}

void MLRender::DoRehup()
{
	mRehup=true;
	if (mMLRenderParent) mMLRenderParent->DoRehup();
}

void MLImage::Draw()
{
	if (mSprite) mSprite->Draw(0,0);
	else if (mDLImage.IsNotNull())
	{
		if (mHeight>0)
		{
			Point aCenter=Point(this->mWidth/2,this->mHeight/2);
			Rect aRect=Rect(_min(mWidth,5),_min(mHeight,10));
			aRect.CenterAt(aCenter);

			gG.SetColor(.25f);
			gG.FillRect(aRect);
			gG.SetColor();

			float aMod=aRect.mHeight*(1.0f-mProgress);
			aRect.mY+=aMod;
			aRect.mHeight-=aMod;
			gG.FillRect(aRect);
		}
	}
}

void MLRender::Update()
{
	gCurrentMLRender=this;
	if (mUpdateObjectList.GetCount()) {EnumSmartList(MLObject,aM,mUpdateObjectList) {if (!aM->Update()) {mUpdateObjectList-=aM;EnumSmartListRewind(MLObject);mRehup=true;}}}
	if (mRehup) Rehup();
}

void MLRender::Notify(void* theData)
{
	gCurrentMLRender=this;
	EnumSmartList(MLLink,aLink,mLinkList)
	{
		if (theData==&aLink->mButton)
		{
			if (aLink->mBrowser) OS_Core::Execute(aLink->mID);
			else if (aLink->mID.Contains("://")) Load(aLink->mID);
			else 
			{
				if (aLink->mID=="_close") Kill();
				mCurrentLink=aLink;
				//if (aLink->mNotify) aLink->mNotify->Notify(this);
				//if (mCustomNotifyCPU) mCustomNotifyCPU->Notify(this);
				bool aHandled=false;

				if (GetResources()->mCustomNotifyFunction) aHandled=GetResources()->mCustomNotifyFunction(this);
				if (mGlobalNotifyHook) aHandled=MLRender::mGlobalNotifyHook(this);

				if (!aHandled) 
				{
					if (mGlobalClickHook) mGlobalClickHook(aLink->mID);
					if (GetResources()->mCustomClickFunction) GetResources()->mCustomClickFunction(aLink->mID);
					ClickLink(aLink->mID);
				}

				mCurrentLink=NULL;
			}
		}
	}
	gCurrentMLRender=NULL;
}

bool MLObject::IsDown() {if (!mMLLink) return false;return mMLLink->mButton.IsDown();}
bool MLObject::IsOver() {if (!mMLLink) return false;return mMLLink->mButton.IsOver();}

void MLRender::AddDefine(String theDefine, String theReplace)
{
	EnumSmartList(DefineStruct,aD,GetResources()->mDefineList)
	{
		if (aD->mDefine==theDefine) {aD->mReplaceWith=theReplace;return;}
	}
	Smart(DefineStruct) aDefine=new DefineStruct;
	aDefine->mDefine=theDefine;
	aDefine->mReplaceWith=theReplace;
	GetResources()->mDefineList+=aDefine;
}

float MLRender::MLStringToFloat(String theString, Rect* theRect)
{
	if (!theRect) theRect=&mPageRect;
	int aPercent=1;

	//
	// Special global defines happen here too...
	//
	if (theRect) theString.Replace("#width",Sprintf("%.0f",theRect->mWidth));

	while (aPercent>=0)
	{
		aPercent=theString.Find('%');
		if (aPercent>=0)
		{
			int aStep=aPercent-1;
			while (aStep>0 && IsNumber(theString[aStep],false)) aStep--;
			float aFix=theString.GetSegment(aStep,aPercent-aStep).ToFloat();

			if (aFix==0) aFix=theRect->mWidth;
			else aFix=theRect->mWidth*(aFix*.01f);

			theString.Delete(aStep,(aPercent-aStep)+1);
			theString.Insert(Sprintf("%.2f",aFix),aStep);
		}
	}

	return gMath.Eval(theString);
}

void MLRender::SetFlag(String theName, String theValue)
{
	EnumSmartList(FlagStruct,aF,GetResources()->mFlagList) if (aF->mName==theName) {aF->mValue=theValue;return;}
	Smart(FlagStruct) aF=new FlagStruct;
	aF->mName=theName;
	aF->mValue=theValue;
	GetResources()->mFlagList+=aF;
}

String MLRender::GetFlag(String theName)
{
	EnumSmartList(FlagStruct,aF,GetResources()->mFlagList) if (aF->mName==theName) return aF->mValue;
	return "";
}

MLObject* MLRender::GetObjectByID(String theID)
{
	EnumSmartList(MLObject,aO,mObjectList) if (aO->mID==theID) return aO.GetPointer();
	EnumSmartList(MLPage,aP,mPageList) 
	{
		MLObject* aO=aP->mMyMLRender.GetObjectByID(theID);
		if (aO) return aO;
	}
	return NULL;

}

void MLRender::GetAllObjectsByID(String theID, List& theList)
{
	EnumSmartList(MLObject,aO,mObjectList) if (aO->mID==theID) theList+=aO;
	EnumSmartList(MLPage,aP,mPageList) aP->mMyMLRender.GetAllObjectsByID(theID,theList);
}

void MLRender::GetAllObjectsStartingWithID(String theID, List& theList)
{
	EnumSmartList(MLObject,aO,mObjectList) if (aO->mID.StartsWith(theID)) theList+=aO;
	EnumSmartList(MLPage,aP,mPageList) aP->mMyMLRender.GetAllObjectsStartingWithID(theID,theList);
}


String MLRender::GetCurrentLinkID()
{
	if (mCurrentLink.IsNotNull()) return mCurrentLink->mID;
	return "";
}

String MLLink::GetParam(String theName)
{
	for (int aCount=0;aCount<mParamName.Size();aCount++) if (mParamName[aCount]==theName) return mParamValue[aCount];
	return "";
}

void MLBox::Initialize()
{
	if (mRenderRect.mWidth==0) mRenderRect=Rect(10,10,mWidth-20,mHeight-20);
	mRender.Size(mRenderRect);
	mRender.ExtraData()=this;
	*this+=&mRender;
}

void MLBox::Update()
{
	if (mRender.mKill)
	{
		mRender.mKill=false;
		Kill();
	}
}

void MLBox::Core_Draw()
{
	if (mFirstDraw) {Setup();mRender.LoadFromString(mContents);mContents="";}
	CPU::Core_Draw();
}
#endif
#endif //RAPT_ML_CPP
#endif
