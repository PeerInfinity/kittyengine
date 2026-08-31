#pragma  once
#include "rapt.h"

#define NOTIFY_LINK		0xABCDABCD

//
// Markup Language:
//
// <br>					Line break
//
// <cr>					Carriage return-- return to leftmost position without scrolling down
//
// <lf>					Line Feed-- go to next line without going to start of line
//
// <-- -->				Between these are comments
//
// <link where>			Designates a link that should be opened locally inside the mlbox.
// </link>
//
// <weblink where>		Makes a link that will open up a browser page with the contents
// </weblink>
//
// <linkfromrecord		Makes a link out of a recorded area.
//	 record linkcmd>	
//
//
// <version ver>		Sets the webpage "version."  This forces re-downloads of cached objects.
//
// <space x>			Moves the cursor forward x pixels to provide spacing
// <sp x>				Abbreviation for space
//
// <movecursor x,y>		Moves the cursor to x,y.
//						The following variations are supported:
//						<movecursor +x,-y> move relative to current position
//						<movecursor x%,y%> move to a percent (y% will be percent of total document drawn so far)
//						<movecursor +x%,-y%> move a relative percent of the document
//						<movecursor bottom> moves the cursor to the next line on the bottom of the document
//						<movecursor end> moves the cursor to the rightmost location on the lowest line of the document
//
// <pushcursor>			Pushes the cursor location onto a stack
//
// <popcursor>			Pops the cursor location off the stack
//
// <confine start end>	Confines all drawing to a certain horizontal part of the page.  Basically, this how you make "tables"
//						Example usage:
//						<confine 100 200> Confines from pixel pos 100 to pixel pos 200
//						<confine 0% 50%> Confines to half the canvas
//						<confine 0% 50% border=5> Confines to half the canvas with a border to keep stuff from touching
//						End with </confine> to return to the previous canvas
//
//						Example to make a 'table':
//						<confine 0 50%>This goes in the left half of the table</confine>
//						<confine 50% 100%>This goes in the right half of the table</confine>
//
// <color>				Set text color, following formats work: <color 1,1,1> <color red> <color #00FF00>
//
// <bkgcolor>			Sets the BKG color.  Totally async, never gets into the pipeline, just sets the color immediately.
//
// <center>				Centers text in the constrained area
//
// <left>				Left-align text in the constrained area
//
// <right>				Right-align text in the constrained area
//
// <font name>			Set the font.  The font is the name you used when adding it to the MLBox with AddFont.  
//						<font myfont1>
//
// <bundle name url>	Downloads a dyna bundle from the server.  The URL should be the image file, and it will find the .dyna
//						file automatically, in the same place.
//						<bundle mybundle "www.site.com/bundles/mybundle.png">
//						Note that it expects to also find "www.site.com/bundles/mybundle.dyna" in this case.
//						In this case, "mybundle" is the name you would use in image loads.
//
//						Note: For special cases, you CAN specify the dyna of the bundle explicitely, like so:
//						<bundle name url_of_image url_of_dyna>
//						But if url_of_dyna is not specified, it will simply use the image name to find the corresponding
//						file with a .dyna extension.
//
// <img url offset=xoff,yoff		Sets an image.  The url can either be an image you've added to the MLBox with AddIMG, or it can
//		scale=x>					be a direct web url.  It can also be a bundle that was specified earlier in the ML, like so:
//									<bundle bundlename url>
//									<img bundlename:SpriteName> (spritename is the FILENAME of the sprite that was put into the bundle... no "m" in front of it!)
//									<img myimage offset=5,5> (Draws it offset 5,5 for tweaking)
//									<img myimage off=5,5> (Draws it offset 5,5 for tweaking)
//									<img myimage scale=.5>
//
// <setup>				Does some page setup:
//						<setup padbottom=5>
//
//							Setup Options:
//							padbottom=x			Adds x pixels to the bottom of the page (for cleaner scrolling)
//							padtop=x			Adds x pixels to the top of the page
//							padleft=x			Adds x pixels to the left of the page
//							padright=x			Adds x pixels to the right of the page
//							borders=x			Pads all borders
//							vborders=x			Pads top and bottom
//							hborders=x			Pads left and right
//							caption=x			Caption... you need to set this yourself, it's just some flag.
//
// <click>				Puts the word "click" or "touch" into the text, depending on whether it's a touch device or a click device.
//						The capitalization of the tagged word will be duplicated in the actual text.
//
// <fillline h>			Fills a line at the cursor in current color.
//						Put this in FRONT of text.
//						<fillline 10> fills a line 10 pixels high.
//
// <custom width,height name>	Renders custom stuff...width/height used for sizing, name used for the call to drawcustom
//
// <record name></record>	Records a union of everything within the record tags for later use via Rect GetRecord("name")
//
// <customfromrecord name>	Makes a custom image from a record (happens in a second pass, so that you can put it in from of the record.
//							but note, if you do that, it will not be able to size itself correctly)
// 
// <flag name=value>	Sets a flag in the class itself that you can reference later
//
// <textbox				Creates a textbox
//		id=name
//		default=text
//	 >	
//
// <checkbox off_image on_image id=whatever> Creates a checkbox
//
// <null>				Creates a nothing thing that nonetheless takes up space (used to make sure records, etc, don't
//						skip over <br>, or to flesh out link sizes.
//
// <os name>			Everything between the braces will only be noticed if the name matches the os.
//						Example: <os android>Google Play!</os><os ios>App Store!</os>
//


