#pragma once
#include "rapt.h"

//
// If you don't have or want a magiccursor, comment this out.
//
//#include "MagicCursor.h"


//
// How to:
// Dialog* aDialog=new Dialog;
// aDialog->Size(whatever);
// aDialog->SetTitle(whatever);				// Sets the dialog title
// aDialog->SetFont(whatever);				// Sets the default font
// aDialog->SetTitleFont(whatever);			// Sets the title font
// aDialog->SetFontColor(whatever);			// Sets the default font color
// aDialog->SetTitleFontColor(whatever);	// Sets the title font color
// aDialog->SetClickableColor(whatever);	// Sets the default clickable font color
// aDialog->SetWidgetBorder(whatever);		// Sets the border that added widgets will get
// aDialog->SetVPadding(whatever);			// Gives V padding to the top and bottom of controls you add, which just makes it look a little nice
//
// DialogPanel* aPanel=aDialog->MakeNewPanel();
// aPanel->Add(new DialogStatic("This is a test"));
//
// For dialogs, override:
// Draw();				// Draws the background...
// DrawGroup();			// Draws a group rect (so you can do rounded rect, etc)
// DrawText();			// Draws text, so you can tweak positions, etc
// SetupButtons();		// Picks where/how to size buttons (they have default places, so you can play with that)
// ClickControl();		// For whenever they click something that's not already handled
// ChangedControl();	// If the hook value of a control changes
//

//
// Standard Controls:
// DialogStatic
// DialogSeperator
// DialogCheckbox
// DialogTextbox
// DialogList
// DialogButton
// DialogGroupHeader
// DialogNextPanel
//

enum
{
	DWIDGET_STATIC=100000,
	DWIDGET_SEPERATOR,
	DWIDGET_CHECKBOX,
	DWIDGET_TEXTBOX,
	DWIDGET_LIST,
	DWIDGET_BUTTON,
	DWIDGET_GROUPHEADER,
	DWIDGET_NEXTPANEL,
	DWIDGET_NUMBERBOX,
	DWIDGET_SLIDER,
	DWIDGET_KEYGRABBER,
	DWIDGET_JOYSTICKGRABBER,
};

enum
{
	DIALOGFLAG_DRAWLINE=0x01,	// Draw a line under the control, as a little seperator
};

enum
{
	DIALOGSOUND_TOUCHCHECKBOX=0,
	DIALOGSOUND_CHOOSE,
	DIALOGSOUND_TOUCHBUTTON,

};

class DialogWidgetBundle : public SpriteBundle
{
public:
	DialogWidgetBundle()
	{
		mTextBoxCursorWidth=1;
		mVerticalPadding=0;
		mBackgroundIs4Corner=false;
		mClickSound=NULL;
	}

	Sprite				mBKGCorner;
	Sprite				mChoiceArrow;
	Sprite				mGroupboxDivider;
	Sprite				mGroupbox;
	Sprite				mTextboxBKG;
	Sprite				mNumberboxBKG;
	Sprite				mFillrect;
	Sprite				mSliderBKG;
	Sprite				mKeygrabberBKG;
	Sprite				mTouchIcon;
	Array<Sprite>		mSliderThumb;

	Array<Sprite>		mBackButton;
	Array<Sprite>		mCheckbox;
	Array<Sprite>		mDoneButton;
	Array<Sprite>		mTweak;
	Font				mFont_Header;
	Font				mFont_Normal;
	Font				mFont_GroupHeader;

public:

	bool			mBackgroundIs4Corner;

	Point			mTextBoxOffset;
	Point			mTextBoxBorder;
	Point			mTextBoxCursorOffset;
	Point			mTextBoxTextOffset;
	float			mTextBoxCursorWidth;
	Color			mTextBoxColor;

	AABBRect		mPanelArea;
	AABBRect		mTinyPanelArea;

	Color			mNormalColor;
	Color			mTitleColor;
	Color			mClickableColor;
	Color			mGroupHeaderColor;

	Point			mKeyGrabberTextOffset;
	Color			mKeyGrabberColor;

