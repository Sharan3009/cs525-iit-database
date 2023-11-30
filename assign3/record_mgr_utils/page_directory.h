#include "../tables.h"
#include "../dt.h"
#include "../buffer_mgr.h"
#ifndef PAGE_DIRECTORY_H
#define PAGE_DIRECTORY_H
    typedef struct PageDirectory{
        int capacity;
        bool *isFull;
    } PageDirectory;

    extern void initPageDirectory(RM_TableData *rel);
    extern PageDirectory *getPageDirectory(RM_TableData *rel);
    extern void doublePageDirectory(RM_TableData *rel);
    extern void closePageDirectory(RM_TableData *rel);
    extern void deletePageDirectory(char *name);
    extern PageNumber getEmptyPage(RM_TableData *rel);
    extern void updatePageInPageDirectory(RM_TableData *rel, PageNumber i, bool isFull);
    static char* getDirectoryFileName(char *name);
#endif