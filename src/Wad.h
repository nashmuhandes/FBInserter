#ifndef _WAD_H_
#define _WAD_H_

#include "File.h"

typedef struct
{
	char   wadType[4];
	ULONG  lumpNumber;
	size_t offset;
}
WadHeader_t;

typedef struct
{
	size_t offset;
	size_t size;
	char   name[8];
}
WadLumpInfo_t;

typedef struct
{
	char          *fileNameFull; // Name with full path
	char          *fileName;     // Name without path
	File_t        *file;
	WadHeader_t   header;
	WadLumpInfo_t *lumpInfo;
}
WadFileInfo_t;

bool ReadWadFile(WadFileInfo_t *wfi);
bool IsLumpName(const char *name);

#endif
