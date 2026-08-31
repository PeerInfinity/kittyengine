#include "rapt_settings.h"
#include "rapt_app.h"


Settings::Settings()
{
	mThreadLock=-1;
	mIsChanged=false;
	Size(-1000,-1000,-1,-1);
	mReadOnly=false;
#ifdef ALLOW_REGISTER_VARIABLES
	mRegisterListWatcher=0;
	mWatchRegisterVariables=true;
#endif
}

Settings::~Settings()
{
	if (mIsChanged)	Save();
	EnumSmartList(SettingEntry,aSE,mSettingList) aSE->mParent=NULL; // Makes all our Registered variables into orphans...
	Reset();
	if (gAppPtr && !gAppPtr->IsQuit() && mThreadLock>=0) KillThreadLock(mThreadLock);
	if (gAppPtr) gAppPtr->mBackgroundProcesses-=this;
}

void Settings::Reset()
{
	ThreadLock();
	mSettingList.Clear();
#ifdef ALLOW_REGISTER_VARIABLES
	mRegisterList.Clear();
#endif
	ThreadUnlock();
}

void Settings::Update()
{
#ifdef ALLOW_REGISTER_VARIABLES
	if (!mWatchRegisterVariables)
	{
		if (mIsChanged) Save();
		else if (gAppPtr) gAppPtr->mBackgroundProcesses-=this;
	}
	else
	{
		if (mRegisterList.GetCount())
		{
			mRegisterListWatcher++;
			mRegisterListWatcher%=mRegisterList.GetCount();
			if (mRegisterList[mRegisterListWatcher]->IsChanged(false)) mIsChanged=true;
		}
		if (mIsChanged) Save();
		else if (gAppPtr) if (mRegisterList.GetCount()==0) gAppPtr->mBackgroundProcesses-=this;
	}
#else
	if (mIsChanged) Save();
	else if (gAppPtr) gAppPtr->mBackgroundProcesses-=this;
#endif
}

void Settings::Load(String theFilename)
{
	mFilename=theFilename;
	IOBuffer aBuffer;
	aBuffer.SetFileSource(theFilename);
	Load(aBuffer);
}

void Settings::Load(IOBuffer& theBuffer)
{
	ThreadLock();

	/*
#ifdef _DEBUG
	theBuffer.CommitFile(PointAtDesktop("Out.txt"));
#endif
	/**/

	while (!theBuffer.IsEnd())
	{
		String aLine=theBuffer.ReadLine();
		if (aLine.Find('=')!=String::mNotFound || aLine.StartsWith("//"))
		{
			String aName=aLine.GetToken('=');
			String aValue=aLine.GetNextToken();

			Smart(SettingEntry) aSE=GetEntry(aName);
			if (aName.StartsWith("//")) aSE=NULL;
			if (aSE.IsNull()) 
			{
				aSE=new SettingEntry(this);
				mSettingList+=aSE;
			}

			aSE->mName=aName;
			aSE->mValue=aValue;
		}
	}

#ifdef ALLOW_REGISTER_VARIABLES
	EnumSmartList(RegisteredVariable,aRV,mRegisterList) LoadRegisteredVariable(aRV);
#endif
	ThreadUnlock();
}

void Settings::Reload()
{
	if (mFilename.Len()) Load(mFilename);
}

bool Settings::Save(String theFilename)
{
	mIsChanged=false;

	if (mReadOnly) return false;
	if (!gAppPtr) return false;
	if (theFilename.Len()>0) mFilename=theFilename;
	if (mFilename.Len()<=0) return false;

/*
#ifdef ALLOW_REGISTER_VARIABLES
	EnumSmartList(RegisteredVariable,aRV,mRegisterList) aRV->Backup();
#endif
*/

	IOBuffer aBuffer;
	bool aResult=Save(aBuffer);
	aBuffer.CommitFile(mFilename);
	return aResult;
}

bool Settings::IsChangedEX()
{
#ifdef ALLOW_REGISTER_VARIABLES
	if (mWatchRegisterVariables) return false;
	if (mIsChanged) return true;
	EnumSmartList(RegisteredVariable,aRV,mRegisterList) if (aRV->IsChanged(false)) return true;
	return false;
#else
	return mIsChanged;
#endif
}


