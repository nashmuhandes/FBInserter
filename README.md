FBInserter
==========

This program and source code was uploaded with permission from carlcyber himself, the creator of this tool.

[15 JUNE 2014 IMPORTANT NOTICE]
The compiled binary version has the fixed functionality for TITLEMAPs, but the source code is from an old version WITHOUT the fix. carlcyber has unfortunately lost the source to the latest fix.

==========

FraggleScript and BEHAVIOR lump inserter
- Written by carlcyber
Date: February/13/2010

========================================
=             WHAT'S NEW?              =
========================================

13/02/2010: Fixed issue with not properly detecting TITLEMAP.

========================================
=             What Is This?            =
========================================
This is a small tool for inserting compiled ACS and FraggleScript files into a WAD, supports TEXTMAP (UDMF). The program is useful for inserting multiple files into multiple maps in a single drag and drop. Hoping this could help some people find a cheaper way of doing this.

========================================
=                 Usage                =
========================================
FBInserter.exe WadFileName InsertFileName [InsertMapName] ...

WadFileName:
  File name of the source WAD file to be inserted. This parameter is not
  restricted at the first parameter followed by the exe name. It can be
  anywhere.

InsertFileName [InsertMapName]:
  InsertFileName:
    Could be FraggleScript file (*.fs) or compiled ACS file (*.o).
  [InsertMapName]:
    Optional. Using this parameter would force to insert into the specified
    map by map name. Without this parameter, it will automatically search for
    the map name same as InsertFileName(without extension) as the insertion
    target.

...:
  Can use multiple InsertFileName [InsertMapName].

Extra parameter, case insensitive: -nopause
  This parameter disables stopping at the final result, but still halts the
  process when errors occur.

========================================
=         Command Line Example         =
========================================
FBInserter MyWad.wad script01.fs map01 script02.fs map02 map03.fs map04.fs ThisIsMap05.o mAp05 map06.o XD.o MaP07

WAD: MyWad.wad
MAP01 (FS) - script01.fs
MAP02 (FS) - script02.fs
MAP03 (FS) - map03.fs
MAP04 (FS) - map04.fs
MAP05 (BEHAVIOR) - ThisIsMap05.o
MAP06 (BEHAVIOR) - map06.o
MAP07 (BEHAVIOR) - XD.o
