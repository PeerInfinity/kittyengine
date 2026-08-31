#pragma once

#include "os_headers.h"
#include "rapt_point.h"
#include "os_core.h"

//
// Matrix form:
//
//	00 10 20 30
//	01 11 21 31
//	02 12 22 32
//	03 13 23 33
//

#define m00 mData.m[0][0]
#define m01 mData.m[0][1]
#define m02 mData.m[0][2]
#define m03 mData.m[0][3]
#define m10 mData.m[1][0]
#define m11 mData.m[1][1]
#define m12 mData.m[1][2]
#define m13 mData.m[1][3]
#define m20 mData.m[2][0]
#define m21 mData.m[2][1]
#define m22 mData.m[2][2]
#define m23 mData.m[2][3]
#define m30 mData.m[3][0]
#define m31 mData.m[3][1]
#define m32 mData.m[3][2]
#define m33 mData.m[3][3]



struct Vertex2D;
struct Vertex2DN;
//struct Vertex2DMT;
//struct Vertex2DNMT;

class Quaternion 
{
public:
	Quaternion() {}
	Quaternion(float x, float y, float z, float w) {mX=x;mY=y;mZ=z;mW=w;}
	float mW=1.0f;
	float mX=0.0f;
	float mY=0.0f;
	float mZ=0.0f;
	Quaternion GetSlerp(Quaternion* theOther, float theAmount);
	Quaternion GetNormal();

	Quaternion operator+(Quaternion b)  { return Quaternion(mX + b.mX, mY + b.mY, mZ + b.mZ, mW + b.mW); }
	Quaternion operator*(float s) { return Quaternion(mX * s, mY * s, mZ * s, mW * s); }
	Quaternion operator*(Quaternion second)
	{
		Quaternion& first=*this;
		return Quaternion(
			second.mW*first.mX + second.mX*first.mW + second.mY*first.mZ - second.mZ*first.mY,
			second.mW*first.mY - second.mX*first.mZ + second.mY*first.mW + second.mZ*first.mX,
			second.mW*first.mZ + second.mX*first.mY - second.mY*first.mX + second.mZ*first.mW,
			second.mW*first.mW - second.mX*first.mX - second.mY*first.mY - second.mZ*first.mZ
		);
	}

	inline float Dot(Quaternion theOther)   {return mX * theOther.mX + mY * theOther.mY + mZ * theOther.mZ + mW * theOther.mW;}
	inline float Length()        { return sqrtf(Dot(*this)); }
	Quaternion Normalize()      { return *this * (1.0f / Length()); }
	Quaternion Conjugate()      { return Quaternion(-mX, -mY, -mZ, mW);}
	Quaternion Reciprocal(Quaternion q)     { return Conjugate() * (1.0f / Dot(q)); }
};

class Matrix
{
public:
	Matrix(void);
	Matrix(const Matrix &theMatrix) {*this=theMatrix;}
	Matrix(Vector vectorFrom, Vector vectorTo) {VectorToVector(vectorFrom,vectorTo);}

	inline static Matrix&	Go();

	Matrix&			Reset();
	inline Matrix&	Identity() {return Reset();}
	char*			ToString();
	inline Matrix&	Nullify() {m33=0.0f;return *this;}
	inline bool		IsNull() {return (m33==0);}
	inline void		Zero() {for (int aCount=0;aCount<16;aCount++) mData.mm[aCount]=0;}


	Matrix&			Invert();

	inline Matrix	GetInvert() {Matrix aMat=*this; aMat.Invert(); return aMat;}

	Matrix&			Multiply(Matrix *theMatrix);
	Matrix&			Subtract(Matrix *subtractThis);

	inline Matrix&	GetMatrix() {return *this;}