bool Settings::Save(IOBuffer& theBuffer)
{
	mIsChanged=false;

	if (!gAppPtr) return false;
	if (mReadOnly) return false;

	ThreadLock();
	//
	// If we have registered variables, they should be hooked up!
	//
	//gOut.Out("$ ----------------------------> Do save?");
#ifdef ALLOW_REGISTER_VARIABLES
	EnumSmartList(RegisteredVariable,aRV,mRegisterList) 
	{
		//gOut.Out("$ ----------------------------> Check RV: %s  : %d",aRV->mName.c(),aRV->IsChanged(false));
		if (aRV->IsChanged(true)) 
		{
#ifdef _DEBUG
			gOut.Out("* ----------------------------> Save RV: %s",aRV->mName.c());
#endif
			SaveRegisteredVariable(aRV);
		}
	}
#endif
	mIsChanged=false;

    
	IOBuffer aBuffer;

	EnumSmartList(SettingEntry,aSE,mSettingList)
	{
		if (aSE->mName.StartsWith("//") && aSE->mValue.Len()==0) theBuffer.WriteLine(Sprintf("%s",aSE->mName.c()));
		else theBuffer.WriteLine(Sprintf("%s=%s",aSE->mName.c(),aSE->mValue.c()));
	}
   
#ifdef ALLOW_REGISTER_VARIABLES
	if (gAppPtr) if (mRegisterList.GetCount()==0) gAppPtr->mBackgroundProcesses-=this;
#else
	gAppPtr->mBackgroundProcesses-=this;
#endif
	ThreadUnlock();

	mIsChanged=false;

	return true;
}

void Settings::Changed()
{
	if (!mIsChanged)
	{
		if (gAppPtr) gAppPtr->mBackgroundProcesses+=this;
		mIsChanged=true;
	}
}

bool Settings::Exists(String theName)
{
	bool aResult=false;

	ThreadLock();
	EnumSmartList(SettingEntry,aSE,mSettingList) if (aSE->mName==theName) {aResult=true;break;}
	ThreadUnlock();

	return aResult;
}


Smart(SettingEntry) Settings::GetEntry(String theName)
{
	Smart(SettingEntry) aResult=NULL;
	EnumSmartList(SettingEntry,aSE,mSettingList) 
	{
		if (aSE->mName==theName) {aResult=aSE;break;}
	}
	return aResult;
}

int Settings::GetInt(String theName, int theDefault)
{
	int aResult=theDefault;
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNotNull()) aResult=aSE->mValue.ToInt();
	return aResult;
}

void Settings::SetInt(String theName, int theInt)
{
	if (mReadOnly) return;
	Changed();

	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNull()) {aSE=new SettingEntry(this);aSE->mName=theName;mSettingList+=aSE;}

	aSE->mValue=Sprintf("%d",theInt);
	ThreadUnlock();
}

void Settings::GetCharArray(String theName, Array<char>& theArray, String theDefault)
{
	String aCSV=GetString(theName,theDefault);
	Utils::CSVToArray(aCSV,theArray);
}

void Settings::GetIntArray(String theName, Array<int>& theArray, String theDefault)
{
	String aCSV=GetString(theName,theDefault);
	Utils::CSVToArray(aCSV,theArray);
}

void Settings::GetBoolArray(String theName, Array<bool>& theArray, String theDefault)
{
	String aCSV=GetString(theName,theDefault);
	Utils::CSVToArray(aCSV,theArray);
}

void Settings::GetStringArray(String theName, Array<String>& theArray, String theDefault)
{
	String aCSV=GetString(theName,theDefault);
	Utils::CSVToArray(aCSV,theArray);
}

void Settings::SetCharArray(String theName, Array<char>& theArray)
{
	String aCSV=Utils::ArrayToCSV(theArray);
	SetString(theName,aCSV);
}

void Settings::SetIntArray(String theName, Array<int>& theArray)
{
	String aCSV=Utils::ArrayToCSV(theArray);
	SetString(theName,aCSV);
}

