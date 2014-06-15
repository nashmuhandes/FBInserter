#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <windows.h>

#define _DEBUG_ 0

#include "File.h"
#include "Wad.h"
#include "Misc.h"

#define MAXMAPSUPPORT 1024
#define MAXSCRIPTSUPPORT 128

typedef struct
{
	int  lumpIndex;
	bool behavior;
	bool textmap;
	int  fsIndex;
	int  behaviorIndex;
}
MapInfo_t;

typedef struct
{
	char   *fileName;
	char   *name;
	size_t size;
	char   *buffer;
}
ScriptInfo_t;

void PrintUsage(void);
void PrintWadInfo(void);
void PrintMapInfo(void);

#if _DEBUG_
void PrintFSFiles(void);
void PrintBehaviorFiles(void);
void PrintLumpInfo(const char *title, WadLumpInfo_t *lumpInfo, ULONG count);
void PrintMapInfoStruct(void);
#endif

void ProcessArgs(void);
void FindMaps(void);
bool ReadScriptFile(const char *fileName, ScriptInfo_t *scriptInfo);
void MakeReferences(void);
void StartInsertion(void);

int  ArgC;
char **ArgV;

//bool NoPause = false;

WadFileInfo_t WadFileInfo;
ULONG         MapCount          = 0;
MapInfo_t     MapInfo[MAXMAPSUPPORT];
ULONG         FSFileCount       = 0;
ScriptInfo_t  FSScriptInfo[MAXSCRIPTSUPPORT];
ULONG         BehaviorFileCount = 0;
ScriptInfo_t  BehaviorScriptInfo[MAXSCRIPTSUPPORT];
ULONG         OutputLumpCount   = 0;

int main(int argc, char **argv)
{
	SetConsoleTitle("FraggleScript & BEHAVIOR Inserter");
	system("cls");

	ArgC = argc;
	ArgV = argv;

	ProcessArgs();
	MakeReferences();
	PrintWadInfo();
	PrintMapInfo();
	StartInsertion();

	/*
	if(!NoPause)
	{
		system("pause");
	}
	*/
	return 0;
}

void PrintUsage(void)
{
	PrintColor(FOREGROUND_YELLOW, "\n========== USAGE ==========\n");

	PrintColor(FOREGROUND_MAGENTA, "%s ", GetFileNameNoPath(ArgV[0]));
	PrintColor(FOREGROUND_CYAN, "WadFileName ");
	PrintColor(FOREGROUND_WHITE, "InsertFileName ");
	PrintColor(FOREGROUND_NORMAL, "[InsertMapName] ");
	PrintColor(FOREGROUND_WHITE, "...\n");

	PrintColor(FOREGROUND_CYAN, "\nWadFileName:\n");
	printf("  File name of the source WAD file to be inserted. This parameter is not\n"
	       "  restricted at the first parameter followed by the exe name. It can be\n"
	       "  anywhere.\n");
	PrintColor(FOREGROUND_WHITE, "\nInsertFileName ");
	PrintColor(FOREGROUND_NORMAL, "[InsertMapName]");
	PrintColor(FOREGROUND_WHITE, ":\n");
	PrintColor(FOREGROUND_WHITE, "  InsertFileName:\n");
	printf("    Could be FraggleScript file (*.fs) or compiled ACS file (*.o).\n");
	PrintColor(FOREGROUND_NORMAL, "  [InsertMapName]:\n");
	printf("    Optional. Using this parameter would force to insert into the specified\n"
	       "    map by map name. Without this parameter, it will automatically search for\n"
	       "    the map name same as InsertFileName(without extension) as the insertion\n"
	       "    target.\n");
	PrintColor(FOREGROUND_WHITE, "\n...:\n");
	printf("  Can use multiple InsertFileName [InsertMapName].\n");

	PrintColor(FOREGROUND_CYAN, "\nExtra parameter, case insensitive: -nopause\n");
	printf("  This parameter disables stopping at the final result, but still halts the\n"
	       "  process when errors occur.\n");
}

