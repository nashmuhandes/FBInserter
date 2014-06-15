#ifndef _FILE_H_
#define _FILE_H_

// For file size only, use filelength

#include <sys/stat.h>

typedef struct stat FileStat_t;
typedef struct
{
	int        handle;
	FileStat_t statistics;
}
File_t;

File_t *_OpenFile(const char *fileName, int flag);
void _CloseFile(File_t *file);
bool Read(int handle, void *buffer, int bytes);
bool ReadAt(int handle, long offset, int origin, void *buffer, int bytes);

#define ReadBytes(handle, buffer, length) Read(handle, buffer, length)
#define ReadBytesAt(handle, offset, origin, buffer, length) ReadAt(handle, offset, origin, buffer, length)

bool Write(int handle, const void *buffer, int bytes);
bool WriteAt(int handle, long offset, int origin, const void *buffer, int bytes);

#define WriteBytes(handle, buffer, bytes) Write(handle, buffer, bytes)
#define WriteBytesAt(handle, offset, origin, buffer, bytes) WriteAt(handle, offset, origin, buffer, bytes)

bool WriteInt(int handle, int value);
bool WriteIntAt(int handle, long offset, int origin, int value);

#endif
