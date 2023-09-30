#include "dt.h"

typedef struct PageEntry{
    int pageNum;
    int pageOffset;
    int occupied; // Flag to indicate whether the slot is occupied
} PageEntry;

typedef struct PageTable {
    PageEntry *table;
    int size;
    int actualSize;
} PageTable;

extern void initPageTable(PageTable *pageTable, int size);
int hashPage(int pageNum, int size);
extern int addPage(PageTable *pageTable, int pageNum, int pageOffset);
extern int getPage(PageTable *pageTable, int pageNum);
extern int removePage(PageTable *pageTable, int pageNum);
extern void clearTable(PageTable *pageTable);
extern bool isTableFull(PageTable *pageTable);