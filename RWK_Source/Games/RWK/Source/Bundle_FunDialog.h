
#pragma once
#include "rapt.h"
#include "Dialog.h"
#include "Bundle_Sounds.h"


class Bundle_FunDialog;
extern Bundle_FunDialog* gBundle_FunDialog;

class Bundle_FunDialog : public DialogWidgetBundle
{
public:
	Bundle_FunDialog()	{gBundle_FunDialog=this;}
	void			Load();
	
	//
	// Define any extra sprites here...
	//
	Sprite			mMenuTop_Hollow;
	Sprite			mDialogRim_Hollow;
};