void Settings::SetBoolArray(String theName, Array<bool>& theArray)
{
	String aCSV=Utils::ArrayToCSV(theArray);
	SetString(theName,aCSV);
}

void Settings::SetStringArray(String theName, Array<String>& theArray)
{
	String aCSV=Utils::ArrayToCSV(theArray);
	SetString(theName,aCSV);
}

float Settings::GetFloat(String theName, float theDefault)
{
	float aResult=theDefault;
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNotNull()) aResult=aSE->mValue.ToFloat();
	ThreadUnlock();
	return aResult;
}

void Settings::SetFloat(String theName, float theFloat)
{
	if (mReadOnly) return;

	Changed();
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNull()) {aSE=new SettingEntry(this);aSE->mName=theName;mSettingList+=aSE;}

	aSE->mValue=Sprintf("%f",theFloat);
	ThreadUnlock();
}

Point Settings::GetPoint(String theName, Point theDefault)
{
	Point aResult=theDefault;
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNotNull()) aResult=aSE->mValue.ToPoint();
	ThreadUnlock();
	return aResult;
}

void Settings::SetPoint(String theName, Point thePoint)
{
	if (mReadOnly) return;

	Changed();
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNull()) {aSE=new SettingEntry(this);aSE->mName=theName;mSettingList+=aSE;}
	aSE->mValue=Sprintf("%f,%f",thePoint.mX,thePoint.mY);
	ThreadUnlock();
}

Rect Settings::GetRect(String theName, Rect theDefault)
{
	Rect aResult=theDefault;
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNotNull()) 
	{
		String aS1=aSE->mValue.GetSegmentBefore(':');
		String aS2=aSE->mValue.GetSegmentAfter(':');
		Point aP1=aS1.ToPoint();
		Point aP2=aS2.ToPoint();
		aResult=Rect(aP1.mX,aP1.mY,aP2.mX,aP2.mY);
	}
	ThreadUnlock();
	return aResult;
}

void Settings::SetRect(String theName, Rect theRect)
{
	if (mReadOnly) return;

	Changed();
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNull()) {aSE=new SettingEntry(this);aSE->mName=theName;mSettingList+=aSE;}

	aSE->mValue=Sprintf("%f,%f:%f,%f",theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);
	ThreadUnlock();
}

IPoint Settings::GetIPoint(String theName, IPoint theDefault)
{
	IPoint aResult=theDefault;
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNotNull()) 
	{
		Point aP=aSE->mValue.ToPoint();
		aResult=IPoint((int)aP.mX,(int)aP.mY);
	}
	ThreadUnlock();
	return aResult;
}

void Settings::SetIPoint(String theName, IPoint thePoint)
{
	if (mReadOnly) return;

	Changed();
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNull()) {aSE=new SettingEntry(this);aSE->mName=theName;mSettingList+=aSE;}

	aSE->mValue=Sprintf("%d,%d",thePoint.mX,thePoint.mY);
	ThreadUnlock();
}

char Settings::GetChar(String theName, char theDefault)
{
	char aResult=theDefault;
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNotNull()) aResult=(char)aSE->mValue.ToInt();
	ThreadUnlock();
	return aResult;
}

void Settings::SetChar(String theName, char theChar)
{
	if (mReadOnly) return;

	Changed();
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNull()) {aSE=new SettingEntry(this);aSE->mName=theName;mSettingList+=aSE;}
	aSE->mValue=Sprintf("%d",theChar);
	ThreadUnlock();
}

bool Settings::GetBool(String theName, bool theDefault)
{
	bool aResult=theDefault;
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNotNull()) aResult=aSE->mValue.ToBool();
	ThreadUnlock();
	return aResult;
}

void Settings::SetBool(String theName, bool theBool)
{
	if (mReadOnly) return;

	Changed();
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNull()) {aSE=new SettingEntry(this);aSE->mName=theName;mSettingList+=aSE;}

	aSE->mValue=Sprintf("%s",theBool ? "true" : "false");
	ThreadUnlock();
}

String Settings::GetString(String theName, String theDefault)
{
	String aResult=theDefault;
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNotNull()) aResult=aSE->mValue;
	ThreadUnlock();
	return aResult;
}

