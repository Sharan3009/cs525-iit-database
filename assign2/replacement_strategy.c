#include <stdio.h>
#include <stdlib.h>

#include "replacement_strategy.h"
#include "page_table.h"

int* fifo;

void initReplacementStrategy(BM_BufferPool *const bm){

    switch (bm->strategy){
        case RS_FIFO:
            if(fifo==NULL)
                fifo = (int*)malloc(bm->numPages*sizeof(int));
            else
                fifo = (int*)malloc(fifo, bm->numPages*sizeof(int));
            break;
        default:
            break;
    }
}

int evictPage(BM_BufferPool *const bm){
    switch (bm->strategy){
        case RS_FIFO:
            return fifo_strategy(bm);
    }
    return -1;
}

static int fifoStrategy(BM_BufferPool *const bm){
    return 1;
}