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

#include "filepath.h"


//clear a path entry string
static void _pathFreeStr(_pathStr *str){
    if(str->len > 0 && str->str){
        free(str->str);
    }
    str->str = NULL;
    str->len = 0;
    return;
}

//set a path entry string
static int _pathSetStr(_pathStr *str, const char *inStr){
    _pathFreeStr(str);
    size_t len = strlen(inStr);
    str->str = calloc(sizeof(char), len+1);
    if(!str->str){
        printf("error allocating path entry\n");
        return 0;
    }
    str->len = len;
    //memcpy(str->str, inStr, len);
    strncpy(str->str, inStr, len);
    return 1;
}



//updates pointer to extension start
static void _pathUpdateExt(FilePath *path){
    if(path->depth == 0 || path->folder) {
        path->ext = NULL;
        return;
    }

    int fileDepth = path->depth - 1;
    char *fileName = path->level[fileDepth].str;

    path->ext = strrchr(fileName, '.');
    if((int)(path->ext - fileName) < strlen(fileName))
        path->ext++;
}






int FilePath_Levels(FilePath *path){
    return path->depth;
}

char *FilePath_LevelStr(FilePath *path, int depth){
    if(depth >= 0 && depth < path->depth){
        return path->level[depth].str;
    }
    return NULL;
}

char *FilePath_FileName(FilePath *path){
    return FilePath_LevelStr(path, path->depth - 1);
}

char *FilePath_FileExt(FilePath *path){
    return path->ext;
}


//return the character length of a file path
int FilePath_Len(FilePath *path){
    //start off with root symbol size
    int len = (path->absolute != 0) *strlen(DIR_ROOT);

    for(int i = 0; i < path->depth; i++){
        len += path->level[i].len + strlen(DIR_SEPARATOR_STR); //add one for each path separator
    }

    //should be close enough
    return len;
}


void FilePath_Clean(FilePath *path){
    if(path->level && path->depth > 0){
	    for(int i = 0; i < path->depth; i++){
	        _pathFreeStr(&path->level[i]);
	    }
	    free(path->level);
	}
    path->level = NULL;
    path->depth = 0;
    path->absolute = 0;
    path->folder = 0;
    path->ext = NULL;
    path->levelsAllocd = 1;

    FilePath_DelStr(path);
    path->fullStr = NULL;
    return;
}

FilePath *FilePath_Init(void){
    FilePath *temp = calloc(sizeof(FilePath), 1);
    if(!temp) {
    	printf("error allocating filepath obj\n");
        return NULL;
    }

    temp->level = NULL;//make sure FilePath_Clean doesn't do anything tricky
    temp->depth = 0;
    FilePath_Clean(temp);
    return temp;
}


void FilePath_Destroy(FilePath *path) {
    if(!path) return;
    FilePath_Clean(path);
    free(path);
}







//grab a fully built path for an entry
int FilePath_ExportEx(FilePath *path, char *outBuf, int outBufLen, int depth){
    int pathLen = FilePath_Len(path)-1;
    if(pathLen <= 0) return 1;

    if(depth < 0 || depth > path->depth){
        printf("BAG: File Path Depth out of bounds\n");
        return 0;
    }

    //first calculate length so we know if buffer has enough room
    if( pathLen >= outBufLen){
        //printf("File path larger than buffer!\n");
        printf("BAG:File path > buffer size\n");
        return 0;
    }
    //make sure the buffer is cleared
    //memset(outBuf, 0, outBufLen);

    //ToDo: Make unicode safe
    //then we can build the path
    if(path->absolute)
    	strncat(outBuf, DIR_ROOT, outBufLen);
    else {
        char localRoot[4] = {'.', DIR_SEPARATOR, '\0'};
        if(strncmp(outBuf, localRoot, strlen(localRoot)))
        	strncat(outBuf, localRoot, outBufLen);
    }
    int i = 0;
    for(; i < depth - 1; i++){
        strncat(outBuf, path->level[i].str, outBufLen);
        strncat(outBuf, DIR_SEPARATOR_STR, outBufLen);
    }
    //finally append the last entry
    strncat(outBuf, path->level[i].str, outBufLen);
    return 1;
}

