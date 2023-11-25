#include "page_directory.h"

RC createDirectory(int currLevel, int totalLevels, PageNumber directoryPageNum, SM_FileHandle fh){
    RC ret = RC_OK;
    PageNumber parentPageNum = -1;
    SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);
    RecordPage *page = (RecordPage *)malloc(PAGE_SIZE);
    while(currLevel<totalLevels){
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
            currentEntry->pageNum = currLevel*numEntries + directoryPageNum + i + 1;
            currentEntry->isFull = false;
        }

        memcpy(ph, page, PAGE_SIZE);
        ret = writeBlock(directoryPageNum, &fh, ph);
        if(ret!=RC_OK){
            free(ph);
            free(page);
            return RC_OK;
        }

        parentPageNum = directoryPageNum;
        currLevel++;
    }
    free(ph);
    free(page);
    return ret;
}