#pragma once
#include "rapt.h"
#include "rapt_vertexformats.h"
//extern App *gAppPtr;

#ifndef LEGACY_GL

class VertexN
{
public:
	VertexN() {}
	VertexN(Vector thePos) {mPos=thePos;}
	VertexN(float x, float y, float z, float nx, float ny, float nz, float u, float v) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal.mX=nx;mNormal.mY=ny;mNormal.mZ=nz;mUV=Point(u,v);}
	VertexN(float x, float y, float z, float nx, float ny, float nz, Point uv) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal.mX=nx;mNormal.mY=ny;mNormal.mZ=nz;mUV=uv;}
	VertexN(float x, float y, float z, Vector theNormal, Point uv) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal=theNormal;mUV=uv;}
	VertexN(float x, float y, float z, Vector theNormal, float u, float v) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal=theNormal;mU=u;mV=v;}
	VertexN(Vector thePos, Vector theNormal, Point uv) {mPos=thePos;mNormal=theNormal,mUV=uv;}
	VertexN(const VertexN& theV) {mPos=theV.mPos;mNormal=theV.mNormal;mUV=theV.mUV;}
	VertexN(VertexKit* theKit, int theSlot) {Set(theKit,theSlot);}

	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
		if (theKit->mNormal.Size()>theSlot) mNormal=theKit->mNormal[theSlot];
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
	}


	//VertexN(aX,aY,0.0f,Vector(0,0,-1),aTU,aTV)

	inline VertexN& operator=(const VertexN& theV) {mPos=theV.mPos;mNormal=theV.mNormal;mUV=theV.mUV;return *this;}

	inline Vector ToVector() {return mPos;}

	union
	{
		Vector mPos;
		struct
		{
			float mX;
			float mY;
			float mZ;
		};
	};

	union
	{
		Vector mNormal;
		struct
		{
			float mNX;
			float mNY;
			float mNZ;
		};
	};
	union
	{
		Point mUV;
		struct
		{
			float mU;
			float mV;
		};
	};

	inline bool	PrettyClose(const VertexN& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mZ, theVertex.mZ) || !::PrettyClose(mNX, theVertex.mNX) || !::PrettyClose(mNY, theVertex.mNY) || !::PrettyClose(mNZ, theVertex.mNZ) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV)); }
	static void VertsToIndexedVerts(int theVCount, VertexN* theVertices, Array<VertexN>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<VertexN>(theVCount,theVertices,theResultVerts,theResultIndices);}
	inline VertexN Lerp(VertexN& theVertex, float theAmount) {return VertexN(mPos.Lerp(theVertex.mPos,theAmount),mNormal.LerpNormal(theVertex.mNormal,theAmount),mUV.Lerp(theVertex.mUV,theAmount));}
	void Flip() {mNormal*=-1;}
	inline void SetProgress(float theProgress) {}

	void Sync(SyncBuffer& theBuffer)
	{
		theBuffer.Sync(&mPos);
		theBuffer.Sync(&mNormal);
		theBuffer.Sync(&mUV);
	}

};


class ShaderN : public Shader
{
public:
	int					mLightDirRef=-1;
	int					mFogLimitRef=-1;
	Point				mFogLimit=Point(9999,9999);
	float				mPadding[4];	// Float padding to make sure dumping four floats to the shader works...

	virtual void		Load();

	inline int			CreateShape(Array<VertexN>& theV, Array<vbindex>& theI) {return Shader::CreateShape(theV,theV.Size(),theI);}

	int					StreamShape(VertexKit* theKit)
	{
		if (!theKit) return -1;
		Array<VertexN> aV;aV.GuaranteeSize(theKit->VCount());
		for (int aCount=0;aCount<aV.Size();aCount++)
		{
			aV[aCount].mPos=theKit->mPos[aCount];
			aV[aCount].mNormal=theKit->mNormal[aCount];
			aV[aCount].mUV=theKit->mUV[aCount];
		}
		return CreateShape(aV,theKit->mI);
	}

	/*
	int					StreamXShape(IOBuffer& theBuffer, Quad* theUVQuad=NULL) 
	{
		Array<VertexN> aV;aV.GuaranteeSize(theBuffer.ReadInt());
		if (!theUVQuad) {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();aV[aCount].mNormal=theBuffer.ReadVector();aV[aCount].mUV=theBuffer.ReadPoint();}}
		else {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();aV[aCount].mNormal=theBuffer.ReadVector();aV[aCount].mUV=theUVQuad->GetUV(theBuffer.ReadPoint());}}
		Array<vbindex> aI;aI.GuaranteeSize(theBuffer.ReadInt());
		for (int aCount=0;aCount<aI.Size();aCount++) {aI[aCount]=theBuffer.ReadVBIndex();}
		return CreateShape(aV,aI);
	}
	*/
};

class ShaderNColored : public ShaderN
{
public:
	void				Load();
};

class ShaderGizmo : public Shader
{
public:
	int					mCenterRef;
	Vector				mCenter;
	int					mRangeRef;
	Point				mRange;
	float				mPadding[4];

	void				Load();
};

