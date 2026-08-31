#include "Anim.h"
#include "MyApp.h"
#include "World.h"

extern World *gWorld;

Anim::Anim(void)
{
}

Anim::~Anim(void)
{
}

Anim_Sprite::Anim_Sprite(Sprite *theSprite)
{
	mSprite=theSprite;
	mGravityMult=Point(1,1);
	mDirMult=Point(1,1);
	mFadeMult=1.0f;
	mFade=1.0f;
	mFadeSpeed=0;
	mScaleSpeed=0;
	mColor=Color(1);
	mColorStep=Color(0,0,0,0);
	mWhite=false;
	mScale=1.0f;
	mRotation=0;
	mRotationDir=0;
	mTimer=0;
	mAdditive=false;
	mDoColorStep=false;
}

Anim_Sprite::~Anim_Sprite(void)
{
}

void Anim_Sprite::Update()
{
	mRotation+=mRotationDir;
	mPos+=mDir;
	mDir*=mDirMult;
	mDir+=mGravity;
	mDir*=mGravityMult;

	mScale+=mScaleSpeed;
	mFade+=mFadeSpeed;
	mFade*=mFadeMult;
	if (mFade<=0) Kill();

	if (mDoColorStep)
	{
		mColor+=mColorStep;
		mColor=mColor.Clip();
	}


	if (mTimer>0) if (--mTimer==0) Kill();

}

void Anim_Sprite::Draw()
{
	if (mAdditive) gG.RenderAdditive();
	gG.SetColor(mColor,_min(_max(mFade,0),1.0f));
	if (mWhite) gG.RenderWhite(true);

	Matrix aMat;
	if (mScale!=1.0f) aMat.Scale(mScale);
	if (mRotation!=0) aMat.Rotate2D(mRotation);
	aMat.Translate(mPos);
	mSprite->Draw(aMat);

	if (mWhite) gG.RenderWhite(false);
	gG.SetColor();
	if (mAdditive) gG.RenderNormal();

}

Anim_ExplodePop::Anim_ExplodePop(Point thePos)
{
	mPos=thePos+(gMath.AngleToVector()*gRand.GetF(10));

	mFade1=1.0f;
	mFade2=1.0f;
	mWhiten=1.0f;
	mOrbit=gRand.GetF(360);
	mOrbitSpeed=6+gRand.GetSpanF(2);
	mScale=1.0f;
	mTotalScale=1.0f+gRand.GetF(.25f);
}

void Anim_ExplodePop::Update()
{
	mOrbit+=mOrbitSpeed;
	mScale+=.01f;
	mWhiten=_max(0,mWhiten-.2f);
	mFade1=_max(0,mFade1-.1f);
	mFade2=_max(0,mFade2-.05f);
	if (mFade2<=0) Kill();
}

void Anim_ExplodePop::Draw()
{
	gG.SetColor(mFade1);
	gBundle_Play->mExplode1.DrawScaled(mPos,mTotalScale);
	gG.SetColor(mFade2);
	gBundle_Play->mExplode2.DrawRotatedScaled(mPos,mOrbit,mScale*mTotalScale);
	if (mWhiten>0)
	{
		gG.RenderWhite(true);
		gG.SetColor(mWhiten*mFade1);
		gBundle_Play->mExplode1.DrawScaled(mPos,mTotalScale);
		gG.SetColor(_min(1.0f,mWhiten*mFade2*2));
		gBundle_Play->mExplode2.DrawRotatedScaled(mPos,mOrbit,mScale*mTotalScale);
		gG.RenderWhite(false);
	}
	gG.SetColor();


}

Anim_Glob::Anim_Glob(Point thePos, Sprite *theSprite)
{
	mSprite=theSprite;
	mPos=thePos;
	mWait=10+gRand.Get(20);
	mScale=.5f+gRand.GetF(.65f);
	mMelt=1.0f;
	mMeltSpeed=.01f+gRand.GetF(.015f);
}

void Anim_Glob::Update()
{
	if (--mWait<=0)
	{
		mMelt-=mMeltSpeed;
		if (mMelt<=0) Kill();
	}
}

void Anim_Glob::Draw()
{
	Matrix aMat;
	aMat.Scale(mScale,mMelt*mScale,mScale);
	aMat.Translate(mPos);
	mSprite->Draw(aMat);
}

Anim_SpriteStrip::Anim_SpriteStrip(Array<Sprite> *theStrip) : Anim_Sprite(&theStrip->Element(0))
{
	mStrip=theStrip;
	mMaxFrame=(float)theStrip->Size();
	mFrame=0;
	mFrameSpeed=.5f;
	mLoop=false;
}

