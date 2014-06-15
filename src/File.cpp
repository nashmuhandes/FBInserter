#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "File.h"

File_t *_OpenFile(const char *fileName, int flag)
{
	File_t *file = (File_t *)malloc(sizeof(File_t));

	if((file->handle = open(fileName, flag)) == -1 || fstat(file->handle, &file->statistics) == -1)
	{
		free(file);
		return NULL;
	}

	return file;
}

void _CloseFile(File_t *file)
{
	if(file)
	{
		close(file->handle);
		free(file);
	}
}

bool Read(int handle, void *buffer, int bytes)
{
	return read(handle, buffer, bytes) == bytes;
}

bool ReadAt(int handle, long offset, int origin, void *buffer, int bytes)
{
	lseek(handle, offset, origin);
	return read(handle, buffer, bytes) == bytes;
}

bool Write(int handle, const void *buffer, int bytes)
{
	return write(handle, buffer, bytes) == bytes;
}

bool WriteAt(int handle, long offset, int origin, const void *buffer, int bytes)
{
	lseek(handle, offset, origin);
	return write(handle, buffer, bytes) == bytes;
}

bool WriteInt(int handle, int value)
{
	return write(handle, &value, sizeof(int)) == sizeof(int);
}

bool WriteIntAt(int handle, long offset, int origin, int value)
{
	lseek(handle, offset, origin);
	return write(handle, &value, sizeof(int)) == sizeof(int);
}
