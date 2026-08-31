#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#define ISUNIX false
#else
#define ISUNIX true
#endif

#ifdef __HEADER
#define _PASTE(a,b) a ## b
#define PASTE(a,b) _PASTE(a, b)
#define _FixPath(path) char PASTE(aResult,__LINE__)[MAX_PATH+1];Common::FixPath(path,PASTE(aResult,__LINE__));path=PASTE(aResult,__LINE__);
namespace Common
{
	void FixPath(char* thePath, char* theResult);
	void GetCustomPath(char* thePrefix, char* theResult);	// Implemented OUTSIDE, in rapt_app.cpp, so that we can have it call into the app.

	struct AliasStruct
	{
		std::string mPrefix;
		std::string mAlias;
	};

	void SetAlias(char* thePrefix, char* theAlias);
	void FixAlias(std::string& theString);

	//
	// kittyengine local-assets loader.  This build ships placeholder art and
	// sound; anyone who owns the original data can point KITTYENGINE_ASSETS at
	// their own copy of the game's Resources folder and get it back.  The
	// substitution is PER FILE -- a folder holding only some of the originals
	// still works, and everything it does not hold keeps coming from the pack.
	//
	const char* AssetOverrideRoot();
	bool AssetFileExists(const char* thePath);
}

#endif

#ifdef __CPP
namespace Common
{
	std::vector<AliasStruct*>* gAliasList=NULL; // Cute... the App can end up initialized before this, so it had/has to be a pointer.
}
void Common::FixAlias(std::string& theString)
{
	if (!gAliasList) return;
	for (auto aAS:*gAliasList) 
	{
		int aLoc=theString.find(aAS->mPrefix);
		if (aLoc!=std::string::npos) {theString.replace(aLoc,aAS->mPrefix.size(),aAS->mAlias);}
	}
}

void Common::SetAlias(char* thePrefix, char* theAlias)
{
	if (!gAliasList) gAliasList=new std::vector<AliasStruct*>;

	if (strlen(thePrefix)==0) return;
	bool aKiller=false;if (strlen(theAlias)==0) aKiller=true;

	for(std::vector<AliasStruct*>::iterator aIT=std::begin(*gAliasList);aIT!=std::end(*gAliasList);++aIT)
	{
		auto aAS=*aIT;
		if (aAS->mPrefix.compare(thePrefix)==0) 
		{
			if (aKiller) {delete aAS;gAliasList->erase(aIT);}
			else aAS->mAlias=theAlias;
			return;
		}
	}
	auto aAS=new AliasStruct;
	aAS->mPrefix=thePrefix;
	aAS->mAlias=theAlias;
	gAliasList->push_back(aAS);
}

const char* Common::AssetOverrideRoot()
{
	//
	// Read once: FixPath is on every asset open, and the answer cannot change
	// while the process is alive.
	//
	static bool aChecked=false;
	static std::string aRoot;
	if (!aChecked)
	{
		aChecked=true;
		const char* aEnv=getenv("KITTYENGINE_ASSETS");
		if (aEnv && *aEnv) aRoot=aEnv;
	}
	return aRoot.empty()?NULL:aRoot.c_str();
}

bool Common::AssetFileExists(const char* thePath)
{
	FILE* aFile=fopen(thePath,"rb");
	if (!aFile) return false;
	fclose(aFile);
	return true;
}

