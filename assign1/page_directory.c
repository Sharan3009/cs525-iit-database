#include "stdio.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "page_directory.h"

RC readPageDirectory(SM_FileHandle *fHandle, SM_PageHandle directory){
    if(fHandle==NULL || directory==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    RC ret;
    int pos = ftell(fHandle->mgmtInfo);
    if(fseek(fHandle->mgmtInfo, 0, SEEK_SET)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            directory[i] = fgetc(fHandle->mgmtInfo);
        }
        ret = RC_OK;
    } else {
        ret = RC_READ_NON_EXISTING_PAGE;
    }
    fseek(fHandle->mgmtInfo, pos, SEEK_SET);
    return ret;
}

RC updatePageDirectory(SM_FileHandle *fHandle, SM_PageHandle directory){
    if(fHandle==NULL || directory==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    if(fseek(fHandle->mgmtInfo, 0, SEEK_SET)==0){
        for(int i=0;i<PAGE_SIZE;i++){
            fputc(directory[i], fHandle->mgmtInfo);
        }
        return RC_OK;
    }
    return RC_WRITE_FAILED;
}