	inline Matrix&	UnTranslate() {mData.m[3][0]=0;mData.m[3][1]=0;mData.m[3][2]=0;return *this;}
	inline Matrix&	Untranslate() {return UnTranslate();}
	inline Matrix	GetUnTranslate() {Matrix aNew=*this;aNew.UnTranslate();return aNew;}
	inline Matrix	GetUntranslate() {return GetUnTranslate();}
	inline Matrix&	Translate(float theX, float theY, float theZ) {mData.m[3][0]+=theX;mData.m[3][1]+=theY;mData.m[3][2]+=theZ;return *this;}
	inline Matrix&	Translate(float theX, float theY) {mData.m[3][0]+=theX;mData.m[3][1]+=theY;return *this;}
	inline Matrix&	Translate(Point thePoint) {return Translate(thePoint.mX,thePoint.mY);}
	inline Matrix&	Translate(Vector thePoint) {return Translate(thePoint.mX,thePoint.mY,thePoint.mZ);}
	inline Matrix&	Normalize()
	{
		mData.m[3][0]=0;mData.m[3][1]=0;mData.m[3][2]=0;mData.m[3][3]=1;

		// Is this ia bad solution?
		// Can I normalize just the columns?
		//*
		Vector aC1=Vector(m00,m10,m20);aC1.Normalize();
		Vector aC2=Vector(m01,m11,m21);aC2.Normalize();
		Vector aC3=Vector(m02,m12,m22);aC3.Normalize();
		m00=aC1.mX;m10=aC1.mY;m20=aC1.mZ;
		m01=aC2.mX;m11=aC2.mY;m21=aC2.mZ;
		m02=aC3.mX;m12=aC3.mY;m22=aC3.mZ;
		/**/
		return *this;
	}
	inline Matrix&	NormalizeR()
	{
		Vector aC1=Vector(m00,m10,m20);aC1.Normalize();
		Vector aC2=Vector(m01,m11,m21);aC2.Normalize();
		Vector aC3=Vector(m02,m12,m22);aC3.Normalize();
		m00=aC1.mX;m10=aC1.mY;m20=aC1.mZ;
		m01=aC2.mX;m11=aC2.mY;m21=aC2.mZ;
		m02=aC3.mX;m12=aC3.mY;m22=aC3.mZ;

		return *this;
	}

	Matrix&			Scale(float theX, float theY, float theZ);
	inline Matrix&	Scale(float theX, float theY) {return Scale(theX,theY,1);}
	inline Matrix&	Scale(float theSize) {return Scale(theSize,theSize,theSize);}			
	inline Matrix&	Scale(Vector theScale) {return Scale(theScale.mX,theScale.mY,theScale.mZ);}

	Matrix			GetBridgeTo(Matrix* theOther); // Multiply this * result to get theOther

	Matrix&			RotateY(float degrees);
	Matrix&			RotateX(float degrees);
	Matrix&			RotateZ(float degrees);
	inline Matrix&	Rotate2D(float degrees) {return RotateZ(degrees);}

	Matrix&			SkewX(float degrees);
	Matrix&			SkewY(float degrees);
	Matrix&			SkewXZ(float degrees);
	Matrix&			SkewZX(float degrees);
	Matrix&			SkewZY(float degrees);

	Matrix&			RotateXYZ(float theX, float theY, float theZ);
	inline Matrix&	RotateXYZ(Vector theVector) {return RotateXYZ(theVector.mX,theVector.mY,theVector.mZ);}

	Matrix&			Transpose();

	inline Point	ProcessPoint(Point thePoint)
	{
		Point aResult;
		aResult.mX=mData.m[0][0]*thePoint.mX+mData.m[1][0]*thePoint.mY+mData.m[3][0];
		aResult.mY=mData.m[0][1]*thePoint.mX+mData.m[1][1]*thePoint.mY+mData.m[3][1];
		return aResult;
	}
	inline Point	Process(Point thePoint) {return ProcessPoint(thePoint);}

	void			ProcessPoints(Point *thePointPtr, int theCount);
	void			ProcessPoints(Vertex2D *theVertexPtr, int theCount);
	void			ProcessPoints(Vertex2DN *theVertexPtr, int theCount);
	void			ProcessPoints(Vertex2DMT *theVertexPtr, int theCount);
	void			ProcessPoints(Vector *theVectorPtr, int theCount);
	void			ProcessPoints(Vertex2D *theInput, Vertex2D* theOutput, int thePoints);
	void			ProcessPoints(Vertex2DN *theInput, Vertex2DN* theOutput, int thePoints);

	void			ProcessVectorXY(Vector *theVectorPtr, int theCount);
	Vector			ProcessVectorXY(Vector& theVector);
	void			ProcessVertexXY(Vertex2D *theVectorPtr, int theCount);

