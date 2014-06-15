#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "Wad.h"
#include "Misc.h"

bool ReadWadFile(WadFileInfo_t *wfi)
{
	size_t lumpInfoSize;
	ULONG i;

	if(!wfi)
	{
		return false;
	}

	// Too small
	if(wfi->file->statistics.st_size < sizeof(WadHeader_t))
	{
		return false;
	}
	if(!ReadBytes(wfi->file->handle, &wfi->header, sizeof(WadHeader_t)))
	{
		return false;
	}
	// Check WAD type
	if((wfi->header.wadType[0] != 'P' && wfi->header.wadType[0] != 'I') ||
	    wfi->header.wadType[1] != 'W'                                   ||
	    wfi->header.wadType[2] != 'A'                                   ||
	    wfi->header.wadType[3] != 'D')
	{
		return false;
	}

	lumpInfoSize = sizeof(WadLumpInfo_t) * wfi->header.lumpNumber;
	// Check the file size meets WAD format
	if(wfi->header.offset + lumpInfoSize != (size_t)wfi->file->statistics.st_size)
	{
		return false;
	}

	wfi->lumpInfo = (WadLumpInfo_t *)malloc(lumpInfoSize);
	if(!ReadBytesAt(wfi->file->handle, wfi->header.offset, SEEK_SET, wfi->lumpInfo, lumpInfoSize))
	{
		free(wfi->lumpInfo);
		wfi->lumpInfo = NULL;
		return false;
	}

	// For safty
	for(i = 0; i < wfi->header.lumpNumber; i++)
	{
		strnupr(wfi->lumpInfo[i].name, 8);
	}

	return true;
}

bool IsLumpName(const char *name)
{
	return !(strlen(name) > 8 || strrchr(name, '.'));
}