	Point			mWidgetBorder;
	float			mVerticalPadding;

	Point			mDoneButtonOffset;
	Point			mHeaderTextOffset;
	Point			mBackButtonOffset;

	Sound*			mClickSound;
};


class Dialog;
class DialogWidget;

#ifdef _MAGICCURSOR
class DialogMagicCursor : public MagicCursor
{
public:
	void			SelectChanged();
	void			Draw();

	DialogWidget*	mParent;
};
#endif

class DialogPanel : public SwipeBox
{
public:
	DialogPanel();
	virtual ~DialogPanel();

	void				Core_Draw();
	void				Initialize() {Rehup();}
	virtual void		Rehup();
	void				Reset();

	void				Draw();
	DialogWidget*		Add(DialogWidget* theWidget);
	inline void			MakeCurrent();

	bool				mFirstFocus;
	void				Focus();
	void				Unfocus();

	Dialog*				mParent;
	Point				mCursor;
	float				mCurrentHeight;

	List				mWidgetList;

	RectComplex			mGroupRect;

	String				mName;
	bool				mNoDraw;

#ifdef _MAGICCURSOR
	DialogMagicCursor	mMCursor;
#endif
};

class DialogWidget : public CPU
{
public:
	DialogWidget() {mParent=NULL;mFlag=0;mIsDown=false;mFont=NULL;mColor.mA=-1;SetCPUName(this,"DialogWidget");}

	//
	// Special note:  In order to make the border "transparent" so I don't need to worry about it in the future for draws,
	// an overly complex system has been set up, where children of dialogwidgets get moved, and the draw is gimped so that
	// everything draws a little bit offset, within the border.
	//
	// Note that the "right" way to have done this, for future frameworks, is to give CPUs an optional click vs draw range.
	//
	CPU*				HitTest(float x, float y);
	void				Core_Draw();

	DialogPanel*		mParent;
	virtual bool		IsGrouped() {return true;}
	inline void			MakeCurrent();
	void				TouchStart(int x, int y);
	void				TouchMove(int x, int y);
	void				TouchEnd(int x, int y);

	bool				mIsDown;
	char				mFlag;

	//
	// Universal... these get turned into defaults if not explicitely set.
	//
	String				mName;
	Font*				mFont;
	Color				mColor;

	Point				mTextOffset;

	void				FixAttributes();

};

class Dialog : public CPU
{
public:
	Dialog();
	virtual ~Dialog();

	void				Initialize();
	void				Update();
	void				Core_Draw();
	void				MakeCurrent();		// Makes the current variables match us
	void				Notify(void* theData);
	void				Back();

