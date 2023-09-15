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
    if(fHandle==NULL)
        return RC_FILE_HANDLE_NOT_INIT;

    // setting fileName
    fHandle->fileName = fileName;

    // setting fileContent
    fHandle->mgmtInfo = file;

    //setting totalNumPages
    fseek(file, 0, SEEK_END); // seek to end of file
    int size = ftell(file); //size of the file
    if(size==0){   //newly created file  
        appendEmptyBlock(fHandle); // page directory
        appendEmptyBlock(fHandle); // empty page
    }

    size -= PAGE_SIZE; //reducing directory size
    fseek(file, 0, SEEK_SET); // seek back to beginning of file
    fHandle->totalNumPages = size/PAGE_SIZE;


    // searching first empty block
    SM_PageHandle ph;
    for (i=0; i < PAGE_SIZE; i++){
        if(fgetc(file)=='\0'){
            fHandle->currPagePos = i;
            break;
        }
    }

    fseek(file, PAGE_SIZE, SEEK_SET); // seek back to first page
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
    if(fHandle==NULL || memPage==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    if(pageNum<1 || pageNum>fHandle->totalNumPages)
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
    return readBlock(1, fHandle, memPage);
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
    if(pageNum<1 || pageNum>fHandle->totalNumPages)
        return RC_WRITE_FAILED;
    long int offset = pageNum*PAGE_SIZE;
    if(fseek(fHandle->mgmtInfo, offset, 0)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            fputc(memPage[i], fHandle->mgmtInfo);
        }
        fHandle->curPagePos++;
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