enum
{
	ML_NULL=0,
	ML_PAGE,
	ML_TEXT,
	ML_LINEBREAK,		// Just a placeholder to help us "stop" when we format backwards...
	ML_COLOR,
	ML_ALIGN,
	ML_IMAGE,
	ML_CR,				// Carriage return
	ML_LF,				// Line Feed
	ML_MOVECURSOR,		// Moves the cursor a set amount
	ML_PUSHCURSOR,	//10
	ML_POPCURSOR,
	ML_CONFINE,
	ML_ENDCONFINE,
	ML_LINK,
	ML_ENDLINK,		//15
	ML_FILLLINE,
	ML_SPACE,
	ML_RECORD,			// Recording...
	ML_TEXTBOX,
	ML_CHECKBOX,
	ML_AD,
};

enum
{
	MLFLAG_WIDTHISPERCENT		=0x00000001,	// the width is a % of the container
	MLFLAG_EXPLICITPOS			=0x00000002,	// the xy position is explicit, don't try to "fiddle" it
	MLFLAG_CENTER				=0x00000004,	// Align center
	MLFLAG_RIGHT				=0x00000008,	// Align right (assume left if neither are set!)
	MLFLAG_NEEDSPACE			=0x00000010,	// Means it needs a space before it...
	MLFLAG_INVISIBLE			=0x00000020,	// Means it can't be seen
	MLFLAG_SETUP				=0x00000040,	// Means it has been set up
	MLFLAG_XISRELATIVE			=0x00000080,	// X Value is relative
	MLFLAG_YISRELATIVE			=0x00000100,	// Y Value is relative
	MLFLAG_XISPERCENT			=0x00000200,	// X Value is a percent of the constrained area
	MLFLAG_YISPERCENT			=0x00000400,	// Y Value is a percent of the total document drawn so far
	MLFLAG_END					=0x00000800,	// This object is indicating the end of the document, for whatever reason
};

class MLLink
{
public:
	MLLink() {mID=NULL;}
	virtual ~MLLink() {if (mID) delete mID;mID=NULL;}

	String				mCommand;	// Command from the html
	String				mRef;		// Extra ref (put in to get link from record)
	bool				mIsWebLink;	// If it's meant to open a browser
	Button*				mButton;	// The button this is linked to

	char*				mID;
	inline String		ID() {if (mID) return mID; else return "";}
};

class MLBox;
class MLObject : public Rect
{
public:
	MLObject() {mFlags=0;mType=0;mWidth=-1;mBox=NULL;mID=NULL;}
	virtual ~MLObject() {if (mID) delete mID;mID=NULL;}

	char				mType;
	int					mFlags;
	Smart(MLLink)		mLink;
	MLBox*				mBox;

	virtual void		Draw() {}
	inline bool			IsVisible() {return (!(mFlags&MLFLAG_INVISIBLE) && mWidth>=0);}
	inline bool			IsSetup() {return ((mFlags&MLFLAG_SETUP)==MLFLAG_SETUP);}
	virtual bool		IsInArea(Rect theRect) {return Intersects(theRect);}

	char*				mID;
	inline String		ID() {if (mID) return mID; else return "";}
};

class MLText : public MLObject
{
public:
	MLText() {mType=ML_TEXT;}
	String				mText;
	Font*				mFont;

