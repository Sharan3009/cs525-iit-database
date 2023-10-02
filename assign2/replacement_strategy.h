#include "dt.h"
#include "buffer_mgr.h"

extern void initReplacementStrategy(BM_BufferPool *const bm);
extern int evictPage(BM_BufferPool *const bm);
static int fifoStrategy(BM_BufferPool *const bm);
