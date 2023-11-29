#include "../tables.h"
#include "../dt.h"
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