class VertexS
{
public:
	VertexS() {}
	VertexS(Vector thePos) {mPos=thePos;}
	VertexS(const VertexS& theV) {mPos=theV.mPos;}
	VertexS(VertexKit* theKit, int theSlot) {Set(theKit,theSlot);}

	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
	}


	inline VertexS& operator=(const VertexS& theV) {mPos=theV.mPos;return *this;}
	inline Vector ToVector() {return mPos;}

	union
	{
		Vector mPos;
		struct
		{
			float mX;
			float mY;
			float mZ;
		};
	};

	inline bool	PrettyClose(const VertexS& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mZ, theVertex.mZ)); }
	static void VertsToIndexedVerts(int theVCount, VertexS* theVertices, Array<VertexS>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<VertexS>(theVCount,theVertices,theResultVerts,theResultIndices);}
	inline VertexS Lerp(VertexS& theVertex, float theAmount) {return VertexS(mPos.Lerp(theVertex.mPos,theAmount));}
	void Flip() {}
	inline void SetProgress(float theProgress) {}
};

class ShaderS : public Shader
{
public:
	void				Load();
	inline int			CreateShape(Array<VertexS>& theV, Array<vbindex>& theI) {return Shader::CreateShape(theV,theV.Size(),theI);}

	int					StreamShape(VertexKit* theKit)
	{
		if (!theKit) return -1;
		Array<VertexS> aV;aV.GuaranteeSize(theKit->VCount());
		for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theKit->mPos[aCount];}
		return CreateShape(aV,theKit->mI);
	}

	/*
	int					StreamXShape(IOBuffer& theBuffer, Quad* theUVQuad=NULL) 
	{
		Array<VertexS> aV;aV.GuaranteeSize(theBuffer.ReadInt());
		if (!theUVQuad) {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();theBuffer.ReadVector();theBuffer.ReadPoint();}}
		else {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();theBuffer.ReadVector();theUVQuad->GetUV(theBuffer.ReadPoint());}}
		Array<vbindex> aI;aI.GuaranteeSize(theBuffer.ReadInt());
		for (int aCount=0;aCount<aI.Size();aCount++) {aI[aCount]=theBuffer.ReadVBIndex();}
		return CreateShape(aV,aI);
	}
	*/

};

class Vertex
{
public:

	Vertex() {mPos=Vector(0,0,0);mColor=0xFFFFFFFF;mUV=Point(0,0);}
	Vertex(Vector thePos) {mPos=thePos;mColor=0xFFFFFFFF;mUV=Point(0,0);}
	Vertex(float x, float y, float z, unsigned int theColor, float u, float v) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mColor=theColor;mUV=Point(u,v);}
	Vertex(float x, float y, float z, unsigned int theColor, Point uv) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mColor=theColor;mUV=uv;}
	Vertex(Vector thePos, unsigned int theColor, Point uv) {mPos=thePos;mColor=theColor;mUV=uv;}
	Vertex(Vector thePos, unsigned int theColor, float u, float v) {mPos=thePos;mColor=theColor,mUV=Point(u,v);}
	Vertex(const Vertex& theV) {mPos=theV.mPos;mColor=theV.mColor;mU=theV.mU;mV=theV.mV;}
	Vertex(VertexKit* theKit, int theSlot) {Set(theKit,theSlot);}

	inline Vertex& operator=(const Vertex& theV) {mPos=theV.mPos;mColor=theV.mColor;mU=theV.mU;mV=theV.mV;return *this;}
	inline Vector ToVector() {return mPos;}

	inline void Set(Vector thePos, Point theUV, unsigned int theColor=0xFFFFFFFF) {mPos=thePos;mUV=theUV;mColor=theColor;}
	inline void Set(Vector thePos, float theU, float theV, unsigned int theColor=0xFFFFFFFF) {mPos=thePos;mU=theU;mV=theV;mColor=theColor;}
	inline void Set(float theX, float theY, float theZ, float theU, float theV, unsigned int theColor=0xFFFFFFFF) {mPos.mX=theX;mPos.mY=theY;mPos.mZ=theZ;mU=theU;mV=theV;mColor=theColor;}
	inline void Set(float theX, float theY, float theZ) {mPos.mX=theX;mPos.mY=theY;mPos.mZ=theZ;mU=0;mV=0;mColor=0xFFFFFFFF;}
	inline void Set(Vector thePos) {mPos=thePos;mU=0;mV=0;mColor=0xFFFFFFFF;}

	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
		//if (theKit->mNormal.Size()>theSlot) mNormal=theKit->mNormal[theSlot];
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}


	union
	{
		Vector mPos;
		struct
		{
			float mX;
			float mY;
			float mZ;
		};
	};

	union
	{
		struct
		{
			unsigned int mColor;
			// This causes problems on Linux, won't compile.
			// Doesn't like Point nested two unions deep.
			union
			{
				Point mUV;
				struct
				{
					float mU;
					float mV;
				};
			};
		};
		Vector mNormal; // Tacked on so we can use this as a POS/NORMAL pair if we don't need UV+Color
	};

	inline bool	PrettyClose(const Vertex& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mZ, theVertex.mZ) || !::PrettyClose(mColor,theVertex.mColor) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV)); }
	static void VertsToIndexedVerts(int theVCount, Vertex* theVertices, Array<Vertex>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<Vertex>(theVCount,theVertices,theResultVerts,theResultIndices);}
	static void ManualTransform(Vertex* theVectors, int theVectorCount, Matrix& theMatrix, Viewport& theViewport);

	inline Vertex Lerp(Vertex& theVertex, float theAmount) {return Vertex(mPos.Lerp(theVertex.mPos,theAmount),LerpRGBA(mColor,theVertex.mColor,theAmount).ToInt(),mUV.Lerp(theVertex.mUV,theAmount));}
	void Flip() {}
	inline void SetProgress(float theProgress) {}
};

