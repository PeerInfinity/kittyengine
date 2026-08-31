#include "rapt_matrix.h"
#include "rapt.h"

#define MANUAL



#define m1	mMatrix1
#define m2  mMatrix2

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

Matrix::Matrix(void)
{
	Reset();
}

Matrix& Matrix::Reset()
{
#ifndef MANUAL
	D3DXMatrixIdentity(&mData);
#else
	m00=1.0f;
	m01=0.0f;
	m02=0.0f;
	m03=0.0f;

	m10=0.0f;
	m11=1.0f;
	m12=0.0f;
	m13=0.0f;

	m20=0.0f;
	m21=0.0f;
	m22=1.0f;
	m23=0.0f;

	m30=0.0f;
	m31=0.0f;
	m32=0.0f;
	m33=1.0f;
#endif
	return *this;
}

Matrix& Matrix::Invert()
{
#ifndef MANUAL
	D3DXMATRIX aOld=mData;
	D3DXMatrixInverse(&mData,NULL,&aOld);
#else
	float aInv[16];

	aInv[0]=mData.mm[5]*mData.mm[10]*mData.mm[15]-mData.mm[5]*mData.mm[11]*mData.mm[14]-mData.mm[9]*mData.mm[6]*mData.mm[15]+mData.mm[9]*mData.mm[7]*mData.mm[14]+mData.mm[13]*mData.mm[6]*mData.mm[11]-mData.mm[13]*mData.mm[7]*mData.mm[10];
	aInv[4]=-mData.mm[4]*mData.mm[10]*mData.mm[15]+mData.mm[4]*mData.mm[11]*mData.mm[14]+mData.mm[8]*mData.mm[6]*mData.mm[15]-mData.mm[8]*mData.mm[7]*mData.mm[14]-mData.mm[12]*mData.mm[6]*mData.mm[11]+mData.mm[12]*mData.mm[7]*mData.mm[10];
	aInv[8]=mData.mm[4]*mData.mm[9]*mData.mm[15]-mData.mm[4]*mData.mm[11]*mData.mm[13]-mData.mm[8]*mData.mm[5]*mData.mm[15]+mData.mm[8]*mData.mm[7]*mData.mm[13]+mData.mm[12]*mData.mm[5]*mData.mm[11]-mData.mm[12]*mData.mm[7]*mData.mm[9];
	aInv[12]=-mData.mm[4]*mData.mm[9]*mData.mm[14]+mData.mm[4]*mData.mm[10]*mData.mm[13]+mData.mm[8]*mData.mm[5]*mData.mm[14]-mData.mm[8]*mData.mm[6]*mData.mm[13]-mData.mm[12]*mData.mm[5]*mData.mm[10]+mData.mm[12]*mData.mm[6]*mData.mm[9];
	aInv[1]=-mData.mm[1]*mData.mm[10]*mData.mm[15]+mData.mm[1]*mData.mm[11]*mData.mm[14]+mData.mm[9]*mData.mm[2]*mData.mm[15]-mData.mm[9]*mData.mm[3]*mData.mm[14]-mData.mm[13]*mData.mm[2]*mData.mm[11]+mData.mm[13]*mData.mm[3]*mData.mm[10];
	aInv[5]=mData.mm[0]*mData.mm[10]*mData.mm[15]-mData.mm[0]*mData.mm[11]*mData.mm[14]-mData.mm[8]*mData.mm[2]*mData.mm[15]+mData.mm[8]*mData.mm[3]*mData.mm[14]+mData.mm[12]*mData.mm[2]*mData.mm[11]-mData.mm[12]*mData.mm[3]*mData.mm[10];
	aInv[9]=-mData.mm[0]*mData.mm[9]*mData.mm[15]+mData.mm[0]*mData.mm[11]*mData.mm[13]+mData.mm[8]*mData.mm[1]*mData.mm[15]-mData.mm[8]*mData.mm[3]*mData.mm[13]-mData.mm[12]*mData.mm[1]*mData.mm[11]+mData.mm[12]*mData.mm[3]*mData.mm[9];
	aInv[13]=mData.mm[0]*mData.mm[9]*mData.mm[14]-mData.mm[0]*mData.mm[10]*mData.mm[13]-mData.mm[8]*mData.mm[1]*mData.mm[14]+mData.mm[8]*mData.mm[2]*mData.mm[13]+mData.mm[12]*mData.mm[1]*mData.mm[10]-mData.mm[12]*mData.mm[2]*mData.mm[9];
	aInv[2]=mData.mm[1]*mData.mm[6]*mData.mm[15]-mData.mm[1]*mData.mm[7]*mData.mm[14]-mData.mm[5]*mData.mm[2]*mData.mm[15]+mData.mm[5]*mData.mm[3]*mData.mm[14]+mData.mm[13]*mData.mm[2]*mData.mm[7]-mData.mm[13]*mData.mm[3]*mData.mm[6];
	aInv[6]=-mData.mm[0]*mData.mm[6]*mData.mm[15]+mData.mm[0]*mData.mm[7]*mData.mm[14]+mData.mm[4]*mData.mm[2]*mData.mm[15]-mData.mm[4]*mData.mm[3]*mData.mm[14]-mData.mm[12]*mData.mm[2]*mData.mm[7]+mData.mm[12]*mData.mm[3]*mData.mm[6];
	aInv[10]=mData.mm[0]*mData.mm[5]*mData.mm[15]-mData.mm[0]*mData.mm[7]*mData.mm[13]-mData.mm[4]*mData.mm[1]*mData.mm[15]+mData.mm[4]*mData.mm[3]*mData.mm[13]+mData.mm[12]*mData.mm[1]*mData.mm[7]-mData.mm[12]*mData.mm[3]*mData.mm[5];
	aInv[14]=-mData.mm[0]*mData.mm[5]*mData.mm[14]+mData.mm[0]*mData.mm[6]*mData.mm[13]+mData.mm[4]*mData.mm[1]*mData.mm[14]-mData.mm[4]*mData.mm[2]*mData.mm[13]-mData.mm[12]*mData.mm[1]*mData.mm[6]+mData.mm[12]*mData.mm[2]*mData.mm[5];
	aInv[3]=-mData.mm[1]*mData.mm[6]*mData.mm[11]+mData.mm[1]*mData.mm[7]*mData.mm[10]+mData.mm[5]*mData.mm[2]*mData.mm[11]-mData.mm[5]*mData.mm[3]*mData.mm[10]-mData.mm[9]*mData.mm[2]*mData.mm[7]+mData.mm[9]*mData.mm[3]*mData.mm[6];
	aInv[7]=mData.mm[0]*mData.mm[6]*mData.mm[11]-mData.mm[0]*mData.mm[7]*mData.mm[10]-mData.mm[4]*mData.mm[2]*mData.mm[11]+mData.mm[4]*mData.mm[3]*mData.mm[10]+mData.mm[8]*mData.mm[2]*mData.mm[7]-mData.mm[8]*mData.mm[3]*mData.mm[6];
	aInv[11]=-mData.mm[0]*mData.mm[5]*mData.mm[11]+mData.mm[0]*mData.mm[7]*mData.mm[9]+mData.mm[4]*mData.mm[1]*mData.mm[11]-mData.mm[4]*mData.mm[3]*mData.mm[9]-mData.mm[8]*mData.mm[1]*mData.mm[7]+mData.mm[8]*mData.mm[3]*mData.mm[5];
	aInv[15]=mData.mm[0]*mData.mm[5]*mData.mm[10]-mData.mm[0]*mData.mm[6]*mData.mm[9]-mData.mm[4]*mData.mm[1]*mData.mm[10]+mData.mm[4]*mData.mm[2]*mData.mm[9]+mData.mm[8]*mData.mm[1]*mData.mm[6]-mData.mm[8]*mData.mm[2]*mData.mm[5];
	float aDet=mData.mm[0]*aInv[0]+mData.mm[1]*aInv[4]+mData.mm[2]*aInv[8]+ mData.mm[3]*aInv[12];
	if (aDet!=0) {aDet=1.0f/aDet;for (int aCount=0;aCount<16;aCount++) mData.mm[aCount]=aInv[aCount]*aDet;}

#endif

	return *this;
}