	inline void		ProcessPoints(Array<Vector>& in) {ProcessPoints(in.mArray,in.Size());}
	void			ProcessPoints(Array<Vector>& in,Array<Vector>& out);
	void			ProcessPoints(Array<Vertex2D>& in,Array<Vertex2D>& out);
	void			ProcessPoints(Array<Vertex2DN>& in,Array<Vertex2DN>& out);


	Vector			ProcessPointNormalized(Vector theVector);
	Vector			ProcessPointN(Vector theVector); // Processes, but no translate
	Vector			ProcessPointN(float x, float y, float z); // Processes, but no translate

	inline void		Process(Vector& theV)
	{
		float aX=theV.mX;
		float aY=theV.mY;
		float aZ=theV.mZ;
		theV.mX=m00*aX+m10*aY+m20*aZ+m30;
		theV.mY=m01*aX+m11*aY+m21*aZ+m31;
		theV.mZ=m02*aX+m12*aY+m22*aZ+m32;
	}

	inline void		Process(Vector& theV, Vector& theNormal)
	{
		float aX=theV.mX;
		float aY=theV.mY;
		float aZ=theV.mZ;
		theV.mX=m00*aX+m10*aY+m20*aZ+m30;
		theV.mY=m01*aX+m11*aY+m21*aZ+m31;
		theV.mZ=m02*aX+m12*aY+m22*aZ+m32;

		theNormal=ProcessPointN(theNormal.mX,theNormal.mY,theNormal.mZ);

		/*
		float aNX=theNormal.mX;
		float aNY=theNormal.mY;
		float aNZ=theNormal.mZ;
		float aW=1.0f/(m03*theNormal.mX+m13*theNormal.mY+m23*theNormal.mZ+m33);

		theNormal.mX=(m00*aNX+m10*aNY+m20*aNZ+m30)*aW;
		theNormal.mY=(m01*aNX+m11*aNY+m21*aNZ+m31)*aW;
		theNormal.mZ=(m02*aNX+m12*aNY+m22*aNZ+m32)*aW;
		*/
	}

	Quaternion		ToQuaternion();
	void			FromQuaternion(Quaternion* theSource);
	
	float			Determinant(void);


	//
	// These are helpers to let us know if the matrix is
	// rotated, or otherwise offset in a way that requires
	// filtering.  I implemented this so that I can make 
	// images that are not moved around draw unfiltered.
	//
	bool			IsRotatedOrScaled();
	bool			IsFractionalTranslate();

	//
	// This allows us to do hardcore editing of the matrix
	//
	Vector			GetTranslate();
	void			SetTranslate(float theX, float theY, float theZ);
	inline void		SetTranslate(Vector thePos) {SetTranslate(thePos.mX,thePos.mY,thePos.mZ);}

	//
	// Math operators
	//
	//inline void operator=(Matrix *theMatrix) {mData=theMatrix->mData;}
	//inline void operator=(Matrix theMatrix)  {mData=theMatrix.mData;}
	inline void	operator*=(Matrix* theMatrix) {Multiply(theMatrix);}
	inline void	operator*=(Matrix theMatrix)  {Multiply(&theMatrix);}

	//
	// 3D Functions...
	//
	inline Vector	ProcessPoint3D(Vector theVector)
	{
		Vector aResult;
		aResult.mX=mData.m[0][0]*theVector.mX+mData.m[1][0]*theVector.mY+mData.m[2][0]*theVector.mZ+mData.m[3][0];
		aResult.mY=mData.m[0][1]*theVector.mX+mData.m[1][1]*theVector.mY+mData.m[2][1]*theVector.mZ+mData.m[3][1];
		aResult.mZ=mData.m[0][2]*theVector.mX+mData.m[1][2]*theVector.mY+mData.m[2][2]*theVector.mZ+mData.m[3][2];
		return aResult;
	}
	inline VectorW ProcessPoint(VectorW theVector)
	{
		VectorW aResult;
		aResult.mX=mData.m[0][0]*theVector.mX+mData.m[1][0]*theVector.mY+mData.m[2][0]*theVector.mZ+mData.m[3][0]*theVector.mW;
		aResult.mY=mData.m[0][1]*theVector.mX+mData.m[1][1]*theVector.mY+mData.m[2][1]*theVector.mZ+mData.m[3][1]*theVector.mW;
		aResult.mZ=mData.m[0][2]*theVector.mX+mData.m[1][2]*theVector.mY+mData.m[2][2]*theVector.mZ+mData.m[3][2]*theVector.mW;
		aResult.mW=theVector.mX*mData.m[0][3]+theVector.mY*mData.m[1][3]+theVector.mZ*mData.m[2][3]+theVector.mW*mData.m[3][3];
		return aResult;
	}
	inline Vector	ProcessPoint(float x=0, float y=0, float z=0)
	{
		Vector aResult;
		aResult.mX=mData.m[0][0]*x+mData.m[1][0]*y+mData.m[2][0]*z+mData.m[3][0];
		aResult.mY=mData.m[0][1]*x+mData.m[1][1]*y+mData.m[2][1]*z+mData.m[3][1];
		aResult.mZ=mData.m[0][2]*x+mData.m[1][2]*y+mData.m[2][2]*z+mData.m[3][2];
		return aResult;
	}


