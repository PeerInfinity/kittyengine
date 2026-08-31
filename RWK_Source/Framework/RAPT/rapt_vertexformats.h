#pragma once
//
// "Flexible" structure to stream vertices to various shaders.
//
struct VertexKit
{
	Array<Vector>			mPos;			// Vertex position
	Array<Vector>			mNormal;		// Vertex normal
	Array<Vector>			mUV;			// Vertex UV
	Array<int>				mDiffuse;		// Vertex color
	Array<unsigned int>		mFlags;			// Vertex flags

	Array<vbindex>	mI;

	inline void		Reset() {mPos.Reset();mNormal.Reset();mUV.Reset();mDiffuse.Reset();mI.Reset();mFlags.Reset();}
	inline void		FixUV(Quad* theQuad) {foreach(aUV,mUV) aUV=theQuad->GetUV(aUV);}
	inline int		GetVCount() {return _max(mDiffuse.Size(),_max(mUV.Size(),_max(mPos.Size(),mNormal.Size())));}
	inline int		VCount() {return GetVCount();}
};

class Vx2D // Actual 2D Vertex... Vertex2D with the z component is going to be phased out soon for GL30!
{
public:

	Vx2D() {mPos=Point(0,0);mColor=0xFFFFFFFF;mUV=Point(0,0);}
	Vx2D(Point thePos) {mPos=thePos;mColor=0xFFFFFFFF;mUV=Point(0,0);}
	Vx2D(float x, float y, unsigned int theColor, float u, float v) {mPos.mX=x;mPos.mY=y;mColor=theColor;mUV=Point(u,v);}
	Vx2D(float x, float y, unsigned int theColor, Point uv) {mPos.mX=x;mPos.mY=y;mColor=theColor;mUV=uv;}
	Vx2D(Point thePos, unsigned int theColor, Point uv) {mPos=thePos;mColor=theColor,mUV=uv;}
	Vx2D(Point thePos, unsigned int theColor, float u, float v) {mPos=thePos;mColor=theColor,mUV=Point(u,v);}
	Vx2D(const Vx2D& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;}
	Vx2D(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot].ToPoint();
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		if (theKit->mDiffuse.Size()>theSlot) mDiffuse=theKit->mDiffuse[theSlot];
	}

	inline Vx2D& operator=(const Vx2D& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;return *this;}
	inline Vector ToVector() {return mPos;}

	inline void Set(Point thePos, unsigned int theColor, Point theUV) {mPos=thePos;mColor=theColor;mUV=theUV;}
	inline void Set(float theX, float theY, unsigned int theColor, Point theUV) {mX=theX;mY=theY;mColor=theColor;mUV=theUV;}
	inline void Set(float theX, float theY, unsigned int theColor, float theU, float theV) {mX=theX;mY=theY;mColor=theColor;mU=theU;mV=theV;}
	inline void Set(Point thePos, unsigned int theColor, float theU, float theV) {mPos=thePos;mColor=theColor;mU=theU;mV=theV;}
	inline void Set(Point thePos, Point theUV) {mPos=thePos;mColor=0xFFFFFFFF;mUV=theUV;}
	inline void Set(float theX, float theY, Point theUV) {mX=theX;mY=theY;mColor=0xFFFFFFFF;mUV=theUV;}
	inline void Set(float theX, float theY, float theU, float theV) {mX=theX;mY=theY;mColor=0xFFFFFFFF;mU=theU;mV=theV;}
	inline void Set(Point thePos, float theU, float theV) {mPos=thePos;mColor=0xFFFFFFFF;mU=theU;mV=theV;}
	inline void Set(const Vx2D& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;}
	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot].ToPoint();
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		if (theKit->mDiffuse.Size()>theSlot) mDiffuse=theKit->mDiffuse[theSlot];
	}

	union
	{
		Point mPos;
		struct
		{
			float mX;
			float mY;
		};
	};

	union
	{
		unsigned int	mColor;
		unsigned int	mDiffuse;
	};

	union
	{
		Point mUV;
		struct {float mU;float mV;};
		struct {float mTextureU;float mTextureV;};
	};

	inline bool	PrettyClose(const Vx2D& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mColor,theVertex.mColor) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV)); }
	static void VertsToIndexedVerts(int theVCount, Vx2D* theVertices, Array<Vx2D>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<Vx2D>(theVCount,theVertices,theResultVerts,theResultIndices);}
	inline Vx2D Lerp(Vx2D& theVertex, float theAmount) {return Vx2D(mPos.Lerp(theVertex.mPos,theAmount),LerpRGBA(mColor,theVertex.mColor,theAmount).ToInt(),mUV.Lerp(theVertex.mUV,theAmount));}
	static void	Transform(Array<Vx2D>& theArray, Matrix& theMatrix, Matrix& theNormalMatrix) {for (int aCount=0;aCount<theArray.Size();aCount++) {Vector aPos=theMatrix.ProcessPoint3D(theArray[aCount].mPos);theArray[aCount].mPos=aPos;}}
	void Flip() {}inline void SetProgress(float theProgress) {}
};