Matrix& Matrix::Transpose()
{
	Matrix aNew;
	for (int aX=0;aX<4;aX++) for (int aY=0;aY<4;aY++) aNew.mData.m[aX][aY]=mData.m[aY][aX];
	*this=aNew;

	return *this;
}

/*
void Matrix::Translate(float theX, float theY, float theZ)
{
#ifndef MANUAL
	D3DXMATRIX aMoveMatrix;
	D3DXMatrixMultiply(&mData,&mData,D3DXMatrixTranslation(&aMoveMatrix,theX,theY,theZ));
#else
	m30+=theX;
	m31+=theY;
	m32+=theZ;
#endif
}

void Matrix::Translate(float theX, float theY)
{
	m30+=theX;
	m31+=theY;
}
*/

Matrix& Matrix::Multiply(Matrix *theMatrix)
{
#ifndef MANUAL
	D3DXMatrixMultiply(&mData,&theMatrix->mData,&mData);
#else
	static Matrix aMat;
	memcpy(&aMat.mData,&mData,sizeof(mData));

	Matrix &m1=aMat;
	Matrix &m2=*theMatrix;
	m00=m1.m00*m2.m00+m1.m01*m2.m10+m1.m02*m2.m20+m1.m03*m2.m30;
	m01=m1.m00*m2.m01+m1.m01*m2.m11+m1.m02*m2.m21+m1.m03*m2.m31;    
	m02=m1.m00*m2.m02+m1.m01*m2.m12+m1.m02*m2.m22+m1.m03*m2.m32;    
	m03=m1.m00*m2.m03+m1.m01*m2.m13+m1.m02*m2.m23+m1.m03*m2.m33;    
	m10=m1.m10*m2.m00+m1.m11*m2.m10+m1.m12*m2.m20+m1.m13*m2.m30;    
	m11=m1.m10*m2.m01+m1.m11*m2.m11+m1.m12*m2.m21+m1.m13*m2.m31;    
	m12=m1.m10*m2.m02+m1.m11*m2.m12+m1.m12*m2.m22+m1.m13*m2.m32;    
	m13=m1.m10*m2.m03+m1.m11*m2.m13+m1.m12*m2.m23+m1.m13*m2.m33;    
	m20=m1.m20*m2.m00+m1.m21*m2.m10+m1.m22*m2.m20+m1.m23*m2.m30;    
	m21=m1.m20*m2.m01+m1.m21*m2.m11+m1.m22*m2.m21+m1.m23*m2.m31;    
	m22=m1.m20*m2.m02+m1.m21*m2.m12+m1.m22*m2.m22+m1.m23*m2.m32;    
	m23=m1.m20*m2.m03+m1.m21*m2.m13+m1.m22*m2.m23+m1.m23*m2.m33;    
	m30=m1.m30*m2.m00+m1.m31*m2.m10+m1.m32*m2.m20+m1.m33*m2.m30;    
	m31=m1.m30*m2.m01+m1.m31*m2.m11+m1.m32*m2.m21+m1.m33*m2.m31;    
	m32=m1.m30*m2.m02+m1.m31*m2.m12+m1.m32*m2.m22+m1.m33*m2.m32;    
	m33=m1.m30*m2.m03+m1.m31*m2.m13+m1.m32*m2.m23+m1.m33*m2.m33;
#endif

	return *this;
}

Matrix& Matrix::Subtract(Matrix *subtractThis)
{
	for (int aCount=0;aCount<16;aCount++) mData.mm[aCount]-=subtractThis->mData.mm[aCount];
	return *this;
}


Matrix& Matrix::Scale(float theX, float theY, float theZ)
{
#ifndef MANUAL
	D3DXMATRIX aScaleMatrix;
	D3DXMatrixMultiply(&mData,&mData,D3DXMatrixScaling(&aScaleMatrix,theX,theY,theZ));
#else
	Matrix aMat;
	aMat.m00=theX;
	aMat.m11=theY;
	aMat.m22=theZ;
	Multiply(&aMat);
#endif

	return *this;
}

Matrix& Matrix::RotateXYZ(float theX, float theY, float theZ)
{
	//
	// Def can speed this sucker up...
	//
	RotateX(theX);
	RotateY(theY);
	RotateZ(theZ);
	return *this;

	/*
	Matrix aMat;
	memcpy(&aMat.mData,&mData,sizeof(mData));

	Matrix &m1=aMat;
	Matrix &m2=*theMatrix;
	m00=m1.m00*m2.m00+m1.m01*m2.m10+m1.m02*m2.m20+m1.m03*m2.m30;
	m01=m1.m00*m2.m01+m1.m01*m2.m11+m1.m02*m2.m21+m1.m03*m2.m31;    
	m02=m1.m00*m2.m02+m1.m01*m2.m12+m1.m02*m2.m22+m1.m03*m2.m32;    
	m03=m1.m00*m2.m03+m1.m01*m2.m13+m1.m02*m2.m23+m1.m03*m2.m33;    
	m10=m1.m10*m2.m00+m1.m11*m2.m10+m1.m12*m2.m20+m1.m13*m2.m30;    
	m11=m1.m10*m2.m01+m1.m11*m2.m11+m1.m12*m2.m21+m1.m13*m2.m31;    
	m12=m1.m10*m2.m02+m1.m11*m2.m12+m1.m12*m2.m22+m1.m13*m2.m32;    
	m13=m1.m10*m2.m03+m1.m11*m2.m13+m1.m12*m2.m23+m1.m13*m2.m33;    
	m20=m1.m20*m2.m00+m1.m21*m2.m10+m1.m22*m2.m20+m1.m23*m2.m30;    
	m21=m1.m20*m2.m01+m1.m21*m2.m11+m1.m22*m2.m21+m1.m23*m2.m31;    
	m22=m1.m20*m2.m02+m1.m21*m2.m12+m1.m22*m2.m22+m1.m23*m2.m32;    
	m23=m1.m20*m2.m03+m1.m21*m2.m13+m1.m22*m2.m23+m1.m23*m2.m33;    
	m30=m1.m30*m2.m00+m1.m31*m2.m10+m1.m32*m2.m20+m1.m33*m2.m30;    
	m31=m1.m30*m2.m01+m1.m31*m2.m11+m1.m32*m2.m21+m1.m33*m2.m31;    
	m32=m1.m30*m2.m02+m1.m31*m2.m12+m1.m32*m2.m22+m1.m33*m2.m32;    
	m33=m1.m30*m2.m03+m1.m31*m2.m13+m1.m32*m2.m23+m1.m33*m2.m33;
*/
}


