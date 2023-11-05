#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS 100

// var to store the current test's name
char *testName;

// test and helper methods

static void testMultithreadedReadIO (void);
static void testMultithreadedLatestWrite(void);

// main method
int
main (void)
{
    initStorageManager();
    testName = "";
    
    testMultithreadedReadIO();
    testMultithreadedLatestWrite();

    return 0;
}

// auxilary method used by read thread
static void *pinPageThread(void *data) 
{ 
    BM_BufferPool *bm = (BM_BufferPool *)data;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();
    CHECK(pinPage(bm, page, 0));
    CHECK(unpinPage(bm, page));
    free(page);
} 

void
testMultithreadedReadIO (void)
{
    testName = "Testing read page by multiple threads";
    BM_BufferPool *bm = MAKE_POOL();
    CHECK(createPageFile("testbuffer.bin"));
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));


    pthread_t threadIds[NUM_THREADS];

    //multiple threads reading same page
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threadIds[i], NULL, pinPageThread, (void *)bm);
    }

    for(int i=0; i<NUM_THREADS; i++){
        pthread_join(threadIds[i], NULL);
    }

    // even though multiple threads, read from the disk should be once only
    ASSERT_EQUALS_INT(1,getNumReadIO(bm),"check");
    
    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));
    
    free(bm);
    TEST_DONE();
}

// auxillary variable and method used by write thread
int val = 0;
static void *forceWrite(void *data) 
{ 
    BM_BufferPool *bm = (BM_BufferPool *)data;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();
    CHECK(pinPage(bm, page, 0));
    sprintf(page->data, "Content-%i", ++val);
    CHECK(forcePage(bm, page));
    CHECK(unpinPage(bm, page));
    free(page);
} 

void
testMultithreadedLatestWrite (void)
{
    testName = "Testing write page by multiple threads";
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();

    CHECK(createPageFile("testbuffer.bin"));
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

    char *expected = malloc(sizeof(char) * 512);
    pthread_t threadIds[NUM_THREADS];

    // multiple threads writing to same page
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threadIds[i], NULL, forceWrite, (void *)bm);
    }

    for(int i=0; i<NUM_THREADS; i++){
        pthread_join(threadIds[i], NULL);
    }

    ASSERT_EQUALS_INT(1,getNumReadIO(bm),"check number of read count");
    CHECK(shutdownBufferPool(bm));

    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

    // everything the last thread write will be updated
    CHECK(pinPage(bm, h, 0));
    sprintf(expected, "Content-%i", val);
    ASSERT_EQUALS_STRING(expected, h->data, "reading back dummy page content");
    CHECK(unpinPage(bm, h));

    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));
    
    free(bm);
    free(expected);
    free(h);
    TEST_DONE();
}