#include "Player.h"
#include "MyApp.h"
#include "World.h"
#include "Anim.h"
#include "Game.h"

#define ROBOTGROUND 22
#define ROCKETMAX 120
//#define ROBOTEXPAND 5
#define ROBOTEXPAND 2.5f
#define MAXSPEED (4.0f)

#define TOUCHEDGROUND {mTouchingGround++;mJumpKludge=10;mZoinging=false;}

#ifdef _DEBUG
Player* gCurrentPlayer=NULL;
#endif


bool	gHelmetOuch=false;
int		gRedChildren=0;

Point gOuchDir;	// Just so we can know what direction an ouch came from
Player::Player(void)
{
	mID=ID_NULL;
	mHP=1;
	mIsEnemy=false;
	mSleep=0;
	mDying=false;
	mCollidedWithLava=false;
	mFirstUpdate=false;
	mEnergyLevel=0;
}

Player::~Player(void)
{
}

void Player::CheckLive()
{
	if (!mIsEnemy) return;

	if (!gWorld->mLiveRect.ContainsPoint(mPos.mX,mPos.mY))
	{
		mSleep++;
		if ((mSleep%10)==0)
		{
			bool aCanSleep=true;
			EnumList(Player,aP,gWorld->mPlayerList)
			{
				if (aP==this) continue;
				if (aP->mSleep==0) continue;
				if (aP->mIsEnemy)
				{
					Point aVec=aP->mPos-mPos;
					if (aVec.LengthSquared()<=Squared(gWorld->mGridSize*3))
					{
						aCanSleep=false;
						break;
					}
				}
			}

			if (aCanSleep)
			{
				gWorld->mPlayerList-=this;
				gWorld->mPausedPlayerList+=this;
			}
		}
	}
	else mSleep=0;
}

void Player::EatEnergyAt(int x, int y)
{
	if (mEnergyLevel>=3) return;
	World::Grid* aG=gWorld->GetGridPtr(x,y);

	if (aG && aG->mLayout==MONSTERENERGY)
	{
		aG->mLayout=0;
		EnumList(Player,aP,gWorld->mPlayerList)
		{
			if (aP->mID==ID_MONSTERENERGY)
			{
				if (aP->mGridPos.mY==0) aP->mGridPos=gWorld->WorldToGrid(aP->mPos);
				if (aP->mGridPos.mX==x && aP->mGridPos.mY==y) 
				{
					aP->Kill();
					Energize();
				}
			}
		}
	}
}

void Player::EatEnergy()
{
	if (mEnergyLevel>=3) return;
	EatEnergyAt(mGridPos.mX,mGridPos.mY);
}

void Player::Update()
{
	gWorld->mCollidedWithLava=false;
	mCollidedWithLava=false;
#ifdef _DEBUG
	gCurrentPlayer=this;
#endif
	//mPreviousGridPos=mGridPos;
	if (mIsEnemy || mID==ID_ACID)
	{
		IPoint aGridPos=gWorld->WorldToGrid(mPos);
		if (aGridPos.mX!=mGridPos.mX || aGridPos.mY!=mGridPos.mY)
		{
			mGridPos=aGridPos;
			EatEnergy();
			CheckLive();
		}
	}
	CheckMyFeet();
}

void Player::CheckMyFeet()
{
	IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(0,22));
	World::Grid* aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) Move(Point(-1,0),CONVEYOR_SPEED);
		if (aG->mLayout==CONVEYOR_RIGHT) Move(Point(1,0),CONVEYOR_SPEED);
	}
}

void Robot::CheckMyFeet()
{
	if (mTouchingGround==0) return;

	IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(0,22));
	World::Grid* aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	World::Grid* aZG=NULL;
	IPoint aZGPos;
	float aMoveDir=0;
	int aZoingCount=0;
	if (aG && aG->mLayout==ZOING_BLOCK && aG->mExtraData==1) 
	{
		aZG=aG;
		aZGPos=aGridPos;
	}

	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}
	aGridPos=gWorld->WorldToGrid(mPos+Point(-10,22));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}
	if (!aZG && aG && aG->mLayout==ZOING_BLOCK && aG->mExtraData==1) 
	{
		aZG=aG;
		aZGPos=aGridPos;
	}

	aGridPos=gWorld->WorldToGrid(mPos+Point(+10,22));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}
	if (!aZG && aG && aG->mLayout==ZOING_BLOCK && aG->mExtraData==1) 
	{
		aZG=aG;
		aZGPos=aGridPos;
	}

	if (aMoveDir) 
	{
		Move(Point(gMath.Sign(aMoveDir),0.0f),CONVEYOR_SPEED);
	}

	if (mGravity>=0)
	{
		IPoint aHoldZGPos=aZGPos;
		if (aZG)
		{
			aZoingCount=1;
			for (int aCount=0;aCount<10;aCount++)
			{
				aZGPos.mY++;
				World::Grid* aG=gWorld->GetGridPtr(aZGPos.mX,aZGPos.mY);
				if (aG && aG->mLayout==ZOING_BLOCK) aZoingCount++;
				else break;
			}
		}
		if (aZoingCount) 
		{
			if (aMoveDir) 
			{
				mSpeed=_max(mSpeed,CONVEYOR_SPEED);
				mFacing=gMath.Sign(aMoveDir);
			}

			Zoing(aZoingCount,aHoldZGPos);
		}
	}
}



Point Player::Move(Point theDir, float theSpeed)
{
	gWorld->mCollidedWithLava=false;
	Point aHold=mPos;

	while (theSpeed>20)
	{
		Move(theDir,20);
		theSpeed-=20;
	}
	mPos+=theDir*theSpeed;

//*
	if (gWorld->CollideDir(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),theDir.mX))
	{
		mPos.mX=aHold.mX;
		Point aDir=theDir.Normal();

		for (int aCount=0;aCount<_min(1.0f,theSpeed*2);aCount++)
		{
			mPos.mX+=aDir.mX;
			if (gWorld->CollideDir(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),theDir.mX))
			{
				mPos.mX-=theDir.mX;
			}
			//mPos.mY+=aDir.mY;
			//if (gWorld->CollideDir(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),theDir.mX)) mPos.mY-=theDir.mY;
			//if (gWorld->Collide(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight))) mPos.mY-=aDir.mY;
		}
	}

	if (gWorld->CollideY(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),gMath.Sign(theDir.mY)))
	{
		mPos.mY=aHold.mY;
		Point aDir=theDir.Normal();
		for (int aCount=0;aCount<_min(1.0f,theSpeed*2);aCount++)
		{
			mPos.mY+=aDir.mY;
			if (gWorld->CollideY(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),gMath.Sign(theDir.mY))) mPos.mY-=theDir.mY;
		}
	}

	if (theDir.mY<=0) mCollidedWithLava|=gWorld->mCollidedWithLava;
	return mPos-aHold;

}

Point Robot::Move(Point theDir, float theSpeed)
{
	Point aHold=mPos;

	while (theSpeed>20)
	{
		Move(theDir,20);
		theSpeed-=20;
	}
	mPos+=theDir*theSpeed;

//*
	if (gWorld->CollideDir(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),theDir.mX))
	{
		mPos.mX=aHold.mX;
		Point aDir=theDir.Normal();

		for (int aCount=0;aCount<_min(1.0f,theSpeed*2);aCount++)
		{
			float aHoldMe=mPos.mX;
			mPos.mX+=aDir.mX;
			if (gWorld->CollideDir(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),theDir.mX)) {mPos.mX=aHoldMe;break;}

			//mPos.mY+=aDir.mY;
			//if (gWorld->CollideDir(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),theDir.mX)) mPos.mY-=theDir.mY;
			//if (gWorld->Collide(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight))) mPos.mY-=aDir.mY;
		}
	}

	bool aDoFall=true;
	if (mRocketCountdown>0) 
	{
		IPoint aGPos=gWorld->WorldToGrid(mPos+Point(0,(gWorld->mGridSize/2)+1));
		int aUnder=gWorld->GetGrid(aGPos.mX,aGPos.mY);
		if (aUnder==SPIKE_BLOCK)
		{
			aDoFall=false;
			mPos.mY=aHold.mY;
		}
	}
	if (aDoFall) if (gWorld->CollideY(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),gMath.Sign(theDir.mY)))
	{
		mPos.mY=aHold.mY;
		if (mRocketUpCountdown) mPos=aHold;
		Point aDir=theDir.Normal();
		for (int aCount=0;aCount<_min(1.0f,theSpeed*2);aCount++)
		{
			float aHoldMe=mPos.mY;
			mPos.mY+=aDir.mY;
			if (gWorld->CollideY(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight),gMath.Sign(theDir.mY))) {mPos.mY=aHoldMe;break;}
		}
	}

	if (theDir.mY<=0) mCollidedWithLava|=gWorld->mCollidedWithLava;
	return mPos-aHold;

}

void Player::FixPosition(bool doDrop)
{
	int aFudge=0;
	while (++aFudge<300)
	{
		Rect aMe=Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight+1);
		if (gWorld->Collide(aMe)) mPos.mY--;
		else break;
	}
	if (doDrop)
	{
		aFudge=0;
		while (++aFudge<300)
		{
			Rect aMe=Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight+1);
			if (!gWorld->Collide(aMe)) mPos.mY++;
			else break;
		}
	}
}

Robot::Robot(void)
{
	mCollide=Rect(-7,-13,14,34);
	mLazorLevel=0;
	mKillPause=0;
	mJumpKludge=0;
	mZoinging=false;
	mDrawFacing=1;
	mGMod=1.0f;
	mOnSlamwall=mLastSlamwall=false;

	mID=ID_ROBOT;

	mHP=mMaxHP=1;

	mPowerupCount=0;
	mTouchingGround=0;
	mSpeed=0;
	mFacing=1;
	mGravity=0;
	mBounceOffset=0;
	mBounceGravity=0;
	mPushing=false;
	mBob=0;
	mBurnup=0.0f;
	mBurnupCounter=0;
	mWantBounce=false;
	mInJump=false;
	mDoubleJumpCount=0;
	mJumpPitch=1.0f;
	mShootAnimateCountdown=0;
	mShootWaitCountdown=0;
	mShootHeldDown=false;
	mRocketCountdown=0;
	mRocketed=false;
	mRocketUpCountdown=0;
	mRocketedUp=false;
	mWantGoLeft=false;
	mWantGoRight=false;
	mRed=0;
	mInvulnerableCount=0;
	mNoControlCount=10;

	mCanJump=false;
	mCanDoubleJump=false;
	mCanShoot=false;
	mCanAnnihiliate=false;
	mCanRocket=false;
	mCanRocketUp=false;
	mHasHelmet=false;
	mHasExplozor=false;
	mHasHaxxor=false;
	mHasVelcro=false;
	mOnVelcro=false;
	mCrystals=0;
	mHasKey[0]=mHasKey[1]=mHasKey[2]=false;
	mGoldKeyCount=0;
}

Robot::~Robot(void)
{
	mAnimatorList.Free();
}

void Robot::Respawn()
{
	if (gGame) gGame->Save();
	mKillPause=0;

	//
	// Go back to computron
	//
	mPos=gWorld->mSpawnSpot;
	gWorld->SwitchMusic(gWorld->mSpawnMusic);

	gWorld->mScrollSpeed=0;
	mGravity=0;
	mBounceOffset=0;
	mBounceGravity=0;
	mPushing=false;
	mWantBounce=false;
	mInJump=false;
	mNoControlCount=25;
	mSpeed=0;
	mOnSlamwall=false;
	mLastSlamwall=false;
	StopRocket();

	FixPosition();

	mInvulnerableCount=100;
}

void Robot::Update()
{
	gWorld->mCollidedWithLava=false;
	mCollidedWithLava=false;

#ifdef _DEBUG
	gCurrentPlayer=this;
#endif

	mLastSlamwall=mOnSlamwall;
	Point aOriginalPos=mPos;

#ifdef _DEBUG
	if (IsKeyPressed(KB_INSERT)) mPos.mX--;
#endif

	mGridPos=gWorld->WorldToGrid(mPos);
	int aOnWhat=gWorld->GetGrid(mGridPos.mX,mGridPos.mY);

	if (aOnWhat==SPIKE_BLOCK || aOnWhat==SPIKE_CEILING_BLOCK) 
	{
		float aMod=gMath.Decimal(mPos.mY/(float)gWorld->mGridSize);
		if (aOnWhat==SPIKE_CEILING_BLOCK || aMod>=.25f) 
		{
			if (aOnWhat==SPIKE_BLOCK || !mHasHelmet || mGravity>.5f) 
			{
				mInvulnerableCount=0;
				mHP=0;
				Die(false);
			}
			else
			{
				if (mGravity<0)
				{
					mGravity=(float)fabs(mGravity);

					Point aImpact=Point(mPos+Point(0,-30));
					IPoint aImpactPos=gWorld->WorldToGrid(aImpact);
					int aImpactGrid=gWorld->GetGrid(aImpactPos.mX,aImpactPos.mY);
					if (aImpactGrid==2)
					{
						for (int aCount=0;aCount<3;aCount++)
						{
							gSounds->mBubble[0].PlayPitched(1.0f+gRand.GetSpanF(-.2f,.2f));
							gSounds->mBubble[1].PlayPitched(1.0f+gRand.GetSpanF(-.2f,.2f));
						}
					}
					else
					{
						if (mGravity>1) 
						{
							gSounds->mHit_Helmet.PlayPitched(1.2f);
							Anim_Sprite *aS;
							aS=new Anim_Sprite(&gBundle_Play->mOuchburst);
							aS->mPos=mPos+Point(0,-20);
							aS->mFade=.75f;
							aS->mFadeSpeed=-.1f;
							aS->mRotation=gRand.GetSpanF(-5,5);
							gWorld->mAnimatorList_Top+=aS;
							gWorld->Shake(5);
						}
					}
				}
			}
		}
	}

	


	mJumpKludge=_max(0,mJumpKludge-1);
//	mCollidedWithLava=false;
	if (mKillPause>0)
	{
		mKillPause--;
		if (mKillPause==1) Respawn();
		return;
	}

	if (mInvulnerableCount!=0)
	{
		mInvulnerableCount=_max(0,mInvulnerableCount-1);
		if (mInvulnerableCount==0) LavaDoubleCheck();
	}

	mNoControlCount=_max(0,mNoControlCount-1);
	mShootHeldDown=false;

	mAnimatorList.Update();

	//
	// If we touch bad ground, die...
	//
	IPoint aGridPos=gWorld->WorldToGrid(mPos);
	gWorld->Touch(aGridPos);
	{
		//IPoint aGridPos=gWorld->WorldToGrid(aGridPos.mX,aTestPos.mY+1);
		if (gWorld->GetGrid(aGridPos.mX,aGridPos.mY+1)==2)
		{
			mGooPos=IPoint(aGridPos.mX,aGridPos.mY+1);
			Die(true,true);
		}
	}

	//
	// If we touch a music block, change music
	//

	UpdateControls();
	UpdateGravity();

	mShootAnimateCountdown=_max(0,mShootAnimateCountdown-15);
	mShootWaitCountdown=_max(0,mShootWaitCountdown-1);
	mRocketCountdown=_max(0,mRocketCountdown-1);
	mRocketUpCountdown=_max(0,mRocketUpCountdown-1);

	if (gWorld->mCollidedWithLava) 
	{
		Die(mGooPos.mY==mGridPos.mY+1);
	}
	else if (!mKillPause && !mInvulnerableCount && !mNoControlCount) if (mCollidedWithLava) Die(false,true);

	//
	// Check one way walls... 
	//
	if (mSpeed>0)
	{
		//		IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(20*mFacing,0.0f));
		IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(6*mFacing,0.0f));

		if (gWorld->IsBlockedOneWayWall(aGridPos.mX,aGridPos.mY,mFacing))
		{
			mPos.mX=aOriginalPos.mX;
		}
	}
	CheckMyFeet();

	if (mPushing || mRocketUpCountdown) 
	{
		mZoinging=false;
		mDrawFacing=mFacing;
	}
}

void Robot::UpdateControls()
{
	Point aDir;
	mPushing=false;


	if (!mRocketCountdown && !mRocketUpCountdown)
	{
		if (mWantGoLeft)
		{
			mPushing=true;
			if (mFacing>0) mSpeed=0;
			mFacing=-1;
		}
		if (mWantGoRight) 
		{
			mPushing=true;
			if (mFacing<0) mSpeed=0;
			mFacing=1;
		}
		if (mPushing) 
		{
			float aOldSpeed=mSpeed;
			if (mOnVelcro) 
			{
				mSpeed+=.1f;
				if (gMath.Sign(mFacing)!=gMath.Sign(mOnVelcro)) gWorld->Shake(4);
			}
			else mSpeed+=.5f;
			if (aOldSpeed<MAXSPEED) if (mSpeed>MAXSPEED) mSpeed=MAXSPEED;
		}
		else 
		{
			if (!mZoinging) mSpeed=_max(0.0f,mSpeed-.5f);
		}

		if (mSpeed>MAXSPEED) 
		{
			mSpeed-=1.0f;//=min(3.0f,mSpeed+.5f);
		}

		if (mSpeed>=2.0f && mTouchingGround && mBounceGravity==0)
		{
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mDust);
			aS->mPos=mPos+Point(-mFacing*(gRand.GetF(3)+2),ROBOTGROUND-2.0f);
			aS->mRotation=gRand.GetF(360);
			aS->mRotationDir=gRand.GetSpanF(-8,8);
			aS->mFade=1.0f-gRand.GetF(.5f);
			aS->mScale=gRand.GetF(1.25f)-gRand.GetF(.5f);
			aS->mFadeSpeed=-(gRand.GetF(.2f)+.1f)*.5f;
			mAnimatorList+=aS;
		}

	}
	aDir.mX=mFacing;

	mOnSlamwall=false;
	IPoint aGridPos=gWorld->WorldToGrid(mPos);
	if (gWorld->GetGrid(aGridPos.mX,aGridPos.mY)==SLAM_BLOCK)
	{
		mOnSlamwall=true;
		mSpeed=_min(mSpeed,.5f);
		gApp.mThrob=1.0f;
	}

	if (mRocketCountdown)
	{
		mSpeed=15;
		if (mRocketCountdown>ROCKETMAX-30)
		{
			float aGrey=gRand.GetF(.1f)+.9f;
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mSmoke);
			aS->mPos=mPos+((gMath.AngleToVector()*5))+Point(0,-5);
			aS->mFade=2.0f-gRand.GetF(.5f);
			aS->mFadeSpeed=-.1f;
			aS->mRotation=gRand.GetF(360);
			aS->mRotationDir=gRand.NegateF(gRand.GetF(gRand.GetF(5)+5));
			aS->mScale=gRand.GetF(.35f)+.5f;
			aS->mColor=Color(aGrey,aGrey,aGrey);
			gWorld->mAnimatorList_UnderPlayers+=aS;
			float aModScale=(mRocketCountdown-(ROCKETMAX-30))/30.0f;
			aS->mFade*=aModScale;
			aS->mScale*=aModScale;
			if (aS->mScale<.5f) aS->mScale=.5f;

			aGrey=gRand.GetF(.1f)+.9f;
			aS=new Anim_Sprite(&gBundle_Play->mSmoke);
			aS->mPos=mPos+((gMath.AngleToVector()*5))+Point(7.5f*mFacing,-5.0f);
			aS->mFade=2.0f-gRand.GetF(.5f);
			aS->mFadeSpeed=-.1f;
			aS->mRotation=gRand.GetF(360);
			aS->mRotationDir=gRand.NegateF(gRand.GetF(gRand.GetF(5)+5));
			aS->mScale=gRand.GetF(.35f)+.5f;
			aS->mColor=Color(aGrey,aGrey,aGrey);
			gWorld->mAnimatorList_UnderPlayers+=aS;
			aModScale=(mRocketCountdown-(ROCKETMAX-30))/30.0f;
			aS->mFade*=aModScale;
			aS->mScale*=aModScale;
			if (aS->mScale<.5f) aS->mScale=.5f;
		}
	}

	if (mRocketUpCountdown)
	{
		Point aDir=Point(0,0);
		if (mWantGoLeft)
		{
			aDir.mX=-.1f;
			mFacing=-1;
		}
		if (mWantGoRight) 
		{
			aDir.mX=.1f;
			mFacing=1;
		}
		Point aMovement=Move(aDir,5);
		mGravity=-15;

		if (mHasHelmet)
		{
			//
			// See if we collided with an enemy who is above us, eh?
			//
			Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
			aRobotRect.mY-=20;
			aRobotRect.mHeight-=20;
			aRobotRect.mX+=mPos.mX;
			aRobotRect.mY+=mPos.mY;

			EnumList(Player,aP,gWorld->mPlayerList)
			{
				if (aP->mIsEnemy)
				{
					Rect aRect=aP->mCollide;
					aRect.mX+=aP->mPos.mX;
					aRect.mY+=aP->mPos.mY;

					if (aRobotRect.Intersects(aRect))
					{
						gHelmetOuch=true;
						aP->Ouch();
						gHelmetOuch=false;
					}
				}
			}

		}


		if (gWorld->IsBlocked(gWorld->WorldToGrid(mPos+Point(0,-(ROBOTGROUND+10)))) ||
			gWorld->IsBlocked(gWorld->WorldToGrid(mPos+Point(-6,-(ROBOTGROUND+10)))) || 
			gWorld->IsBlocked(gWorld->WorldToGrid(mPos+Point(6,-(ROBOTGROUND+10))))) 
		{
			StopRocketUp();
			mRocketedUp=true;
			mGravity=2;

			Point aImpact=Point(mPos+Point(0,-30));
			IPoint aImpactPos=gWorld->WorldToGrid(aImpact);
			int aImpactGrid=gWorld->GetGrid(aImpactPos.mX,aImpactPos.mY);

			if (mHasHelmet)
			{
				//
				// See if we collided with an enemy who is above us, eh?
				//
				Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
				aRobotRect.mY-=20;
				aRobotRect.mHeight-=20;
				aRobotRect.mX-=gWorld->mGridSize/2;
				aRobotRect.mWidth+=gWorld->mGridSize;
				aRobotRect.mX+=mPos.mX;
				aRobotRect.mY+=mPos.mY;

				EnumList(Player,aP,gWorld->mPlayerList)
				{
					if (aP->mIsEnemy)
					{
						Rect aRect=aP->mCollide;
						aRect.mX+=aP->mPos.mX;
						aRect.mY+=aP->mPos.mY;

						if (aRobotRect.Intersects(aRect))
						{
							gHelmetOuch=true;
							aP->Ouch();
							gHelmetOuch=false;
						}
					}
				}



				Point aTestPos=mPos+Point(0,-(ROBOTGROUND+10));
				gWorld->Smash(aTestPos);
				gWorld->Smash(aTestPos+Point(20,0));
				gWorld->Smash(aTestPos+Point(-20,0));

				if (aImpactGrid==2)
				{
					if (aImpactGrid==2)
					{
						for (int aCount=0;aCount<10;aCount++)
						{
							gSounds->mBubble[0].PlayPitched(1.0f+gRand.GetSpanF(-.5f,.5f));
							gSounds->mBubble[1].PlayPitched(1.0f+gRand.GetSpanF(-.2f,.2f));
						}
					}
					gWorld->Shake(5);
					mGravity=1;

					Anim_Sprite *aS;
					aS=new Anim_Sprite(&gBundle_Play->mOuchburst);
					aS->mPos=mPos+Point(0,-20);
					aS->mFade=.25f;
					aS->mFadeSpeed=-.1f;
					aS->mRotation=gRand.GetSpanF(-5,5);
					gWorld->mAnimatorList_Top+=aS;

				}
				else
				{
					mGravity=10;
					gSounds->mHit_Helmet.Play();
					gWorld->Shake(20);

					Anim_Sprite *aS;
					aS=new Anim_Sprite(&gBundle_Play->mOuchburst);
					aS->mPos=mPos+Point(0,-20);
					aS->mFade=.75f;
					aS->mFadeSpeed=-.1f;
					aS->mRotation=gRand.GetSpanF(-5,5);
					gWorld->mAnimatorList_Top+=aS;

					for (int aCount=-1;aCount<=1;aCount++)
					{
						Point aAngle=gMath.AngleToVector(((aCount*20.0f)+gRand.GetSpanF(-5,5))+90);
						aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
						aS->mDir=aAngle*(10+gRand.GetF(2));
						if (aCount==0) aS->mDir*=1.5f;
						aS->mPos=mPos+(aS->mDir);
						aS->mPos+=Point(0,-20);
						aS->mDirMult=.8f;
						aS->mFade=.75f;
						aS->mFadeSpeed=-.025f;
						gWorld->mAnimatorList_Top+=aS;

						aAngle=gMath.AngleToVector(((aCount*20.0f)+gRand.GetSpanF(-5,5))-90);
						aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
						aS->mDir=aAngle*(10+gRand.GetF(2));
						if (aCount==0) aS->mDir*=1.5f;
						aS->mPos=mPos+(aS->mDir);
						aS->mPos+=Point(0,-20);
						aS->mDirMult=.8f;
						aS->mFade=.75f;
						aS->mFadeSpeed=-.025f;
						gWorld->mAnimatorList_Top+=aS;

					}
				}

			}
			else if (aImpactGrid==2) 
			{
				mCollidedWithLava=true;
			}
		}
		else
		{
			if (mRocketUpCountdown>ROCKETMAX-30)
			{
				for (int aCount=0;aCount<20;aCount+=5)
				{
					float aGrey=gRand.GetF(.1f)+.9f;
					Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mSmoke);
					aS->mPos=mPos+Point(0,ROBOTGROUND-15)+((gMath.AngleToVector()*5))+Point(0,aCount);
					aS->mFade=2.0f-gRand.GetF(.5f);
					aS->mFadeSpeed=-.125f;
					aS->mRotation=gRand.GetF(360);
					aS->mRotationDir=gRand.NegateF(gRand.GetF(5)+5);
					aS->mScale=gRand.GetF(.35f)+.35f;
					aS->mColor=Color(aGrey,aGrey,aGrey);
					gWorld->mAnimatorList_UnderPlayers+=aS;
					float aModScale=(mRocketUpCountdown-(ROCKETMAX-30))/30.0f;
					aS->mFade*=aModScale;
					aS->mScale*=aModScale;
					if (aS->mScale<.5f) aS->mScale=.5f;

				}
			}
		}

	}
	else
	{
		float aOldSpeed=mSpeed;

		if (mRocketCountdown>0) CheckVelcro(mSpeed);
		Point aMovement=Move(aDir,mSpeed);
		mSpeed=(float)fabs(aMovement.mX);

		if (mRocketCountdown>0 && mOnSlamwall && !mLastSlamwall) 
		{
			mPos.mX-=aMovement.mX*2;
			mSpeed=0;
		}

		if (!mOnVelcro)
		if (mSpeed<5) 
		{
			if (mRocketCountdown>0)
			{
				//
				// If velcro, don't slam...
				//

				Anim_Sprite *aS;
				aS=new Anim_Sprite(&gBundle_Play->mOuchburst);
				aS->mPos=mPos+Point(mFacing*20,0.0f);
				aS->mFade=.95f;
				aS->mFadeSpeed=-.1f;
				aS->mRotation=90*mFacing;
				gWorld->mAnimatorList_Top+=aS;

				gSounds->mHit_Helmet.PlayPitched(.8f);
				gWorld->Shake(20);
				gWorld->mFlashWhite=.5f;


			}
			mRocketCountdown=0;
		}

		if (mSpeed==0) 
		{
			mPushing=false;
			if (mRocketCountdown) 
			{
				StopRocket();
				//
				// We hit a wall, so we don't want to allow rocketing again until
				// they release the rocket button...
				//
				mRocketed=true;
			}

			if (aOldSpeed) 
			{
				if (gWorld->PushWall(mPos+Point((float)gWorld->mGridSize*aDir.mX,0.0f)))
				{
					mSpeed=aOldSpeed;
					mPushing=true;
				}
			}
		}
	}

	if (mGravity>-1 && mGravity<1)
	{
		//		mBob+=mSpeed*10;
		mBob+=mSpeed*7;
		if (mBob>=3600) mBob-=3600;
	}

	mWantGoLeft=mWantGoRight=false;
}