Matrix& Matrix::RotateX(float degrees)
{
#ifndef MANUAL
	D3DXMATRIX aRotateMatrix;
//	D3DXMatrixMultiply(&mData,&mData,D3DXMatrixRotationX(&aRotateMatrix,gMath.Rad(degrees)));
#else
	Matrix aMat;
	aMat.m11=-gMath.Cos(-degrees);
	aMat.m21=gMath.Sin(-degrees);
	aMat.m12=-gMath.Sin(-degrees);
	aMat.m22=-gMath.Cos(-degrees);
	Multiply(&aMat);
#endif

	return *this;
}

Matrix& Matrix::RotateY(float degrees)
{
#ifndef MANUAL
	D3DXMATRIX aRotateMatrix;
//	D3DXMatrixMultiply(&mData,&mData,D3DXMatrixRotationY(&aRotateMatrix,gMath.Rad(degrees)));
#else
	Matrix aMat;
	aMat.m00=-gMath.Cos(-degrees);
	aMat.m02=gMath.Sin(-degrees);
	aMat.m20=-gMath.Sin(-degrees);
	aMat.m22=-gMath.Cos(-degrees);
	Multiply(&aMat);
#endif

	return *this;
}

Matrix& Matrix::RotateZ(float degrees)
{
#ifndef MANUAL
	D3DXMATRIX aRotateMatrix;
//	D3DXMatrixMultiply(&mData,&mData,D3DXMatrixRotationZ(&aRotateMatrix,gMath.Rad(degrees)));
#else
	Matrix aMat;
	aMat.m00=-gMath.Cos(-degrees);
	aMat.m01=-gMath.Sin(-degrees);
	aMat.m10=gMath.Sin(-degrees);
	aMat.m11=-gMath.Cos(-degrees);
	Multiply(&aMat);
#endif
	return *this;
}

Matrix& Matrix::SkewX(float degrees)
{
	Matrix aMat;
	aMat.m10=-gMath.Tan(degrees);
	Multiply(&aMat);
	return *this;
}

Matrix& Matrix::SkewY(float degrees)
{
	Matrix aMat;
	aMat.m01=-gMath.Tan(degrees);
	Multiply(&aMat);
	return *this;
}

Matrix& Matrix::SkewXZ(float degrees)
{
	Matrix aMat;
	aMat.m02=-gMath.Tan(degrees);
	Multiply(&aMat);
	return *this;
}


Matrix& Matrix::SkewZX(float degrees)
{
	Matrix aMat;
	aMat.m20=-gMath.Tan(degrees);
	Multiply(&aMat);
	return *this;
}

Matrix& Matrix::SkewZY(float degrees)
{
	Matrix aMat;
	aMat.m21=-gMath.Tan(degrees);
	Multiply(&aMat);
	return *this;
}


/*
inline Point Matrix::ProcessPoint(Point thePoint)
{
	Point aResult;

#ifndef MANUAL
	aResult.mX=
				mData.m[0][0]*thePoint.mX +
				mData.m[1][0]*thePoint.mY +
				mData.m[3][0];

	aResult.mY=
				mData.m[0][1]*thePoint.mX +
				mData.m[1][1]*thePoint.mY +
				mData.m[3][1];

#else
	aResult.mX=m00*thePoint.mX+m10*thePoint.mY+m30;
	aResult.mY=m01*thePoint.mX+m11*thePoint.mY+m31;
#endif

	return aResult;
}
*/


void Matrix::ProcessPoints(Point *thePointPtr, int theCount)
{
	for (int aCount=0;aCount<theCount;aCount++)
	{
		float aX=thePointPtr->mX;
		float aY=thePointPtr->mY;
		thePointPtr->mX=m00*aX+m10*aY+m30;
		thePointPtr->mY=m01*aX+m11*aY+m31;
		thePointPtr++;
	}
}

void Matrix::ProcessPoints(Vertex2D *theVertexPtr, int theCount)
{
	for (int aCount=0;aCount<theCount;aCount++)
	{
		float aX=theVertexPtr->mX;
		float aY=theVertexPtr->mY;
		float aZ=theVertexPtr->mZ;
		theVertexPtr->mX=m00*aX+m10*aY+m20*aZ+m30;
		theVertexPtr->mY=m01*aX+m11*aY+m21*aZ+m31;
		theVertexPtr->mZ=m02*aX+m12*aY+m22*aZ+m32;
		theVertexPtr++;
	}
}


void Matrix::ProcessPoints(Vertex2DN *theVertexPtr, int theCount)
{
	for (int aCount=0;aCount<theCount;aCount++)
	{
		float aX=theVertexPtr->mX;
		float aY=theVertexPtr->mY;
		float aZ=theVertexPtr->mZ;
		float aNX=theVertexPtr->mNX;
		float aNY=theVertexPtr->mNY;
		float aNZ=theVertexPtr->mNZ;
		theVertexPtr->mX=m00*aX+m10*aY+m20*aZ+m30;
		theVertexPtr->mY=m01*aX+m11*aY+m21*aZ+m31;
		theVertexPtr->mZ=m02*aX+m12*aY+m22*aZ+m32;
		//theVertexPtr->mNX=m00*aNX+m10*aNY+m20*aNZ+m30;
		//theVertexPtr->mNY=m01*aNX+m11*aNY+m21*aNZ+m31;
		//theVertexPtr->mNZ=m02*aNX+m12*aNY+m22*aNZ+m32;
		theVertexPtr->mNX=m00*aNX+m10*aNY+m20*aNZ;
		theVertexPtr->mNY=m01*aNX+m11*aNY+m21*aNZ;
		theVertexPtr->mNZ=m02*aNX+m12*aNY+m22*aNZ;
		// Issue: The normal gets scaled.  Do we care?

		theVertexPtr++;
	}
}


void Matrix::ProcessPoints(Array<Vertex2D>& in,Array<Vertex2D>& out)
{
	out.SetSize(in.Size());
	Vertex2D* theVertexPtr=in.mArray;
	Vertex2D* theVertexOutPtr=out.mArray;

	for (int aCount=0;aCount<in.Size();aCount++)
	{
		float aX=theVertexPtr->mX;
		float aY=theVertexPtr->mY;
		float aZ=theVertexPtr->mZ;
		theVertexOutPtr->mX=m00*aX+m10*aY+m20*aZ+m30;
		theVertexOutPtr->mY=m01*aX+m11*aY+m21*aZ+m31;
		theVertexOutPtr->mZ=m02*aX+m12*aY+m22*aZ+m32;
		theVertexPtr++;
		theVertexOutPtr++;
	}

	//out.Copy(in);
	//ProcessPoints(out.mArray,out.Size());
}

