#include "dt.h"
#include "buffer_mgr.h"

extern void initReplacementStrategy(BM_BufferPool *const bm);
extern int evictPage(BM_BufferPool *const bm);
static int evictFifo(BM_BufferPool *const bm);
extern void admitPage(BM_BufferPool *const bm, PageNumber pageNum);
static void admitFifo(PageNumber pageNum);
