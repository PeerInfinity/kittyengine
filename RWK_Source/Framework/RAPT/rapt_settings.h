#pragma once

#include "rapt_headers.h"

// This seems to be unsafe on mobile
#define ALLOW_REGISTER_VARIABLES

enum
{
	VARTYPE_INT=0,
	VARTYPE_FLOAT,
	VARTYPE_POINT,
	VARTYPE_IPOINT,
	VARTYPE_CHAR,
	VARTYPE_BOOL,
	VARTYPE_STRING,
	VARTYPE_RECT,
	VARTYPE_INTARRAY,
	VARTYPE_BOOLARRAY,
	VARTYPE_STRINGARRAY,
	VARTYPE_CHARARRAY,

	VARTYPE_MAX
};

class Settings;
class SettingEntry
{
public:
	SettingEntry(Settings* theParent) {mParent=theParent;}
	String			mName;
	String			mValue;
	Settings*		mParent=NULL;
};

template <typename vtype>
class Setting
{
public:
	Setting() {}
	Setting(Setting<vtype>& other) {Copy(&other);}

	Smart(SettingEntry) mEntry=NULL;

	inline bool Lock();
	inline void Unlock();
	inline void Copy(Setting* theSetting);

	//
	// If we need direct access...
	//
	inline vtype Type() {vtype aHint=NULL;return Get(aHint);}
	inline vtype VType() {vtype aHint=NULL;return Get(aHint);}

	void		Set(String theValue) {if (Lock()) mEntry->mValue=theValue;Unlock();}
	void		Set(int theValue) {if (Lock()) mEntry->mValue=Sprintf("%d",theValue);Unlock();}
	void		Set(float theValue) {if (Lock()) mEntry->mValue=Sprintf("%f",theValue);Unlock();}
	void		Set(Point theValue) {if (Lock()) mEntry->mValue=theValue.ToString(4);Unlock();}
	void		Set(IPoint theValue) {if (Lock()) mEntry->mValue=theValue.ToString();Unlock();}
	void		Set(bool theValue) {if (Lock()) mEntry->mValue=theValue ? "true" : "false";Unlock();}
	void		Set(char theValue) {if (Lock()) mEntry->mValue=Sprintf("%d",theValue);Unlock();}
	void		Set(Rect theValue) {if (Lock()) mEntry->mValue=theValue.ToString();Unlock();}

	//
	// Dumb: I need to overload like this to do a nice "get"
	//
	String&		Get(String theHint) {static String aResultString;aResultString=mEntry->mValue;return aResultString;}
	int&		Get(int theHint) {static int aResultInt;aResultInt=mEntry->mValue.ToInt();return aResultInt;}
	float&		Get(float theHint) {static float aResultFloat;aResultFloat=mEntry->mValue.ToFloat();return aResultFloat;}
	Point&		Get(Point theHint) {static Point aResultPoint;aResultPoint=mEntry->mValue.ToPoint();return aResultPoint;}
	IPoint		Get(IPoint theHint) {Point aP=mEntry->mValue.ToPoint();static IPoint aResultIPoint;aResultIPoint=IPoint((int)aP.mX,(int)aP.mY);return aResultIPoint;}
	bool&		Get(bool theHint) {static bool aResultBool;aResultBool=mEntry->mValue.ToBool();return aResultBool;}
	char&		Get(char theHint) {static char aResultChar;aResultChar=(char)mEntry->mValue.ToInt();return aResultChar;}
	Rect&		Get(Rect theHint) {String aS1=mEntry->mValue.GetSegmentBefore(':');String aS2=mEntry->mValue.GetSegmentAfter(':');Point aP1=aS1.ToPoint();Point aP2=aS2.ToPoint();static Rect aResultRect;aResultRect=Rect(aP1.mX,aP1.mY,aP2.mX,aP2.mY);return aResultRect;}
	vtype&		Get() {vtype aHint=NULL;return Get(aHint);}