void Anim_SpriteStrip::Update()
{
	Anim_Sprite::Update();
	mFrame+=mFrameSpeed;
	if (mFrame>=mMaxFrame)
	{
		if (mLoop) mFrame-=mMaxFrame;
		else Kill();
	}
}

void Anim_SpriteStrip::Draw()
{
	mSprite=&mStrip->Element((int)mFrame);
	Anim_Sprite::Draw();
}

Anim_Bubble::Anim_Bubble(Point thePos)
{
	mPos=thePos;
	mPos.mY+=15;
	mScale=.75f+gRand.GetF(.5f);
	mPos.mX+=gRand.GetSpanF(10/mScale);
	mSpeed=1.0f+gRand.GetF(2.0f);
	mSpeed*=.5f;
	mFade=0.0f;
	mType=0;
	mPopOffset=0;
}

void Anim_Bubble::Update()
{
	mPos.mY-=mSpeed;
	mSpeed+=.025f;

	IPoint aGPos=gWorld->WorldToGrid(mPos.mX,mPos.mY-3+mPopOffset);
	World::Grid *aG=gWorld->GetGridPtr(aGPos.mX,aGPos.mY);
	if (!aG) Kill();
	else if (aG->mLayout!=2) 
	{
		gSounds->mBubble[gRand.Get(2)].PlayPitched(1.0f+gRand.GetF(.25f),gWorld->GetVolume(mPos));
		Kill();
	}

	mFade=_min(1.0f,mFade+.025f);
}

void Anim_Bubble::Draw()
{
	gG.SetColor(mFade);
	gBundle_Play->mBubble[mType].DrawScaled(mPos,mScale);
	gG.SetColor();
}

Anim_Evaporate::Anim_Evaporate(Sprite *theSprite, Point thePos)
{
	mPos=thePos;
	mSprite=theSprite;
	mWhiten=0;
	mFade=1.0f;
}

void Anim_Evaporate::Update()
{
	mWhiten=_min(1.0f,mWhiten+.05f);
	if (mWhiten>=1.0f)
	{
		mFade-=.015f;
		if (mFade<=0) Kill();
	}
}

void Anim_Evaporate::Draw()
{
	if (mWhiten<1.0f)
	{
		mSprite->Center(mPos);
	}

	gG.RenderWhite(true);
	gG.SetColor(mWhiten*mFade);
	mSprite->Center(mPos);
	gG.SetColor();
	gG.RenderWhite(false);
}

Anim_GooSink::Anim_GooSink(Point theRobotPos, IPoint theGooPos)
{
	mPos=theRobotPos;

	mGooLeft=-999999;
	mGooRight=999999;

	mRobot.mFacing=gWorld->mRobot->mFacing;
	mRobot.mHasHelmet=gWorld->mRobot->mHasHelmet;

	if (gWorld->GetGrid(theGooPos.mX-1,theGooPos.mY)!=2)
	{
		float aWantX=gWorld->GridToWorld(theGooPos).mX;
		mPos.mX=_max(mPos.mX,aWantX);
		mGooLeft=aWantX-gWorld->mHalfGrid.mX;
		mGooLeft+=10;
	}
	if (gWorld->GetGrid(theGooPos.mX+1,theGooPos.mY)!=2)
	{
		float aWantX=gWorld->GridToWorld(theGooPos).mX;
		mPos.mX=_min(mPos.mX,aWantX);
		mGooRight=aWantX+gWorld->mHalfGrid.mX;
		mGooRight-=10;
	}

	World::Grid *aGG=gWorld->GetGridPtr(theGooPos.mX,theGooPos.mY);
	mGooType=0;
	if (aGG->mPaint>46+47+47+47 && aGG->mPaint<=46+47+47+47+47)	mGooType=0; // Red Goo
	if (aGG->mPaint>46+47+47+47+47 && aGG->mPaint<=46+47+47+47+47+47)	mGooType=1; // Green Goo

	Point aP=gWorld->GridToWorld(theGooPos);
	mClipY=aP.mY+gWorld->mHalfGrid.mY;
	mGooTop=aP.mY-gWorld->mHalfGrid.mY+5;
	mSinkSpeed=1.0f;
}

void Anim_GooSink::Update()
{
	mPos.mY+=mSinkSpeed;
	mSinkSpeed+=.05f;
	if (mPos.mY>mClipY) Kill();

	for (int aCount=0;aCount<4;aCount++)
	{
		Point aPos=Point(_min(mGooRight,_max(mGooLeft,mPos.mX+gRand.GetSpanF(30))),mGooTop+gRand.GetF(10));
		Anim_Bubble *aB=new Anim_Bubble(aPos);//mClipY-30-gRand.GetF(20,true)));
		aB->mType=mGooType;
		aB->mFade=.5f;
		aB->mPopOffset=(1.0f-((float)fabs(aPos.mX-mPos.mX)/30))*5;
		aB->mScale*=1.25f;//=1.0f-gRand.GetF(.1f);
		aB->mPos=aPos;
		gWorld->mAnimatorList_Top+=aB;
	}
}

