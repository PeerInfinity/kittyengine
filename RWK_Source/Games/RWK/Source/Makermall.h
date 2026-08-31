#pragma once
#include "rapt.h"
#include "MyApp.h"

#define LEVELFETCHCOUNT	10

class ListWidget;
class TransferPacket;

class AvatarSprite : public Object
{
public:
	AvatarSprite() {mCountdown=-1;mExtraData=NULL;}

	Smart(IOBuffer)	mBuffer;
	Smart(Sprite)	mSprite;
	String			mName;
	void*			mExtraData;
	int				mCountdown;
	char			mFlag=1;		// 1 = pending

	void			Go();
	void			Update();


};


void UpdateAvatarList();
Smart(AvatarSprite) GetAvatar(String theAvatar, bool isManaged);


class Makermall : public CPU
{
public:
	Makermall();
	virtual ~Makermall();

	void				LeaveMakermall();
	void				ReturnToMakermall();
	void				Initialize();
	void				Update();
	void				Draw();
	void				Core_Draw();
	void				Notify(void* theData);
	void				ReturnFromEditor(String theLevel);
	void				SetMostRecentLevel(String theLevel);
	void				RemoveFromRecentLevels(String theLevel);
	void				CheckLogin();
	void				AddMessage(String theMessage);
	void				ManualUnshareLevel(int theID);
	void				BadVersion();
	void				UnloadAvatars();
	void				FillProfileInfo(String theProfileInfo);
	void				SendIPlayed();	
	void				SendIWon(int theGameTime);
	void				GetLocalLevelStats(Array<int>& theLevels);
	void				FillLocalStats(String theString);
	void				FillLevelStats(String theString);
	void				BackFromPlaying();
	void				RemoveMail(int theID);
	void				Back() {Notify(&mMainMenuButton);}
	void				UploadLevel(TransferPacket* theTP);
	void				UnshareLevel(TransferPacket* theTP);
	void				StartPlaying(int theLevelID);

	void				AddToRecents(String theRML);
	Array<String>		mRecentLevels;
	void				SaveRecents();
	void				LoadRecents();
	void				RemoveFromRecentList(int theID);

	float				mStep;
	bool				mAdmin;

	//
	// Our author's info...
	//
	String					mKarma;
	String					mKarmaUp;
	String					mKarmaDown;
	String					mStars;
	String					mLevel;
	String					mAvatar;
	SuperBits				mBonus;	// Our user's unlocked features

	//
	// Draw helpers
	//
	float					mKarmaWidth;
	int						mStarCount;
	bool					mNotifyLevelup;	// If we can level up

	Button				mMainMenuButton;
	Button				mTagsButton;
	Button				mMyLevelsButton;
	Button				mNewLevelsButton;
	Button				mHotLevelsButton;
	Button				mRandomLevelsButton;
	Button				mStarLevelsButton;
	Button				mSearchButton;
	Button				mSignInButton;	// Bottom of screen
	Button				mLevelupButton;
	Button				mMailButton;
	Button				mPlayingLevelsButton;

	void				FixBottomOfScreen();

	void*				mViewing;
	void*				mNextViewing;
	void				View(void* theViewing);
	void				SwapToNewView();

	RComm::RQueryList	mPendingQueryList;
	void				UpdatePendingQueryList();
	String				mNow;
	String				mSavedNow;
	int					mPage;
	longlong			mFilter;

	ListWidget*			mListWidget;

	void				BuildLevels(String theLevelRML);

	Array<String>		mTags;
	void				FillTags(String theString);

	Array<int>			mLocalLevels;
	int					mPlayingID;
	int					mRatedID;

	Array<String>		mMsgList;
	Array<int>			mMsgIDList;
	bool				mUnreadMsg;

	String				mSearchName;
	String				mSearchAuthor;
	bool				mSearchNew;
	bool				mSearchAgain;	// Lets us know we're searching again (i.e. we changed bitmasks, so we don't go back to the search widget)

	bool				mInstaplay=false;	// Means we're playing from "go=levelid" in command line...
	bool				mWasInstaplay=false;
	String				mInstaplayName;		// Name
	String				mInstaplayAuthor;	// Author

};

class ListWidget : public SwipeBox
{
public:
	ListWidget();
	virtual ~ListWidget();

	void				Rehup();
	void				Draw();

	String				mMessage;
	bool				mHasLevels;

};

class Widget : public CPU
{
public:
	Widget();
	virtual ~Widget();

	void				Load(String theRML);
	void				Draw();
	void				TouchStart(int x, int y);
	void				TouchEnd(int x, int y);
	void				TouchMove(int x, int y);
	void				Notify(void* theData);
	char				mType;
	String				mRML;	// Keep the RML for recent levels...