void Robot::Die(bool isLava,bool instantKill)
{
	mCollidedWithLava=false;
	if (isLava) 
	{
		if (mGravity<-.1f) return;
		if (mRocketCountdown) 
		{
			World::Grid *aGG=gWorld->GetGridPtr(mGooPos.mX,mGooPos.mY);
			int aGooType=0;
			if (aGG->mPaint>46+47+47+47 && aGG->mPaint<=46+47+47+47+47)	aGooType=0; // Red Goo
			if (aGG->mPaint>46+47+47+47+47 && aGG->mPaint<=46+47+47+47+47+47)	aGooType=1; // Green Goo

			Point aGooPos=gWorld->GridToWorld(mGooPos);
			Anim_Wake *aW=new Anim_Wake(aGooType,Point(mPos.mX+gRand.GetSpanF(5),aGooPos.mY-gWorld->mHalfGrid.mY+4));
			gWorld->mAnimatorList_Top+=aW;

			return;
		}

#ifdef PATCHED
		mKillPause=0;
		mInvulnerableCount=0;
#endif
	}

	if (mKillPause>0) return;
	if (mInvulnerableCount>0) return;
#ifdef PATCHED
	if (!instantKill && !isLava)
#else
	if (!instantKill)
#endif
	{
		mHP--;
		if (mHP>0)
		{
			gWorld->mFlashWhite=1.0f;
			gWorld->Shake(10);
			gSounds->mRobotouch.Play();
			mInvulnerableCount=100;
			return;
		}
	}

	mHP=mMaxHP;

	if (!isLava)
	{
		for (float aCount=0;aCount<360;aCount+=25)
		{
			float aMyAngle=aCount+gRand.GetSpanF(4);
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
			aS->mFade=2.5f;
			aS->mFadeSpeed=-.05f;
			aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(2));
			aS->mPos=mPos+(aS->mDir*3);
			aS->mRotation=gRand.GetF(360);
			aS->mScale=gRand.GetF(.25f)+.75f;
			aS->mColor=Color((215*.75f)/255.0f,(204*.75f)/255.0f,(184*.75f)/255.0f);
			//aS->mDir*=1.0f+gRand.GetF(1.5f);
			aS->mDirMult=Point(.75f,.75f);
			gWorld->mAnimatorList_Top+=aS;
		}
		for (float aCount=0;aCount<360;aCount+=25)
		{
			float aMyAngle=aCount+gRand.GetSpanF(4);
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
			aS->mFade=2.0f;
			aS->mFadeSpeed=-.05f;
			aS->mRotation=gRand.GetF(360);
			aS->mScale=gRand.GetF(.25f)+.75f;
			aS->mDir=gMath.AngleToVector(aMyAngle)*(9+gRand.GetSpanF(2));
			aS->mPos=mPos+(aS->mDir*3);
			//aS->mDir*=1.0f+gRand.GetF(1.5f);
			aS->mDirMult=Point(.85f,.85f);
			gWorld->mAnimatorList_Top+=aS;
		}
		gSounds->mDie.Play();
		gWorld->Shake(30);

		Anim_BouncyBit *aBit=new Anim_BouncyBit(&gBundle_Play->mRobits[0],mPos+Point(0,-20));
		gWorld->mAnimatorList_Top+=aBit;

		for (int aCount=0;aCount<8;aCount++)
		{
			Anim_BouncyBit *aBit=new Anim_BouncyBit(&gBundle_Play->mRobits[gRand.Get(5)+1],mPos+Point(0,-20)+(gMath.AngleToVector()*gRand.GetF(20)));
			aBit->mScale+=.25f+gRand.GetF(.2f);
			aBit->mBounce=gRand.GetSpanF(aBit->mBounceMax);
			gWorld->mAnimatorList_Top+=aBit;
		}
	}

	gWorld->mDied=true;
	mKillPause=30;
	gWorld->mFlashWhite=1.0f;
	gWorld->mScrollSpeed=0;
	gWorld->mFastTimeCountdown=125;
	mGravity=0;
	mBounceOffset=0;
	mBounceGravity=0;
	mPushing=false;
	mWantBounce=false;
	mInJump=false;
	mNoControlCount=25;
	mSpeed=0;
	StopRocket();


	if (isLava)
	{
		gSounds->mGooped.Play();
		Anim_GooSink *aGS=new Anim_GooSink(mPos+Point(0,10),mGooPos);
		gWorld->mAnimatorList_UnderWorld+=aGS;
	}


}

void Robot::GoLeft()
{
	if (mNoControlCount>0) return;
	mWantGoLeft=true;
	if (gWorld) gWorld->mInitialTimerPause=0;
}

void Robot::GoRight()
{
	if (mNoControlCount>0) return;
	mWantGoRight=true;
	if (gWorld) gWorld->mInitialTimerPause=0;
}


void Robot::Jump()
{
	if (mNoControlCount>0) return;

	if (!mCanJump) return;
	if (mRocketCountdown) return;
	if (mRocketUpCountdown) return;

	if (mOnVelcro)
	{
		if (!mPushing || gMath.Sign(mFacing)==gMath.Sign(mOnVelcro)) return;
	}

	if (gWorld) gWorld->mInitialTimerPause=0;


	Point aTestPos=mPos+Point(0,ROBOTGROUND+5);
	IPoint aGridPos=gWorld->WorldToGrid(aTestPos);
	//
	// Lava thing is not related to this section...
	//
#ifdef PATCHED
	mPos.mX=(float)((int)(mPos.mX+.5f));
#endif

	if (gWorld->IsBlocked(aGridPos) || mBounceGravity==-2) 
	{
		_DLOG("Touched Ground?");
		TOUCHEDGROUND
	}
	else
	{
		Point aTestPos=mPos+Point(7,ROBOTGROUND+5);
		IPoint aGridPos=gWorld->WorldToGrid(aTestPos);
		if (gWorld->IsBlocked(aGridPos)) 
		{
			Point aTestPos=mPos+Point(3,ROBOTGROUND+5);
			IPoint aGridPos=gWorld->WorldToGrid(aTestPos);
			if (gWorld->IsBlocked(aGridPos)) {
				_DLOG("Touched Ground? (2)");

				TOUCHEDGROUND}
		}
		else
		{
			Point aTestPos=mPos+Point(-7,ROBOTGROUND+5);
			IPoint aGridPos=gWorld->WorldToGrid(aTestPos);
			if (gWorld->IsBlocked(aGridPos)) 
			{
				Point aTestPos=mPos+Point(-3,ROBOTGROUND+5);
				IPoint aGridPos=gWorld->WorldToGrid(aTestPos);
				if (gWorld->IsBlocked(aGridPos)) 
				{
					_DLOG("Touched Ground? (3)");

					TOUCHEDGROUND}
			}
		}
	}

	if ((mTouchingGround>1 || mJumpKludge>0) || (mCanDoubleJump && mDoubleJumpCount<1))
	{
		_DLOG("Jump Condition: (%d>1 || %d>0) || (%d && %d)",mTouchingGround,mJumpKludge,mCanDoubleJump,mDoubleJumpCount);
		if (mTouchingGround || mJumpKludge>0)
		{
			mDoubleJumpCount=0;
			mJumpPitch=1.0f;
			mInJump=false;

			//
			// FIXME:
			// I think this is the one-way wall problem!
			//
			IPoint aGPos=gWorld->WorldToGrid(mPos+Point(0,-20));
			if (!gWorld->IsBlockedGround(aGPos.mX,aGPos.mY,false))
			{
			
				if (mWantGoLeft) Move(Point(-1,0),15);
				if (mWantGoRight) Move(Point(1,0),15);
			}
		}

		mGravity=_min(mGravity,-8.75f);//-9.6f;//7.75f;
		mBounceGravity=0;
		mBounceOffset=0;

		mInJump=true;

		gSounds->mJump.PlayPitched(mJumpPitch);
		if (mJumpPitch>1.10)
		{
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mJumpburst);
			aS->mPos=mPos+Point(0,ROBOTGROUND);
			aS->mFadeSpeed=-.1f;
			aS->mRotation=0;
			aS->mRotationDir=gRand.NegateF(5);
			gWorld->mAnimatorList_Top+=aS;
		}

		
		if (!mTouchingGround && mJumpKludge==0) mDoubleJumpCount++;
		mJumpPitch+=.25f;
		mJumpKludge=0;
	}
	else
	{
	}
}

void Robot::LavaDoubleCheck(int howFar)
{
	Point aTestPos=mPos+Point(0,ROBOTGROUND+howFar);
	IPoint aGridPos=gWorld->WorldToGrid(aTestPos);
	int aGridMiddle=gWorld->GetGrid(aGridPos.mX,aGridPos.mY);

	aTestPos=mPos+Point(7,ROBOTGROUND+howFar);
	IPoint aGridRightPos=gWorld->WorldToGrid(aTestPos);
	int aGridRight=gWorld->GetGrid(aGridRightPos.mX,aGridRightPos.mY);

	aTestPos=mPos+Point(-7,ROBOTGROUND+howFar);
	IPoint aGridLeftPos=gWorld->WorldToGrid(aTestPos);
	int aGridLeft=gWorld->GetGrid(aGridLeftPos.mX,aGridLeftPos.mY);

	if (aGridMiddle==0 || aGridMiddle==2)
	{
		if (ISLAVA(aGridRight))
		{
			mGooPos=IPoint(aGridRightPos.mX,aGridRightPos.mY);
			if (aGridRight==2) mGravity=0;
			Die(true);
		}
		if (ISLAVA(aGridLeft))
		{
			mGooPos=IPoint(aGridLeftPos.mX,aGridLeftPos.mY);
			if (aGridLeft==2) mGravity=0;
			Die(true);
		}
	}
}


void Robot::UpdateGravity()
{
	if (mGravity>.1f)
	{
		IPoint aGPos=gWorld->WorldToGrid(mPos);
		if (gWorld->GetGrid(aGPos.mX,aGPos.mY)==COSMIC_BLOCK) 
		{
			gApp.mHiss=.5f;

			mGMod=_max(.2f,mGMod-.05f);
			Anim_Sprite* aAS=new Anim_Sprite(&gBundle_Play->mFloaty);

			
//			aAS->mPos=mPos+Point(mDrawFacing*10,20.0f);
			aAS->mPos=mPos+Point(mDrawFacing*2,18.0f);
			//if (mPushing) aAS->mPos=mPos+Point(0.0f,20.0f);
			if (mInJump) aAS->mPos=mPos+Point(mDrawFacing*10,20.0f);
			aAS->mFade=gRand.GetSpanF(.5f,.75f);
			aAS->mRotation=gRand.GetSpanF(-5,5);
			aAS->mScale=gRand.GetSpanF(.5f,1.1f);
			aAS->mFadeSpeed=-gRand.GetSpanF(.1f,.25f);
			gWorld->mAnimatorList_UnderPlayers+=aAS;
		}
		else mGMod=_min(1.0f,mGMod+.1f);
	}
	else mGMod=_min(1.0f,mGMod+.1f);

	CheckVelcro();

	if (mOnVelcro)
	{
		mGMod=0;
		mGravity*=.5f;//=0;
		mInJump=false;
		TOUCHEDGROUND;
	}



	if (mRocketCountdown) mGravity+=.01f*mGMod;
	else if (mRocketUpCountdown) mGravity+=0;
	else mGravity+=.35f*mGMod;

	if (mGMod<.9f) if (mGravity>1.5f) mGravity=1.5f;
	if (mGravity>100) mGravity=100;

	mBounceGravity+=.35f;

	mBounceOffset+=mBounceGravity;
	if (mBounceOffset>0)
	{
		mBounceGravity=0;
		mBounceOffset=0;
	}

	if (mGravity>8.25f) mWantBounce=true;

	if (mGravity>80)
	{
		mBurnupCounter++;
		if (mBurnupCounter>100)
		{
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mSmoke);
			aS->mColor=Color(1,gRand.GetF(1),0);
			aS->ColorStep(Color(-.1f,-.3f,0));
			aS->mFade=mBurnup*2;
			aS->mFadeSpeed=-.025f;
			aS->mRotation=gRand.GetF(360);
			aS->mScale=1.0f+gRand.GetF(.25f);
//			aS->mPos=mPos+(gMath.AngleToVector()*gRand.GetSpanF(7));
			aS->mPos=mPos+(gMath.AngleToVector()*gRand.GetSpanF(5,15));
			aS->mDir=Point(0.0,mGravity*.95f);

			if ((gAppPtr->AppTime()%10)==0) gSounds->mCrumble.PlayPitched(1.0f+gRand.GetSpanF(-.1f,.1f),.2f);
			if (gRand.GetBool()) gWorld->mAnimatorList_UnderWorld+=aS;
			else 
			{
				gWorld->mAnimatorList_Top+=aS;
				aS->mScale*=.5f;
			}

			mBurnup+=.01f;
			if (mBurnup>1.0f) Die(false,true);
		}
	}
	else 
	{
		mBurnupCounter=0;
		mBurnup=0;
	}


	//int aLast=mTouchingGround;

	Point aTestPos=mPos+Point(0,ROBOTGROUND+1);
	IPoint aGridPos=gWorld->WorldToGrid(aTestPos);
	if (!gWorld->IsBlockedY(aGridPos.mX,aGridPos.mY,1)) 
	{
		mTouchingGround=0;
	}
	bool aDoLavaDoubleCheck=false;
	float aOldGravity=mGravity;
	Point aMovement=Move(Point(0.0f,gMath.Sign(mGravity)),(float)fabs(mGravity));
//	if (fabs(aMovement.mY-mGravity)>=.002f) 
	float aMovementSize=(float)fabs(aMovement.mY-mGravity);
	if (aMovementSize>=_max(.002f,(float)fabs(mGravity)*.02f)) 
	{

		if (mGravity>.35f) aDoLavaDoubleCheck=true;

		int aWhatUnder=gWorld->GetGrid(aGridPos.mX,aGridPos.mY);
		if (aWhatUnder!=SPIKE_BLOCK && aWhatUnder!=SPIKE_CEILING_BLOCK)// && (aWhatUnder!=0 || mGravity<0)) 
		{
			mGravity=0;
		}

		mZoinging=false;

		if (mHasHelmet)
		{
			if (aOldGravity<-4)
			{
				mGravity=(float)fabs(aOldGravity);

				Point aImpact=Point(mPos+Point(0,-30));
				IPoint aImpactPos=gWorld->WorldToGrid(aImpact);
				int aImpactGrid=gWorld->GetGrid(aImpactPos.mX,aImpactPos.mY);
				if (aImpactGrid==2)
				{
					for (int aCount=0;aCount<3;aCount++)
					{
						gSounds->mBubble[0].PlayPitched(1.0f+gRand.GetSpanF(-.2f,.2f));
						gSounds->mBubble[1].PlayPitched(1.0f+gRand.GetSpanF(-.2f,.2f));
					}
				}
				else
				{
					gSounds->mHit_Helmet.PlayPitched(1.2f);

					Anim_Sprite *aS;
					aS=new Anim_Sprite(&gBundle_Play->mOuchburst);
					aS->mPos=mPos+Point(0,-20);
					aS->mFade=.75f;
					aS->mFadeSpeed=-.1f;
					aS->mRotation=gRand.GetSpanF(-5,5);
					gWorld->mAnimatorList_Top+=aS;
					gWorld->Shake(5);
				}

				Point aTestPos=mPos+Point(0,-(ROBOTGROUND+10));

				IPoint aPos[3];
				int aPosCount=0;
				aPos[aPosCount++]=gWorld->WorldToGrid(aTestPos.mX,aTestPos.mY);
				aPos[aPosCount]=gWorld->WorldToGrid(aTestPos.mX+10,aTestPos.mY);
				if (aPos[aPosCount].mX!=aPos[aPosCount-1].mX) aPosCount++;
				aPos[aPosCount]=gWorld->WorldToGrid(aTestPos.mX-10,aTestPos.mY);
				if (aPos[aPosCount].mX!=aPos[aPosCount-1].mX)
				{
					if (aPosCount==2) {if (aPos[aPosCount].mX!=aPos[aPosCount-2].mX) aPosCount++;}
					else aPosCount++;
				}


				for (int aCount=0;aCount<aPosCount;aCount++)
				{
					gWorld->Crack(gWorld->GridToWorld(aPos[aCount]));
				}
			}
		}
		if (mWantBounce) 
		{
			//
			// POOF CLOUDS
			//
			Point aHold=mPos;
			int aHoldT=mTouchingGround;
			mPos.mY+=15;

			mTouchingGround=1;
			CheckMyFeet();
			mPos=aHold;
			mTouchingGround=aHoldT;
			if (mZoinging) return;

			mBounceGravity=-2.5f;
			mWantBounce=false;

			Point aGroundPos=mPos+Point(0.0f,ROBOTGROUND-2.0f);
			for(int aCount=0;aCount<8;aCount++)
			{
				if (gWorld->IsBlocked(gWorld->WorldToGrid(aGroundPos))) break;
				aGroundPos.mY++;
			}


			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mLandpoof);
			aS->mPos=aS->mPos=aGroundPos;
			aS->mDir=Point(3.0f,0.0f);
			aS->mDirMult=Point(.75f,1.0f);
			aS->mFade=1.0f;
			aS->mFadeSpeed=-.1f;
			gWorld->mAnimatorList_Top+=aS;

			aS=new Anim_Sprite(&gBundle_Play->mLandpoof);
			aS->mPos=aS->mPos=aGroundPos;
			aS->mDir=Point(-3.0f,0.0f);
			aS->mScale=-1.0f;
			aS->mDirMult=Point(.75f,1.0f);
			aS->mFade=1.0f;
			aS->mFadeSpeed=-.1f;
			gWorld->mAnimatorList_Top+=aS;

			LavaDoubleCheck(10);
		}
	}

	{
		if (gWorld->Collide(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY)+1,mCollide.mWidth,mCollide.mHeight)))
		{
			TOUCHEDGROUND;
			mDoubleJumpCount=0;
			mJumpPitch=1.0f;
			mInJump=false;

			if (aDoLavaDoubleCheck) 
			{
				aDoLavaDoubleCheck=false;
				LavaDoubleCheck();
			}
		}
	}

	{
		Point aTestPos=mPos+Point(0,ROBOTGROUND+3);
		IPoint aGridPos=gWorld->WorldToGrid(aTestPos);
		if (gWorld->GetGrid(aGridPos.mX,aGridPos.mY)==2)
		{
			mGooPos=IPoint(aGridPos.mX,aGridPos.mY);
			Die(true,true);
		}
	}
}

void Robot::Draw()
{
	mAnimatorList.Draw();

	if (mInvulnerableCount) if (((gAppPtr->AppTime()/3)%2)==0) gG.ClipColor(Color(1,0,0));
	DrawBot();
	if (mRed)
	{
		gG.RenderWhite(true);
		gG.ClipColor(Color(mRed,0,0,.5f));
		DrawBot();
		gG.RenderWhite(false);
	}
	if (mBurnup)
	{
		gG.RenderWhite(true);
		gG.ClipColor(Color(1,0,0,mBurnup));
		DrawBot();
		gG.RenderWhite(false);
	}
	gG.ClipColor();
}

void Robot::DrawBot()
{
	if (mKillPause) return;

	gG.Translate(0,-6);
	Matrix aMat;
	aMat.Scale(-mDrawFacing,1,1);
	aMat.Translate(mPos+Point(0.0f,mBounceOffset));

	if (mRocketUpCountdown)
	{
		bool aYellow=(gRand.Get(5)==1);
		if (aYellow)
		{
			gG.RenderWhite(true);
			gG.SetColor(1,1,0);
		}
		Matrix aFireMat;
		aFireMat.Scale(1,(1.5f+(gMath.Sin(mPos.mY*6)*.25f)),1);
		if (mDrawFacing<0) aFireMat.Translate(mPos+Point(2.0f,ROBOTGROUND-10.0f));
		else aFireMat.Translate(mPos+Point(1.0f,ROBOTGROUND-10.0f));
		
		gBundle_Play->mRocketfireup.Draw(aFireMat);
		if (aYellow)
		{
			gG.RenderWhite(false);
			gG.SetColor();
		}

		if (mHasHelmet) gBundle_Play->mRobot_Rocketup_Helmet.Draw(aMat);
		else gBundle_Play->mRobot_Rocketup.Draw(aMat);
	}
	else if (mSpeed>4.0f)
	{
		//
		// Draw flames!
		//
		if (mRocketCountdown)
		{
			bool aYellow=(gRand.Get(5)==1);
			if (aYellow)
			{
				gG.RenderWhite(true);
				gG.SetColor(1,1,0);
			}
			Matrix aFireMat;
			aFireMat.Scale(-mDrawFacing*(1.0f+(gMath.Sin(mPos.mX*6)*.25f)),1,1);
			aFireMat.Translate(mPos+Point(-mDrawFacing*5,0.0f));
			gBundle_Play->mRocketfire.Draw(aFireMat);
			if (aYellow)
			{
				gG.RenderWhite(false);
				gG.SetColor();
			}
		}
		gBundle_Play->mRobot_Rocket.Draw(aMat);
		if (mHasHelmet) gBundle_Play->mRobot_Rocket_Helmet.Draw(aMat);
	}
	else if (mInJump)
	{
		if (mShootAnimateCountdown || mShootHeldDown)
		{
			gBundle_Play->mRobot_Jumpshoot.Draw(aMat);
			if (mHasHelmet) gBundle_Play->mRobot_Jumpshoot_Helmet.Draw(aMat);

			if (mShootAnimateCountdown)
			{
				DrawBoltGlow(mPos+(gBundle_Play->mRobot_Jumpshoot.mKey[0]*Point(-mDrawFacing,1.0f)),0);
				DrawBoltGlow(mPos+(gBundle_Play->mRobot_Jumpshoot.mKey[1]*Point(-mDrawFacing,1.0f)),1);
			}
		}
		else 
		{
			gBundle_Play->mRobot_Jump.Draw(aMat);
			if (mHasHelmet) gBundle_Play->mRobot_Jump_Helmet.Draw(aMat);
		}
	}
	else if (mPushing)
	{
		gG.Translate((gMath.Sin(mBob/2))/2,0.0f);
		gBundle_Play->mRobot_Runwheel.Draw(aMat);
		gG.Translate(-(gMath.Sin(mBob/2))/2,0.0f);
		gG.Translate(0.0f,(gMath.Sin(mBob)));
		gBundle_Play->mRobot_Run.Draw(aMat);
		if (mHasHelmet) gBundle_Play->mRobot_Run_Helmet.Draw(aMat);

		if (mShootAnimateCountdown)
		{
			DrawBoltGlow(mPos+(gBundle_Play->mRobot_Run.mKey[0]*Point(-mFacing,1.0f)),0);
			DrawBoltGlow(mPos+(gBundle_Play->mRobot_Run.mKey[1]*Point(-mFacing,1.0f)),1);
		}

		gG.Translate(0.0f,-(gMath.Sin(mBob)));

	}
	else
	{
		if (mShootAnimateCountdown || mShootHeldDown)
		{
			gBundle_Play->mRobot_Idleshoot.Draw(aMat);
			if (mHasHelmet) gBundle_Play->mRobot_Idleshoot_Helmet.Draw(aMat);

			if (mShootAnimateCountdown)
			{
				DrawBoltGlow(mPos+(gBundle_Play->mRobot_Idleshoot.mKey[0]*Point(-mDrawFacing,1.0f)),0);
				DrawBoltGlow(mPos+(gBundle_Play->mRobot_Idleshoot.mKey[1]*Point(-mDrawFacing,1.0f)),1);
			}
		}
		else
		{
			gBundle_Play->mRobot_Idle.Draw(aMat);
			if (mHasHelmet) gBundle_Play->mRobot_Idle_Helmet.Draw(aMat);

		}
	}

	gG.SetColor();

	gG.Translate(0,6);
}	

void Robot::DrawBoltGlow(Point thePos,int theBolt)
{
	gG.SetColor(1,.45f,1,_min(((mShootAnimateCountdown)/180.0f)*1,1.0f));
	gBundle_Play->mGlow.DrawScaled(thePos,.6f);
	gG.SetColor();

	if (mShootAnimateCountdown>90)
	{
		gG.RenderWhite(true);
		gG.SetColor(1,1,1,_min(((mShootAnimateCountdown-90.0f)/90.0f)*1,1.0f));
		gBundle_Play->mGlow.DrawScaled(thePos,.8f);
		gG.SetColor();
		gG.RenderWhite(false);
	}

	Matrix aBoltMat;
	aBoltMat.Scale(gMath.Sin((float)mShootAnimateCountdown)*1.0f);
	aBoltMat.Rotate2D((float)(mShootAnimateCountdown)+mBoltGlowAngle[theBolt]);
	aBoltMat.Translate(thePos);

	gG.RenderWhite(true);
	gG.SetColor(_min(((mShootAnimateCountdown)/180.0f)*3,1.0f)*(gRand.GetF(.25f)+.75f));
	gBundle_Play->mBoltglow.Draw(aBoltMat);
	gG.SetColor();
	gG.RenderWhite(false);
}

void Robot::Shoot()
{
	if (mNoControlCount>0) return;

	mShootHeldDown=true;
	if (mRocketCountdown || mSpeed>4.0f) return;
	if (mRocketUpCountdown) return;
	if (!mCanShoot) return;
	if (mShootWaitCountdown>0) return;

	mShootWaitCountdown=15+5;
	if (mCanAnnihiliate) mShootWaitCountdown=6;
	mShootAnimateCountdown=180;

	gSounds->mLaser.PlayPitched(1.0f+gRand.GetSpanF(.05f));

	mBoltGlowAngle[0]=gRand.GetF(360);
	mBoltGlowAngle[1]=gRand.GetF(360);

	float aLazorDist=20;
	if (mPushing) aLazorDist+=12;

	Lazor *aL=new Lazor();
	aL->mExplode=mHasExplozor;
	aL->mPos=mPos+Point(mDrawFacing*aLazorDist,-13.0f);
	aL->mDir=mDrawFacing;
	aL->mDamage=mLazorLevel;
	gWorld->mPlayerList+=aL;

	Point aPos=aL->mPos;
	aPos.mX+=(aL->mDir*7.0f)*aL->mSpeed;

	bool aBlocked=false;
	while (gWorld->IsBlockedDir(gWorld->WorldToGrid(aPos),mDrawFacing)) 
	{
		aPos.mX-=(aL->mDir*7.0f)*aL->mSpeed;
		aBlocked=true;
	}
	if (aBlocked) aL->mPos=aPos;
}

