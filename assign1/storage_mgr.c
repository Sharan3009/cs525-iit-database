#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "page_directory.h"

#define READ "r"
#define WRITE "wb+"
#define READ_WRITE "rb+"

SM_PageHandle directory = NULL;
void initStorageManager() {
    // initialize page directory
    directory = (SM_PageHandle)malloc(PAGE_SIZE);
    memset(directory, '\0', PAGE_SIZE);
}

RC createPageFile(char *fileName){

    // Check if file already exists
    FILE *file = fopen(fileName, READ);
    if(file!=NULL){
        fclose(file);
        return RC_OK;
    }

    // Creates file if it does not exist
    file = fopen(fileName, WRITE);
    if(file==NULL){
        return RC_FILE_NOT_FOUND;
    }

    //fHandle initialization
    SM_FileHandle fHandle;
    fHandle.mgmtInfo = (void*)file;
    fHandle.fileName = fileName;
    fHandle.curPagePos = 0;
    fHandle.totalNumPages = 0;

    //add empty page directory in the file
    directory = (SM_PageHandle)realloc(directory, PAGE_SIZE);
    memset(directory, '\0', PAGE_SIZE);
    RC code;
    if((code = updatePageDirectory(&fHandle, directory))!=RC_OK)
        return code;

    //added empty page
    if((code = appendEmptyBlock(&fHandle))!=RC_OK)
        return code;

    // close fHandle
    if((code = closePageFile(&fHandle))!=RC_OK)
        return code;
    return RC_OK;
}

RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    // open file in read-write mode
    FILE *file = fopen(fileName, READ_WRITE);
    if(file==NULL)
        return RC_FILE_NOT_FOUND;
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;

    // setting fHandle properties
    fHandle->fileName = fileName;
    fHandle->mgmtInfo = file;

    directory = (SM_PageHandle)realloc(directory, PAGE_SIZE);
    RC code;
    if((code = readPageDirectory(fHandle, directory))!=RC_OK)
        return code;

    fHandle->curPagePos = 0;

    //setting totalPages
    fHandle->totalNumPages = 0;
    for(int i=0;i<PAGE_SIZE;i++){
        if(directory[i]=='1')
            fHandle->totalNumPages++;
    }

    return RC_OK;
}

RC closePageFile (SM_FileHandle *fHandle){
    //before closing page, make sure directory is saved in the file
    RC code;
    if(code = (updatePageDirectory(fHandle, directory))!=RC_OK)
        return RC_FILE_NOT_FOUND;
    if(fclose(fHandle->mgmtInfo)==0){
        return RC_OK;
    }
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
    // if page is outside the range of directory, or no entry in the directory
    if(pageNum<0 || pageNum>=fHandle->totalNumPages || directory[pageNum]=='\0')
        return RC_READ_NON_EXISTING_PAGE;

    long int offset = (pageNum+1)*PAGE_SIZE; //+1 because first page is directory
    if(fseek(fHandle->mgmtInfo, offset, 0)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            memPage[i] = fgetc(fHandle->mgmtInfo);
        }
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
    RC ret = readBlock(0, fHandle, memPage);
    if(ret == RC_OK){
        fHandle->curPagePos = 0;
    }
    return ret;
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    RC ret = readBlock(fHandle->curPagePos-1, fHandle, memPage);
    if(ret==RC_OK){
        fHandle->curPagePos--;
    }
    return ret;
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    RC ret = readBlock(fHandle->curPagePos+1, fHandle, memPage);
    if(ret==RC_OK){
        fHandle->curPagePos++;
    }
    return ret;
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    RC ret = readBlock(fHandle->totalNumPages-1, fHandle, memPage);
    if(ret==RC_OK){
        fHandle->curPagePos = fHandle->totalNumPages-1;
    }
    return ret;
}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL || memPage==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    // if page is outside the range of directory, or no entry in the directory
    if(pageNum<0 || pageNum>=PAGE_SIZE || ensureCapacity(pageNum, fHandle)!=RC_OK)
        return RC_WRITE_FAILED;
    
    long int offset = (pageNum+1)*PAGE_SIZE; //+1 is for first page is directory
    if(fseek(fHandle->mgmtInfo, offset, 0)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            fputc(memPage[i], fHandle->mgmtInfo);
        }
        if(directory[pageNum]=='\0'){
            directory[pageNum] = '1'; // set index in directory to used
            fHandle->totalNumPages++; // increase number of total pages
        }
        return RC_OK;
    }
    return RC_WRITE_FAILED;
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
    memset(ph, '\0', PAGE_SIZE);
    RC ret = writeBlock(fHandle->totalNumPages, fHandle, ph);
    free(ph);
    return ret;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    //appendEmptyBlocks for remaining number of pages
    int totalPages = fHandle->totalNumPages;
    for(int i=0;i<(numberOfPages-totalPages);i++){
        if(appendEmptyBlock(fHandle)!=RC_OK){
            return RC_WRITE_FAILED;
        }
    }
    return RC_OK;
}