class Vx2DEX // Actual 2D Vertex... Vertex2D with the z component is going to be phased out soon for GL30!
{
public:
	Vx2DEX() {mPos=Point(0,0);mColor=0xFFFFFFFF;mUV=Point(0,0);}
	Vx2DEX(Point thePos) {mPos=thePos;mColor=0xFFFFFFFF;mUV=Point(0,0);}
	Vx2DEX(float x, float y, unsigned int theColor, float u, float v) {mPos.mX=x;mPos.mY=y;mColor=theColor;mUV=Point(u,v);}
	Vx2DEX(float x, float y, unsigned int theColor, Point uv) {mPos.mX=x;mPos.mY=y;mColor=theColor;mUV=uv;}
	Vx2DEX(Point thePos, unsigned int theColor, Point uv) {mPos=thePos;mColor=theColor,mUV=uv;}
	Vx2DEX(Point thePos, unsigned int theColor, float u, float v) {mPos=thePos;mColor=theColor,mUV=Point(u,v);}
	Vx2DEX(float x, float y, unsigned int theColor, float u, float v, float e1, float e2, float e3) {mPos.mX=x;mPos.mY=y;mColor=theColor;mUV=Point(u,v);mExtra.mX=e1;mExtra.mY=e2;mExtra.mZ=e3;}
	Vx2DEX(float x, float y, unsigned int theColor, Point uv, Vector theExtra) {mPos.mX=x;mPos.mY=y;mColor=theColor;mUV=uv;mExtra=theExtra;}
	Vx2DEX(Point thePos, unsigned int theColor, Point uv, Vector theExtra) {mPos=thePos;mColor=theColor,mUV=uv;mExtra=theExtra;}
	Vx2DEX(Point thePos, unsigned int theColor, float u, float v, Vector theExtra) {mPos=thePos;mColor=theColor,mUV=Point(u,v);mExtra=theExtra;}
	Vx2DEX(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot].ToPoint();
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}

	Vx2DEX(const Vx2DEX& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;mExtra=theV.mExtra;}

	inline Vx2DEX& operator=(const Vx2DEX& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;return *this;}
	inline Vector ToVector() {return mPos;}

	inline void Set(Point thePos, unsigned int theColor, Point theUV) {mPos=thePos;mColor=theColor;mUV=theUV;}
	inline void Set(float theX, float theY, unsigned int theColor, Point theUV) {mX=theX;mY=theY;mColor=theColor;mUV=theUV;}
	inline void Set(float theX, float theY, unsigned int theColor, float theU, float theV) {mX=theX;mY=theY;mColor=theColor;mU=theU;mV=theV;}
	inline void Set(Point thePos, unsigned int theColor, float theU, float theV) {mPos=thePos;mColor=theColor;mU=theU;mV=theV;}
	inline void Set(Point thePos, Point theUV) {mPos=thePos;mColor=0xFFFFFFFF;mUV=theUV;}
	inline void Set(float theX, float theY, Point theUV) {mX=theX;mY=theY;mColor=0xFFFFFFFF;mUV=theUV;}
	inline void Set(float theX, float theY, float theU, float theV) {mX=theX;mY=theY;mColor=0xFFFFFFFF;mU=theU;mV=theV;}
	inline void Set(Point thePos, float theU, float theV) {mPos=thePos;mColor=0xFFFFFFFF;mU=theU;mV=theV;}

	inline void Set(Point thePos, unsigned int theColor, Point theUV, Vector theExtra) {mPos=thePos;mColor=theColor;mUV=theUV;mExtra=theExtra;}
	inline void Set(float theX, float theY, unsigned int theColor, Point theUV, Vector theExtra) {mX=theX;mY=theY;mColor=theColor;mUV=theUV;mExtra=theExtra;}
	inline void Set(float theX, float theY, unsigned int theColor, float theU, float theV, Vector theExtra) {mX=theX;mY=theY;mColor=theColor;mU=theU;mV=theV;mExtra=theExtra;}
	inline void Set(Point thePos, unsigned int theColor, float theU, float theV, Vector theExtra) {mPos=thePos;mColor=theColor;mU=theU;mV=theV;mExtra=theExtra;}
	inline void Set(Point thePos, Point theUV, Vector theExtra) {mPos=thePos;mColor=0xFFFFFFFF;mUV=theUV;mExtra=theExtra;}
	inline void Set(float theX, float theY, Point theUV, Vector theExtra) {mX=theX;mY=theY;mColor=0xFFFFFFFF;mUV=theUV;mExtra=theExtra;}
	inline void Set(float theX, float theY, float theU, float theV, Vector theExtra) {mX=theX;mY=theY;mColor=0xFFFFFFFF;mU=theU;mV=theV;mExtra=theExtra;}
	inline void Set(Point thePos, float theU, float theV, Vector theExtra) {mPos=thePos;mColor=0xFFFFFFFF;mU=theU;mV=theV;mExtra=theExtra;}

	inline void Set(const Vx2DEX& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;mExtra=theV.mExtra;}

	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot].ToPoint();
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}


	inline Vx2DEX& SetPos(Point thePos) {mPos=thePos;return *this;}
	inline Vx2DEX& SetPos(float x, float y) {mPos=Point(x,y);return *this;}
	inline Vx2DEX& SetColor(int theColor) {mColor=theColor;return *this;}
	inline Vx2DEX& SetColor(Color theColor) {mColor=theColor.ToInt();return *this;}
	inline Vx2DEX& SetUV(Point theUV) {mUV=theUV;return *this;}
	inline Vx2DEX& SetUV(float u, float v) {mUV=Point(u,v);return *this;}
	inline Vx2DEX& SetExtra(Vector theExtra) {mExtra=theExtra;return *this;}
	inline Vx2DEX& SetUV(float e1, float e2, float e3) {mExtra.mX=e1,mExtra.mY=e2;mExtra.mZ=e3;return *this;}

	union
	{
		Point mPos;
		struct {float mX;float mY;};
	};

	unsigned int	mColor;

	union
	{
		Point mUV;
		struct {float mU;float mV;};
	};

	Vector mExtra;


	inline bool	PrettyClose(const Vx2DEX& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mColor,theVertex.mColor) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV)); }
	static void VertsToIndexedVerts(int theVCount, Vx2DEX* theVertices, Array<Vx2DEX>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<Vx2DEX>(theVCount,theVertices,theResultVerts,theResultIndices);}
	inline Vx2DEX Lerp(Vx2DEX& theVertex, float theAmount) {return Vx2DEX(mPos.Lerp(theVertex.mPos,theAmount),LerpRGBA(mColor,theVertex.mColor,theAmount).ToInt(),mUV.Lerp(theVertex.mUV,theAmount));}
	static void	Transform(Array<Vx2DEX>& theArray, Matrix& theMatrix, Matrix& theNormalMatrix) {for (int aCount=0;aCount<theArray.Size();aCount++) {Vector aPos=theMatrix.ProcessPoint3D(theArray[aCount].mPos);theArray[aCount].mPos=aPos;}}
	void Flip() {}inline void SetProgress(float theProgress) {}
};


