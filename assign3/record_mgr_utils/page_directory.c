#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "page_directory.h"
#include "../buffer_mgr.h"
#include "record_index.h"
#define INITIAL_PAGES 10

void initPageDirectory(RM_TableData *rel){
    PageDirectory *directory = (PageDirectory *)malloc(sizeof(PageDirectory));
    directory->capacity = INITIAL_PAGES;
    bool* list = (bool *)malloc(directory->capacity*sizeof(bool));
    memset(list, '\0', directory->capacity*sizeof(bool));
    for(int i=0;i<directory->capacity;i++){
        directory->isFull[i] = false;
    }
    memcpy((char*)rel->mgmtData + sizeof(BM_BufferPool) + sizeof(RecordIndexLinkedList), directory, sizeof(PageDirectory));
    free(directory);
}

PageDirectory *getPageDirectory(RM_TableData *rel){
    return (PageDirectory *)((char*)rel->mgmtData + sizeof(BM_BufferPool) + sizeof(RecordIndexLinkedList));
}

void doublePageDirectory(RM_TableData *rel){
    PageDirectory *directory = getPageDirectory(rel);
    int newCap = 2*directory->capacity;
    directory->isFull = (bool *)realloc(directory->isFull, newCap);
    for(int i=directory->capacity;i<newCap;i++){
        directory->isFull[i] = false;
    }
    directory->capacity = newCap;
}

void destroyPageDirectory(RM_TableData *rel){
    free(getPageDirectory(rel)->isFull);
}