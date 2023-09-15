#include "stdio.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "fcntl.h"
#include "stdlib.h"

void initStorageManager() {

}

RC createPageFile(char *fileName){

    FILE *file = fopen(fileName, "r"); //open file in readmode
    // if file already exists, then stop
    if(file!=NULL){
        fclose(file);
        return RC_OK;
    }

    // initialize fHandle
    SM_FileHandle *fHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));

    file = fopen(fileName, "w"); //open in write mode
    if(file==NULL){
        fclose(file);
        return RC_FILE_NOT_FOUND;
    }

    // setting mgmtInfo
    fHandle->mgmtInfo = (void*)file;

    // setting fileName
    fHandle->fileName = fileName;

    appendEmptyBlock(fHandle); // add empty page
    closePageFile(fHandle);
    return RC_OK;
}

RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    FILE *file = fopen(fileName, "r+");
    if(file==NULL)
        return RC_FILE_NOT_FOUND;
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;

    // setting fileName
    fHandle->fileName = fileName;

    // setting fileContent
    fHandle->mgmtInfo = file;

    //setting totalNumPages
    fseek(file, 0, SEEK_END); // seek to end of file
    int size = ftell(file); //size of the file
    fseek(file, 0, SEEK_SET); // seek back to the first page
    fHandle->totalNumPages = size/PAGE_SIZE;

    return RC_OK;
}

RC closePageFile (SM_FileHandle *fHandle){
    if(fclose(fHandle->mgmtInfo)==0)
        return RC_OK;
    return RC_FILE_NOT_FOUND;
}

RC destroyPageFile (char *fileName){
    if(remove(fileName)==0)
        return RC_OK;
    return RC_FILE_NOT_FOUND;
}

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL || memPage==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    if(pageNum<0 || pageNum>fHandle->totalNumPages)
        return RC_READ_NON_EXISTING_PAGE;
    long int offset = pageNum*PAGE_SIZE;
    if(fseek(fHandle->mgmtInfo, offset, 0)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            memPage[i] = fgetc(fHandle->mgmtInfo);
        }
        fHandle->curPagePos++;
        return RC_OK;
    }
    return RC_READ_NON_EXISTING_PAGE;
}

int getBlockPos (SM_FileHandle *fHandle){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    return fHandle->curPagePos;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(0, fHandle, memPage);
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    return readBlock(fHandle->curPagePos--, fHandle, memPage);
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    return readBlock(fHandle->curPagePos++, fHandle, memPage);
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    return readBlock(fHandle->totalNumPages, fHandle, memPage);
}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL || memPage==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    if(pageNum<0)
        return RC_WRITE_FAILED;
    long int offset = pageNum*PAGE_SIZE;
    if(fseek(fHandle->mgmtInfo, offset, 0)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            fputc(memPage[i], fHandle->mgmtInfo);
        }
        fHandle->curPagePos++;
        fHandle->totalNumPages++;
        return RC_OK;
    }
    return RC_OK;
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

RC appendEmptyBlock (SM_FileHandle *fHandle){
    if(fHandle==NULL){
        return RC_FILE_HANDLE_NOT_INIT;
    }
    SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);
    RC ret = writeCurrentBlock(fHandle, ph);
    return ret;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    return RC_OK;
}