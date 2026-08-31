#include "rapt_list.h"
#include "rapt.h"

void Debug(char *format, ...)
{
	char aString[2048];

	va_list argp;
	va_start(argp, format);
	vsprintf(aString,format, argp);
	va_end(argp);

	gOut.Out(aString);
}

