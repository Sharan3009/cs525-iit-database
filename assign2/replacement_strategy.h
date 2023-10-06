#include "dt.h"
#include "buffer_mgr.h"
#include "page_table.h"

extern void initReplacementStrategy(BM_BufferPool *const bm);
extern PageNumber evictPage(BM_BufferPool *const bm);
static PageNumber evictFifo(BM_BufferPool *const bm);
static PageNumber evictLru(BM_BufferPool *const bm);
extern void admitPage(BM_BufferPool *const bm, PageEntry* entry);
static void admitFifo(PageEntry* entry);
static void admitLru(PageEntry* entry);
extern void reorderPage(BM_BufferPool *const bm, PageEntry* entry);
static void reorderLru(BM_BufferPool *const bm, PageEntry* entry);
