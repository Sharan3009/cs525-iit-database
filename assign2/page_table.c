#include <stdio.h>
#include <stdlib.h>

#include "page_table.h"
#include "dt.h"

void initPageTable(PageTable *pageTable, int size){
    pageTable->size = size;
    pageTable->table = (PageEntry *)malloc(size * sizeof(PageEntry));
    for (int i = 0; i < size; ++i) {
        pageTable->table[i].occupied = 0; // Mark all slots as unoccupied
    }
}

int hashPage(PageNumber pageNum, int size){
    return pageNum % size;
}

int addPage(PageTable *pageTable, PageNumber pageNum, char* pageData) {
    int index = hashPage(pageNum, pageTable->size);

    if(isTableFull(pageTable)==true){
        return -1;
    }
    
    // Linear probing to find the next available slot
    while (pageTable->table[index].occupied) {
        index = (index + 1) % pageTable->size;
    }
    
    // Store the key-value pair in the found slot
    pageTable->table[index].pageNum = pageNum;
    pageTable->table[index].pageData = pageData;
    pageTable->table[index].occupied = 1;
    pageTable->actualSize++;
    return index;
}

char* getPage(PageTable *pageTable, PageNumber pageNum) {
    int index = hashPage(pageNum, pageTable->size);
    
    // Linear probing to find the key
    while (pageTable->table[index].occupied) {
        if (pageTable->table[index].pageNum == pageNum) {
            return pageTable->table[index].pageData; // Key found, return the corresponding value
        }
        index = (index + 1) % pageTable->size;
    }
    
    return NULL; // Key not found
}


int removePage(PageTable *pageTable, PageNumber pageNum) {
    int index = hashPage(pageNum, pageTable->size);
    
    // Linear probing to find the key
    while (pageTable->table[index].occupied) {
        if (pageTable->table[index].pageNum == pageNum) {
            // Mark the slot as "deleted"
            pageTable->table[index].occupied = 0;
            return 0; // Key found and marked as deleted
        }
        index = (index + 1) % pageTable->size;
    }
    
    return -1; // Key not found
}

void clearTable(PageTable *pageTable) {
    free(pageTable->table);
}

bool isTableFull(PageTable *pageTable) {
    return pageTable->actualSize == pageTable->size;
}