class Vx2DZ // A 2D Vertex with a Z component (used for 2D drawing where we have ZBuffer enabled)
{
public:

	Vx2DZ() {mPos=0;mColor=0xFFFFFFFF;mUV=Point(0,0);}
	Vx2DZ(Vector thePos) {mPos=thePos;mColor=0xFFFFFFFF;mUV=Point(0,0);}
	Vx2DZ(float x, float y, float z, unsigned int theColor, float u, float v) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mColor=theColor;mUV=Point(u,v);}
	Vx2DZ(float x, float y, float z, unsigned int theColor, Point uv) {mPos.mX=x;mPos.mY=y;mPos.mZ=z;mColor=theColor;mUV=uv;}
	Vx2DZ(Vector thePos, unsigned int theColor, Point uv) {mPos=thePos;mColor=theColor,mUV=uv;}
	Vx2DZ(Vector thePos, unsigned int theColor, float u, float v) {mPos=thePos;mColor=theColor,mUV=Point(u,v);}
	Vx2DZ(const Vx2DZ& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;}
	Vx2DZ(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}

	inline Vx2DZ& operator=(const Vx2DZ& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;return *this;}
	inline Vector ToVector() {return mPos;}

	inline void Set(Vector thePos, unsigned int theColor, Point theUV) {mPos=thePos;mColor=theColor;mUV=theUV;}
	inline void Set(float theX, float theY, float theZ, unsigned int theColor, Point theUV) {mX=theX;mY=theY;mZ=theZ;mColor=theColor;mUV=theUV;}
	inline void Set(float theX, float theY, float theZ, unsigned int theColor, float theU, float theV) {mX=theX;mY=theY;mZ=theZ;mColor=theColor;mU=theU;mV=theV;}
	inline void Set(Vector thePos, unsigned int theColor, float theU, float theV) {mPos=thePos;mColor=theColor;mU=theU;mV=theV;}
	inline void Set(Vector thePos, Point theUV) {mPos=thePos;mColor=0xFFFFFFFF;mUV=theUV;}
	inline void Set(float theX, float theY, float theZ, Point theUV) {mX=theX;mY=theY;mZ=theZ;mColor=0xFFFFFFFF;mUV=theUV;}
	inline void Set(float theX, float theY, float theZ, float theU, float theV) {mX=theX;mY=theY;mZ=theZ;mColor=0xFFFFFFFF;mU=theU;mV=theV;}
	inline void Set(Vector thePos, float theU, float theV) {mPos=thePos;mColor=0xFFFFFFFF;mU=theU;mV=theV;}
	inline void Set(const Vx2DZ& theV) {mPos=theV.mPos;mColor=theV.mColor;mUV=theV.mUV;}
	inline void Set(VertexKit* theKit, int theSlot)
	{
		if (theKit->mPos.Size()>theSlot) mPos=theKit->mPos[theSlot];
		if (theKit->mUV.Size()>theSlot) mUV=theKit->mUV[theSlot];
		if (theKit->mDiffuse.Size()>theSlot) mColor=theKit->mDiffuse[theSlot];
	}

	union
	{
		Vector mPos;
		struct {float mX;float mY;float mZ;};
	};

	union
	{
		unsigned int	mColor;
		float			mFData;
	};

	union
	{
		Point mUV;
		struct {float mU;float mV;};
	};

	inline bool	PrettyClose(const Vx2DZ& theVertex) { return !(!::PrettyClose(mPos.mX, theVertex.mX) || !::PrettyClose(mPos.mY, theVertex.mY) || !::PrettyClose(mPos.mZ, theVertex.mZ) || !::PrettyClose(mColor,theVertex.mColor) || !::PrettyClose(mU, theVertex.mU) || !::PrettyClose(mV, theVertex.mV)); }
	static void VertsToIndexedVerts(int theVCount, Vx2DZ* theVertices, Array<Vx2DZ>& theResultVerts, Array<vbindex>& theResultIndices) {::VertsToIndexedVerts<Vx2DZ>(theVCount,theVertices,theResultVerts,theResultIndices);}
	inline Vx2DZ Lerp(Vx2DZ& theVertex, float theAmount) {return Vx2DZ(mPos.Lerp(theVertex.mPos,theAmount),LerpRGBA(mColor,theVertex.mColor,theAmount).ToInt(),mUV.Lerp(theVertex.mUV,theAmount));}
	static void	Transform(Array<Vx2DZ>& theArray, Matrix& theMatrix, Matrix& theNormalMatrix) {for (int aCount=0;aCount<theArray.Size();aCount++) {Vector aPos=theMatrix.ProcessPoint3D(theArray[aCount].mPos);theArray[aCount].mPos=aPos;}}
	void Flip() {}inline void SetProgress(float theProgress) {}
};
