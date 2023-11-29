#include "../tables.h"
#include "../dt.h"
#include "../buffer_mgr.h"
#ifndef PAGE_DIRECTORY_H
    typedef struct PageDirectory{
        int capacity;
        bool *isFull;
    } PageDirectory;
#endif
extern void initPageDirectory(RM_TableData *rel);
extern PageDirectory *getPageDirectory(RM_TableData *rel);
extern void doublePageDirectory(RM_TableData *rel);
extern void destroyPageDirectory(RM_TableData *rel);
extern PageNumber getEmptyPage(RM_TableData *rel);
extern void updatePageInPageDirectory(RM_TableData *rel, PageNumber i, bool isFull);