	void				Initialize();

	bool				mLocal;
	int					mID;
	float				mClientVersion;
	String				mName;
	String				mAuthor;
	String				mAuthorLevel;
	int					mAuthorLevelColor;
	Rect				mAuthorLevelBound;
	int					mFlag;
	longlong			mSourceTags;
	Array<String>		mTags;
	Array<float>		mTagWidth;
	String				mTried;
	String				mWon;
	String				mCommentCount;
	String				mRating;
	char				mAnimate;
	int					mStarCount;
	bool				mNoDying;

	Color				mColor;
	bool				mIsDown;

	String				mCommand;
	bool				mCleanMe;	// Means destroy this if we load more, etc (since it will get added automatically)

	Button				mCommentButton;
	Smart(AvatarSprite) mAvatar;
};

extern Makermall* gMakermall;

class MMBrowser : public Browser
{
public:
	void				Notify(void* theData);
};

class TransDialog : public CPU
{
public:
	TransDialog();
	virtual ~TransDialog();

	void				Initialize();
	void				Update();
	void				Draw();
	void				TouchStart(int x, int y);
	void				SetStatus(String theStatus);

public:
	RComm::RQuery		mQuery;
	String				mStatus;

	bool				mCanCancel;
	String				mCancelText;
	Rect				mCancelRect;
	Rect				mBox;

	float				mProgress;
	bool				mStarting;
	bool				mDone;
	float				mSpeedMod;
	bool				mAddQueryToPending;

};

extern TransDialog* gTransDialog;

class TransferPacket
{
public:
	String				mName;
	String				mFN;
	TransDialog*		mDialog;
	longlong			mTags;
	int					mUploadID;
};

bool IsLoginValid();
void UploadStub(void* theArg);
void UnshareStub(void* theArg);

void SetNotifyMLBox(MyMLBox* theBox);
MyMLBox* GetNotifyMLBox();

void OpenComments(int theLevelID);

class CommentWidget : public CPU
{
public:
	void				Initialize();
	void				Update();
	void				Draw();
	void				Notify(void* theData);

public:
	int					mLevelID;
	TextBox				mTextBox;

	Button				mSayIt;
	Button				mClose;
};
CommentWidget* AddCommentWidget(int theLevelID);

class AbuseWidget : public CommentWidget
{
public:
	void				Draw();
	void				Notify(void* theData);
};


class RatingScreen : public CPU
{
public:
	RatingScreen();
	virtual ~RatingScreen();
	void				Initialize();
	void				Update();
	void				Draw();
	void				Notify(void* theData);
	void				Back() {Kill();}

public:
	int					mLevelID;
	int					mVote;
	bool				mGiveStar;
	bool				mReportAbuse;
	bool				mOldReportAbuse;

	float				mStarFade;
	float				mAbuseFade;

	float				mFade;

	Button				mComments;
	Button				mLike;
	Button				mHate;
	Button				mDone;
	Button				mStar;
	Button				mReport;

	int					mMyStars;

};

class FilterWidget : public Widget
{
public:
	FilterWidget();
	void				Initialize();
	void				Draw();
	void				TouchEnd(int x, int y);
	//void				TouchMove(int x, int y) {}

	longlong			mTags;

	Array<String>		mTagName;
	Array<Rect>			mTagRect;
};

void ShowFilterDialog();
void ShowMail();

class DialogPickTags;
class FilterDialog : public FunDialog
{
public:
	bool				Done();

	DialogPickTags*		mTagDialog;
};

class RetagDialog : public FilterDialog
{
public:
	bool				Done();
	int					mLevelID;
};


class SearchWidget : public Widget
{
public:
	SearchWidget();
	void				Initialize();
	void				Draw();
	void				Notify(void* theData);
	void				TouchEnd(int x, int y) {mIsDown=false;}

	TextBox				mSearchName;
	TextBox				mSearchAuthor;
	Button				mOrderNew;
	Button				mOrderOld;
	Button				mSearch;

	bool				mNewest;
};
extern MMBrowser* gMailBrowser;
extern bool gValidLogin;

class PickMenu : public CPU
{
public:
	PickMenu();
	void				Initialize();
	void				Update();
	void				Draw();

	void				AddButton(String theText);
	void				Notify(void* theData);
//	String				Go(Point theCenter, bool doFit=true, float extraButtonSize=0);
	void				GoX(Point theCenter, bool doFit, float extraButtonSize,MBHOOKPTR theHook);


public:
	Array<String>		mButtonText;
	List				mButtonList;

	String				mResult;

	Font*				mFont;
	Sprite*				mRimSprite;
	float				mRimBorder;
	bool				mDarken=false;

	MBHOOKPTR			mHook;

};

void MarkUntested();

