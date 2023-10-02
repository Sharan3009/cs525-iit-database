#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "dt.h"
#include "page_table.h"
#include "storage_mgr.h"

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		const int numPages, ReplacementStrategy strategy, void *stratData){
    bm->pageFile = strdup(pageFileName);
    bm->numPages = numPages;
    bm->strategy = strategy;
    initPageTable(bm, numPages);
    return RC_OK;
}
RC shutdownBufferPool(BM_BufferPool *const bm){

    // get pageTable contents
    forceFlushPool(bm);
    PageTable* pageTable = getPageTable(bm);

    // check if all fixCounts are 0
    for(int i=0;i<pageTable->capacity;i++){
        PageEntry entry = pageTable->table[i];
        if(entry.fixCount!=0){
            return RC_WRITE_FAILED;
        }
    }
    // bufferpool is stored in bm.mgmtData including metadata
    free(bm->mgmtData);
    return RC_OK;
}
RC forceFlushPool(BM_BufferPool *const bm){

    PageTable* pageTable = getPageTable(bm);
    // initialize filehandle variables
    SM_FileHandle fh;
    SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);

    // open file from harddisk to start writing
    openPageFile(bm->pageFile, &fh);

    // writing dirty data
    for(int i=0;i<pageTable->capacity;i++){
        PageEntry entry = pageTable->table[i];
        if(entry.dirty){
            strcpy(ph, entry.pageData);
            writeBlock(entry.pageNum, &fh, ph);
            unmarkPageDirty(bm, i);
        }
    }

    free(ph);
    // close file after writing
    closePageFile(&fh);
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
    int index = writePage(bm, page);
    if(index==-1){
        return RC_WRITE_FAILED;
    }
    markPageDirty(bm, index);
    return RC_OK;
}
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    int index = readPage(bm, page);
    if(index==-1)
        return RC_WRITE_FAILED;
    decrementPageFixCount(bm, index);
    return RC_OK;
}
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    PageTable* pageTable = getPageTable(bm);
    // initialize filehandle variables
    SM_FileHandle fh;
    SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);

    // open file from harddisk to start writing
    openPageFile(bm->pageFile, &fh);

    // writing dirty data
    int index = readPage(bm, page);
    if(index==-1)
        return RC_WRITE_FAILED;
    strcpy(ph, pageTable->table[index].pageData);
    writeBlock(pageTable->table[index].pageNum, &fh, ph);
    unmarkPageDirty(bm, index);

    free(ph);
    // close file after writing
    closePageFile(&fh);
    return RC_OK;
}
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
		const PageNumber pageNum){
    page->pageNum = pageNum;
    int index = readPage(bm, page);
    if(index==-1){
        PageTable* pageTable = getPageTable(bm);
        // initialize filehandle variables
        SM_FileHandle fh;
        SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);

        // open file from harddisk to start reading
        openPageFile(bm->pageFile, &fh);
        ensureCapacity(pageNum, &fh);
        readBlock(pageNum, &fh, ph);
        page->data = ph;
        writePage(bm, page);
        free(ph);
    }
    incrementPageFixCount(bm, index);
    return RC_OK;
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){
    return RC_OK;
}
bool *getDirtyFlags (BM_BufferPool *const bm){
    bool *dirtyFlag = (bool *)malloc(sizeof(bool));
    return dirtyFlag;
}
int *getFixCounts (BM_BufferPool *const bm){
    int *fixCount = (int *)malloc(sizeof(int));
    return fixCount;
}
int getNumReadIO (BM_BufferPool *const bm){
    return 0;
}
int getNumWriteIO (BM_BufferPool *const bm){
    return 0;
}
