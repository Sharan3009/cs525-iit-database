#include "dt.h"
#include "buffer_mgr.h"

#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H
    typedef struct PageEntry{
        PageNumber pageNum;
        char* pageData;
        int fixCount;
        bool dirty;
    } PageEntry;

    typedef struct PageTable {
        PageEntry *table;
        int capacity;
        int size;
        int readIOCount;
        int writeIOCount;
    } PageTable;
#endif

extern void initPageTable(BM_BufferPool *const bm, int capacity);
extern int putPage(BM_BufferPool *const bm, BM_PageHandle *const page);
extern int getPage(BM_BufferPool *const bm, BM_PageHandle *const page);
extern int deletePage(BM_BufferPool *const bm, PageNumber pageNum);
extern int hasPage(BM_BufferPool *const bm, PageNumber pageNum);
extern void incrementPageFixCount(BM_BufferPool *const bm, int pageIndex);
extern void decrementPageFixCount(BM_BufferPool *const bm, int pageIndex);
extern void markPageDirty(BM_BufferPool *const bm, int pageIndex);
extern void unmarkPageDirty(BM_BufferPool *const bm, int pageIndex);
extern void incrementReadCount(BM_BufferPool *const bm);
extern void incrementWriteCount(BM_BufferPool *const bm);
extern PageTable* getPageTable(BM_BufferPool *const bm);
static void changePageFixCount(BM_BufferPool *const bm, int pageIndex, int val);
static void changePageDirty(BM_BufferPool *const bm, int pageIndex, bool boolean);
static bool isTableFull(BM_BufferPool *const bm);
static int generatePageTableHash(PageTable * pageTable, PageNumber pageNum);