void Settings::SetString(String theName, String theString)
{
	if (mReadOnly) return;

	Changed();
	ThreadLock();
	Smart(SettingEntry) aSE=GetEntry(theName);
	if (aSE.IsNull()) {aSE=new SettingEntry(this);aSE->mName=theName;mSettingList+=aSE;}
	aSE->mValue=theString;
	ThreadUnlock();
}

void Settings::Dump(String thePrefix)
{
	gOut.Out("%sSettings------------------------------",thePrefix.c());
	EnumSmartList(SettingEntry,aSE,mSettingList)
	{
		gOut.Out("%sEnter #%d  -->  [%s]=[%s]",thePrefix.c(),EnumListCounter,aSE->mName.c(),aSE->mValue.c());
	}
	gOut.Out("%sEnd Settings--------------------------",thePrefix.c());
}


#ifdef ALLOW_REGISTER_VARIABLES

void Settings::MarkRegisteredVariableChanged(String theName)
{
	EnumSmartList(RegisteredVariable,aRV,mRegisterList) if (aRV->mName==theName) aRV->mManualChanged=true;
}

void Settings::SaveRegisteredVariable(Smart(RegisteredVariable) aRV)
{
	switch (aRV->mType)
	{
	case VARTYPE_INT:
		SetInt(aRV->mName,*((int*)aRV->mPointer));
		break;
	case VARTYPE_CHARARRAY:
		SetCharArray(aRV->mName,*((Array<char>*)aRV->mPointer));
		break;
	case VARTYPE_INTARRAY:
		SetIntArray(aRV->mName,*((Array<int>*)aRV->mPointer));
		break;
	case VARTYPE_BOOLARRAY:
		SetBoolArray(aRV->mName,*((Array<bool>*)aRV->mPointer));
		break;
	case VARTYPE_STRINGARRAY:
		SetStringArray(aRV->mName,*((Array<String>*)aRV->mPointer));
		break;
	case VARTYPE_FLOAT:
		SetFloat(aRV->mName,*((float*)aRV->mPointer));
		break;
	case VARTYPE_BOOL:
		SetBool(aRV->mName,*((bool*)aRV->mPointer));
		break;
	case VARTYPE_POINT:
		SetPoint(aRV->mName,*((Point*)aRV->mPointer));
		break;
	case VARTYPE_IPOINT:
		SetIPoint(aRV->mName,*((IPoint*)aRV->mPointer));
		break;
	case VARTYPE_CHAR:
		SetChar(aRV->mName,*((char*)aRV->mPointer));
		break;
	case VARTYPE_STRING:
		SetString(aRV->mName,*((String*)aRV->mPointer));
		break;
	case VARTYPE_RECT:
		SetRect(aRV->mName,*((Rect*)aRV->mPointer));
		break;
	}
	aRV->Backup();
}

void Settings::LoadRegisteredVariable(Smart(RegisteredVariable) aRV)
{
	switch (aRV->mType)
	{
	case VARTYPE_INT:
		*((int*)aRV->mPointer)=GetInt(aRV->mName);
		break;
	case VARTYPE_CHARARRAY:
		GetCharArray(aRV->mName,*((Array<char>*)aRV->mPointer));
		break;
	case VARTYPE_INTARRAY:
		GetIntArray(aRV->mName,*((Array<int>*)aRV->mPointer));
		break;
	case VARTYPE_BOOLARRAY:
		GetBoolArray(aRV->mName,*((Array<bool>*)aRV->mPointer));
		break;
	case VARTYPE_STRINGARRAY:
		GetStringArray(aRV->mName,*((Array<String>*)aRV->mPointer));
		break;
	case VARTYPE_FLOAT:
		*((float*)aRV->mPointer)=GetFloat(aRV->mName);
		break;
	case VARTYPE_BOOL:
		*((bool*)aRV->mPointer)=GetBool(aRV->mName);
		break;
	case VARTYPE_POINT:
		*((Point*)aRV->mPointer)=GetPoint(aRV->mName);
		break;
	case VARTYPE_IPOINT:
		*((IPoint*)aRV->mPointer)=GetIPoint(aRV->mName);
		break;
	case VARTYPE_CHAR:
		*((char*)aRV->mPointer)=GetChar(aRV->mName);
		break;
	case VARTYPE_STRING:
		*((String*)aRV->mPointer)=GetString(aRV->mName);
		break;
	case VARTYPE_RECT:
		*((Rect*)aRV->mPointer)=GetRect(aRV->mName);
		break;
	}
	aRV->Backup();
}