	inline void			SetPanelArea(float x, float y, float width, float height) {mPanelArea=Rect(x,y,width,height);}
	inline void			SetPanelArea(Rect theRect) {SetPanelArea(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	DialogPanel*		MakeNewPanel(String theName, bool mainPanel=true);

	inline void			SetTitle(String theName) {mTitle=theName;}
	inline void			SetFont(Font* theFont) {mFont=theFont;}
	inline void			SetFontColor(Color theColor) {mFontColor=theColor;}
	inline void			SetTitleFont(Font* theFont) {mTitleFont=theFont;}
	inline void			SetTitleFontColor(Color theColor) {mTitleFontColor=theColor;}
	inline void			SetClickableColor(Color theColor) {mClickableColor=theColor;}
	inline void			SetWidgetBorder(Point theSize) {mWidgetBorder=theSize;}
	inline void			SetVPadding(float thePadding) {mVPadding=thePadding;}

	void				SetBundle(DialogWidgetBundle* theBundle);

	inline void			Tiny() 
	{
		SetPanelArea(mBundle->mTinyPanelArea.mX1,mBundle->mTinyPanelArea.mY1,mWidth+mBundle->mTinyPanelArea.mX2,mHeight+mBundle->mTinyPanelArea.mY2);
		mTiny=true;
	}

public:
	//
	// Must override!
	//
	virtual void		Draw();
	virtual void		DrawTitle();	// Draws the title, for if you want to tweak that...
	virtual void		DrawOverlay();
	virtual void		DrawText(Font* theFont, String theText, Point thePos=Point(0,0)); // Optional, but you'll need it because the fonts are so wonky about their height.
	virtual void		SetupButtons();
	virtual bool		Done() {return true;}	// Gets called when you click done... optional
	virtual void		ClickControl(DialogWidget* theWidget) {}	// When you click a control
	virtual void		ChangedControl(DialogWidget* theWidget) {}	// When a value gets changed, like for a listbox

public:
	bool				mTiny;
	Rect				mPanelArea;
	Font*				mFont;
	Font*				mTitleFont;
	Color				mFontColor;
	Color				mTitleFontColor;
	Color				mClickableColor;
	Point				mWidgetBorder; // Default space around the contents of a widget
	float				mVPadding;
	String				mTitle;

	DialogWidgetBundle*	mBundle;

	Button				mDone;		// The done button, just closes the dialog
	Button				mBack;		// The back button

	float				mScrollSpeed;

public:
	//
	// For moving panels around
	//
	Stack<DialogPanel*>	mPanelStack;
	DialogPanel*		mCurrentPanel;
	DialogPanel*		mNextPanel;
	List				mPanelList;

	virtual void		NextPanel(DialogPanel* thePanel);
	virtual void		PreviousPanel();
	char				mMovePanel;	// Direction we're moving in
	float				mMovePanelProgress;

	bool				mProcessedNotify;	// Lets us know if the main dialog processed the notify.
	bool				mDisableMagicCursor;	// Lets us disable magic cursors if we need to

	bool				mNoKill;			// If Dialog should never kill itself (like if you're killing it with a transition)

};

class DialogStatic : public DialogWidget
{
public:
	DialogStatic(String theText) {mID=DWIDGET_STATIC;mName=theText;mSprite=NULL;}
	DialogStatic(Sprite* theSprite,String theText) {mID=DWIDGET_STATIC;mName=theText;mSprite=theSprite;}
	void				Initialize();
	void				Draw();

	Sprite*				mSprite;
};

class DialogSeperator : public DialogWidget
{
public:
	DialogSeperator(float theHeight) {mID=DWIDGET_SEPERATOR;mHeight=theHeight;DisableTouch();}
	virtual bool		IsGrouped() {return false;}
};

class DialogCheckbox : public DialogWidget
{
public:
	DialogCheckbox(String theText, bool* theHook) {mID=DWIDGET_CHECKBOX;mName=theText;mHook=theHook;}
	void				Initialize();
	void				Draw();
	void				TouchEnd(int x, int y);

public:
	bool*				mHook;
};

class DialogCheckboxInverted : public DialogCheckbox
{
public:
	DialogCheckboxInverted(String theText, bool* theHook) : DialogCheckbox(theText,theHook) {}
	void				Draw();
};

class DialogTextbox : public DialogWidget
{
public:
	DialogTextbox(String theText, String* theHook)
	{
		mID=DWIDGET_TEXTBOX;
		mName=theText;
		mHook=theHook;
	}

	void				Initialize();
	void				Draw();
	void				Notify(void* theData);
public:
	String*				mHook;
	TextBox				mTextBox;
};

class DialogNumberbox : public DialogWidget
{
public:
	DialogNumberbox(String theText, int* theHook, int aMin=0, int aMax=999999999, String fitNumber="999999999")
	{
		mID=DWIDGET_NUMBERBOX;
		mName=theText;
		mHook=theHook;
		mFitNumber=fitNumber;
		mMin=aMin;
		mMax=aMax;
	}

	void				Initialize();
	void				Draw();
	void				Notify(void* theData);

public:
	int*				mHook;
	String				mFitNumber;
	TextBox				mTextBox;
	Button				mTweakUp;
	Button				mTweakDown;

	int					mMin;
	int					mMax;
};

class DialogSlider : public DialogWidget
{
public:
	DialogSlider(String theText, float* theHook)
	{
		mID=DWIDGET_SLIDER;
		mName=theText;
		mHook=theHook;
	}

public:
	float*				mHook;
	Slider				mSlider;

	void				Initialize();
	void				Draw();
	void				Notify(void* theData);
};

class DialogList : public DialogWidget
{
public:
	DialogList(String theText, char* theHook, String theChoices)
	{
		mID=DWIDGET_LIST;
		mName=theText;
		mHook=theHook;
		String aChoice=theChoices.GetToken('|');
		int aStepValue=0;
		while (aChoice.Len()) 
		{
			String aText=aChoice.GetSegmentBefore('=');
			String aValue=aChoice.GetSegmentAfter('=');

			if (aValue.Len()) aStepValue=aValue.ToInt();

			Choice& aC=mChoiceList.AddLast();
			aC.mText=aText;
			aC.mValue=aStepValue;

			aChoice=theChoices.GetNextToken('|');
			aStepValue++;
		}
	};

	void				Initialize();
	void				Draw();
	void				TouchEnd(int x, int y);
	void				Notify(void* theData);

public:
	char*				mHook;
	struct Choice
	{
		String			mText;
		int				mValue;
	};
	Array<Choice>		mChoiceList;
	Array<int>			mChoiceListRef;
	DialogPanel*		mPanel;
};

class DialogButton : public DialogWidget
{
public:
	DialogButton(String theText, bool isIcon=false, CPU* theNotify=NULL)
	{
		mID=DWIDGET_BUTTON;
		mName=theText;
		mNotify=theNotify;
		mIcon=isIcon;
	}
	void				Initialize();
	void				Draw();

	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);
public:
	bool				mIcon;
};

class DialogKeyGrabber : public DialogWidget
{
public:
	DialogKeyGrabber(String theText, int *theHook)
	{
		mID=DWIDGET_KEYGRABBER;
		mName=theText;
		mHook=theHook;
		mGrabbing=false;
	}
	void				Initialize();
	void				Draw();

	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);

public:
	int*				mHook;
	bool				mGrabbing;
};

