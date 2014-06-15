#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Misc.h"

HANDLE HOutput = GetStdHandle(STD_OUTPUT_HANDLE);

char *strnupr(char *string, size_t length)
{
	int shift = 'A' - 'a';
	size_t i;

	for(i = 0; i < length && string[i]; i++)
	{
		if(string[i] >= 'a' && string[i] <= 'z')
		{
			string[i] += shift;
		}
	}

	return string;
}

char *StringFormat(char *format, ...)
{
	static char String[1024];

	va_list argPtr;

	va_start(argPtr, format);
	vsprintf(String, format, argPtr);
	va_end(argPtr);

	return String;
}

void PrintColor(int color, char *format, ...)
{
	static char String[1024];

	va_list argPtr;

	va_start(argPtr, format);
	vsprintf(String, format, argPtr);
	va_end(argPtr);

	SetConsoleTextAttribute(HOutput, color);
	printf(String);
	DefaultColor();
}

void Warning(const char *format, ...)
{
	static char String[1024];

	va_list argPtr;

	va_start(argPtr, format);
	vsprintf(String, format, argPtr);
	va_end(argPtr);

	SetConsoleTextAttribute(HOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
	printf("WARNING - %s\n", String);
	DefaultColor();
}

void Error(const char *format, ...)
{
	static char String[1024];

	va_list argPtr;

	va_start(argPtr, format);
	vsprintf(String, format, argPtr);
	va_end(argPtr);

	SetConsoleTextAttribute(HOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
	printf("ERROR - %s\n", String);
	DefaultColor();
}

void Exit(void)
{
	system("pause");
	exit(1);
}

char *GetFileNameNoPath(char *fileName)
{
	if(!fileName)
	{
		return NULL;
	}

	char *check = strrchr(fileName, '\\');
	return check ? check + 1 : fileName;
}

char *GetFileNameNoPathNoExt(const char *fileName)
{
	if(!fileName)
	{
		return NULL;
	}

	char *slash  = strrchr(fileName, '\\'),
	     *result = strdup(slash ? slash + 1 : fileName), // Don't forget this, may memory leak
		 *dot    = strrchr(result, '.');

	if(dot)
	{
		*dot = '\0';
	}

	return result;
}

bool IsFileExtension(const char *fileName, const char *extension)
{
	if(!fileName || !extension)
	{
		return false;
	}

	char *dot = strrchr(fileName, '.');
	return dot ? !stricmp(++dot, extension) : false;
}

char *SizeToString(size_t size)
{
	static char String[1024];

	if(size < 1048576 * 95 / 100)
	{
		sprintf(String, "%.2f KB", (float)size / 1024.0);
	}
	else
	{
		sprintf(String, "%.2f MB", (float)size / 1048576.0);
	}

	return String;
}
