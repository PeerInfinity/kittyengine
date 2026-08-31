
#pragma once
#include "rapt.h"


class Bundle_ETools;
extern Bundle_ETools* gBundle_ETools;

class Bundle_ETools : public SpriteBundle
{
public:
	Bundle_ETools()	{gBundle_ETools=this;}
	void			Load();
		
public:

	// Begin Bundler Automatic Code
	Sprite				mAutofix;
	Sprite				mAutofix_Check;
	Sprite				mBack;
	Sprite				mBrowse;
	Sprite				mBrowse_Cancel;
	Sprite				mBrowse_Delete;
	Sprite				mCancel;
	Sprite				mEdit;
	Sprite				mEditCursor;
	Sprite				mEraser;
	Sprite				mFilecaret;
	Sprite				mFileicon;
	Sprite				mLayout;
	Sprite				mPaint;
	Sprite				mProcessing;
	Sprite				mSave;
	Sprite				mSave_Editbox;
	Sprite				mSave_Levelname;
	Sprite				mSave_Save;
	Sprite				mSelected_Layout;
	Sprite				mSelected_Paint;
	Sprite				mSettings;
	Sprite				mShare;
	Sprite				mTagCorner;
	Sprite				mTest;
	Sprite				mThumb;
	Sprite				mThumbpad;
	Sprite				mTrash;
	Sprite				mFillrect;

	Array<Sprite>		mFiletool;
	// End Bundler Automatic Code

};