class VertexC // Vertex that is colorized by gG.mColor only (no gradients, etc)
{
public:

	VertexC() {mPos=Vector(0,0,0);mUV=Point(0,0);}
	VertexC(Vector thePos) {mPos=thePos;mUV=Point(0,0);}
	VertexC(float x, float y, float z, float u, float v) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mUV=Point(u,v);}
	VertexC(float x, float y, float z, Point uv) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mUV=uv;}
	VertexC(Vector thePos, Point uv) {mPos=thePos;mUV=uv;}
	VertexC(Vector thePos, float u, float v) {mPos=thePos;mUV=Point(u,v);}
	VertexC(const VertexC& theV) {mPos=theV.mPos;mUV=theV.mUV;}
	VertexC(VertexKit* theKit, int theSlot) {Set(theKit,theSlot);}

	inline VertexC& operator=(const VertexC& theV) {mPos=theV.mPos;mUV=theV.mUV;return *this;}
	inline Vector ToVector() {return mPos;}

	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
		//if (theKit->mNormal.Size()>theSlot) mNormal=theKit->mNormal[theSlot];
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		//if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}


	union
	{
		Vector mPos;
		struct
		{
			float mX;
			float mY;
			float mZ;
		};
	};

	union
	{
		Point mUV;
		struct
		{
			float mU;
			float mV;
		};
	};

	inline bool	PrettyClose(const VertexC& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mZ, theVertex.mZ) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV)); }
	static void VertsToIndexedVerts(int theVCount, VertexC* theVertices, Array<VertexC>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<VertexC>(theVCount,theVertices,theResultVerts,theResultIndices);}
	static void ManualTransform(VertexC* theVectors, int theVectorCount, Matrix& theMatrix, Viewport& theViewport);

	inline VertexC Lerp(Vertex& theVertex, float theAmount) {return VertexC(mPos.Lerp(theVertex.mPos,theAmount),mUV.Lerp(theVertex.mUV,theAmount));}
	void Flip() {}
	inline void SetProgress(float theProgress) {}

};

class ShaderV : public Shader
{
public:
	void				Load();
	inline int			CreateShape(Array<Vertex>& theV, Array<vbindex>& theI) {return Shader::CreateShape(theV,theV.Size(),theI);}

#ifdef _DEBUG
	Array<Vertex> mDebugStreamV;
	Array<vbindex> mDebugStreamI;
#endif

	int					StreamShape(VertexKit* theKit)
	{
		if (!theKit) return -1;
		Array<Vertex> aV;aV.GuaranteeSize(theKit->VCount());
		for (int aCount=0;aCount<aV.Size();aCount++)
		{
			aV[aCount].mPos=theKit->mPos[aCount];
			aV[aCount].mUV=theKit->mUV[aCount];
		}
		return CreateShape(aV,theKit->mI);
	}

	/*
	int					StreamXShape(IOBuffer& theBuffer, Quad* theUVQuad=NULL)
	{
		Array<Vertex> aV;aV.GuaranteeSize(theBuffer.ReadInt());
		if (!theUVQuad) {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();theBuffer.ReadVector();aV[aCount].mUV=theBuffer.ReadPoint();}}
		else {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();theBuffer.ReadVector();aV[aCount].mUV=theUVQuad->GetUV(theBuffer.ReadPoint());}}
		Array<vbindex> aI;aI.GuaranteeSize(theBuffer.ReadInt());
		for (int aCount=0;aCount<aI.Size();aCount++) {aI[aCount]=theBuffer.ReadVBIndex();}

#ifdef _DEBUG
		mDebugStreamV.Copy(aV);
		mDebugStreamI.Copy(aI);
#endif

		return CreateShape(aV,aI);
	}
	*/

};

class ShaderC : public Shader
{
public:
	void				Load();
	inline int			CreateShape(Array<VertexC>& theV, Array<vbindex>& theI) {return Shader::CreateShape(theV,theV.Size(),theI);}

#ifdef _DEBUG
	Array<VertexC> mDebugStreamV;
	Array<vbindex> mDebugStreamI;
#endif

	int					StreamShape(VertexKit* theKit)
	{
		if (!theKit) return -1;
		Array<VertexC> aV;aV.GuaranteeSize(theKit->VCount());
		for (int aCount=0;aCount<aV.Size();aCount++)
		{
			aV[aCount].mPos=theKit->mPos[aCount];
			aV[aCount].mUV=theKit->mUV[aCount];
		}
		return CreateShape(aV,theKit->mI);
	}

