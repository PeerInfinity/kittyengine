
#pragma once
#include "rapt.h"


class Bundle_Loader;
extern Bundle_Loader* gBundle_Loader;

class Bundle_Loader : public SpriteBundle
{
public:
	Bundle_Loader()	{gBundle_Loader=this;}
	void			Load();
		
public:

	// Begin Bundler Automatic Code
	Sprite				mFull;
	Sprite				mHLogo;
	Sprite				mHURL;
	Sprite				mLoadbar;
	Sprite				mLogo;
	Sprite				mRURL;
	Sprite				mFillrect;

	Font				mFont_Text;
	// End Bundler Automatic Code

};