	void				Draw();
};

class MLTextBox : public MLObject
{
public:
	MLTextBox() {mType=ML_TEXTBOX;}
	virtual ~MLTextBox();
	Font*				mFont;

	void				Draw();
	TextBox				mTextBox;
	String				mAllow;
};

class MLCheckBox : public MLObject
{
public:
	MLCheckBox() {mType=ML_CHECKBOX;}

	//void				Draw();
	CheckBox			mCheckBox;
	Point				mOffset;
};

class MLMoveCursor : public MLObject
{
public:
	MLMoveCursor() {mType=ML_MOVECURSOR;mFlags=MLFLAG_INVISIBLE;}
	Point			mMove;
};

class MLSpace : public MLObject
{
public:
	MLSpace() {mType=ML_SPACE;}
	float			mSpace;
};


class MLRecord : public MLObject
{
public:
	MLRecord() {mType=ML_RECORD;mStop=false;}
	bool			mStop;
	String			mName;
	longlong		mHash;
};


class MLImage : public MLObject
{
public:
	MLImage() {mType=ML_IMAGE;mSprite=NULL;mScale=1.0f;mExtra=0;mExtraData=NULL;mIsNull=false;}
	Sprite*				mSprite;	// For regular images
	String				mName;		// For custom images
	Point				mOffset;	// Offset if we specify one
	float				mScale;		// Scale of image
	float				mExtra;		// Extra area around it for visibility testing
	void*				mExtraData;	// Extra Data...
	bool				mIsNull;

	void				Draw();

	virtual bool		IsInArea(Rect theRect) {return Translate(mOffset).Expand(mExtra).Intersects(theRect);}

};

class MLAlign : public MLObject
{
public:
	MLAlign() {mType=ML_ALIGN;mAlign=-1;mFlags=MLFLAG_INVISIBLE;}
	short				mAlign;
};

class MLColor : public MLObject
{
public:
	MLColor() {mType=ML_COLOR;mFlags=MLFLAG_INVISIBLE;}
	Color				mColor;
	void				Draw() {gG.SetColor(mColor);}
};

class MLFillLine : public MLObject
{
public:
	MLFillLine() {mType=ML_FILLLINE;}
	void				Draw();

	float				mHeight;
};

struct Preloader
{
	String		mReformatString;

	int			mPending=0;
	Smart(Preloader) mSelf; // Keeps our self alive until we're ready to destroy for realz...
};

class MLBox : public SwipeBox
{
public:
	MLBox();
	virtual ~MLBox();
	void				Draw();
	void				Update();
	void				Notify(void* theData);
	void				SetLinkNotify(CPU* theCPU) {mLinkNotify=theCPU;}

	virtual void		DrawPending() {}	// Override this to draw a wait indicator while things are loading...
	virtual void		DrawBackground();	// Override this to draw the background...
	virtual void		DrawCustom(String theName, MLImage* theImage) {}	// Override this to draw custom images
	virtual bool		ClickLink(String theCommand);	// Override this to process a local link (http: links will be handle automagically)
														// Return TRUE if it's already been handled...
	virtual bool		ClickWebLink(String theCommand) {return false;} // Lets you override web linking too... typically not needed, unless you want a sound effect or something
	virtual void		Changed() {}	// After completing a reformat... use for sizing, etc.

	void				Format(String theString);
	void				FormatTag(String theTag, SmartList(MLObject)& theObjectList);

	void				Load(String theURL);
	void				LoadFromString(String theString);
	inline void			LoadString(String theString) {LoadFromString(theString);}
	inline void			LoadAsString(String theString) {LoadFromString(theString);}

	static void			LoadStub(void* theArg);

	virtual void		Exe(Array<String>& theParams) {} // Executes an <EXE> on loadup

	Smart(Preloader)	Preload(String theString);
	Smart(Preloader)	mPreloader;


public:
	String				mLayoutURL;
	String				mLayoutData;

	CPU*				mLinkNotify;

	bool				mChanged;


	void				Layout();

	struct Setup
	{
		Setup() 
		{
			Reset();
		}

		void Reset()
		{
			mPadTop=mPadBottom=mPadLeft=mPadRight=0;
			mCaption="";
		}

		float			mPadTop;
		float			mPadBottom;
		float			mPadLeft;
		float			mPadRight;
		String			mCaption;
	} mSetup;

public:
	SmartList(MLObject)	mObjectList;

