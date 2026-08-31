#include "rapt_point.h"
#include "rapt.h"
#include "rapt_rect.h"

Point::Point(const Vector& theVector)
{
	mX=theVector.mX;
	mY=theVector.mY;
}

Point& Point::operator=(Vector &theVector)
{
	mX=theVector.mX;
	mY=theVector.mY;
	return *this;
}


Point::Point(const IPoint &thePoint)
{
	mX=(float)thePoint.mX;
	mY=(float)thePoint.mY;
}

/*
void Point::SetLengthFast(const float theLen)
{
	float aScale=gMath.FastSqrt(LengthSquared());
	aScale=theLen/aScale;
	*this*=aScale;
}
 */

char* Point::ToString(int thePrecision)
{
	String& aResult=GetDiscardableString();
	switch (thePrecision)
	{
	case 0:aResult=Sprintf("%.0f , %.0f",mX,mY);break;
	case 1:aResult=Sprintf("%.1f , %.1f",mX,mY);break;
	case 2:aResult=Sprintf("%.2f , %.2f",mX,mY);break;
	case 3:aResult=Sprintf("%.2f , %.2f",mX,mY);break;
	default:aResult=Sprintf("%.4f , %.4f",mX,mY);break;
	}
	return aResult.c();
}

#ifdef _DEBUG
char* FloatToString(float theFloat)
{
	static char aCC[25];
	sprintf(aCC,"%10.6f",theFloat);
	return aCC;
}

char* Point::ToCString()
{

	String& aResult=GetDiscardableString();
	aResult=FloatToString(mX);
	aResult+="f,";
	aResult+=FloatToString(mY);
	aResult+="f";
	return aResult.c();
}

char* Vector::ToCString()
{

	String& aResult=GetDiscardableString();
	aResult=FloatToString(mX);
	aResult+="f,";
	aResult+=FloatToString(mY);
	aResult+="f,";
	aResult+=FloatToString(mZ);
	aResult+="f";
	return aResult.c();
}

#endif

char* IPoint::ToString(int thePrecision)
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%d,%d",mX,mY);
	return aResult.c();
}

char* Vector::ToString(int thePrecision)
{
	String& aResult=GetDiscardableString();
	switch (thePrecision)
	{
	case 0:aResult=Sprintf("%.0f,%.0f,%.0f",mX,mY,mZ);break;
	case 1:aResult=Sprintf("%.1f,%.1f,%.1f",mX,mY,mZ);break;
	case 2:aResult=Sprintf("%.2f,%.2f,%.2f",mX,mY,mZ);break;
	case 3:aResult=Sprintf("%.2f,%.2f,%.3f",mX,mY,mZ);break;
	default:aResult=Sprintf("%f,%f,%f",mX,mY,mZ);break;
	}
	return aResult.c();
}

char* VectorW::ToString(int thePrecision)
{
	String& aResult=GetDiscardableString();
	switch (thePrecision)
	{
		case 0:aResult=Sprintf("%.0f,%.0f,%.0f,%.0f",mX,mY,mZ,mW);break;
		case 1:aResult=Sprintf("%.1f,%.1f,%.1f,%.1f",mX,mY,mZ,mW);break;
		case 2:aResult=Sprintf("%.2f,%.2f,%.2f,%.2f",mX,mY,mZ,mW);break;
		case 3:aResult=Sprintf("%.2f,%.2f,%.3f,%.3f",mX,mY,mZ,mW);break;
		default:aResult=Sprintf("%f,%f,%f,%f",mX,mY,mZ,mW);break;
	}
	return aResult.c();
}

char* IVector::ToString()
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%d,%d,%d",mX,mY,mZ);
	return aResult.c();
}


/*
float Vector::FastLength()
{
	float aLen=LengthSquared();
	if (aLen<=0) return 0;
	return gMath.FastSqrt(aLen);
}

double DVector::FastLength()
{
	double aLen=LengthSquared();
	if (aLen<=0) return 0;
	return gMath.FastSqrt(aLen);
}
 */

Vector Vector::MajorAxis()
{
	float aX=(float)fabs(mX);
	float aY=(float)fabs(mY);
	float aZ=(float)fabs(mZ);

	if (aX>=aY && aX>=aZ) return Vector(gMath.Sign(mX),0.0f,0.0f);
	if (aY>=aX && aY>=aZ) return Vector(0.0f,gMath.Sign(mY),0.0f);
	if (aZ>=aX && aZ>=aY) return Vector(0.0f,0.0f,gMath.Sign(mZ));
	return *this;
}

void SuperBits::Reset()
{
	mBits.Reset(0);
}

void SuperBits::Set(char* theString)
{
	String aString=theString;
	String aToken=aString.GetToken(',');
	while (aToken.Len())
	{
		Set(aToken.ToInt());
		aToken=aString.GetNextToken(',');
	}
}

