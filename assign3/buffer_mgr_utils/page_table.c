#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "page_table.h"

void initPageTableAndStats(BM_BufferPool *const bm){

    int capacity = bm->numPages;
    bm->mgmtData = (void*)malloc(sizeof(PageTable) + sizeof(PageTableStatistics));

    PageTable* pageTable = (PageTable*)malloc(sizeof(PageTable));
    pageTable->table = (PageEntry *)malloc(capacity*sizeof(PageEntry));

    for (int i = 0; i < capacity; ++i) {
        pageTable->table[i].pageNum = -1; // initially all pages are unoccupied
        pageTable->table[i].pageData = (char*)malloc(PAGE_SIZE);
        pageTable->table[i].dirty = false;
        pageTable->table[i].fixCount = 0;
    }
    pageTable->capacity = capacity;
    pageTable->size = 0;
    pageTable->readIOCount = 0;
    pageTable->writeIOCount = 0;

    PageTableStatistics* pageTableStats = (PageTableStatistics*)malloc(sizeof(PageTableStatistics));
    pageTableStats->fixCounts = (int*)malloc(capacity*sizeof(int));
    pageTableStats->dirtyFlags = (bool*)malloc(capacity*sizeof(bool));
    pageTableStats->frameContents = (PageNumber*)malloc(capacity*sizeof(PageNumber));

    for (int i=0;i<capacity;i++){
        pageTableStats->fixCounts[i] = 0;
        pageTableStats->dirtyFlags[i] = false;
        pageTableStats->frameContents[i] = NO_PAGE;
    }

    memcpy(bm->mgmtData, pageTable, sizeof(PageTable));
    memcpy(bm->mgmtData + sizeof(PageTable), pageTableStats, sizeof(PageTableStatistics));
    free(pageTable);
    free(pageTableStats);
}

int putPage(BM_BufferPool *const bm, BM_PageHandle *const page){
    // Retrieve the PageTable from bm->mgmtData
    PageTable *pageTable = getPageTable(bm);

    int index = hasPage(bm, page->pageNum); // check if pageNum is already in the table

    if(index==-1){ // if its not then try allocate new space
        if(isTableFull(bm))
            return -1;

        // Calculate the hash index for the given page number
        index = generatePageTableHash(pageTable, page->pageNum);

        // Linear probing to find the next available slot
        while (pageTable->table[index].pageNum != -1) {
            index = (index + 1) % pageTable->capacity; // Move to the next slot (wrap around if necessary)
        }
        // Update the size of the PageTable
        pageTable->size++;
    }

    // Insert the entry into the found slot
    pageTable->table[index].pageNum = page->pageNum;
    strcpy(pageTable->table[index].pageData, page->data); // Copy page data

    return index;
}

int getPage(BM_BufferPool *const bm, BM_PageHandle *const page){
    // Retrieve the PageTable from bm->mgmtData
    PageTable *pageTable = getPageTable(bm);

    int index = hasPage(bm, page->pageNum); // check if pageNum is already in the table

    if(index==-1){
        return index;
    }

    // Check if the page is found
    if (pageTable->table[index].pageNum == page->pageNum) {
        // Page found, populate BM_PageHandle with page data
        page->data = pageTable->table[index].pageData;
        return index; // return index in the pagetable
    }
    // Page not found, return -1
    return -1;
}

int deletePage(BM_BufferPool *const bm, PageNumber pageNum) {
    // Retrieve the PageTable from bm->mgmtData
    PageTable *pageTable = getPageTable(bm);

    int index = hasPage(bm, pageNum); // check if pageNum is already in the table

    if(index==-1){
        return index;
    }

    PageEntry *page = &pageTable->table[index];
    // Check if the page with pageNum is found
    if (page->pageNum == pageNum) {
        // Page found, mark the entry as unoccupied (-1 for pageNum)
        page->pageNum = -1;

        // Free the memory associated with the removed PageEntry
        memset(page->pageData, '\0', PAGE_SIZE);

        //Free fixCount and dirty bit
        page->fixCount = 0;
        page->dirty = false;

        // Update the size of the PageTable and return index
        pageTable->size--;
        return index; // Return index of hashtable from where it removed
    }

    return -1; // -1 indicates failure (page not found)
}

int hasPage(BM_BufferPool *const bm, PageNumber pageNum){
    // Retrieve the PageTable from bm->mgmtData
    PageTable *pageTable = getPageTable(bm);

    // Calculate the hash index for the given page number
    int index = generatePageTableHash(pageTable, pageNum);
    int initialIndex = index; 
    // Linear probing to find the page
    while (pageTable->table[index].pageNum != pageNum && pageTable->table[index].pageNum != -1) {
        index = (index + 1) % pageTable->capacity; // Move to the next slot (wrap around if necessary)

        if(index==initialIndex) //break if couldn't find index after searching all the table
            break;
    }

    // Check if the page is found
    if (pageTable->table[index].pageNum == pageNum) {
        return index; // return index in the pagetable
    }
    // Page not found, return -1
    return -1;
}

void incrementPageFixCount(BM_BufferPool *const bm, int pageIndex){
    changePageFixCount(bm, pageIndex, 1);
}

void decrementPageFixCount(BM_BufferPool *const bm, int pageIndex){
    changePageFixCount(bm, pageIndex, -1);
}

void markPageDirty(BM_BufferPool *const bm, int pageIndex){
    changePageDirty(bm, pageIndex, true);
}

void unmarkPageDirty(BM_BufferPool *const bm, int pageIndex){
    changePageDirty(bm, pageIndex, false);
}

void incrementReadCount(BM_BufferPool *const bm){
    PageTable *pageTable = getPageTable(bm);
    pageTable->readIOCount++;
}

void incrementWriteCount(BM_BufferPool *const bm){
    PageTable *pageTable = getPageTable(bm);
    pageTable->writeIOCount++;
}

PageTable* getPageTable(BM_BufferPool *const bm){
    return (PageTable *)bm->mgmtData;
}

PageTableStatistics* getPageTableStatistics(BM_BufferPool *const bm){
    return (PageTableStatistics*)(bm->mgmtData + sizeof(PageTable));
}

void clearPageTableAndStatistics(BM_BufferPool *const bm){
    PageTable* pageTable = getPageTable(bm);
    PageTableStatistics* pageTableStats = getPageTableStatistics(bm);
    for (int i=0;i<bm->numPages;i++){
        free(pageTable->table[i].pageData);
    }
    free(pageTable->table);
    free(pageTableStats->fixCounts);
    free(pageTableStats->dirtyFlags);
    free(pageTableStats->frameContents);
}

static void changePageFixCount(BM_BufferPool *const bm, int pageIndex, int val){
    PageEntry *entry = &getPageTable(bm)->table[pageIndex];
    entry->fixCount+=val;
    if(entry->fixCount<0)
        entry->fixCount = 0;
}

static void changePageDirty(BM_BufferPool *const bm, int pageIndex, bool boolean){
    (&getPageTable(bm)->table[pageIndex])->dirty = boolean;
}

static bool isTableFull(BM_BufferPool *const bm){
    PageTable *pageTable = getPageTable(bm);

    if(pageTable->size == pageTable->capacity)
        return true;

    return false;
}

static int generatePageTableHash(PageTable * pageTable, PageNumber pageNum){
    return pageNum % pageTable->capacity;
}