void Robot::Rocket()
{
	if (mNoControlCount>0) return;

	StopRocketUp();
	if (!mCanRocket) return;

	mFacing=mDrawFacing;
	mZoinging=false;
	
	if (mRocketCountdown<=0 && !mRocketed)
	{
		if (!gWorld->Collide(Rect(mPos.mX+(mCollide.mX)+(mFacing*25),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight-2)))
		if (!gWorld->Collide(Rect(mPos.mX+(mCollide.mX)+(mFacing*25),mPos.mY+(mCollide.mY),mCollide.mWidth,mCollide.mHeight-2)))
		{
			mGravity=0;
			mBounceGravity=0;
			mBounceOffset=0;
			mRocketCountdown=ROCKETMAX;
			mRocketed=true;
			gSounds->mRocket.Play();

			for (float aCount=0;aCount<360;aCount+=15)
			{
				float aGrey=gRand.GetF(.1f)+.9f;
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mSmoke);
				aS->mPos=mPos+Point(0,-5)+((gMath.AngleToVector(aCount)*40)*Point(.5f,1.0f));
				aS->mFade=2.0f-gRand.GetF(.5f);
				aS->mFadeSpeed=-.1f;
				aS->mRotation=gRand.GetF(360);
				aS->mRotationDir=gRand.NegateF(gRand.GetF(5)+5);
				aS->mScale=gRand.GetF(.35f)+.5f;
				aS->mColor=Color(aGrey,aGrey,aGrey);
				gWorld->mAnimatorList_Top+=aS;
			}
		}
	}
}

void Robot::StopRocket()
{
	mRocketCountdown=0;	
	mRocketed=false;
}

void Robot::RocketUp()
{
	if (mNoControlCount>0) return;

	bool aOnGround=false;
	Point aHold=mPos;
	Move(Point(0,1),3);
	if ((mPos.mY-aHold.mY)<1) aOnGround=true;
	mPos=aHold;

	if (!aOnGround) return;
	if (mRocketCountdown) return;
	if (!mCanRocketUp) return;

#ifdef PATCHED
	mPos.mX=(float)((int)(mPos.mX+.5f));
#endif

	if (mRocketUpCountdown<=0 && !mRocketedUp)
	{
		if (!gWorld->Collide(Rect(mPos.mX+(mCollide.mX),mPos.mY+(mCollide.mY)-25,mCollide.mWidth,mCollide.mHeight)))
		{
			mGravity=0;
			mBounceGravity=0;
			mBounceOffset=0;
			mRocketUpCountdown=ROCKETMAX;
			mRocketedUp=true;
			gSounds->mRocket.Play();

			for (float aCount=0;aCount<360;aCount+=15)
			{
				float aGrey=gRand.GetF(.1f)+.9f;
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mSmoke);
				aS->mPos=mPos+Point(0,ROBOTGROUND)+((gMath.AngleToVector(aCount)*60)*Point(1.0f,.15f));
				aS->mDir=gMath.AngleToVector(aCount)*Point(1,0);
				aS->mFade=2.0f-gRand.GetF(.5f);
				aS->mFadeSpeed=-.1f;
				aS->mRotation=gRand.GetF(360);
				aS->mRotationDir=gRand.NegateF(gRand.GetF(5)+5);
				aS->mScale=gRand.GetF(.35f)+.75f;
				aS->mColor=Color(aGrey,aGrey,aGrey);
				gWorld->mAnimatorList_Top+=aS;
			}
		}
	}
	mZoinging=false;
}

void Robot::StopRocketUp()
{
	mRocketUpCountdown=0;
	mRocketedUp=false;
}

Lazor::Lazor(void)
{
	mID=ID_LAZOR;

	mGlint=gRand.GetF(360);
	mSpeed=1.0f;
	mCheck=0;
	mDamage=0;
	mExplode=false;
}

Lazor::~Lazor(void)
{
}

void Lazor::Update()
{
	mPos.mX+=(mDir*7.0f)*mSpeed;
	mGlint+=4;
	if (gWorld->IsBlockedDir(gWorld->WorldToGrid(mPos),mDir))
	{
		gSounds->mLaserhit.Play(gWorld->GetVolume(mPos));
		Kill();

		EnumList(Player,aP,gWorld->mPlayerList)
		{
			if (aP->mID==ID_GUNTARGET)
			{
//				Rect aGTRect=Rect(aP->mPos.mX-35,aP->mPos.mY-20,70,40);
				Rect aGTRect=Rect(aP->mPos.mX-25,aP->mPos.mY-20,50,40);
				if (mExplode) aGTRect=aGTRect.ExpandH(40);
				if (aGTRect.ContainsPoint(mPos.mX,mPos.mY))
				{
					GunTarget* aGT=(GunTarget*)aP;
					aGT->Hit();
				}
			}
		}


		Anim_ExplodePop *aEP=new Anim_ExplodePop(mPos+Point(-(mDir*7.0f)*mSpeed,0.0f));
		gWorld->mAnimatorList_Top+=aEP;

		if (mExplode)
		{
			gSounds->mBoom.Play(gWorld->GetVolume(mPos));

			Anim_SpriteStrip *aSS=new Anim_SpriteStrip(&gBundle_Play->mExplozor);
			aSS->mPos=mPos+Point(-(mDir*7.0f)*mSpeed,0.0f);
			aSS->mFrameSpeed=.55f;//.75f;
			aSS->mDir=Point(0,-1);
			aSS->mScale=1.5f;
			aSS->mAdditive=true;
			gWorld->mAnimatorList_Top+=aSS;

			aEP->mTotalScale*=2.0f;

			//
			// Enemy nearby?
			//
			EnumList(Player,aP,gWorld->mPlayerList)
			{
				if (aP->mIsEnemy)
				{
					Point aVec=aP->mPos-mPos;
					if (aVec.Length()<45)
					{
						gSounds->mAlienhit.Play();
						Kill();

						aP->Ouch();
					}
				}
			}

			//
			// Any explodo-block nearby?
			//
			ExplodeBlocks();
		}
	}

	if (!gWorld->mVisibleRect.Expand(400).ContainsPoint(mPos.mX,mPos.mY)) Kill();

	if (--mCheck<=0 && !mKill)
	{
		mCheck=2;
		EnumList(Player,aP,gWorld->mPlayerList)
		{
			if (aP->mIsEnemy && !aP->mDying)
			{
				Rect aTestRect=aP->mCollide;
				aTestRect.mX+=aP->mPos.mX;
				aTestRect.mY+=aP->mPos.mY;
				if (aTestRect.ContainsPoint(mPos.mX,mPos.mY))//aTestPos.mX,aTestPos.mY))
				{
					gSounds->mAlienhit.Play();
					gSounds->mLaserhit.Play();
					Anim_ExplodePop *aEP=new Anim_ExplodePop(mPos+Point(-(mDir*7.0f)*mSpeed,0.0f));
					gWorld->mAnimatorList_Top+=aEP;
					Kill();

					gOuchDir=Point(mDir,0.0f);
					aP->Ouch(mDamage);

					if (mExplode)
					{
						gSounds->mBoom.Play(gWorld->GetVolume(mPos));

						Anim_SpriteStrip *aSS=new Anim_SpriteStrip(&gBundle_Play->mExplozor);
						aSS->mPos=mPos+Point(-(mDir*7.0f)*mSpeed,0.0f);
						aSS->mFrameSpeed=.55f;//.75f;
						aSS->mDir=Point(0,-1);
						aSS->mScale=1.5f;
						aSS->mAdditive=true;
						gWorld->mAnimatorList_Top+=aSS;

						aEP->mTotalScale*=2.0f;

						//
						// Enemy nearby?
						//
						EnumList(Player,aP,gWorld->mPlayerList)
						{
							if (aP->mIsEnemy)
							{
								Point aVec=aP->mPos-mPos;
								if (aVec.Length()<45)
								{
									gSounds->mAlienhit.Play();
									Kill();

									aP->Ouch();
								}
							}
						}

						//
						// Any explodo-block nearby?
						//
						ExplodeBlocks();
					}

					break;
				}
			}
		}
	}
}

void Lazor::ExplodeBlocks()
{
	Point aCircleCenter=mPos;
	float aCircleRadius=25;//*1.5f;


	IPoint aGridPos=gWorld->WorldToGrid(mPos);
	//if (gWorld->GetGrid(aGridPos.mX,aGridPos.mY)==EXPLODE_BLOCK) gWorld->Explode(aGridPos.mX,aGridPos.mY,0);

	int aGot=gWorld->GetGrid(aGridPos.mX,aGridPos.mY);
	if (aGot==EXPLODE_BLOCK) gWorld->Explode(aGridPos.mX,aGridPos.mY,0);
	if (aGot==ACID_BLOCK) gWorld->Melt(aGridPos.mX,aGridPos.mY);
	//if (aGot==BREAK_BRICK) gWorld->Crack(gWorld->GridToWorld(aGridPos));

	for (int aCount=0;aCount<8;aCount++)
	{
		IPoint aTest=aGridPos+gMath.GetCardinal8Direction(aCount);
		int aGot=gWorld->GetGrid(aTest.mX,aTest.mY);

		if (aGot==EXPLODE_BLOCK) 
		//if (gWorld->GetGrid(aTest.mX,aTest.mY)==EXPLODE_BLOCK) 
		{
			Point aPos=gWorld->GridToWorld(aTest.mX,aTest.mY);
			Rect aRect=Rect(aPos.mX-gWorld->mHalfGrid.mX,aPos.mY-gWorld->mHalfGrid.mY,(float)gWorld->mGridSize,(float)gWorld->mGridSize);
			if (gMath.RectangleIntersectCircle(aRect,aCircleCenter,aCircleRadius)) gWorld->Explode(aTest.mX,aTest.mY,0);
		}
	}
}

void Lazor::Draw()
{
	Matrix aMat;
	aMat.Scale(-mDir,1,1);
	if (mExplode) aMat.Scale(1.5f);
	aMat.Translate(mPos);
	gBundle_Play->mLazor.Draw(aMat);
	gBundle_Play->mLazorglint.DrawRotated(mPos,mGlint);
}

int mTailFrame[]={0,1,2,1,0,0};
Kitty::Kitty(void)
{
	mID=ID_KITTY;
	mNullKitty=false;
	mTail=0;
	mBlinkCountdown=0;
	mFirstUpdate=true;
	mGravity=0;
	mFalling=false;

//	mCollide=Rect(-10,-22,20,46);
	mCollide=Rect(-10,-12,20,36);
}

Kitty::~Kitty(void)
{
}

void Kitty::CheckMyFeet()
{
	if (mFalling) 
	{
		IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(0,0));
		IPoint aGooPos;
		World::Grid* aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
		if (aG) if (ISSPIKE(aG->mLayout)) DieKitty(2);

		return;
	}


	IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(0,25));
	IPoint aGooPos;
	World::Grid* aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	float aMoveDir=0;

	int aFall=0;
	bool aLava=false;

	if (aG)
	{
		if (aG->mLayout==0 || aG->mLayout==COSMIC_BLOCK) aFall++;
		if (aG->mLayout==2) 
		{
			aLava=true;
			aGooPos=aGridPos;
		}
		if (ISSPIKE(aG->mLayout))
		{
			DieKitty(2);
		}
		if (aG->mCustomDraw)
		{
			if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
			if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
		}
	}


	aGridPos=gWorld->WorldToGrid(mPos+Point(-15,25));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG)
	{
		if (aG->mLayout==0 || aG->mLayout==COSMIC_BLOCK) aFall++;
		if (aG->mCustomDraw)
		{
			if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
			if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
		}
	}

	aGridPos=gWorld->WorldToGrid(mPos+Point(+15,25));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG)
	{
		if (aG->mLayout==0 || aG->mLayout==COSMIC_BLOCK) aFall++;
		if (aG->mCustomDraw)
		{
			if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
			if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
		}
	}

	if (aMoveDir) 
	{
		Move(Point(gMath.Sign(aMoveDir),0.0f),CONVEYOR_SPEED);
	}
	if (aFall>=3) 
	{
		mFalling=true;
		mGravity=0;
	}
	if (aLava) 
	{
		DieKitty(0,aGooPos);
	}
}


void Kitty::DieKitty(int theType, IPoint theGridPos)
{
	gWorld->mNoMoreKitty=true;
	if (gWorld) if (gWorld->mKitty==this) gWorld->mKitty=NULL;
	Kill();


	gSounds->mNomorekitty.Play();
	if (theType==0) // Lava
	{
		gSounds->mGooped.Play();
		Anim_GooSinkKitty *aGS=new Anim_GooSinkKitty(mPos+Point(0,2),theGridPos);
		gWorld->mAnimatorList_UnderWorld+=aGS;
	}
	if (theType==1) // Offscreen
	{
	}
	if (theType==2) // Spikes
	{
		gSounds->mLaserouch.Play();
		for (float aCount=0;aCount<360;aCount+=30)
		{
			float aMyAngle=aCount+gRand.GetSpanF(4);
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
			aS->mFade=1.5f;
			aS->mFadeSpeed=-.1f;
			aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(2));
			aS->mPos=mPos+(aS->mDir*3);
			aS->mColor=Color(1,0,0);
			//aS->mDir*=1.0f+gRand.GetF(1.5f);
			aS->mDirMult=Point(.75f,.75f);
			gWorld->mAnimatorList_Top+=aS;
		}
		for (float aCount=0;aCount<360;aCount+=30)
		{
			float aMyAngle=aCount+gRand.GetSpanF(4);
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
			aS->mFade=1.5f;
			aS->mFadeSpeed=-.1f;
			aS->mDir=gMath.AngleToVector(aMyAngle)*(9+gRand.GetSpanF(2));
			aS->mPos=mPos+(aS->mDir*3);
			//aS->mDir*=1.0f+gRand.GetF(1.5f);
			aS->mDirMult=Point(.77f,.77f);
			gWorld->mAnimatorList_Top+=aS;
		}
		
	}

/*
		for (float aCount=0;aCount<360;aCount+=15)
		{
			float aGrey=gRand.GetF(.1f)+.9f;
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mSmoke);
			aS->mPos=mPos+Point(0,ROBOTGROUND)+((gMath.AngleToVector(aCount)*60)*Point(1.0f,.15f));
			aS->mDir=gMath.AngleToVector(aCount)*Point(1,0);
			aS->mFade=2.0f-gRand.GetF(.5f);
			aS->mFadeSpeed=-.1f;
			aS->mRotation=gRand.GetF(360);
			aS->mRotationDir=gRand.NegateF(gRand.GetF(5)+5);
			aS->mScale=gRand.GetF(.35f)+.75f;
			aS->mColor=Color(aGrey,aGrey,aGrey);
			gWorld->mAnimatorList_Top+=aS;
		}
*/
}

void Kitty::Update()
{
	if (mNullKitty) return;

	if (mFirstUpdate) {FixPosition();mFirstUpdate=false;}

	CheckMyFeet();

	if (mFalling)
	{
		IPoint aGPos=gWorld->WorldToGrid(mPos);
		if (gWorld->GetGrid(aGPos.mX,aGPos.mY)==COSMIC_BLOCK) 
		{
			mGravity+=.25f*.2f;
			if (mGravity>1.5f) mGravity=1.5f;
		}
		else
		{
			mGravity+=.25f;
			if (mGravity>20) mGravity=20;
		}

		Point aMove=Move(Point(0.0f,mGravity),1);
		if (aMove.mY<mGravity*.9f)
		{
			mFalling=false;
			mGravity=0;
			IPoint aGrid=gWorld->WorldToGrid(mPos);
			mPos.mY=gWorld->GridToWorld(aGrid).mY-5;
		}

		if (gWorld->WorldToGrid(mPos).mY>gWorld->mGridHeight)
		{
			if (!gWorld->mVisibleRect.ContainsPoint(mPos))
			{
				DieKitty(1);
			}
		}
	}

	mTail+=.1f;
	if (mTail>=6) mTail-=6;

	mBlinkCountdown=_max(0,mBlinkCountdown-1);
	if (!mBlinkCountdown) 
	{
		if (gRand.Get(50)==1) mBlinkCountdown=10;
	}


	if (gMath.DistanceSquared(mPos,gWorld->mRobot->mPos)<35*35)
	{
		gWorld->Win();
	}
}

void Kitty::Draw()
{
	if (mNullKitty) return;
	gBundle_Play->mKitty.Center(mPos);
	gBundle_Play->mKitty_Tail[mTailFrame[(int)mTail]].Center(mPos);
	if (mBlinkCountdown) gBundle_Play->mKitty_Blink.Center(mPos);
}

Pickup::Pickup(void)
{
	mBob=gRand.GetF(360);
	mID=ID_PICKUP;
	mScale=1.0f;
	mShrink=false;
}

Pickup::~Pickup()
{
}

void Pickup::Update()
{
	mBob++;
	if (mBob>=360) mBob-=360;

	if (mShrink)
	{
		mScale-=.1f;
		if (mScale<=0.0f) Kill();
	}
	else
	{
		Point aVec=mPos-gWorld->mRobot->mPos;
		if (aVec.Length()<((float)gWorld->mGridSize/1.5f)) 
		{
			if (mType>=Pickup_ShootCancel && mType<=Pickup_HelmetCancel) gSounds->mPowerup.PlayPitched(.7f);
			else gSounds->mPowerup.Play();

			//gApp.mSounds.mPowerup.Play();
			mShrink=true;

			bool aAddToQueue=true;
			int aRemoveFromQueue=-1;

			switch (mType)
			{
			case Pickup_Jump:{gWorld->mButtons.EnableButton(gWorld->mButton_Jump,true);gWorld->mRobot->mCanJump=true;if (gApp.mTouchControls) gWorld->AddMessage("YOU CAN JUMP!","JUMP APP");else gWorld->AddMessage(Sprintf("PRESS '%s' TO JUMP!",gInput.GetShortKeyName(gApp.mJump).c()),"JUMP APP");break;}
			case Pickup_DoubleJump:{gWorld->mRobot->mCanDoubleJump=true;gWorld->AddMessage("YOU CAN DOUBLE JUMP!","JUMP ADDON");break;}
			case Pickup_Shoot:{gWorld->mButtons.EnableButton(gWorld->mButton_Shoot,true);gWorld->mRobot->mCanShoot=true;if (gApp.mTouchControls) gWorld->AddMessage("YOU CAN FIRE DEADLY LAZORZ!","DEFENSE APP");else gWorld->AddMessage(Sprintf("PRESS '%s' TO FIRE DEADLY LAZORS!",gInput.GetShortKeyName(gApp.mShoot).c()),"DEFENSE APP"); break;}
			case Pickup_Rocket:{gWorld->mButtons.EnableButton(gWorld->mButton_Rocket,true);gWorld->mRobot->mCanRocket=true;if (gApp.mTouchControls) gWorld->AddMessage("YOU CAN ROCKET AT LUDICROUS SPEED!","ZOOM APP");else gWorld->AddMessage(Sprintf("PRESS '%s' TO ROCKET AT LUDICROUS SPEED!",gInput.GetShortKeyName(gApp.mRocket).c()),"ZOOM APP");break;}
			case Pickup_RocketUp:{gWorld->mButtons.EnableButton(gWorld->mButton_RocketUp,true);gWorld->mRobot->mCanRocketUp=true;if (gApp.mTouchControls) gWorld->AddMessage("YOU CAN ROCKET SKYWARD!","LAUNCH APP");else gWorld->AddMessage(Sprintf("PRESS '%s' TO ROCKET SKYWARD!",gInput.GetShortKeyName(gApp.mRocketUp).c()),"LAUNCH APP");break;}
			case Pickup_Annihiliate:{gWorld->mRobot->mCanAnnihiliate=true;gWorld->AddMessage("YOU CAN ANNIHILIATE!","DEFENSE ADDON");break;}
			case Pickup_ShootUp:{if (gWorld->mRobot->mLazorLevel>0) aAddToQueue=false;gWorld->mRobot->mLazorLevel++;gWorld->AddMessage("LAZOR POWER UPGRADED!","GUN CONTROL APP");break;}
			case Pickup_Helmet:
				{
					gWorld->mRobot->mHasHelmet=true;gWorld->AddMessage("YOU CAN SMASH BRICKS ABOVE YOU!","HELMET APP");
					break;
				}
			case Pickup_Redkey:{aAddToQueue=false;gWorld->mRobot->mHasKey[0]=true;gWorld->AddMessage("YOU CAN OPEN RED GATES NOW!","RED KEYCARD");break;}
			case Pickup_Greenkey:{aAddToQueue=false;gWorld->mRobot->mHasKey[1]=true;gWorld->AddMessage("YOU CAN OPEN GREEN GATES!","GREEN KEYCARD");break;}
			case Pickup_Bluekey:{aAddToQueue=false;gWorld->mRobot->mHasKey[2]=true;gWorld->AddMessage("BLUE GATES CAN'T STOP YOU!","BLUE KEYCARD");break;}
			case Pickup_Goldkey:{aAddToQueue=false;gWorld->mRobot->mGoldKeyCount++;gWorld->AddMessage("OPENS ONE GOLDEN GATE!","GOLD KEYCARD");break;}
			case Pickup_Explozor:{gWorld->mRobot->mHasExplozor=true;gWorld->AddMessage("YOUR LAZORZ ARE EXPLOZORS!","DEFENSE ADDON");break;}
			case Pickup_Haxxor:{gWorld->mRobot->mHasHaxxor=true;gWorld->AddMessage("MAKES SAD COMPUTERS HAPPY!","HAXXOR APP");break;}
			case Pickup_Timesaver:{aAddToQueue=false;gWorld->mTimerPause+=(10*50);gWorld->AddMessage("SAVES TEN SECONDS!","TIMESAVER APP");break;}
			case Pickup_Teleport:{aAddToQueue=false;gWorld->mRobot->mCrystals++;gWorld->AddMessage("POWERS A TELEMATIC","QUANTUM CRYSTAL");break;}
			case Pickup_Life:{aAddToQueue=false;gWorld->mRobot->mMaxHP++;gWorld->mRobot->mHP++;gWorld->AddMessage("ADDS A ROBOHEART","HEART APP");break;}

			//case Pickup_JumpCancel:{gWorld->mButtons.EnableButton(gWorld->mButton_Jump,false);gWorld->mRobot->mCanJump=false;gWorld->AddMessage("DELETED","JUMP APP");break;}
			//case Pickup_DoubleJumpCancel:{gWorld->mButtons.EnableButton(gWorld->mButton_Jump,false);gWorld->mRobot->mCanDoubleJump=false;gWorld->AddMessage("DELETED","JUMP ADDON");break;}
			case Pickup_ShootCancel:{aAddToQueue=false;aRemoveFromQueue=Pickup_Shoot;gWorld->mButtons.EnableButton(gWorld->mButton_Shoot,false);gWorld->mRobot->mCanShoot=false;gWorld->AddMessage("DELETED","DEFENSE APP");break;}
			//case Pickup_RocketCancel:{gWorld->mButtons.EnableButton(gWorld->mButton_Rocket,false);gWorld->mRobot->mCanRocket=false;gWorld->AddMessage("DELETED","ZOOM APP");break;}
			case Pickup_RocketUpCancel:{aAddToQueue=false;aRemoveFromQueue=Pickup_RocketUp;gWorld->mButtons.EnableButton(gWorld->mButton_RocketUp,false);gWorld->mRobot->mCanRocketUp=false;gWorld->AddMessage("DELETED","LAUNCH APP");break;}
			//case Pickup_AnnihiliateCancel:{gWorld->mRobot->mCanAnnihiliate=false;gWorld->AddMessage("DELETED","DEFENSE ADDON");break;}
			case Pickup_HelmetCancel:{aAddToQueue=false;aRemoveFromQueue=Pickup_Helmet;gWorld->mRobot->mHasHelmet=false;gWorld->AddMessage("DELETED","HELMET APP");break;}
			//case Pickup_ExplozorCancel:{gWorld->mRobot->mHasExplozor=false;gWorld->AddMessage("DELETED","DEFENSE ADDON");break;}
			case Pickup_Velcro:{gWorld->mButtons.EnableButton(gWorld->mButton_Jump,true);gWorld->mRobot->mHasVelcro=true;gWorld->AddMessage("STICK TO SOME WALLS!","ELECTRIC VELCRO");break;}
			}

			if (aAddToQueue) 
			{
				int aType=4+mType;
				if (mType>=Pickup_Explozor) aType=25+(mType-Pickup_Explozor);
				if (mType==Pickup_Life) aType=42;
				gWorld->mRobot->mPowerupOrder+=aType;
				gWorld->mRobot->mPowerupCount=gWorld->mRobot->mPowerupOrder.Size();
			}
			if (aRemoveFromQueue>=0)
			{
				aRemoveFromQueue+=4;
				for (int aCount=0;aCount<gWorld->mRobot->mPowerupOrder.Size();aCount++)
				{
					if (gWorld->mRobot->mPowerupOrder[aCount]==aRemoveFromQueue)
					{
						gWorld->mRobot->mPowerupOrder.DeleteElement(aCount);
						aCount--;
					}
				}
				gWorld->mRobot->mPowerupCount=gWorld->mRobot->mPowerupOrder.Size();
			}
		}
	}
}

void Pickup::Draw()
{
	bool aJiggle=false;

	int aType=4+mType;
	if (mType>=Pickup_Explozor) aType=25+(mType-Pickup_Explozor);
	if (mType==Pickup_Life) aType=42;
	if (mType==Pickup_ShootUp) aType=72;
	if (mType>=Pickup_ShootCancel && mType<=Pickup_HelmetCancel) 
	{
		aType=44+(mType-Pickup_ShootCancel);
		aJiggle=true;
	}
	if (mType==Pickup_Goldkey) aType=50;
	if (mType==Pickup_Velcro) aType=PICKUP_VELCRO;

	Sprite *aSprite=&gBundle_Tiles->mBlock[aType];
	if (aSprite)
	{

		if (mType==Pickup_Haxxor)
		{
			
			float aMod=(1.0f+(gMath.Sin(mBob*8)*.025f))*mScale;
			aSprite->DrawScaled(mPos+Point(-.5f,0.0f),aMod);

			if (mScale>.95f)
			{
				{
					gG.RenderAdditive();
					gG.SetColor(gRand.GetF(.75f)+.2f);
					gBundle_Play->mHaxxor.DrawRotatedScaled(mPos+Point(-.5f,0.0f),0,aMod);
					gG.RenderNormal();
					gG.SetColor();
				}
				gBundle_Play->mHaxxormask.DrawScaled(mPos+Point(-.5f,0.0f),aMod);
			}
		}
		else
		{
			//aSprite->DrawScaled(mPos,(1.0f+(gMath.Sin(mBob*8)*.05f))*mScale);
			if (aJiggle) 
			{
				aSprite->DrawRotatedScaled(mPos,gRand.GetSpanF(-6,6),1.0f*mScale);//(1.0f+(gMath.Sin(mBob*8)*.05f))*mScale);
				gBundle_Tiles->mDeleteapp.DrawRotatedScaled(mPos,gRand.GetSpanF(-6,6),1.0f*mScale);//(1.0f+(gMath.Sin(mBob*8)*.05f))*mScale);
			}
			else aSprite->DrawScaled(mPos,(1.0f+(gMath.Sin(mBob*8)*.05f))*mScale);
		}
	}
}


Computron::Computron(void)
{
	mID=ID_COMPUTRON;
	mActivated=false;
	mEyeOffset=0;
	mEyeDir=gRand.NegateF(1);
	mEyeWait=0;
	mRandCountdown=0;
	mBlinkCountdown=0;
	mBlinkWait=100;
	mFlash=0;
}

Computron::~Computron()
{
}

