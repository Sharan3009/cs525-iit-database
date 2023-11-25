#include "page_directory.h"

RC createDirectories(int currLevel, int totalLevels, PageNumber directoryPageNum, SM_FileHandle *fh){
    RC ret;
    // creating root directory
    if(ret = createDirectory(currLevel++, directoryPageNum, -1, fh)!=RC_OK){
        return ret;
    }
    
    PageNumber parentPageNum = directoryPageNum;
    SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);
    memset(ph, '\0', PAGE_SIZE);
    size_t sizeOfEntry = sizeof(DirectoryEntry);
    size_t availableSpace = PAGE_SIZE - sizeof(RecordPage);
    int numEntries = availableSpace / sizeOfEntry;
    while(currLevel<totalLevels){
        ret = readBlock(parentPageNum, fh, ph);
        if(ret!=RC_OK){
            free(ph);
            return ret;
        }
        DirectoryEntry *directoryEntries = (DirectoryEntry *)(RecordPage*)ph;
        for (int i=0; i<numEntries; i++) {
            DirectoryEntry *currentEntry = &directoryEntries[i];
            ret = createDirectory(currLevel, currentEntry->pageNum, directoryPageNum, fh);
            if(ret!=RC_OK){
                free(ph);
                return ret;
            }
            currentEntry = &directoryEntries[++i];
        }
        currLevel++;
    }
    free(ph);
    return ret;
}

RC createDirectory(int level, PageNumber directoryPageNum, PageNumber parentPageNum, SM_FileHandle *fh){
    RC ret = RC_OK;
    SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);
    RecordPage *page = (RecordPage *)malloc(PAGE_SIZE);
    memset(ph, '\0', PAGE_SIZE);
    memset(page, '\0', PAGE_SIZE);
    page->isDirectory = true;
    page->parentPageNum = parentPageNum;
    page->data = (char *)(page + 1);
    size_t sizeOfEntry = sizeof(DirectoryEntry);
    size_t availableSpace = PAGE_SIZE - sizeof(RecordPage);
    int numEntries = availableSpace / sizeOfEntry;
    DirectoryEntry *directoryEntries = (DirectoryEntry *)page->data;
    for (int i=0; i<numEntries; i++) {
        DirectoryEntry *currentEntry = &directoryEntries[i];
        currentEntry->pageNum = level*numEntries + directoryPageNum + i + 1;
        currentEntry->isFull = false;
    }

    memcpy(ph, page, PAGE_SIZE);
    ret = writeBlock(directoryPageNum, fh, ph);
    if(ret!=RC_OK){
        free(ph);
        free(page);
        return RC_OK;
    }
    
    free(ph);
    free(page);
    return ret;
}