void PrintWadInfo(void)
{
	PrintColor(FOREGROUND_MAGENTA, "\nWAD information:  (%s)\n", WadFileInfo.fileName);
	PrintColor(FOREGROUND_WHITE, "  File size  - %d bytes (%s)\n"
	                             "  Lump count - %d\n",
	                             WadFileInfo.file->statistics.st_size, SizeToString(WadFileInfo.file->statistics.st_size),
	                             WadFileInfo.header.lumpNumber);
}

// Call this after MakeReferences
void PrintMapInfo(void)
{
	ULONG i;
	char name[9];

	name[8] = '\0';

	PrintColor(FOREGROUND_MAGENTA, "\nMap information: (%d found)\n", MapCount);
	PrintColor(FOREGROUND_WHITE, "           Name, BEHAVIOR?, TEXTMAP?, Insertion(s)\n");
	for(i = 0; i < MapCount; i++)
	{
		strncpy(name, WadFileInfo.lumpInfo[MapInfo[i].lumpIndex].name, 8);
		PrintColor(FOREGROUND_WHITE, "  %3d: %8s, %9s, %8s, %s %s\n",
		                             i + 1,
		                             name,
		                             MapInfo[i].behavior ? "Yes" : "No",
		                             MapInfo[i].textmap ? "Yes" : "No",
		                             MapInfo[i].fsIndex == -1 ? "" : FSScriptInfo[MapInfo[i].fsIndex].fileName,
		                             MapInfo[i].behaviorIndex == -1 ? "" : BehaviorScriptInfo[MapInfo[i].behaviorIndex].fileName);
	}
}

#if _DEBUG_
void PrintFSFiles(void)
{
	ULONG i;
	char name[9];

	name[8] = '\0';
	printf("===== FS File List =====\n"
	       "Total count = %d\n"
	       "index\t-         name, size\t, filename\n",
	       FSFileCount);
	for(i = 0; i < FSFileCount; i++)
	{
		strncpy(name, FSScriptInfo[i].name, 8);
		printf("%d\t- %12s, %d\t, %s\n", i, name, FSScriptInfo[i].size, FSScriptInfo[i].fileName);
	}
	printf("\n");
}

void PrintBehaviorFiles(void)
{
	ULONG i;
	char name[9];

	name[8] = '\0';
	printf("===== Behavior File List =====\n"
	       "Total count = %d\n"
	       "index\t-         name, size, filename\n",
	       BehaviorFileCount);
	for(i = 0; i < BehaviorFileCount; i++)
	{
		strncpy(name, BehaviorScriptInfo[i].name, 8);
		printf("%d\t- %12s, %d\t, %s\n", i, name, BehaviorScriptInfo[i].size, BehaviorScriptInfo[i].fileName);
	}
	printf("\n");
}

void PrintLumpInfo(const char *title, WadLumpInfo_t *lumpInfo, ULONG count)
{
	ULONG c;
	char name[9];

	name[8] = '\0';
	printf("===== %s Lump List =====\n"
	       "Total count = %d\n"
	       "index\t-         name, size\t, offset\n",
	       title, count);
	for(c = 0; c < count; c++)
	{
		strncpy(name, lumpInfo[c].name, 8);
		printf("%d\t- %12s, %d\t, %d\n", c, name, lumpInfo[c].size, lumpInfo[c].offset);
	}
	printf("\n");
}

void PrintMapInfoStruct(void)
{
	ULONG i;
	char name[9];

	name[8] = '\0';
	printf("===== Map Information =====\n"
	       "Total count = %d\n"
	       "index\t-         name, wadI\t, fsI\t, behI\t, beh?\t, txtmap?\n",
	       MapCount);
	for(i = 0; i < MapCount; i++)
	{
		strncpy(name, WadFileInfo.lumpInfo[MapInfo[i].index].name, 8);
		printf("%d\t- %12s, %d\t, %d\t, %d\t, %c\t, %c\n", i, name, MapInfo[i].index, MapInfo[i].fsIndex, MapInfo[i].behaviorIndex, MapInfo[i].behavior ? 'O' : 'X', MapInfo[i].textmap ? 'O' : 'X');
	}
	printf("\n");
}
#endif