void Anim_GooSink::Draw()
{
	gG.PushClip();
	gG.Clip(mPos.mX-100.0f,mPos.mY-100,1000.0f,mClipY-40-(mPos.mY-100));
	mRobot.mPos=mPos;
	mRobot.DrawBot();
	gG.PopClip();
}

float gXDir=1;

Anim_BouncyBit::Anim_BouncyBit(Sprite *theSprite, Point thePos)
{
	mSprite=theSprite;
	mPos=thePos;

	mRotate=gRand.GetF(360);

	mXDir=gXDir;
	gXDir*=-1;

	mRotateSpeed=.5f+gRand.GetF(25);
	mRotateSpeed*=mXDir;

	mXDir*=.5f+gRand.GetF(3.5f);
	mXDir*=2.0f;
	mBounceMax=-(gRand.GetF(5)+5);
	mBounceMax*=.35f;
	mBounce=mBounceMax;
	mFade=12.0f+gRand.GetF(4);
	mScale=1.0f;

}

void Anim_BouncyBit::Update()
{
	mPos.mX+=mXDir;
	mPos.mY+=mBounce;
	mBounce+=.2f;
	mRotate+=mRotateSpeed;

	//
	// Hit ground?  Bounce up!  (or bounce X)
	//
	IPoint aTest=gWorld->WorldToGrid(mPos+Point(0,7));
	bool aPlink=false;
	if (gWorld->GetGrid(aTest.mX,aTest.mY)!=0)
	{
		mXDir*=.75f;
		mBounce=mBounceMax;
		aPlink=true;
	}
	aTest=gWorld->WorldToGrid(mPos+Point(7,0));
	if (gWorld->GetGrid(aTest.mX,aTest.mY)!=0) {mXDir=-(float)fabs(mXDir)*.5f;aPlink=true;}
	aTest=gWorld->WorldToGrid(mPos+Point(-7,0));
	if (gWorld->GetGrid(aTest.mX,aTest.mY)!=0) {mXDir=(float)fabs(mXDir)*.5f;aPlink=true;}
	aTest=gWorld->WorldToGrid(mPos+Point(0,-7));
	if (gWorld->GetGrid(aTest.mX,aTest.mY)!=0) {mBounce=(float)fabs(mBounce);aPlink=true;}
	if (aPlink)gSounds->mPlink.PlayPitched(1.0f+gRand.GetF(.1f),gWorld->GetVolume(mPos));


	mFade-=.15f;
	if (mFade<=0) Kill();

}

void Anim_BouncyBit::Draw()
{
	gG.SetColor(mFade);
	mSprite->DrawRotatedScaled(mPos,mRotate,mScale);
	gG.SetColor();
}

int gWakeSound=-1;
Anim_Wake::Anim_Wake(int theType, Point thePos)
{
	mPos=thePos;
	mSprite=&gBundle_Play->mSplash[theType];
	mScale=0;
	mScaleCount=0;

	if (gApp.AppTime()>gWakeSound+5) 
	{
		gSounds->mWake.PlayPitched(1.0f+gRand.GetF(.5f));
		gWakeSound=gApp.AppTime();
	}

	mModScale=1.0f-gRand.GetF(.5f);
}

void Anim_Wake::Update()
{
	mScaleCount+=20;
	if (mScaleCount>90) mScaleCount-=10;
	mScale=gMath.Sin(mScaleCount);
	if (mScaleCount>=180) Kill();
}

void Anim_Wake::Draw()
{
	Matrix aMat;
	aMat.Scale(1,mScale*mModScale,1);
	aMat.Translate(mPos);
	mSprite->Draw(aMat);
}


Anim_GooSinkKitty::Anim_GooSinkKitty(Point theRobotPos, IPoint theGooPos)
{
	mPos=theRobotPos;

	mGooLeft=-999999;
	mGooRight=999999;

	if (gWorld->GetGrid(theGooPos.mX-1,theGooPos.mY)!=2)
	{
		float aWantX=gWorld->GridToWorld(theGooPos).mX;
		mPos.mX=_max(mPos.mX,aWantX);
		mGooLeft=aWantX-gWorld->mHalfGrid.mX;
		mGooLeft+=10;
	}
	if (gWorld->GetGrid(theGooPos.mX+1,theGooPos.mY)!=2)
	{
		float aWantX=gWorld->GridToWorld(theGooPos).mX;
		mPos.mX=_min(mPos.mX,aWantX);
		mGooRight=aWantX+gWorld->mHalfGrid.mX;
		mGooRight-=10;
	}

	World::Grid *aGG=gWorld->GetGridPtr(theGooPos.mX,theGooPos.mY);
	mGooType=0;
	if (aGG->mPaint>46+47+47+47 && aGG->mPaint<=46+47+47+47+47)	mGooType=0; // Red Goo
	if (aGG->mPaint>46+47+47+47+47 && aGG->mPaint<=46+47+47+47+47+47)	mGooType=1; // Green Goo

	Point aP=gWorld->GridToWorld(theGooPos);
	mClipY=aP.mY+gWorld->mHalfGrid.mY;
	mGooTop=aP.mY-gWorld->mHalfGrid.mY+5;
	mSinkSpeed=1.0f;
}