void Matrix::ProcessPoints(Array<Vertex2DN>& in,Array<Vertex2DN>& out)
{
	//out.GuaranteeSize(in.Size());
	out.SetSize(in.Size());
	Vertex2DN* theVertexPtr=in.mArray;
	Vertex2DN* theVertexOutPtr=out.mArray;

	for (int aCount=0;aCount<in.Size();aCount++)
	{
		float aX=theVertexPtr->mX;
		float aY=theVertexPtr->mY;
		float aZ=theVertexPtr->mZ;
		float aNX=theVertexPtr->mNX;
		float aNY=theVertexPtr->mNY;
		float aNZ=theVertexPtr->mNZ;
		theVertexOutPtr->mX=m00*aX+m10*aY+m20*aZ+m30;
		theVertexOutPtr->mY=m01*aX+m11*aY+m21*aZ+m31;
		theVertexOutPtr->mZ=m02*aX+m12*aY+m22*aZ+m32;
		theVertexOutPtr->mNX=m00*aNX+m10*aNY+m20*aNZ;
		theVertexOutPtr->mNY=m01*aNX+m11*aNY+m21*aNZ;
		theVertexOutPtr->mNZ=m02*aNX+m12*aNY+m22*aNZ;
		theVertexPtr++;
		theVertexOutPtr++;
	}
}


void Matrix::ProcessPoints(Vertex2DMT *theVertexPtr, int theCount)
{
	for (int aCount=0;aCount<theCount;aCount++)
	{
		float aX=theVertexPtr->mX;
		float aY=theVertexPtr->mY;
		float aZ=theVertexPtr->mZ;
		theVertexPtr->mX=m00*aX+m10*aY+m20*aZ+m30;
		theVertexPtr->mY=m01*aX+m11*aY+m21*aZ+m31;
		theVertexPtr->mZ=m02*aX+m12*aY+m22*aZ+m32;
		theVertexPtr++;
	}
}

void Matrix::ProcessPoints(Vector *theVertexPtr, int theCount)
{
	for (int aCount=0;aCount<theCount;aCount++)
	{
		float aX=theVertexPtr->mX;
		float aY=theVertexPtr->mY;
		float aZ=theVertexPtr->mZ;
		theVertexPtr->mX=m00*aX+m10*aY+m20*aZ+m30;
		theVertexPtr->mY=m01*aX+m11*aY+m21*aZ+m31;
		theVertexPtr->mZ=m02*aX+m12*aY+m22*aZ+m32;
		theVertexPtr++;
	}
}

void Matrix::ProcessPoints(Array<Vector>& theIn,Array<Vector>& theOut)
{
	theOut.SetSize(theIn.Size());

	//if (theOut.Size()>theIn.Size()) theOut.Truncate(theIn.Size());
	//else theOut.GuaranteeSize(theIn.Size());

	for (int aCount=0;aCount<theIn.Size();aCount++)
	{
		float aX=theIn[aCount].mX;
		float aY=theIn[aCount].mY;
		float aZ=theIn[aCount].mZ;
		theOut[aCount].mX=m00*aX+m10*aY+m20*aZ+m30;
		theOut[aCount].mY=m01*aX+m11*aY+m21*aZ+m31;
		theOut[aCount].mZ=m02*aX+m12*aY+m22*aZ+m32;
	}
}




Vector Matrix::ProcessPointNormalized(Vector theVector)
{
	Vector aResult;

	aResult.mX=m00*theVector.mX+m10*theVector.mY+m20*theVector.mZ+m30;
	aResult.mY=m01*theVector.mX+m11*theVector.mY+m21*theVector.mZ+m31;
	aResult.mZ=m02*theVector.mX+m12*theVector.mY+m22*theVector.mZ+m32;
	float aW=1/(m03*theVector.mX+m13*theVector.mY+m23*theVector.mZ+m33);

	aResult.mX*=aW;
	aResult.mY*=aW;
	aResult.mZ*=aW;

	return aResult;
}

Vector Matrix::ProcessPointN(Vector theVector)
{
	Vector aResult;
	aResult=ProcessPoint(theVector)-ProcessPoint();
	/*
	aResult.mX=m00*theVector.mX+m10*theVector.mY+m20*theVector.mZ;
	aResult.mY=m01*theVector.mX+m11*theVector.mY+m21*theVector.mZ;
	aResult.mZ=m02*theVector.mX+m12*theVector.mY+m22*theVector.mZ;
	*/
	return aResult;
}

Vector Matrix::ProcessPointN(float x, float y, float z)
{
	Vector aResult;
	aResult=ProcessPoint(x,y,z)-ProcessPoint();

	/*
	aResult.mX=m00*x+m10*y+m20*z;
	aResult.mY=m01*x+m11*y+m21*z;
	aResult.mZ=m02*x+m12*y+m22*z;
	*/
	return aResult;
}


void Matrix::ProcessPoints(Vertex2D *theInput, Vertex2D* theOutput, int thePoints)
{
	for (int aCount=0;aCount<thePoints;aCount++)
	{
		float aX=theInput->mX;
		float aY=theInput->mY;
		float aZ=theInput->mZ;
		theOutput->mX=m00*aX+m10*aY+m20*aZ+m30;
		theOutput->mY=m01*aX+m11*aY+m21*aZ+m31;
		theOutput->mZ=m02*aX+m12*aY+m22*aZ+m32;
		theInput++;
		theOutput++;
	}
}


void Matrix::ProcessPoints(Vertex2DN *theInput, Vertex2DN* theOutput, int thePoints)
{
	for (int aCount=0;aCount<thePoints;aCount++)
	{
		float aX=theInput->mX;
		float aY=theInput->mY;
		float aZ=theInput->mZ;
		float aNX=theInput->mX;
		float aNY=theInput->mY;
		float aNZ=theInput->mZ;
		theOutput->mX=m00*aX+m10*aY+m20*aZ+m30;
		theOutput->mY=m01*aX+m11*aY+m21*aZ+m31;
		theOutput->mZ=m02*aX+m12*aY+m22*aZ+m32;
		theOutput->mNX=m00*aNX+m10*aNY+m20*aNZ;
		theOutput->mNY=m01*aNX+m11*aNY+m21*aNZ;
		theOutput->mNZ=m02*aNX+m12*aNY+m22*aNZ;
		theInput++;
		theOutput++;
	}
}


void Matrix::ProcessVectorXY(Vector *theVertexPtr, int theCount)
{
	for (int aCount=0;aCount<theCount;aCount++)
	{
		float aX=theVertexPtr->mX;
		float aY=theVertexPtr->mY;
		theVertexPtr->mX=m00*aX+m10*aY+m30;
		theVertexPtr->mY=m01*aX+m11*aY+m31;
		theVertexPtr++;
	}
}


