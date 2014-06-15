#ifndef _MISC_H_
#define _MISC_H_

#include <windows.h>

#define FOREGROUND_WHITE   (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FOREGROUND_NORMAL  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define FOREGROUND_YELLOW  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FOREGROUND_MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FOREGROUND_CYAN    (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

#define DefaultColor() SetConsoleTextAttribute(HOutput, FOREGROUND_NORMAL)

char *strnupr(char *string, size_t length);
char *StringFormat(char *format, ...);
void PrintColor(int color, char *format, ...);
void Warning(const char *format, ...);
void Error(const char *format, ...);
void Exit(void);

char *GetFileNameNoPath(char *fileName);
char *GetFileNameNoPathNoExt(const char *fileName);
bool IsFileExtension(const char *fileName, const char *extension);

char *SizeToString(size_t size);

#endif
