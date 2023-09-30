#include "dt.h"
#include "buffer_mgr.h"

typedef struct PageEntry{
    PageNumber pageNum;
    char* pageData;
    int occupied; // Flag to indicate whether the slot is occupied
} PageEntry;

typedef struct PageTable {
    PageEntry *table;
    int size;
    int actualSize;
} PageTable;

extern void initPageTable(PageTable *pageTable, int size);
int hashPage(int pageNum, int size);
extern int addPage(PageTable *pageTable, PageNumber pageNum, char* pageData);
extern char* getPage(PageTable *pageTable, PageNumber pageNum);
extern int removePage(PageTable *pageTable, PageNumber pageNum);
extern void clearTable(PageTable *pageTable);
extern bool isTableFull(PageTable *pageTable);