	void		operator=(Setting<vtype> theValue) {mEntry=theValue.mEntry;}
	void		operator=(vtype theValue) {Set(theValue);}
	void		operator++(int) {Set(Get()+1);}
	void		operator--(int) {Set(Get()-1);}
	void		operator++() {Set(Get()+1);}
	void		operator--() {Set(Get()-1);}
	void		operator+=(vtype theValue) {Set(Get()+theValue);}
	void		operator-=(vtype theValue) {Set(Get()-theValue);}
	void		operator*=(vtype theValue) {Set(Get()*theValue);}
	void		operator/=(vtype theValue) {Set(Get()/theValue);}
	void		operator%=(vtype theValue) {Set(Get()%theValue);}
	void		operator|=(vtype theValue) {Set(Get()|theValue);}
	void		operator&=(vtype theValue) {Set(Get()&theValue);}
	void		operator^=(vtype theValue) {Set(Get()^theValue);}
	void		operator<<=(vtype theValue) {Set(Get()<<theValue);}
	void		operator>>=(vtype theValue) {Set(Get()>>theValue);}
	operator	vtype&() {vtype aHint=NULL;return Get(aHint);}
};

typedef Setting<String> SettingString;
typedef Setting<int> SettingInt;
typedef Setting<float> SettingFloat;
typedef Setting<Point> SettingPoint;
typedef Setting<IPoint> SettingIPoint;
typedef Setting<bool> SettingBool;
typedef Setting<char> SettingChar;
typedef Setting<Rect> SettingRect;

class Settings : public CPU
{
public:
	Settings();
	virtual ~Settings();

	void				Update();
	void				Load(String theFilename);
	void				Reload();
	bool				Save(String theFilename="");

	void				Load(IOBuffer& theBuffer);
	bool				Save(IOBuffer& theBuffer);


	void				Changed();
	void				Reset();

	bool				Exists(String theName);

	template <typename vtype>
	void Register(String theName, Setting<vtype>* hookMe, vtype theDefault)
	{
		Setting<vtype> aSetting;
		aSetting.mEntry=GetEntry(theName);
		if (aSetting.mEntry.IsNull())
		{
			aSetting.mEntry=new SettingEntry(this);
			aSetting.mEntry->mName=theName;
			aSetting.Set(theDefault);
			mSettingList+=aSetting.mEntry;
		}
		hookMe->Copy(&aSetting);
	}

	void Register(String theName, SettingString* hookMe, String theDefault) {Register<String>(theName,hookMe,theDefault);}
	void Register(String theName, SettingInt* hookMe, int theDefault) {Register<int>(theName,hookMe,theDefault);}
	void Register(String theName, SettingFloat* hookMe, float theDefault) {Register<float>(theName,hookMe,theDefault);}
	void Register(String theName, SettingPoint* hookMe, Point theDefault) {Register<Point>(theName,hookMe,theDefault);}
	void Register(String theName, SettingIPoint* hookMe, IPoint theDefault) {Register<IPoint>(theName,hookMe,theDefault);}
	void Register(String theName, SettingBool* hookMe, bool theDefault) {Register<bool>(theName,hookMe,theDefault);}
	void Register(String theName, SettingChar* hookMe, char theDefault) {Register<char>(theName,hookMe,theDefault);}
	void Register(String theName, SettingRect* hookMe, Rect theDefault) {Register<Rect>(theName,hookMe,theDefault);}

	//
	// Manual setting of variables...
	//
	int					GetInt(String theName, int theDefault=0);
	void				SetInt(String theName, int theInt);
	float				GetFloat(String theName, float theDefault=0.0f);
	void				SetFloat(String theName, float theFloat);
	Point				GetPoint(String theName, Point theDefault=Point(0,0));
	void				SetPoint(String theName, Point thePoint);
	IPoint				GetIPoint(String theName, IPoint theDefault=IPoint(0,0));
	void				SetIPoint(String theName, IPoint thePoint);
	char				GetChar(String theName, char theDefault=0);
	void				SetChar(String theName, char theChar);
	bool				GetBool(String theName, bool theDefault=false);
	void				SetBool(String theName, bool theBool);
	String				GetString(String theName, String theDefault="");
	void				SetString(String theName, String theString);
	Rect				GetRect(String theName, Rect theDefault=Rect(0,0,0,0));
	void				SetRect(String theName, Rect theRect);

	void				GetCharArray(String theName, Array<char>& theArray, String theDefault="");
	void				SetCharArray(String theName, Array<char>& theArray);
	void				GetIntArray(String theName, Array<int>& theArray, String theDefault="");
	void				SetIntArray(String theName, Array<int>& theArray);
	void				GetBoolArray(String theName, Array<bool>& theArray, String theDefault="");
	void				SetBoolArray(String theName, Array<bool>& theArray);
	void				GetStringArray(String theName, Array<String>& theArray, String theDefault="");
	void				SetStringArray(String theName, Array<String>& theArray);

