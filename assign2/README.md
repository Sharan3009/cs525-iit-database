# GROUP 62
Sharandeep Singh


# Ideation
- Buffer Pool has 3 main components
    - `buffer_mgr.c` - The actual buffer manager
    - `page_table.c` - To speed up the page lookups in the buffer pool.
    - `replacement_strategy.c` - This handles how each page should be evicted or admitted in the buffer pool based on the input strategy.
- `FIFO`, `LRU`, `LRU-k` and `LFU` are implemented using linkedlist. Whereas `Clock` is implemented using a cyclic `array`.
- There is no separate method for `LRU`. `LRU` is executing with the help of `LRU-k` where k=1.
- Extra test cases are written and to test the robustness. Also, valgrind complains no memory leaks.
    - `test_assign2_3.c` to test `LRU-k`
    - `test_assign2_4.c` to test `LFU`
    - `test_assign2_5.c` to test `CLOCK`
    - `test_assign2_6.c` to test `thread safe`

# How to execute
- Navigate to the folder `assign2` and open the terminal here.
- Run `make clean` to clear test files file if any.    
- To execute test cases you can do any of the following ways
    - Run `make test` that will execute `test_assign2_1` and `test_assign2_2`
    - Or Run `make test-all` to execute all the test files including additoinal ones.
    - Or Run `make` and execute each test case individually like `./test_assign2_1`
- Make sure all the database files are deleted before running the test cases
- All the current and additional test cases pass and you will see no errors in it.

# Code structure
- `page_table.h` has the struct of `PageTable` and `PageEntry` as described below
    - `PageTable` consists of 
        - `table` that stores the `HashTable`.
        - `capacity` is the size of buffer pool. 
        - `size` indicates to see how many pages are filled up. This is also to check if `PageTable` is full or not.
        - `readIOCount` to store the read IO statistics.
        - `writeIOCount` to store the write IO statistics.
    - `PageEntry` is a datastructure representing each page in the page table. It consists of
        - `pageNum` which is page number of the page.
        - `pageData` which is a pointer to the data location in buffer pool.
        - `fixCount` is the number of threads currently accessing the page.
        - `dirty` indicates if the page has data to be written on the disk or not.

- `page_table.c` - It is just a immitation of `HashMap` in java but specific to the page table. It has the below methods and their working
    - `initPageTable` - initializes page table in `mgmtData` of `bm`
    - `getPage` - retrieves the page if exists otherwise it updates the `pageNum` to `-1` in the input page.
    - `hasPage` - It checks whether the page table has a given `PageNumber`. If not then it returns `-1`
    - `putPage` - It adds the new page if not exists. if already exists then it updates the given page. If page table is full then it returns `-1`
    - `deletePage` - It deletes the page in the page table if present.
    - `incrementPageFixCount` - to increment `fixCount` of any given page by 1.
    - `decrementPageFixCount` - to decrement `fixCount` of any given page by 1.
    - `markPageDirty` - to set `dirty` flag of any given page to `true`.
    - `unmarkPageDirty` - to set `dirty` flag of any given page to `false`.
    - `generatePageTableHash` - This is the hash function to access the page in constant amount of time. **Please note that the hash function is a modulus function. So each page will be stored at arbitrary location in the page table as instructed by the hash method**

- `linkedlist.h` is a regular linkedlist data structure with some additional information to get the assignment done.
    - `Node` - it is a node in the linkedlist. it consists of
        - `PageEntry` pointer to each page in the page table.
        - `next` is the next `PageEntry` to any given node.
        - `bp` is the backward k distance used by `LRU-k`. It is initialized as `infinity` that indicates that infinite backward distance in `LRU-k`.
        - `time` it is the time of last occurence starting from k-1. Before k-1, it stays as -1. Value of `bp` is dependent on this `time` property.
        - `occurences` tracks the number of times the given page number has been occured since it has been in the page table. This is to keep track of `k` in `LRU-k`. It starts with 1 value.
        - `frequency` is the number of times each page is occuring to work out `LFU` algorithm.

    - `Linkedlist` is a normal linkedlist with a `head` and `tail`. It involved all those methods that is expected in a linkedlist.