void ProcessArgs(void)
{
	int argc;
	char *string;

	memset(&WadFileInfo, 0, sizeof(WadFileInfo));

	for(argc = 1; argc < ArgC; argc++)
	{
		if(IsFileExtension(ArgV[argc], "wad")) // WAD file
		{
			if(WadFileInfo.file)
			{
				Warning("WAD duplication, ignores \"%s\"", GetFileNameNoPath(ArgV[argc]));
				continue;
			}

			if(!(WadFileInfo.file = _OpenFile(ArgV[argc], O_RDONLY | O_BINARY)))
			{
				Error("Cannot open file \"%s\"!", ArgV[argc]);
				Exit();
			}
			if(!ReadWadFile(&WadFileInfo))
			{
				Error("\"%s\" is not a WAD file!", ArgV[argc]);
				Exit();
			}
			WadFileInfo.fileNameFull = strdup(ArgV[argc]);
			WadFileInfo.fileName     = GetFileNameNoPath(WadFileInfo.fileNameFull);
			if(WadFileInfo.header.wadType[0] == 'I')
			{
				Error("Modifying IWAD is forbidden! (\"%s\")", WadFileInfo.fileName);
				Exit();
			}

			FindMaps();

			if(!MapCount)
			{
				Error("WAD file \"%s\" file doesn't contain any maps", WadFileInfo.fileName);
				Exit();
			}
		}
		else if(IsFileExtension(ArgV[argc], "fs")) // FraggleScript file
		{
			if(!ReadScriptFile(ArgV[argc], &FSScriptInfo[FSFileCount]))
			{
				Warning("Cannot read FS file \"%s\"", GetFileNameNoPath(ArgV[argc]));
				continue;
			}

			FSScriptInfo[FSFileCount].fileName = GetFileNameNoPath(ArgV[argc]);
			if(argc < ArgC - 1 && IsLumpName(ArgV[argc + 1]))
			{
				FSScriptInfo[FSFileCount].name = ArgV[argc + 1];
				argc++;
			}
			else
			{
				string = GetFileNameNoPathNoExt(ArgV[argc]);
				if(strlen(string) > 8)
				{
					Warning("Cannot use file name \"%s\" as a map name reference.", string);
					continue;
				}
				FSScriptInfo[FSFileCount].name = string;
			}
			FSFileCount++;
		}
		else if(IsFileExtension(ArgV[argc], "o")) // BEHAVIOR file
		{
			if(!ReadScriptFile(ArgV[argc], &BehaviorScriptInfo[BehaviorFileCount]))
			{
				Warning("Cannot read BEHAVIOR file \"%s\"", GetFileNameNoPath(ArgV[argc]));
				continue;
			}

			BehaviorScriptInfo[BehaviorFileCount].fileName = GetFileNameNoPath(ArgV[argc]);
			if(argc < ArgC - 1 && IsLumpName(ArgV[argc + 1]))
			{
				BehaviorScriptInfo[BehaviorFileCount].name = ArgV[argc + 1];
				argc++;
			}
			else
			{
				string = GetFileNameNoPathNoExt(ArgV[argc]);
				if(strlen(string) > 8)
				{
					Warning("Cannot use file name as map name reference \"%s\"", string);
					continue;
				}
				BehaviorScriptInfo[BehaviorFileCount].name = string;
			}
			BehaviorFileCount++;
		}
		/*
		else if(!stricmp(GetFileNameNoPath(ArgV[argc]), "-nopause")) // Extra parameter
		{
			NoPause = true;
		}
		*/
		else
		{
			Warning("Input file \"%s\" is not FS (*.fs) or compiled ACS (*.o)!", GetFileNameNoPath(ArgV[argc]));
			continue;
		}
	}

	if(!WadFileInfo.file)
	{
		Error("No input WAD file!");
		PrintUsage();
		Exit();
	}

	if(!FSFileCount && !BehaviorFileCount)
	{
		Error("No FS or BEHAVIOR file for insertion!");
		PrintUsage();
		Exit();
	}

#if _DEBUG_
	PrintFSFiles();
	PrintBehaviorFiles();
	PrintLumpInfo("SourceWad", WadFileInfo.lumpInfo, WadFileInfo.header.lumpNumber);
#endif
}

