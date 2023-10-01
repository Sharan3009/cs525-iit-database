#include "dt.h"
#include "buffer_mgr.h"

typedef struct PageEntry{
    PageNumber pageNum;
    char* pageData;
} PageEntry;

typedef struct PageTable {
    PageEntry *table;
    int capacity;
    int size;
} PageTable;

extern void initPageTable(BM_BufferPool *const bm, int capacity);
extern int addPage(BM_BufferPool *const bm, BM_PageHandle *const page);
extern void getPage(BM_BufferPool *const bm, BM_PageHandle *const page);
extern int removePage(BM_BufferPool *const bm, PageNumber pageNum);
extern bool isTableFull(BM_BufferPool *const bm);
int hashPage(PageTable * pageTable, PageNumber pageNum);