Vector Matrix::ProcessVectorXY(Vector& theVector)
{
	Vector aResult;

	aResult.mX=m00*theVector.mX+m10*theVector.mY+m30;
	aResult.mY=m01*theVector.mX+m11*theVector.mY+m31;
	aResult.mZ=theVector.mZ;

	return aResult;
}

void Matrix::ProcessVertexXY(Vertex2D *theVertexPtr, int theCount)
{
	for (int aCount=0;aCount<theCount;aCount++)
	{
		float aX=theVertexPtr->mX;
		float aY=theVertexPtr->mY;
		theVertexPtr->mX=m00*aX+m10*aY+m30;
		theVertexPtr->mY=m01*aX+m11*aY+m31;
		theVertexPtr++;
	}
}


/*
Vertex Matrix::ProcessVertex(Vertex theVertex)
{
	Vertex aResult;

#ifndef MANUAL
	aResult.mX=
		mData.m[0][0]*theVertex.mX +
		mData.m[1][0]*theVertex.mY +
		mData.m[2][0]*theVertex.mZ +
		mData.m[3][0];

	aResult.mY=
		mData.m[0][1]*theVertex.mX +
		mData.m[1][1]*theVertex.mY +
		mData.m[2][1]*theVertex.mZ +
		mData.m[3][1];

	aResult.mZ=
		mData.m[0][2]*theVertex.mX +
		mData.m[1][2]*theVertex.mY +
		mData.m[2][2]*theVertex.mZ +
		mData.m[3][2];

#else
	aResult.mX=
		m00*theVertex.mX +
		m10*theVertex.mY +
		m20*theVertex.mZ +
		m30;

	aResult.mY=
		m01*theVertex.mX +
		m11*theVertex.mY +
		m21*theVertex.mZ +
		m31;

	aResult.mZ=
		m02*theVertex.mX +
		m12*theVertex.mY +
		m22*theVertex.mZ +
		m32;

#endif

	return aResult;
}
*/

bool Matrix::IsRotatedOrScaled()
{
#ifndef MANUAL
	if (
		mData.m[0][0]==1 &&
		mData.m[1][0]==0 &&
		mData.m[2][0]==0 &&
		mData.m[0][1]==0 &&
		mData.m[1][1]==1 &&
		mData.m[2][1]==0 &&
		mData.m[0][2]==0 &&
		mData.m[1][2]==0 &&
		mData.m[2][2]==1
	) return false;
#else
	if (
		m00==1 &&
		m10==0 &&
		m20==0 &&
		m01==0 &&
		m11==1 &&
		m21==0 &&
		m02==0 &&
		m12==0 &&
		m22==1
		) return false;
#endif
	return true;
}

bool Matrix::IsFractionalTranslate()
{
#ifndef MANUAL
	if (
		mData.m[3][0]==(float)((int)mData.m[3][0]) &&
		mData.m[3][1]==(float)((int)mData.m[3][1]) &&
		mData.m[3][2]==(float)((int)mData.m[3][2])
		) 
	{
		return false;
	}
#else
	if (
		m30==(float)((int)m30) &&
		m31==(float)((int)m31) &&
		m32==(float)((int)m32)
		) 
	{
		return false;
	}
#endif
	return true;
}

Vector Matrix::GetTranslate()
{
#ifndef MANUAL
	return Vector(mData.m[3][0],mData.m[3][1],mData.m[3][2]);
#else
	return Vector(m30,m31,m32);
#endif
}

void Matrix::SetTranslate(float theX, float theY, float theZ)
{
#ifndef MANUAL
	mData.m[3][0]=theVector.mX;
	mData.m[3][1]=theVector.mY;
	mData.m[3][2]=theVector.mZ;
#else
	m30=theX;
	m31=theY;
	m32=theZ;
#endif
}


float Matrix::Determinant(void)
{
	float aValue;
	aValue= 
		m03 *m12 *m21 *m30-m02 *m13 *m21 *m30-m03 *m11 *m22 *m30+m01 *m13 *m22 *m30+
		m02 *m11 *m23 *m30-m01 *m12 *m23 *m30-m03 *m12 *m20 *m31+m02 *m13 *m20 *m31+
		m03 *m10 *m22 *m31-m00 *m13 *m22 *m31-m02 *m10 *m23 *m31+m00 *m12 *m23 *m31+
		m03 *m11 *m20 *m32-m01 *m13 *m20 *m32-m03 *m10 *m21 *m32+m00 *m13 *m21 *m32+
		m01 *m10 *m23 *m32-m00 *m11 *m23 *m32-m02 *m11 *m20 *m33+m01 *m12 *m20 *m33+
		m02 *m10 *m21 *m33-m00 *m12 *m21 *m33-m01 *m10 *m22 *m33+m00 *m11 *m22 *m33;
	return aValue;
}

void Matrix::DumpMakeCommand(char* theFilename)
{
	Matrix aID;

	IOBuffer aBuffer;

	String aString="";

	aBuffer.WriteLine("Matrix aMat;");
	for (int aSpanX=0;aSpanX<4;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<4;aSpanY++)
		{
			if (mData.m[aSpanX][aSpanY]!=aID.mData.m[aSpanX][aSpanY])
			{
				aString+=Sprintf("aMat.mData.m[%d][%d]=%ff;",aSpanX,aSpanY,mData.m[aSpanX][aSpanY]);
			}
		}
	}
	aBuffer.WriteLine(aString);

	//aString=Sprintf("Make(%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff);",m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33);

	aBuffer.CommitFile(theFilename);
}


Matrix& Matrix::RotateAroundAxis3D(Vector theVector, float theAngle)
{
	//
	// Multiply the angle by -1 to put it into Raptis Space.
	//
	theAngle=180-theAngle;

	theVector.Normalize();

	Matrix aMat;

	float aSin=gMath.Sin(theAngle);
	float aCos=gMath.Cos(theAngle);
	float aT=1.0f-aCos;
	aMat.m00=aT * theVector.mX * theVector.mX + aCos;
	aMat.m10=aT * theVector.mX * theVector.mY - aSin * theVector.mZ;
	aMat.m20=aT * theVector.mX * theVector.mZ + aSin * theVector.mY;
	aMat.m01=aT * theVector.mY * theVector.mX + aSin * theVector.mZ;
	aMat.m11=aT * theVector.mY * theVector.mY + aCos;
	aMat.m21=aT * theVector.mY * theVector.mZ - aSin * theVector.mX;
	aMat.m02=aT * theVector.mZ * theVector.mX - aSin * theVector.mY;
	aMat.m12=aT * theVector.mZ * theVector.mY + aSin * theVector.mX;
	aMat.m22=aT * theVector.mZ * theVector.mZ + aCos;

	return Multiply(&aMat);
}

Matrix& Matrix::Planify(Plane thePlane, float theScale)
{
	Matrix aMat=*this;

	aMat.Translate(-thePlane.mPos);
	aMat*=VectorToVectorMatrix(thePlane.mNormal,Vector(0,0,1));
	aMat.Scale(theScale,theScale,0);

	*this=aMat;
	return *this;
	//return aMat;//Multiply(&aMat);
}