void Computron::Update()
{
	mFlash=_max(0,mFlash-.1f);
	if (!mActivated)
	{
		Point aVec=mPos-gWorld->mRobot->mPos;
		if (aVec.Length()<((float)gWorld->mGridSize/1.5f)) 
		{
			gWorld->TurnOffComputrons();

			mActivated=true;
			gSounds->mComputron.Play();
			mEyeOffset=0;
			mEyeDir=gRand.NegateF(1);
			mEyeWait=100;
			gWorld->AddMessage("ROBOT PATTERN ENCODED","TRANSMATTER COMPUTRON",8);
			gWorld->mSpawnSpot=mPos;
			gWorld->mSpawnMusic=gWorld->mCurrentMusic;
			gWorld->mRobot->mHP=gWorld->mRobot->mMaxHP;
			if (gWorld->mWantNewMusic) gWorld->mSpawnMusic=gWorld->mNewMusic;

			mFlash=1.0f;
		}
	}
	else
	{
		Point aVec=mPos-gWorld->mRobot->mPos;
		if (aVec.Length()<((float)gWorld->mGridSize/1.5f)) 
		{
			if (gWorld->mRobot->mHP<gWorld->mRobot->mMaxHP)
			{
				gSounds->mComputron.Play();
				gWorld->AddMessage("ROBOT PATTERN ENCODED","TRANSMATTER COMPUTRON",8);
			}
			gWorld->mRobot->mHP=gWorld->mRobot->mMaxHP;
		}

		mBlinkCountdown=_max(0,mBlinkCountdown-1);
		if (--mRandCountdown<=0)
		{
			mRandSeed=gRand.Get(10000);
			mRandCountdown=gRand.Get(5)+5;
		}
		mEyeWait=_max(mEyeWait-1,0);
		if (mEyeWait<=0)
		{
			mEyeOffset+=mEyeDir;//*.25f;
			if (mEyeOffset>8)
			{
				mEyeWait=15+gRand.Get(50);
				mEyeOffset=8;
				mEyeDir=-1;
			}
			if (mEyeOffset<-8)
			{
				mEyeWait=15+gRand.Get(50);
				mEyeOffset=-8;
				mEyeDir=1;
			}
		}

		if (--mBlinkWait<=0) 
		{
			mBlinkCountdown=gRand.Get(10)+5;
			mBlinkWait=50+gRand.Get(100);
			if (gRand.Get(10)==3) mBlinkWait=5+gRand.Get(10);
		}
	}
}

void Computron::Draw()
{
	if (mActivated)
	{
		gBundle_Play->mComputron_Activated.Center(mPos+Point(0,-8));
		if (!mBlinkCountdown) gBundle_Play->mComputron_Eye.Center(mPos+Point(mEyeOffset,-8.0f));
		else gBundle_Play->mComputron_Blink.Center(mPos+Point(mEyeOffset,-8.0f));

		Random aRand;
		aRand.Seed(mRandSeed);
		int aRandMax=2+aRand.Get(5);
		bool aMultiColor=true;//aRand.GetBool();

		for (float aSpanX=6;aSpanX>=0;aSpanX--)
		{
			for (float aSPanY=4;aSPanY>=0;aSPanY--)
			{
				if (aRand.Get(aRandMax)==0)
				{
					if (aMultiColor)
					{
						Color aColor;
						aColor.Primary(aRand.Get(5));
						//aColor.Pastel(.5f);
						gG.SetColor(aColor,aRand.GetF(.5f)+.5f);
					}
					gBundle_Play->mComputron_Glint.Center(mPos+Point(-17,-6)+Point(aSpanX*4.65f,aSPanY*4.5f));
					gG.SetColor(gRand.GetF(.5f)+.5f);
					gBundle_Play->mComputron_Glintwhite.Center(mPos+Point(-17.5f,-6.5f)+Point(aSpanX*4.65f,aSPanY*4.5f));
				}
			}
		}
		gG.SetColor();
	}
	else gBundle_Play->mComputron.Center(mPos+Point(0,-8));

	if (mFlash)
	{
		gG.RenderWhite(true);
		gG.SetColor(mFlash);
		gBundle_Play->mComputron.Center(mPos+Point(0,-8));
		gG.SetColor();
		gG.RenderWhite(false);
	}

}

RedGuy::RedGuy(void)
{
	mID=ID_REDGUY;
	mChomp=gRand.GetF(360);
	mStep=0;
	mFacing=1;
	mChompSpeed=gRand.GetF(10)+6;
	mHP=3;
	mFlashWhite=0;
	mIsEnemy=true;
	mIsChild=false;

	mCollide=Rect(-15,-25,30,45);
	mDying=false;
	mDyingCountdown=20;
	mAge=0;

	mIsFalling=false;
	mFallSpeed=0;
	mChecker=0;
}

RedGuy::~RedGuy()
{
	if (mIsChild) gRedChildren=_max(0,gRedChildren-1);
}


void RedGuy::Update()
{
	Player::Update();
	mFlashWhite=_max(0,mFlashWhite-.1f);

	mChecker++;
	if (mChecker>=5) mChecker=0;

	if (mDying)
	{
		mDyingCountdown--;
		if (mDyingCountdown<=0)
		{
			// Explode
			Kill();
			if (!mIsChild) gWorld->mTimerPause+=(1*50);
			gSounds->mLaserouch.PlayPitched(1.0f+gRand.GetSpanF(.05f));
			//gWorld->mFlashWhite=1.0f;

			for (float aCount=0;aCount<360;aCount+=30)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.1f;
				aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(2));
				aS->mPos=mPos+(aS->mDir*3);
				aS->mColor=Color(1,0,0);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.75f,.75f);
				gWorld->mAnimatorList_Top+=aS;
			}
			for (float aCount=0;aCount<360;aCount+=30)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.1f;
				aS->mDir=gMath.AngleToVector(aMyAngle)*(9+gRand.GetSpanF(2));
				aS->mPos=mPos+(aS->mDir*3);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.77f,.77f);
				gWorld->mAnimatorList_Top+=aS;
			}
		}
	}
	else
	{
		//
		// Did we hit Robot?
		//
		{
			Rect aRect=mCollide;
//			aRect.mX-=5;
//			aRect.mWidth+=10;
			aRect.mX+=mPos.mX;
			aRect.mY+=mPos.mY;

			Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
//			aRobotRect.mX-=5;
//			aRobotRect.mWidth+=10;
			aRobotRect.mY+=5;
			aRobotRect.mHeight-=10;
			aRobotRect.mX+=gWorld->mRobot->mPos.mX;
			aRobotRect.mY+=gWorld->mRobot->mPos.mY;

			if (aRobotRect.Intersects(aRect))
			{
				gWorld->mRobot->Die(false);
			}
		}

		if (mIsFalling)
		{
			IPoint aGPos=gWorld->WorldToGrid(mPos);
			float aMod=1.0f;
			float aMax=20;
			int aOnGrid=gWorld->GetGrid(aGPos.mX,aGPos.mY);
			if (aOnGrid)
			{
				if (aOnGrid==COSMIC_BLOCK) {aMod=.2f;aMax=1.5f;}
				if (ISSPIKE(aOnGrid)) Ouch();
			}


			mFallSpeed=_min(mFallSpeed+(.5f*aMod),aMax);
			Point aMove=Move(Point(0,1),mFallSpeed);
			if (aMove.Length()<=0) mIsFalling=false;

			mChomp+=mChompSpeed;
			if (mChomp>=360) mChomp-=360;

			mStep+=(15)*1.5f;
			if (mStep>=3600) mStep-=3600;
		}
		else
		{
			mChomp+=mChompSpeed;
			if (mChomp>=360) mChomp-=360;

			mStep+=(15)*1.5f;
			if (mStep>=3600) mStep-=3600;

			float aOldFacing=mFacing;


			Point aTest=Point(mPos.mX+(-mFacing*(gWorld->mGridSize/3)),mPos.mY);
			if (gWorld->IsBlockedDir(gWorld->WorldToGrid(aTest),-mFacing)) mFacing=-aOldFacing;	
			aTest.mY+=(gWorld->mGridSize/2)+3;
			if (!gWorld->IsBlocked(gWorld->WorldToGrid(aTest),false)) mFacing=-aOldFacing;


			mPos.mX+=(-mFacing*.75f)*1.5f;

			if (mChecker==0)
			{
				Point aHold=mPos;
				mPos+=Point(0,20);
				Point aMove=Move(Point(0,1),5);
				if (aMove.Length()>=2)
				{
					mIsFalling=true;
					mFallSpeed=0;
					aHold.mY+=5;
				}
				mPos=aHold;
			}
		}
	}
}

void RedGuy::CheckMyFeet()
{
	if (mDying) return;

	IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(0,22));
	World::Grid* aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	float aMoveDir=0;

	IPoint aGooPos;
	bool aLava=false;
	if (aG && aG->mLayout==2) 
	{
		aLava=true;
		aGooPos=aGridPos;
	}

	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}

	aGridPos=gWorld->WorldToGrid(mPos+Point(-15,22));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}

	aGridPos=gWorld->WorldToGrid(mPos+Point(+15,22));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}


	if (aMoveDir) Move(Point(gMath.Sign(aMoveDir),0.0f),CONVEYOR_SPEED);

	if (aLava) 
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}

}

void RedBoss::CheckMyFeet()
{
	IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(0,22));
	World::Grid* aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	float aMoveDir=0;

	IPoint aGooPos;
	bool aLava=false;
	if (aG && aG->mLayout==2) 
	{
		aLava=true;
		aGooPos=aGridPos;
	}

	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}

	aGridPos=gWorld->WorldToGrid(mPos+Point(-50,22));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}

	aGridPos=gWorld->WorldToGrid(mPos+Point(+50,22));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}


	if (aMoveDir) Move(Point(gMath.Sign(aMoveDir),0.0f),CONVEYOR_SPEED);

	if (aLava) 
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}

}

void Boss::CheckMyFeet()
{
	IPoint aGridPos=gWorld->WorldToGrid(mPos+Point(0,22));
	World::Grid* aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	float aMoveDir=0;

	IPoint aGooPos;
	bool aLava=false;
	if (aG && aG->mLayout==2) 
	{
		aLava=true;
		aGooPos=aGridPos;
	}

	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}

	aGridPos=gWorld->WorldToGrid(mPos+Point(-62,22));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}

	aGridPos=gWorld->WorldToGrid(mPos+Point(+62,22));
	aG=gWorld->GetGridPtr(aGridPos.mX,aGridPos.mY);
	if (aG && aG->mCustomDraw)
	{
		if (aG->mLayout==CONVEYOR_LEFT) aMoveDir+=-1;
		if (aG->mLayout==CONVEYOR_RIGHT) aMoveDir+=1;
	}


	if (aMoveDir) Move(Point(gMath.Sign(aMoveDir),0.0f),CONVEYOR_SPEED);

	if (aLava) 
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}

}


void RedGuy::Draw()
{
	DrawGuy();
	if (mDying)
	{
		gG.SetColor(1.0f-((float)mDyingCountdown/20.0f));
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}
	else if (mFlashWhite)
	{
		gG.SetColor(mFlashWhite);
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}

/*
	gG.SetColor(.5f);
	gG.Translate(mPos);
	gG.FillRect(mCollide);
	gG.Translate(-mPos);
	gG.SetColor();
*/
}

void RedGuy::DrawGuy()
{
	Point aShake;
	if (mDying)
	{
		aShake=gMath.AngleToVector()*gRand.GetF(6);
		gG.Translate(aShake);
	}
//	gG.Translate(0,-2);
	Matrix aMat;
	aMat.Scale(mFacing,1,1);
	aMat.Translate(mPos);


	Point aLeg=gMath.AngleToVector(-mStep*mFacing)*4;
	if (aLeg.mY>0) aLeg.mY=0;

	Array<Sprite>* aRedguy=&gBundle_Play->mRedguy;
	switch (mEnergyLevel)
	{
	case 1:aRedguy=&gBundle_Play->mRedguy1;break;
	case 2:aRedguy=&gBundle_Play->mRedguy2;break;
	case 3:aRedguy=&gBundle_Play->mRedguy3;break;
	}

	gG.Translate(aLeg);
	aRedguy->Element(2).Draw(aMat);
	gG.Translate(-aLeg);

	aRedguy->Element(3).Draw(aMat);

	aLeg=gMath.AngleToVector(-(mStep*mFacing)+180)*Point(3,4);
	if (aLeg.mY>0) aLeg.mY=0;
	gG.Translate(aLeg);
	aRedguy->Element(4).Draw(aMat);
	gG.Translate(-aLeg);

	float aChomp=-(float)fabs(gMath.Sin(mChomp)*5);
	gG.Translate(0.0f,aChomp);
	aRedguy->Element(5).Draw(aMat);

	int aBob=gAppPtr->AppTime()%30;
	if (aBob>=0 && aBob<=5) gG.Translate(0,2);
	aRedguy->Element(0).Draw(aMat);
	if (aBob>=0 && aBob<=5) gG.Translate(0,-2);
	if (aBob>=15 && aBob<=20) gG.Translate(0,2);
	aRedguy->Element(1).Draw(aMat);
	if (aBob>=15 && aBob<=20) gG.Translate(0,-2);
	gG.Translate(0.0f,-aChomp);
	//gG.Translate(0,2);
	if (mDying) gG.Translate(-aShake);
}

void RedGuy::Ouch(int theExtraDamage)
{
	mFlashWhite=1.0f;
	mHP-=(theExtraDamage+1);
	if (mHP>0) mFacing*=-1;
	else
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}
}

BlueGuy::BlueGuy(void)
{
	mID=ID_BLUEGUY;
	mFrame=gRand.GetF(4);
	mFrameDir=gRand.NegateF(1);
	mFlashWhite=0.0f;
	mIsEnemy=true;
	mBlinkCountdown=0;

	mCollide=Rect(-15,-25,30,45);
	mDying=false;
	mDyingCountdown=20;

	mLookatAngle=mWantLookatAngle=gRand.GetF(360);
	mLookatDistance=mWantLookatDistance=4;

	mDir=-1;
	mHP=1;
}

BlueGuy::~BlueGuy()
{
}

void BlueGuy::Update()
{
	Player::Update();

	mFlashWhite=_max(0,mFlashWhite-.1f);

	float aStep=.15f*mFrameDir;
	float aOldFrame=mFrame;
	mFrame+=aStep;
	if (mFrame>=4) mFrame-=4;
	if (mFrame<0) mFrame+=4;

	if (mDying)
	{
		mBlinkCountdown=0;
		mDyingCountdown--;
		mWantLookatAngle=mLookatAngle=gRand.GetF(360);
		mWantLookatDistance=mLookatDistance=gRand.GetF(4);
		if (mDyingCountdown<=0)
		{
			// Explode
			Kill();
			gWorld->mTimerPause+=(1*50);
			gSounds->mLaserouch.PlayPitched(1.0f+gRand.GetSpanF(.05f));
			//gWorld->mFlashWhite=1.0f;

			for (float aCount=0;aCount<360;aCount+=30)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.1f;
				aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(2));
				aS->mPos=mPos+(aS->mDir*3);
				aS->mColor=Color(.3f,.5f,1.0f);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.75f,.75f);
				gWorld->mAnimatorList_Top+=aS;
			}
			for (float aCount=0;aCount<360;aCount+=30)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.1f;
				aS->mDir=gMath.AngleToVector(aMyAngle)*(9+gRand.GetSpanF(2));
				aS->mPos=mPos+(aS->mDir*3);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.77f,.77f);
				gWorld->mAnimatorList_Top+=aS;
			}
		}
	}
	else
	{
		//
		// Did we hit Robot?
		//
		{
			Rect aRect=mCollide;
			//			aRect.mX-=5;
			//			aRect.mWidth+=10;
			aRect.mX+=mPos.mX;
			aRect.mY+=mPos.mY;

			Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
			//			aRobotRect.mX-=5;
			//			aRobotRect.mWidth+=10;
			aRobotRect.mY+=5;
			aRobotRect.mHeight-=10;

			aRobotRect.mX+=gWorld->mRobot->mPos.mX;
			aRobotRect.mY+=gWorld->mRobot->mPos.mY;

			if (aRobotRect.Intersects(aRect))
			{
				gWorld->mRobot->Die(false);
			}
		}

		mPos.mY+=mDir;
		Point aTestPos=mPos+Point(0.0f,(mDir*20)+3);
		IPoint aTestGrid=gWorld->WorldToGrid(aTestPos);
		if (gWorld->IsBlocked(aTestGrid)) mDir*=-1;

		mBlinkCountdown=_max(0,mBlinkCountdown-1);
		if (!mBlinkCountdown) 
		{
			if (gRand.Get(100)==1)
			{
				mBlinkCountdown=10;
				mWantLookatAngle=mLookatAngle=gRand.GetF(360);
				mWantLookatDistance=mLookatDistance=4;
			}

		}

		if (gRand.Get(200)==5)
		{
			mWantLookatAngle=gRand.GetF(360);
			mWantLookatDistance=gRand.GetF(4);
		}

		float aBest=gMath.GetBestRotation(mLookatAngle,mWantLookatAngle);
		mLookatAngle+=aBest*2;
		float aDiff=mWantLookatDistance-mLookatDistance;
		mLookatDistance+=aDiff*.1f;
	}




}

void BlueGuy::Draw()
{
	DrawGuy();
	if (mDying)
	{
		gG.SetColor(1.0f-((float)mDyingCountdown/20.0f));
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}
	else if (mFlashWhite)
	{
		gG.SetColor(mFlashWhite);
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}
}

void BlueGuy::DrawGuy()
{
	Point aShake;
	if (mDying)
	{
		aShake=gMath.AngleToVector()*gRand.GetF(6);
		gG.Translate(aShake);
	}

	switch (mEnergyLevel)
	{
	case 0:gBundle_Play->mBlueguy[(int)mFrame].Center(mPos);break;
	case 1:gBundle_Play->mBlueguy1[(int)mFrame].Center(mPos);break;
	case 2:gBundle_Play->mBlueguy2[(int)mFrame].Center(mPos);break;
	case 3:gBundle_Play->mBlueguy3[(int)mFrame].Center(mPos);break;
	}
	if (mBlinkCountdown) gBundle_Play->mBlueguy_Blink[_min(3,mEnergyLevel)].Center(mPos);
	else gBundle_Play->mBlueguy_Pupile.Center(mPos+(gMath.AngleToVector(mLookatAngle)*mLookatDistance));
	if (mDying)
	{
		gG.Translate(-aShake);
	}
}

void BlueGuy::Ouch(int theExtraDamage)
{
	mFlashWhite=1.0f;
	mHP-=(theExtraDamage+1);

	if (gHelmetOuch) 
	{
		mHP=-1;
		mDyingCountdown=-1;
	}
	if (mHP<=0)
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}
}

Drip::Drip(void)
{
	mID=ID_DRIP;
	mFrame=gRand.GetF(4);
	mFlashWhite=0.0f;
	mIsEnemy=true;

	mCollide=Rect(-15,-25,30,45);
	mDying=false;
	mDyingCountdown=20;

	mHP=4;

	//mShootCountdown=gRand.Get(70);
	mShootCountdown=-1;
	mShootPause=0;

	mCheckFooting=0;
}

Drip::~Drip()
{
}

void Drip::Update()
{
	if (mShootCountdown==-1) mShootCountdown=QuickRand((int)(mPos.mX*mPos.mY))%70;
	Player::Update();
	mFlashWhite=_max(0,mFlashWhite-.1f);

	if (++mCheckFooting>5) mCheckFooting=0;

	if (mDying)
	{
		mDyingCountdown--;
		if (mDyingCountdown<=0)
		{
			// Explode
			Kill();
			gWorld->mTimerPause+=(1*50);
			gSounds->mLaserouch.PlayPitched(1.0f+gRand.GetSpanF(.05f));
			//gWorld->mFlashWhite=1.0f;

			for (float aCount=0;aCount<360;aCount+=30)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.1f;
				aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(2));
				aS->mPos=mPos+(aS->mDir*3);
				aS->mColor=Color(1,.5f,0);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.75f,.75f);
				gWorld->mAnimatorList_Top+=aS;
			}
			for (float aCount=0;aCount<360;aCount+=30)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.1f;
				aS->mDir=gMath.AngleToVector(aMyAngle)*(9+gRand.GetSpanF(2));
				aS->mPos=mPos+(aS->mDir*3);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.77f,.77f);
				gWorld->mAnimatorList_Top+=aS;
			}
		}
	}
	else
	{
		if (mCheckFooting==0)
		{
			IPoint aMyPos=gWorld->WorldToGrid(mPos);
			if (gWorld->GetGrid(aMyPos.mX,aMyPos.mY-1)==0)
			{
				mHP=-1;
				mDyingCountdown=-1;
				mDying=true;
			}
		}

		if (mShootPause)
		{
			mShootPause=_max(0,mShootPause-1);
			if (mShootPause==0) mShootCountdown=70;
			if (mShootPause>15) mFrame=4;
			else mFrame=0;

			if (mShootPause==15)
			{
				mFlashWhite=.5f;
				gSounds->mDrip.Play(gWorld->GetVolume(mPos));
				Acid *aAcid=new Acid();
				aAcid->mEnergyLevel=mEnergyLevel;
				aAcid->mMyDrip=this;
				aAcid->mPos=mPos+Point(0,20);
				gWorld->mPlayerList+=aAcid;
			}
		}
		else
		{
			mFrame+=.1f;
			if (mFrame>=4) mFrame-=4;
		
			if (--mShootCountdown<=0) mShootPause=30;
			//			if (mShootCountdown<0) mShootCountdown=1;
		}

		{
			Rect aRect=mCollide;
			aRect.mX+=mPos.mX;
			aRect.mY+=mPos.mY;

			Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
			aRobotRect.mY+=5;
			aRobotRect.mHeight-=10;
			aRobotRect.mX+=gWorld->mRobot->mPos.mX;
			aRobotRect.mY+=gWorld->mRobot->mPos.mY;

			if (aRobotRect.Intersects(aRect))
			{
				gWorld->mRobot->Die(false);
			}
		}

	}
}

void Drip::Draw()
{
	DrawGuy();
	if (mDying)
	{
		gG.SetColor(1.0f-((float)mDyingCountdown/20.0f));
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}
	else if (mFlashWhite)
	{
		gG.SetColor(mFlashWhite);
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}
}

void Drip::DrawGuy()
{
	Point aShake;
	if (mDying)
	{
		aShake=gMath.AngleToVector()*gRand.GetF(6);
		aShake.mY=0;
		gG.Translate(aShake);
	}

	Array<Sprite>* aDrip=&gBundle_Play->mDrip;
	switch (mEnergyLevel)
	{
	case 1:aDrip=&gBundle_Play->mDrip1;break;
	case 2:aDrip=&gBundle_Play->mDrip2;break;
	case 3:aDrip=&gBundle_Play->mDrip3;break;
	}
	aDrip->Element((int)mFrame).Center(mPos+Point(0,3));

	if (mDying) gG.Translate(-aShake);
}

void Drip::Ouch(int theExtraDamage)
{
	mFlashWhite=1.0f;
	mHP-=theExtraDamage+1;

	if (gHelmetOuch) 
	{
		mHP=-1;
		mDyingCountdown=-1;
	}
	if (mHP<=0)
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}
}


Acid::Acid()
{
	mID=ID_ACID;
	mRotate=gRand.GetF(360);
	mFallSpeed=5;
	mMyDrip=NULL;
}

Acid::~Acid()
{
}

void Acid::Energize()
{
	mEnergyLevel++;
	if (mMyDrip) if (mMyDrip->mEnergyLevel<3) mMyDrip->Energize();
}

void Acid::Update()
{
	Player::Update();
	
	float aMod=1.0f;
	IPoint aGridPos=gWorld->WorldToGrid(mPos);
	if (gWorld->GetGrid(aGridPos.mX,aGridPos.mY)==COSMIC_BLOCK) aMod=.2f;
	mRotate+=30;
	mPos.mY+=mFallSpeed*aMod;
	if (gWorld->IsBlocked(aGridPos))
	{
		Plop();
	}
	else if (gRand.Get(2)==1)
	{
		Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mAciddot);
		aS->mScale=gRand.GetF(.5f)+.75f;
		aS->mPos=mPos+(gMath.AngleToVector()*gRand.GetF(8));
		aS->mFade=1.0f;
		aS->mFadeSpeed=-(.1f+gRand.GetF(.05f));
		gWorld->mAnimatorList_Top+=aS;
	}

	Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
	aRobotRect.mY+=5;
	aRobotRect.mHeight-=10;
	aRobotRect.mX-=5;
	aRobotRect.mWidth+=10;
	aRobotRect.mX+=gWorld->mRobot->mPos.mX;
	aRobotRect.mY+=gWorld->mRobot->mPos.mY;

	if (aRobotRect.ContainsPoint(mPos.mX,mPos.mY))
	{
		if (!gWorld->mRobot->mHasHelmet) gWorld->mRobot->Die(false);
		else gSounds->mDriphelm.Play();
		Plop();
	}

	if (!gWorld->mWakeRect.ContainsPoint(mPos.mX,mPos.mY)) Kill();

}

void Acid::Plop()
{
	gSounds->mPlop.Play(gWorld->GetVolume(mPos));

	Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mJumpburst);
	aS->mPos=mPos;
	aS->mFadeSpeed=-.1f;
	aS->mRotation=0;
	aS->mColor=Color(.5f,1,.5f);
	aS->mRotationDir=gRand.NegateF(5);
	gWorld->mAnimatorList_UnderWorld+=aS;


	for (int aAngle=-90;aAngle<90;aAngle+=10)
	{
		Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mAciddot);
		aS->mScale=gRand.GetF(.75f)+1.25f;
		aS->mDir=gMath.AngleToVector(aAngle+gRand.GetSpanF(5))*(3+gRand.GetF(2));
		aS->mPos=mPos+(aS->mDir);
		aS->mGravity=Point(0.0f,.5f);
		aS->mFade=1.0f;
		aS->mFadeSpeed=-(.1f+gRand.GetF(.05f));
		gWorld->mAnimatorList_Top+=aS;

	}
	Kill();
}

void Acid::Draw()
{
	gBundle_Play->mAcid.DrawRotated(mPos,gMath.Sin(mRotate)*10);
}

int gBossFrame[]={0,1,2,2,2,1,0};

Boss::Boss()
{
	mIsFalling=false;
	mBoss=NULL;
	mFallSpeed=0;
	mID=ID_BOSS;
	mIsEnemy=true;
	mCollide=Rect(-60,-60,120,80);
	mDying=false;
	mDyingCountdown=40;
	mFlashWhite=0.0f;

	mHP=40;
	mMouthFrame=0;
	mSlimeWait=0;

	for (int aCount=0;aCount<11;aCount++)
	{
		mBlinkCountdown[aCount]=0;
		mLookat[aCount]=gRand.GetF(360);
	}

	mDir=0;//gRand.NegateF(1);
	mSpeed=1;

	mBob=gRand.GetF(360);
	mFireCountdown=10;
	mSpeedDownCounter=500;
}

Boss::~Boss()
{
}

void Boss::FixBossGraphic()
{
	mBoss=&gBundle_Play->mBoss0;
	switch (mEnergyLevel)
	{
	case 1:mBoss=&gBundle_Play->mBoss1;break;
	case 2:mBoss=&gBundle_Play->mBoss2;break;
	case 3:mBoss=&gBundle_Play->mBoss3;break;
	}
}

Array<Sprite>* FixRedBossGraphic(int theEnergyLevel)
{
	Array<Sprite>* aBoss;
	aBoss=&gBundle_Play->mRedboss;
	switch (theEnergyLevel)
	{
	case 1:aBoss=&gBundle_Play->mRedboss1;break;
	case 2:aBoss=&gBundle_Play->mRedboss2;break;
	case 3:aBoss=&gBundle_Play->mRedboss3;break;
	}
	return aBoss;
}