void FindMaps(void)
{
	static const char *MapLumpCheck[] =
	{
		"", // Map marker itself
		"THINGS",
		"LINEDEFS",
		"SIDEDEFS",
		"VERTEXES",
		"SEGS",
		"SSECTORS",
		"NODES",
		"SECTORS",
		"REJECT",
		"BLOCKMAP",
		"BEHAVIOR"
	};

	ULONG index, check;

	for(index = 0; index < WadFileInfo.header.lumpNumber; index++)
	{
		// Check for binary map
		if(index < WadFileInfo.header.lumpNumber - 10)
		{
			for(check = 1;
				check < 12 && !strncmp(WadFileInfo.lumpInfo[index + check].name, MapLumpCheck[check], 8);
				check++);

			if(check >= 11)
			{
				MapInfo[MapCount].lumpIndex     = index;
				MapInfo[MapCount].behavior      = check == 12;
				MapInfo[MapCount].textmap       = false;
				MapInfo[MapCount].fsIndex       = -1;
				MapInfo[MapCount].behaviorIndex = -1;

				index += check - 1;

				if(++MapCount == MAXMAPSUPPORT)
				{
					Warning("Sorry. Map number exceeds supported number (%d)", MAXMAPSUPPORT);
					return;
				}

				continue;
			}
		}

		// Check for text map
		if(index < WadFileInfo.header.lumpNumber - 2)
		{
			if(!strncmp(WadFileInfo.lumpInfo[index + 1].name, "TEXTMAP", 8))
			{
				// Check for BEHAVIOR
				MapInfo[MapCount].behavior = false;
				for(check = index + 2;
				    check < WadFileInfo.header.lumpNumber && strncmp(WadFileInfo.lumpInfo[check].name, "ENDMAP", 8);
				    check++)
				{
						if(!strncmp(WadFileInfo.lumpInfo[check].name, "BEHAVIOR", 8))
						{
							MapInfo[MapCount].behavior = true;
						}
				}

				if(check >= WadFileInfo.header.lumpNumber)
				{
					Error("Textmap ending lump \"ENDMAP\" not found!");
					Exit();
				}

				MapInfo[MapCount].lumpIndex     = index;
				MapInfo[MapCount].textmap       = true;
				MapInfo[MapCount].fsIndex       = -1;
				MapInfo[MapCount].behaviorIndex = -1;

				index = check;

				if(++MapCount == MAXMAPSUPPORT)
				{
					Warning("Sorry. Map number exceeds supported number (%d)", MAXMAPSUPPORT);
					return;
				}

				continue;
			}
		}
	}
}

bool ReadScriptFile(const char *fileName, ScriptInfo_t *scriptInfo)
{
	File_t *file = _OpenFile(fileName, O_RDONLY | O_BINARY);

	if(!file)
	{
		return false;
	}
	scriptInfo->size = file->statistics.st_size;
	scriptInfo->buffer = (char *)malloc(scriptInfo->size);
	if(!ReadBytes(file->handle, scriptInfo->buffer, scriptInfo->size))
	{
		free(scriptInfo->buffer);
		_CloseFile(file);
		return false;
	}

	_CloseFile(file);

	return true;
}

