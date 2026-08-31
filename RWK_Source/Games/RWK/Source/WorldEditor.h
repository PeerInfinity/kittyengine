#pragma once
#include "rapt.h"
#include "MyApp.h"
#include "World.h"

class Toolbox;
class WorldEditor :
	public World
{
public:
	WorldEditor(void);
	virtual ~WorldEditor(void);

	void				Initialize();
	void				Update();
	void				Draw();
	void				DrawOverlay();
	void				DrawBackground();
	void				DrawUI();
	void				DeleteLevel();
	void				SetGrid(int theX, int theY, char theValue);
	void				FinishSharing();


	void				SetRadioMessage(IPoint thePos, String theText);

	String				QuickSave();

	void				Clean();
	void				MakeBlank();

	void				EnlargeWorld(int newX, int newY, int newWidth, int newHeight);
	void				MemToGrid(Grid *theSource, int theSourceStride, Rect theSourceRect, Point theDestUpperLeft);

	int					GetTileMatch47(int x, int y, int theID);
	bool				IsTileMatch(int x, int y, int theID);

	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);
	void				TouchMove(int x, int y);
	void				MouseDoubleClick(int x, int y, int theButton);

	void				OpenSettings();



	void				Save(String theFilename);
	void				Load(String theFilename);
#ifdef _WIN32
	void				LoadOldVersion(String theFilename);
#endif

	void				Notify(void *theData);

	bool				mDraggingRobot;
	bool				mDraggingKitty;
	Point				mDragOffset;

	float				mTempConveyorSpeed;

public:
	int					mUploadID;
	String				mName;
	longlong			mTags;
	bool				mTestedOK;
	bool				mTestedNoDying;
	char				mFlagBits;

	bool				mScrolling;

	Button				mBackButton;
	Button				mSettingsButton;
	Button				mShareButton;
	Button				mLayoutButton;
	Button				mPaintButton;
	Button				mEraseButton;
	Button				mTrashButton;

	Button				mSaveButton;
	Button				mTestButton;

	Point				mThumbPadCenter;

	World				*mTestWorld;
	
	Toolbox				*mToolbox;

	//
	// Tools currently being used
	//
	int					mToolType;
	int					mTool;
	int					GetRealTool();

	bool				mWantNewID;
	unsigned int		mPaintID;
	int					GetPaintRef();
	int					GetPaintBase(int theTool);

	int					mPaintMask;

	bool				mMouseIsDown;
	IPoint				mMouseGrid;
	void				SetMouseGrid(int theX, int theY);

	//
	// Painting and plotting
	//
	void				Paint(int x, int y);
	void				Plot(int x, int y);
	void				PlotPaint(int x, int y);
	bool				PlotPaint1(int x, int y, int theTool, int theID, bool noPaint=false);
	void				PaintFix(int x, int y, int theTool, int theID=-1);
	int					PaintToTool(int thePaint);

	//
	// Level info (for saving)
	//
	String				mLevelname;
};

extern WorldEditor *gWorldEditor;

class Toolbox : public CPUPlus
{
public:
	Toolbox();
	virtual ~Toolbox();

	void				Update();
	void				Draw();
	void				TouchStart(int x, int y);

	virtual void		DrawTools() {}
	virtual void		PickTool(int theNumber) {}

public:
	String				mCaption;
	float				mToolWidth;
};

class Toolbox_Layout : public Toolbox
{
public:
	void				DrawTools();
	virtual void		PickTool(int theNumber);
};

class Toolbox_Paint : public Toolbox
{
public:
	void				DrawTools();
	virtual void		PickTool(int theNumber);
	void				TouchStart(int x, int y);

};

class Toolbox_File : public Toolbox
{
public:
	void				DrawTools();
	virtual void		PickTool(int theNumber);

};

class SaveDialog : public CPUPlus
{
public:
	SaveDialog(String theFilename);
	virtual ~SaveDialog();

	void				Update();
	void				Draw();
	void				Notify(void *theData);

	TextBox				mEditBox;
	//EditBox				mEditBox;
	Button				mCancel;
	Button				mSave;
	Button				mBrowse;

	bool				mHide;
};

class BrowseDialog : public CPUPlus
{
public:
	BrowseDialog();
	virtual ~BrowseDialog();

	void				Update();
	void				Draw();
	void				Notify(void *theData);

	void				MouseDown(int x, int y, int theButton);

	void				Freshen();

	List				mDirList;

	Button				mCancel;
	Button				mDelete;
	Button				mSelect;
	CPUPlus				*mCallback;

	struct KittyFile 
	{
		String			mName;
		int				mDifficulty;
	};

	int					mSelected;

	void				StartDrag(int x, int y);
	void				Drag(int x, int y);
	Point				mDragPos;
	float				mScroll;
	float				mScrollMin;
	float				mScrollMax;
	float				mDragSpeed;

	String				mSelectText;
	String				mCaption;
};

class UploadDialog : public CPUPlus
{
public:
	UploadDialog();
	virtual ~UploadDialog();
	
	void				Update();
	void				Draw();
	void				Notify(void *theData);

	void				StartUpload();
	
public:

	bool				mWantUpload;
	bool				mFirstDraw;

	float				mProgress;

	Button				mButton;
	bool				mSuccess;
	
};


class DownloadDialog : public CPUPlus
{
public:
	DownloadDialog();
	virtual ~DownloadDialog();

	void				Update();
	void				Draw();
	void				Notify(void *theData);

	void				StartDownload();

	bool				mWantDownload;
	bool				mFirstDraw;

	float				mProgress;

	Button				mButton;
	bool				mSuccess;

	IOBuffer			mDownloadBuffer;

};

class DialogPickTags;
class LevelSettingsDialog : public FunDialog
{
public:
	LevelSettingsDialog();
	void				ChangedControl(DialogWidget* theWidget);
	bool				Done();
	void				FinishRename();

	String				mOldName;
	DialogPickTags*		mTagDialog;
	DialogPanel*		mMusicPanel;
	DialogWidget*		mSongButton[3];

	void				RehupMusicPanel();
	void				NextPanel(DialogPanel* thePanel);
	void				PreviousPanel();

};

class DialogPickTags : public DialogWidget
{
public:
	DialogPickTags(longlong theTags);
	void				Initialize();
	void				Draw();
	void				Notify(void* theData);
	void				UpdateButtons();

	List				mButtonList;
	Array<int>			mTags;
	longlong			mTagBits;
	bool				mAllowInfiniteTagPicks;
};

extern bool gWantTutorDialog;
