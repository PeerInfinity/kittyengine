#pragma once

#include "rapt_headers.h"

//#define _TRACESYNC


#define IOBUFFERHOOK(func) [=](void *theArg) {IOBuffer* theBuffer=(IOBuffer*)theArg;if (theBuffer->IsDownloadOK()) {func}}
#define IOBUFFERHOOKPTR std::function<void(void* theArg)>

// SYNCHOOKPTR expects to be within a template with type stype
#define SYNCHOOK(stype,func) [&](stype& theArg) func
#define SYNCHOOKPTR() std::function<void(stype& theArg)>

enum
{
	IOBUFFER_STATUS_DATAISPOINTER=0x01,
	IOBUFFER_STATUS_READONLY=0x02,
	IOBUFFER_STATUS_DOWNLOADING=0x04,
};

//
// These includes can be removed, for cross compatibility
//

//
// Some global functions, just to wrap
// some more obscure file functions
//
bool DoesFileExist(String theFilename);
bool DoesFileExist(String theFilename, String theVersion);
void MakeDirectory(String theFilename);
void DeleteDirectory(String theDirectoryName);
void DeleteDirectoryContents(String theDirectoryName);
bool RenameFile(String theOldName, String theNewName);
bool CopyFile(String theOldName, String theNewName);
inline bool RenameDirectory(String theOldName, String theNewName) {return RenameFile(theOldName,theNewName);}
void EnumFiles(String theDirectoryName, Array<String> &theArray);
void RemoveFile(String theFilename);
inline void KillFile(String theFilename) {RemoveFile(theFilename);}
inline void DelFile(String theFilename) {RemoveFile(theFilename);}
inline void Unlink(String theFilename) {RemoveFile(theFilename);}
inline void DeleteFile(String theFilename) {RemoveFile(theFilename);}
void RemoveFileFromCloud(String theFilename);
void Execute(String theCommand);
void OpenURL(String theURL);
void FileToArray(String theFilename,Array<String>& theArray);
void ArrayToFile(String theFilename,Array<String>& theArray);
void DeleteCacheFiles(String theFolder);
bool IsDirectory(String thePath);
inline bool DoesDirectoryExist(String thePath) {return IsDirectory(thePath);}
void ClearTemp(String theSubDir="");
void CopyDirectory(String theSource, String theDestination, float* theProgress=NULL);
String FilenameToCloudKey(String theFN);	// Takes a filename and turns it into a cloud key compatible thing
String FileToString(String theFilename);

String GetTruePath(String thePath);	// Gets the "true" path on the local OS

//
// Enums all files/folders in the directory (with paths)
//
void EnumDirectory(String theDirectoryName, Array<String>& theArray, bool includeSubdirs=false);
void EnumDirectory_Legacy(String theDirectoryName, Array<String>& theArray, bool includeSubdirs=false); // For legacy stuff that used '::' to indicate folders...

//
// Enums just the contents of a single directory (makes it easier to, say, quickly get all the subdirs in a folder for a 
// profile list, or quickly get all the files in a folder for browsing, say, custom levels)
//
void EnumDirectoryFiles(String theDirectoryName, Array<String> &theArray);
void EnumDirectoryFolders(String theDirectoryName, Array<String> &theArray);
void EnumDirectoryFilesAndFolders(String theDirectoryName, Array<String> &theArray);


//
// For putting things into the cloud (if cloud is supported)
//
void EnumCloud(Array<String> &theArray);


//
// Downloads and uncompresses a wad file.  True if it worked, false if not.
//
bool DownloadWad(String theURL, String theVersion, bool forceDownload=false);


void MakeWAD(String theFolder, IOBuffer& theBuffer, String ignoreExtensions="", String ignoreFilenames="", bool isRecursive=false); // extensions: "png;jpg;gif;" ... semicolon after each!
void MakeWAD(String theFolder, String theOutFile, String ignoreExtensions="", String ignoreFilenames="");
void ExtractWAD(IOBuffer& theBuffer, String theOutFolder, Array<String>* theCreatedFiles=NULL);
void ExtractWAD(String theInFile, String theOutFolder, Array<String>* theCreatedFiles=NULL);
bool IsWAD(IOBuffer& theBuffer);
bool IsWAD(String theInFile);

void MakeArchive(String theArchiveName, String theFolder, float* theProgress=NULL, String ignoreExtensions="", String ignoreFilenames="");
void ExtractArchive(String theArchiveName, String theFolder, float* theProgress=NULL);