	inline Vector	ProcessPoint(Vector theVector) {return ProcessPoint3D(theVector);}
	Matrix&			RotateAroundAxis3D(Vector theVector, float theAngle);
	Matrix&			Perspective(float theFieldOfView, float theAspectRatio, float zNear, float zFar);
	Matrix&			Perspective();
	Matrix&			Planify(Plane thePlane, float theScale=1.0f);

	Matrix&			LookAt(Vector theLookat, Vector theCamera, Vector theUpVector);
	Matrix&			PointAt(Vector thePointDir, Vector theUpVector=Vector(0,0,-1));
	Matrix&			VectorToVector(Vector theV1, Vector theV2, bool doNormalize=true); // Gives a rotation matrix to go from one vector to the other
	inline Matrix&  RotateVectorToVector(Vector theV1, Vector theV2) {return VectorToVector(theV1,theV2);}
	inline Matrix&	Align(Vector myVector, Vector alignTo) {return VectorToVector(myVector,alignTo);}
	Matrix&			VectorToVector(Vector theV1, Vector theV2, Vector theUp); // Gives a rotation matrix to go from one vector to the other

	Matrix&			Interpolate(Matrix& theDestination,float theProgress);
	Matrix&			GetInterpolate(Matrix& theDestination,float theProgress);
	inline Matrix&	Blend(Matrix& theDestination,float theProgress) {return Interpolate(theDestination,theProgress);}
	inline Matrix&	GetBlend(Matrix& theDestination,float theProgress) {return GetInterpolate(theDestination,theProgress);}
	Matrix&			Slerp(Matrix& theDestination,float theProgress); // Tries to make the rotation morph better
	Matrix			GetSlerp(Matrix& theDestination,float theProgress);

	Matrix			Billboard();

	//Vector			GetRow(int theCount);

	void			MakeOrthographic(float theLeft, float theRight, float theTop, float theBottom, float theNear, float theFar);
	void			MakePerspective(float theLeft, float theRight, float theTop, float theBottom, float theNear, float theFar);

public:

	typedef struct CoreMatrix 
	{
		union {
			struct 
			{
				float        _11, _12, _13, _14;
				float        _21, _22, _23, _24;
				float        _31, _32, _33, _34;
				float        _41, _42, _43, _44;
			};
			float m[4][4];
			float mm[16];
		};
	} CoreMatrix;
	CoreMatrix		mData;

	inline float*	FloatPtr() {return mData.mm;}

#ifdef _DEBUG
	inline bool		IsNAN() {for (int aCount=0;aCount<16;aCount++) if (mData.mm[aCount]!=mData.mm[aCount]) return true;return false;}
#endif

	//
	// Get a column as a vector...
	// For DrawMatrix, Column(0) = to the right
	// 	               Column(1) = up
	//				   Column(2) = into the screen
	//
	Vector Column(int theColumn) {return Vector(mData.m[0][theColumn],mData.m[1][theColumn],mData.m[2][theColumn]);}
	Vector Row(int theColumn) {return Vector(mData.m[theColumn][0],mData.m[theColumn][1],mData.m[theColumn][2]);}
	inline Vector GetColumn(int theColumn) {return Column(theColumn);}
	inline Vector GetRow(int theRow) {return Row(theRow);}
	void SetColumn(int theColumn, Vector theVector) {mData.m[0][theColumn]=theVector.mX;mData.m[1][theColumn]=theVector.mY;mData.m[2][theColumn]=theVector.mZ;}
	void SetRow(int theRow, Vector theVector) {mData.m[theRow][0]=theVector.mX;mData.m[theRow][1]=theVector.mY;mData.m[theRow][2]=theVector.mZ;}