	Font*					mDefaultFont;
	Font*					mCurrentFont;
	struct FontStruct
	{
		FontStruct() {mFont=NULL;}
		Font*			mFont;
		String			mName;
	};
	SmartList(FontStruct)	mFontList;
	inline void				SetDefaultFont(Font* theFont) {mDefaultFont=theFont;}
	inline void				AddFont(Font* theFont, String theName)
	{
		theFont->CopyCharacter('<',127);
		Smart(FontStruct) aF=new FontStruct;
		aF->mFont=theFont;
		aF->mName=theName;
		mFontList+=aF;
		if (!mDefaultFont) mDefaultFont=theFont;
	}

	struct IMGStruct
	{
		IMGStruct() {mSprite=NULL;mOwnSprite=false;}
		~IMGStruct() {if (mOwnSprite) delete mSprite;mSprite=NULL;}
		Sprite*			mSprite;
		String			mName;
		bool			mOwnSprite;
	};
	Sprite*					mDefaultIMG;
	SmartList(IMGStruct)	mIMGList;
	inline void				SetDefaultIMG(Sprite* theIMG) {mDefaultIMG=theIMG;}
	inline void				AddIMG(Sprite* theSprite, String theName)
	{
		Smart(IMGStruct) aI=new IMGStruct;
		aI->mSprite=theSprite;
		aI->mName=theName;
		mIMGList+=aI;
	}

	int					mThreadLock;

	bool				mStubPending;
	bool				mReadyToDisplay;

	String				DownloadFile(String theURL);
	Sprite*				GetWebImage(String theURL);
	Sprite*				GetLocalImage(String theURL);

	struct WebImageStruct
	{
		WebImageStruct() {mSprite=NULL;}
		~WebImageStruct() {gG.UnloadTexture(mSprite->mTexture);delete mSprite;mSprite=NULL;}

		Sprite*			mSprite;
	};
	SmartList(WebImageStruct)	mWebImageList;			// Only exists to ensure deletion of images/sprites

	struct WebBundleStruct
	{
	public:
		SpriteBundle	mBundle;
		String			mName;
	};
	SmartList(WebBundleStruct)	mWebBundleList;
//	OverrideTextureSize();
//	if (!SpriteBundle::LoadDyna(PointAtDynamics("images\\Blocks.dyna"))) return;



	bool						mAbort;					// Lets the thread know it's abort time..

	//
	// List of all hyperlinks...
	//
	SmartList(MLLink)			mLinkList;				
	Smart(MLLink)				mCurrentLink;
	List						mLinkButtonList;
	List						mHelperList;		// Extra CPUs in a different list, for access
	List						mCPUList;			// List of extra CPUs, like textboxes, etc...

	void						ClearContents();
	inline void					Reset() {ClearContents();}

	String						mVersion;

	Color						mDefaultColor;
	inline void					SetDefaultColor(Color theColor) {mDefaultColor=theColor;}

	struct RecordStruct
	{
		String					mName;
		longlong				mHash;
		Rect					mRect;

		SmartList(MLObject)		mObjectList;
	};
	SmartList(RecordStruct)		mRecordList;
	Rect						GetRecord(String theName);
	Rect						GetRecord(int theHash);

	Stack<Smart(RecordStruct)>	mRecordStack;

	struct FlagStruct
	{
		String					mName;
		String					mValue;
	};
	SmartList(FlagStruct)		mFlagList;

	bool						IsFlagSet(String theName);
	String						GetFlagValue(String theName);
	FlagStruct*					GetFlagStruct(String theName);

	Smart(MLObject)				GetObjectByID(String theID);

	struct DefineStruct
	{
		String					mFrom;
		String					mTo;
	};
	SmartList(DefineStruct)		mDefineList;
	void						Define(String theOriginal, String theReplace);
	String						GetDefine(String theDefine);

	//
	// List of <customfromrecord> images that are marked for final processing
	//
	SmartList(MLImage)			mCustomFromRecordList;
	SmartList(MLLink)			mLinkFromRecordList;

	//
	// The link we just clicked-- in case we need extra data.
	//
	MLLink*						mClickedLink;
	
	//
	// Background Color
	//
	Color						mBKGColor;

	//
	// Skip tags because we're on the wrong OS...
	//
	bool						mSkipOSTags;
};