void Anim_GooSinkKitty::Update()
{
	mPos.mY+=mSinkSpeed;
	mSinkSpeed+=.05f;
	if (mPos.mY>mClipY) Kill();

	for (int aCount=0;aCount<4;aCount++)
	{
		Point aPos=Point(_min(mGooRight,_max(mGooLeft,mPos.mX+gRand.GetSpanF(30))),mGooTop+gRand.GetF(10));
		Anim_Bubble *aB=new Anim_Bubble(aPos);//mClipY-30-gRand.GetF(20,true)));
		aB->mType=mGooType;
		aB->mFade=.5f;
		aB->mPopOffset=(1.0f-((float)fabs(aPos.mX-mPos.mX)/30))*5;
		aB->mScale*=1.25f;//=1.0f-gRand.GetF(.1f);
		aB->mPos=aPos;
		gWorld->mAnimatorList_Top+=aB;
	}
}

void Anim_GooSinkKitty::Draw()
{
	gG.PushClip();
	gG.Clip(mPos.mX-100.0f,mPos.mY-100,1000.0f,mClipY-40-(mPos.mY-100));
	mKitty.mPos=mPos;
	mKitty.Draw();
	gG.PopClip();
}


Anim_GooSinkRedguy::Anim_GooSinkRedguy(Point theRobotPos, IPoint theGooPos)
{
	mPos=theRobotPos;

	mGooLeft=-999999;
	mGooRight=999999;

	if (gWorld->GetGrid(theGooPos.mX-1,theGooPos.mY)!=2)
	{
		float aWantX=gWorld->GridToWorld(theGooPos).mX;
		mPos.mX=_max(mPos.mX,aWantX);
		mGooLeft=aWantX-gWorld->mHalfGrid.mX;
		mGooLeft+=10;
	}
	if (gWorld->GetGrid(theGooPos.mX+1,theGooPos.mY)!=2)
	{
		float aWantX=gWorld->GridToWorld(theGooPos).mX;
		mPos.mX=_min(mPos.mX,aWantX);
		mGooRight=aWantX+gWorld->mHalfGrid.mX;
		mGooRight-=10;
	}

	World::Grid *aGG=gWorld->GetGridPtr(theGooPos.mX,theGooPos.mY);
	mGooType=0;
	if (aGG->mPaint>46+47+47+47 && aGG->mPaint<=46+47+47+47+47)	mGooType=0; // Red Goo
	if (aGG->mPaint>46+47+47+47+47 && aGG->mPaint<=46+47+47+47+47+47)	mGooType=1; // Green Goo

	Point aP=gWorld->GridToWorld(theGooPos);
	mClipY=aP.mY+gWorld->mHalfGrid.mY;
	mGooTop=aP.mY-gWorld->mHalfGrid.mY+5;
	mSinkSpeed=1.0f;
}

void Anim_GooSinkRedguy::Update()
{
	mPos.mY+=mSinkSpeed;
	mSinkSpeed+=.05f;
	if (mPos.mY>mClipY) Kill();

	for (int aCount=0;aCount<4;aCount++)
	{
		Point aPos=Point(_min(mGooRight,_max(mGooLeft,mPos.mX+gRand.GetSpanF(30))),mGooTop+gRand.GetF(10));
		Anim_Bubble *aB=new Anim_Bubble(aPos);//mClipY-30-gRand.GetF(20,true)));
		aB->mType=mGooType;
		aB->mFade=.5f;
		aB->mPopOffset=(1.0f-((float)fabs(aPos.mX-mPos.mX)/30))*5;
		aB->mScale*=1.25f;//=1.0f-gRand.GetF(.1f);
		aB->mPos=aPos;
		gWorld->mAnimatorList_Top+=aB;
	}
}

void Anim_GooSinkRedguy::Draw()
{
	gG.PushClip();
	gG.Clip(mPos.mX-100.0f,mPos.mY-100,1000.0f,mClipY-40-(mPos.mY-100));
	mKitty.mPos=mPos;
	mKitty.Draw();
	gG.PopClip();
}


