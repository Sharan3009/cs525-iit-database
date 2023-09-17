#include "stdio.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "page_directory.h"

RC readPageDirectory(SM_FileHandle *fHandle, SM_PageHandle directory){
    if(fHandle==NULL || directory==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    for(int i=0;i<PAGE_SIZE;i++){
        directory[i] = fgetc(fHandle->mgmtInfo);
    }
    return RC_OK;
}

RC updatePageDirectory(SM_FileHandle *fHandle, SM_PageHandle directory){
    if(fHandle==NULL || directory==NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    fseek(fHandle->mgmtInfo, 0, SEEK_SET);
    for(int i=0;i<PAGE_SIZE;i++){
        fputc(directory[i], fHandle->mgmtInfo);
    }
    return RC_OK;
}