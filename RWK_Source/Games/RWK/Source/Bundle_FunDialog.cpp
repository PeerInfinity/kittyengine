
#include "Bundle_FunDialog.h"
Bundle_FunDialog* gBundle_FunDialog=NULL;

void Bundle_FunDialog::Load()
{
	//
	// Tweak font placement for taste or font weirdness
	// in text boxes...
	//
	mTextBoxBorder=Point(8,5);		
	mTextBoxCursorOffset=Point(0,0);
	mTextBoxCursorWidth=3;
	mTextBoxTextOffset=Point(1,1);
	mTextBoxColor=mKeyGrabberColor=IColor(255,255,255);
	mKeyGrabberTextOffset=Point(0,1);
	
	//
	// Panel area... x1,y1 is where it starts, Width/Height-x2,y2 is where it ends.
	//
	float aExtraX=30;
	float aExtraY=21;
	//mPanelArea=AABBRect(20+aExtraX,75+aExtraY,-(40+(aExtraX*2)),-((75*2)+(aExtraY*2)));
	mPanelArea=AABBRect(20+aExtraX,75+aExtraY,-(40+(aExtraX*2)),-(100+aExtraY*2));	// If you want fully to bottom
	
	mNormalColor=IColor(255,255,255);	// Color of normal text in controls
	mTitleColor=IColor(255,255,255);	// Color of the title headers
	mGroupHeaderColor=Color(255,255,255);	// Color of group headers
	mClickableColor=IColor(200,255,200);	// Color of stuff we can click
	mWidgetBorder=Point(10,10);				// How much to pad the widgets themselves (like border around the text and graphics)
	mVerticalPadding=14;					// How much space to pad the top and bottom of the scrolly area with
	
	mBackgroundIs4Corner=true;				// Means the background graphic supplies all four corners
	
	mDoneButtonOffset=Point(20,-10+10);
	mHeaderTextOffset=Point(0,0+5);
	mBackButtonOffset=Point(20,-0+5);

	
	mClickSound=&gSounds->mClick;
	
	
	OverrideTextureSize();
	if (IsRetina("FunDialog")) {if (!SpriteBundle::Load("FunDialog@2X")) return;LoadData("FunDialog@2X");}
	else {if (!SpriteBundle::Load("FunDialog")) return;LoadData("FunDialog");}
	



	
	// Begin Bundler Automatic Code
	int aSCount=0;
	mBKGCorner.ManualLoad(GetTextureRef(0),*mData);
	mChoiceArrow.ManualLoad(GetTextureRef(0),*mData);
	mDialogRim_Hollow.ManualLoad(GetTextureRef(0),*mData);
	mGroupboxDivider.ManualLoad(GetTextureRef(0),*mData);
	mGroupbox.ManualLoad(GetTextureRef(0),*mData);
	mKeygrabberBKG.ManualLoad(GetTextureRef(0),*mData);
	mMenuTop_Hollow.ManualLoad(GetTextureRef(0),*mData);
	mSliderBKG.ManualLoad(GetTextureRef(0),*mData);
	mTextboxBKG.ManualLoad(GetTextureRef(0),*mData);
	mTouchIcon.ManualLoad(GetTextureRef(0),*mData);
	mFillrect.ManualLoad(GetTextureRef(0),*mData);

	mBackButton.GuaranteeSize(2);for (aSCount=0;aSCount<2;aSCount++) mBackButton[aSCount].ManualLoad(GetTextureRef(0),*mData);
	mCheckbox.GuaranteeSize(2);for (aSCount=0;aSCount<2;aSCount++) mCheckbox[aSCount].ManualLoad(GetTextureRef(0),*mData);
	mDoneButton.GuaranteeSize(2);for (aSCount=0;aSCount<2;aSCount++) mDoneButton[aSCount].ManualLoad(GetTextureRef(0),*mData);
	mSliderThumb.GuaranteeSize(2);for (aSCount=0;aSCount<2;aSCount++) mSliderThumb[aSCount].ManualLoad(GetTextureRef(0),*mData);
	mFont_Header.ManualLoad(GetTextureRef(0),*mData);
	mFont_Normal.ManualLoad(GetTextureRef(0),*mData);
	mFont_GroupHeader.ManualLoad(GetTextureRef(0),*mData);
	// End Bundler Automatic Code

	gG.RegisterFillrect(&mFillrect);
	UnloadData();

	//
	// Font tweak for our dialog stuff...
	//
	mFont_Normal.TweakOffset(Point(0,-3));
	mFont_Header.TweakOffset(Point(0,-1));
	mFont_GroupHeader.TweakOffset(Point(0,-2));
	
	mFont_Normal.SetExtraSpacing(-3);
	mFont_Header.SetExtraSpacing(-3);
	mFont_GroupHeader.SetExtraSpacing(-3);
	mFont_Normal.SetCharacterWidth(32,mFont_Normal.GetCharacter(32)->mWidth-5);
	mFont_Header.SetCharacterWidth(32,mFont_Header.GetCharacter(32)->mWidth-5);
	mFont_GroupHeader.SetCharacterWidth(32,mFont_GroupHeader.GetCharacter(32)->mWidth-5);

}

