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

static void testClock (void);

// main method
int
main (void)
{
    initStorageManager();
    testName = "";
    
    testClock();
    return 0;
}


void
testClock (void)
{
    // expected results
    const char *poolContents[] = {
        // read first five pages and directly unpin them
        "[0 0],[-1 0],[-1 0]" ,
        "[0 0],[1 0],[-1 0]",
        "[0 0],[1 0],[2 0]",
        // pin page 1
        "[0 0],[1 1],[2 0]",
        // add page 3
        "[3 0],[1 1],[2 0]",
        // add page 5
        "[3 0],[1 1],[5 0]"
    };
    
    int i;
    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing Clock page replacement";
    
    CHECK(createPageFile("testbuffer.bin"));
  
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_CLOCK, NULL));
    
    // reading first five pages linearly with direct unpin and no modifications
    for(i = 0; i < 3; i++)
    {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    }

    pinPage(bm, h, 1);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");

    //add page 3
    pinPage(bm, h, 3);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    
    //add page 5
    pinPage(bm, h, 5);
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    
    // unpinning page 1 to shutdown
    h->pageNum = 1;
    unpinPage(bm, h);
    
    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));
    
    free(bm);
    free(h);
    TEST_DONE();
}