void SuperBits::Set(short theFlag)
{
	if (theFlag==0) return;

	int aArraySlot=theFlag/8;
	int aFlagShift=(theFlag-1)%8;

	mBits[aArraySlot]|=1<<aFlagShift;
}

void SuperBits::Unset(short theFlag)
{
	if (theFlag==0) return;

	int aArraySlot=theFlag/8;
	int aFlagShift=(theFlag-1)%8;

	mBits[aArraySlot]|=1<<aFlagShift;
	mBits[aArraySlot]^=1<<aFlagShift;
}

bool SuperBits::IsSet(short theFlag)
{
	if (theFlag==0) return false;

	int aArraySlot=theFlag/8;
	int aFlagShift=(theFlag-1)%8;

	return ((mBits[aArraySlot]&(1<<aFlagShift))!=0);
}

Vector Vector::Bounce(const Vector& theNormal)
{
	Vector aResult=-2*(Dot(theNormal))*theNormal+(*this);
	return aResult;
}

bool Plane::IsSame(Plane theOther, float theThreshold)	// Same plane
{
	if (mNormal.Dot(theOther.mNormal)<1.0f-gMath.mMachineEpsilon) return false;
	if (theOther.DistanceTo(mPos)<=theThreshold) return true;
	return false;
}
bool Plane::IsCoplanar(Plane theOther, float theThreshold) // Same plane, possibly reversed normal
{
	if (fabs(mNormal.Dot(theOther.mNormal))<1.0f-gMath.mMachineEpsilon) return false;
	if (theOther.SignedDistanceTo(mPos)<=theThreshold) return true;
	return false;
}
bool Plane::IsOpposite(Plane theOther, float theThreshold)	// Same plane, opposite normals
{
	if (mNormal.Dot(theOther.mNormal)>=-1.0f+gMath.mMachineEpsilon) return false;
	if (theOther.DistanceTo(mPos)<=theThreshold) return true;
	return false;
}

Vector Plane::Planify(Vector thePos)
{
	thePos-=mPos;

	Matrix aMat;
	aMat.VectorToVector(mNormal,Vector(0,0,1));
	return aMat.ProcessPoint(thePos);
}

Vector Plane::PlanifyN(Vector thePos)
{
	Matrix aMat;
	aMat.VectorToVector(mNormal,Vector(0,0,1));
	return aMat.ProcessPoint(thePos);
}

Vector Plane::UnPlanify(Vector thePos)
{
	Matrix aMat;
	aMat.VectorToVector(Vector(0,0,1),mNormal);
	return aMat.ProcessPoint(thePos)+mPos;
}

Vector Plane::UnPlanifyN(Vector thePos)
{
	Matrix aMat;
	aMat.VectorToVector(Vector(0,0,1),mNormal);
	return aMat.ProcessPoint(thePos);
}

void Plane::Planify(Array<Vector>& theList)
{
	Matrix aMat;
	aMat.VectorToVector(mNormal,Vector(0,0,1));
	for (int aCount=0;aCount<theList.Size();aCount++) theList[aCount]=aMat.ProcessPoint(theList[aCount]-mPos);
}

void Plane::UnPlanify(Array<Vector>& theList)
{
	Matrix aMat;
	aMat.VectorToVector(Vector(0,0,1),mNormal);
	for (int aCount=0;aCount<theList.Size();aCount++) theList[aCount]=aMat.ProcessPoint(theList[aCount])+mPos;
}

void Plane::Planify(Vector* theList, int theCount)
{
	Matrix aMat;
	aMat.VectorToVector(mNormal,Vector(0,0,1));
	for (int aCount=0;aCount<theCount;aCount++) theList[aCount]=aMat.ProcessPoint(theList[aCount]-mPos);
}

void Plane::UnPlanify(Vector* theList, int theCount)
{
	Matrix aMat;
	aMat.VectorToVector(Vector(0,0,1),mNormal);
	for (int aCount=0;aCount<theCount;aCount++) theList[aCount]=aMat.ProcessPoint(theList[aCount])+mPos;
}

void Plane::UnPlanify(Array<Point>& theInList, Array<Vector>& theOutList)
{
	Matrix aMat;
	aMat.VectorToVector(Vector(0,0,1),mNormal);
	theOutList.Reset();
	theOutList.GuaranteeSize(theInList.Size());
	for (int aCount=0;aCount<theInList.Size();aCount++) theOutList[aCount]=aMat.ProcessPoint(theInList[aCount])+mPos;
}


void Plane::Planify(Array<Vector>& theList,Array<Point>& theResult, float theSnap)
{
	theResult.ForceSize(theList.Size());
	Matrix aMat;
	aMat.VectorToVector(mNormal,Vector(0,0,1));
	for (int aCount=0;aCount<theList.Size();aCount++) 
	{
		theResult[aCount]=aMat.ProcessPoint(theList[aCount]-mPos).ToPoint();
		if (theSnap!=0)
		{
			Utils::Snap(theResult[aCount].mX,theSnap);
			Utils::Snap(theResult[aCount].mY,theSnap);
		}
	}
}