void RedBoss::FixBossGraphic() {mBoss=FixRedBossGraphic(mEnergyLevel);}

void DripBoss::FixBossGraphic()
{
	mBoss=&gBundle_Play->mDripboss;
	switch (mEnergyLevel)
	{
	case 1:mBoss=&gBundle_Play->mDripboss1;break;
	case 2:mBoss=&gBundle_Play->mDripboss2;break;
	case 3:mBoss=&gBundle_Play->mDripboss3;break;
	}
}

void Boss::Update()
{
	if (mDir==0) mDir=(QuickRand((int)(mPos.mX*mPos.mY))%2)?1.0f:-1.0f;
	if (!mBoss) FixBossGraphic();
	Player::Update();
	mFlashWhite=_max(0,mFlashWhite-.1f);

	if (--mSpeedDownCounter<=0)
	{
		mSpeedDownCounter=0;
		mSpeed-=.0025f;
		if (mSpeed<1) mSpeed=1;
	}


	if (mDying)
	{
		mDyingCountdown--;
		if (mDyingCountdown<=0)
		{
			// Explode
			Kill();

			gApp.GetAchievement("DavidBotVsGoliathoid");
			gSounds->mLaserouch.PlayPitched(1.0f+gRand.GetSpanF(.05f));
			gSounds->mKillboss.Play();
			gWorld->mFlashWhite=1.0f;

			gWorld->mFlashWhite=1.0f;
			gWorld->Shake(20);

			gWorld->KilledBoss(gWorld->WorldToGrid(mPos));

			for (float aCount=0;aCount<360;aCount+=10)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.025f;
				aS->mScale=1.0f+gRand.GetF(.5f);
				aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(8));
				aS->mPos=mPos+(aS->mDir*(3+gRand.GetF(5)));
				aS->mColor=Color(.3f,.5f,1.0f);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.85f,.85f);
				gWorld->mAnimatorList_Top+=aS;
			}
			for (float aCount=0;aCount<360;aCount+=10)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.025f;
				aS->mScale=1.0f+gRand.GetF(.5f);
				aS->mDir=gMath.AngleToVector(aMyAngle)*(10+gRand.GetSpanF(10));
				aS->mPos=mPos+(aS->mDir*(3+gRand.GetF(5)));
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.87f,.87f);
				gWorld->mAnimatorList_Top+=aS;
			}

			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mJumpburst);
			aS->mPos=mPos;
			aS->mFadeSpeed=-.1f;
			aS->mRotation=0;
			aS->mScale=3.0f;
			aS->mColor=Color(.5f,1,.5f);
			aS->mRotationDir=gRand.NegateF(5);
			gWorld->mAnimatorList_UnderWorld+=aS;

			for (int aAngle=-90;aAngle<90;aAngle+=5)
			{
				Anim_Sprite *aS=new Anim_Sprite(&mBoss->Element(3));
				aS->mScale=gRand.GetF(.25f)+.75f;
				aS->mDir=gMath.AngleToVector(aAngle+gRand.GetSpanF(10))*(3+gRand.GetF(2));
				aS->mDir*=1.5f;
				aS->mDir*=1.0f+gRand.GetF(.2f);
				aS->mDir.mY*=2.0f;
				aS->mDir.mX*=1.5f;
				if (gRand.Get(2)==1) aS->mDir*=.5f;
				aS->mPos=mPos+(aS->mDir*(8+gRand.GetF(8)));
				aS->mGravity=Point(0.0f,.5f);
				aS->mFade=2.0f;
				aS->mFadeSpeed=-(.1f+gRand.GetF(.05f));
				aS->mFadeSpeed*=.5f;
				gWorld->mAnimatorList_Top+=aS;

			}

			for (float aSpan=gWorld->mVisibleRect.mX;aSpan<gWorld->mVisibleRect.mX+gWorld->mVisibleRect.mWidth;aSpan+=20)
			{
				Anim_Sprite *aS=new Anim_Sprite(&mBoss->Element(3));
				aS->mScale=gRand.GetF(.75f)+.75f;
				aS->mDir=Point(0.0f,gRand.GetF(15)+5);
				aS->mPos=Point(aSpan+gRand.GetSpanF(10),gWorld->mVisibleRect.mY-(500+gRand.GetF(400)));
				aS->mGravity=Point(0.0f,.5f);
				aS->mFade=2.0f;
				aS->mFadeSpeed=-.02f;
				gWorld->mAnimatorList_Top+=aS;
			}

		}
	}
	else
	{
		mMouthFrame+=.2f;
		while (mMouthFrame>=7) mMouthFrame-=7;

		mBob+=10;
		for (int aCount=0;aCount<11;aCount++)
		{
			mBlinkCountdown[aCount]=_max(0,mBlinkCountdown[aCount]-1);
			if (!mBlinkCountdown[aCount]) 
			{
				if (gRand.Get(150)==1)
				{
					mBlinkCountdown[aCount]=10;
					mLookat[aCount]=gRand.GetF(360);
				}
			}
		}

		if (!mIsFalling)
		{
			mPos.mX+=mSpeed*mDir;
			{
	//			Point aTestPos=mPos+Point(mDir*70,0.0f);//(float)(gWorld->mGridSize));
				Point aTestPos=mPos+Point(mDir*70,(float)(gWorld->mGridSize));
				aTestPos.mY++;
				IPoint aTestGrid=gWorld->WorldToGrid(aTestPos);
				//if (!gWorld->IsBlocked(aTestGrid,false))
				//FIXBLUEBOSS
				if (!gWorld->IsBlockedDir(aTestGrid,mDir,false)) mDir*=-1;
			}
			{
				Point aTestPos=mPos+Point(mDir*70,0.0f);
				IPoint aTestGrid=gWorld->WorldToGrid(aTestPos);
				//if (!gWorld->IsBlocked(aTestGrid,false)) 
				//FIXBLUEBOSS
				if (gWorld->IsBlockedDir(aTestGrid,mDir,false)) mDir*=-1;
			}

			{
				Rect aRect=mCollide;
				aRect.mX+=mPos.mX;
				aRect.mY+=mPos.mY;

				Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
				aRobotRect.mY+=5;
				aRobotRect.mHeight-=10;
				aRobotRect.mX+=gWorld->mRobot->mPos.mX;
				aRobotRect.mY+=gWorld->mRobot->mPos.mY;

				if (aRobotRect.Intersects(aRect))
				{
					gWorld->mRobot->Die(false);
				}
			}

			Point aHold=mPos;
			mPos+=Point(0,20);
			Point aMove=Move(Point(0,1),5);
			if (aMove.Length()>=2)
			{
				mIsFalling=true;
				mFallSpeed=0;
				aHold.mY+=5;
			}
			mPos=aHold;

		}
		else
		{
			float aMod=1.0f;
			float aMax=20;
			IPoint aGridPos=gWorld->WorldToGrid(mPos);
			int aOnGrid=gWorld->GetGrid(aGridPos.mX,aGridPos.mY);
			if (aOnGrid)
			{
				if (aOnGrid==COSMIC_BLOCK) {aMod=.2f;aMax=1.5f;}
				if (ISSPIKE(aOnGrid)) 
				{
					mHP=0;
					Ouch();
				}
			}

			mFallSpeed=_min(mFallSpeed+(.5f*aMod),aMax);
			Point aMove=Move(Point(0,1),mFallSpeed);
			if (aMove.Length()<=0) 
			{
				mIsFalling=false;
				mFallSpeed=0;
			}
		}


		mSlimeWait-=(int)mSpeed;
		if (mSlimeWait<=0)
		{
			if (gRand.Get(2)==1)
			{
				Point aSlimePos=mPos+Point(mDir*-60,(float)(gWorld->mGridSize/2));
				Anim_Glob *aG=new Anim_Glob(aSlimePos,&mBoss->Element(4));
				gWorld->mAnimatorList_UnderPlayers+=aG;
				mSlimeWait=(int)(27.0f*aG->mScale);
			}
		}

		mFireCountdown-=(int)(mSpeed*(mSpeed));
		if (mFireCountdown<=0)
		{
			mFlashWhite=.5f;

			gSounds->mBossshoot.PlayPitched(1.0f+gRand.GetF(.1f),gWorld->GetVolume(mPos));
			mFireCountdown=165;
			EnergyBolt *aB=new EnergyBolt();
			aB->mPos=mPos;
			aB->mDir=Point(-mDir,-(float)fabs(mDir));
			aB->mPos+=aB->mDir*40;
			aB->mDir*=6;
			aB->mDir.mX*=.75f;
			//aB->mPos+=gMath.AngleToVector()*gRand.GetF(25);
			gWorld->mPlayerList+=aB;

			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mGlow);
			aS->mPos=aB->mPos;
			aS->mFade=1.0f;
			aS->mFadeSpeed=-.1f;
			aS->mScale=1.0f+gRand.GetF(.5f);
			gWorld->mAnimatorList_Top+=aS;

			aB=new EnergyBolt();
			aB->mPos=mPos;
			aB->mDir=Point(mDir,-(float)fabs(mDir));
			aB->mPos+=aB->mDir*40;
			aB->mDir*=6;
			aB->mDir.mX*=.75f;
			//aB->mPos+=gMath.AngleToVector()*gRand.GetF(25);
			gWorld->mPlayerList+=aB;

			aS=new Anim_Sprite(&gBundle_Play->mGlow);
			aS->mPos=aB->mPos;
			aS->mFade=1.0f;
			aS->mFadeSpeed=-.1f;
			aS->mScale=1.0f+gRand.GetF(.5f);
			gWorld->mAnimatorList_Top+=aS;


		}
	}

}


void Boss::Draw()
{
	if (!mBoss) FixBossGraphic();
	gG.Translate(0,-17);
	DrawGuy();

	if (mDying)
	{
		gG.SetColor(1.0f-((float)mDyingCountdown/40.0f));
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}
	else if (mFlashWhite)
	{
		gG.SetColor(mFlashWhite);
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}
	gG.Translate(0,17);
}

void Boss::DrawGuy()
{
	Matrix aMat;
	aMat.Scale(1.0f,1.0f+(gMath.Sin(mBob)*.01f),1.0f);
	aMat.Translate(mPos);
	gG.Translate(0,36);
	mBoss->Element(0).Draw(aMat);
	gG.Translate(0,-36);
	mBoss->Element(5+gBossFrame[(int)mMouthFrame]).Draw(aMat);

	Point aKeyScaler=Point(1.0f,1.0f+(gMath.Sin(mBob)*.025f));

	for (int aCount=0;aCount<11;aCount++)
	{
		if (mBlinkCountdown[aCount] && !mDying)
		{
			if (aCount<=2) mBoss->Element(2).Center(mPos+(gBundle_Play->mBoss.mKey[aCount]*aKeyScaler)+Point(0.0f,.5f));
			else mBoss->Element(1).Center(mPos+(gBundle_Play->mBoss.mKey[aCount]*aKeyScaler)+Point(0.0f,.5f));
		}
		else
		{
			if (aCount<=2) gBundle_Play->mBlueguy_Pupile.DrawScaled(mPos+(gBundle_Play->mBoss.mKey[aCount]*aKeyScaler)+Point(-1,-1)+(gMath.AngleToVector(mLookat[aCount])*2),.7f);
			else gBundle_Play->mBlueguy_Pupile.Center(mPos+(gBundle_Play->mBoss.mKey[aCount]*aKeyScaler)+Point(-1,-1)+(gMath.AngleToVector(mLookat[aCount])*4));
		}
	}

}

void Boss::Ouch(int theExtraDamage)
{
	mFlashWhite=1.0f;
	mHP-=theExtraDamage+1;
	mSpeed+=.1f;
	mSpeedDownCounter=1000;
	if (mHP<=0)
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}
}

EnergyBolt::EnergyBolt()
{
	mRotate=gRand.GetF(360);
	mPhase=gRand.GetF(2);
}

EnergyBolt::~EnergyBolt()
{
}

void EnergyBolt::Update()
{
	mRotate+=10;
	mPhase+=.25f;
	if (mPhase>=2) mPhase-=2;

	mPos+=mDir*.8f;
	mDir.mY+=(.25f)*.8f;

	IPoint aTest=gWorld->WorldToGrid(mPos);
	if (gWorld->IsBlockedDir(aTest,mDir.mX))
	//if (gWorld->IsBlocked(aTest))
	{
		Anim_ExplodePop *aEP=new Anim_ExplodePop(mPos);
		gWorld->mAnimatorList_Top+=aEP;
		Kill();
	}

	if (mDir.mY>10) if (!gWorld->mVisibleRect.ContainsPoint(mPos.mX,mPos.mY)) Kill();

	Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
	aRobotRect.mY+=5;
	aRobotRect.mHeight-=10;
	aRobotRect.mX-=5;
	aRobotRect.mWidth+=10;
	aRobotRect.mX+=gWorld->mRobot->mPos.mX;
	aRobotRect.mY+=gWorld->mRobot->mPos.mY;

	if (aRobotRect.ContainsPoint(mPos.mX,mPos.mY))
	{
		gWorld->mRobot->Die(false);
	}

}

void EnergyBolt::Draw()
{
	gG.SetColor(1,.75f+gRand.GetF(.25f),1);
	gBundle_Play->mGlow.DrawScaled(mPos,gRand.GetF(.25f)+.25f);
	gG.SetColor();
	gBundle_Play->mBoltglow.DrawRotatedScaled(mPos,mRotate,.75f);
}

Computer::Computer(void)
{
	mID=ID_COMPUTER;
	mHappy=false;
	mBlinkCountdown=0;
	mBob=gRand.GetF(360);
	mFlash=0;
	mHappyCountdown=0;
}

Computer::~Computer()
{
}

void Computer::Update()
{
	mBob++;
	if (mBob>=360) mBob-=360;
	mFlash=_max(0.0f,mFlash-.1f);
	mBlinkCountdown=_max(0,mBlinkCountdown-1);
	if (!mBlinkCountdown && !mHappyCountdown) 
	{
		if (gRand.Get(150)==1) mBlinkCountdown=10;
	}

	if (gWorld->mRobot->mHasHaxxor) if (!mHappy)
	{
		if (gWorld->mVisibleRect.ContainsPoint(mPos.mX,mPos.mY))
		{
			Point aVec=mPos-gWorld->mRobot->mPos;
			if (aVec.Length()<((float)gWorld->mGridSize*1.5f)) 
			{
				mHappy=true;
				gSounds->mHappycomputer.Play();
				mFlash=1.0f;
				mHappyCountdown=50;
			}
		}
	}

	if (mHappyCountdown>0)
	{
		if (--mHappyCountdown==0)
		{
			gWorld->OpenComputerDoor(gWorld->WorldToGrid(mPos));
		}
	}
}

void Computer::Draw()
{
	gBundle_Play->mComputer.Center(mPos+Point(5,-8));

	if (!mHappy)
	{
		if (mBlinkCountdown) gBundle_Play->mComputer_Sad_Blink.Center(mPos+Point(5.0f,-8+(gMath.Sin(mBob)*.5f)));
		else gBundle_Play->mComputer_Sad.Center(mPos+Point(5.0f,-8+(gMath.Sin(mBob)*.5f)));
	}
	else
	{
		if (mBlinkCountdown || ((mHappyCountdown/7)%2)==1) gBundle_Play->mComputer_Happy_Blink.Center(mPos+Point(5.0f,-11+(gMath.Sin(mBob)*.5f)));
		else gBundle_Play->mComputer_Happy.Center(mPos+Point(5.0f,-11+(gMath.Sin(mBob)*.5f)));
	}

	if (mFlash)
	{
		gG.RenderWhite(true);
		gG.SetColor(mFlash);
		gBundle_Play->mComputer.Center(mPos+Point(5,-8));
		gG.SetColor();
		gG.RenderWhite(false);
	}

}

RedBoss::RedBoss() : RedGuy()
{
	mID=ID_REDBOSS;
	mHP=40;
	mBirthing=0;
	mBirthingDelay=0;
	mBoss=NULL;

	mCollide=Rect(-48,-65,96,72+16);
	mFacing=0;
}

RedBoss::~RedBoss()
{
	gRedChildren=0;
}

void RedBoss::Update()
{
	if (mFacing==0) mFacing=(QuickRand((int)(mPos.mX*mPos.mY))%2)?1.0f:-1.0f;

	if (!mBoss) FixRedBossGraphic(mEnergyLevel);
	Player::Update();
	mFlashWhite=_max(0,mFlashWhite-.1f);

	if (++mChecker>5) mChecker=0;

	if (mDying)
	{
		mDyingCountdown--;
		if (mDyingCountdown<=0)
		{
			gApp.GetAchievement("DavidBotVsGoliathoid");

			Kill();
			gWorld->mTimerPause+=(10*50);
			gSounds->mLaserouch.PlayPitched(1.0f+gRand.GetSpanF(.05f));
			gSounds->mKillboss.Play();
			gWorld->mFlashWhite=1.0f;

			gWorld->mFlashWhite=1.0f;
			gWorld->Shake(20);

			gWorld->KilledBoss(gWorld->WorldToGrid(mPos));

			EnumList(Player,aP,gWorld->mPlayerList)
			{
				if (aP->mID==ID_REDGUY)
				{
					RedGuy *aRG=(RedGuy*)aP;
					if (aRG->mIsChild)
					{
						aRG->mDying=true;
						aRG->mDyingCountdown=30+gRand.Get(30);
					}
				}
			}

			for (float aCount=0;aCount<360;aCount+=10)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.025f;
				aS->mScale=1.0f+gRand.GetF(.5f);
				aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(8));
				aS->mPos=mPos+(aS->mDir*(3+gRand.GetF(5)));
				aS->mColor=Color(1,.25f,.25f);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.85f,.85f);
				gWorld->mAnimatorList_Top+=aS;
			}
			for (float aCount=0;aCount<360;aCount+=10)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.025f;
				aS->mScale=1.0f+gRand.GetF(.5f);
				aS->mDir=gMath.AngleToVector(aMyAngle)*(10+gRand.GetSpanF(10));
				aS->mPos=mPos+(aS->mDir*(3+gRand.GetF(5)));
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.87f,.87f);
				gWorld->mAnimatorList_Top+=aS;
			}

			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mJumpburst);
			aS->mPos=mPos;
			aS->mFadeSpeed=-.1f;
			aS->mRotation=0;
			aS->mScale=3.0f;
			aS->mColor=Color(.5f,1,.5f);
			aS->mRotationDir=gRand.NegateF(5);
			gWorld->mAnimatorList_UnderWorld+=aS;

			for (int aAngle=-90;aAngle<90;aAngle+=5)
			{
				Anim_Sprite *aS=new Anim_Sprite(&mBoss->Element(6));
				aS->mScale=gRand.GetF(.25f)+.75f;
				aS->mDir=gMath.AngleToVector(aAngle+gRand.GetSpanF(10))*(3+gRand.GetF(2));
				aS->mDir*=1.5f;
				aS->mDir*=1.0f+gRand.GetF(.2f);
				aS->mDir.mY*=2.0f;
				aS->mDir.mX*=1.5f;
				if (gRand.Get(2)==1) aS->mDir*=.5f;
				aS->mPos=mPos+(aS->mDir*(8+gRand.GetF(8)));
				aS->mGravity=Point(0.0f,.5f);
				aS->mFade=2.0f;
				aS->mFadeSpeed=-(.1f+gRand.GetF(.05f));
				aS->mFadeSpeed*=.5f;
				gWorld->mAnimatorList_Top+=aS;

			}

			for (float aSpan=gWorld->mVisibleRect.mX;aSpan<gWorld->mVisibleRect.mX+gWorld->mVisibleRect.mWidth;aSpan+=20)
			{
				Anim_Sprite *aS=new Anim_Sprite(&mBoss->Element(6));
				aS->mScale=gRand.GetF(.75f)+.75f;
				aS->mDir=Point(0.0f,gRand.GetF(15)+5);
				aS->mPos=Point(aSpan+gRand.GetSpanF(10),gWorld->mVisibleRect.mY-(500+gRand.GetF(400)));
				aS->mGravity=Point(0.0f,.5f);
				aS->mFade=2.0f;
				aS->mFadeSpeed=-.02f;
				gWorld->mAnimatorList_Top+=aS;
			}
		}
	}
	else if (mBirthing)
	{
		mBirthing=_max(0,mBirthing-1);
		if (mBirthing==0)
		{
			mBirthingDelay=100;
			if (mHP<35) mBirthingDelay=40;
			if (mHP<20) mBirthingDelay=15;

			mFlashWhite=1.0f;

			//
			// Pop out a red monster
			//
			gSounds->mRedpop.PlayPitched(1.0f+gRand.GetF(.25f),gWorld->GetVolume(mPos));


			RedBaby *aB=new RedBaby(mPos);
			aB->mBoss=mBoss;
			aB->mEnergyLevel=mEnergyLevel;
			gWorld->mPlayerList.Insert(aB);

			if (mHP<25)
			{
				RedBaby *aB2=new RedBaby(mPos);
				aB->mBoss=mBoss;
				aB->mEnergyLevel=mEnergyLevel;
				aB2->mDir.mX*=-1;
				gWorld->mPlayerList.Insert(aB2);

				//while (fabs(aB2->mDir.mX-aB->mDir.mX)<3) aB2->mDir.mX*=.75f;
			}

		}
	}
	else
	{
		if (mIsFalling)
		{
			float aMod=1.0f;
			float aMax=20;
			IPoint aGridPos=gWorld->WorldToGrid(mPos);
			int aOnGrid=gWorld->GetGrid(aGridPos.mX,aGridPos.mY);
			if (aOnGrid)
			{
				if (aOnGrid==COSMIC_BLOCK) {aMod=.2f;aMax=1.5f;}
				if (ISSPIKE(aOnGrid)) 
				{
					mHP=0;
					Ouch();
				}
				}

			mFallSpeed=_min(mFallSpeed+(.5f*aMod),aMax);
			Point aMove=Move(Point(0,1),mFallSpeed);
			if (aMove.Length()<=0) 
			{
				IPoint aGP=gWorld->WorldToGrid(mPos);
				mPos.mY=gWorld->GridToWorld(aGP).mY-2;
				mIsFalling=false;
			}

			mChomp+=mChompSpeed;
			if (mChomp>=360) mChomp-=360;

			mStep+=(15)*1.5f;
			if (mStep>=3600) mStep-=3600;
		}
		else
		{
			mBirthingDelay=_max(0,mBirthingDelay-1);

			mChomp+=mChompSpeed;
			if (mChomp>=360) mChomp-=360;

			mStep+=(15)*1.0f;
			if (mStep>=3600) mStep-=3600;

			Point aTest=Point(mPos.mX+(-mFacing*(gWorld->mGridSize+20)),mPos.mY);
			if (gWorld->IsBlockedDir(gWorld->WorldToGrid(aTest),-mFacing)) mFacing*=-1;	
			//if (gWorld->IsBlocked(gWorld->WorldToGrid(aTest))) mFacing*=-1;	
			else 
			{
				aTest.mY+=(gWorld->mGridSize/2)+3;
				if (!gWorld->IsBlocked(gWorld->WorldToGrid(aTest),false)) mFacing*=-1;
				else 
				{
					aTest.mY-=(gWorld->mGridSize*2)+3;
					if (gWorld->IsBlockedDir(gWorld->WorldToGrid(aTest),-mFacing)) mFacing*=-1;
					//if (gWorld->IsBlocked(gWorld->WorldToGrid(aTest))) mFacing*=-1;
				}
			}

			mPos.mX+=(-mFacing*.75f)*1.5f;

			if (mChecker==0)
			{
				Point aHold=mPos;
				Point aMove=Move(Point(0,1),5);
				if (aMove.Length()>=2)
				{
					mIsFalling=true;
					mFallSpeed=0;
				}
				mPos=aHold;
			}

		}
	}

	if (!mDying)
	{
		//
		// Did we hit Robot?
		//

		{
			Rect aRect=mCollide;
			//			aRect.mX-=5;
			//			aRect.mWidth+=10;
			aRect.mX+=mPos.mX;
			aRect.mY+=mPos.mY;

			Rect aRobotRect=gWorld->mRobot->mCollide;//.Expand(10);
			//			aRobotRect.mX-=5;
			//			aRobotRect.mWidth+=10;
			aRobotRect.mY+=5;
			aRobotRect.mHeight-=10;
			aRobotRect.mX+=gWorld->mRobot->mPos.mX;
			aRobotRect.mY+=gWorld->mRobot->mPos.mY;

			if (aRobotRect.Intersects(aRect))
			{
				gWorld->mRobot->Die(false);
			}
		}
	}
}

void RedBoss::DrawGuy()
{
	if (!mBoss) FixBossGraphic();

#define MOVEREDBOSS 18
	gG.Translate(0,MOVEREDBOSS);
	Point aShake;
	if (mDying || mBirthing)
	{
		aShake=gMath.AngleToVector()*gRand.GetF(6);
		gG.Translate(aShake);
	}
	Matrix aMat;
	aMat.Scale(mFacing,1,1);
	aMat.Translate(mPos);

	Point aLeg;
	aLeg=gMath.AngleToVector(-(((mStep)+180))*mFacing)*4;
	aLeg+=Point(-40*mFacing,0.0f);
	if (aLeg.mY>0) aLeg.mY=0;
	gG.Translate(aLeg);
	mBoss->Element(4).Draw(aMat);
	gG.Translate(-aLeg);

	float aBodyBob=gMath.Sin(mPos.mX*20)*4;
	gG.Translate(0.0f,aBodyBob);
	mBoss->Element(3).Draw(aMat);
	gG.Translate(0.0f,-aBodyBob);


	aLeg=gMath.AngleToVector(-(((mStep)+180))*mFacing)*4;
	aLeg+=Point(25*mFacing,0.0f);
	if (aLeg.mY>0) aLeg.mY=0;
	gG.Translate(aLeg);
	mBoss->Element(4).Draw(aMat);
	gG.Translate(-aLeg);


	aLeg=gMath.AngleToVector(-(mStep)*mFacing)*4;
	if (aLeg.mY>0) aLeg.mY=0;
	gG.Translate(aLeg);
	mBoss->Element(4).Draw(aMat);
	gG.Translate(-aLeg);



	gG.Translate(0.0f,aBodyBob);
	float aChomp=-(float)fabs(gMath.Sin(mChomp)*5);
	gG.Translate(0.0f,aChomp);
	mBoss->Element(5).Draw(aMat);
	if (mBirthing) mBoss->Element(2).Draw(aMat);

	int aBob=gAppPtr->AppTime()%20;
	if (aBob>=0 && aBob<=5) gG.Translate(0,3);
	mBoss->Element(0).Draw(aMat);
	if (aBob>=0 && aBob<=5) gG.Translate(0,-3);
	if (aBob>=15 && aBob<=20) gG.Translate(0,3);
	mBoss->Element(1).Draw(aMat);
	if (aBob>=15 && aBob<=20) gG.Translate(0,-3);

	gG.Translate(0.0f,-aBodyBob);
	gG.Translate(0.0f,-aChomp);

	if (mDying || mBirthing) gG.Translate(-aShake);
	gG.Translate(0,-MOVEREDBOSS);
}