	/*
	int					StreamXShape(IOBuffer& theBuffer, Quad* theUVQuad=NULL)
	{
		Array<VertexC> aV;aV.GuaranteeSize(theBuffer.ReadInt());
		if (!theUVQuad) {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();theBuffer.ReadVector();aV[aCount].mUV=theBuffer.ReadPoint();}}
		else {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();theBuffer.ReadVector();aV[aCount].mUV=theUVQuad->GetUV(theBuffer.ReadPoint());}}
		Array<vbindex> aI;aI.GuaranteeSize(theBuffer.ReadInt());
		for (int aCount=0;aCount<aI.Size();aCount++) {aI[aCount]=theBuffer.ReadVBIndex();}

#ifdef _DEBUG
		mDebugStreamV.Copy(aV);
		mDebugStreamI.Copy(aI);
#endif

		return CreateShape(aV,aI);
	}
	*/

};

class VertexMT
{
public:

	VertexMT() {mPos=Vector(0,0,0);mColor=0xFFFFFFFF;mUV=Point(0,0);mUV2=Point(0,0);}
	VertexMT(Vector thePos) {mPos=thePos;mColor=0xFFFFFFFF;mUV=Point(0,0);mUV2=Point(0,0);}
	VertexMT(float x, float y, float z, unsigned int theColor, float u, float v) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mColor=theColor;mUV=Point(u,v);mUV2=mUV;}
	VertexMT(float x, float y, float z, unsigned int theColor, float u, float v, float u2, float v2) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mColor=theColor;mUV=Point(u,v);mUV2=Point(u2,v2);}
	VertexMT(float x, float y, float z, unsigned int theColor, Point uv) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mColor=theColor;mUV=uv;mUV2=mUV;}
	VertexMT(float x, float y, float z, unsigned int theColor, Point uv, Point uv2) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mColor=theColor;mUV=uv;mUV2=uv2;}
	VertexMT(Vector thePos, unsigned int theColor, Point uv) {mPos=thePos;mColor=theColor,mUV=uv;mUV2=mUV;}
	VertexMT(Vector thePos, unsigned int theColor, Point uv, Point uv2) {mPos=thePos;mColor=theColor,mUV=uv;mUV2=uv2;}
	VertexMT(const VertexMT& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;mUV2=theV.mUV2;}
	VertexMT(VertexKit* theKit, int theSlot) {Set(theKit,theSlot);}

	inline VertexMT& operator=(const VertexMT& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;mUV2=theV.mUV2;return *this;}
	inline Vector ToVector() {return mPos;}

	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
		//if (theKit->mNormal.Size()>theSlot) mNormal=theKit->mNormal[theSlot];
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}


	union
	{
		Vector mPos;
		struct
		{
			float mX;
			float mY;
			float mZ;
		};
	};

	unsigned int	mColor;

	union
	{
		Point mUV;
		struct
		{
			float mU;
			float mV;
		};
	};

	union
	{
		Point mUV2;
		struct
		{
			float mU2;
			float mV2;
		};
	};

	inline bool	PrettyClose(const VertexMT& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mZ, theVertex.mZ) || !::PrettyClose(mColor,theVertex.mColor) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV) || !::PrettyClose(mU2, theVertex.mU2) || !::PrettyClose(mV2, theVertex.mV2)); }
	static void VertsToIndexedVerts(int theVCount, VertexMT* theVertices, Array<VertexMT>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<VertexMT>(theVCount,theVertices,theResultVerts,theResultIndices);}
	static void ManualTransform(VertexMT* theVectors, int theVectorCount, Matrix& theMatrix, Viewport& theViewport);
	inline VertexMT Lerp(VertexMT& theVertex, float theAmount) {return VertexMT(mPos.Lerp(theVertex.mPos,theAmount),LerpRGBA(mColor,theVertex.mColor,theAmount).ToInt(),mUV.Lerp(theVertex.mUV,theAmount),mUV2.Lerp(theVertex.mUV2,theAmount));}
	void Flip() {}
	inline void SetProgress(float theProgress) {}

};

class VertexNMT
{
public:

