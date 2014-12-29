/*---------------------------------------------------------------------------------
 libBAG  Copyright (C) 2010 - 2013
  BassAceGold - <BassAceGold@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.

  3. Restricting the usage of or portions of code in this library
     from other software developers and or content creators
     is strictly prohibited in applications with software creation capabilities
     and or with the intent to profit from said restrictions.

  4. This notice may not be removed or altered from any source
     distribution.
---------------------------------------------------------------------------------*/

#ifndef _BAG_FILEPATH_
#define _BAG_FILEPATH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_STR "/"
#define DIR_ROOT "/"

#define MAX_PATH 512

#define lstat stat


#ifdef __cplusplus
extern "C" {
#endif

//file path string struct
typedef struct _pathStr_s{
	char *str;
	unsigned char len;
}_pathStr;


//file path structure, each level is a directory in a file path with the last being
//a file hopefully
typedef struct FilePath_s{
	_pathStr *level;
	int depth;
  char absolute, folder;
  char *ext;
  unsigned short levelsAllocd;
  char *fullStr;
}FilePath;

//clear and initialized filepath
extern FilePath *FilePath_Init(void);

//destroy a file path created with FilePath_Init
extern void FilePath_Destroy(FilePath *path);

//return length of stored file path
extern int FilePath_Len(FilePath *path);

//clean a given filepath
extern void FilePath_Clean(FilePath *path);

//add a directory entry to the file path
extern int FilePath_Add(FilePath *path, const char *entry);

//remove the last directory entry from a file path
extern int FilePath_Remove(FilePath *path);

//export file path as a string to a buffer
extern int FilePath_Export(FilePath *path, char *outBuf, int outBufLen);

//export file path to a newly allocated buffer, must be free'd afterward
extern char *FilePath_Str(FilePath *path);

//export directory path of a file, must be free'd afterward with FilePath_DelStr
extern char *FilePath_StrDir(FilePath *path);

//delete a string created by FilePath_Str
extern void FilePath_DelStr(FilePath *path);

//export directory path to an entry
extern int FilePath_ExportDirPath(FilePath *path, char *outBuf, int outBufLen);

//returns the number of directory levels in a file path
extern int FilePath_Levels(FilePath *path);

//get the last entry in a file path (usually a file name)
extern char *FilePath_FileName(FilePath *path);

//get the file extension from a file path, returns NULL if path is directory
extern char *FilePath_FileExt(FilePath *path);

//swap a file extension for a given file
extern int FilePath_SwapExt(FilePath *path, char *ext);

//Converts a string path to a path object
extern int FilePath_Convert(FilePath *path, const char *sourcePath, int len);

//copy one path to another
extern int FilePath_Copy(FilePath *dest, FilePath *src);

//concatenate one path to another
extern int FilePath_Cat(FilePath *dest, FilePath *src);
#ifdef __cplusplus
}
#endif


#endif
