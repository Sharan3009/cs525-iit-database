#include "stdio.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "fcntl.h"
#include "stdlib.h"
#include "page_directory.h"

SM_PageHandle directory = NULL;
void initStorageManager() {
    // initialize page directory
    directory = (SM_PageHandle)malloc(PAGE_SIZE);
}

RC createPageFile(char *fileName){

    FILE *file = fopen(fileName, "r"); //open file in readmode
    // if file already exists, then stop
    if(file!=NULL){
        fclose(file);
        return RC_OK;
    }

    file = fopen(fileName, "w+"); //open in read-write mode
    if(file==NULL){
        return RC_FILE_NOT_FOUND;
    }

    SM_FileHandle* fHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));

    // setting mgmtInfo
    fHandle->mgmtInfo = (void*)file;

    // setting fileName
    fHandle->fileName = fileName;

    // setting curPage
    fHandle->curPagePos = 0;

    // setting totalPages
    fHandle->totalNumPages = 0;

    updatePageDirectory(fHandle, directory); // create empty directory
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

    // setting curPage
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
    updatePageDirectory(fHandle, directory); // update page directory
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
    if(pageNum<0 || pageNum>=fHandle->totalNumPages || directory[pageNum]=='\0')
        return RC_READ_NON_EXISTING_PAGE;
    long int offset = (pageNum+1)*PAGE_SIZE;
    if(fseek(fHandle->mgmtInfo, offset, 0)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            memPage[i] = fgetc(fHandle->mgmtInfo);
        }
        // fseek(fHandle->mgmtInfo, -PAGE_SIZE, ftell(fHandle->mgmtInfo));
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
    return readBlock(fHandle->totalNumPages-1, fHandle, memPage);
}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle==NULL || memPage==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    if(pageNum<0 || pageNum>=PAGE_SIZE || directory[pageNum]=='\0')
        return RC_WRITE_FAILED;
    long int offset = (pageNum+1)*PAGE_SIZE;
    if(fseek(fHandle->mgmtInfo, offset, 0)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            fputc(memPage[i], fHandle->mgmtInfo);
        }
        // fseek(fHandle->mgmtInfo, -PAGE_SIZE, ftell(fHandle->mgmtInfo));
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
    directory[fHandle->curPagePos] = '1';
    SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);
    RC ret = writeCurrentBlock(fHandle, ph);
    return ret;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    for(int i=0;i<(numberOfPages-fHandle->totalNumPages);i++){
        fHandle->curPagePos++;
        if(appendEmptyBlock(fHandle)!=RC_OK){
            return RC_WRITE_FAILED;
        }
    }
    return RC_OK;
}