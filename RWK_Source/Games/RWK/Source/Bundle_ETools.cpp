
#include "Bundle_ETools.h"
Bundle_ETools* gBundle_ETools=NULL;

void Bundle_ETools::Load()
{
	OverrideTextureSize();
	if (IsRetina("ETools")) {if (!SpriteBundle::Load("ETools@2X")) return;LoadData("ETools@2X");}
	else {if (!SpriteBundle::Load("ETools")) return;LoadData("ETools");}
	
	// Begin Bundler Automatic Code
	int aSCount=0;
	mAutofix.ManualLoad(GetTextureRef(0),*mData);
	mAutofix_Check.ManualLoad(GetTextureRef(0),*mData);
	mBack.ManualLoad(GetTextureRef(0),*mData);
	mBrowse.ManualLoad(GetTextureRef(0),*mData);
	mBrowse_Cancel.ManualLoad(GetTextureRef(0),*mData);
	mBrowse_Delete.ManualLoad(GetTextureRef(0),*mData);
	mCancel.ManualLoad(GetTextureRef(0),*mData);
	mEdit.ManualLoad(GetTextureRef(0),*mData);
	mEditCursor.ManualLoad(GetTextureRef(0),*mData);
	mEraser.ManualLoad(GetTextureRef(0),*mData);
	mFilecaret.ManualLoad(GetTextureRef(0),*mData);
	mFileicon.ManualLoad(GetTextureRef(0),*mData);
	mLayout.ManualLoad(GetTextureRef(0),*mData);
	mPaint.ManualLoad(GetTextureRef(0),*mData);
	mProcessing.ManualLoad(GetTextureRef(0),*mData);
	mSave.ManualLoad(GetTextureRef(0),*mData);
	mSave_Editbox.ManualLoad(GetTextureRef(0),*mData);
	mSave_Levelname.ManualLoad(GetTextureRef(0),*mData);
	mSave_Save.ManualLoad(GetTextureRef(0),*mData);
	mSelected_Layout.ManualLoad(GetTextureRef(0),*mData);
	mSelected_Paint.ManualLoad(GetTextureRef(0),*mData);
	mSettings.ManualLoad(GetTextureRef(0),*mData);
	mShare.ManualLoad(GetTextureRef(0),*mData);
	mTagCorner.ManualLoad(GetTextureRef(0),*mData);
	mTest.ManualLoad(GetTextureRef(0),*mData);
	mThumb.ManualLoad(GetTextureRef(0),*mData);
	mThumbpad.ManualLoad(GetTextureRef(0),*mData);
	mTrash.ManualLoad(GetTextureRef(0),*mData);
	mFillrect.ManualLoad(GetTextureRef(0),*mData);

	mFiletool.GuaranteeSize(5);for (aSCount=0;aSCount<5;aSCount++) mFiletool[aSCount].ManualLoad(GetTextureRef(0),*mData);
	// End Bundler Automatic Code

	gG.RegisterFillrect(&mFillrect);
	UnloadData();
}

