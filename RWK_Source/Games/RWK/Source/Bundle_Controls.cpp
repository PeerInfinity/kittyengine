
#include "Bundle_Controls.h"
Bundle_Controls* gBundle_Controls=NULL;

void Bundle_Controls::Load()
{
	OverrideTextureSize();
	if (IsRetina("Controls")) {if (!SpriteBundle::Load("Controls@2X")) return;LoadData("Controls@2X");}
	else {if (!SpriteBundle::Load("Controls")) return;LoadData("Controls");}
	
	// Begin Bundler Automatic Code
	int aSCount=0;
	mArrows.ManualLoad(GetTextureRef(0),*mData);
	mWASD.ManualLoad(GetTextureRef(0),*mData);
	mFillrect.ManualLoad(GetTextureRef(0),*mData);

	// End Bundler Automatic Code

	gG.RegisterFillrect(&mFillrect);
	UnloadData();
}

