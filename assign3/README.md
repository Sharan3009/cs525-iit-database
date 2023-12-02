# GROUP 62
Sharandeep Singh


# Ideation
- Record Manager has 3 main components
    - `record_index.c` - It is an index of `keys`. Using this index, we can fast search the keys as well as tell the number of tuples.
    - `page_directory.c` - Page directory keep tracks of which page is empty and which is full. Using this directory, the next empty slot is allocated.
    - `record_mgr_serialization.c` - This file serializes the struct data so that we can store it in the file. It also deserializes from the disk to memory (struct). It serialize and deserialize schema, record index and page directory.

# How to execute
- Navigate to the folder `assign3` and open the terminal here.
- Run `make clean` to clear test files file if any.    
- To execute test cases you can do any of the following ways
    - Run `make test` that will execute `test_assign3_1`.
    - Or Run `make test-all` to execute all the test files including `test_expr`.
    - Or Run `make` and execute each test case individually like `./test_assign3_1`
- Make sure all the database files are deleted before running the test cases.
- For every table file you will see two extra files with same name but suffix of `_index` and `_directory`. They store the record index and page directories respectively.
- All the test cases pass. **Note** - You might see some memory leaks but they are because of the test file as memory deallocation is not handled propertly in `test_assign3_1.c`.

# Code structure
- `record_mgr_utils` folder consist of auxillary files needed by `record_mgr.c` to pull through its working. This is for making code modular and readable. This folder consists of `page_directory.c`,`record_index.c` and `record_mgr_serializer.c`.
- `page_directory.h` has the struct of `PageDirectory`.
    - `PageDirectory` consists of 
        - `capacity` that stores the size of page directory. its initial value is 10 pages.
        - `isFull` is the dynamic array of booleans. `true` means the page is full and `false` means the page has an empty slot. Whenever the `isFull` array fills up, the size doubles to accomodate new added pages.
- `page_directory.c`- It creates a file under the hood. The name of this file is always the table name plus the suffix of `_directory`. It maintains the `PageDirectory` structure in the file. It initializes, closes and gets deleted along with the original file.

- `record_index.h` has the struct of `RecordIndexLinkedList` and `RecordIndexNode` as described below
    - `RecordIndexNode` - it is a node in the linkedlist. it consists of
        - `pageNum` - PageNumber that the record is in.
        - `slot` - the offset where the record is within that page
        - `data` - It contains only they **key** part of the record.
        - `next` - It points to the next record index.
    - `RecordIndexLinkedList` - it is the linkedlist storing all the keys in the file.
        -  `size` - It is the size of the linked lists. Or we can say it is the number of tuples in the file.
        - `keySize` - this is the size of key that each node is holding in since we have constant size records.
        - `head` - the starting point of the linked list.
- **Primary key constraint** - Please note that this record manager strictly require a primary key and will not add duplicates based on the key mentioned. Without the key, the test cases may not pass.

- `record_index.c` -  It creates a file under the hood. The name of this file is always the table name plus the suffix of `_index_`. It maintains the `RecordIndexLinkedList` structure in the file. It initializes, closes and gets deleted along with the original file.

- `record_mgr_serializer.c` - It is a file that helps serialize and deserialize the above structs and `Schema`. This file contains the methods to serialize `PageDirectory`, `RecordIndexLinkedList` and `Schema` into a page. Correspondingly it also has the deserializing methods that extract out the structs from the `char*` page.

- `record_mgr.c`
    - `ScanMetadata` - It has a struct to store metadata required for scanning the data. This involves
        - `expr` - Expression input while starting the scan
        - `recordSize` - the size of the records to prevent calculating recordSize again and again
        - `slots` - The total number of slots in a single page
        - `totalTuples` - The number of tuples in a file. This decides when to return `RC_RM_NO_MORE_TUPLES`
        - `pageNum` - initializes to `1`. But it gets incremented based on the `next` method call.
        - `slot` - initializes to `0`. But it gets incremented based on the `next` method call.
        - `currTuple` - initializes to `0`. It is the number of tuples processed by the scan.
        - `page` - the page of the page number.
    
    - `initRecordManager` - It will initialize record manager and calls `initStorageManager` from within.
    - `shutdownRecordManager` - It should be used to free up all the resources. However we do not have any global variables to free so this method does free anything.
    - `createTable` - It creates the pagefile with the table name. It also adds the schema to the first page of the file. it uses the serialization method to serialize and store in the first page. Hence, Page `0` is for schema.
    - `openTable` - This method initializes the buffer pool. I am using `LRU` and 100 pages of buffer pool. After that it reads the schema out of the 0th page by deserializing and store to `rel->mgmtData`. In addition to that, `PageDirectory` and `RecordIndexLinkedList` is initialized and stored to `rel->mgmtdata`.
    - `closeTable` - it closes the file and save its corresponding directory and index data. It also cleans up all the `rel->mgmtData` i.e `Schema`, `PageDirectory` and `RecordIndexLinkedList`.
    - `deleteTable` - It deletes the file and its corresponding directory and index file.
    - `getNumTuples` - returns the `size` key from `RecordIndexLinkedList` struct.
    - `insertRecord` - It first checks if the `key` already exists in the data using the record index. If it does, then it simply returns `RC_IM_KEY_ALREADY_EXISTS`. Otherwise it finds the appropriate empty page and slot and insert the record there. After that it inserts the key in record index and sets the page `isFull` status to `true` if the page is full.
    - `deleteRecord` - It deletes the record in the given page and slot and also deletes the record index otherwise it returns `RC_IM_KEY_NOT_FOUND`. Also it sets the `isFull` flag to false because we delete the record.
    - `updateRecord` - It first checks if the key exists in the data using the record index. If it does not, then it returns `RC_IM_KEY_NOT_FOUND`. It updates the record in the given page and slot.
    - `getRecord` - It first check if the key exist in the record index. if it does not then it simply return `RC_IM_KEY_NOT_FOUND`. Otherwise it goes to that page and slot and reads the record data.
    - `startScan` - It initializes `ScanMetadata` and store it in the `scan->mgmtData`. Its initial values are explained in the `ScanMetadata` struct part.
    - `next` - It uses storage manager directly to get the page data instead of buffer manager. This is to prevent buffer pool pollute because its a sequential scan. This method will only read the page if `slot==0` which indicates its a new page. After that appropriate page, slot and currTuple value is updated. if `evalExpr` returns true then it returns RC_OK other wise the error code. In the end if it cross the total number of records then it returns `RC_RM_NO_MORE_TUPLES`
    - `closeScan` - it frees the `scan->mgmtData` metadata.
    - `getRecordSize` - It calculates the totalRecords size using `numAttr`. If datatype is a string then it uses `typeLength`+1 otherwise it just adds sizeof(datatype).
    - `createSchema`, `freeSchema`, `createRecord`, `freeRecord`, `getAttr`, `setAttr` just create the schemas, records and set/get attribute values in the records.