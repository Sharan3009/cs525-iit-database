#include "dt.h"
#include "buffer_mgr.h"

typedef struct PageEntry{
    PageNumber pageNum;
    char* pageData;
    int pin;
    bool dirty;
} PageEntry;

typedef struct PageTable {
    PageEntry *table;
    int capacity;
    int size;
} PageTable;

extern void initPageTable(BM_BufferPool *const bm, int capacity);
extern int writePage(BM_BufferPool *const bm, BM_PageHandle *const page);
extern void readPage(BM_BufferPool *const bm, BM_PageHandle *const page);
extern int deletePage(BM_BufferPool *const bm, PageNumber pageNum);
bool isTableFull(BM_BufferPool *const bm);
int hashPage(PageTable * pageTable, PageNumber pageNum);