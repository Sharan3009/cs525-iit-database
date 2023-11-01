#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "dt.h"
#include "buffer_mgr_utils/page_table.h"
#include "storage_mgr.h"
#include "buffer_mgr_utils/replacement_strategy.h"

pthread_mutex_t pinLock;
pthread_mutex_t writeLock;

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		const int numPages, ReplacementStrategy strategy, void *stratData){
    
    // if bm variable not initialized then return
    if(bm==NULL)
        return RC_WRITE_FAILED;

    // initializing filename in bm
    bm->pageFile = strdup(pageFileName);

    // check if file exists
    SM_FileHandle fh;
    RC ret = openPageFile(bm->pageFile, &fh);
    if(ret!=RC_OK){
        free(bm->pageFile);
        bm->mgmtData = NULL;
        return ret;
    }

    // initializing bm properties
    bm->numPages = numPages;
    bm->strategy = strategy;
    bm->mgmtData = NULL;

    // initializing pagetable
    initPageTableAndStats(bm);

    // initialize replacement strategy
    initReplacementStrategy(bm, stratData);

    // closing opened page file
    closePageFile(&fh);

    // initialize locks
    pthread_mutex_init(&pinLock, NULL);
    pthread_mutex_init(&writeLock, NULL);
    
    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm){

    // if pagetable not initialized, then return
    if(bm==NULL || bm->mgmtData==NULL)
        return RC_WRITE_FAILED;

    RC res = forceFlushPool(bm);

    // get pageTable contents
    PageTable* pageTable = getPageTable(bm);

    // check if all fixCounts are 0
    for(int i=0;i<pageTable->capacity;i++){
        PageEntry entry = pageTable->table[i];
        if(entry.fixCount!=0){
            res = RC_WRITE_FAILED;
            break;
        }
    }

    // clear strategy related data
    clearStrategyData(bm);

    // clear bookkeeping
    clearPageTableAndStatistics(bm);
    free(bm->mgmtData);

    // destroy locks
    pthread_mutex_destroy(&pinLock);
    pthread_mutex_destroy(&writeLock);

    return res;
}

RC forceFlushPool(BM_BufferPool *const bm){

    // if pagetable not initialized, then return
    if(bm==NULL || bm->mgmtData==NULL)
        return RC_WRITE_FAILED;

    PageTable* pageTable = getPageTable(bm);

    // writing dirty data using forcePage
    for(int i=0;i<pageTable->capacity;i++){
        PageEntry entry = pageTable->table[i];
        if(entry.dirty && entry.fixCount==0){
            BM_PageHandle *page = MAKE_PAGE_HANDLE();
            page->pageNum = entry.pageNum;
            page->data = (char *)malloc(PAGE_SIZE);

            strcpy(page->data, entry.pageData);
            if(forcePage(bm, page)!=RC_OK){
                free(page->data);
                free(page);
                return RC_WRITE_FAILED;
            }
            free(page->data);
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

    // critical sectin starts to write
    pthread_mutex_lock(&writeLock);
    // check if page table has the page, if not then return error
    int index = hasPage(bm, page->pageNum);
    if(index==-1){
        pthread_mutex_unlock(&writeLock); // release lock if error
        return RC_WRITE_FAILED;
    }

    // update the content in the pool
    index = putPage(bm, page);
    
    // mark the page as dirty
    markPageDirty(bm, index);
    //release lock otherwise
    pthread_mutex_unlock(&writeLock);
    return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){

    // if pagetable not initialized or page not present
    if(bm==NULL || bm->mgmtData==NULL || page==NULL || page->pageNum<0)
        return RC_WRITE_FAILED;
    
    // check if the given pageNum is in the pool
    int index = hasPage(bm, page->pageNum);
    RC res = RC_OK;
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
    if(openPageFile(bm->pageFile, &fh)!=RC_OK)
        return RC_FILE_NOT_FOUND;

    // writing dirty data
    int index = getPage(bm, page);
    if(index==-1){
        free(ph);
        closePageFile(&fh);
        return RC_WRITE_FAILED;
    }
    strcpy(ph, pageTable->table[index].pageData);

    if(writeBlock(pageTable->table[index].pageNum, &fh, ph)!=RC_OK){
        free(ph);
        closePageFile(&fh);
        return RC_WRITE_FAILED;
    }

    // updating stats
    incrementWriteCount(bm);

    // page is not dirty anymore
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

    memset(page, '\0', sizeof(BM_PageHandle));
    if(page->data==NULL)
        page->data = (char*)malloc(PAGE_SIZE);
    else
        page->data = (char*)realloc(page->data, PAGE_SIZE);
    memset(page->data, '\0', PAGE_SIZE);
    page->pageNum = pageNum;

    // try reading from frame
    int index = getPage(bm, page);

    PageTable *pageTable = getPageTable(bm); //get pageTable

    // MISS
    if(index==-1){
        // critical section starts

        // hold lock
        pthread_mutex_lock(&pinLock);

        // if some other thread has done the page replacement
        // then check again and try redoing pinPage
        // because this time it might have been solved by other thread
        if(hasPage(bm, pageNum)!=-1){
            free(page->data);
            free(page);
            page->data = NULL;
            pthread_mutex_unlock(&pinLock);
            return pinPage(bm, page, pageNum);
        }

        // initialize filehandle variables
        SM_FileHandle fh;

        // readBlock from the file on disk
        openPageFile(bm->pageFile, &fh);
        ensureCapacity(pageNum+1, &fh);
        readBlock(pageNum, &fh, page->data);

        // updating read stats
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
            free(evictedPage->data);
            free(evictedPage);

            // try again putting page
            index = putPage(bm, page);
        }
        // whenever we add page in page_table, we admit page to replacement strategy (oppostive of evict)
        admitPage(bm, &(pageTable->table[index]));
        // release lock
        pthread_mutex_unlock(&pinLock);
    }
    // HIT
    else 
    {
        reorderPage(bm, &(pageTable->table[index])); // just reorder their priorities based on strategy
    }

    // fix count ++
    incrementPageFixCount(bm, index);
    return RC_OK;
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){

    // if page table not initialized
    if(bm==NULL || bm->mgmtData==NULL)
        return NULL;

    PageTable *pageTable = getPageTable(bm);

    // get frame contents memory
    PageNumber* frameContents = getPageTableStatistics(bm)->frameContents;

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
    
    // get dirty flags memory
    bool* dirtyFlags = getPageTableStatistics(bm)->dirtyFlags;

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
    
    // get fix count memory
    int* fixCounts = getPageTableStatistics(bm)->fixCounts;
    
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
