#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "page_directory.h"
#include "../buffer_mgr.h"
#include "record_index.h"
#include "record_mgr_serializer.h"
#define INITIAL_PAGES 10

#define READ "r"
#define WRITE "wb+"

void initPageDirectory(RM_TableData *rel){
    // opening file
    char *fileName = getDirectoryFileName(rel->name);
    FILE *file = fopen(fileName, READ);
    PageDirectory *directory = (PageDirectory *)malloc(sizeof(PageDirectory));
    memset(directory, '\0', sizeof(PageDirectory));

    //file never existed
    if(file==NULL){
        directory->capacity = INITIAL_PAGES;
        directory->isFull = (bool *)malloc(directory->capacity*sizeof(bool));
        memset(directory->isFull, '\0', directory->capacity*sizeof(bool));
        directory->isFull[0] = true; // because first page is schema
        for(int i=1;i<directory->capacity;i++){
            directory->isFull[i] = false;
        }
        memcpy((char*)rel->mgmtData + sizeof(BM_BufferPool) + sizeof(RecordIndexLinkedList), directory, sizeof(PageDirectory));
    } else {
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate memory for the entire file
        char* page = (char*)malloc(fileSize); 

        fread(page, 1, fileSize, file);

        deserializePageDirectoryFromPage(directory, page);

        memcpy((char*)rel->mgmtData + sizeof(BM_BufferPool) + sizeof(RecordIndexLinkedList), directory, sizeof(PageDirectory));

        free(page);  
        fclose(file);
    }
    free(fileName);
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

void closePageDirectory(RM_TableData *rel){
    
    PageDirectory *directory = getPageDirectory(rel);
    char *fileName = getDirectoryFileName(rel->name);
    char *page;
    int size = serializePageDirectoryIntoPage(directory, &page);
    
    FILE* file = fopen(fileName, WRITE);

    fwrite(page, 1, size, file);

    fclose(file);

    free(fileName);
    free(directory->isFull);
    free(page);
}

void deletePageDirectory(char *name){
    char* fileName = getDirectoryFileName(name);
    remove(fileName);
    free(fileName);
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

char *getDirectoryFileName(char *name){
    char *fileName = malloc(strlen(name) + strlen("_directory") + 1);
    strcpy(fileName, name);
    strcat(fileName, "_directory");
    return fileName;
}