class DialogJoystickButtonGrabber : public DialogWidget
{
public:
	DialogJoystickButtonGrabber(String theText, int *theHook)
	{
		mID=DWIDGET_JOYSTICKGRABBER;
		mName=theText;
		mHook=theHook;
		mGrabbing=false;
	}
	void				Initialize();
	void				Draw();

	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);

public:
	int*				mHook;
	bool				mGrabbing;
};

class DialogNextPanel : public DialogWidget
{
public:
	DialogNextPanel(String theText, String thePanelText, DialogPanel* thePanel)
	{
		mID=DWIDGET_NEXTPANEL;
		mName=theText;
		mPanelText=thePanelText;
		mPanel=thePanel;
	}
	void				Initialize();
	void				Draw();

	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);
public:
	DialogPanel*		mPanel;
	String				mPanelText;
};


class DialogGroupHeader : public DialogWidget
{
public:
	DialogGroupHeader(String theName, bool canClose=false) {mID=DWIDGET_GROUPHEADER;mName=theName;if (!canClose) DisableTouch();}
	DialogGroupHeader(String theName, Font* theFont, Color theColor=Color(-1,-1,-1,-1), bool canClose=false) {mID=DWIDGET_GROUPHEADER;mName=theName;mFont=theFont;mColor=theColor;if (!canClose) DisableTouch();}
	virtual bool		IsGrouped() {return false;}

	void				Initialize();
	void				Draw();

public:
};

class DialogGroupSpacer : public DialogWidget
{
public:
	DialogGroupSpacer(float theSpace) {mSpace=theSpace;}

	virtual bool		IsGrouped() {return false;}
	void				Initialize() {mHeight=mSpace;}
	void				Draw() {}

	float				mSpace;

};


Font* CurrentDialogFont();
Color CurrentDialogFontColor();
Dialog* CurrentDialog();
Point CurrentWidgetBorder();
inline void DialogPanel::MakeCurrent() {mParent->MakeCurrent();}
inline void	DialogWidget::MakeCurrent() {mParent->MakeCurrent();}
DialogWidgetBundle& CurrentBundle();
void SetDialogDragMargin(int theAmount);
