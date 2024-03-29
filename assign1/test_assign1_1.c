#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testCreateOpenClose(void);
static void testSinglePageContent(void);
static void testTenPageContent(void);
static void testMultipleCreationWithoutDeletion(void);
static void testPagePosAndTotal(void);
static void testEnsureCapacity(void);

/* main function running all tests */
int
main (void)
{
  testName = "";
  
  initStorageManager();

  testCreateOpenClose();
  testSinglePageContent();
  testTenPageContent();
  testMultipleCreationWithoutDeletion();
  testPagePosAndTotal();
  testEnsureCapacity();

  return 0;
}


/* check a return code. If it is not RC_OK then output a message, error description, and exit */
/* Try to create, open, and close a page file */
void
testCreateOpenClose(void)
{
  SM_FileHandle fh;

  testName = "test create open and close methods";

  TEST_CHECK(createPageFile (TESTPF));
  
  TEST_CHECK(openPageFile (TESTPF, &fh));
  ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
  ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
  ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");

  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));

  // after destruction trying to open the file should cause an error
  ASSERT_TRUE((openPageFile(TESTPF, &fh) != RC_OK), "opening non-existing file should return an error.");

  TEST_DONE();
}

/* Try to create, open, and close a page file */
void
testSinglePageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test single page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");
  
  // read first page into handle
  TEST_CHECK(readFirstBlock (&fh, ph));
  // the page should be empty (zero bytes)
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
  printf("first block was empty\n");
    
  // change ph to be a string and write that one to disk
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeBlock (0, &fh, ph));
  printf("writing first block\n");

  // read back the page containing the string and check that it is correct
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("reading first block\n");

  // destroy new page file
  TEST_CHECK(destroyPageFile (TESTPF));  
  
  TEST_DONE();
}

void
testTenPageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test ten page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");
    
  // change ph to be a string and write that one to disk
  for(int p=0;p<10;p++){
    for (i=0; i < PAGE_SIZE; i++)
      ph[i] = (i % 10) + p + '0';
    TEST_CHECK(writeBlock (p, &fh, ph));
    printf("writing block %d\n",p);
  }

  // read back the page containing the string and check that it is correct
   for(int p=0;p<10;p++){
    if(p==0){
      TEST_CHECK(readCurrentBlock(&fh, ph));
    } else {
      TEST_CHECK(readNextBlock(&fh, ph));
    }
    int readPassed = 1;
    for(i=0;i<PAGE_SIZE;i++){
      if(ph[i]!=(i%10) + p + '0'){
        readPassed = 0;
        break;
      }
    }
    ASSERT_TRUE(readPassed==1,"All characters are same");
    printf("Block #%d content is as expected\n",p);
    
  }

  ASSERT_TRUE(fh.curPagePos==9,"Current page position is 9");
  ASSERT_TRUE(fh.totalNumPages==10,"Total number of pages is 10");

  TEST_CHECK(closePageFile (&fh));
  
  
  TEST_DONE();
}

void
testMultipleCreationWithoutDeletion(void) {
  testName = "Re running the test case by just closing and again creating then opening";
  testTenPageContent();
  // destroy new page file
  TEST_CHECK(destroyPageFile (TESTPF));  
}

void
testPagePosAndTotal(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test Page Positions and Total";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");
    
  // change ph to be a string and write that one to disk
  for(int p=0;p<10;p++){
    for (i=0; i < PAGE_SIZE; i++)
      ph[i] = (i % 10) + p + '0';
    TEST_CHECK(writeBlock (p, &fh, ph));
    printf("writing block %d\n",p);
  }

  ASSERT_TRUE(getBlockPos(&fh)==0, "Current page position is 0");
  ASSERT_TRUE(fh.totalNumPages==10, "Total number of pages is 10");

  TEST_CHECK(readBlock(3, &fh, ph));
  printf("Reading 4th block\n");
  ASSERT_TRUE(getBlockPos(&fh)==0, "Current page position is 0");


  ASSERT_ERROR(readPreviousBlock(&fh, ph),"Cant read before 0th block");
  ASSERT_TRUE(getBlockPos(&fh)==0, "Current page position is 0");

  TEST_CHECK(readNextBlock(&fh, ph));
  printf("Reading 2nd block\n");

  ASSERT_TRUE(getBlockPos(&fh)==1, "Current page position is 2");
  ASSERT_TRUE(fh.totalNumPages==10,"Total number of pages is 10");

  TEST_CHECK(closePageFile (&fh));
  // destroy new page file
  TEST_CHECK(destroyPageFile (TESTPF));  
  
  TEST_DONE();
}


void
testEnsureCapacity(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test Ensure capacity method";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");

  ASSERT_TRUE(getBlockPos(&fh)==0, "Current page position is 0");
  ASSERT_TRUE(fh.totalNumPages==1, "Total number of pages is 1");

  // change ph to be a string and write that one to disk
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '9';
  TEST_CHECK(writeBlock (9, &fh, ph));
  printf("writing 10th block\n");

  ASSERT_TRUE(getBlockPos(&fh)==0, "Current page position is 0");
  ASSERT_TRUE(fh.totalNumPages==10, "Total number of pages is 10");

  for(i=1;i<10;i++){
    TEST_CHECK(readNextBlock(&fh, ph));
    printf("Reading %dth block\n", i);
  }

  TEST_CHECK(readFirstBlock(&fh, ph));
  printf("Read first block\n");

  ASSERT_TRUE(getBlockPos(&fh)==0, "Current page position is 0");

  TEST_CHECK(readLastBlock(&fh, ph));
  printf("Read last block again\n");

  ASSERT_TRUE(getBlockPos(&fh)==9, "Current page position is 9");

  ASSERT_ERROR(readNextBlock(&fh, ph),"Cant read after page position 9");


  TEST_CHECK(closePageFile (&fh));
  // destroy new page file
  TEST_CHECK(destroyPageFile (TESTPF));  
  
  TEST_DONE();
}