- `replacement_strategy.h` has extra data structur for the `clock` algorithm. other than from `#include "linkedlist.h"` 
    - `ClockEntry` is a datastructure representing each page in the page table along with a `referenceBit`. It consists of
        - `entry` which is a `PageEntry` pointer in the page table.
        - `referenceBit` which is a `short` datatype number used by the clock algorithm.
    - `Clock` consists of 
        - `arr` that stores the array of `ClockEntry`
        - `hand` represents the clock hand.
        - `capacity` is the size of buffer pool. 
        - `size` indicates to see how many pages are filled up.

- `replacement_strategy.c` - It is the main file which decides which page to **evict** and in which index to **admit** it to make the replacement strategy effective. It consists of 5 main public methods
    - `initReplacementStrategy` - It initializes the replacement strategy and all the data structures which are needed to get going. It uses `linkedlist` for `FIFO`, `LRU`, `LRU-k` and `LFU`. It uses an `array` for `Clock`. Also it sets `k=1` if `stratData` is `NULL`, otherwise `k` will be whatever is stored in `stratData`.
    - `evictPage` - It is a public method which internally calls various evict methods based on the input strategy. This method does not check whether the page exists or not. That condition are being handled inside the `buffer_mgr.c` and then this method is called.
    - `admitPage` - It is the opposite of evict. We need to apprpriately insert the page pointer so that the eviction in the `evictPage` happens according to the replacement strategy. Again this method does not check whether the page exists or not. That condition are being handled inside the `buffer_mgr.c` and then this method is called. For `LRU` there is no specific eviction method as `LRU` is same as `LRU-k` with `k=1`.
    - `reorderPage` happens when there is HIT in the page table. It reorders the priority of the pages based on the replacement strategy. Please not `FIFO` does not need re ordering as it solely depends on when did it enter and when will it
    - `clearStrategyData` is called on shutting down the buffer pool.

- `buffer_mgr.c`
    - `initBufferPool` - This checks if the page exists and initializes page table from `page_table.c` and replacement strategy in `replacement_strategy.c`. Also it initializes mutex locks for `pinPage` and `markDirty` methods.
    - `shutDownBufferPool` - It first writes down all the dirty pages using `forceFlushPool` method. If `forceFlushPool` fails or if it has any pages with `fixCount>0` then it returns errors. If succeeds then it clears replacement strategy `clearStrategyData` and clears up all the page table stored in the `bm`. Additionally, it destroys the mutex locks initialized for `pinPage` and `markDirty`.
    - `forceFlushPool` - It loops over the page table entries, and which every has `dirty=true`, it writes down using `forcePage` method.
    - `markDirty` - It is a thread safe method. This allows only one thread at a time to write content in the buffer pool and marking it as a dirty. It returns error if it is trying to marking dirty the page which does not exist in the page table.
    - `unpinPage` - It reduces the fix count value by 1. It returns error if it is trying to unpin the page which does not exist in the page table.
    - `forcePage` - This is the core method which writes down data in the page file. We are only updated the write IO stats in this method. Any other method trying to write down content on the disk should use this method only. Also it marks the page `dirty=false` on successful disk writing.
    - `pinPage` - This page handles all the MISS and HIT and reading from the disk in case of MISS. This method is thread safe if there is a MISS because we dont want to unecessarily read the page from the disk in case of multiple threads. In other case (not MISS), the method does not restrict the threads as reading is idempotent in nature. It uses the replacement strategies and make use of `evictPage`, `admitPage` and `reorderPage`, whereever necesary. Please note all these 3 methods are internally deciding what to do based on the input replacement strategy. So page replacement is an independent module with no impact on buffer_mgr file. In the end it increments the `fixCount` by 1.
    - `getFrameContent` reads the `pageNum` in the page table. **Please note that the frame content can be in any order as page table is based on hashing algorithm (modulus)**
    - `getDirtyFlags` loops over the page table and return which pages are dirty.
    - `getNumReadIO` and `getNumWriteIO` use the properties of `PageTable` struct and reads from `readIOCount` and `writeIOCount`