void MakeReferences(void)
{
	ULONG i, j;
	bool hasReference = false;

	// Check for FS refernce
	for(i = 0; i < FSFileCount; i++)
	{
		strupr(FSScriptInfo[i].name);
		for(j = 0; j < MapCount; j++)
		{
			if(!strncmp(FSScriptInfo[i].name, WadFileInfo.lumpInfo[MapInfo[j].lumpIndex].name, 8))
			{
				MapInfo[j].fsIndex = i;
				hasReference = true;
				break;
			}
		}
		if(j == MapCount)
		{
			Warning("%s: %s doesn't have a referenced map name for insertion",
			        GetFileNameNoPath(FSScriptInfo[i].fileName), FSScriptInfo[i].name);
		}
	}

	// Check for BEHAVIOR refernce
	OutputLumpCount = WadFileInfo.header.lumpNumber;
	for(i = 0; i < BehaviorFileCount; i++)
	{
		strupr(BehaviorScriptInfo[i].name);
		for(j = 0; j < MapCount; j++)
		{
			if(MapInfo[j].textmap)
			{
				if(!strncmp(BehaviorScriptInfo[i].name, WadFileInfo.lumpInfo[MapInfo[j].lumpIndex].name, 8))
				{
					if(!MapInfo[j].behavior)
					{
						OutputLumpCount++;
					}
					MapInfo[j].behaviorIndex = i;
					hasReference = true;
					break;
				}
			}
			else
			{
				// A map without BEHAVIOR is likely not to be the format in Hexen style,
				// force inserting this might cause the map currupted.
				if(!MapInfo[j].behavior)
				{
					continue;
				}

				if(!strncmp(BehaviorScriptInfo[i].name, WadFileInfo.lumpInfo[MapInfo[j].lumpIndex].name, 8))
				{
					MapInfo[j].behaviorIndex = i;
					hasReference = true;
					break;
				}
			}
		}
		if(j == MapCount)
		{
			Warning("%s: %s doesn't have a referenced map name for insertion",
			        GetFileNameNoPath(BehaviorScriptInfo[i].fileName),
			        BehaviorScriptInfo[i].name);
		}
	}
#if _DEBUG_
	PrintMapInfoStruct();
#endif

	if(!hasReference)
	{
		Error("Cannot find any referenced map name for insertion!");
		Exit();
	}
}

void ReadAtLump(ULONG lumpIndex, char *buffer)
{
	if(!ReadBytesAt(WadFileInfo.file->handle, WadFileInfo.lumpInfo[lumpIndex].offset, SEEK_SET, buffer, WadFileInfo.lumpInfo[lumpIndex].size))
	{
		Error("Fail to read WAD lump!");
		Exit();
	}
}

void WriteLump(int handle, const char *buffer, size_t size)
{
	if(!Write(handle, buffer, size))
	{
		Error("Fail to write WAD lump!");
		Exit();
	}
}

