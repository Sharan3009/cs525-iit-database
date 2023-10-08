#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// var to store the current test's name
char *testName;

// check whether two the content of a buffer pool is the same as an expected content
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)                    \
do {                                    \
char *real;                                \
char *_exp = (char *) (expected);                                   \
real = sprintPoolContent(bm);                    \
if (strcmp((_exp),real) != 0)                    \
{                                    \
printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
free(real);                            \
exit(1);                            \
}                                    \
printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
free(real);                                \
} while(0)

// test and helper methods

static void testLRU_1 (void);

static void testLRU_2 (void);

static void testLRU_2_withContention (void);

// main method
int
main (void)
{
    initStorageManager();
    testName = "";
    
    testLRU_1();
    testLRU_2();
    testLRU_2_withContention();
    return 0;
}


// test the LRU_K page replacement strategy with k=1
void
testLRU_1 (void)
{
    int k=1;
    // expected results
    const char *poolContents[] = {
        // read first five pages and directly unpin them
        "[0 0],[-1 0],[-1 0],[-1 0],[-1 0]" ,
        "[0 0],[1 0],[-1 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        // use some of the page to create a fixed LRU_K order without changing pool content
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        // check that pages get evicted in LRU_K order
        "[0 0],[1 0],[2 0],[5 0],[4 0]",
        "[0 0],[1 0],[2 0],[5 0],[6 0]",
        "[7 0],[1 0],[2 0],[5 0],[6 0]",
        "[7 0],[1 0],[8 0],[5 0],[6 0]",
        "[7 0],[9 0],[8 0],[5 0],[6 0]"
    };
    const int orderRequests[] = {3,4,0,2,1};
    const int numLRU_KOrderChange = 5;
    
    int i;
    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing LRU_1 page replacement";
    
    CHECK(createPageFile("testbuffer.bin"));
  
    CHECK(initBufferPool(bm, "testbuffer.bin", 5, RS_LRU_K, &k));
    
    // reading first five pages linearly with direct unpin and no modifications
    for(i = 0; i < 5; i++)
    {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    }
    
    // read pages to change LRU_K order
    for(i = 0; i < numLRU_KOrderChange; i++)
    {
        pinPage(bm, h, orderRequests[i]);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    }
    
    // replace pages and check that it happens in LRU_K order
    for(i = 0; i < 5; i++)
    {
        pinPage(bm, h, 5 + i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    }
    
    // check number of write IOs
    ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
    ASSERT_EQUALS_INT(10, getNumReadIO(bm), "check number of read I/Os");
    
    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));
    
    free(bm);
    free(h);
    TEST_DONE();
}


// test the LRU_K page replacement strategy with k=2
void
testLRU_2 (void)
{
    int k=2;
    // expected results
    const char *poolContents[] = {
        // read first 2 pages and directly unpin them
        "[0 0],[-1 0],[-1 0]" ,
        "[0 0],[1 0],[-1 0]",
        // reading page 1 again
        "[0 0],[1 0],[-1 0]",
        // reading page 3,4,5
        "[0 0],[1 0],[2 0]",
        "[0 0],[3 0],[2 0]",
        "[0 0],[3 0],[4 0]",
        // reading page 1 again
        "[0 0],[3 0],[4 0]",
        // reading page 6
        "[0 0],[5 0],[4 0]",
        // reading page 2
        "[0 0],[5 0],[1 0]",
        
    };
    
    int i;
    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing LRU_2 page replacement";
    
    CHECK(createPageFile("testbuffer.bin"));
  
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_LRU_K, &k));
    
    // reading first 2 pages
    for(i = 0; i < 2; i++)
    {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    }

    // reading page 1
    pinPage(bm, h, 0);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    
    // reading 3,4,5 pages
    for(i = 2; i < 5; i++)
    {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    }
    // reading page 1
    pinPage(bm, h, 0);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");

    //reading page 6
    pinPage(bm, h, 5);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");

    //reading page 2
    pinPage(bm, h, 1);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    
    // check number of write IOs
    ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
    ASSERT_EQUALS_INT(7, getNumReadIO(bm), "check number of read I/Os");
    
    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));
    
    free(bm);
    free(h);
    TEST_DONE();
}

// test the LRU_K page replacement strategy with k=2
void
testLRU_2_withContention (void)
{
    int k=2;
    // expected results
    const char *poolContents[] = {
        "[0 0],[-1 0],[-1 0]" , // read 0
        "[0 0],[1 0],[-1 0]", // read 1
        "[0 0],[1 0],[-1 0]", // read 0
        "[0 0],[1 0],[2 0]", // read 2
        "[0 0],[1 0],[2 0]", // read 2
        "[0 0],[1 0],[2 0]", // read 1
        "[0 0],[3 0],[2 0]", // read 3 -> evict page 1 because 1 has high distance from its k-1th occurence
        "[0 0],[4 0],[2 0]" // read 4 -> evict page 3 because page 3 never had k-1 occurences
    };
    
    int i;
    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing LRU_2 page contention only";
    
    CHECK(createPageFile("testbuffer.bin"));
  
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_LRU_K, &k));
    
    // reading page 0
    pinPage(bm, h, 0);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");


    // reading page 1
    pinPage(bm, h, 1);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");

    // reading page 0
    pinPage(bm, h, 0);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");

    // reading page 2
    pinPage(bm, h, 2);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");

    // reading page 2
    pinPage(bm, h, 2);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");

    // reading page 1
    pinPage(bm, h, 1);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");

    // reading page 3
    pinPage(bm, h, 3);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");

    // reading page 4
    pinPage(bm, h, 4);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    
    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));
    
    free(bm);
    free(h);
    TEST_DONE();
}