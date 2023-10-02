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
    memset(ph, '\0', PAGE_SIZE);

    // open file from harddisk to start writing
    openPageFile(bm->pageFile, &fh);

    // writing dirty data
    for(int i=0;i<pageTable->capacity;i++){
        PageEntry entry = pageTable->table[i];
        if(entry.dirty && entry.fixCount==0){
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
    int index = putPage(bm, page);
    if(index==-1){
        return RC_WRITE_FAILED;
    }
    markPageDirty(bm, index);
    return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    int index = hasPage(bm, page->pageNum);
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
    memset(ph, '\0', PAGE_SIZE);

    // open file from harddisk to start writing
    openPageFile(bm->pageFile, &fh);

    // writing dirty data
    int index = getPage(bm, page);
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
    int index = hasPage(bm, page->pageNum);
    if(index==-1){
        // initialize filehandle variables
        SM_FileHandle fh;
        page->data = (char *) malloc(PAGE_SIZE);

        // readBlock from the file on disk
        openPageFile(bm->pageFile, &fh);
        ensureCapacity(pageNum+1, &fh);
        readBlock(pageNum, &fh, page->data);
        closePageFile(&fh);

        index = putPage(bm, page);
    }
    incrementPageFixCount(bm, index);
    return RC_OK;
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){

    PageTable *pageTable = getPageTable(bm);
    // Allocate memory for the frame contents array
    PageNumber *frameContents = (PageNumber *)malloc(bm->numPages * sizeof(PageNumber));

    // Iterate through the page table and populate the frameContents array
    for (int i = 0; i < bm->numPages; i++) {
        if (pageTable->table[i].pageNum != -1) {
            frameContents[i] = pageTable->table[i].pageNum;
        } else {
            frameContents[i] = NO_PAGE;
        }
    }

    return frameContents;
}

bool *getDirtyFlags (BM_BufferPool *const bm){

    PageTable *pageTable = getPageTable(bm);
    // Allocate memory for the dirty flags array
    bool *dirtyFlags = (bool *)malloc(bm->numPages * sizeof(bool));

    // Iterate through the page table and populate the dirtyFlags array
    for (int i = 0; i < bm->numPages; i++) {
        dirtyFlags[i] = pageTable->table[i].dirty;
    }
    
    return dirtyFlags;
}

int *getFixCounts (BM_BufferPool *const bm){

    PageTable *pageTable = getPageTable(bm);
    // Allocate memory for the dirty flags array
    int *fixCounts = (int *)malloc(bm->numPages * sizeof(int));

    // Iterate through the page table and populate the dirtyFlags array
    for (int i = 0; i < bm->numPages; i++) {
        fixCounts[i] = pageTable->table[i].fixCount;
    }
    
    return fixCounts;
}
int getNumReadIO (BM_BufferPool *const bm){
    return 0;
}
int getNumWriteIO (BM_BufferPool *const bm){
    return 0;
}
