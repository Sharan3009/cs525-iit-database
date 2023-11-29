#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "page_directory.h"
#include "../buffer_mgr.h"
#include "record_index.h"
#define INITIAL_PAGES 10

void initPageDirectory(RM_TableData *rel){
    PageDirectory *directory = (PageDirectory *)malloc(sizeof(PageDirectory));
    memset(directory, '\0', sizeof(PageDirectory));
    directory->capacity = INITIAL_PAGES;
    directory->isFull = (bool *)malloc(directory->capacity*sizeof(bool));
    memset(directory->isFull, '\0', directory->capacity*sizeof(bool));
    directory->isFull[0] = true; // because first page is schema
    for(int i=1;i<directory->capacity;i++){
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
    directory->isFull = (bool *)realloc(directory->isFull, newCap*sizeof(bool));
    for(int i=directory->capacity;i<newCap;i++){
        directory->isFull[i] = false;
    }
    directory->capacity = newCap;
}

void destroyPageDirectory(RM_TableData *rel){
    free(getPageDirectory(rel)->isFull);
}

PageNumber getEmptyPage(RM_TableData *rel){
    PageDirectory *directory = getPageDirectory(rel);
    PageNumber i;
    for(i=1;i<directory->capacity;i++){ // because first page is schema
        if(directory->isFull[i]==false){
            return i;
        }
    }
    doublePageDirectory(rel);
    return i;
}

void updatePageInPageDirectory(RM_TableData *rel, PageNumber i, bool isFull){
    PageDirectory *directory = getPageDirectory(rel);
    while(i>=directory->capacity){
        doublePageDirectory(rel);
    }
    if(i==0) return; // because firs tpage is schema
    directory->isFull[i] = isFull;
}