int FilePath_Export(FilePath *path, char *outBuf, int outBufLen){
    return FilePath_ExportEx(path, outBuf, outBufLen, path->depth);
}

int FilePath_ExportDirPath(FilePath *path, char *outBuf, int outBufLen){
    return FilePath_ExportEx(path, outBuf, outBufLen, path->depth - 1);
}



//get filepath represented as a string
char *FilePath_Str(FilePath *path) {
    FilePath_DelStr(path);
    size_t fullLen = FilePath_Len(path);
    path->fullStr = calloc(sizeof(char), fullLen);
    if(!path->fullStr ){
        printf("BAG: error allocating string export\n");
        return NULL;
    }

    FilePath_Export(path, path->fullStr , fullLen);
    return path->fullStr ;
}

//get the directory of a filepath as a string
char *FilePath_StrDir(FilePath *path) {
    FilePath_DelStr(path);
    size_t fullLen = FilePath_Len(path);
    path->fullStr = calloc(sizeof(char), fullLen);
    if(!path->fullStr ){
        printf("BAG: error allocating string export\n");
        return NULL;
    }

    FilePath_ExportDirPath(path, path->fullStr , fullLen);
    return path->fullStr ;
}


void FilePath_DelStr(FilePath *path) {
    if(path->fullStr ) free(path->fullStr );
    path->fullStr  = NULL;
}




int FilePath_SwapExt(FilePath *path, char *ext) {
    if(path->folder > 0) return 1; //don't do anything for folders

    //make backup of name
    char *fileName = FilePath_FileName(path);
    char *nameWExt = NULL;

    size_t len = strlen(fileName) + strlen(ext) +1;
    //check if there is a . marking the extension
    char *dot = strchr(ext, '.');
    len += (dot != NULL);

    nameWExt = calloc(sizeof(char), len);
    if(!nameWExt) {
        printf("Failed to swap extension\n");
        return 0;
    }

    char *origExt = strrchr(fileName, '.');
    if(origExt) {
        int cpyLen = (int)origExt - (int)fileName;
        strncpy(nameWExt, fileName, cpyLen);
    } else {
        strncpy(nameWExt, fileName, len);
    }

    if(!dot) strncat(nameWExt, ".", len);
    strncat(nameWExt, ext, len);
    int status = 0;
    //Now fix the file path
    if(FilePath_Remove(path)) {
        status = FilePath_Add(path, nameWExt);
    }

    //done, hopefully;
    free(nameWExt);
    return status;
}


//add an entry to the file path
int FilePath_Add(FilePath *path, const char *entry){
    char localRoot[4] = {'.', DIR_SEPARATOR, '\0'};
    char *addPath = (char*)entry;

    //ignore "this" directory
    if(!strncmp(addPath, ".", strlen(addPath))) return 1;

    //ignore locality, since it just means current directory anyway
    else if(!strncmp(addPath, localRoot, strlen(localRoot))) {
        addPath += strlen(localRoot);
        if(!(*addPath)) return 1; //all the entry contained was local ./
    }

    //if dealing with absolute paths, we can simplify them

    if(path->absolute){
        //printf("root path given\n");
        //go up a directory
        if(!strncmp(addPath, "..", strlen(addPath))){
            //printf("removing ..\n");
            return FilePath_Remove(path);
        }
        //do nothing if adding the current directory
        else if(!strncmp(addPath, ".", strlen(addPath))){
            //printf("this shouldn't be happening\n");
            return 1;
        }
    }
    //printf("adding: %s\n", entry);
    //exponentially grow buffer size
    if(path->depth + 1 >= path->levelsAllocd) {
        path->levelsAllocd = (path->levelsAllocd<<1);

        unsigned int size = sizeof(_pathStr) * path->levelsAllocd;
        _pathStr *temp = realloc(path->level, size);
        if(!temp){
            printf("error allocating extra path entry\n");
            return 0;
        }
        path->level = temp;
    }
    //clear out path string that was newly allocated
    path->level[path->depth].str = NULL;
    path->level[path->depth].len = 0;

    int rtrn = _pathSetStr(&path->level[path->depth], addPath);
    path->depth++;

    //add the entry
    return rtrn;
}