void RedBoss::Ouch(int theExtraDamage)
{
	mFlashWhite=1.0f;
	mHP-=theExtraDamage+1;
	if (mHP>0)
	{
		if (gWorld->mRobot->mPos.mX<mPos.mX) mFacing=1;
		else mFacing=-1;

		if (mBirthingDelay<=0 && mBirthing==0 && gRedChildren<25) 
		{
			mBirthing=25;
			if (mHP<30) mBirthing=18;
			if (mHP<15) mBirthing=10;
		}
	}
	else
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}
}

RedBaby::RedBaby(Point thePos)
{
	mBoss=NULL;
	mID=ID_REDBABY;
	mHP=3;

	float aDir=1;
	if (gWorld->mRobot) if (gWorld->mRobot->mPos.mX<thePos.mX) aDir=-1;
	if (gRand.Get(6)==2) aDir*=-1;

	mDir=Point((5+gRand.GetF(3))*aDir,-6.0f);
	mGravity=0;
	mFlash=1.0f;
	mRotate=gRand.GetSpanF(45);
	mPos=thePos+(mDir*5);
	mPos.mY-=30;
	mDir*=.75f;
	mDir.mX*=.75f;
	mIsEnemy=true;
	mBounce=0;

	mCollide=Rect(-10,-10,20,20);

	Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mExplode);
	aS->mPos=mPos;
	aS->mFade=1.0f;
	aS->mScale=1.5f;
	aS->mRotation=gRand.GetF(360);
	aS->mFadeSpeed=-.1f;
	gWorld->mAnimatorList_UnderPlayers+=aS;
}

RedBaby::~RedBaby()
{
}

void RedBaby::Update()
{
	if (!mBoss) mBoss=FixRedBossGraphic(mEnergyLevel);
	mFlash=_max(0,mFlash-.1f);

	mPos+=mDir;
	mDir.mY+=mGravity;
	mRotate+=gMath.Sign(mDir.mX);

	IPoint aGPos=gWorld->WorldToGrid(mPos);
	mGravity+=.05f;
	if (mBounce) 
	{
		mGravity+=.025f;
		mRotate+=gMath.Sign(mDir.mX)*10;
	}

	if (mGravity>=0)
	{

		Point aTestPos=mPos+Point(0,15);
		IPoint aCheck=gWorld->WorldToGrid(aTestPos);

		if (gWorld->GetGrid(aCheck.mX,aCheck.mY)==2)
		{
			//
			// It's goo!  Kill im!
			//
			Kill();

			Anim_Sprite *aS=new Anim_Sprite(&mBoss->Element(7));
			aS->mRotation=mRotate;
			aS->mPos=mPos;
			aS->mDir=Point(0,1);
			aS->mTimer=40;
			gWorld->mAnimatorList_UnderWorld+=aS;
			return;

		}
		if (gWorld->IsBlockedDir(aCheck,mDir.mX))
		//if (gWorld->IsBlocked(aCheck))
		{
			if (mBounce>=2)
			{
				RedGuy *aRG=new RedGuy();
				aRG->mPos=mPos;
				aRG->mEnergyLevel=mEnergyLevel;
				aRG->mFlashWhite=1.0f;
				aRG->FixPosition();
				aRG->mFacing=gMath.Sign(-mDir.mX);
				aRG->mHP=_max(1,mHP);
				aRG->mIsChild=true;
				gRedChildren++;
				gWorld->mPlayerList+=aRG;

				gSounds->mRedpop_Emerge.PlayPitched(1.0f+gRand.GetSpanF(.1f),gWorld->GetVolume(mPos));

				for (int aCount=0;aCount<2;aCount++)
				{
					Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mJumpburst);
					aS->mPos=aRG->mPos;
					aS->mFade=1.0f;
					aS->mScale=1.5f;
					aS->mRotationDir=gRand.NegateF(10);
					aS->mRotation=gRand.GetF(360);
					aS->mFadeSpeed=-.1f;
					gWorld->mAnimatorList_UnderPlayers+=aS;
				}

				
				Kill();
			}
			else
			{
				for (;;)
				{
					Point aTestPos=mPos+Point(0,15);
					IPoint aCheck=gWorld->WorldToGrid(aTestPos);
					//if (!gWorld->IsBlocked(aCheck)) break;
					if (!gWorld->IsBlockedDir(aCheck,mDir.mX)) break;

					mPos.mY-=2;
				}
				
				mDir.mX*=.75f;
				mDir.mY=-4;
				if (mBounce==1) 
				{
					mDir.mY=-2;
					mDir.mX*=.75f;
				}
				mGravity=0;
				mBounce++;
			}
		}
	}

	Rect aRect=mCollide;
	aRect.mX+=mPos.mX;
	aRect.mY+=mPos.mY;

	Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
	aRobotRect.mY+=5;
	aRobotRect.mHeight-=10;
	aRobotRect.mX+=gWorld->mRobot->mPos.mX;
	aRobotRect.mY+=gWorld->mRobot->mPos.mY;

	if (aRobotRect.Intersects(aRect))
	{
		gWorld->mRobot->Die(false);
	}

	if (gWorld->CollideDir(Rect(mPos.mX+mCollide.mX+(gMath.Sign(mDir.mX)*10),mPos.mY+mCollide.mY,mCollide.mWidth,mCollide.mHeight),mDir.mX))
	//if (gWorld->Collide(Rect(mPos.mX+mCollide.mX+(gMath.Sign(mDir.mX)*10),mPos.mY+mCollide.mY,mCollide.mWidth,mCollide.mHeight)))
	{
		mDir.mX*=-1;
	}






}

void RedBaby::Draw()
{
	if (!mBoss) mBoss=FixRedBossGraphic(mEnergyLevel);
	mBoss->Element(7).DrawRotated(mPos,mRotate);
	if (mFlash) 
	{
		gG.RenderWhite(true);
		gG.SetColor(mFlash);
		mBoss->Element(7).DrawRotated(mPos,mRotate);
		gG.SetColor();
		gG.RenderWhite(false);
	}
}

void RedBaby::Ouch(int theExtraDamage)
{
	mFlash=1.0f;
	mHP-=theExtraDamage+1;
}

Telematic::Telematic()
{
	mID=ID_TELEMATIC;
	mCrystals=0;
	mCrystalsNeeded=0;
	mActivated=false;

	mEyeOffset=0;
	mEyeDir=gRand.NegateF(1);
	mEyeWait=0;
	mBlinkCountdown=0;
	mBlinkWait=100;
	mCycle=0;
	mFlash=0;

	for (int aCount=0;aCount<6;aCount++) {mLizer[aCount]=0;mLizerWant[aCount]=0;}
}

Telematic::~Telematic()
{
}

void Telematic::Update()
{
	mFlash=_max(0,mFlash-.1f);
	if (mActivated)
	{
		mBlinkCountdown=_max(0,mBlinkCountdown-1);
		if (--mRandCountdown<=0)
		{
			mRandSeed=gRand.Get(10000);
			mRandCountdown=gRand.Get(5)+5;
		}
		mEyeWait=_max(mEyeWait-1,0);
#define EYEMAX 15
		if (mEyeWait<=0)
		{
			mEyeOffset+=mEyeDir*.25f;//*.25f;
			if (mEyeOffset>EYEMAX)
			{
				mEyeWait=15+gRand.Get(50);
				mEyeOffset=EYEMAX;
				mEyeDir=-1;
			}
			if (mEyeOffset<-EYEMAX)
			{
				mEyeWait=15+gRand.Get(50);
				mEyeOffset=-EYEMAX;
				mEyeDir=1;
			}
		}

		if (--mBlinkWait<=0) 
		{
			mBlinkCountdown=gRand.Get(10)+10;
			mBlinkWait=50+gRand.Get(100);
			if (gRand.Get(10)==3) mBlinkWait=5+gRand.Get(10);
		}

		for (int aCount=0;aCount<6;aCount++)
		{
			if (mLizer[aCount]>mLizerWant[aCount]) mLizer[aCount]-=.1f;
			if (mLizer[aCount]<mLizerWant[aCount]) mLizer[aCount]+=.1f;

			if (gRand.Get(25)==3) mLizerWant[aCount]=(float)gRand.Get(4);
		}
	}

	mCycle+=.05f;
}

void Telematic::Draw()
{
/*
	int aBodyBob=0;
	if (mActivated)
	{
		aBodyBob=((gAppPtr->mAppTime/5)%2)*1;
	}
*/
	gBundle_Play->mTelematic.Center(mPos);
//	gBundle_Play->mTelematicbody.Center(mPos+Point(0,42+aBodyBob));

	if (!mActivated)
	{
		gBundle_Play->mTelematic_Off.Center(mPos);
	}
	else
	{
		if (!mBlinkCountdown) gBundle_Play->mTelematic_On.Center(mPos+Point(mEyeOffset,-1.0f));
		else gBundle_Play->mTelematic_Blink.Center(mPos+Point(mEyeOffset,-1.0f));

		Point aFidge;
		for (int aCount=0;aCount<6;aCount++)
		{
			Point aDraw=Point(mPos+Point(-20,15))+aFidge;
			for (int aMizer=0;aMizer<mLizer[aCount];aMizer++)
			{
				gBundle_Play->mTelematiclight.Center(aDraw);
				aDraw.mY-=4.5f;
			}
			aFidge.mX+=7;
			if (aCount==2) aFidge.mX+=4;
		}

		int aCycle=(int)mCycle;

		#define COLORIZE(cycle) {Color aColor;aColor.Primary((cycle)%4);aColor.Pastel(.27f);gG.SetColor(aColor);}

		COLORIZE(aCycle);
		gBundle_Play->mComputron_Glint.Center(mPos+Point(-3,-24));
		gG.SetColor(gRand.GetF(.5f)+.5f);gBundle_Play->mComputron_Glintwhite.Center(mPos+Point(-3,-24));
		COLORIZE(aCycle+1);
		gBundle_Play->mComputron_Glint.Center(mPos+Point(-3-7,-24));
		gG.SetColor(gRand.GetF(.5f)+.5f);gBundle_Play->mComputron_Glintwhite.Center(mPos+Point(-3-7,-24));
		COLORIZE(aCycle+2);
		gBundle_Play->mComputron_Glint.Center(mPos+Point(-3-7-7,-24));
		gG.SetColor(gRand.GetF(.5f)+.5f);gBundle_Play->mComputron_Glintwhite.Center(mPos+Point(-3-7-7,-24));
		COLORIZE(aCycle+3);
		gBundle_Play->mComputron_Glint.Center(mPos+Point(-3+14,-24));
		gG.SetColor(gRand.GetF(.5f)+.5f);gBundle_Play->mComputron_Glintwhite.Center(mPos+Point(-3+14,-24));
		COLORIZE(aCycle+4);
		gBundle_Play->mComputron_Glint.Center(mPos+Point(-3+14+7,-24));
		gG.SetColor(gRand.GetF(.5f)+.5f);gBundle_Play->mComputron_Glintwhite.Center(mPos+Point(-3+14+7,-24));
		COLORIZE(aCycle+5);
		gG.SetColor();


		if (mFlash)
		{
			gG.RenderWhite(true);
			gG.SetColor(mFlash);
			gBundle_Play->mTelematic.Center(mPos);
			//gBundle_Play->mTelematicbody.Center(mPos+Point(0,42+aBodyBob));
			gG.SetColor();
			gG.RenderWhite(false);
		}

		//26,-4
	}
}

void Telematic::SortTelepads()
{
	if (mTelepadList.GetCount()==0) return;

	struct Sorter
	{
		float mAngle;
		Telepad *mTP;
	} aSorter;

	List aSortList;

	EnumList(Telepad,aTP,mTelepadList)
	{
		Sorter *aS=new Sorter;
		aS->mTP=aTP;
		aS->mAngle=gMath.VectorToAngle(aTP->mPos-mPos);

		//
		// If counterclockwise, multiply angle by -1
		//
		if (!mClockwise) aS->mAngle*=-1;
		aSortList+=aS;
	}

	mTelepadList.Clear();

	Utils::SortListByZ(aSortList,&aSorter,&aSorter.mAngle);
	EnumList(Sorter,aS,aSortList) mTelepadList+=aS->mTP;
	Sorter *aS=(Sorter*)aSortList[0];
	mTelepadList+=aS->mTP;
	_FreeList(Sorter,aSortList);
}

void Telematic::Teleport(Player *theFrom)
{
	gSounds->mTelesizzle.Stop();
	gWorld->mFlashWhite=1.5f;

	bool aGoNow=false;
	EnumList(Telepad,aTP,mTelepadList)
	{
		if (aGoNow)
		{
			gWorld->mRobot->mPos=aTP->mPos;
			gWorld->mRobot->FixPosition(false);
		
			aTP->mNoGrab=10;
			gSounds->mTeleport.Play();
			gWorld->Shake(20);

			IPoint aIP=gWorld->WorldToGrid(theFrom->mPos);
			mLastTeleportTo=aIP;

			break;
		}
		if (theFrom==aTP) aGoNow=true;
	}

/*
	Telepad *aBest=NULL;
	float aClosest=9999999;

	for (int aCount=0;aCount<2;aCount++)
	{
		EnumList(Player,aP,gWorld->mPlayerList.mList)
		{
			if (aP==theFrom) continue;
			if (aP->mID==ID_TELEPAD)
			{
				IPoint aIP=gWorld->WorldToGrid(aP->mPos);
				if (aIP.mX==mLastTeleportTo.mX && aIP.mY==mLastTeleportTo.mY) continue;

				float aDist=gMath.Distance(aP->mPos,theFrom->mPos);
				if (aDist<aClosest)
				{
					aClosest=aDist;
					aBest=(Telepad*)aP;
				}
			}
		}
		if (aBest) break;
		mLastTeleportTo=IPoint(-1,-1);
	}
	if (aBest)
	{
		gWorld->mRobot->mPos=aBest->mPos;
		gWorld->mRobot->FixPosition();
		aBest->mNoGrab=10;
		gSounds->mTeleport.Play();
		gWorld->Shake(20);

		IPoint aIP=gWorld->WorldToGrid(theFrom->mPos);
		mLastTeleportTo=aIP;
	
	}
*/
}

void Telematic::AddCrystal(int theCount)
{
	mCrystals+=theCount;
	if (mCrystals>=mCrystalsNeeded) 
	{
		mActivated=true;
		for (int aCount=0;aCount<6;aCount++) {mLizer[aCount]=0;mLizerWant[aCount]=(float)gRand.Get(2)+2;}
		if (mCrystalsNeeded>0) 
		{
			gSounds->mTelematicon.Play();
			mFlash=1.0f;
		}

		IPoint aGPos=gWorld->WorldToGrid(mPos);

		EnumList(Player,aP,gWorld->mPlayerList)
		{
			if (aP->mID==ID_TELEPAD)
			{
				Telepad *aPP=(Telepad*)aP;
				if (aPP->mMyTelematic.mX==aGPos.mX && aPP->mMyTelematic.mY==aGPos.mY)
				{
					aPP->mActivated=true;
				}
			}
		}
	}
}


Telematic_Slot::Telematic_Slot()
{
	mID=ID_TELEMATIC_SLOT;
	mActivated=false;
	mGlow=gRand.GetF(360);
	mProximity=false;
}

Telematic_Slot::~Telematic_Slot()
{
}

void Telematic_Slot::Update()
{
	if (mActivated)
	{
		mGlow+=10;
		if (mGlow>=360) mGlow-=360;
	}
	else
	{
		bool aOldProximity=mProximity;
		mProximity=false;
		if (gWorld->mVisibleRect.ContainsPoint(mPos.mX,mPos.mY))
		{
			Point aVec=mPos-gWorld->mRobot->mPos;
			if (aVec.Length()<((float)gWorld->mGridSize*1.25f)) 
			{
				mProximity=true;
				if (gWorld->mRobot->mCrystals)
				{
					mActivated=true;
					gWorld->mRobot->mCrystals--;
					gSounds->mCrystal.Play();

					Telematic *aTelematic=gWorld->GetTelematic(mMyTelematic);
					if (aTelematic)
					{
						aTelematic->AddCrystal(1);
					}
				}
				else
				{
					if (!aOldProximity)
					{
						gWorld->AddMessage("NEEDS A CRYSTAL TO TELEPORT!","DISABLED TELEMATIC");
					}
				}
			}
		}
	}
}

void Telematic_Slot::Draw()
{
	gBundle_Play->mTelematic_Slot.Center(mPos);

	if (mActivated)
	{
		gBundle_Tiles->mBlock[28].Center(mPos);
		gG.RenderWhite(true);
		gG.SetColor(.5f+(gMath.Sin(mGlow)*.25f));
		gBundle_Tiles->mBlock[28].Center(mPos);
		gG.SetColor();
		gG.RenderWhite(false);
	}
	
}

Telepad::Telepad()
{
	mID=ID_TELEPAD;
	mFacing=0;
	mActivated=false;

	mGlow=0;
	mPulse=gRand.GetF(360);
	mGrab=0;
	mNoGrab=0;
}

Telepad::~Telepad()
{
}

void Telepad::Update()
{
	if (mActivated)
	{
		mGlow=_min(1.0f,mGlow+=.05f);
		mPulse+=25;
		if (mPulse>=360) mPulse-=360;

		bool aGrabbing=false;
		if (gWorld->mVisibleRect.ContainsPoint(mPos.mX,mPos.mY))
		{
			Point aRPos=gWorld->mRobot->mPos;

			bool aGotcha=false;

			if (mFacing<=1) {if (aRPos.mX>mPos.mX-10 && aRPos.mX<mPos.mX+10) if (aRPos.mY>mPos.mY-10 && aRPos.mY<mPos.mY+10) aGotcha=true;}
			else {if (aRPos.mY>mPos.mY-10 && aRPos.mY<mPos.mY+10) if (aRPos.mX>mPos.mX-20 && aRPos.mX<mPos.mX+20) aGotcha=true;}

			if (aGotcha)
			{
				aGrabbing=true;
				if (!mNoGrab)
				{
					if (mGrab==0) gSounds->mTelesizzle.Play();
					//mGrab+=.02f;
					mGrab+=.03f;
					if (mGrab>=1.0f)
					{
						//
						// Teleport...
						//
						Telematic *aT=gWorld->GetTelematic(mMyTelematic);
						if (aT) aT->Teleport(this);
						mGrab=0;
					}
				}
			}
		}
		if (!aGrabbing) 
		{
			if (mGrab) gSounds->mTelesizzle.Stop();
			mGrab=0;
			mNoGrab=_max(0,mNoGrab-1);
		}
	}
}


void Telepad::Draw()
{
	if (mGrab)
	{
		gG.RenderWhite(true);
		gG.ClipColor(Color(1,1,1,mGrab));
		gWorld->mRobot->Draw();
		gG.ClipColor();
		gG.RenderWhite(false);

		float aRotate=0;
		if (mFacing==1) aRotate=180;
		if (mFacing==2) aRotate=-90;
		if (mFacing==3) aRotate=90;
		gG.SetColor(mGrab);
		gBundle_Play->mTelelight.DrawRotated(mPos,aRotate);
		gG.SetColor();
	}
	gBundle_Play->mTelepad_Off[mFacing].Center(mPos);
	if (mActivated)
	{
		float aMod=.75f+(gMath.Sin(mPulse)*.25f);
		if (mGrab) aMod=1.0f;
		gG.SetColor(mGlow*(aMod));
		gBundle_Play->mTelepad_On[mFacing].Center(mPos);
		gG.SetColor();
	}
}

DripBoss::DripBoss()
{
	mID=ID_DRIPBOSS;

	mBoss=NULL;
	mBadCounter=0;
	mHP=40;
	mIsEnemy=true;
	mCollide=Rect(-38,-36,74,74).Expand(ROBOTEXPAND);
	mDying=false;
	mDyingCountdown=40;
	mFlashWhite=0.0f;

	mBlinkCountdown=0;
	mBlinkWait=0;

	for (int aCount=0;aCount<7;aCount++)
	{
		mWiggle[aCount].mX=gRand.GetF(360);
		mWiggle[aCount].mY=gRand.NegateF(gRand.GetF(5)+5);
	}

	mDir=Point(0,0);

	mDirMod=Point(1,1);
	if (gRand.GetBool()) mDirMod.mX*=.9f;
	else mDirMod.mY*=.9f;

	mSpeed=1.0f;
	mSpeedDecay=.1f;

	mDripCountdownMax=100;
	mDripCountdown=mDripCountdownMax;

}

DripBoss::~DripBoss()
{
}

void DripBoss::Update()
{
	if (mDir.mX==0)
	{
		switch (QuickRand((int)(mPos.mX*mPos.mY))%4)
		{
		case 0:mDir=Point(-1,-1);break;
		case 1:mDir=Point(-1,1);break;
		case 2:mDir=Point(1,-1);break;
		case 3:mDir=Point(1,1);break;
		}
	}
	if (!mBoss) FixBossGraphic();

	Player::Update();
	mFlashWhite=_max(0,mFlashWhite-.1f);

	if (mDying)
	{
		mDyingCountdown--;
		if (mDyingCountdown<=0)
		{
			// Explode
			gApp.GetAchievement("DavidBotVsGoliathoid");

			Kill();
			gSounds->mLaserouch.PlayPitched(1.0f+gRand.GetSpanF(.05f));
			gSounds->mKillboss.Play();
			gWorld->mFlashWhite=1.0f;

			gWorld->mFlashWhite=1.0f;
			gWorld->Shake(20);

			gWorld->KilledBoss(gWorld->WorldToGrid(mPos));

			for (float aCount=0;aCount<360;aCount+=10)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.025f;
				aS->mScale=1.0f+gRand.GetF(.5f);
				aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(8));
				aS->mPos=mPos+(aS->mDir*(3+gRand.GetF(5)));
				aS->mColor=Color(216/255.0f,147/255.0f,40/255.0f);
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.85f,.85f);
				gWorld->mAnimatorList_Top+=aS;
			}
			for (float aCount=0;aCount<360;aCount+=10)
			{
				float aMyAngle=aCount+gRand.GetSpanF(4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.025f;
				aS->mScale=1.0f+gRand.GetF(.5f);
				aS->mDir=gMath.AngleToVector(aMyAngle)*(10+gRand.GetSpanF(10));
				aS->mPos=mPos+(aS->mDir*(3+gRand.GetF(5)));
				//aS->mDir*=1.0f+gRand.GetF(1.5f);
				aS->mDirMult=Point(.87f,.87f);
				gWorld->mAnimatorList_Top+=aS;
			}

			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mJumpburst);
			aS->mPos=mPos;
			aS->mFadeSpeed=-.1f;
			aS->mRotation=0;
			aS->mScale=3.0f;
			aS->mColor=Color(.5f,1,.5f);
			aS->mRotationDir=gRand.NegateF(5);
			gWorld->mAnimatorList_UnderWorld+=aS;

			for (int aAngle=-90;aAngle<90;aAngle+=5)
			{
				Anim_Sprite *aS=new Anim_Sprite(&mBoss->Element(2));
				aS->mScale=gRand.GetF(.25f)+.75f;
				aS->mDir=gMath.AngleToVector(aAngle+gRand.GetSpanF(10))*(3+gRand.GetF(2));
				aS->mDir*=1.5f;
				aS->mDir*=1.0f+gRand.GetF(.2f);
				aS->mDir.mY*=2.0f;
				aS->mDir.mX*=1.5f;
				if (gRand.Get(2)==1) aS->mDir*=.5f;
				aS->mPos=mPos+(aS->mDir*(8+gRand.GetF(8)));
				aS->mGravity=Point(0.0f,.5f);
				aS->mFade=2.0f;
				aS->mFadeSpeed=-(.1f+gRand.GetF(.05f));
				aS->mFadeSpeed*=.5f;
				gWorld->mAnimatorList_Top+=aS;
			}

			for (float aSpan=gWorld->mVisibleRect.mX;aSpan<gWorld->mVisibleRect.mX+gWorld->mVisibleRect.mWidth;aSpan+=20)
			{
				Anim_Sprite *aS=new Anim_Sprite(&mBoss->Element(2));
				aS->mScale=gRand.GetF(.75f)+.75f;
				aS->mDir=Point(0.0f,gRand.GetF(15)+5);
				aS->mPos=Point(aSpan+gRand.GetSpanF(10),gWorld->mVisibleRect.mY-(500+gRand.GetF(400)));
				aS->mGravity=Point(0.0f,.5f);
				aS->mFade=2.0f;
				aS->mFadeSpeed=-.02f;
				gWorld->mAnimatorList_Top+=aS;
			}

		}
	}
	else
	{
		mBlinkCountdown=_max(0,mBlinkCountdown-1);
		if (mBlinkCountdown<=0) if (--mBlinkWait<=0) 
		{
			mBlinkCountdown=gRand.Get(10)+5;
			mBlinkWait=50+gRand.Get(100);
			if (gRand.Get(10)==3) mBlinkWait=5+gRand.Get(10);
		}

		for (int aCount=0;aCount<7;aCount++)
		{
			mWiggle[aCount].mX+=mWiggle[aCount].mY+gRand.GetSpanF(3);
			if (mWiggle[aCount].mX>=360) mWiggle[aCount]-=360;
		}

		//
		// Move him... first see if we need to bounce...
		//
		Point aHoldPos=mPos;
		for (int aCount=0;aCount<25;aCount++)
		{
			Point aNewDir=mDir*mDirMod*mSpeed;
			{
				Rect aRect=mCollide;
				aRect.mX+=mPos.mX;
				aRect.mY+=mPos.mY;
				aRect.mX+=mDir.mX*mSpeed*mDirMod.mX;
				aRect.mY+=mDir.mY*mSpeed*mDirMod.mY;

				mDirMod=Point(1,1);
				if (gRand.GetBool()) mDirMod.mX*=.9f;
				else mDirMod.mY*=.9f;

				bool aDoBad=false;


				if (gWorld->CollideDir(aRect.Translate(gMath.Sign(mDir.mX)*40,gMath.Sign(mDir.mY)*40),mDir.mX))
	//			if (gWorld->CollideDir(aRect.Expand(mSpeed+1).Translate(gMath.Sign(mDir.mX)*40,gMath.Sign(mDir.mY)*40),mDir.mX))
					//if (gWorld->Collide(aRect.Expand(mSpeed+1)))
				{
					//mPos-=mDir*(mSpeed*1.5f);
					aRect.mX-=mDir.mX*(mSpeed*1.5f)*mDirMod.mX;
					aRect.mY-=mDir.mY*(mSpeed*1.5f)*mDirMod.mY;

					if (aNewDir.mY<0)
					{
						aRect.mY-=mSpeed+5;
						//if (gWorld->CollideDir(aRect,mDir.mX)) {mDir.mY=fabs(mDir.mY);mSpeed=.1f;aNewDir.mY=0;}
						if (gWorld->Collide(aRect)) {mDir.mY=(float)fabs(mDir.mY);mSpeed=2;aDoBad=true;}
						aRect.mY+=mSpeed+5;
					}
					else if (aNewDir.mY>0)
					{
						aRect.mY+=mSpeed+5;
						//if (gWorld->CollideDir(aRect,mDir.mX)) {mDir.mY=-fabs(mDir.mY);mSpeed=.1f;aNewDir.mY=0;}
						if (gWorld->Collide(aRect)) {mDir.mY=-(float)fabs(mDir.mY);mSpeed=2;aDoBad=true;}
						aRect.mY-=mSpeed+5;
					}

					if (aNewDir.mX<0)
					{
						aRect.mX-=mSpeed+5;
						if (gWorld->CollideDir(aRect,mDir.mX)) {mDir.mX=(float)fabs(mDir.mX);}// mSpeed=.1f;}
						//if (gWorld->Collide(aRect)) {mDir.mX=fabs(mDir.mX);mSpeed=2;}
						aRect.mX+=mSpeed+5;
					}
					else if (aNewDir.mX>0)
					{
						aRect.mX+=mSpeed+5;
						if (gWorld->CollideDir(aRect,mDir.mX)) {mDir.mX=-(float)fabs(mDir.mX);}// mSpeed=.1f;}
						//if (gWorld->Collide(aRect)) {mDir.mX=-fabs(mDir.mX);mSpeed=2;}
						aRect.mX-=mSpeed+5;
					}
					aNewDir=mDir*mDirMod*mSpeed;
					if (mBadCounter>20) 
					{
						mSpeed=.1f;
					}
				}

				if (aDoBad) mBadCounter++;
				else mBadCounter=0;//_max(0,mBadCounter-1);
			}

			mPos+=aNewDir;
			mSpeed=_max(1.0f,mSpeed-mSpeedDecay);
			if (mBadCounter>20)
			{
				switch (gRand.Get(4))
				{
				case 0:mDir=Point(-1,-1);break;
				case 1:mDir=Point(1,1);break;
				case 2:mDir=Point(1,-1);break;
				case 3:mDir=Point(-1,1);break;
				}
			}
			break;
		}
	

		if (--mDripCountdown<=0)
		{
			mDripCountdown=mDripCountdownMax;
			mBlinkCountdown=10;
			mBlinkWait=50+gRand.Get(100);


			mFlashWhite=.5f;
			gSounds->mDrip.Play(gWorld->GetVolume(mPos));


			for (int aCount=0;aCount<7;aCount++)
			{
				if (gRand.GetBool())
				{
					Point aWig=gMath.AngleToVector(mWiggle[aCount].mX)*2;
					Point aPos=mPos+gBundle_Play->mDripbosseye[aCount].mKey[0];

					Acid *aAcid=new Acid();
					aAcid->mEnergyLevel=99;
					//aAcid->mMyDrip=this;
					aAcid->mPos=aPos+Point(0,20);
					aAcid->mFallSpeed+=gRand.GetF(2);
					gWorld->mPlayerList+=aAcid;
				}
			}
		}
	}

	if (!mDying)
	{
		//
		// Did we hit Robot?
		//

		{
			Rect aRect=mCollide;
			aRect.mX+=mPos.mX;
			aRect.mY+=mPos.mY;

			Rect aRobotRect=gWorld->mRobot->mCollide;//.Expand(10);
			aRobotRect.mY+=5;
			aRobotRect.mHeight-=10;
			aRobotRect.mX+=gWorld->mRobot->mPos.mX;
			aRobotRect.mY+=gWorld->mRobot->mPos.mY;

			if (aRobotRect.Intersects(aRect))
			{
				gWorld->mRobot->Die(false);
			}
		}
	}

}

