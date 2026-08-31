
#pragma once
#include "rapt.h"

class Bundle_Controls;
extern Bundle_Controls* gBundle_Controls;

class Bundle_Controls : public SpriteBundle
{
public:
	Bundle_Controls()	{gBundle_Controls=this;}
	void			Load();
		
public:

	// Begin Bundler Automatic Code
	Sprite				mArrows;
	Sprite				mWASD;
	Sprite				mFillrect;

	// End Bundler Automatic Code

};

