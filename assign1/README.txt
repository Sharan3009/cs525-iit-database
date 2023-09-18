GROUP 62
=========================
Sharandeep Singh


#Ideation
=========================
1) In the beginning of each file, I am saving an extra page that will act as a page directory. Initially all the bits of this page will be '\0'. This page just act as an index to each page which tells if that index page is filled ('1') or empty/unused ('\0'). Directory related functions are in `page_directory.c`.
2) This directory is stored in memory whenever file is opened. This page gets modified by each operation and is stored back in the file when close operation is called.
3) At the core level, all reading and writing is handled by `readBlock` and `writeBlock` function. All other functions are re-using these functions to do specific task. This prevents the code and logic duplication in each of the functions.
4) Files are opened in binary mode ("rb+", "wb+"), as this helps reading and writing faster than normal text file.
5) Extra test cases are written and to test the robustness. Also, valgrind complains no memory leaks.


# Code structure
=========================

storage_mgr.c

initStorageManager()
Method initializes directory variable at global level available to all the functions.

createPageFile()
It checks if file is already there. Because recreating file will get rid of all the content.
Creates and Opens file in binary mode.
directory is added at the beginning.
Empty page is added using appendEmptyBlock method.
Finally the page closes.

openPageFile()
loads Directory that was stored in the file by createPageFile method.
set curPagePos to 0.
set totalPageNums to number of `1` in the loaded directory.

closePageFile()
writes back the directory in the file.
closes the file.

destroyPageFile()
deletes the file.

readBlock()
returns `RC_READ_NON_EXISTING_PAGE` if pageNum is out of range, or corresponding bit in page directory is '\0'
seeks to that location in the page using `fseek` and read PAGE_SIZE from there.

getBlockPos()
returns curPagePos value

readFirstBlock()
reads using readBlock(0)

readPreviousBlock()
reads using readBlock(curPagePos - 1)
if read succeeds then curPagePos position is updated to curPagePos-- in memory.

readCurrentBlock()
reads using readBlock(curPagePos)

readNextBlock()
reads using readBlock(curPagePos + 1)
if read succeeds then curPagePos position is updated to curPagePos++ in memory.

readLastBlock()
reads using readBlock(totalNumPages - 1)
if read succeeds then curPagePos position is updated to totalNumPages-1 in memory.


writeBlock()
returns `RC_WRITE FAILED` if pageNum is out of range, or ensureCapacity() capacity fails
seeks to that location in the page using `fseek` and writes PAGE_SIZE to there.
if write succeeds, then the directory variable is updated to `1` for that page.
totalNumPages is incremented

appendEmptyBlock()
empty block is added using writeBlock(totalNumPages)

ensureCapacity()
appendEmptyBlock() is called repeatedly for (numberOfPages - totalNumPages) times


page_directory.c

readPageDirectory()
stores the current file position using `ftell`
moves file pointer to the beginning where the page directory is
reads the page directory from the file.
seeks back the pointer where it was initially.

writePageDirectory()
stores the current file position using `ftell`
moves file pointer to the beginning where the page directory is
writes the page directory to the file.
seeks back the pointer where it was initially.