#include "page_directory.h"
#include <math.h>

RC createDirectories(int currLevel, int totalLevels, PageNumber directoryPageNum, SM_FileHandle *fh){
    RC ret = RC_OK;
    PageNumber parentPageNum = -1;
    PageNumber firstEntryPageNum = directoryPageNum + 1;
    size_t sizeOfEntry = sizeof(DirectoryEntry);
    size_t availableSpace = PAGE_SIZE - sizeof(RecordPage);
    int numEntries = availableSpace / sizeOfEntry;
    for(int level=currLevel; level<totalLevels;level++){
        for (int i=0; i<pow(numEntries, level); i++) {
            ret = createDirectory(directoryPageNum++, -1, firstEntryPageNum, fh);
            if(ret!=RC_OK){
                return ret;
            }
            firstEntryPageNum += numEntries;
        }
    }
    return ret;
}

RC createDirectory(PageNumber directoryPageNum, PageNumber parentPageNum, PageNumber firstEntryPageNum, SM_FileHandle *fh){
    RC ret = RC_OK;
    SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);
    RecordPage *page = (RecordPage *)malloc(sizeof(RecordPage));
    memset(ph, '\0', PAGE_SIZE);
    memset(page, '\0', sizeof(RecordPage));
    page->isDirectory = true;
    page->parentPageNum = parentPageNum;
    size_t sizeOfEntry = sizeof(DirectoryEntry);
    size_t availableSpace = PAGE_SIZE - sizeof(RecordPage);
    page->data = (char *)malloc(availableSpace);
    memset(page->data, '\0', availableSpace);
    int numEntries = availableSpace / sizeOfEntry;
    DirectoryEntry *directoryEntries = (DirectoryEntry *)page->data;
    for (int i=0; i<numEntries; i++) {
        DirectoryEntry *currentEntry = &directoryEntries[i];
        currentEntry->pageNum = firstEntryPageNum++;
        currentEntry->isFull = false;
        // if((i==0 || i==numEntries-1) && (directoryPageNum==1 || directoryPageNum==2 || directoryPageNum==3)){
        //     printf("creating directory=%d and entry=%d\n", directoryPageNum, currentEntry->pageNum);
        // }
    }
    memcpy(ph, page, sizeof(RecordPage));
    memcpy(ph + sizeof(RecordPage), page->data, availableSpace);
    ret = writeBlock(directoryPageNum, fh, ph);
    if(ret!=RC_OK){
        free(ph);
        free(page->data);
        free(page);
        return RC_OK;
    }

    free(ph);
    free(page->data);
    free(page);
    return ret;
}