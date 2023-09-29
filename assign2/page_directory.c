#include "stdio.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "page_directory.h"

RC readPageDirectory(SM_FileHandle *fHandle, SM_PageHandle directory){
    if(fHandle==NULL || directory==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    RC ret;
    int pos = ftell(fHandle->mgmtInfo); // store current position of file pointer
    if(fseek(fHandle->mgmtInfo, 0, SEEK_SET)==0){ // move pointer to the beginning
        // read the directory
        for(int i=0;i<PAGE_SIZE;i++){
            directory[i] = fgetc(fHandle->mgmtInfo);
        }
        ret = RC_OK;
    } else {
        ret = RC_READ_NON_EXISTING_PAGE;
    }
    // move the pointer back
    fseek(fHandle->mgmtInfo, pos, SEEK_SET);
    return ret;
}

RC updatePageDirectory(SM_FileHandle *fHandle, SM_PageHandle directory){
    if(fHandle==NULL || directory==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    RC ret;
    int pos = ftell(fHandle->mgmtInfo); // store current position of file pointer
    if(fseek(fHandle->mgmtInfo, 0, SEEK_SET)==0){ // move pointer to the beginning
        // write the directory
        for(int i=0;i<PAGE_SIZE;i++){
            fputc(directory[i], fHandle->mgmtInfo);
        }
        ret = RC_OK;
    } else {
        ret = RC_READ_NON_EXISTING_PAGE;
    }
    // move the pointer back
    fseek(fHandle->mgmtInfo, pos, SEEK_SET);
    return ret;
}