	VertexNMT() {mPos=Vector(0,0,0);mNormal=Vector(0,0,0);mUV=Point(0,0);mUV2=Point(0,0);}
	VertexNMT(Vector thePos) {mPos=thePos;mNormal=Vector(0,0,0);;mUV=Point(0,0);mUV2=Point(0,0);}
	VertexNMT(float x, float y, float z, Vector theNormal, float u, float v) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal=theNormal;mUV=Point(u,v);mUV2=mUV;}
	VertexNMT(float x, float y, float z, Vector theNormal, float u, float v, float u2, float v2) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal=theNormal;mUV=Point(u,v);mUV2=Point(u2,v2);}
	VertexNMT(float x, float y, float z, Vector theNormal, Point uv) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal=theNormal;mUV=uv;mUV2=mUV;}
	VertexNMT(float x, float y, float z, Vector theNormal, Point uv, Point uv2) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal=theNormal;mUV=uv;mUV2=uv2;}
	VertexNMT(Vector thePos, Vector theNormal, Point uv) {mPos=thePos;mNormal=theNormal;mUV=uv;mUV2=mUV;}
	VertexNMT(Vector thePos, Vector theNormal, Point uv, Point uv2) {mPos=thePos;mNormal=theNormal;mUV=uv;mUV2=uv2;}
	VertexNMT(const VertexNMT& theV) {mPos=theV.mPos;mNormal=theV.mNormal;mUV=theV.mUV;mUV2=theV.mUV2;}
	VertexNMT(VertexKit* theKit, int theSlot) {Set(theKit,theSlot);}

	inline VertexNMT& operator=(const VertexNMT& theV) {mPos=theV.mPos;mNormal=theV.mNormal;mUV=theV.mUV;mUV2=theV.mUV2;return *this;}
	inline Vector ToVector() {return mPos;}

	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
		if (theKit->mNormal.Size()>theSlot) mNormal=theKit->mNormal[theSlot];
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		//if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}


	union
	{
		Vector mPos;
		struct
		{
			float mX;
			float mY;
			float mZ;
		};
	};

	union
	{
		Vector mNormal;
		struct
		{
			float mNX;
			float mNY;
			float mNZ;
		};
	};

	union
	{
		Point mUV;
		struct
		{
			float mU;
			float mV;
		};
	};

	union
	{
		Point mUV2;
		struct
		{
			float mU2;
			float mV2;
		};
	};


	inline bool	PrettyClose(const VertexNMT& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mZ, theVertex.mZ) || !mNormal.PrettyClose(theVertex.mNormal) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV) || !::PrettyClose(mU2, theVertex.mU2) || !::PrettyClose(mV2, theVertex.mV2)); }
	static void VertsToIndexedVerts(int theVCount, VertexNMT* theVertices, Array<VertexNMT>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<VertexNMT>(theVCount,theVertices,theResultVerts,theResultIndices);}
	static void ManualTransform(VertexNMT* theVectors, int theVectorCount, Matrix& theMatrix, Viewport& theViewport);
	inline VertexNMT Lerp(VertexNMT& theVertex, float theAmount) {return VertexNMT(mPos.Lerp(theVertex.mPos,theAmount),mNormal.Lerp(theVertex.mNormal,theAmount),mUV.Lerp(theVertex.mUV,theAmount),mUV2.Lerp(theVertex.mUV2,theAmount));}
	void Flip() {}
	inline void SetProgress(float theProgress) {}

};

class ShaderVMT : public Shader
{
public:
	//
	// Draws with two textures, SetTexture for under texture, SetMultiTexture for over texture
	//
	void				Load();
	inline int			CreateShape(Array<VertexMT>& theV, Array<vbindex>& theI) {return Shader::CreateShape(theV,theV.Size(),theI);}

	int					StreamShape(VertexKit* theKit)
	{
		if (!theKit) return -1;
		Array<VertexMT> aV;aV.GuaranteeSize(theKit->VCount());
		for (int aCount=0;aCount<aV.Size();aCount++)
		{
			aV[aCount].mPos=theKit->mPos[aCount];
			aV[aCount].mUV=theKit->mUV[aCount];
		}
		return CreateShape(aV,theKit->mI);
	}

	/*
	int					StreamXShape(IOBuffer& theBuffer, Quad* theUVQuad=NULL)
	{
		Array<VertexMT> aV;aV.GuaranteeSize(theBuffer.ReadInt());
		if (!theUVQuad) {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();theBuffer.ReadVector();aV[aCount].mUV=theBuffer.ReadPoint();}}
		else {for (int aCount=0;aCount<aV.Size();aCount++) {aV[aCount].mPos=theBuffer.ReadVector();theBuffer.ReadVector();aV[aCount].mUV=theUVQuad->GetUV(theBuffer.ReadPoint());}}
		Array<vbindex> aI;aI.GuaranteeSize(theBuffer.ReadInt());
		for (int aCount=0;aCount<aI.Size();aCount++) {aI[aCount]=theBuffer.ReadVBIndex();}
		return CreateShape(aV,aI);
	}
	*/

};


class ShaderBillboard : public Shader
{
public:
	void				Load();
	void				Draw(Sprite& theSprite, Vector thePos, float theScale, bool oneSize=true);
	inline void			Draw(Sprite& theSprite, Vector thePos, bool oneSize=true) {Draw(theSprite,thePos,1.0f,oneSize);}
	void				Draw(Sprite& theSprite, float theScale=1.0f, bool oneSize=true) {Draw(theSprite,Vector(0,0,0),theScale,oneSize);}

	int					mInfoRef=-1;
	float				mInfo[4];

	float				mGlobalScalar=1.0f;
	void				SetGlobalScalar(float theScalar=1.0f) {mGlobalScalar=theScalar;}
};


/*
class ShaderRoundedRect : public Shader
{
public:
	int					mDimensionRef=-1;
	int					mRadiusRef=-1;

	void				Load();
	void				Draw(Vertex* theV, Point theDims, float theRadius);
	void				Draw(Vertex* theV, float theRadius);
	void				Draw(Rect theRect, float theRadius);
	void				Draw(Quad theRect, float theRadius);
	void				DrawShape(void* theVerts, int theVCount, vbindex* theI, int theICount);
};
*/