void Settings::UnregisterVariable(void* thePtr)
{
	EnumSmartList(RegisteredVariable,aRV,mRegisterList) if (aRV->mPointer==thePtr) {mRegisterList-=aRV;break;}
}

void Settings::UnregisterVariable(String theName)
{
	EnumSmartList(RegisteredVariable,aRV,mRegisterList) if (aRV->mName==theName) {mRegisterList-=aRV;break;}
}


void Settings::RegisterVariable(String theName, int& theValue, int theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_INT;
	aRV->mName=theName;
	aRV->mPointer=&theValue;

	theValue=GetInt(theName,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theValue,sizeof(theValue));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, Array<int>& theArray, String theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_INTARRAY;
	aRV->mName=theName;
	aRV->mPointer=&theArray;

	GetIntArray(theName,theArray,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theArray.mArray,sizeof(theArray.mArray));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, Array<char>& theArray, String theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_CHARARRAY;
	aRV->mName=theName;
	aRV->mPointer=&theArray;

	GetCharArray(theName,theArray,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theArray.mArray,sizeof(theArray.mArray));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, Array<bool>& theArray, String theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_BOOLARRAY;
	aRV->mName=theName;
	aRV->mPointer=&theArray;

	GetBoolArray(theName,theArray,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theArray.mArray,sizeof(theArray.mArray));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, Array<String>& theArray, String theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_STRINGARRAY;
	aRV->mName=theName;
	aRV->mPointer=&theArray;

	GetStringArray(theName,theArray,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theArray.mArray,sizeof(theArray.mArray));
		gAppPtr->mBackgroundProcesses+=this;
	}
}


void Settings::RegisterVariable(String theName, float& theValue, float theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_FLOAT;
	aRV->mName=theName;
	aRV->mPointer=&theValue;

	theValue=GetFloat(theName,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theValue,sizeof(theValue));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, bool& theValue, bool theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_BOOL;
	aRV->mName=theName;
	aRV->mPointer=&theValue;

	theValue=GetBool(theName,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theValue,sizeof(theValue));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, Point& theValue, Point theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_POINT;
	aRV->mName=theName;
	aRV->mPointer=&theValue;

	theValue=GetPoint(theName,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theValue,sizeof(theValue));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, IPoint& theValue, IPoint theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_IPOINT;
	aRV->mName=theName;
	aRV->mPointer=&theValue;

	theValue=GetIPoint(theName,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theValue,sizeof(theValue));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, char& theValue, char theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_CHAR;
	aRV->mName=theName;
	aRV->mPointer=&theValue;

	theValue=GetChar(theName,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theValue,sizeof(theValue));
		gAppPtr->mBackgroundProcesses+=this;
	}
}

void Settings::RegisterVariable(String theName, String& theValue, String theDefault)
{
	RegisteredVariable *aRV=new RegisteredVariable;
	mRegisterList+=aRV;

	aRV->mType=VARTYPE_STRING;
	aRV->mName=theName;
	aRV->mPointer=&theValue;

	theValue=GetString(theName,theDefault);

	//if (mWatchRegisterVariables)
	{
		aRV->Watch(&theValue.mData,sizeof(theValue.mData));
		gAppPtr->mBackgroundProcesses+=this;
	}
}
#endif

void Settings::ThreadLock(bool doCreate)
{
	if (mThreadLock==-1) 
	{
		if (doCreate) mThreadLock=CreateThreadLock();
		else return;
	}
	::ThreadLock(mThreadLock);
}

void Settings::ThreadUnlock()
{
	if (mThreadLock>=0) ::ThreadUnlock(mThreadLock);
}