Matrix& Matrix::Perspective()
{
	return Perspective(57.0f,1.0f,1.0f,gG.ZDepth());
	
}


Matrix& Matrix::Perspective(float theFieldOfView, float theAspectRatio, float zNear, float zFar)
{
	//
	// This perspective routine is MAJORLY fucked up.
	// First off, I've randomly negated some things.  I've put an
	// arbitrary number where the value -1 should be according to the web.
	//
	// I think the problem HAS to be the projection matrix.  The perspective of this
	// matrix skews as it moves away from 0,0-- 0,0 seems to be a "correct" center, but
	// I'm 100% not able to move anything away from it.  So I'm thinking we're going to
	// have to do perspective from inside the Core, like we had to do for the 2D camera.
	// 
	// I was not able to get it to work before.  This is, in fact, the closest I've come to
	// getting something to work!
	//
	const float h = gMath.Cot(theFieldOfView);
	float neg_depth = (zNear-zFar)*-1;

	m00 = h / theAspectRatio;
	m01 = 0;
	m02 = 0;
	m03 = 0;

	m10 = 0;
	m11 = h;
	m12 = 0;
	m13 = 0;

	m20 = 0;
	m21 = 0;
	m22 = (zFar + zNear)/neg_depth;
	m23 = .001f; // Completely arbitrary number

	m30 = 0;
	m31 = 0;
	m32 = -2.0f*(zNear*zFar)/neg_depth;
	m33 = 0;

	return *this;
}

Matrix Matrix::Billboard()
{
	Matrix aNew;

	/*
	aNew=*this;
	aNew.m00=1.0f;
	aNew.m01=0.0f;
	aNew.m02=0.0f;
	aNew.m10=0.0f;
	aNew.m11=1.0f;
	aNew.m12=0.0f;
	aNew.m20=0.0f;
	aNew.m21=0.0f;
	aNew.m22=1.0f;
	/**/

	//*
	aNew.m00=m00;
	aNew.m01=m10;
	aNew.m02=m20;
	aNew.m10=m01;
	aNew.m11=m11;
	aNew.m12=m21;
	aNew.m20=m02;
	aNew.m21=m12;
	aNew.m22=m22;
	/**/
	//aNew.m03=m03;
	//aNew.m13=m13;
	//aNew.m23=m23;
	//aNew.m33=m33;
	//aNew.m30=m30;
	//aNew.m31=m31;
	//aNew.m32=m32;
	//aNew.m33=m33;

	return aNew;

}

Matrix& Matrix::LookAt(Vector theLookat, Vector theCamera, Vector theUpVector)
{
//	Vector aZAxis=theLookat-theCamera;//theCamera-theLookat;//-theCamera;
	Vector aZAxis=theCamera-theLookat;
	aZAxis.Normalize();
	Vector aXAxis=gMath.Cross(theUpVector,aZAxis);
	aXAxis.Normalize();
	Vector aYAxis=gMath.Cross(aZAxis,aXAxis);

	m00=aXAxis.mX;
	m01=aXAxis.mY;
	m02=aXAxis.mZ;
	m03=0;

	m10=aYAxis.mX;
	m11=aYAxis.mY;
	m12=aYAxis.mZ;
	m13=0;

	m20=aZAxis.mX;
	m21=aZAxis.mY;
	m22=aZAxis.mZ;
	m23=0;

	m30=-gMath.Dot(aXAxis,theCamera);
	m31=-gMath.Dot(aYAxis,theCamera);
	m32=-gMath.Dot(aZAxis,theCamera);
	m33=1.0f;

	return *this;
}

Matrix& Matrix::PointAt(Vector thePointDir, Vector theUpVector)
{
	//	Vector aZAxis=theLookat-theCamera;//theCamera-theLookat;//-theCamera;
	theUpVector.Normalize();
	Vector aZAxis=-thePointDir.Normal();
	if (gMath.Abs(theUpVector.Dot(aZAxis))>1.0f-gMath.mMachineEpsilon) theUpVector=theUpVector.Perp();

	Vector aXAxis=gMath.Cross(theUpVector,aZAxis);
	aXAxis.Normalize();
	Vector aYAxis=gMath.Cross(aZAxis,aXAxis);

	m00=aXAxis.mX;
	m01=aXAxis.mY;
	m02=aXAxis.mZ;
	m03=0;

	m10=aYAxis.mX;
	m11=aYAxis.mY;
	m12=aYAxis.mZ;
	m13=0;

	m20=aZAxis.mX;
	m21=aZAxis.mY;
	m22=aZAxis.mZ;
	m23=0;

	m30=-gMath.Dot(aXAxis,Vector(0,0,0));
	m31=-gMath.Dot(aYAxis,Vector(0,0,0));
	m32=-gMath.Dot(aZAxis,Vector(0,0,0));
	m33=1.0f;

	return *this;
}


Matrix& Matrix::VectorToVector(Vector theV1, Vector theV2, bool doNormalize)
{
	if (doNormalize) {theV1.Normalize();theV2.Normalize();}

	float aDot=theV1.Dot(theV2);
	if (gMath.Abs(aDot)>=1.0f-gMath.mMachineEpsilon)
	{
		if (aDot>0) return *this;

		//
		// Okay, our problem is that a direct negative crosses to produce an axis of 0,0,0
		// I can't rotate around that... so we have to do a double-step to hit some other angle first (yeesh)
		//

		Vector aAxis;
		float aAngle=0;
		Vector aVMid=theV2.Perp();

		aAxis=gMath.Cross(theV1,aVMid);
		aAngle=gMath.Deg((float)acos(gMath.Dot(theV1,aVMid)));
		RotateAroundAxis3D(aAxis,aAngle);

		aAxis=gMath.Cross(aVMid,theV2);
		aAngle=gMath.Deg((float)acos(gMath.Dot(aVMid,theV2)));
		RotateAroundAxis3D(aAxis,aAngle);
	}
	else
	{
		Vector aAxis;
		float aAngle=0;

		aAxis=gMath.Cross(theV1,theV2);
		aAngle=gMath.Deg((float)acos(gMath.Dot(theV1,theV2)));
		RotateAroundAxis3D(aAxis,aAngle);
	}

	return *this;
}

Matrix& Matrix::VectorToVector(Vector theV1, Vector theV2, Vector theUp)
{
	theV1.Normalize();
	theV2.Normalize();

	float aDot=theV1.Dot(theV2);
	if (gMath.Abs(aDot)>=1.0f-gMath.mMachineEpsilon)
		//if (theV1==theV2 || theV1==-theV2)
	{
		if (aDot<0) 
		{
			Vector aAxis=theV1.Perp();
			if (aAxis.Dot(theUp)<0) aAxis*=-1;
			RotateAroundAxis3D(aAxis,180);
		}
	}
	else
	{
		Vector aAxis;
		float aAngle=0;

		aAxis=gMath.Cross(theV1,theV2);
		if (aAxis.Dot(theUp)<0) aAxis*=-1;
		aAngle=gMath.Deg((float)acos(gMath.Dot(theV1,theV2)));
		RotateAroundAxis3D(aAxis,aAngle);
	}

	return *this;

}