void Common::FixPath(char* thePath, char* theResult)
{
	//OS_Core::Printf("FIXPATH(in): %s",thePath);
	std::string aResult=thePath;
	FixAlias(aResult);

	//OS_Core::Printf("fixpath: %s",aResult.c_str());

	bool aDoubleTweak=false;

	int aLoc=aResult.find("://");
	if (aLoc!=std::string::npos)
	{
		aLoc+=3;
		std::string aPrefix=aResult.substr(0,aLoc);
		std::string aPath=aResult.substr(aLoc);

		char aNewPrefix[1024];
		aNewPrefix[0]=0;

		//
		// Profile is common, so we'll make sure it processes first...
		//
		if (aPrefix.compare("desktop://")==0) {OS_Core::GetDesktopFolder(aNewPrefix);}
		else if (aPrefix.compare("sandbox://")==0) {OS_Core::GetSandboxFolder(aNewPrefix);}
		else if (aPrefix.compare("cache://")==0) {OS_Core::GetCacheFolder(aNewPrefix);}
		else if (aPrefix.compare("temp://")==0) {OS_Core::GetTempFolder(aNewPrefix);}
		else if (aPrefix.compare("package://")==0) {OS_Core::GetPackageFolder(aNewPrefix);}
		else if (aPrefix.compare("local://")==0) {OS_Core::GetPackageFolder(aNewPrefix);}
		else if (aPrefix.compare("images://")==0) {OS_Core::GetPackageFolder(aNewPrefix);strcat(aNewPrefix,"images/");}
		else if (aPrefix.compare("data://")==0) {OS_Core::GetPackageFolder(aNewPrefix);strcat(aNewPrefix,"data/");}
		else if (aPrefix.compare("sounds://")==0) {OS_Core::GetPackageFolder(aNewPrefix);strcat(aNewPrefix,"sounds/");}
		else if (aPrefix.compare("music://")==0) {OS_Core::GetPackageFolder(aNewPrefix);strcat(aNewPrefix,"music/");}
		else if (aPrefix.compare("models://")==0) {OS_Core::GetPackageFolder(aNewPrefix);strcat(aNewPrefix,"models/");}
		else if (aPrefix.compare("social://")==0) {OS_Core::GetSandboxFolder(aNewPrefix);strcat(aNewPrefix,"_social/");}
		else if (aPrefix.compare("shaders://")==0) {OS_Core::GetPackageFolder(aNewPrefix);strcat(aNewPrefix,"shaders/");}
		else if (aPrefix.compare("scratch://")==0) {OS_Core::GetTempFolder(aNewPrefix);strcat(aNewPrefix,"_scratch/");}
		else if (aPrefix.compare("content://")==0) {strcpy(theResult,thePath);return;} // Reserved on Android
		else if (aPrefix.compare("intent://")==0) {strcpy(theResult,thePath);return;} // Reserved on Android
		#ifdef _LINUX
		else if (aPrefix.compare("home://")==0) {OS_Core::GetHomeFolder(aNewPrefix);}
		#endif

		//
		// Avoid using "cloud://" folder... I originally established that for the purpose of having a section
		// that would upload.  But the "new userbase" expects everything to be stored, always.  So just put
		// everything in Sandbox, and we'll have Sandbox dump up.
		//
		else if (aPrefix.compare("cloud://")==0) 
		{
			#ifdef _DEBUG
			OS_Core::Printf("!Hey! cloud:// is depreciated-- everything is just sandbox.");
			#endif			
			OS_Core::GetCloudFolder(aNewPrefix);
		}
		else {GetCustomPath((char*)aPrefix.c_str(),aNewPrefix);aDoubleTweak=true;}

		aResult=aNewPrefix;
		aResult+=aPath;

		//
		// kittyengine local-assets loader (see the declaration above).  Only the
		// schemes that resolve INSIDE the package folder are redirectable: the
		// sandbox, the cache and the temp folder are the player's own state and
		// have nothing to do with which assets are installed.
		//
		const char* aOverride=AssetOverrideRoot();
		if (aOverride && !aDoubleTweak)
		{
			char aPackage[MAX_PATH+1];aPackage[0]=0;
			OS_Core::GetPackageFolder(aPackage);
			std::string aResolved=aNewPrefix;
			std::string aBase=aPackage;
			if (aBase.size() && aResolved.compare(0,aBase.size(),aBase)==0)
			{
				std::string aTry=aOverride;
				if (aTry.size() && aTry[aTry.size()-1]!='/' && aTry[aTry.size()-1]!='\\') aTry+="/";
				aTry+=aResolved.substr(aBase.size());
				aTry+=aPath;
				if (AssetFileExists(aTry.c_str())) aResult=aTry;
			}
		}
	}

	char* aPath=(char*)aResult.c_str();
	if (aDoubleTweak) FixPath(aPath,theResult); // Means we got a custom path from the MyApp, so it needs to be processed again...
	else
	{
		if (ISUNIX) {char *aPtr=aPath;while (*aPtr!=0) {if (*aPtr=='\\') *aPtr='/';aPtr++;}}
		else {char *aPtr=aPath;while (*aPtr!=0) {if (*aPtr=='/') *aPtr='\\';aPtr++;}}
		strncpy(theResult,aResult.c_str(),MAX_PATH);
	}
	
	//OS_Core::Printf("  FIXPATH(out): %s",theResult);
}

