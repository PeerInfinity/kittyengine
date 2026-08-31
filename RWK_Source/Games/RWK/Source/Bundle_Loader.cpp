
#include "Bundle_Loader.h"
Bundle_Loader* gBundle_Loader=NULL;

void Bundle_Loader::Load()
{
	OverrideTextureSize();
	if (IsRetina("Loader")) {if (!SpriteBundle::Load("Loader@2X")) return;LoadData("Loader@2X");}
	else {if (!SpriteBundle::Load("Loader")) return;LoadData("Loader");}
	
	// Begin Bundler Automatic Code
	int aSCount=0;
	mFull.ManualLoad(GetTextureRef(0),*mData);
	mHLogo.ManualLoad(GetTextureRef(0),*mData);
	mHURL.ManualLoad(GetTextureRef(0),*mData);
	mLoadbar.ManualLoad(GetTextureRef(0),*mData);
	mLogo.ManualLoad(GetTextureRef(0),*mData);
	mRURL.ManualLoad(GetTextureRef(0),*mData);
	mFillrect.ManualLoad(GetTextureRef(0),*mData);

	mFont_Text.ManualLoad(GetTextureRef(0),*mData);
	// End Bundler Automatic Code

	gG.RegisterFillrect(&mFillrect);
	UnloadData();
}