/*
Vector Matrix::GetRow(int theCount)
{
	if (theCount==0) return Vector(m00,m01,m02);
	if (theCount==1) return Vector(m10,m11,m12);
	if (theCount==2) return Vector(m20,m21,m22);
	if (theCount==3) return Vector(m30,m31,m32);
	return Vector(0,0,0);
}
*/

/*
char* Matrix::ToString()
{
	static String aMString;
	aMString="";
	for (int aSpanY=0;aSpanY<4;aSpanY++)
	{
		if (aSpanY>0) aMString+="\n";
		for (int aSpanX=0;aSpanX<4;aSpanX++) aMString+=Sprintf("%f ",mData.m[aSpanX][aSpanY]);
	}
	return aMString.c();
}
*/


void Matrix::MakeOrthographic(float theLeft, float theRight, float theTop, float theBottom, float theNear, float theFar)
{
	m00 = 2 / (theRight - theLeft);
	m01 = 0;
	m02 = 0;
	m03 = 0;
	m10 = 0;
	m11 = 2 / (theTop - theBottom);
	m12 = 0;
	m13 = 0;
	m20 = 0;
	m21 = 0;
	m22 = 2 / (theFar - theNear);
	m23 = 0;
	m30 = -((theRight + theLeft) / (theRight - theLeft));
	m31 = -((theTop + theBottom) / (theTop - theBottom));
	m32 = -((theFar + theNear) / (theFar - theNear));
	m33 = 1;
}

void Matrix::MakePerspective(float theLeft, float theRight, float theTop, float theBottom, float theNear, float theFar)
{
	float aFar=-_max(1.0f,theFar);

	Identity();
	m00=(2.0f)/(theRight-theLeft);
	m10=0;
	m20=0;
	m30=(-theRight-theLeft)/(theRight-theLeft);
	m01=0;
	m11=(2.0f)/(theTop-theBottom);
	m21=0;
	m31=(-theTop-theBottom)/(theTop-theBottom);
	m02=0;
	m12=0;
	m22=-2/(aFar-theNear);
	m32=(-aFar-theNear)/(aFar-theNear);
	m03=0;
	m13=0;
	m23=0;
	m33=1.0f;
}



/*
void Mat4::Ortho(	float theLeft, float theRight, float theTop, float theBottom, float theNear, float theFar)
{
	m00 = 2 / (theRight - theLeft);
	m01 = 0;
	m02 = 0;
	m03 = 0;
	m10 = 0;
	m11 = 2 / (theTop - theBottom);
	m12 = 0;
	m13 = 0;
	m20 = 0;
	m21 = 0;
	m22 = 2 / (theFar - theNear);
	m23 = 0;
	m30 = -((theRight + theLeft) / (theRight - theLeft));
	m31 = -((theTop + theBottom) / (theTop - theBottom));
	m32 = -((theFar + theNear) / (theFar - theNear));
	m33 = 1;
}
*/

/*
Matrix& Matrix::Align(Vector myVector, Vector alignTo)
{
	Vector aAxis=myVector.Cross(alignTo).Normal();
	float aDotProduct=_clamp(-1,myVector.Dot(alignTo),1);
	float aAngleRad=acosf(aDotProduct);
	Matrix aMat;
	aMat.RotateAroundAxis3D(aAxis,gMath.Deg(aAngleRad));
	return Multiply(&aMat);
}
*/

char* Matrix::ToString()
{
	String& aResult=GetDiscardableString();
	aResult="";

	bool aComma=true;
	for (int aCount=0;aCount<16;aCount++)
	{
		aComma=true;
		if ((aCount%4)==0) {if (aCount>0) aResult+=" :: ";aComma=false;}
		if (aComma) {aResult+=",";aComma=false;}
		aResult+=Sprintf("%f",mData.mm[aCount]);
	}

	return aResult.c();
}

Matrix& Matrix::Interpolate(Matrix& theDestination,float theProgress) {for (int aCount=0;aCount<16;aCount++) mData.mm[aCount]=gMath.InterpolateLinear(mData.mm[aCount],theDestination.mData.mm[aCount],theProgress);return *this;}
Matrix&	Matrix::GetInterpolate(Matrix& theDestination,float theProgress) {static Matrix aResult;for (int aCount=0;aCount<16;aCount++) aResult.mData.mm[aCount]=gMath.InterpolateLinear(mData.mm[aCount],theDestination.mData.mm[aCount],theProgress);return aResult;}

Matrix& Matrix::Slerp(Matrix& theDestination,float theProgress)
{
	Vector aT1=GetTranslate();
	Vector aT2=theDestination.GetTranslate();

	Quaternion aQ1=ToQuaternion();
	Quaternion aQ2=theDestination.ToQuaternion();
	Quaternion aQ3=aQ1.GetSlerp(&aQ2,theProgress);

	FromQuaternion(&aQ3);
	Translate(aT1.Lerp(aT2,theProgress));

	return *this;

	/*
	Vector aBase=theDestination.DecomposeR().MajorAxis();
	if (aBase.LengthSquared()==0) aBase=Vector(0,0,-1);

	aBase.Shift();

	Vector aUp1=ProcessPointN(aBase);
	Vector aUp2=theDestination.ProcessPointN(aBase);

	if (aUp1.Dot(aUp2)>=1.0f)
	{
		aBase=Vector(1,0,0);
		aUp1=ProcessPointN(aBase);
		aUp2=theDestination.ProcessPointN(aBase);
	}

	Vector aT1=GetTranslate();
	Vector aT2=theDestination.GetTranslate();

	Vector aRot=aUp1.Slerp(aUp2,theProgress);
	Vector aTran=aT1.Lerp(aT2,theProgress);

	Identity();
	VectorToVector(aBase,aRot,false);
	Translate(aTran);

	return *this;
	/**/
}

Matrix Matrix::GetSlerp(Matrix& theDestination,float theProgress)
{
	/*
	WARNING("UNDO ME!");
	Matrix aM=*this;
	aM.Slerp(theDestination,theProgress);
	return aM;
	/**/

	Vector aT1=GetTranslate();
	Vector aT2=theDestination.GetTranslate();

	Quaternion aQ1=ToQuaternion();
	Quaternion aQ2=theDestination.ToQuaternion();
	Quaternion aQ3=aQ1.GetSlerp(&aQ2,theProgress);

	Matrix aNew;
	aNew.FromQuaternion(&aQ3);
	aNew.Translate(aT1.Lerp(aT2,theProgress));
	return aNew;
}



void Matrix::Debug(char* thePrefix)
{
	gOut.Out("%s%.4f,%.4f,%.4f,%.4f",thePrefix,m00,m01,m02,m03);
	gOut.Out("%s%.4f,%.4f,%.4f,%.4f",thePrefix,m10,m11,m12,m13);
	gOut.Out("%s%.4f,%.4f,%.4f,%.4f",thePrefix,m20,m21,m22,m23);
	gOut.Out("%s%.4f,%.4f,%.4f,%.4f",thePrefix,m30,m31,m32,m33);
}

