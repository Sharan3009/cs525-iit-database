#include "stdio.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "fcntl.h"

void initStorageManager() {

}

RC createPageFile(char *fileName){
    FILE *file = fopen(fileName, O_CREAT);
    if(file==NULL)
        return RC_FILE_NOT_FOUND;
    return RC_OK;
}

RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    FILE *file = fopen(fileName, O_RDWR);
    if(file==NULL)
        return RC_FILE_NOT_FOUND;
    fHandle->fileName = fileName;
    fHandle->mgmtInfo = file;
    return RC_OK;
}

RC closePageFile (SM_FileHandle *fHandle){
    if(fclose(fHandle->fileName)==0)
        return RC_OK;
    return RC_FILE_NOT_FOUND;
}

RC destroyPageFile (char *fileName){
    if(remove(fileName)==0)
        return RC_OK;
    return RC_FILE_NOT_FOUND;
}

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    return RC_OK;
}

int getBlockPos (SM_FileHandle *fHandle){
    return 1;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return RC_OK;
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return RC_OK;
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return RC_OK;
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return RC_OK;
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return RC_OK;
}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    return RC_OK;
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return RC_OK;
}

RC appendEmptyBlock (SM_FileHandle *fHandle){
    return RC_OK;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    return RC_OK;
}