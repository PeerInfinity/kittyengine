#include "rapt.h"
#include "os_core.h"


Console gConsole;
bool gNoOutput=true;

Console::Console()
{
	mOn=true;
	mIndent=0;
	mIndentString="  ";
}
Console::~Console()
{
#ifdef _DEBUG
	if (!gNoOutput)
	{
		Out("");
		Out("*** END SESSION ***");
	}
#endif
}

void Console::Go()
{
#ifdef _DEBUG
	Out("*** START SESSION ***");
	Out("");
#endif
}

// void Echo(char *format, ...)
// {
// 	va_list argp;
// 	va_start(argp, format);

// 	String aString;
// 	aString=Sprintf(format,argp);
// 	va_end(argp);

// 	gOut.Out(aString);
// }


//#include "rapt_string.h"
void Console::Out(char *format, ...)
{

	if (mOn)
	{
		bool aSpacing=false;
		if (format[0]==format[1])
		{
			if (format[0]=='!' || format[0]=='$' || format[0]=='*' || format[0]=='_') {Out("");format++;aSpacing=true;}
		}

		va_list argp;
		va_start(argp, format);

		String aString;
		aString=Sprintf(format,argp);
		va_end(argp);

		if (gNoOutput) gNoOutput=false;

		if (mIndent>0)
		{
			String aMyString;
			for (int aCount=0;aCount<mIndent;aCount++) aMyString+=mIndentString;

			int aSlot=0;
			if (aString[0]=='!' || aString[0]=='$' || aString[0]=='*' || aString[0]=='_' || aString[0]=='.') aSlot=1;
			aString.Insert(aMyString,aSlot);
		}

		if (aString.Contains('\n'))
		{
			String aToken=aString.GetToken('\n');
			while (!aToken.IsNull())
			{
				Out(aToken.c());
				aToken=aString.GetNextToken('\n');
			}
			
			return;
		}


		/*
		bool aEmphasis=false;
		if (aString.Contains("!!!!")) aEmphasis=true;

		if (aEmphasis) 
		{
			Out("");
			Out("************************************************************************");
		}
		*/

		if (!OS_Core::Output(aString.c()))
		{
		}

		if (aSpacing) Out("");


		/*
		if (aEmphasis) 
		{
			Out("************************************************************************");
			Out("");
		}
		*/

		//_DeleteArray(aString);
	}
}

void Console::XAlert(char *format, ...)
{
	if (mOn)
	{
		bool aSpacing=false;

		va_list argp;
		va_start(argp, format);

		String aString;
		aString=Sprintf(format,argp);
		va_end(argp);

		OS_Core::MessageBox("CONSOLE ALERT",aString.c(),0);
		exit(0);
	}
}

void Console::Alert(char *format, ...)
{
	if (mOn)
	{
		bool aSpacing=false;

		va_list argp;
		va_start(argp, format);

		String aString;
		aString=Sprintf(format,argp);
		va_end(argp);

		OS_Core::MessageBox("CONSOLE ALERT",aString.c(),0);
	}
}

#ifdef _DEBUG
void Console::Debug(char *format, ...)
{
	if (mOn)
	{
		va_list argp;
		va_start(argp, format);

		String aString;
		aString=Sprintf(format,argp);
		va_end(argp);

		if (gNoOutput) gNoOutput=false;

		if (mIndent>0)
		{
			String aMyString;
			for (int aCount=0;aCount<mIndent;aCount++) aMyString+=mIndentString;
			aString.Insert(aMyString);
		}


		bool aEmphasis=false;
		if (aString.Contains("!!!!")) aEmphasis=true;

		if (aEmphasis)
		{
			Out("");
			Out("************************************************************************");
		}

		if (!OS_Core::Output(aString.c()))
		{
		}

		if (aEmphasis) 
		{
			Out("************************************************************************");
			Out("");
		}

		//_DeleteArray(aString);
	}
}
#endif

void Console::OutPlain(char *theText)
{
	if (mOn)
	{
		String aMyString;
		for (int aCount=0;aCount<mIndent;aCount++) aMyString+=mIndentString;
		aMyString+=theText;

		if (gNoOutput)
		{
			gNoOutput=false;
//			OS_Core::Output("*** START SESSION ***");
//			OS_Core::Output("");
		}

		bool aEmphasis=false;
		if (aMyString.Contains("!!!!")) aEmphasis=true;

		if (aEmphasis) 
		{
			Out("");
			Out("************************************************************************");
		}

		if (!OS_Core::Output(aMyString.c()))
		{
			//
			// No window found, dump to file!
			//
/*
			IOBuffer aBuffer;
			aBuffer.Load(PointAtDesktop("log.txt"));
			aBuffer.SeekEnd();
			aBuffer.WriteLine(aString);
			aBuffer.CommitFile(PointAtDesktop("log.txt"));
*/
		}

		if (aEmphasis) 
		{
			Out("************************************************************************");
			Out("");
		}
	}
}


Tracer::Tracer()
{
	mBuffer=new IOBuffer;
}

Tracer::~Tracer()
{
	mBuffer->CommitFile(mFilename);
	delete mBuffer;
}

void Tracer::Out(char *format, ...)
{
	if (mFilename.Len()==0) mFilename="sandbox://trace.txt";

	static char *aString;
	if (!aString) aString=new char[MAX_PATH];
	
	va_list argp;
	va_start(argp, format);
	vsprintf(aString,format, argp);
	va_end(argp);

	mBuffer->WriteLine(aString);
	mBuffer->CommitFile(mFilename);
}