class ShaderCircle : public ShaderBillboard
{
public:
	void				Load();
	void				Draw(Vector thePos, float theSize, bool oneSize=true);
};

class ShaderGlowCircle : public ShaderCircle
{
public:
	void				Load();
	//void				Draw(Vector thePos, float theSize, bool oneSize=true);
};

class ShaderCircle3D : public Shader
{
public:
	void				Load();
	void				Draw(Quad3D theQuad);
};





/*
class ShaderRoundedRectInvert : public ShaderRoundedRect
{
public:
	void				Load();
};
*/


class ShaderEllipse : public Shader
{
public:
	int					mBorderRef=-1;
	int					mDimsRef=-1;
	float				mBorder;
	float				mDims[2];

	float				mPadding[4];	// Float padding to make sure dumping four floats to the shader works...

	void				Load();
	void				Draw(Vertex* theV, float theBorder=1.0f);
	void				Draw(Point thePos, Point theRadius, float theBorder=1.0f);
	inline void			Draw(Point thePos, float theRadius, float theBorder=1.0f) {Draw(thePos,Point(theRadius,theRadius),theBorder);}
	void				DrawShape(void* theVerts, int theVCount, vbindex* theI, int theICount);
};



class VertexNEX
{
public:
	VertexNEX() {}
	VertexNEX(Vector thePos, unsigned int theFlag=0) {mPos=thePos;mFlag=theFlag;}
	VertexNEX(float x, float y, float z, float nx, float ny, float nz, float u, float v, unsigned int theFlag=0) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal.mX=nx;mNormal.mY=ny;mNormal.mZ=nz;mUV=Point(u,v);mFlag=theFlag;}
	VertexNEX(float x, float y, float z, float nx, float ny, float nz, Point uv, unsigned int theFlag=0) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal.mX=nx;mNormal.mY=ny;mNormal.mZ=nz;mUV=uv;mFlag=theFlag;}
	VertexNEX(float x, float y, float z, Vector theNormal, Point uv, unsigned int theFlag=0) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal=theNormal;mUV=uv;mFlag=theFlag;}
	VertexNEX(float x, float y, float z, Vector theNormal, float u, float v, unsigned int theFlag=0) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mNormal=theNormal;mU=u;mV=v;mFlag=theFlag;}
	VertexNEX(Vector thePos, Vector theNormal, Point uv, unsigned int theFlag=0) {mPos=thePos;mNormal=theNormal,mUV=uv;mFlag=theFlag;}
	VertexNEX(const VertexNEX& theV) {mPos=theV.mPos;mNormal=theV.mNormal;mUV=theV.mUV;mFlag=theV.mFlag;}
	VertexNEX(VertexKit* theKit, int theSlot) {Set(theKit,theSlot);}

	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
		if (theKit->mNormal.Size()>theSlot) mNormal=theKit->mNormal[theSlot];
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		//if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}


	//VertexN(aX,aY,0.0f,Vector(0,0,-1),aTU,aTV)

	inline VertexNEX& operator=(const VertexNEX& theV) {mPos=theV.mPos;mNormal=theV.mNormal;mUV=theV.mUV;mFlag=theV.mFlag;return *this;}

	inline Vector ToVector() {return mPos;}

	union
	{
		Vector mPos;
		struct
		{
			float mX;
			float mY;
			float mZ;
		};
	};

	union
	{
		Vector mNormal;
		struct
		{
			float mNX;
			float mNY;
			float mNZ;
		};
	};
	union
	{
		Point mUV;
		struct
		{
			float mU;
			float mV;
		};
	};

	unsigned int mFlag=0; // Flag bits for internal processing...

	inline bool	PrettyClose(const VertexNEX& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mZ, theVertex.mZ) || !::PrettyClose(mNX, theVertex.mNX) || !::PrettyClose(mNY, theVertex.mNY) || !::PrettyClose(mNZ, theVertex.mNZ) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV) || !::PrettyClose(mFlag, theVertex.mFlag)); }
	inline bool	PrettyClose_PosNormal(const VertexNEX& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mZ, theVertex.mZ) || !::PrettyClose(mNX, theVertex.mNX) || !::PrettyClose(mNY, theVertex.mNY) || !::PrettyClose(mNZ, theVertex.mNZ)); }
	static void VertsToIndexedVerts(int theVCount, VertexNEX* theVertices, Array<VertexNEX>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<VertexNEX>(theVCount,theVertices,theResultVerts,theResultIndices);}
	static void	Transform(Array<VertexNEX>& theArray, Matrix& theMatrix, Matrix& theNormalMatrix)
	{
		for (int aCount=0;aCount<theArray.Size();aCount++)
		{
			Vector aPos=theMatrix.ProcessPoint3D(theArray[aCount].mPos);
			Vector aNorm=theNormalMatrix.ProcessPoint3D(theArray[aCount].mNormal);
			theArray[aCount].mPos=aPos;
			theArray[aCount].mNormal=aNorm;
		}
	}

	inline VertexNEX Lerp(VertexNEX& theVertex, float theAmount) {return VertexNEX(mPos.Lerp(theVertex.mPos,theAmount),mNormal.LerpNormal(theVertex.mNormal,theAmount),mUV.Lerp(theVertex.mUV,theAmount),mFlag|theVertex.mFlag);}
	void Flip() {mNormal*=-1;}
	inline void SetProgress(float theProgress) {}
};

