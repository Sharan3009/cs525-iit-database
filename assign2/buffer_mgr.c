#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "dt.h"
#include "page_table.h"
#include "storage_mgr.h"
#include "replacement_strategy.h"

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		const int numPages, ReplacementStrategy strategy, void *stratData){
    
    // if bm variable no initialized then return
    if(bm==NULL)
        return RC_WRITE_FAILED;

    // initializing filename in bm
    bm->pageFile = strdup(pageFileName);

    // check if file exists
    SM_FileHandle fh;
    RC ret = openPageFile(bm->pageFile, &fh);
    if(ret!=RC_OK){
        return ret;
    }

    // initializing bm properties
    bm->numPages = numPages;
    bm->strategy = strategy;
    bm->mgmtData = NULL;
    // initializing pagetable
    initPageTable(bm, numPages);
    // initialize replacement strategy
    initReplacementStrategy(bm, stratData);

    // closing opened page file
    closePageFile(&fh);
    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm){

    RC res = forceFlushPool(bm);
    
    // if failed to flush then return
    if(res!=RC_OK)
        return res;

    // get pageTable contents
    PageTable* pageTable = getPageTable(bm);

    // check if all fixCounts are 0
    for(int i=0;i<pageTable->capacity;i++){
        PageEntry entry = pageTable->table[i];
        if(entry.fixCount!=0){
            return RC_WRITE_FAILED;
        }
    }
    // clear strategy related data
    clearStrategyData(bm);

    // bufferpool is stored in bm.mgmtData including metadata
    free(bm->mgmtData);
    bm->mgmtData = NULL;

    return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm){

    // if pagetable not initialized, then return
    if(bm==NULL || bm->mgmtData==NULL)
        return RC_WRITE_FAILED;

    // open file from harddisk to start writing
    PageTable* pageTable = getPageTable(bm);

    // writing dirty data
    for(int i=0;i<pageTable->capacity;i++){
        PageEntry entry = pageTable->table[i];
        if(entry.dirty && entry.fixCount==0){
            BM_PageHandle *page = MAKE_PAGE_HANDLE();
            page->pageNum = entry.pageNum;
            page->data = (char *)malloc(PAGE_SIZE);

            strcpy(page->data, entry.pageData);
            forcePage(bm, page);
            free(page);
        }
    }

    return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){

    // if pagetable not initialized or page not present
    if(bm==NULL || bm->mgmtData==NULL || page==NULL || page->data==NULL || page->pageNum<0)
        return RC_WRITE_FAILED;

    // check if it already has the page.
    int index = hasPage(bm, page->pageNum);
    if(index==-1){
        return RC_WRITE_FAILED;
    }

    // update the content in the pool
    index = putPage(bm, page);
    
    // mark the page as dirty
    markPageDirty(bm, index);
    return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){

    // if pagetable not initialized or page not present
    if(bm==NULL || bm->mgmtData==NULL || page==NULL || page->data==NULL || page->pageNum<0)
        return RC_WRITE_FAILED;

    // check if the given pageNum is in the pool
    int index = hasPage(bm, page->pageNum);
    if(index==-1)
        return RC_WRITE_FAILED;
    // if yes then decrement the fix count
    decrementPageFixCount(bm, index);
    return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){

    // if pagetable not initialized or page not present
    if(bm==NULL || bm->mgmtData==NULL || page==NULL || page->data==NULL || page->pageNum<0)
        return RC_WRITE_FAILED;

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
    incrementWriteCount(bm);
    unmarkPageDirty(bm, index);

    free(ph);
    // close file after writing
    closePageFile(&fh);
    return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
		const PageNumber pageNum){

    // handling negative page numbers
    if(pageNum<0 || bm==NULL || bm->mgmtData==NULL)
        return RC_WRITE_FAILED;

    page->pageNum = pageNum;
    int index = hasPage(bm, page->pageNum);
    PageTable *pageTable = getPageTable(bm); //get pageTable

    if(index==-1){
        // initialize filehandle variables
        SM_FileHandle fh;
        page->data = (char *) malloc(PAGE_SIZE);

        // readBlock from the file on disk
        openPageFile(bm->pageFile, &fh);
        ensureCapacity(pageNum+1, &fh);
        readBlock(pageNum, &fh, page->data);
        incrementReadCount(bm);
        closePageFile(&fh);
        index = putPage(bm, page);
        // if index=-1 or pageTable is full
        if(index == -1){

            //try evicting page
            PageNumber evictedPageNum = evictPage(bm);

            if(evictedPageNum==-1) // all pages are pinned
                return RC_WRITE_FAILED;

            // evict page from replacement strategy
            BM_PageHandle *evictedPage = MAKE_PAGE_HANDLE();
            evictedPage->pageNum = evictedPageNum;
            evictedPage->data = (char *)malloc(PAGE_SIZE);

            // get Data of the evicted page from page_table
            index = getPage(bm, evictedPage);
            if(pageTable->table[index].dirty){ // if dirty then force write
                forcePage(bm, evictedPage);
            }

            // delete page from the table
            deletePage(bm, evictedPage->pageNum);
            free(evictedPage);

            // try again putting page
            index = putPage(bm, page);
        }
        // whenever we add page in page_table, we admit page to replacement strategy
        admitPage(bm, &(pageTable->table[index]));
    } else {
        reorderPage(bm, &(pageTable->table[index]));
    }
    incrementPageFixCount(bm, index);
    return RC_OK;
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){

    // if page table not initialized
    if(bm==NULL || bm->mgmtData==NULL)
        return NULL;

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

    // if page table not initialized
    if(bm==NULL || bm->mgmtData==NULL)
        return NULL;

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

    // if page table not initialized
    if(bm==NULL || bm->mgmtData==NULL)
        return NULL;

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

    // if page table not initialized
    if(bm==NULL || bm->mgmtData==NULL)
        return -1;

    return getPageTable(bm)->readIOCount;
}
int getNumWriteIO (BM_BufferPool *const bm){

    // if page table not initialized
    if(bm==NULL || bm->mgmtData==NULL)
        return -1;

    return getPageTable(bm)->writeIOCount;
}
