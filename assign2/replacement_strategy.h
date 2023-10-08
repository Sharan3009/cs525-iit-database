#include "dt.h"
#include "buffer_mgr.h"
#include "page_table.h"

extern void initReplacementStrategy(BM_BufferPool *const bm, void* stratData);
extern PageNumber evictPage(BM_BufferPool *const bm);
static PageNumber evictFromHead(BM_BufferPool *const bm);
extern void admitPage(BM_BufferPool *const bm, PageEntry* entry);
static void admitLruK(PageEntry* entry);
extern void reorderPage(BM_BufferPool *const bm, PageEntry* entry);
static void reorderLruK(BM_BufferPool *const bm, PageEntry* entry);
extern void clearStrategyData(BM_BufferPool *const bm);
