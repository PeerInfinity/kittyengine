
#pragma once
#include "rapt.h"


class Bundle_Tiles;
extern Bundle_Tiles* gBundle_Tiles;

class Bundle_Tiles : public SpriteBundle
{
public:
	Bundle_Tiles()	{gBundle_Tiles=this;}
	void			Load();
		
public:

	// Begin Bundler Automatic Code
	Sprite				mConveyorBottom;
	Sprite				mConveyorTop;
	Sprite				mConveyor;
	Sprite				mDeleteapp;
	Sprite				mForceField;
	Sprite				mZoingertop;
	Sprite				mZoinger;
	Sprite				mFillrect;

	Array<Sprite>			mBKG;
	Array<Sprite>			mBlock;
	Array<Array<Sprite> >	mPaint;
	Array<Sprite>			mPipes;
	// End Bundler Automatic Code


	Array<Sprite*>		mTile_Paint;

};