class ShaderNEX : public Shader
{
public:
	int					mLightDirRef=-1;
	int					mFogLimitRef=-1;
	Point				mFogLimit=Point(9999,9999);
	float				mPadding[4];	// Float padding to make sure dumping four floats to the shader works...

	void				Load();
	inline int			CreateShape(Array<VertexNEX>& theV, Array<vbindex>& theI) {return Shader::CreateShape(theV,theV.Size(),theI);}
};

class ShaderDecalN : public ShaderN
{
public:
	int					mBiasRef=-1;
	float				mBias=1.0001f;
	float				mPadding[4];

	inline void			SetBias(float theBias=1.0001f) {mBias=theBias;}
	void				Load();
};

class ShaderDecalNEX : public ShaderDecalN
{
public:
	void				Load();
};

//class ShaderNEXStart : public ShaderNEX {public:void Load();};

class ShaderLine : public Shader
{
public:
	void				Load();
	void				Draw(Vector theStart, Vector theEnd, float theThickness=1.0f, float theThickness2=-1);
	inline void			Draw(Line3D theLine, float theThickness=1.0f, float theThickness2=-1) {Draw(theLine.mPos[0],theLine.mPos[1],theThickness,theThickness2);}
	void				Draw(Array<Line3D>& theList, float theThickness=1.0f);

	struct Kit
	{
		Array<VertexN>	mV;
		Array<vbindex>	mI;
		int				mVCount;
		int				mICount;
		float			mThickness;
		float			mThickness2;
	};

	Smart(Kit)			mBaseKit;

	void				Draw(Kit* theKit);
	Smart(Kit)			MakeKit(Array<Line3D>& theList, float theThickness=1.0f, float theThickness2=-1, bool isConnected=true, bool perpType=false, Smart(Kit) theKit=NULL);
	Smart(Kit)			MakeKit(Line3D& theLine, float theThickness=1.0f, float theThickness2=-1, bool isConnected=true, Smart(Kit) theKit=NULL);

	int					mInfoRef=-1;
	float				mInfo[4];
};

class ShaderLineGradient : public Shader
{
public:
	void				Load();
	void				Draw(Vector theStart, Vector theEnd, float theThickness=1.0f, float theThickness2=-1, Color theC1=Color(1), Color theC2=Color(1));
	inline void			Draw(Vector theStart, Vector theEnd, float theThickness, Color theC1, Color theC2) {Draw(theStart,theEnd,theThickness,theThickness,theC1,theC2);}
	inline void			Draw(Line3D theLine, float theThickness=1.0f, float theThickness2=-1,Color theC1=Color(1), Color theC2=Color(1)) {Draw(theLine.mPos[0],theLine.mPos[1],theThickness,theThickness2,theC1,theC2);}

	int					mInfoRef=-1;
	float				mInfo[2];
};


class ShaderRoundLine : public Shader
{
public:
	void				Load();
	void				Draw(Vector theStart, Vector theEnd, float theThickness=1.0f);
	void				DrawEX(Vector theStart, Vector theEnd, float theThickness=1.0f, unsigned char theEnds=0xFF);
	inline void			Draw(Line theLine, float theThickness=1.0f) {Draw(theLine.mPos[0],theLine.mPos[1],theThickness);}
	int					mInfoRef=-1;
	float				mInfo[4];
};

class ShaderDottedLine : public ShaderLine
{
public:
	void				Load();
	void				SetMarch(float theMarch=0.0f) {mInfo[3]=theMarch;}
	float				GetMarch() {return mInfo[3];}

	void				Draw(Vector theStart, Vector theEnd, float theThickness=1.0f, float theDotSize=0);
	void				Draw(Array<Line3D>& theList, float theThickness=1.0f, float theDotSize=0);
	inline void			Draw(Line3D theLine, float theThickness=1.0f, float theDotSize=1.0f) {Draw(theLine.mPos[0],theLine.mPos[1],theThickness,theDotSize);}
	void				Draw(Kit* theKit, float theDotSize=0);

};

class ShaderShowZ : public Shader
{
public:
	void				Load();
	void				Draw();
	void				Draw(int theZBuffer);
};

class Shader2D : public Shader
{
public:
	void				Load();
};

class Shader2DWhite : public Shader
{
public:
	void				Load();
};

class Shader2DGrey : public Shader
{
public:
	void				Load();
};

class Shader2DColorMix : public Shader
{
public:
	void				Load();
};


class ShaderRRPipeline : public Shader
{
public:
	void				Load();
};

class ShaderRRPipelineI : public ShaderRRPipeline
{
public:
	void				Load();
};

class ShaderRRPipelineT : public ShaderRRPipeline
{
public:
	void				Load();
};

class ShaderRRPipelineTI : public ShaderRRPipeline
{
public:
	void				Load();
};

class ShaderCirclePipeline : public Shader
{
public:
	void				Load();
};