	Vector RightVector() {return Column(0);}
	Vector UpVector() {return Column(1);}
	Vector AwayVector() {return Column(2);}
	//
	// Some extra, crazy operators.
	// Dump dumps out a statement containing the contents of the matrix in a format
	// that can be used in a game to set the matrix up without having to do multiplies
	// and whatnot.
	//
	// Make is just that function, it just lets you pass 16 params to the matrix to 
	// create a matrix in a certain state.
	//
	void			DumpMakeCommand(char* theFilename);
	void			Debug(char* thePrefix="");
	void			DebugOneLine(char* thePrefix="");
	//char*			ToString();

	Vector			DecomposeR()
	{
		Vector aResult;
		aResult.mX=atan2f(mData.m[1][2],mData.m[2][2]);
		aResult.mY=atan2f(-mData.m[0][2],sqrtf(mData.m[1][2]*mData.m[1][2]+mData.m[2][2]*mData.m[2][2]));
		aResult.mZ=atan2f(mData.m[0][1],mData.m[0][0]);
		return aResult;
	}

	void			ToVectors(Vector& theXDir, Vector& theYDir, Vector& theZDir);
	void			FromVectors(Vector& theXDir, Vector& theYDir, Vector& theZDir);

};


inline Matrix& Identity() {static Matrix aMat;return aMat.Identity();}
inline Matrix& IdentityMatrix() {static Matrix aMat;return aMat.Identity();}
inline Matrix& NullMatrix() {static Matrix aMat;return aMat.Nullify();}
inline Matrix& VectorToVectorMatrix(Vector theV1, Vector theV2) {static Matrix aMat;aMat.Identity();aMat.VectorToVector(theV1,theV2);return aMat;}
inline Matrix& PointAtMatrix(Vector theV1, Vector theV2) {return VectorToVectorMatrix(theV1,theV2);}
inline Matrix& AxisToMatrix(Vector theX, Vector theY, Vector theZ) {static Matrix aMat;aMat.Identity();aMat.FromVectors(theX,theY,theZ);return aMat;}
inline Matrix& VectorToMatrix(Vector theX, Vector theY, Vector theZ) {return AxisToMatrix(theX,theY,theZ);}
inline Matrix& QuickMatrix() {return Identity();}
inline Matrix& TempMatrix() {return Identity();}
inline Matrix& NewMatrix() {return Identity();}
inline Matrix& MakeMatrix() {return Identity();}
inline Matrix& Mat() {return Identity();}
Matrix& Matrix::Go() {return IdentityMatrix();}
inline Matrix& DifferenceMatrix(Matrix* theM1, Matrix* theM2) {static Matrix aMat;for (int aCount=0;aCount<16;aCount++) aMat.mData.mm[aCount]=theM1->mData.mm[aCount]-theM2->mData.mm[aCount];return aMat;}

#undef m00
#undef m01
#undef m02
#undef m03
#undef m10
#undef m11
#undef m12
#undef m13
#undef m20
#undef m21
#undef m22
#undef m23
#undef m30
#undef m31
#undef m32
#undef m33

//
// Matrix kit for common 3d stuff, that stashes the inverse and normalized version
// for quick/easy access (put in for Hamsterball, which needed access to these quickly)
//
/*
class MatrixKit : public Matrix
{
public:

	MatrixKit(void) : Matrix() {}
	MatrixKit(const Matrix &theMatrix) : Matrix(theMatrix) {}
	MatrixKit(Vector vectorFrom, Vector vectorTo) : Matrix(vectorFrom, vectorTo) {}

	Matrix			mInverse;
	Matrix			mNormal;

	inline Matrix&	Inverse() {return mInverse;}
	inline Matrix&	Normal() {return mNormal;}

	inline void		Reset() {Matrix::Reset();mInverse.Reset();mNormal.Reset();}

	inline void		Rehup(unsigned char theFlags=0xFF)
	{
		if (theFlags&0x01) mInverse=GetInvert();
		if (theFlags&0x02) {mNormal=*this;mNormal.Normalize();}
	}
};
/**/

//*

#define ALWAYSREHUP mRehup=0xFF
//*
#undef ALWAYSREHUP
#define ALWAYSREHUP
/**/
class MatrixKit
{
public:

	Matrix			mMat;
	Matrix			mInverse;
	Matrix			mNormal;

	Matrix			_mMat;
	Matrix			_mInverse;
	Matrix			_mNormal;

	MatrixKit*			mParent=NULL;
	Array<MatrixKit*>	mChild;
	inline void			SetParent(MatrixKit* theKit) {mParent=theKit;if (theKit) mParent->mChild+=this;}

	enum
	{
		REHUP_I = 0x01,
		REHUP_N= 0x02,
		REHUP_PARENT=0x04,
		REHUP_PARENTI=0x08,
		REHUP_PARENTN=0x10,
	};

	enum
	{
		HARD_NORMAL=0x01,
		HARD_INVERSE=0x02,
	};

	unsigned char		mRehup=0;

	Matrix ParentalizeM()
	{
		if (!mParent) return mMat;
		//Matrix aResult=mParent->ParentalizeM();
		//aResult*=mMat;
		Matrix aResult=mMat;
		aResult*=mParent->ParentalizeM();

		return aResult;
	}

	Matrix ParentalizeN()
	{
		if (!mParent) return mNormal;
		//Matrix aResult=mParent->ParentalizeN();
		//aResult*=mNormal;
		if (mRehup&REHUP_N) {mNormal=mMat;mNormal.Normalize();UNSETBITS(mRehup,REHUP_N);}
		Matrix aResult=mNormal;
		aResult*=mParent->ParentalizeN();

		return aResult;
	}

	inline Matrix&	GetMatrix()
	{
		ALWAYSREHUP;
		if (!mParent) return mMat;
		if (mRehup&REHUP_PARENT) {_mMat=ParentalizeM();UNSETBITS(mRehup,REHUP_PARENT);}
		return _mMat;
	}

	/*
	inline Matrix GetUntranslatedMatrix()
	{
		ALWAYSREHUP;
		if (!mParent) return mMat.GetUnTranslate();
		if (mRehup&REHUP_PARENT) {_mMat=ParentalizeM();UNSETBITS(mRehup,REHUP_PARENT);}
		return _mMat.GetUnTranslate();
	}
	*/



	inline Matrix&	GetInverse() 
	{
		ALWAYSREHUP;
		if (mRehup&REHUP_I) {mInverse=mMat.GetInvert();UNSETBITS(mRehup,REHUP_I);}
		if (!mParent) return mInverse;
		if (mRehup&REHUP_PARENTI) {_mInverse=ParentalizeM().GetInvert();;UNSETBITS(mRehup,REHUP_PARENTI);}
		return _mInverse;
	}
	inline Matrix&	GetInvert() {return GetInverse();}
	inline Matrix&	GetNormal() 
	{
		ALWAYSREHUP;
		if (mRehup&REHUP_N) {mNormal=mMat;mNormal.Normalize();UNSETBITS(mRehup,REHUP_N);}
		if (!mParent) return mNormal;
		if (mRehup&REHUP_PARENTN) {_mNormal=ParentalizeN();UNSETBITS(mRehup,REHUP_PARENTN);}
		//_mNormal=ParentalizeN();
		return _mNormal;
	}

	inline void		Reset() {mMat.Reset();mInverse.Reset();mNormal.Reset();}
	inline void		Rehup(int whatBits=0xFF)
	{
		mRehup=whatBits;
		if (mChild.Size()) foreach (aM,mChild) aM->Rehup(whatBits);
		//if (theFlags&0x01) mInverse=mMat.GetInvert();
		//if (theFlags&0x02) {mNormal=mMat;mNormal.Normalize();}
	}

	inline void		UnRehupNormal() {UNSETBITS(mRehup,REHUP_N);/*if (mChild.Size()) foreach (aM,mChild) aM->UnRehupNormal();*/}
	inline void		UnRehupInverse() {UNSETBITS(mRehup,REHUP_I);/*if (mChild.Size()) foreach (aM,mChild) aM->UnRehupInverse();*/}