//String PointAtDesktop(String theString);
//String PointAtSandbox(String theString);
//String PointAtCache(String theString);
//String PointAtPackage(String theString);
//String PointAtImages(String theString);
//String PointAtModels(String theString);
//String PointAtTemp(String theString);
//String PointAtCloud(String theString);
//String FixFN(String theFN);
//
// This fixes a filename to convert the following:
// desktop://filename.ext -> Point at desktop
// sandbox://filename.ext -> Point at sandbox
// cache://filename.ext -> Point at cache
// temp://filename.ext -> Point at temp
// package://filename.ext -> Point at package
// local: //filename.ext -> Point at package
// images://filename.ext -> Point at package/images
// data://filename.ext -> Point at package/data
// sounds://filename.ext -> Point at package/sounds
// music://filename.ext -> Point at package/music
//
//String RouteSandbox(String theNewSandbox="");	// Routes all sandbox calls somewhere else.  This lets you mass-save stuff into a temporary location.
//String RouteCloud(String theNewCloud="");	// Routes all cloud calls somewhere else.  This lets you mass-save stuff into a temporary location.

String URLToFN(String theURL);
String URLtoWebCache(String theURL, String theVersion="");

String InsertFilePrefix(String thePath, String thePrefix);
String GetFilenameFromPath(String thePath);
String GetPathFromPath(String thePath);
String GetLastFolderFromPath(String thePath);
String GetUpPathFromPath(String thePath);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IOBuffer Class
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IOBuffer
{
public:
	IOBuffer(void);
	virtual ~IOBuffer(void);

	//
	// The sourcing functions will allow you to set
	// a source for the data.
	//
	virtual void		SetFileSource(char* theFilename);
	virtual void		SetFileSource(char* theFilename, int theBytesToRead);
	inline void			Load(char *theFilename) {SetFileSource(theFilename);}
	inline void			Load(char *theFilename, int theBytesToRead) {SetFileSource(theFilename,theBytesToRead);}
	void				Download(String theURL, float *theProgressIndicator=NULL);
	void				DownloadA(String theURL, float *theProgressIndicator=NULL, IOBUFFERHOOKPTR runWhenComplete=NULL);
	bool				IsDownloadOK();

	void				Upload(String theURL, IOBuffer* theResultBuffer, float *theProgressIndicator=NULL);
	void				UploadA(String theURL, IOBuffer* theResultBuffer, float *theProgressIndicator=NULL, IOBUFFERHOOKPTR runWhenComplete=NULL);
	inline bool			IsUploadOK() {return IsDownloadOK();}

	virtual void		SetMemorySource(void *theData, int theDataLen);
	virtual void		SetMemoryPointerSource(void *theData, int theDataLen, bool takeControlofData=false); // use doAdopt if you're taking total control of this memory
	virtual void		Reset(bool freeMemory);
	inline void			Reset() {Reset(true);}
	inline void			Clear() {Reset(true);}

	// Cloud handling stuff...
	bool				CloudLoad(String theKeyName);
	bool				CloudSave(String theKeyName, bool doImmediate=false);

	//
	// For getting CRC to check integrity of data...
	//
	int					GetCRC();
	inline int			GetHash() {return GetCRC();}
	longlong			GetLongCRC();
	inline longlong		GetLongHash() {return GetLongCRC();}


	//
	// Lets you snag the whole file as a string.
	// Useful for when you want to just pull the whole
	// thing in and process it.
	// 
	String				ToString(int theLenToCopy=-1);// {return String(mData);}
	void				LoadFromString(String theString);
	bool				StartsWithTag();

	//
	// Tells us if a file is binary (it's slow!  Checks the whole file looking for anything out of range)
	//
	bool				IsBinary();
	bool				IsWAD();

	//
	// The commit functions allow you to dump the
	// contents of the IO buffer to another location
	//
	void				CommitFile(char *theFilename);

	//
	// Dumps the buffer to gOut, for debugging
	//
	void				Dump(String theMessage="");

	//
	// Various functions for reading info out of the
	// buffer...
	//
	char				ReadChar();
	unsigned char		ReadUChar();
	short				ReadShort();
	vbindex				ReadVBIndex();
	unsigned short		ReadUShort();
	int					ReadInt();
	longlong			ReadLongLong();
	unsigned int		ReadUInt();
	inline unsigned int	ReadLong() {return ReadUInt();}
	String				ReadString();  // This just points to a copy of the string, which is destroyed after IOBuffer is
	void				ReadBytes(void* thePtr);
	int					PeekReadBytes();	// Peeks at how many bytes are going to be read (for storage!)
	float				ReadFloat();
	float				ReadSafeFloat();
	float				ReadSmallFloat();
	float				ReadTinyFloat();	// Reads a float that can be -128 to 128, with 256 levels of precision
	float				ReadTinyUFloat();	// Reads a float that can be -128 to 128, with 256 levels of precision
	float				ReadFraction();		// Reads a 0 - 1 fraction that has 256 steps
	double				ReadDouble();
	bool				ReadBool();
	Matrix				ReadMatrix();

	void 				ReadRaw(void *theDestination, int theSize); // if destination is null, it will tell how many bytes you need to allocate
	void				ReadBuffer(IOBuffer &theBuffer);	// Reads data into an IOBuffer
	GlobalID			ReadGlobalID();

	char				PeekChar(int stepForward);						// Reads a char, but does not advance the cursor.
	bool				IsPeekString(int stepForward, char* theString);	// Tells us if the string is next in the buffer

	//
	// Reads regular text lines
	//
	int					ReadLine(char *theDestination); // if destination is null, it'll tell how much space you need to allocate
	String				ReadLine();
	String				ReadRawLine();

	//
	// Reads ini lines in... in lines come in as plain text, but allow
	// you to put braces around returns to allow them to come in.  
	//
	String				ReadINILine(char theStartBrace='[', char theEndBrace=']');


	//
	// Reads a line as though it were a C++ file.
	// Ignores CRLF, turns tab into a space, and
	// doesn't stop until it hits a semicolon
	//
	String				ReadCLine(String theTerminators=";", bool includeBreaker=false);

	//
	// Various functions for writing info into the
	// buffer...
	//
	void				WriteChar(char theChar);
	void				WriteShort(short theShort);
	void				WriteVBIndex(vbindex theShort);
	void				WriteUShort(unsigned short theShort);
	void				WriteUChar(unsigned char theChar);
	void				WriteInt(int theInt);
	void				WriteUInt(unsigned int theInt);
	inline void			WriteLong(unsigned int theLong) {WriteUInt(theLong);}
	void				WriteLongLong(longlong theInt);
	void				WriteString(char *theString);
	void				WriteString(String theString);
	void				WriteBytes(void* thePtr, int theSize);
	void				WriteFloat(float theFloat);
	void				WriteDouble(double theDouble);
	void				WriteSafeFloat(float theFloat);			// Converts to fixed point
	void				WriteSmallFloat(float theFloat);		// Writes a float that can go from -32767 to 32767, with 256 levels of precision
	void				WriteTinyFloat(float theFloat);			// Writes a float that can go from -128 to 128, with 256 levels of precision
	void				WriteTinyUFloat(float theFloat);		// Writes a float that can go from 0 to 255, with 256 levels of precision
	void				WriteFraction(float theFraction);		// Writes a float from 0-1 that has 256 steps
	void				WriteBool(bool theBool);
	void				WriteMatrix(Matrix& theMatrix);
	void				WriteRaw(void *theMemory, int theSize);
	void				WriteLine(char *theLine);
	void				WriteLine(String theLine) {WriteLine(theLine.c());}
	void				WriteBuffer(IOBuffer &theBuffer);
	void				WriteGlobalID(GlobalID theID);

	//inline void			Write(short theVar) {WriteShort(theVar);}
	//inline void			Write(int theVar) {WriteInt(theVar);}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// This lets us set some "logic" into the file, so we can put in jumps that can be processed, etc.
	//
	int					StartJump() {int aHold=mCursor;WriteInt(0);return aHold;}
	void				WriteJump(int theChunkID) {int aHold=mCursor;mCursor=theChunkID;WriteInt(aHold-mCursor);mCursor=aHold;}
	int					ReadJump() {int aHold=mCursor;int aStep=ReadInt();return aHold+aStep;}
	int					StartShortJump() {int aHold=mCursor;WriteShort(0);return aHold;}
	void				WriteShortJump(int theChunkID) {int aHold=mCursor;mCursor=theChunkID;WriteShort((short)(aHold-mCursor));mCursor=aHold;}
	int					ReadShortJump() {int aHold=mCursor;int aStep=ReadShort();return aHold+aStep;}
	int					StartTinyJump() {int aHold=mCursor;WriteUChar(0);return aHold;}
	void				WriteTinyJump(int theChunkID) {int aHold=mCursor;mCursor=theChunkID;WriteUChar((unsigned char)(aHold-mCursor));mCursor=aHold;}
	int					ReadTinyJump() {int aHold=mCursor;unsigned char aStep=ReadUChar();return aHold+aStep;}
	//
	// So you do this:
	// int aJump=StartJump();
	// ..write your stuff...
	// WriteJump(aJump);
	//
	// Then in reading:
	// int aJumpTo=ReadJump();
	// ..read what you want..
	// Seek(aJumpTo);
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//
	// To make sure we have enough room for data...
	//
	inline void			SetSize(int theSize) {EnsureDataFit(theSize);}
	virtual bool		EnsureDataFit(int theDataSize);
	inline bool			GuaranteeSize(int theDataSize) {return EnsureDataFit(theDataSize-mDataLen);}
	inline bool			PreAlloc(int theDataSize) {bool aResult=EnsureDataFit(theDataSize-mDataLen);Reset(false);return aResult;}

	//
	// Functions to see if we're at the end of the file
	//
	inline bool			IsEnd() {return (mCursor>=mDataLen || (!mData))|mError;}
	inline int			Length() {return mDataLen;}
	inline int			Len() {return mDataLen;}
	inline int			Filesize() {return mDataLen;}

	inline int			MemoryLength() {return mAllocLen;}
	inline int			MemoryLen() {return mAllocLen;}
	inline int			MemLen() {return mAllocLen;}

	//
	// Seeking
	//
	void				Seek(int thePos);
	void				SeekEnd();
	void				SeekBeginning();
	void				Rewind(int howFar);
	void				FastForward(int howFar);

public:
	///////////////////////////////////////////////
	//
	// Just the raw data stored in the IOBuffer,
	// and associated info.
	// 
	char				*mData;
	int					mDataLen;
	int					mAllocLen;			// How much has been alloc'd... so we can pre-alloc things...
	char				mStatus;			// Status bits (see enum up top)
	inline bool			IsDataIsPointer() {return ((mStatus&IOBUFFER_STATUS_DATAISPOINTER)!=0);}
	inline bool			IsReadOnly() {return ((mStatus&IOBUFFER_STATUS_READONLY)!=0);}
	inline bool			IsDownloading() {return ((mStatus&IOBUFFER_STATUS_DOWNLOADING)!=0);}
	inline void			SetDataIsPointer(bool theState) {mStatus|=IOBUFFER_STATUS_DATAISPOINTER;if (!theState) mStatus^=IOBUFFER_STATUS_DATAISPOINTER;}
	inline void			SetReadOnly(bool theState) {mStatus|=IOBUFFER_STATUS_READONLY;if (!theState) mStatus^=IOBUFFER_STATUS_READONLY;}
	void				SetDownloading(bool theState);

	//bool				mDataIsPointer;		// If the memory is a pointer, we don't free it...
	//bool				mReadOnly;			// If the data is read only...
	//
	///////////////////////////////////////////////

	///////////////////////////////////////////////
	//
	// Cursor information, for reading/writing
	// with the IOBuffer.
	//
	int					mCursor;
	inline int			Tell() {return mCursor;}
	inline int			GetCursor() {return mCursor;}
	char*				GetCursorPtr() {return (mData+mCursor);}
	//
	///////////////////////////////////////////////

	///////////////////////////////////////////////
	//
	// For easy copying of IOBuffers
	//
	void operator =			(IOBuffer *theBuffer) {Copy(theBuffer);}
	void operator =			(IOBuffer theBuffer) {Copy(&theBuffer);}
	virtual void			Copy(IOBuffer* theBuffer);
	virtual void			Copy(IOBuffer* theBuffer, int theStartPos, int theLength);
	virtual void			Copy(char* theData, int theLen);
	void					Append(IOBuffer* theBuffer);
	//
	///////////////////////////////////////////////

public:
	///////////////////////////////////////////////
	//
	// This allows us to look for errors or data
	// 
	bool					mError;
	inline bool				IsError() {return mError;}
	inline bool				Error() {return mError;}

	bool					IsRML();



public:

	///////////////////////////////////////////////
	//
	// Special, specific functions that
	// can be torn out.  These are framework
	// and helper specific.
	//
	inline Point		ReadPoint() {Point aPoint;aPoint.mX=ReadFloat();aPoint.mY=ReadFloat();return aPoint;}
	inline void			WritePoint(Point thePoint) {WriteFloat(thePoint.mX);WriteFloat(thePoint.mY);}
	inline IPoint		ReadIPoint() {IPoint aPoint;aPoint.mX=ReadInt();aPoint.mY=ReadInt();return aPoint;}
	inline void			WriteIPoint(IPoint thePoint) {WriteInt(thePoint.mX);WriteInt(thePoint.mY);}
	inline Point		ReadSafePoint() {Point aPoint;aPoint.mX=ReadSafeFloat();aPoint.mY=ReadSafeFloat();return aPoint;}
	inline void			WriteSafePoint(Point thePoint) {WriteSafeFloat(thePoint.mX);WriteSafeFloat(thePoint.mY);}
	inline Point		ReadTinyPoint() {Point aPoint;aPoint.mX=ReadTinyFloat();aPoint.mY=ReadTinyFloat();return aPoint;}
	inline void			WriteTinyPoint(Point thePoint) {WriteTinyFloat(thePoint.mX);WriteTinyFloat(thePoint.mY);}
	inline Point		ReadSmallPoint() {Point aPoint;aPoint.mX=ReadSmallFloat();aPoint.mY=ReadSmallFloat();return aPoint;}
	inline void			WriteSmallPoint(Point thePoint) {WriteSmallFloat(thePoint.mX);WriteSmallFloat(thePoint.mY);}
	inline Vector		ReadTinyVector() {Vector aPoint;aPoint.mX=ReadTinyFloat();aPoint.mY=ReadTinyFloat();aPoint.mZ=ReadTinyFloat();return aPoint;}
	inline void			WriteTinyVector(Vector thePoint) {WriteTinyFloat(thePoint.mX);WriteTinyFloat(thePoint.mY);WriteTinyFloat(thePoint.mZ);}
	inline Vector		ReadSmallVector() {Vector aPoint;aPoint.mX=ReadSmallFloat();aPoint.mY=ReadSmallFloat();aPoint.mZ=ReadSmallFloat();return aPoint;}
	inline void			WriteSmallVector(Vector thePoint) {WriteSmallFloat(thePoint.mX);WriteSmallFloat(thePoint.mY);WriteSmallFloat(thePoint.mZ);}
	inline Vector		ReadVector() {Vector aPoint;aPoint.mX=ReadFloat();aPoint.mY=ReadFloat();aPoint.mZ=ReadFloat();return aPoint;}
	inline void			WriteVector(Vector thePoint) {WriteFloat(thePoint.mX);WriteFloat(thePoint.mY);WriteFloat(thePoint.mZ);}
	inline Vector		ReadSafeVector() {Vector aPoint;aPoint.mX=ReadSafeFloat();aPoint.mY=ReadSafeFloat();aPoint.mZ=ReadSafeFloat();return aPoint;}
	inline void			WriteSafeVector(Vector thePoint) {WriteSafeFloat(thePoint.mX);WriteSafeFloat(thePoint.mY);WriteSafeFloat(thePoint.mZ);}
	inline IVector		ReadIVector() {IVector aPoint;aPoint.mX=ReadInt();aPoint.mY=ReadInt();aPoint.mZ=ReadInt();return aPoint;}
	inline void			WriteIVector(IVector thePoint) {WriteInt(thePoint.mX);WriteInt(thePoint.mY);WriteInt(thePoint.mZ);}
	//
	inline Color		ReadColor() {Color aColor;aColor.mR=ReadFloat();aColor.mG=ReadFloat();aColor.mB=ReadFloat();aColor.mA=ReadFloat();return aColor;}
	inline void			WriteColor(Color theColor) {WriteFloat(theColor.mR);WriteFloat(theColor.mG);WriteFloat(theColor.mB);WriteFloat(theColor.mA);}
	//
	inline Rect			ReadRect() {Rect aRect;aRect.mX=ReadFloat();aRect.mY=ReadFloat();aRect.mWidth=ReadFloat();aRect.mHeight=ReadFloat();return aRect;}
	inline void			WriteRect(Rect theRect) {WriteFloat(theRect.mX);WriteFloat(theRect.mY);WriteFloat(theRect.mWidth);WriteFloat(theRect.mHeight);}
	inline IRect		ReadIRect() {IRect aRect;aRect.mX=ReadInt();aRect.mY=ReadInt();aRect.mWidth=ReadInt();aRect.mHeight=ReadInt();return aRect;}
	inline void			WriteIRect(IRect theRect) {WriteInt(theRect.mX);WriteInt(theRect.mY);WriteInt(theRect.mWidth);WriteInt(theRect.mHeight);}
	inline XYRect		ReadXYRect() {XYRect aRect;aRect.mX1=ReadInt();aRect.mY1=ReadInt();aRect.mX2=ReadInt();aRect.mY2=ReadInt();return aRect;}
	inline void			WriteXYRect(XYRect theRect) {WriteInt(theRect.mX1);WriteInt(theRect.mY1);WriteInt(theRect.mX2);WriteInt(theRect.mY2);}
	inline AABBRect		ReadAABBRect() {AABBRect aRect;aRect.mX1=ReadFloat();aRect.mY1=ReadFloat();aRect.mX2=ReadFloat();aRect.mY2=ReadFloat();return aRect;}
	inline void			WriteAABBRect(AABBRect theRect) {WriteFloat(theRect.mX1);WriteFloat(theRect.mY1);WriteFloat(theRect.mX2);WriteFloat(theRect.mY2);}
	inline Quad			ReadQuad() {Quad aQuad;aQuad.mCorner[0]=ReadPoint();aQuad.mCorner[1]=ReadPoint();aQuad.mCorner[2]=ReadPoint();aQuad.mCorner[3]=ReadPoint();return aQuad;}
	inline void			WriteQuad(Quad theQuad) {WritePoint(theQuad.mCorner[0]);WritePoint(theQuad.mCorner[1]);WritePoint(theQuad.mCorner[2]);WritePoint(theQuad.mCorner[3]);}

	inline Cube			ReadCube() {Cube aCube;aCube.mX=ReadFloat();aCube.mY=ReadFloat();aCube.mZ=ReadFloat();aCube.mXSize=ReadFloat();aCube.mYSize=ReadFloat();aCube.mZSize=ReadFloat();return aCube;}
	inline void			WriteCube(Cube theCube) {WriteFloat(theCube.mX);WriteFloat(theCube.mY);WriteFloat(theCube.mZ);WriteFloat(theCube.mXSize);WriteFloat(theCube.mYSize);WriteFloat(theCube.mZSize);}
	inline void			WritePlane(Plane& thePlane) {WriteVector(thePlane.mPos);WriteVector(thePlane.mNormal);WriteFloat(thePlane.mD);}
	inline Plane		ReadPlane() {Plane aP;aP.mPos=ReadVector();aP.mNormal=ReadVector();aP.mD=ReadFloat();return aP;}


	void				ReadRandom(Random& theRandom);
	void				WriteRandom(Random& theRandom);

	void				ReadSuperbits(SuperBits& theBits);
	void				WriteSuperbits(SuperBits& theBits);
	//
	///////////////////////////////////////////////

	//
	// Misc reads...
	//
	//inline void			Read(short& theVar) {theVar=ReadShort();}
	//inline void			Read(int& theVar) {theVar=ReadInt();}

public:

	///////////////////////////////////////////////
	//
	// Helper functions to help us do reading and
	// writing with the same function.
	//
	bool				mQuickSyncType;
	inline void			QuickSyncWrite() {mQuickSyncType=false;}
	inline void			QuickSyncRead() {mQuickSyncType=true;}
	inline bool			IsQuickSyncWrite() {return !mQuickSyncType;}
	inline bool			IsQuickSyncRead() {return mQuickSyncType;}
	inline void			Sync(char* theChar) {if (mQuickSyncType) *theChar=ReadChar(); else WriteChar(*theChar);}
	inline void			Sync(short* theShort) {if (mQuickSyncType) *theShort=ReadShort(); else WriteShort(*theShort);}
	inline void			Sync(unsigned short* theShort) {if (mQuickSyncType) *theShort=ReadUShort(); else WriteUShort(*theShort);}
	inline void			Sync(unsigned char* theChar) {if (mQuickSyncType) *theChar=ReadUChar(); else WriteUChar(*theChar);}
	inline void			Sync(int* theInt) {if (mQuickSyncType) *theInt=ReadInt(); else WriteInt(*theInt);}
    inline void			Sync(unsigned int* theInt) {if (mQuickSyncType) *theInt=ReadUInt(); else WriteUInt(*theInt);}
	inline void			Sync(longlong* theInt) {if (mQuickSyncType) *theInt=ReadLongLong(); else WriteLongLong(*theInt);}
	inline void			Sync(String* theString) {if (mQuickSyncType) *theString=ReadString(); else WriteString(*theString);}
	inline void			Sync(float* theFloat) {if (mQuickSyncType) *theFloat=ReadFloat(); else WriteFloat(*theFloat);}
	inline void			Sync(double* theDouble) {if (mQuickSyncType) *theDouble=ReadDouble(); else WriteDouble(*theDouble);}
	inline void			Sync(bool* theBool) {if (mQuickSyncType) *theBool=ReadBool(); else WriteBool(*theBool);}
	inline void			Sync(IOBuffer* theBuffer) {if (mQuickSyncType) ReadBuffer(*theBuffer); else WriteBuffer(*theBuffer);}
	inline void			Sync(Point* thePoint) {if (mQuickSyncType) *thePoint=ReadPoint(); else WritePoint(*thePoint);}
	inline void			Sync(IPoint* theIPoint) {if (mQuickSyncType) *theIPoint=ReadIPoint(); else WriteIPoint(*theIPoint);}
	inline void			Sync(Color* theColor) {if (mQuickSyncType) *theColor=ReadColor(); else WriteColor(*theColor);}
	inline void			Sync(Vector* thePoint) {if (mQuickSyncType) *thePoint=ReadVector(); else WriteVector(*thePoint);}
	inline void			Sync(SuperBits* theBits) {if (mQuickSyncType) ReadSuperbits(*theBits); else WriteSuperbits(*theBits);}
	inline void			Sync(Random* theRandom) {if (mQuickSyncType) ReadRandom(*theRandom); else WriteRandom(*theRandom);}
	

	//
	///////////////////////////////////////////////


	///////////////////////////////////////////////
	//
	// A few goofy things we can do, since they'll
	// help us along the road...
	//
	// 
	void				ExportAsCCharList(String theFilename);


	//
	// Gets the hash of the IOBuffer...
	//
	longlong			Hash();
};