void Matrix::DebugOneLine(char* thePrefix)
{
	gOut.Out("%s%.4f,%.4f,%.4f,%.4f %.4f,%.4f,%.4f,%.4f %.4f,%.4f,%.4f,%.4f %.4f,%.4f,%.4f,%.4f",thePrefix,m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33);
}

void Matrix::ToVectors(Vector& theXDir, Vector& theYDir, Vector& theZDir)
{
	theXDir=Vector(m00,m01,m02);
	theYDir=Vector(m10,m11,m12);
	theZDir=Vector(m20,m21,m22);
}

void Matrix::FromVectors(Vector& theXDir, Vector& theYDir, Vector& theZDir)
{
	m00=theXDir.mX;m01=theXDir.mY;m02=theXDir.mZ;
	m10=theYDir.mX;m11=theYDir.mY;m12=theYDir.mZ;
	m20=theZDir.mX;m21=theZDir.mY;m22=theZDir.mZ;
}


Matrix Matrix::GetBridgeTo(Matrix* theOther)
{
	if (!theOther) return GetInvert(); // Converts ourself to identity

	Matrix aResult=GetInvert();
	aResult*=*theOther;
	return aResult;
}

Quaternion Matrix::ToQuaternion()
{
	Quaternion aQ;
	float aTrace = m00+m11+m22;

	if (aTrace>0) 
	{
		float aS = sqrtf(aTrace + 1.0f) * 2.0f;
		aQ.mW = 0.25f * aS;
		aQ.mX = (m21 - m12) / aS;
		aQ.mY = (m02 - m20) / aS;
		aQ.mZ = (m10 - m01) / aS;
	} 
	else if ((m00>m11) && (m00>m22)) 
	{
		float aS = sqrtf(1.0f + m00 - m11 - m22) * 2.0f;
		aQ.mW = (m21 - m12) / aS;
		aQ.mX = 0.25f * aS;
		aQ.mY = (m01 + m10) / aS;
		aQ.mZ = (m02 + m20) / aS;
	} 
	else if (m11 > m22) 
	{
		float aS = sqrtf(1.0f + m11 - m00 - m22) * 2.0f;
		aQ.mW = (m02 - m20) / aS;
		aQ.mX = (m01 + m10) / aS;
		aQ.mY = 0.25f * aS;
		aQ.mZ = (m12 + m21) / aS;
	} 
	else 
	{
		float aS = sqrtf(1.0f + m22 - m00 - m11) * 2.0f;
		aQ.mW = (m10 - m01) / aS;
		aQ.mX = (m02 + m20) / aS;
		aQ.mY = (m12 + m21) / aS;
		aQ.mZ = 0.25f * aS;
	}

	return aQ;
}

void  Matrix::FromQuaternion(Quaternion* theSource)
{
	float xx = theSource->mX * theSource->mX;
	float xy = theSource->mX * theSource->mY;
	float xz = theSource->mX * theSource->mZ;
	float xw = theSource->mX * theSource->mW;

	float yy = theSource->mY * theSource->mY;
	float yz = theSource->mY * theSource->mZ;
	float yw = theSource->mY * theSource->mW;

	float zz = theSource->mZ * theSource->mZ;
	float zw = theSource->mZ * theSource->mW;

	m00 = 1 - 2 * (yy + zz);
	m01 = 2 * (xy - zw);
	m02 = 2 * (xz + yw);
	m03 = 0;

	m10 = 2 * (xy + zw);
	m11 = 1 - 2 * (xx + zz);
	m12 = 2 * (yz - xw);
	m13 = 0;

	m20 = 2 * (xz - yw);
	m21 = 2 * (yz + xw);
	m22 = 1 - 2 * (xx + yy);
	m23 = 0;

	m30 = 0;
	m31 = 0;
	m32 = 0;
	m33 = 1;
}

Quaternion Quaternion::GetNormal()
{
	Quaternion aQ;
	float aNorm=sqrtf(mW*mW+mX*mX+mY*mY+mZ*mZ);
	if (aNorm!=0) 
	{
		aQ.mW=mW/aNorm;
		aQ.mX=mX/aNorm;
		aQ.mY=mY/aNorm;
		aQ.mZ=mZ/aNorm;
	} 
	return aQ;
}


Quaternion Quaternion::GetSlerp(Quaternion* theOther, float theAmount)
{
	if (theOther==NULL) return *this;

	//*
	Quaternion aQ1Normalized=GetNormal();
	Quaternion aQ2Normalized=theOther->GetNormal();

	// Calculate the cosine of the angle between the quaternions
	float aCosTheta =	aQ1Normalized.mX * aQ2Normalized.mX +
						aQ1Normalized.mY * aQ2Normalized.mY +
						aQ1Normalized.mZ * aQ2Normalized.mZ +
						aQ1Normalized.mW * aQ2Normalized.mW;

	// If the dot product is negative, negate one of the quaternions to take the shorter path
	if (aCosTheta < 0.0) 
	{
		aQ2Normalized.mX = -aQ2Normalized.mX;
		aQ2Normalized.mY = -aQ2Normalized.mY;
		aQ2Normalized.mZ = -aQ2Normalized.mZ;
		aQ2Normalized.mW = -aQ2Normalized.mW;
		aCosTheta = -aCosTheta;
	}

	// Calculate coefficients
	float aK0;
	float aK1;
	if (aCosTheta > 0.9995f) 
	{
		aK0=1.0f-theAmount;
		aK1=theAmount;
	} 
	else 
	{
		float aTheta = acosf(aCosTheta);
		float aSinTheta = sinf(aTheta);
		aK0 = sinf((1.0f - theAmount) * aTheta) / aSinTheta;
		aK1 = sinf(theAmount * aTheta) / aSinTheta;
	}

	// Interpolate
	Quaternion aResult;
	aResult.mW = aQ1Normalized.mW * aK0 + aQ2Normalized.mW * aK1;
	aResult.mX = aQ1Normalized.mX * aK0 + aQ2Normalized.mX * aK1;
	aResult.mY = aQ1Normalized.mY * aK0 + aQ2Normalized.mY * aK1;
	aResult.mZ = aQ1Normalized.mZ * aK0 + aQ2Normalized.mZ * aK1;
	return aResult;
	/**/

	/*
	float d = Dot(*theOther);
	float s0, s1;
	bool shortPath=true;
	float sd = shortPath ? (d > 0) - (d < 0) : 1.0f;

	d = fabsf(d);

	if (d < 0.9995f)
	{
		float s = sqrtf(1 - d * d);    //   Sine of relative angle
		float a = atan2f(s, d);
		float c = cosf(theAmount*a);


		s1 = sqrtf(1 - c * c) / s;
		s0 = c - d * s1;
	}
	else
	{
		s0 = 1.0f - theAmount;
		s1 = theAmount;
	}

	return *this * s0 + *theOther * sd * s1;
	/**/
}