	inline Vector	ProcessPoint(Vector thePos) {return GetMatrix().ProcessPoint(thePos);}
	inline Vector	ProcessPointN(Vector thePos) {return GetNormal().ProcessPoint(thePos);}
	inline Vector	ProcessPointNormalized(Vector thePos) {return GetMatrix().ProcessPointNormalized(thePos);}
	inline Matrix&	Set(Matrix *theMatrix) {if (!theMatrix) mMat.Identity();else mMat=*theMatrix;Rehup();return mMat;}
	inline Matrix&	Set(Matrix *theMatrix, Matrix* theNormalMatrix) 
	{
		if (!theMatrix) mMat.Identity();else mMat=*theMatrix;
		if (!theNormalMatrix) mNormal.Identity();else mNormal=*theNormalMatrix;
		Rehup(0xFF^(REHUP_N));
		return mMat;
	}
	inline Matrix&	Multiply(Matrix *theMatrix) {mMat.Multiply(theMatrix);Rehup();return mMat;}
	inline Matrix&	Scale(float theScale) {mMat.Scale(theScale);Rehup();return mMat;}
	inline Matrix&	RotateAroundAxis3D(Vector theVector, float theAngle) {mMat.RotateAroundAxis3D(theVector,theAngle);Rehup();return mMat;}
	inline Matrix&	UnTranslate() {mMat.UnTranslate();Rehup();return mMat;}
	inline Matrix&	Translate(float theX, float theY, float theZ) {mMat.Translate(theX,theY,theZ);Rehup();return mMat;}
	inline Matrix&	Translate(float theX, float theY) {mMat.Translate(theX,theY);Rehup();return mMat;}
	inline Matrix&	Translate(Point thePoint) {mMat.Translate(thePoint);Rehup();return mMat;}
	inline Matrix&	Translate(Vector thePoint) {mMat.Translate(thePoint);Rehup();return mMat;}
	inline Matrix&	Normalize() {mMat.Normalize();Rehup();return mMat;}

	inline bool					IsChildOf(MatrixKit* theKit) // Check if theKit is anywhere in our mParent structure
	{
		if(theKit==this) return true;
		if (!theKit || !mParent) return false;
		return mParent->IsChildOf(theKit);
	}
	inline bool					IsParentOf(MatrixKit* theKit) {if (!theKit) return false;return theKit->IsChildOf(this);}

	inline Matrix&	GetCoreMatrix() {return mMat;}
};
/**/

/*
class MatrixKit
{
public:

	Matrix			mMat;
	Matrix			mInverse;
	Matrix			mNormal;

	Matrix			_mMat;
	Matrix			_mInverse;
	Matrix			_mNormal;

	MatrixKit*		mParent=NULL;
	inline void		SetParent(MatrixKit* theKit) {mParent=theKit;}

	Matrix ParentalizeM()
	{
		if (!mParent) return mMat;
		Matrix aResult=mMat;
		aResult*=mParent->ParentalizeM();

		return aResult;
	}
	Matrix ParentalizeN()
	{
		if (!mParent) return mNormal;
		Matrix aResult=mNormal;
		aResult*=mParent->ParentalizeN();

		return aResult;
	}

	inline Matrix&	GetMatrix() 
	{
		if (!mParent) return mMat;
		_mMat=ParentalizeM();
		return _mMat;
	}

	inline Matrix&	GetInverse() 
	{
		if (!mParent) return mInverse;
		_mInverse=ParentalizeM().GetInvert();
		return _mInverse;
	}
	inline Matrix&	GetNormal() 
	{
		if (!mParent) return mNormal;
		_mNormal=ParentalizeN();
		return _mNormal;
	}

	inline void		Reset() {mMat.Reset();mInverse.Reset();mNormal.Reset();}
	inline void		Rehup(unsigned char theFlags=0xFF)
	{
		if (theFlags&0x01) mInverse=mMat.GetInvert();
		if (theFlags&0x02) {mNormal=mMat;mNormal.Normalize();}
	}

	inline Vector	ProcessPoint(Vector thePos) {return GetMatrix().ProcessPoint(thePos);}
	inline Matrix&	Multiply(Matrix *theMatrix) {mMat.Multiply(theMatrix);return mMat;}

	inline bool					IsChildOf(MatrixKit* theKit) // Check if theKit is anywhere in our mParent structure
	{
		if(theKit==this) return true;
		if (!theKit || !mParent) return false;
		return mParent->IsChildOf(theKit);
	}
	inline bool					IsParentOf(MatrixKit* theKit) {if (!theKit) return false;return theKit->IsChildOf(this);}
	inline Matrix&	GetCoreMatrix() {return mMat;}
};
/**/

