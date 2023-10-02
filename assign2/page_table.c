#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "page_table.h"
#include "dt.h"

void initPageTable(BM_BufferPool *const bm, int capacity){

    bm->mgmtData = malloc(sizeof(PageTable) + capacity*sizeof(PageEntry) + capacity*PAGE_SIZE);

    PageTable *pageTable = getPageTable(bm);
    pageTable->table = (PageEntry *)((char *)bm->mgmtData + sizeof(PageTable));

    for (int i = 0; i < capacity; ++i) {
        pageTable->table[i].pageNum = -1; // initially all pages are unoccupied
        pageTable->table[i].pageData = (char *)bm->mgmtData + sizeof(PageTable) + sizeof(PageEntry) * capacity + i*PAGE_SIZE;
        pageTable->table[i].dirty = false;
        pageTable->table[i].fixCount = 0;
    }
    pageTable->capacity = capacity;
    pageTable->size = 0;
    bm->mgmtData = (void *)pageTable;
}

int writePage(BM_BufferPool *const bm, BM_PageHandle *const page){
    // Retrieve the PageTable from bm->mgmtData
    PageTable *pageTable = getPageTable(bm);

    int index = readPage(bm, page); // check if pageNum is already in the table

    if(index==-1){ // if its not then try allocate new space
        if(isTableFull(bm))
            return -1;

        // Calculate the hash index for the given page number
        index = hashPage(pageTable, page->pageNum);

        // Linear probing to find the next available slot
        while (pageTable->table[index].pageNum != -1) {
            index = (index + 1) % pageTable->capacity; // Move to the next slot (wrap around if necessary)
        }
    }

    // Insert the entry into the found slot
    pageTable->table[index].pageNum = page->pageNum;
    strcpy(pageTable->table[index].pageData, page->data); // Copy page data

    // Update the size of the PageTable
    pageTable->size++;

    return index;
}

int readPage(BM_BufferPool *const bm, BM_PageHandle *const page){
    // Retrieve the PageTable from bm->mgmtData
    PageTable *pageTable = getPageTable(bm);

    // Calculate the hash index for the given page number
    int index = hashPage(pageTable, page->pageNum);
    int initialIndex = index; 
    // Linear probing to find the page
    while (pageTable->table[index].pageNum != page->pageNum && pageTable->table[index].pageNum != -1) {
        index = (index + 1) % pageTable->capacity; // Move to the next slot (wrap around if necessary)

        if(index==initialIndex) //break if couldn't find index after searching all the table
            break;
    }

    // Check if the page is found
    if (pageTable->table[index].pageNum == page->pageNum) {
        // Page found, populate BM_PageHandle with page data
        strcpy(page->data, pageTable->table[index].pageData);

        return index; // return index in the pagetable
    }
    // Page not found, return -1
    return -1;
}

int deletePage(BM_BufferPool *const bm, PageNumber pageNum) {
    // Retrieve the PageTable from bm->mgmtData
    PageTable *pageTable = getPageTable(bm);

    // Calculate the hash index for the given page number
    int index = hashPage(pageTable, pageNum);

    // Linear probing to find the PageEntry with the specified pageNum
    while (pageTable->table[index].pageNum != pageNum && pageTable->table[index].pageNum != -1) {
        index = (index + 1) % pageTable->capacity; // Move to the next slot (wrap around if necessary)
    }

    // Check if the page with pageNum is found
    if (pageTable->table[index].pageNum == pageNum) {
        // Page found, mark the entry as unoccupied (-1 for pageNum)
        pageTable->table[index].pageNum = -1;

        // Free the memory associated with the removed PageEntry
        memset(pageTable->table[index].pageData, '\0', PAGE_SIZE);

        // Update the size of the PageTable and return index
        pageTable->size--;
        return index; // Return index of hashtable from where it removed
    }

    // Page not found, return an error code (-1)
    return -1; // -1 indicates failure (page not found)
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

void changePageFixCount(BM_BufferPool *const bm, int pageIndex, int val){
    getPageTable(bm)->table[pageIndex].fixCount += val;
}

void changePageDirty(BM_BufferPool *const bm, int pageIndex, bool boolean){
    getPageTable(bm)->table[pageIndex].dirty = boolean;
}

PageTable* getPageTable(BM_BufferPool *const bm){
    return (PageTable *)bm->mgmtData;
}

bool isTableFull(BM_BufferPool *const bm){
    PageTable *pageTable = getPageTable(bm);

    if(pageTable->size == pageTable->capacity)
        return true;

    return false;
}

int hashPage(PageTable * pageTable, PageNumber pageNum){
    return pageNum % pageTable->capacity;
}