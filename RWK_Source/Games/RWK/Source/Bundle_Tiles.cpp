
#include "Bundle_Tiles.h"
Bundle_Tiles* gBundle_Tiles=NULL;

void Bundle_Tiles::Load()
{
	OverrideTextureSize();
	if (IsRetina("Tiles")) {if (!SpriteBundle::Load("Tiles@2X")) return;LoadData("Tiles@2X");}
	else {if (!SpriteBundle::Load("Tiles")) return;LoadData("Tiles");}
	
	// Begin Bundler Automatic Code
	int aSCount=0;
	mConveyorBottom.ManualLoad(GetTextureRef(0),*mData);
	mConveyorTop.ManualLoad(GetTextureRef(0),*mData);
	mConveyor.ManualLoad(GetTextureRef(0),*mData);
	mDeleteapp.ManualLoad(GetTextureRef(0),*mData);
	mForceField.ManualLoad(GetTextureRef(0),*mData);
	mZoingertop.ManualLoad(GetTextureRef(0),*mData);
	mZoinger.ManualLoad(GetTextureRef(0),*mData);
	mFillrect.ManualLoad(GetTextureRef(0),*mData);

	mBKG.GuaranteeSize(1);mBKG[0].ManualLoad(GetTextureRef(0),*mData);
	mBlock.GuaranteeSize(74);for (aSCount=0;aSCount<74;aSCount++) mBlock[aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint.GuaranteeSize(10);
	mPaint[0].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[0][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[1].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[1][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[2].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[2][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[3].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[3][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[4].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[4][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[5].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[5][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[6].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[6][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[7].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[7][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[8].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[8][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPaint[9].GuaranteeSize(47);for (aSCount=0;aSCount<47;aSCount++) mPaint[9][aSCount].ManualLoad(GetTextureRef(0),*mData);
	mPipes.GuaranteeSize(11);for (aSCount=0;aSCount<11;aSCount++) mPipes[aSCount].ManualLoad(GetTextureRef(0),*mData);
	// End Bundler Automatic Code


	for (int aTileType=0;aTileType<10;aTileType++)
	{
		for (int aTile=0;aTile<mPaint[aTileType].Size();aTile++)
		{
			mTile_Paint.Add(&mPaint[aTileType][aTile]);
		}
	}

	gG.RegisterFillrect(&mFillrect);
	UnloadData();
}