void DripBoss::Draw()
{
	if (!mBoss) FixBossGraphic();

	gG.Translate(0,5);
	DrawGuy();
	gG.Translate(0,-5);

	if (mDying)
	{
		gG.SetColor(1.0f-((float)mDyingCountdown/40.0f));
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}
	else if (mFlashWhite)
	{
		gG.SetColor(mFlashWhite);
		gG.RenderWhite(true);
		DrawGuy();
		gG.SetColor();
		gG.RenderWhite(false);
	}

/*
	Rect aRect=mCollide;
	aRect.mX+=mPos.mX;
	aRect.mY+=mPos.mY;

	gG.SetColor(.5f);
	gG.FillRect(aRect.Translate(gMath.Sign(mDir.mX)*40,gMath.Sign(mDir.mY)*40));
	gG.SetColor();
	/**/
}

void DripBoss::DrawGuy()
{
	mBoss->Element(0).Center(mPos);
	for (int aCount=0;aCount<7;aCount++)
	{
		Point aWig=gMath.AngleToVector(mWiggle[aCount].mX)*2;
		gG.Translate(aWig);
		
		mBoss->Element(aCount+3).Center(mPos);

		if (mBlinkCountdown)
		{
			mBoss->Element(1).Center(mPos+gBundle_Play->mDripbosseye[aCount].mKey[0]);
		}
		else
		{
			Point aVec=gWorld->mRobot->mPos-(mPos+gBundle_Play->mDripbosseye[aCount].mKey[0]);
			aVec.Normalize();
			gBundle_Play->mDripboss_Pupil.Center(mPos+gBundle_Play->mDripbosseye[aCount].mKey[0]+(aVec*2));
		}
		gG.Translate(-aWig);
	}
}

void DripBoss::Ouch(int theExtraDamage)
{
	mFlashWhite=1.0f;
	mHP-=theExtraDamage+1;

	if (mHP<=0)
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}
	else 
	{
		mSpeed=5.0f;
		mDripCountdownMax=_max(15,mDripCountdownMax-1);
		if ((mHP%2)==0) mDripCountdownMax--;
	}

}


void Player::Sync(SaveGame &theSG)
{
	theSG.StartChunk();
	theSG.Sync(&mID);
	theSG.Sync(&mPos);
	theSG.Sync(&mHP);
	theSG.Sync(&mIsEnemy);
	theSG.Sync(&mGridPos);
	theSG.Sync(&mCollide);
	theSG.Sync(&mSleep);
	theSG.Sync(&mEnergyLevel);
	theSG.EndChunk();
}

void Robot::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mSpeed);
	theSG.Sync(&mFacing);
	theSG.Sync(&mGravity);
	theSG.Sync(&mBounceOffset);
	theSG.Sync(&mBounceGravity);
	theSG.Sync(&mPushing);
	theSG.Sync(&mBob);
	theSG.Sync(&mWantBounce);
	theSG.Sync(&mInJump);
	theSG.Sync(&mDoubleJumpCount);
	theSG.Sync(&mTouchingGround);
	theSG.Sync(&mJumpPitch);
	theSG.Sync(&mWantGoLeft);
	theSG.Sync(&mWantGoRight);
	theSG.Sync(&mShootAnimateCountdown);
	theSG.Sync(&mShootWaitCountdown);
	theSG.Sync(&mShootHeldDown);
	theSG.Sync(&mRocketCountdown);
	theSG.Sync(&mRocketed);
	theSG.Sync(&mRocketUpCountdown);
	theSG.Sync(&mRocketedUp);
	theSG.Sync(&mRed);
	theSG.Sync(&mCanJump);
	theSG.Sync(&mCanDoubleJump);
	theSG.Sync(&mCanShoot);
	theSG.Sync(&mCanAnnihiliate);
	theSG.Sync(&mCanRocket);
	theSG.Sync(&mCanRocketUp);
	theSG.Sync(&mHasHelmet);
	theSG.Sync(&mHasKey[0]);
	theSG.Sync(&mHasKey[1]);
	theSG.Sync(&mHasKey[2]);
	theSG.Sync(&mHasExplozor);
	theSG.Sync(&mHasHaxxor);
	theSG.Sync(&mCrystals);
	theSG.Sync(&mGooPos);
	theSG.Sync(&mInvulnerableCount);
	theSG.Sync(&mNoControlCount);
	theSG.Sync(&mMaxHP);
	theSG.Sync(&mKillPause);
	theSG.Sync(&mPowerupCount);
	mPowerupOrder.GuaranteeSize(mPowerupCount);
	for (int aCount=0;aCount<mPowerupCount;aCount++) theSG.Sync(&mPowerupOrder[aCount]);
	theSG.Sync(&mGoldKeyCount);
	theSG.Sync(&mZoinging);
	theSG.Sync(&mGMod);
	if (mGMod==0) mGMod=1.0f;
	theSG.Sync(&mOnSlamwall);
	theSG.Sync(&mLastSlamwall);
	theSG.Sync(&mHasVelcro);
	theSG.Sync(&mOnVelcro);
	theSG.Sync(&mLazorLevel);
	theSG.EndChunk();
}

void Lazor::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mGlint);
	theSG.Sync(&mDir);
	theSG.Sync(&mSpeed);
	theSG.Sync(&mCheck);
	theSG.Sync(&mExplode);
	theSG.Sync(&mDamage);
	theSG.EndChunk();
}

void Kitty::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mTail);
	theSG.Sync(&mBlinkCountdown);
	theSG.Sync(&mFirstUpdate);
	theSG.Sync(&mFalling);
	theSG.Sync(&mGravity);
	theSG.Sync(&mNullKitty);
	theSG.EndChunk();
}

void Pickup::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mType);
	theSG.Sync(&mBob);
	theSG.Sync(&mShrink);
	theSG.Sync(&mScale);
	theSG.EndChunk();
}

void Computron::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mActivated);
	theSG.Sync(&mEyeOffset);
	theSG.Sync(&mEyeDir);
	theSG.Sync(&mEyeWait);
	theSG.Sync(&mRandSeed);
	theSG.Sync(&mRandCountdown);
	theSG.Sync(&mBlinkCountdown);
	theSG.Sync(&mBlinkWait);
	theSG.Sync(&mFlash);
	theSG.EndChunk();
}

void RedGuy::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mChomp);
	theSG.Sync(&mStep);
	theSG.Sync(&mFacing);
	theSG.Sync(&mChompSpeed);
	theSG.Sync(&mFlashWhite);
	theSG.Sync(&mDying);
	theSG.Sync(&mDyingCountdown);
	theSG.Sync(&mIsChild);
	theSG.Sync(&mAge);
	theSG.Sync(&mIsFalling);
	theSG.Sync(&mFallSpeed);
	theSG.EndChunk();
}

void BlueGuy::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mFrame);
	theSG.Sync(&mFrameDir);
	theSG.Sync(&mFlashWhite);
	theSG.Sync(&mDying);
	theSG.Sync(&mDyingCountdown);
	theSG.Sync(&mDir);
	theSG.Sync(&mBlinkCountdown);
	theSG.Sync(&mLookatAngle);
	theSG.Sync(&mWantLookatAngle);
	theSG.Sync(&mLookatDistance);
	theSG.Sync(&mWantLookatDistance);
	theSG.EndChunk();
}

void Drip::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mFrame);
	theSG.Sync(&mFlashWhite);
	theSG.Sync(&mDying);
	theSG.Sync(&mDyingCountdown);
	theSG.Sync(&mShootCountdown);
	theSG.Sync(&mShootPause);
	theSG.EndChunk();
}

void Acid::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mRotate);
	theSG.Sync(&mFallSpeed);
	theSG.EndChunk();
}

void Boss::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mSpeed);
	theSG.Sync(&mDir);
	theSG.Sync(&mFlashWhite);
	theSG.Sync(&mDying);
	theSG.Sync(&mDyingCountdown);
	theSG.Sync(&mSlimeWait);
	theSG.Sync(&mFireCountdown);
	theSG.EndChunk();
}

void EnergyBolt::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mRotate);
	theSG.Sync(&mPhase);
	theSG.Sync(&mDir);
	theSG.EndChunk();
}

void Computer::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mHappy);
	theSG.Sync(&mBlinkCountdown);
	theSG.Sync(&mBob);
	theSG.Sync(&mFlash);
	theSG.Sync(&mHappyCountdown);
	theSG.EndChunk();
}

void RedBoss::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mBirthing);
	theSG.Sync(&mBirthingDelay);
	theSG.EndChunk();
}

void RedBaby::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mGravity);
	theSG.Sync(&mDir);
	theSG.Sync(&mFlash);
	theSG.Sync(&mRotate);
	theSG.Sync(&mBounce);
	theSG.EndChunk();
}

void Telematic::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mCrystalsNeeded);
	theSG.Sync(&mCrystals);
	theSG.Sync(&mActivated);
	theSG.Sync(&mEyeOffset);
	theSG.Sync(&mEyeDir);
	theSG.Sync(&mEyeWait);
	theSG.Sync(&mRandSeed);
	theSG.Sync(&mRandCountdown);
	theSG.Sync(&mBlinkCountdown);
	theSG.Sync(&mBlinkWait);
	theSG.Sync(&mCycle);
	theSG.Sync(&mFlash);
	theSG.Sync(&mLastTeleportTo);
	theSG.Sync(&mClockwise);
	theSG.EndChunk();
}

void Telematic_Slot::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mActivated);
	theSG.Sync(&mGlow);
	theSG.Sync(&mMyTelematic);
	theSG.Sync(&mProximity);
	theSG.EndChunk();
}

void Telepad::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mFacing);
	theSG.Sync(&mActivated);
	theSG.Sync(&mMyTelematic);
	theSG.Sync(&mGlow);
	theSG.Sync(&mPulse);
	theSG.Sync(&mGrab);
	theSG.Sync(&mNoGrab);
	theSG.EndChunk();
}

void DripBoss::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mDyingCountdown);
	theSG.Sync(&mDying);
	theSG.Sync(&mFlashWhite);
	theSG.Sync(&mBlinkCountdown);
	theSG.Sync(&mBlinkWait);
	theSG.Sync(&mDir);
	theSG.Sync(&mSpeed);
	theSG.Sync(&mSpeedDecay);
	theSG.Sync(&mDripCountdown);
	theSG.Sync(&mDripCountdownMax);
	theSG.EndChunk();
}

AcidBurner::AcidBurner()
{
	mID=ID_ACIDBURNER;
	mHP=3000;
	mIsEnemy=false;

	mCollide=Rect(-20,-20,40,40);
	mDying=false;
	mFade=0;
	mFadeDir=1;
	mDidMelt=false;

	mMeltDownType=0;
	mMeltDownMoveSpeed=1;
	mMeltCount=3;
	mDoneMode=false;
}

AcidBurner::~AcidBurner()
{
}

void AcidBurner::Update()
{
	if (mDoneMode)
	{
		if ((gAppPtr->AppTime()%10)==0) if (gWorld->GetGrid(mGridPos.mX,mGridPos.mY)==0) Kill();
		return;
	}

	if (mFadeDir>0)
	{
		if (mFade>.5f) HitRobot();

		mFade+=.02f;
		if (mFade>1.0f)
		{
			mGridPos=mFirstPos;

			mFade=1.0f;
			mFadeDir=0;

			MeltDown();
		}
	}
	else if (mFadeDir<0)
	{
		if (mFade>.5f) HitRobot();
		mFade-=.04f;
		if (mFade<0) mFade=0;
	}
	else if (mFadeDir==0)
	{
		switch (mMeltDownType)
		{
		case 0:
			{
				//
				// Melting a square
				//
				mPos.mY+=mMeltDownMoveSpeed;
				if (--mMeltDownCountdown<=0) MeltDown();

				HitRobot();
				break;
			}
		case 1:
			{
				//
				// Fell into space
				//
				mPos.mY+=mMeltDownMoveSpeed;
				if (mMeltDownMoveSpeed>0) mMeltDownMoveSpeed+=.5f;

				mFade-=.06f;
				if (mFade<0) mFade=0;

				if (mFade>.5f) HitRobot();
				break;
			}
		case 2:
			{
				mPos.mY+=mMeltDownMoveSpeed;
				mMeltDownMoveSpeed+=.5f;
				if (mMeltDownMoveSpeed>10) mMeltDownMoveSpeed=10;

				IPoint aG=gWorld->WorldToGrid(mPos+Point(0,20));
				if (aG.mY>mGridPos.mY)
				{
					mPos=gWorld->GridToWorld(aG.mX,aG.mY-1);
					mGridPos=gWorld->WorldToGrid(mPos);
					MeltDown();
				}

				HitRobot();
				break;
			}
		case 3:
			{
				mPos.mY+=mMeltDownMoveSpeed;
				if (--mKillCountdown<=0) Kill();
				break;
			}
		}
	}

	if (mFade<=0) 
	{
		if (mMeltCount<=0) 
		{
			mDoneMode=true;
			mGridPos.mY++;
			mPos.mY+=40;

			gWorld->mPlayerList-=this;
			gWorld->mPlayerList.Insert(this);
		}
		else Kill();
	}
	if (mPos.mY>(gWorld->mGridHeight*gWorld->mGridSize)+320) mMeltDownType=1;		

	if ((gAppPtr->AppTime()%10)==0)
	{
		float aVol=.2f;
		if (mMeltDownType==0 && mGridPos.mY>mFirstPos.mY) aVol=.6f;
		gSounds->mCrumble.PlayPitched(1.5f-gRand.GetSpanF(-.25f,.25f),aVol*mFade*gWorld->GetVolume(mPos));
	}

}

void AcidBurner::Draw()
{
	if (mDoneMode)
	{
		gBundle_Play->mAcidDamage.Center(mPos);
		//gG.SetAdditive(true);
		//gBundle_Play->mAciddamage.Center(mPos);
		//gG.SetAdditive(false);
		return;
	}

	float aScale=1.0f;
	if ((mMeltDownType==0 || mMeltDownType==3) && mGridPos.mY>mFirstPos.mY) aScale+=.1f+(gMath.Sin(gApp.AppTimeF()*25.0f)*.1f);

	gG.SetColor(mFade);
	gBundle_Play->mAcidglow.DrawScaled(mPos,aScale);
	gG.RenderAdditive();
	gG.SetColor(mFade*mFade*mFade);
	gBundle_Play->mAcidglow.DrawScaled(mPos,aScale);
	gG.RenderNormal();

	if (mMeltCount==0)
	{
		if (mMeltDownMoveSpeed==0)
		{
			gG.SetColor(1-mFade);
			gBundle_Play->mAcidDamage.Center(mPos+Point(0,40));
			//gG.SetAdditive(true);
			//gBundle_Play->mAciddamage.Center(mPos+Point(0,40));
			//gG.SetAdditive(false);
		}
	}




	gG.SetColor();

}

void AcidBurner::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();

	theSG.Sync(&mFade);
	theSG.Sync(&mDidMelt);
	theSG.Sync(&mFadeDir);
	theSG.Sync(&mFirstPos);
	theSG.Sync(&mGridPos);
	theSG.Sync(&mMeltDownCountdown);
	theSG.Sync(&mMeltDownType);
	theSG.Sync(&mMeltDownMoveSpeed);
	theSG.Sync(&mMeltCount);
	theSG.Sync(&mDoneMode);
	theSG.Sync(&mKillCountdown);

	theSG.EndChunk();
}

void AcidBurner::MeltDown()
{

//	gWorld->SetGrid(mGridPos.mX,mGridPos.mY,0);
	World::Grid* aG=gWorld->GetGridPtr(mGridPos.mX,mGridPos.mY);
	if (aG) 
	{
		if (aG->mLayout>0)
		{
			if (aG->mLayout!=2 && aG->mLayout!=38 && aG->mLayout!=39 && aG->mLayout!=40 && aG->mLayout!=43 && aG->mLayout!=20)
			{
				gWorld->SetGridEX(mGridPos.mX,mGridPos.mY,0);
				aG->mPaintID=0;
			}
			if (mGridPos.mY>mFirstPos.mY) mMeltCount--;
		}
	}

	mGridPos.mY++;
	aG=gWorld->GetGridPtr(mGridPos.mX,mGridPos.mY);
	if (aG)
	{
		if (aG->mLayout==EXPLODE_BLOCK || aG->mLayout==EXPLODE_CRATE)
		{
			gWorld->Explode(mGridPos.mX,mGridPos.mY,10);
			mMeltDownType=3;
			mKillCountdown=10;
			return;
		}
	}

	int aType=gWorld->GetGrid(mGridPos.mX,mGridPos.mY);


	if (aType==2 || aType==38 || aType==39 || aType==43 || aType==20)
	{
		mGridPos.mY--;
		mMeltDownMoveSpeed=0;
		mMeltDownType=1;
	}
	else if (aType==0)
	{
		if (mDidMelt)
		{
			mMeltDownType=1;
			mMeltDownMoveSpeed=1;
			mMeltDownCountdown=(int)(40.0f/mMeltDownMoveSpeed);
		}
		else mMeltDownType=2;
	}
	else
	{
		if (mMeltCount<=0)
		{
			mGridPos.mY--;
			mMeltDownMoveSpeed=0;
			mMeltDownType=1;
		}
		else
		{
			mMeltDownMoveSpeed=1;
			mMeltDownCountdown=(int)(40.0f/mMeltDownMoveSpeed);
			mMeltDownType=0;
			mDidMelt=true;
		}
	}
}

void AcidBurner::HitRobot()
{
	Rect aRect=mCollide;
	aRect.mX+=mPos.mX;
	aRect.mY+=mPos.mY;

	Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
	aRobotRect.mY+=5;
	aRobotRect.mHeight-=10;
	aRobotRect.mX+=gWorld->mRobot->mPos.mX;
	aRobotRect.mY+=gWorld->mRobot->mPos.mY;

	if (aRobotRect.Intersects(aRect))
	{
		gWorld->mRobot->Die(false);
	}
}

GunTarget::GunTarget()
{
	mOn=true;
	mID=ID_GUNTARGET;
	mHP=3000;
	mIsEnemy=false;
	mCollide=Rect(-20,-20,40,40);
	mDying=false;
	mChangeCount=0;
	mScale=1.0f;
	mFlip=gRand.NegateF(1);
	mCuredCountdown=0;
}

void GunTarget::Update()
{
	if (--mChangeCount<=0)
	{
		mChangeCount=15;
		mScale=1.0f-gRand.GetF(.2f);
		mFlip=gRand.NegateF(1);
	}

	if (mCuredCountdown>0)
	{
		if (--mCuredCountdown==0) 
		{
			gWorld->OpenVirusDoor(gWorld->WorldToGrid(mPos));
		}
	}
}

void GunTarget::Draw()
{
	gBundle_Play->mGunTarget.Center(mPos);
	if (mOn) if (((gApp.AppTime()/8)%2)==0)
	{
		Matrix aMat;
		aMat.Scale(mScale*mFlip,mScale,1);
		aMat.Translate(mPos+Point(-1,0));
		gBundle_Play->mGuntargeticon.Draw(aMat);
	}
}

void GunTarget::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mOn);
	theSG.Sync(&mChangeCount);
	theSG.Sync(&mScale);
	theSG.Sync(&mFlip);
	theSG.Sync(&mCuredCountdown);
	theSG.EndChunk();
}

void GunTarget::Hit()
{
	if (mSleep>0) return;
	if (mOn)
	{
		mOn=false;
		gSounds->mExplodecrate.PlayPitched(1.25f);
		gSounds->mHappycomputer.PlayPitched(1.25f);
		gWorld->mFlashWhite=1.0f;

		Anim_SpriteStrip *aSS=new Anim_SpriteStrip(&gBundle_Play->mExplozor);
		aSS->mPos=mPos;
		aSS->mFrameSpeed=.75f;//.75f;
		aSS->mDir=Point(0,-1);
		aSS->mColor=Color(.2f,1,.2f);
		aSS->mScale=1.5f;
		aSS->mAdditive=true;
		gWorld->mAnimatorList_Top+=aSS;

		mCuredCountdown=50;
	}
}

Greenguy::Greenguy()
{
	mID=ID_GREENGUY;
	mHP=8;
	mIsEnemy=true;

	mCollide=Rect(-20,-20,40,40);
	mDying=false;
	mAge=gRand.Get(360);
	mFacing=-1;
	mRise=0;
	mWantRise=0;
	mSinkDelay=0;
	mCheckFooting=0;
	mFeelerOffset=0;
}

void Greenguy::Update()
{
	mAge++;


	if (gWorld->mVisibleRect.Expand(400).ContainsPoint(mPos.mX,mPos.mY))
	{
		mArmPos[gRand.Get(20)][gRand.Get(2)]=Point(gRand.GetSpanF(-2,2),gRand.GetSpanF(-1,1));
		if (++mCheckFooting>5) mCheckFooting=0;

		if (gRand.Get(30)==3) mFeelerOffset=gRand.GetF(1);

		if (gWorld->mRobot->mPos.mX>mPos.mX) mFacing=-1;
		else mFacing=1;

		//if ((gWorld->mRobot->mPos.mY)<=(mPos.mY+mCollide.mY))

		if ((gWorld->mRobot->mPos.mY+5)<mPos.mY-mWantRise)
		{
			mWantRise=(mPos.mY-gWorld->mRobot->mPos.mY);
			if (mWantRise>120) mWantRise=120;
			for (float aTick=0;aTick<mWantRise;aTick+=5)
			{
				IPoint aP=gWorld->WorldToGrid(mPos.mX,(mPos.mY-aTick)-20);
				if (gWorld->IsBlocked(aP)) mWantRise=aTick;
			}

		}
		else 
		{
			mWantRise=0;
		}

		if (mRise>mWantRise)
		{
			if (mSinkDelay<=0)
			{
				mRise--;
				if (mRise<mWantRise) mRise=mWantRise;
			}
		}
		if (mRise<mWantRise)
		{
			mRise+=10;
			if (mRise>mWantRise) 
			{
				mRise=mWantRise;
			}
		}

		if (mRise<=mWantRise) mSinkDelay=80;

		float aRealHeight=40+(mRise);
		mCollide=Rect(-20,-((aRealHeight-20)),40,aRealHeight);

		mSinkDelay--;
		if (mSinkDelay<0) mSinkDelay=0;

		Rect aRect=mCollide;
		aRect.mX+=mPos.mX;
		aRect.mY+=mPos.mY;

		Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
		aRobotRect.mY+=5;
		aRobotRect.mHeight-=10;
		aRobotRect.mX+=gWorld->mRobot->mPos.mX;
		aRobotRect.mY+=gWorld->mRobot->mPos.mY;

		if (aRobotRect.Intersects(aRect)) gWorld->mRobot->Die(false);

		if (mCheckFooting==0)
		{
			IPoint aMyPos=gWorld->WorldToGrid(mPos);
			if (gWorld->GetGrid(aMyPos.mX,aMyPos.mY+1)==0)
			{
				mHP=-1;
				mDying=true;
			}
		}
	}


	if (mDying)
	{
		Kill();
		gWorld->mTimerPause+=(1*50);
		gSounds->mLaserouch.PlayPitched(1.0f+gRand.GetSpanF(-.05f,.05f));
		gWorld->mFlashWhite=.5f;

		for (float aCount=0;aCount<360;aCount+=30)
		{
			float aMyAngle=aCount+gRand.GetSpanF(-4,4);
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
			aS->mFade=1.5f;
			aS->mFadeSpeed=-.1f;
			aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(-2,2));
			aS->mPos=mPos+(aS->mDir*3);
			aS->mColor=Color(1,.5f,0);
			//aS->mDir*=1.0f+gRand.GetFloat(1.5f);
			aS->mDirMult=Point(.75f,.75f);
			gWorld->mAnimatorList_Top+=aS;
		}
		for (float aCount=0;aCount<360;aCount+=30)
		{
			float aMyAngle=aCount+gRand.GetSpanF(-4,4);
			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
			aS->mFade=1.5f;
			aS->mFadeSpeed=-.1f;
			aS->mDir=gMath.AngleToVector(aMyAngle)*(9+gRand.GetSpanF(-2,2));
			aS->mPos=mPos+(aS->mDir*3);
			//aS->mDir*=1.0f+gRand.GetFloat(1.5f);
			aS->mDirMult=Point(.77f,.77f);
			gWorld->mAnimatorList_Top+=aS;
		}
	}
}