//equivalent of going back one directory or path entry
int FilePath_Remove(FilePath *path){
    if(path->depth <= 0) return 1;

    path->depth--;
     _pathFreeStr(&path->level[path->depth]);

    //exponential shrink of buffer
    if(path->depth < (path->levelsAllocd>>1)) {
        path->levelsAllocd = path->levelsAllocd>>1;

        unsigned int size = sizeof(_pathStr) * path->levelsAllocd;
        _pathStr *temp = realloc(path->level, size);
        if(!temp){
            printf("Error shrinking file path\n");
            return 0;
        }
        path->level = temp;
    }

    //if this path originally pointed to a file, it is now pointing to a folder
    if(path->folder == 0) {
        path->folder = 1;
        path->ext = NULL;//folders don't have extensions
    }

    return 1;
}



//change a string path into a path object
int FilePath_Convert(FilePath *path, const char *sourcePath, int len){
    char *resolvedPath = (char*)sourcePath;
    char cleanup = 0;
    path->levelsAllocd = 1;

//for unix systems, convert relative paths to absolute
#if (defined __gnu_linux__) || (defined __APPLE__ && defined __MACH__)
    resolvedPath = realpath(sourcePath, NULL);
    if(resolvedPath != NULL) {
        //successfully resolved the path, so get the length of the new path
        len = (int)strlen(resolvedPath);
        cleanup++;
    } else {
        //otherwise, it failed, default back to the given path
        resolvedPath = (char*)sourcePath;
    }


#endif

    //ignore paths containing just "./\0" or just ".\0" for platforms without realpath support
    char localRoot[4] = {'.', DIR_SEPARATOR, '\0'};
    if (!resolvedPath || !strncmp(resolvedPath, localRoot, strlen(resolvedPath)) ||
        !strncmp(resolvedPath, ".", strlen(resolvedPath)))
    {
        //missing path, so no path really defined
        if(cleanup)free(resolvedPath);
        return 1;
    }

    //printf("Converting: %s\n", sourcePath);

    if(!strncasecmp(resolvedPath, DIR_ROOT, strlen(DIR_ROOT))){
        path->absolute = 1;
    }

	//printf("path len: %d\n", len);
	char *temp = calloc(sizeof(char), len + 1);
	if(!temp){
		printf("error alloc path\n");
        goto _pathErr;
		return 0;
	}

	strcpy(temp, resolvedPath);
	strcat(temp, "\0");

	char delimiters[2] = {DIR_SEPARATOR, '\0'};
	char *entry = strtok(temp, delimiters);

    //printf("add loop!\n");
	while(entry){
		FilePath_Add(path, entry);
		entry = strtok(NULL, delimiters);
	}
    free(temp);


    path->folder = 0;
    struct stat st = {};
    if(!lstat(resolvedPath, &st))
        path->folder = S_ISDIR(st.st_mode);

    _pathUpdateExt(path);

    if(cleanup)free(resolvedPath);
	return 1;

    _pathErr:
        if(cleanup)free(resolvedPath);
        if(temp) free(temp);
        return 0;
}


int FilePath_Copy(FilePath *dest, FilePath *src){
    //clean up the destination
    FilePath_Clean(dest);

    dest->folder = src->folder;
    dest->absolute = src->absolute;

    for(int i = 0; i < src->depth; i++){
        if(!FilePath_Add(dest, FilePath_LevelStr(src, i))){
            printf("BAG:Error copying FilePath\n");
            return 0;
        }
    }

    _pathUpdateExt(dest);
    return 1;
}

int FilePath_Cat(FilePath *dest, FilePath *src){
	if(!dest || !src) return 1;
	//if we are cating a file name to a folder path, folder path becomes
	//a file path
	if(src->folder == 0 && dest->folder == 1) dest->folder = 0;

    for(int i = 0; i < src->depth; i++){
        if(!FilePath_Add(dest, FilePath_LevelStr(src, i))){
            printf("BAG:Error copying FilePath\n");
            return 0;
        }
    }

    _pathUpdateExt(dest);
    return 1;
}