void StartInsertion(void)
{
	int handle;
	char fileName[512], backupName[512];
	ULONG i;
	WadLumpInfo_t *outputLumpInfo;
	size_t maxLumpSize;
	char *buffer;
	ULONG mapIndex, nextMapLump;
	ULONG oi, ni;
	size_t writtenSize, // Bytes written to the output file, also the "offset" of the current lump
	       newFileSize;

	// Prepare for output
	strcpy(fileName, WadFileInfo.fileNameFull);
	*strrchr(fileName, '.') = '\0';
	sprintf(fileName, "%s_New.wad", fileName);

	SetFileAttributes(fileName, FILE_ATTRIBUTE_NORMAL);
	remove(fileName);

	if((handle = open(fileName, O_CREAT | O_TRUNC | O_RDWR | O_BINARY)) == -1)
	{
		Error("Cannot create output file \"%s\"", GetFileNameNoPath(fileName));
		Exit();
	}

	// Prepare lump buffer for input
	maxLumpSize = 0;
	for(i = 0; i < WadFileInfo.header.lumpNumber; i++)
	{
		if(WadFileInfo.lumpInfo[i].size > maxLumpSize)
		{
			maxLumpSize = WadFileInfo.lumpInfo[i].size;
		}
	}
	buffer = (char *)malloc(maxLumpSize);

	// Prepare output lump
	outputLumpInfo = (WadLumpInfo_t *)malloc(sizeof(WadLumpInfo_t) * OutputLumpCount);

	// Output header, has to be changed after insertion
	if(!Write(handle, &WadFileInfo.header, sizeof(WadHeader_t)))
	{
		Error("Fail to write WAD header!");
		Exit();
	}
	writtenSize = sizeof(WadHeader_t);

	printf("\n");

	// Insertion
	oi = 0; // old WAD lump index
	ni = 0; // new WAD lump index
	nextMapLump = MapInfo[0].lumpIndex;
	mapIndex = 0;
	while(1)
	{
		// Non-map lumps, just copy them
		for(; oi < nextMapLump; oi++, ni++)
		{
			ReadAtLump(oi, buffer);
			WriteLump(handle, buffer, WadFileInfo.lumpInfo[oi].size);

			memcpy(&outputLumpInfo[ni], &WadFileInfo.lumpInfo[oi], sizeof(WadLumpInfo_t));
			outputLumpInfo[ni].offset = writtenSize;

			writtenSize += WadFileInfo.lumpInfo[oi].size;
		}

		// Finished
		if(oi >= WadFileInfo.header.lumpNumber)
		{
			break;
		}

		// FraggleScript insertion
		if(MapInfo[mapIndex].fsIndex != -1 && !MapInfo[mapIndex].textmap)
		{
			ScriptInfo_t *scriptInfo = &FSScriptInfo[MapInfo[mapIndex].fsIndex];

			WriteLump(handle, scriptInfo->buffer, scriptInfo->size);

			strncpy(outputLumpInfo[ni].name, WadFileInfo.lumpInfo[oi].name, 8);
			outputLumpInfo[ni].offset = writtenSize;
			outputLumpInfo[ni].size = scriptInfo->size;

			writtenSize += scriptInfo->size;

			PrintColor(FOREGROUND_CYAN, "- FS \"%s\" inserts into %s, %d bytes (%s)\n",
			                            scriptInfo->fileName,
			                            WadFileInfo.lumpInfo[MapInfo[mapIndex].lumpIndex].name,
			                            scriptInfo->size,
			                            SizeToString(scriptInfo->size));
		}
		else
		{
			if(MapInfo[mapIndex].fsIndex != -1 && MapInfo[mapIndex].textmap)
			{
				Warning("FS cannot be inserted into TEXTMAP\n");
			}

			// Just copy
			ReadAtLump(oi, buffer);
			WriteLump(handle, buffer, WadFileInfo.lumpInfo[oi].size);

			memcpy(&outputLumpInfo[ni], &WadFileInfo.lumpInfo[oi], sizeof(WadLumpInfo_t));
			outputLumpInfo[ni].offset = writtenSize;

			writtenSize += WadFileInfo.lumpInfo[oi].size;
		}

		// Moves to the next index
		oi++;
		ni++;

		// TEXTMAP, nonTEXTMAP operations
		if(MapInfo[mapIndex].textmap)
		{
			if(MapInfo[mapIndex].behaviorIndex != -1)
			{
				if(MapInfo[mapIndex].behavior)
				{
					// Process the lumps before ENDMAP
					for(; strncmp(WadFileInfo.lumpInfo[oi].name, "ENDMAP", 8); oi++, ni++)
					{
						if(!strncmp(WadFileInfo.lumpInfo[oi].name, "BEHAVIOR", 8))
						{
							// For BEHAVIOR, replace the origial one
							ScriptInfo_t *scriptInfo = &BehaviorScriptInfo[MapInfo[mapIndex].behaviorIndex];

							WriteLump(handle, scriptInfo->buffer, scriptInfo->size);

							strncpy(outputLumpInfo[ni].name, "BEHAVIOR", 8);
							outputLumpInfo[ni].offset = writtenSize;
							outputLumpInfo[ni].size = scriptInfo->size;

							writtenSize += scriptInfo->size;

							PrintColor(FOREGROUND_CYAN, "- BEHAVIOR \"%s\" inserts into %s, %d bytes (%s)\n",
							                            scriptInfo->fileName,
							                            WadFileInfo.lumpInfo[MapInfo[mapIndex].lumpIndex].name,
							                            scriptInfo->size,
							                            SizeToString(scriptInfo->size));
						}
						else
						{
							// Just copy the lump
							ReadAtLump(oi, buffer);
							WriteLump(handle, buffer, WadFileInfo.lumpInfo[oi].size);

							memcpy(&outputLumpInfo[ni], &WadFileInfo.lumpInfo[oi], sizeof(WadLumpInfo_t));
							outputLumpInfo[ni].offset = writtenSize;

							writtenSize += WadFileInfo.lumpInfo[oi].size;
						}
					}
				}
				else // No BAHAVIOR, we have to create one
				{
					// Copy all the lumps before ENDMAP, put BEHAVIOR in the last position
					for(; strncmp(WadFileInfo.lumpInfo[oi].name, "ENDMAP", 8); oi++, ni++)
					{
						ReadAtLump(oi, buffer);
						WriteLump(handle, buffer, WadFileInfo.lumpInfo[oi].size);

						memcpy(&outputLumpInfo[ni], &WadFileInfo.lumpInfo[oi], sizeof(WadLumpInfo_t));
						outputLumpInfo[ni].offset = writtenSize;

						writtenSize += WadFileInfo.lumpInfo[oi].size;
					}

					// Create BEHAVIOR
					ScriptInfo_t *scriptInfo = &BehaviorScriptInfo[MapInfo[mapIndex].behaviorIndex];

					WriteLump(handle, scriptInfo->buffer, scriptInfo->size);

					strncpy(outputLumpInfo[ni].name, "BEHAVIOR", 8);
					outputLumpInfo[ni].offset = writtenSize;
					outputLumpInfo[ni].size = scriptInfo->size;

					writtenSize += scriptInfo->size;

					ni++;

					PrintColor(FOREGROUND_CYAN, "- BEHAVIOR \"%s\" inserts into %s, %d bytes (%s)\n",
					                            scriptInfo->fileName,
					                            WadFileInfo.lumpInfo[MapInfo[mapIndex].lumpIndex].name,
					                            scriptInfo->size,
					                            SizeToString(scriptInfo->size));
				}
			}
			else
			{
				// Just copy the lumps before ENDMAP
				for(; strncmp(WadFileInfo.lumpInfo[oi].name, "ENDMAP", 8); oi++, ni++)
				{
					ReadAtLump(oi, buffer);
					WriteLump(handle, buffer, WadFileInfo.lumpInfo[oi].size);

					memcpy(&outputLumpInfo[ni], &WadFileInfo.lumpInfo[oi], sizeof(WadLumpInfo_t));
					outputLumpInfo[ni].offset = writtenSize;

					writtenSize += WadFileInfo.lumpInfo[oi].size;
				}
			}

			// ENDMAP lump
			ReadAtLump(oi, buffer);
			WriteLump(handle, buffer, WadFileInfo.lumpInfo[oi].size);

			memcpy(&outputLumpInfo[ni], &WadFileInfo.lumpInfo[oi], sizeof(WadLumpInfo_t));
			outputLumpInfo[ni].offset = writtenSize;

			writtenSize += WadFileInfo.lumpInfo[oi].size;

			oi++;
			ni++;
		}
		else
		{
			// THINGS ~ BLOCKMAP
			for(i = 1; i < 11; i++, oi++, ni++)
			{
				ReadAtLump(oi, buffer);
				WriteLump(handle, buffer, WadFileInfo.lumpInfo[oi].size);

				memcpy(&outputLumpInfo[ni], &WadFileInfo.lumpInfo[oi], sizeof(WadLumpInfo_t));
				outputLumpInfo[ni].offset = writtenSize;

				writtenSize += WadFileInfo.lumpInfo[oi].size;
			}

			// BEHAVIOR
			if(MapInfo[mapIndex].behavior)
			{
				if(MapInfo[mapIndex].behaviorIndex != -1)
				{
					// Replace the original
					ScriptInfo_t *scriptInfo = &BehaviorScriptInfo[MapInfo[mapIndex].behaviorIndex];

					WriteLump(handle, scriptInfo->buffer, scriptInfo->size);

					strncpy(outputLumpInfo[ni].name, "BEHAVIOR", 8);
					outputLumpInfo[ni].offset = writtenSize;
					outputLumpInfo[ni].size = scriptInfo->size;

					writtenSize += scriptInfo->size;

					PrintColor(FOREGROUND_CYAN, "- BEHAVIOR \"%s\" inserts into %s, %d bytes (%s)\n",
					                            scriptInfo->fileName,
					                            WadFileInfo.lumpInfo[MapInfo[mapIndex].lumpIndex].name,
					                            scriptInfo->size,
					                            SizeToString(scriptInfo->size));
				}
				else
				{
					// Just copy
					ReadAtLump(oi, buffer);
					WriteLump(handle, buffer, WadFileInfo.lumpInfo[oi].size);

					memcpy(&outputLumpInfo[ni], &WadFileInfo.lumpInfo[oi], sizeof(WadLumpInfo_t));
					outputLumpInfo[ni].offset = writtenSize;

					writtenSize += WadFileInfo.lumpInfo[oi].size;
				}

				oi++;
				ni++;
			}
		}

		nextMapLump = mapIndex + 1 < MapCount ? MapInfo[++mapIndex].lumpIndex : WadFileInfo.header.lumpNumber;
	}

	// Write directories
	if(!WriteBytes(handle, outputLumpInfo, sizeof(WadLumpInfo_t) * OutputLumpCount))
	{
		Error("Fail to write lump directory list!");
		Exit();
	}
	newFileSize = writtenSize + sizeof(WadLumpInfo_t) * OutputLumpCount;

	// Modify header
	if(!WriteIntAt(handle, 4, SEEK_SET, OutputLumpCount) ||
	   !WriteIntAt(handle, 8, SEEK_SET, writtenSize))
	{
		Error("Fail to write header!");
		Exit();
	}

	// Close output and cleanup
	close(handle);
	SetFileAttributes(fileName, FILE_ATTRIBUTE_NORMAL);
	_CloseFile(WadFileInfo.file);
	free(buffer);

	// Backup original WAD file
	strcpy(backupName, WadFileInfo.fileNameFull);
	*strrchr(backupName, '.') = '\0';
	sprintf(backupName, "%s.bak", backupName);

	SetFileAttributes(backupName, FILE_ATTRIBUTE_NORMAL);
	remove(backupName);
	SetFileAttributes(WadFileInfo.fileNameFull, FILE_ATTRIBUTE_NORMAL);
	if(rename(WadFileInfo.fileNameFull, backupName))
	{
		Warning("\nBacking up file error with \"%s\"\n", WadFileInfo.fileName);
		PrintColor(FOREGROUND_WHITE, "File \"%s\" output - %d bytes (%s)\n", GetFileNameNoPath(fileName), newFileSize, SizeToString(newFileSize));
	}
	else
	{
		rename(fileName, WadFileInfo.fileNameFull);
		PrintColor(FOREGROUND_WHITE, "\nFile \"%s\" output - %d bytes (%s)\n"
		                             "Backup file name: \"%s\"\n",
		                             WadFileInfo.fileName, newFileSize, SizeToString(newFileSize),
		                             GetFileNameNoPath(backupName));
	}
}
