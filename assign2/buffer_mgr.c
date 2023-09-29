#include <stdio.h>
#include <stdlib.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "dt.h"

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		const int numPages, ReplacementStrategy strategy, void *stratData){
    return RC_OK;
}
RC shutdownBufferPool(BM_BufferPool *const bm){
    return RC_OK;
}
RC forceFlushPool(BM_BufferPool *const bm){
    return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
    return RC_OK;
}
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    return RC_OK;
}
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    return RC_OK;
}
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
		const PageNumber pageNum){
    return RC_OK;
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){
    return RC_OK;
}
bool *getDirtyFlags (BM_BufferPool *const bm){
    bool *dirtyFlag = (bool *)malloc(sizeof(bool));
    return dirtyFlag;
}
int *getFixCounts (BM_BufferPool *const bm){
    int *fixCount = (int *)malloc(sizeof(int));
    return fixCount;
}
int getNumReadIO (BM_BufferPool *const bm){
    return 0;
}
int getNumWriteIO (BM_BufferPool *const bm){
    return 0;
}