class ShaderSpline3D : public Shader // Shader for drawing a line along a spline.  Will NOT do hard corners.
{
public:
	struct Kit
	{
		Array<Vx2DZ>	mV;
		Array<vbindex>	mI;
	};

	void				Load();
	void				Draw(Kit* theKit, float rounded=0.0f);
	Smart(Kit)			MakeKit(Array<Line3D>& theList, Array<Vector>& theNormals, float theThickness);
};

class GlobalShaders
{
public:
	ShaderN					mShaderN;
	ShaderNColored			mShaderNColored;
	ShaderGizmo				mShaderGizmo;
	ShaderS					mShaderS;
	ShaderV					mShaderV;
	ShaderC					mShaderC;
	Shader2DColorMix		mShader2DColorMix;
	ShaderVMT				mShaderVMT;
	ShaderBillboard			mShaderBillboard;
	//ShaderRoundedRect		mShaderRoundedRect;
	//ShaderRoundedRectInvert	mShaderRoundedRectInvert;
	ShaderNEX				mShaderNEX;
	ShaderDecalN			mShaderDecalN;
	ShaderDecalNEX			mShaderDecalNEX;
	ShaderEllipse			mShaderEllipse;
	ShaderLine				mShaderLine;
	ShaderLineGradient		mShaderLineGradient;
	ShaderRoundLine			mShaderRoundLine;
	ShaderCircle			mShaderCircle;
	ShaderCircle3D			mShaderCircle3D;
	ShaderDottedLine		mShaderDottedLine;
	ShaderGlowCircle		mShaderGlowCircle;
	ShaderShowZ				mShaderShowZ;
	Shader2D				mShader2D;
	Shader2DWhite			mShader2DWhite;
	Shader2DGrey			mShader2DGrey;
	ShaderRRPipeline		mShaderRRPipeline;
	ShaderRRPipelineI		mShaderRRPipelineI;
	ShaderRRPipelineT		mShaderRRPipelineT;
	ShaderRRPipelineTI		mShaderRRPipelineTI;
	ShaderCirclePipeline	mShaderCirclePipeline;
	ShaderSpline3D			mShaderSpline3D;

	inline void				Load()
	{
		//
		// Unsure!  I had a problem with shaders loading in another thread, but it doesn't seem to manifest now?
		// So I can uncomment this if I need to.  
		//

		/*
		mShaderN.Load();
		mShaderGizmo.Load();
		mShaderS.Load();
		mShaderV.Load();
		mShaderC.Load();
		mShaderVMT.Load();
		mShaderBillboard.Load();
		mShaderRoundedRect.Load();
		mShaderRoundedRectInvert.Load();
		mShaderNEX.Load();
		mShaderDecalN.Load();
		mShaderDecalNEX.Load();
		mShaderEllipse.Load();
		mShaderLine.Load();
		mShaderRoundLine.Load();
		mShaderCircle.Load();
		mShaderDottedLine.Load();
		mShaderGlowCircle.Load();
		mShaderShowZ.Load();
		mShader2D.Load();
		mShader2DWhite.Load();
		mShader2DGrey.Load();
		/**/
	};

#define SHLibrary_SeamlessNoise2D() \
		"vec2 modulo(vec2 divident, vec2 divisor){vec2 positiveDivident = mod(divident, divisor) + divisor;return mod(positiveDivident, divisor);}"	\
		"vec2 random(vec2 value){value = vec2( dot(value, vec2(127.1,311.7) ),dot(value, vec2(269.5,183.3) ) );return -1.0 + 2.0 * fract(sin(value) * 43758.5453123);}"	\
		"float SeamlessNoise2D(vec2 uv)" \
		"{" \
		"	vec2 _period=vec2(6.,6.);" \
		"	uv = uv * _period.x;" \
		"	vec2 cellsMinimum = floor(uv);" \
		"	vec2 cellsMaximum = ceil(uv);" \
		"	vec2 uv_fract = fract(uv);" \
		"	cellsMinimum = modulo(cellsMinimum, _period);" \
		"	cellsMaximum = modulo(cellsMaximum, _period);" \
		"	vec2 blur = smoothstep(0.0, 1.0, uv_fract);" \
		"	vec2 lowerLeftDirection = random(vec2(cellsMinimum.x, cellsMinimum.y));" \
		"	vec2 lowerRightDirection = random(vec2(cellsMaximum.x, cellsMinimum.y));" \
		"	vec2 upperLeftDirection = random(vec2(cellsMinimum.x, cellsMaximum.y));" \
		"	vec2 upperRightDirection = random(vec2(cellsMaximum.x, cellsMaximum.y));" \
		"	vec2 fraction = fract(uv);" \
		"	return mix( mix( dot( lowerLeftDirection, fraction - vec2(0, 0) ),dot( lowerRightDirection, fraction - vec2(1, 0) ), blur.x),mix( dot( upperLeftDirection, fraction - vec2(0, 1) ),	dot( upperRightDirection, fraction - vec2(1, 1) ), blur.x), blur.y) * 0.8 + 0.5;" \
		"}" \

};

extern GlobalShaders gSh;
#endif