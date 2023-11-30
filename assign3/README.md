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
- All the test cases pass but you will see some **valgrind memory leaks happening in the given test case files**.

# Code structure