class QuickStack : public IOBuffer
{
public:
	void			Push(void *theMemory, int theLength);
	inline void		Push(char *theChar) {Push(theChar,sizeof(char));}
	inline void		Push(int *theInt) {Push(theInt,sizeof(int));}
	inline void		Push(short *theShort) {Push(theShort,sizeof(short));}
	inline void		Push(bool *theBool) {Push(theBool,sizeof(bool));}
	inline void		Push(Point *thePoint) {Push(thePoint,sizeof(Point));}
	inline void		Push(IPoint *theIPoint) {Push(theIPoint,sizeof(IPoint));}
	inline void		Push(float *theFloat) {Push(theFloat,sizeof(float));}
	inline void		Push(double *theDouble) {Push(theDouble,sizeof(double));}
	inline void		Push(Color *theColor) {Push(theColor,sizeof(Color));}
	inline void		Push(Rect *theRect) {Push(theRect,sizeof(Rect));}

	void			Pop(void *theMemory, int theLength);
	inline void		Pop(char *theChar) {Pop(theChar,sizeof(char));}
	inline void		Pop(int *theInt) {Pop(theInt,sizeof(int));}
	inline void		Pop(short *theShort) {Pop(theShort,sizeof(short));}
	inline void		Pop(bool *theBool) {Pop(theBool,sizeof(bool));}
	inline void		Pop(Point *thePoint) {Pop(thePoint,sizeof(Point));}
	inline void		Pop(IPoint *theIPoint) {Pop(theIPoint,sizeof(IPoint));}
	inline void		Pop(float *theFloat) {Pop(theFloat,sizeof(float));}
	inline void		Pop(double *theDouble) {Pop(theDouble,sizeof(double));}
	inline void		Pop(Color *theColor) {Pop(theColor,sizeof(Color));}
	inline void		Pop(Rect *theRect) {Pop(theRect,sizeof(Rect));}

};


//
// For downloading files (and saving!)
//
Smart(IOBuffer) DownloadFile(String theURL, String theDestination, String theVersion="1",float* theProgressIndicator=NULL,HOOKARGPTR runWhenComplete=NULL, void* theArg=NULL);