void Greenguy::Draw()
{
	gG.PushTranslate();
	gG.Translate(mPos);

	gG.PushClip();

	gG.Clip(mCollide.mX-100,mCollide.mY,200.0f,mCollide.mHeight);
	gG.Translate(mFacing*5,+(gMath.Sin(mAge*4.0f)*2));

	if (mCollide.mY<-20)
	{
		char aSkip=1;
		int aArm=0;
		for (float aSpanY=mCollide.UpperLeft().mY+35;aSpanY<mCollide.LowerRight().mY+8;aSpanY+=8)
		{
			if (aSkip==0)
			{
				Matrix aMat;
				aMat.Scale(mFacing,1,1);
				if (mFacing>0) aMat.Translate(mCollide.mX+10,aSpanY-5);
				else aMat.Translate(mCollide.mX+30,aSpanY-5);
				aMat.Translate(mArmPos[aArm][0]);
				gBundle_Play->mGreenGuy_Arm.Draw(aMat);
				aArm++;
			}
			aSkip++;
			if (aSkip>1) aSkip=0;
		}

		for (float aSpanY=mCollide.UpperLeft().mY+35;aSpanY<mCollide.LowerRight().mY+8;aSpanY+=8)
		{
			Matrix aMat;
			aMat.Scale(mFacing,1,1);
			aMat.Translate(mCollide.mX+20,aSpanY);
			gBundle_Play->mGreenGuy_Body.Draw(aMat);

		}

		aSkip=1;
		aArm=0;
		for (float aSpanY=mCollide.UpperLeft().mY+35;aSpanY<mCollide.LowerRight().mY+8;aSpanY+=8)
		{
			if (aSkip==0)
			{
				Matrix aMat;
				aMat.Scale(mFacing,1,1);
				aMat.Translate(mCollide.mX+20,aSpanY);
				aMat.Translate(mArmPos[aArm][1]);
				gBundle_Play->mGreenGuy_Arm.Draw(aMat);

				aArm++;
			}
			aSkip++;
			if (aSkip>1) aSkip=0;
		}
	}


	Matrix aMat;
	aMat.Scale(mFacing,1,1);
	aMat.Translate(mCollide.mX+20,mCollide.mY+35);//+(gMath.Sin(mAge*4)*2));
	gBundle_Play->mGreenGuy_Shell.Draw(aMat);
	aMat.Translate(-mFeelerOffset,gMath.Sin(mAge*10.0f)*.5f);
	gBundle_Play->mGreenGuy_Feelers.Draw(aMat);

	gG.PopClip();
	gG.PopTranslate();

	gBundle_Play->mGreenGuy_Hole.Center(mPos);



/*
	float aNeckScale=1.0f+(mRise/25);
	aNeckScale+=gMath.Sin(mAge*6)*.05f;
	
	aMat.Reset();
	aMat.Scale(mFacing,aNeckScale,1);
	aMat.Translate(mPos+Point(0,-18));
	gBundle_Play->mGreenguy_Neck.Draw(aMat);

	aMat.Reset();
	aMat.Scale(mFacing,1,1);
	aMat.Translate(mPos+Point(0.0f,-18-(25*aNeckScale)));
	gBundle_Play->mGreenguy_Eye.Draw(aMat);

/*
	aMat.Reset();
	aMat.Scale(mFacing,1,1);
	aMat.Translate(mPos);
	gBundle_Play->mGreenguy_Mouth.Draw(aMat);
*/
}

void Greenguy::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mFacing);
	theSG.Sync(&mRise);
	theSG.Sync(&mWantRise);
	theSG.Sync(&mSinkDelay);
	theSG.EndChunk();
}

Buzzoid::Buzzoid()
{
	mID=ID_BUZZOID;
	mHP=2;
	mIsEnemy=true;

	mCollide=Rect(-20,-20,40,40);
	mDying=false;
	mDyingCountdown=10;
	mFacing=0;
	mFrame=gRand.GetF(2);

	mFlashWhite=0;
	mSpeed=1.0f;
	mWait=0;

	mFirst=true;
}

void Buzzoid::Update()
{
	if (mFacing==0) mFacing=(QuickRand((int)(mPos.mX*mPos.mY))%2)?1.0f:-1.0f;

	mFlashWhite=_max(0,mFlashWhite-.1f);
	Player::Update();

	if (mDying)
	{
		mDyingCountdown--;
		if (mDyingCountdown<=0)
		{
			// Explode
			Kill();
			gWorld->mTimerPause+=(1*50);
			gSounds->mLaserouch.PlayPitched(1.0f+gRand.GetSpanF(-.05f,.05f));
			//gWorld->mFlashWhite=1.0f;

			for (float aCount=0;aCount<360;aCount+=30)
			{
				float aMyAngle=aCount+gRand.GetSpanF(-4,4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.1f;
				aS->mDir=gMath.AngleToVector(aMyAngle)*(8+gRand.GetSpanF(-2,2));
				aS->mPos=mPos+(aS->mDir*3);
				aS->mColor=Color(1,0,1);
				//aS->mDir*=1.0f+gRand.GetFloat(1.5f);
				aS->mDirMult=Point(.75f,.75f);
				gWorld->mAnimatorList_Top+=aS;
			}
			for (float aCount=0;aCount<360;aCount+=30)
			{
				float aMyAngle=aCount+gRand.GetSpanF(-4,4);
				Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mOuchstar);
				aS->mFade=1.5f;
				aS->mFadeSpeed=-.1f;
				aS->mDir=gMath.AngleToVector(aMyAngle)*(9+gRand.GetSpanF(-2,2));
				aS->mPos=mPos+(aS->mDir*3);
				//aS->mDir*=1.0f+gRand.GetFloat(1.5f);
				aS->mDirMult=Point(.77f,.77f);
				gWorld->mAnimatorList_Top+=aS;
			}
		}
	}
	else if (gWorld->mVisibleRect.Expand(400).ContainsPoint(mPos.mX,mPos.mY))
	{
		if (mFirst)
		{
			Point aTest=Point(mPos.mX+((gWorld->mGridSize)),mPos.mY);
			if (gWorld->IsBlockedDir(gWorld->WorldToGrid(aTest),1)) mFacing=-1;

			aTest=Point(mPos.mX-((gWorld->mGridSize)),mPos.mY);
			if (gWorld->IsBlockedDir(gWorld->WorldToGrid(aTest),-1)) mFacing=1;

			mFirst=false;
		}

		float aOldFacing=mFacing;

		Point aTest=Point(mPos.mX+(mFacing*(gWorld->mGridSize/2)),mPos.mY);
		if (gWorld->IsBlockedDir(gWorld->WorldToGrid(aTest),mFacing)) 
		{
			mFacing=-aOldFacing;	
			mSpeed=0;
			mWait=45;
			mFrame=0;
			mPos=gWorld->GridToWorld(gWorld->WorldToGrid(mPos));
			mPos.mX-=mFacing*4;
		}

		mPos.mX+=mFacing*mSpeed*2;

		if (--mWait<=0)
		{
			mSpeed+=.1f;
			if (mSpeed>3.5f) mSpeed=3.5f;
			mWait=0;
		}

		mFrame+=.4f*mSpeed;
		if (mFrame>=2) mFrame-=2;

		Rect aRect=mCollide;
		aRect.mX+=mPos.mX;
		aRect.mY+=mPos.mY;

		Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND);
		aRobotRect.mY+=5;
		aRobotRect.mHeight-=10;
		aRobotRect.mX+=gWorld->mRobot->mPos.mX;
		aRobotRect.mY+=gWorld->mRobot->mPos.mY;

		if (aRobotRect.Intersects(aRect)) gWorld->mRobot->Die(false);
	}

}

void Buzzoid::Draw()
{

	gG.Translate(0,4);

	Point aShake;
	if (mDying)
	{
		aShake=gMath.AngleToVector()*gRand.GetF(10);
		gG.Translate(aShake);
	}

	Array<Sprite>* aBuzzoid=&gBundle_Play->mBuzzoid;
	switch (mEnergyLevel)
	{
	case 1:aBuzzoid=&gBundle_Play->mBuzzoid1;break;
	case 2:aBuzzoid=&gBundle_Play->mBuzzoid2;break;
	case 3:aBuzzoid=&gBundle_Play->mBuzzoid3;break;
	}

	Matrix aMat;
	aMat.Scale(mFacing,1,1);
	aMat.Translate(mPos);
	aBuzzoid->Element((int)mFrame).Draw(aMat);

	if (mFlashWhite>0)
	{
		gG.RenderWhite(true);
		gG.SetColor(mFlashWhite);
		gBundle_Play->mBuzzoid[(int)mFrame].Draw(aMat);
		gG.SetColor();
		gG.RenderWhite(false);
	}

	if (mDying) 
	{
		gG.SetColor(1.0f-((float)mDyingCountdown/20.0f));
		gG.RenderWhite(true);
		gBundle_Play->mBuzzoid[(int)mFrame].Draw(aMat);

		gG.Translate(-aShake);
		gG.RenderWhite(false);
		gG.SetColor();
	}

	gG.Translate(0,-4);

}

void Buzzoid::Ouch(int theExtraDamage)
{
	mFlashWhite=1.0f;
	mHP-=theExtraDamage+1;
	if (mHP<=0) 
	{
		if (!mDying) gSounds->mCritterscreech.Play();
		mDying=true;
	}
}


void Buzzoid::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mFacing);
	theSG.Sync(&mFacing);
	theSG.Sync(&mWait);
	theSG.Sync(&mSpeed);
	theSG.Sync(&mDyingCountdown);
	theSG.Sync(&mFlashWhite);
	theSG.Sync(&mFirst);
	theSG.EndChunk();
}

Coin::Coin()
{
	mID=ID_COIN;
	mFrame=gRand.GetF((float)gBundle_Play->mCoin.Size());
	mLift=0;
	mWhite=0;
}

void Coin::Update()
{
	mFrame+=.2f;

	if (!mLift)
	{
		Rect aRect=mCollide;
		aRect.mX+=mPos.mX;
		aRect.mY+=mPos.mY;

		Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND+10);
		aRobotRect.mY+=5;
		aRobotRect.mHeight-=10;
		aRobotRect.mX+=gWorld->mRobot->mPos.mX;
		aRobotRect.mY+=gWorld->mRobot->mPos.mY;

		if (aRobotRect.Intersects(aRect)) 
		{
			gSounds->mGotcoin.Play();
			mLift=-1;
			gWorld->mCoins=_min(gWorld->mCoins+1,gWorld->mMaxCoins);
			if (gWorld->mCoins==gWorld->mMaxCoins) 
			{
				gWorld->AddMessage("ALL THE BITCOINS!","");
				gWorld->mRemoveBitcoinBlockCountdown=50;
			}

			else gWorld->AddMessage(Sprintf("%d/%d",gWorld->mCoins,gWorld->mMaxCoins),"BITCOIN!");
		}
	}
	else
	{
		mFrame+=.1f;
		mLift-=5;
		if (mLift<-50) Kill();

		mWhite=_min(1.0f,mWhite+.1f);
	}
	if (mFrame>=gBundle_Play->mCoin.Size()) mFrame-=gBundle_Play->mCoin.Size();
}

void Coin::Draw()
{
	gBundle_Play->mCoin[(int)mFrame].Center(mPos+Point(0.0f,mLift));
	if (mWhite)
	{
		gG.RenderWhite(true);
		gG.SetColor(mWhite);
		gBundle_Play->mCoin[(int)mFrame].Center(mPos+Point(0.0f,mLift));
		gG.RenderWhite(false);
		gG.SetColor();
	}
}

void Coin::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mFrame);
	theSG.Sync(&mLift);
	theSG.EndChunk();
}


void Robot::Zoing(int theCount, IPoint theGridPos)
{
	if (mRocketCountdown) return;
	if (mRocketUpCountdown) return;
	if (mGravity<0) return;
/*
	if (gWorld->GetGrid(aGridPos.mX,aGridPos.mY)==2) 
	{
		mGooPos=IPoint(aGridPos.mX,aGridPos.mY);
		Die(true,true);
		return;
	}
*/
	//if (gWorld->IsBlocked(aGridPos) || mBounceGravity==-2) mTouchingGround++;
	//
	// Lava thing is not related to this section...
	//
	//mPos.mX=(float)((int)(mPos.mX+.5f));

	
	mGravity=-(((float)theCount/2.0f)+1.1f)*5;
	Point aMoved=Move(Point(0,-1),mGravity*2);
	//mPos.mY+=(mGravity*2);
	mZoinging=true;
	//mZoingCount=10-theCount;
	mTouchingGround=0;

	mDoubleJumpCount=0;
	mJumpPitch=1.11f;
	mJumpKludge=0;

	gSounds->mZoing.Play();

	Smart(ZoingTimer) aZT=new ZoingTimer;
	aZT->mTime=gAppPtr->AppTime();
	aZT->mGridPos=theGridPos;
	gWorld->mZoingTimerList+=aZT;
	//if (aMoved.mY) is too small, we increase aZT->mTime so it doesn't zoing as hard.

}

Lever::Lever()
{
	mID=ID_LEVER;
	mState=false;
	mOnRobot=false;
}

void Lever::Update()
{
	int aOld=mOnRobot;

	Rect aRect=mCollide;
	aRect.mX+=mPos.mX;
	aRect.mY+=mPos.mY;

	Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND+10).ExpandH(20);
	aRobotRect.mY+=5;
	aRobotRect.mHeight-=10;
	aRobotRect.mX+=gWorld->mRobot->mPos.mX;
	aRobotRect.mY+=gWorld->mRobot->mPos.mY;

	if (aRobotRect.Intersects(aRect)) 
	{
		mOnRobot=true;
		if (aOld==false)
		{
			gSounds->mClick.Play();
			mState=!mState;
			gWorld->ReverseConveyor();
			//gWorld->mWantConveyorSpeed*=-1;
			gSounds->mSwitch.Play();
		}
	}
	else mOnRobot=false;
}

void Lever::Draw()
{
	gBundle_Play->mLever[mState].Center(mPos+Point(4,4));
}

void Lever::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mState);
	theSG.Sync(&mOnRobot);
	theSG.EndChunk();
}

void Robot::CheckVelcro(float extraDistance)
{
	char aLastOnVelcro=mOnVelcro;
	mOnVelcro=false;
	if (mHasVelcro)
	{
#define VELCRODISTANCE 15
		{
			Point aCheck=mPos+Point(VELCRODISTANCE+extraDistance,0.0f);
			IPoint aCGrid=gWorld->WorldToGrid(aCheck);
			if (gWorld->GetGrid(aCGrid.mX,aCGrid.mY)==VELCRO_BLOCK) mOnVelcro=1;
		}
		{
			Point aCheck=mPos+Point(-(VELCRODISTANCE+extraDistance),0.0f);
			IPoint aCGrid=gWorld->WorldToGrid(aCheck);
			if (gWorld->GetGrid(aCGrid.mX,aCGrid.mY)==VELCRO_BLOCK) mOnVelcro=-1;
		}

	}

	if (aLastOnVelcro!=mOnVelcro)
	{
		static int aLastPlayed=-1;
		int aLP=gApp.AppTime();

		if (aLP>aLastPlayed+5)
		{
			if (!mOnVelcro) 
			{
				gSounds->mVelcro.Play();
			}
			else gSounds->mVelcro.PlayPitched(.9f,.5f);
			aLastPlayed=aLP;
		}
	}
}


/* RADIO TOWER */

Radio::Radio()
{
	mID=ID_RADIO;
	mOnRobot=false;
	mTick=0;
}

void Radio::Update()
{
	int aOld=mOnRobot;

	Rect aRect=mCollide;
	aRect.mX+=mPos.mX;
	aRect.mY+=mPos.mY;

	Rect aRobotRect=gWorld->mRobot->mCollide.Expand(ROBOTEXPAND+10).ExpandH(20);
	aRobotRect.mY+=5;
	aRobotRect.mHeight-=10;
	aRobotRect.mX+=gWorld->mRobot->mPos.mX;
	aRobotRect.mY+=gWorld->mRobot->mPos.mY;

	if (aRobotRect.Intersects(aRect)) 
	{
		mOnRobot=true;
		if (aOld==false)
		{
			if (mText.Len()==0) 
			{
				mText=gWorld->GetRadioMessage(mGridPos);
			}
			//
			// Play a radio sound!
			//
			if (mText.Len()) 
			{
				
				gWorld->AddMessage("",PFilter(gBundle_Play->mFont_Commodore64Angled18,mText,false));
				gSounds->mRadio.Play();
			}
		}
	}
	else mOnRobot=false;

	EnumList(Wireless,aW,mWirelessList)
	{
		aW->mScale+=.025f;
		if (aW->mScale>2.0f)
		{
			mWirelessList-=aW;
			delete aW;
			EnumListRewind(Wireless);
		}
	}

	if (--mTick<=0)
	{
		Wireless* aW=new Wireless;
		aW->mScale=.25f;
		aW->mAngle=gRand.GetSpanF(-5,5);
		mWirelessList+=aW;
		mTick=30+gRand.Get(10);
	}

}

void Radio::Draw()
{
	gBundle_Play->mRadio.Center(mPos);
	EnumList(Wireless,aW,mWirelessList)
	{
		float aMod=1.0f-(aW->mScale/2.0f);
		gG.SetColor(_min(.65f,aMod*1.5f));
		gBundle_Play->mWireless.DrawRotatedScaled(mPos+Point(-1.0f,-2.0f),aW->mAngle,aW->mScale+.5f);
	}
	gG.SetColor();
}

void Radio::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();
	theSG.Sync(&mOnRobot);
	theSG.Sync(&mText);
	theSG.EndChunk();
}

List gPhageList;

Phage::Phage(void)
{
	mID=ID_RADIO;
	mAngle=-9999;
	mSpeed=0;
	mMouth=1;
	mTurnSpeed=2.0f;
	mEatPitch=1.0f;
	if (gRand.Get(2)==0) mEatPitch=1.25f;
	mScale=0;
	mIsEnemy=true;

	mMouthTick=gRand.Get(3);//0;//.SetDuration(7+gRand.Get(3,true));
	mMaxSpeed=0;//.75f+(gRand.GetSpanF(-1,1)*.25f);
	mEatScale=1.0f;
	mActiveCountdown=0;

	mMode=0;

	gPhageList+=this;
	mInList=true;
}

Phage::~Phage(void)
{
	gPhageList-=this;
}

void Phage::Sync(SaveGame &theSG)
{
	Player::Sync(theSG);
	theSG.StartChunk();

	theSG.Sync(&mAngle);
	theSG.Sync(&mSpeed);
	theSG.Sync(&mMaxSpeed);
	theSG.Sync(&mTurnSpeed);
	theSG.Sync(&mEatPitch);
	theSG.Sync(&mScale);
	theSG.Sync(&mMode);
	theSG.Sync(&mMouth);
	theSG.Sync(&mMouthTick);
	theSG.Sync(&mEatScale);
	theSG.Sync(&mActiveCountdown);

	theSG.EndChunk();
}


void Phage::Update()
{
	if (mMaxSpeed==0)
	{
		mMaxSpeed=3.25f+(QuickRandFraction((int)(mPos.mX*mPos.mY))*.5f);
		mAngle=QuickRandFraction((int)(mPos.mX*mPos.mY*2))*360;
		mScale=.8f+(QuickRandFraction((int)(mPos.mX*mPos.mY*2))*.22f);
		mCollide=Rect(-25*mScale,-25*mScale,50*mScale,50*mScale);
		mTurnSpeed=1.5f-(QuickRandFraction((int)(mPos.mX*mPos.mY*5))*.75f);

	}

	if (--mMouthTick<0) mMouthTick=3;//4+gRand.GetSpan(-1,1);
	if (mEatScale>1.001f)
	{
		mEatScale=_max(1.0f,mEatScale-.015f);
		if (mMouthTick<=0)
		{
			mMouth=1-mMouth;
			if (mMouth==0)
			{
				gSounds->mPhageNibble.PlayPitched(mEatPitch,mScale);
				if (mEatPitch==1.0f) mEatPitch=1.25f;
				else mEatPitch=1.0f;
				//
				// Throw out hap fragments
				//
				mSpeed=mMaxSpeed;
			}
			/*
			//if (gRand.Get(2)==1)
			{
				Animator_Sprite *aS=new Animator_Sprite(&mApp->mHotDot);
				aS->mPos=mPos;
				aS->mDir=gMath.AngleToVector(mAngle+gRand.GetFloat(20,true));
				aS->mPos+=aS->mDir*15;

				aS->mDir*=2+(gRand.GetFloat(3));
				aS->mGravity=Point(0.0f,.25f);

				aS->mDir*=.5f;
				aS->mGravity*=.5f;

				aS->mScale=1.0f;
				aS->mFade=2.0f;
				aS->mFadeSpeed=.075f;

				if (gRand.GetBool()) aS->mColor=Color(1,0,0);

				mBoard->mMiddleAnimatorList+=aS;
			}
			*/
		}


		return;

	}

	mActiveCountdown=_max(0,mActiveCountdown-1);
	bool aChase=false;
	bool aIsVisible=gWorld->mVisibleRect.ContainsPoint(mPos);
	if (aIsVisible) mActiveCountdown=400;

	if (mActiveCountdown>0) aIsVisible=true;
	if (!aIsVisible) 
	{
		if (mInList)
		{
			mInList=false;
			gPhageList-=this;
		}
		mMode=0;
		return;
	}



	if (!mInList)
	{
		mInList=true;
		gPhageList+=this;
	}
	if (gWorld->mRobot) //if (gMath.DistanceSquared(mPos,gWorld->mRobot->mPos))
	{
		//
		// Hap has to be facing away or climbing ladder
		//
		aChase=true;

		//Facing: 0=left, 1=right

		//if (mBoard->mHapHazard->mMode==HapHazard::ModeRunning || mBoard->mHapHazard->mMode==HapHazard::ModeJump)// || mBoard->mHapHazard->mMode==HapHazard::ModeLadder)
		{
			if (gWorld->mRobot->mFacing>0)
			//if (mBoard->mHapHazard->mFacing==0)
			{
				if (mPos.mX>gWorld->mRobot->mPos.mX) aChase=false;
			}

			if (gWorld->mRobot->mFacing<0)
			//if (mBoard->mHapHazard->mFacing==1)
			{
				if (mPos.mX<gWorld->mRobot->mPos.mX) aChase=false;
			}

		}

	}

	if (!aChase) 
	{
		if (mMode==1) 
		{
			float aVolume=1.0f-_clamp(0,(gMath.Distance(mPos,gWorld->mRobot->mPos)-gG.HalfWidthF())/gG.WidthF(),1.0f);
			gSounds->mPhageStop.Play(mScale*aVolume);

			Anim_Sprite *aS=new Anim_Sprite(&gBundle_Play->mJumpburst);
			aS->mPos=mPos;
			aS->mScale=1.25f;
			aS->mScaleSpeed=.1f;
			aS->mFade=1.0f;
			aS->mFadeSpeed=-.1f;
			gWorld->mAnimatorList_UnderPlayers+=aS;
		}
		mSpeed*=.95f;
		mSpeed*=.95f;
		mMouth=0;

		mPos+=gMath.AngleToVector(mAngle)*mSpeed;

		mMode=0;
	}
	else
	{
		mMode=1;
		mSpeed+=.0135f;
		if (mSpeed>mMaxSpeed) mSpeed=mMaxSpeed;

		float aHapAngle=gMath.PointAtAngle2D(gWorld->mRobot->mPos,mPos);
		float aMove=gMath.GetBestRotation(mAngle,aHapAngle);

		mAngle+=(gMath.Sign(aMove)*mTurnSpeed);

		mPos+=gMath.AngleToVector(mAngle)*mSpeed;

		if (mMouthTick<=0) 
		{
			mMouth=1-mMouth;
			if (mMouth==0)
			{
				float aVolume=1.0f-_clamp(0,(gMath.Distance(mPos,gWorld->mRobot->mPos)-gG.HalfWidthF())/gG.WidthF(),1.0f);
				gSounds->mPhageNibble.PlayPitched(mEatPitch,mScale*aVolume);
				if (mEatPitch==1.0f) mEatPitch=1.25f;
				else mEatPitch=1.0f;
			}
		}
	}

	//
	// See if we're embedded in another phage
	//
	float aPhageRadius=20;
	EnumList(Phage,aP,gPhageList)
	{
		if (aP==this) continue;

		if (gMath.EllipseIntersect(aP->mPos,Point(aPhageRadius,aPhageRadius)*aP->mScale,mPos,Point(aPhageRadius,aPhageRadius)*mScale))
		{
			//
			// We're intersecting someone's personal space... move us out!
			//
			float aOffAngle=gMath.PointAtAngle2D(aP->mPos,mPos);
			mPos-=gMath.AngleToVector(aOffAngle)*.75f;
		}
	}

	{
		Rect aRect=mCollide;
		//			aRect.mX-=5;
		//			aRect.mWidth+=10;
		aRect.mX+=mPos.mX;
		aRect.mY+=mPos.mY;

		Rect aRobotRect=gWorld->mRobot->mCollide;//.Expand(ROBOTEXPAND);
		//			aRobotRect.mX-=5;
		//			aRobotRect.mWidth+=10;
		aRobotRect.mY+=5;
		aRobotRect.mHeight-=10;
		aRobotRect.mX+=gWorld->mRobot->mPos.mX;
		aRobotRect.mY+=gWorld->mRobot->mPos.mY;

		if (aRobotRect.Intersects(aRect))
		{
			gWorld->mRobot->Die(false);
			mEatScale=1.25f;
		}
	}
}

void Phage::Draw()
{
	Matrix aMat;
	aMat.Rotate2D(mAngle);
	aMat.Scale(mScale*mEatScale);
	aMat.Translate(mPos);
	gBundle_Play->mPhage[mMouth].Draw(aMat);
}

void Phage::Ouch(int theExtraDamage)
{
	mPos+=gOuchDir.Normal()*8;
}

MonsterEnergy::MonsterEnergy()
{
	mID=ID_MONSTERENERGY;
	mCollide=Rect(60,60);
}

void MonsterEnergy::Draw()
{
	gBundle_Tiles->mBlock[73].DrawScaled(mPos,1.0f+(((float)((gAppPtr->AppTime()*3)%25)/25.0f)*.2f));
}

void DripBoss::EatEnergy()
{
	if (mEnergyLevel>=3) return;

	EatEnergyAt(mGridPos.mX-1,mGridPos.mY-1);
	EatEnergyAt(mGridPos.mX,mGridPos.mY-1);
	EatEnergyAt(mGridPos.mX+1,mGridPos.mY-1);
	EatEnergyAt(mGridPos.mX-1,mGridPos.mY);
	EatEnergyAt(mGridPos.mX,mGridPos.mY);
	EatEnergyAt(mGridPos.mX+1,mGridPos.mY);
	EatEnergyAt(mGridPos.mX-1,mGridPos.mY+1);
	EatEnergyAt(mGridPos.mX,mGridPos.mY+1);
	EatEnergyAt(mGridPos.mX+1,mGridPos.mY+1);
}

void RedBoss::EatEnergy()
{
	if (mEnergyLevel>=3) return;

	EatEnergyAt(mGridPos.mX-1,mGridPos.mY);
	EatEnergyAt(mGridPos.mX+1,mGridPos.mY);
	EatEnergyAt(mGridPos.mX-1,mGridPos.mY-1);
	EatEnergyAt(mGridPos.mX,mGridPos.mY-1);
	EatEnergyAt(mGridPos.mX+1,mGridPos.mY-1);
}

void Boss::EatEnergy()
{
	if (mEnergyLevel>=3) return;

	EatEnergyAt(mGridPos.mX-1,mGridPos.mY);
	EatEnergyAt(mGridPos.mX+1,mGridPos.mY);
	EatEnergyAt(mGridPos.mX-1,mGridPos.mY-1);
	EatEnergyAt(mGridPos.mX,mGridPos.mY-1);
	EatEnergyAt(mGridPos.mX+1,mGridPos.mY-1);
}