Vector Plane::ProjectOnto(Vector thePos)
{
	Vector aP=thePos.Dot(mNormal)/mNormal.Dot(mNormal)*mNormal;
	return Vector(thePos.mX-aP.mX,thePos.mY-aP.mY,thePos.mZ-aP.mZ);
}



bool Plane::GetSurfaceVectors(Vector& theResult1, Vector& theResult2)
{
	Vector aUseVector=Vector(1,0,0);
	if (gMath.Abs(mNormal.Dot(aUseVector))>=.99999f)
	{
		aUseVector=Vector(0,1,0);
		if (gMath.Abs(mNormal.Dot(aUseVector))>=.99999f) aUseVector=Vector(0,0,1);
	}
	Vector aPos1=mPos+aUseVector;
	Vector aPos2=mPos-aUseVector;
	Vector aOther1;
	Vector aOther2;
	gMath.DoesInfiniteLineIntersectPlane(aPos1,aPos1+mNormal,*this,&aOther1);
	gMath.DoesInfiniteLineIntersectPlane(aPos2,aPos2+mNormal,*this,&aOther2);

	theResult1=::Normalize(aOther1-aOther2);

	Matrix aMat;
	aMat.RotateAroundAxis3D(mNormal,90);
	theResult2=aMat.ProcessPoint(theResult1);


	return true;
}

Point Point::RMod(float theNumber) {return Point((float)gMath.RMod(mX,theNumber),(float)gMath.RMod(mY,theNumber));}
Point Point::RMod(Point theNumber) {return Point((float)gMath.RMod(mX,theNumber.mX),(float)gMath.RMod(mY,theNumber.mY));}

void Point::Write(IOBuffer& theBuffer) {theBuffer.WriteFloat(mX);theBuffer.WriteFloat(mY);}
void Point::Read(IOBuffer& theBuffer) {mX=theBuffer.ReadFloat();mY=theBuffer.ReadFloat();}

void Vector::Write(IOBuffer& theBuffer) {theBuffer.WriteFloat(mX);theBuffer.WriteFloat(mY);theBuffer.WriteFloat(mZ);}
void Vector::Read(IOBuffer& theBuffer) {mX=theBuffer.ReadFloat();mY=theBuffer.ReadFloat();mZ=theBuffer.ReadFloat();}

void IVector::Write(IOBuffer& theBuffer) {theBuffer.WriteInt(mX);theBuffer.WriteInt(mY);theBuffer.WriteInt(mZ);}
void IVector::Read(IOBuffer& theBuffer) {mX=theBuffer.ReadInt();mY=theBuffer.ReadInt();mZ=theBuffer.ReadInt();}

void DVector::Write(IOBuffer& theBuffer) {theBuffer.WriteDouble(mX);theBuffer.WriteDouble(mY);theBuffer.WriteDouble(mZ);}
void DVector::Read(IOBuffer& theBuffer) {mX=theBuffer.ReadDouble();mY=theBuffer.ReadDouble();mZ=theBuffer.ReadDouble();}

Point& Point::Randomize(float theMagnitude, bool doNormalize)
{
	mX=gRand.GetSpanF(-theMagnitude,theMagnitude);
	mY=gRand.GetSpanF(-theMagnitude,theMagnitude);
	if (doNormalize) Normalize();
	return *this;
}

Vector& Vector::Randomize(float theMagnitude, bool doNormalize)
{
	mX=gRand.GetSpanF(-theMagnitude,theMagnitude);
	mY=gRand.GetSpanF(-theMagnitude,theMagnitude);
	mZ=gRand.GetSpanF(-theMagnitude,theMagnitude);
	if (doNormalize) Normalize();
	return *this;
}

//
// Unity SLERP
// https://forum.unity.com/threads/slerp-demystified.1328832/
//
Vector Vector::Slerp(Vector theOther, float t) 
{
	Vector aV0=GetNormal();
	Vector aV1=theOther.GetNormal();
	float aDotProduct = aV0.Dot(aV1);

	aDotProduct=_min(1.0f,_max(-1.0f,aDotProduct));

	if (aDotProduct>=1.0f-.0001f) return aV0.Lerp(aV1,t);
	if (aDotProduct<=(-1.0f+.0001f))
	{
		Vector aAxis=aV0.FastOrtho(false);

		Matrix aM;
		aM.RotateAroundAxis3D(aAxis,180*t);
		Vector aSlerped=aM.ProcessPoint(aV0);
		return aSlerped;		
	}

	float aTheta=acosf(aDotProduct);
	float aSinTheta=sinf(aTheta);
	float aA=sinf((1 - t) * aTheta)/aSinTheta;
	float aB=sinf(t*aTheta)/aSinTheta;

	// Perform the interpolation
	return Vector(
		aA * aV0.mX + aB * aV1.mX,
		aA * aV0.mY + aB * aV1.mY,
		aA * aV0.mZ + aB * aV1.mZ
	);
}