	//
	// Registering variables for automatic use
	//
#ifdef ALLOW_REGISTER_VARIABLES
	void				RegisterVariable(String theName, int& theValue, int theDefault);
	void				RegisterVariable(String theName, float& theValue, float theDefault);
	void				RegisterVariable(String theName, bool& theValue, bool theDefault);
	void				RegisterVariable(String theName, Point& theValue, Point theDefault);
	void				RegisterVariable(String theName, IPoint& theValue, IPoint theDefault);
	void				RegisterVariable(String theName, char& theValue, char theDefault);
	void				RegisterVariable(String theName, String& theValue, String theDefault);
	void				RegisterVariable(String theName, Rect& theValue, Rect theDefault);
	void				RegisterVariable(String theName, Array<char>& theArray, String theDefault);
	void				RegisterVariable(String theName, Array<int>& theArray, String theDefault);
	void				RegisterVariable(String theName, Array<bool>& theArray, String theDefault);
	void				RegisterVariable(String theName, Array<String>& theArray, String theDefault);
	void				UnregisterVariable(void* thePtr);
	void				UnregisterVariable(String theName);

	bool				mWatchRegisterVariables;	// For backwards compatibility to the old days when RegisterVariables were NOT watched...
	inline void			WatchRegisteredVariables() {mWatchRegisterVariables=true;}
	inline void			DontWatchRegisteredVariables() {mWatchRegisterVariables=false;}
#endif


public:
	String				mFilename;

	bool				mReadOnly;
	inline void			SetReadOnly(bool theState=true) {mReadOnly=theState;}

	bool				mIsChanged;
	inline bool			IsChanged() {return mIsChanged;}

	bool				IsChangedEX();

	SmartList(SettingEntry)		mSettingList;
	Smart(SettingEntry)			GetEntry(String theName);
	bool						IsEmpty() {return mSettingList.GetCount()==0;}

#ifdef ALLOW_REGISTER_VARIABLES
	struct RegisteredVariable
	{
		String			mName;
		void*			mPointer;
		int				mType;
		bool			mManualChanged=false;

		RegisteredVariable() {mBackup=NULL;mMemPtr=NULL;}
		~RegisteredVariable() {if (mBackup) delete [] mBackup;}

		void*			mMemPtr;
		int				mMemSize;
		char*			mBackup;

		void			Watch(void* thePtr, int theSize) {mMemPtr=thePtr;mMemSize=theSize;Backup();}
		void			Backup() {if (mBackup==NULL) mBackup=new char[mMemSize];memcpy(mBackup,mMemPtr,mMemSize);mManualChanged=false;}
		bool			IsChanged(bool doBackup) {if (mManualChanged || memcmp(mMemPtr,mBackup,mMemSize)!=0) {if (doBackup) Backup();return true;}return false;}
	};
	
	int								mRegisterListWatcher;
	SmartList(RegisteredVariable)	mRegisterList;
	void							SaveRegisteredVariable(Smart(RegisteredVariable) aRV);
	void							LoadRegisteredVariable(Smart(RegisteredVariable) aRV);
	void							MarkRegisteredVariableChanged(String theName);

#endif

	//
	// For debugging...
	//
	void				Dump(String thePrefix="");

	int					mThreadLock;
	void				ThreadLock(bool doCreate=false);
	void				ThreadUnlock();
};

template <typename vtype>
bool Setting<vtype>::Lock() 
{
	#ifdef _DEBUG
	if (!mEntry->mParent) OS_Core::Printf("! Tried to save ORPHAN Setting variable: [%s]",mEntry->mName.c());
	#endif	
	if (!mEntry->mParent) return false;mEntry->mParent->ThreadLock();return !mEntry->mParent->mReadOnly;
}

template <typename vtype>
void Setting<vtype>::Unlock() {if (!mEntry->mParent) return;mEntry->mParent->ThreadUnlock();mEntry->mParent->Changed();}

template <typename vtype>
void Setting<vtype>::Copy(Setting* theSetting) {mEntry=theSetting->mEntry;}