#endif

#ifdef __HELPERS
inline int QRand(int theSeed)
{
    int aSeed=(214013*theSeed+2531011);
    return (aSeed>>16)&0x7FFF;
}

//
// This horrifying template complexity makes it so that the arrays start at  if they're primitives,
// or if they're pointers, but not if you make, say, an array of classes that might be constructing stuff.
//
// So if you did:
//
//		Array<Class*> mArray;
//		mArray[2]=blah;
//
// ...then you can count on mArray[0] and mArray[1] being null.
//
// BUT, if you did:
//
//		struct Something {int mX;}
//		Array<Something> mArray;
//		mArray[2].mX=blah;
//
// ...then mArray[0].mX and mArray[1].mX are undefined, unless you set them in the constructor of the array.
//
// Note that this only happens with the CleanArray class (or if you set mZeroData to true) in order to keep it
// quarantined, and give an option to not use the functionality unless needed.  This was originally put in because
// the sprite BillboardShape array was non-zero and had no good way to zero it efficiently.
// 

template<typename T> struct IsPointer { static const bool value = false; };
template<typename T> struct IsPointer<T*> { static const bool value = true; };
template <typename var_type> inline void SetArrayData(var_type* thePos,char theData, int theLen) {}
//
// Note to self: theLen in the functions below already has sizeof factored in.  Don't do theLen*sizeof(type)
//
template <> inline void SetArrayData<char>(char* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<bool>(bool* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<int>(int* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<float>(float* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<unsigned int>(unsigned int* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<unsigned char>(unsigned char* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<double>(double* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}

//
// Note: Use CleanArray if you need values zeroed out
// (And it won't zero values out for classes anyway)
//

#include <iterator>

template <typename var_type>
class Array
{
public:
	Array() {mArray=NULL;mSize=0;mLockSize=false;mZeroData=false;}
	Array(int initialSize) {mArray=NULL;mSize=0;mLockSize=false;GuaranteeSize(initialSize);mZeroData=false;}
	Array(int initialSize, var_type theValue) {mArray=NULL;mSize=0;mLockSize=false;Reset(initialSize,theValue);mZeroData=false;}
	Array(Array<var_type>& theValue) {mArray=NULL;mSize=0;mLockSize=0;mZeroData=theValue.mZeroData;Copy(theValue);}
	~Array() {Reset();}

	var_type		*mArray=NULL;
	int				mSize=0;
	inline int		Size() {return mSize;}
	inline float	SizeF() {return (float)mSize;}

	int				Hash() {return (int)PtrToLong(mArray+mSize);}
	
	union
	{
		char mFlags=0;
		struct
		{
			char			mLockSize:1;
			char			mModLock:1;
			char			mZeroData:1;
		};
	};

	void Reset() {delete [] mArray;mArray=NULL;mSize=0;mLockSize=false;}
	void Reset(int theSize) {delete [] mArray;mArray=NULL;mSize=0;Reset(theSize,NULL);mLockSize=false;}
	void Reset(int theSize, var_type theValue) {_DeleteArray(mArray);mArray=NULL;mSize=0;GuaranteeSize(theSize);for (int aCount=0;aCount<Size();aCount++) mArray[aCount]=theValue;mLockSize=false;}

	inline void ModLock(bool theState=true) {mModLock=theState;} // When true, elements fetched will always be loc%size
	inline void Wrap() {ModLock(true);}

	inline operator void* () {return (void*)mArray;}
	
	Array(std::initializer_list<var_type> theValues)
	{
		GuaranteeSize(theValues.size());
		int aSlot=0;
		for (auto aItem=theValues.begin();aItem!=theValues.end();++aItem) {Element(aSlot++)=*aItem;}	
	}
	
	Array<var_type>&				operator=(Array<var_type>& rhs)
	{
		Copy(rhs);
		return *this;
	}
	Array<var_type>&				operator=(std::initializer_list<var_type> theValues)
	{
		Reset();GuaranteeSize(theValues.size());
		int aSlot=0;
		for (auto aItem=theValues.begin();aItem!=theValues.end();++aItem) {Element(aSlot++)=*aItem;}
		return *this;
	}

	inline void			Copy(Array<var_type>& rhs)
	{
		if (this==&rhs) return;
		Reset();
		GuaranteeSize(rhs.Size());
		for (int aCount=0;aCount<rhs.Size();aCount++) Element(aCount)=rhs.Element(aCount);
	}
	inline void			Copy(Array<var_type>& rhs, int theStart, int theEndIncluding)
	{
		Reset();
		GuaranteeSize((theStart-theEndIncluding)+1);
		int aAdd=0;for (int aCount=theStart;aCount<=theEndIncluding;aCount++) Element(aAdd++)=rhs.Element(aCount);
	}

	inline var_type&	operator [] (int theSlot) {return Element(theSlot);}

	inline int			FixSlot(int theSlot)
	{
		if (mModLock) 
		{
			if (mSize!=0) theSlot%=mSize;
			if (theSlot<0) theSlot+=mSize;
		}
		else if (theSlot>=mSize) GuaranteeSize(theSlot+1);
		return theSlot;
	}
	var_type&			Element(int theSlot) {int aNew=FixSlot(theSlot);return mArray[aNew];}
	//var_type&			Element(int theSlot) {if (theSlot>=mSize) GuaranteeSize(theSlot+1);return mArray[theSlot];}
	/*
	var_type&			Element(int theSlot, var_type theDefault) 
	{
		if (theSlot>=mSize || theSlot<0)
		{
			if (mModLock) 
			{
				theSlot=theSlot%mSize;
				if (theSlot<0) theSlot+=mSize;
				//theSlot=abs(theSlot)%mSize;
			}
			else if (theSlot<0) theSlot=0;
			{
				GuaranteeSize(theSlot+1);
				mArray[theSlot]=theDefault;
			}
		};
		return mArray[theSlot];
	}
	*/

	inline var_type&	Set(int theSlot) {return Element(theSlot);}

	inline var_type&	First() {return Element(0);}
	inline var_type&	Last() {return Element(_max(0,Size()-1));}
	inline var_type&	AddLast() {return Element(Size());}
	virtual void operator+=		(var_type theObject) {Element(Size())=theObject;}
	virtual void		Add(var_type theObject) {Element(Size())=theObject;}

	inline var_type&	ElementAtScalar(float theScalar) {return Element((int)(((float)Size()*_clamp(0.0f,theScalar,1.0f))+.5f));}
	inline var_type&	ElementAtScalar(int theScalar) {return theScalar?Last():First();}
	inline var_type&	ElementAtInterval(float theScalar) {return ElementAtScalar(theScalar);}
	inline var_type&	ElementAtInterval(int theScalar) {return ElementAtScalar(theScalar);}

	inline void operator+=		(Array<var_type> &theOther) {Append(theOther);}
	void						Append(Array<var_type> &theOther)
	{
		int aCursor=Size();
		GuaranteeSize(Size()+theOther.Size());
		for (int aCount=0;aCount<theOther.Size();aCount++) Element(aCount+aCursor)=theOther.Element(aCount);
	}

	inline void			Clip(int theSize)
	{
		if (mSize<theSize) return;
		
		int aOldSize=mSize;
		mSize=theSize;
		var_type *aNewArray=new var_type[theSize];

		if (mArray)
		{
			for (int aCount=0;aCount<_min(aOldSize,mSize);aCount++) aNewArray[aCount]=mArray[aCount];
			if (mZeroData && mSize>aOldSize) ZeroData(&aNewArray[aOldSize],(mSize-aOldSize)*sizeof(var_type));
			_DeleteArray(mArray);
		}
		else if (mZeroData) ZeroData(aNewArray,(theSize)*sizeof(var_type));
		mArray=aNewArray;
	}

	inline int			ClampIndex(int theIndex) {return _clamp(0,theIndex,mSize-1);}
	inline int			WrapIndex(int theIndex) {return theIndex%(mSize);}
	inline void			SetSize(int theSize) {GuaranteeSize(theSize);}
	inline void			InitialSize(int theSize) {GuaranteeSize(theSize);}
	inline void			StartSize(int theSize) {GuaranteeSize(theSize);}
	inline bool			IsEmpty() {return mSize==0;}

	void GuaranteeSize(int theSize, bool lockSize=false)
	{
		if (theSize<=mSize) return;
		if (mLockSize || theSize<=0) return;
		mLockSize=lockSize;

		int aOldSize=mSize;
		var_type *aNewArray=NULL;
		aNewArray=new var_type[theSize];
		mSize=theSize;

		if (aNewArray)
		{
			if (mArray)
			{
				for (int aCount=0;aCount<aOldSize;aCount++) aNewArray[aCount]=mArray[aCount];
				if (mZeroData) ZeroData(&aNewArray[aOldSize],(mSize-aOldSize)*sizeof(var_type));
				delete [] mArray;
			}
			else if (mZeroData) ZeroData(aNewArray,(theSize)*sizeof(var_type));
		}
		mArray=aNewArray;
		aNewArray=NULL;
	}
    
	inline void ZeroData(var_type* theData, int theSize)
	{
		memset(theData,0,theSize);
		//if (IsPointer<var_type>::value) memset(theData,0,theSize);
		//else SetArrayData(theData,0,theSize);
	}
	inline void Clean() {ZeroData(mArray,Size()*sizeof(var_type));}
	inline void ZeroData() {Clean();}
	inline void ForceSize(int theSize) {if (Size()>theSize) Clip(theSize); if (Size()<theSize) GuaranteeSize(theSize);}
	inline void SetAll(var_type theValue) {var_type* aPtr=mArray;for (int aCount=0;aCount<Size();aCount++) {*aPtr=theValue;aPtr++;}}
	
	void InsertElement(var_type theObject, int thePos=0)
	{
		GuaranteeSize(Size()+1);
		for (int aCount=Size()-1;aCount>thePos;aCount--) Element(aCount)=Element(aCount-1);
		Element(thePos)=theObject;
	}

	void Insert(Array<var_type>& theArray, int thePos=0, int howMany=INT_MAX)
	{
		howMany=_min(howMany,theArray.Size());

		if (thePos>Size()) Append(theArray);
		else
		{
			Array<var_type> aNew;aNew.GuaranteeSize(howMany+Size());
			int aWritePos=0;
			for (int aCount=0;aCount<thePos;aCount++) aNew[aWritePos++]=Element(aCount);
			for (int aCount=0;aCount<howMany;aCount++) aNew[aWritePos++]=theArray[aCount];
			for (int aCount=thePos;aCount<Size();aCount++) aNew[aWritePos++]=Element(aCount);
			Reset();
			mArray=aNew.mArray;
			mSize=aNew.mSize;
			aNew.mArray=NULL;
			aNew.mSize=0;
		}

		/*
		if (thePos>Size()) Append(theArray);
		else
		{
			int aOldSize=Size();
			GuaranteeSize(Size()+howMany);
			for (int aCount=thePos;aCount<aOldSize;aCount++) Element(aCount+howMany)=Element(aCount);
			for (int aCount=0;aCount<howMany;aCount++) Element(thePos+aCount)=theArray[aCount];
		}
		/**/
	}

	int InsertElementSorted(var_type theValue, char theDir=1)
	{
		if (theDir!=-1) {int aCount;for (aCount=0;aCount<Size();aCount++) if (theValue<Element(aCount)) break;InsertElement(theValue,aCount);return aCount;}
		else {int aCount;for (aCount=0;aCount<Size();aCount++) if (theValue>Element(aCount)) break;InsertElement(theValue,aCount);return aCount;}
	}
	inline int InsertElementSortedReverse(var_type theObject) {return InsertElementSorted(theObject,-1);}
	
	int InsertElementSortedNoDuplicates(var_type theValue, var_type theEpsilon=1, char theDir=1)
	{
		{int aCount;for (aCount=0;aCount<Size();aCount++) if (abs(Element(aCount)-theValue)<theEpsilon) return aCount;}
		if (theDir!=-1) {int aCount;for (aCount=0;aCount<Size();aCount++) if (theValue<Element(aCount)) break;InsertElement(theValue,aCount);return aCount;}
		else {int aCount;for (aCount=0;aCount<Size();aCount++) if (theValue>Element(aCount)) break;InsertElement(theValue,aCount);return aCount;}
	}

	int InsertElementSortedReplaceDuplicates(var_type theValue, var_type theEpsilon=1, char theDir=1)
	{
		{int aCount;for (aCount=0;aCount<Size();aCount++) if (abs(Element(aCount)-theValue)<theEpsilon) {Element(aCount)=theValue;return aCount;}}
		if (theDir!=-1) {int aCount;for (aCount=0;aCount<Size();aCount++) if (theValue<Element(aCount)) break;InsertElement(theValue,aCount);return aCount;}
		else {int aCount;for (aCount=0;aCount<Size();aCount++) if (theValue>Element(aCount)) break;InsertElement(theValue,aCount);return aCount;}
	}


   
	void DeleteElement(int theSlot)
	{
		if (theSlot<0 || theSlot>=Size()) return;
		if (Size()==1) {Reset();return;}

		var_type* aNewArray=new var_type[Size()-1]; 
		int aNewPtr=0;
		for (int aCount=0;aCount<Size();aCount++)
		{
			if (aCount!=theSlot)
			{
				aNewArray[aNewPtr]=mArray[aCount];
				aNewPtr++;
			}
		}

		delete [] mArray;
		mArray=aNewArray;
		mSize--;
	}
	inline void Truncate(int theNewSize)
	{
		if (theNewSize>=Size()) return;

		var_type* aNewArray=new var_type[theNewSize]; 
		int aNewPtr=0;
		for (int aCount=0;aCount<theNewSize;aCount++)
		{
			aNewArray[aNewPtr]=mArray[aCount];
			aNewPtr++;
		}
		delete [] mArray;
		mArray=aNewArray;
		mSize=theNewSize;
	}

	void Shuffle(int theRandomSeed)
	{
		int aSeed=theRandomSeed;
		for (int aCount=0;aCount<Size();aCount++)
		{
			aSeed=QRand(aSeed);
			int aOther=aSeed%Size();
			var_type aHold=Element(aCount);
			Element(aCount)=Element(aOther);
			Element(aOther)=aHold;
		}
	}

	int FindValue(var_type aValue)
	{
		for (int aCount=0;aCount<Size();aCount++) if (Element(aCount)==aValue) return aCount;
		return -1;
	}
	
	inline bool Exists(var_type aValue) {return (FindValue(aValue)>=0);}

	void SwapValues(var_type aValue1, var_type aValue2)
	{
		int aValue1Slot=FindValue(aValue1);
		int aValue2Slot=FindValue(aValue2);
		if (aValue1Slot>=0 && aValue2Slot>=0)
		{
			Element(aValue2Slot)=aValue1;
			Element(aValue1Slot)=aValue2;
		}
	}

	var_type* begin() {if (Size()==0) return NULL;return &First();}
	var_type* end() {if (Size()==0) return NULL;return &Last()+1;}

	void Swap(int theSlot1, int theSlot2)
	{
		var_type aHold=Element(theSlot1);
		Element(theSlot1)=Element(theSlot2);
		Element(theSlot2)=aHold;
	}

	//
	// Quicksorts...
	//
	int __Sort(int theP, int theQ) {var_type aX=Element(theP);int aI=theP;for(int aJ=theP+1;aJ<theQ;aJ++) {var_type aO=Element(aJ);if(aO<aX) {aI=aI+1;Swap(aI,aJ);}}Swap(aI,theP);return aI;}
	void Sort(int theStart=0, int theEnd=-1) {if (theEnd==-1) theEnd=Size();if (theStart<theEnd){int aR=__Sort(theStart,theEnd);Sort(theStart,aR);Sort(aR+1,theEnd);}}


	//
	// Example use:
	// 
	// Array<Vector> aSlime;
	// Vector aSlime;
	// aV.SortByOffsetF((longlong)(&aSlime.mZ)-(longlong)(&aSlime));
	//
	inline int __SortByOffsetF(int theOffset, int theP, int theQ) {char* aX=(char*)&Element(theP);float* aSortX=(float*)(aX+theOffset);int aI=theP;for(int aJ=theP+1;aJ<theQ;aJ++) {char* aO=(char*)&Element(aJ);float* aSortO=(float*)(aO+theOffset);if(*aSortO<*aSortX) {aI=aI+1;Swap(aI,aJ);}}Swap(aI,theP);return aI;}
	inline void SortByOffsetF(int theOffset, int theStart=0, int theEnd=-1) {if (theEnd==-1) theEnd=Size();if (theStart<theEnd){int aR=__SortByOffsetF(theOffset, theStart,theEnd);SortByOffsetF(theOffset, theStart,aR);SortByOffsetF(theOffset,aR+1,theEnd);}}
	inline void SortByOffsetF(void* theBase, void* theVar, int theStart=0, int theEnd=-1) {SortByOffsetF((longlong)theVar-(longlong)theBase,theStart,theEnd);}
	//
	// Allows culling elements by sending a bool array
	// Where TRUE, it will cull the element, where false it will keep the element.
	//
	void Cull(Array<bool>& theCullArray)
	{
		if (theCullArray.Size()!=Size()) return;
		for (int aCount=theCullArray.Size()-1;aCount>=0;aCount--) {if (theCullArray[aCount]) DeleteElement(aCount);}
	}

};


//
// ReadOnlyArray does NO bounds checking or expanding.
// You MUST call GuaranteeSize before using it.  This exists mainly as compatibility for porting from other languages, where
// you have an array of data and it has specific size, but you need the length of the array constantly in the code... but you
// don't want the overhead every time you call it of checking if you're in the bounds.
//
// Again, only difference is no bounds checking when you read/set items.  MUST CALL GUARANTEESIZE!
//
template <typename var_type>
class ReadOnlyArray : public Array<var_type>
{
public:
	inline var_type&	operator [] (int theSlot) {return this->Element(theSlot);}
	var_type&			Element(int theSlot) {return this->mArray[theSlot];}

	//
	// If you do need to expand a read only array, use "Set"
	//
};

//
// Use a CleanArray when you need the data to be zeroed
//
template <typename var_type>
class CleanArray : public Array<var_type>
{
public:
    CleanArray() {Array<var_type>::mArray=NULL;Array<var_type>::mSize=0;Array<var_type>::mLockSize=false;Array<var_type>::mZeroData=true;}
	CleanArray(int initialSize) {Array<var_type>::mArray=NULL;Array<var_type>::mSize=0;Array<var_type>::mLockSize=false;Array<var_type>::mZeroData=true;Array<var_type>::GuaranteeSize(initialSize);}
	CleanArray(int initialSize, var_type theValue) {Array<var_type>::mArray=NULL;Array<var_type>::mSize=0;Array<var_type>::mLockSize=false;Array<var_type>::mZeroData=true;Array<var_type>::Reset(initialSize,theValue);}
};

template <class var_type>
class Stack : public Array<var_type>
{
public:
	Stack() {mStackPos=0;}
	
	inline void		Push(var_type theValue) {this->Element(this->mStackPos)=theValue;this->mStackPos++;}
	inline var_type	Pop() {this->mStackPos=_max(0,this->mStackPos-1);return this->Element(mStackPos);}
	inline var_type	Peek() {return this->Element(_max(0,mStackPos-1));}
	inline var_type	Peek(int howFarBack) {return this->Element(_max(0,mStackPos-howFarBack));}
	inline void		Empty() {this->mStackPos=0;}
	inline bool		IsEmpty() {return (this->mStackPos<=0);}
	inline int		GetStackCount() {return mStackPos;}
	void			Copy(Stack<var_type>& rhs)
	{
		if (this==&rhs) return;
		this->Reset();
		this->ForceSize(rhs.Size());
		for (int aCount=0;aCount<rhs.Size();aCount++) this->Element(aCount)=rhs.Element(aCount);
		mStackPos=rhs.mStackPos;
	}

	//
	// Removes an item from the stack.
	//
	void			Unpush(var_type theItem)
	{
		int aStart=-1;
		for (int aCount=0;aCount<mStackPos;aCount++) if (this->Element(aCount)==theItem) {aStart=aCount;break;}
		if (aStart>=0) {mStackPos--;for (int aCount=aStart;aCount<mStackPos;aCount++) this->Element(aCount)=this->Element(aCount+1);}
	}

public:
	int				mStackPos;
};
#endif
