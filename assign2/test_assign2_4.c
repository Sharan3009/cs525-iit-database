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

static void testLFU (void);
static void testLFU_complex (void);


// main method
int
main (void)
{
    initStorageManager();
    testName = "";
    
    testLFU();
    testLFU_complex();
    return 0;
}


void
testLFU (void)
{
    // expected results
    const char *poolContents[] = {
        // read first five pages and directly unpin them
        "[0 0],[-1 0],[-1 0],[-1 0],[-1 0]" ,
        "[0 0],[1 0],[-1 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        // use some of the page to create a fixed LFU order without changing pool content
        "[0 0],[1 0],[2 0],[3 0],[4 0]", //freq of 3 is 2
        "[0 0],[1 0],[2 0],[3 0],[4 0]", // freq of 3 is 3
        "[0 0],[1 0],[2 0],[3 0],[4 0]", // freq of 0 is 2
        "[0 0],[1 0],[2 0],[3 0],[4 0]", // freq of 2 is 2
        "[0 0],[1 0],[2 0],[3 0],[4 0]", // freq of 1 is 2
        "[0 0],[1 0],[2 0],[3 0],[5 0]", // freq of 5 is 1
        // check that pages get evicted in LFU order
        "[0 0],[1 0],[2 0],[3 0],[5 0]", // freq of 5 is 2
        "[6 0],[1 0],[2 0],[3 0],[5 0]", // 0 will be replaced with 6 of freq 1, because 0 is LRU with freq 2.
        "[7 0],[1 0],[2 0],[3 0],[5 0]", // 6 is replaced of freq 1
        "[8 0],[1 0],[2 0],[3 0],[5 0]", // 7 is replaced of freq 1
        "[9 0],[1 0],[2 0],[3 0],[5 0]", // 8 is replaced of freq 1
    };
    const int orderRequests[] = {3,3,0,2,1,5};
    const int numLFUOrderChange = 6;
    
    int i;
    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing LFU page replacement";
    
    CHECK(createPageFile("testbuffer.bin"));
  
    CHECK(initBufferPool(bm, "testbuffer.bin", 5, RS_LFU, NULL));
    
    // reading first five pages linearly with direct unpin and no modifications
    for(i = 0; i < 5; i++)
    {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    }
    
    // read pages to change LFU order
    for(i = 0; i < numLFUOrderChange; i++)
    {
        pinPage(bm, h, orderRequests[i]);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    }
    
    // replace pages and check that it happens in LFU order
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


void
testLFU_complex (void)
{
    // expected results
    const char *poolContents[] = {
        "[-1 0],[7 0],[-1 0]", // read 7
        "[0 0],[7 0],[-1 0]", // read 0
        "[0 0],[7 0],[1 0]", // read 1
        "[0 0],[2 0],[1 0]", // read 2
        "[0 0],[2 0],[1 0]", // read 0
        "[0 0],[2 0],[3 0]", // read 3
        "[0 0],[2 0],[3 0]", // read 0
        "[0 0],[4 0],[3 0]", // read 4
        "[0 0],[4 0],[2 0]", // read 2
        "[0 0],[3 0],[2 0]", // read 3
        "[0 0],[3 0],[2 0]", // read 0
        "[0 0],[3 0],[2 0]", // read 3
        "[0 0],[3 0],[2 0]", // read 2
        "[0 0],[1 0],[2 0]", // read 1
        "[0 0],[1 0],[2 0]", // read 2
    };

    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing LFU complex page replacement";
    
    CHECK(createPageFile("testbuffer.bin"));
  
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_LFU, NULL));

    pinPage(bm, h, 7);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 7");

    pinPage(bm, h, 0);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 0");

    pinPage(bm, h, 1);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 1");

    pinPage(bm, h, 2);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 2");

    pinPage(bm, h, 0);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 0");

    pinPage(bm, h, 3);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 3");

    pinPage(bm, h, 0);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 0");

    pinPage(bm, h, 4);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 4");

    pinPage(bm, h, 2);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 2");

    pinPage(bm, h, 3);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 3");

    pinPage(bm, h, 0);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 0");

    pinPage(bm, h, 3);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 3");

    pinPage(bm, h, 2);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 2");

    pinPage(bm, h, 1);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 1");

    pinPage(bm, h, 2);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading page 2");
    
    
    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));
    
    free(bm);
    free(h);
    TEST_DONE();
}