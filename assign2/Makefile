# List of common source files
COMMON_FILES = storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c buffer_mgr.c page_table.c linkedlist.c replacement_strategy.c

.PHONY: all
all: test_assign2_1 test_assign2_2 test_assign2_3 test_assign2_4

test_assign2_1: test_assign2_1.c $(COMMON_FILES)
	gcc -o test_assign2_1 test_assign2_1.c $(COMMON_FILES)

test_assign2_2: test_assign2_2.c $(COMMON_FILES)
	gcc -o test_assign2_2 test_assign2_2.c $(COMMON_FILES)

test_assign2_3: test_assign2_3.c $(COMMON_FILES)
	gcc -o test_assign2_3 test_assign2_3.c $(COMMON_FILES)

test_assign2_4: test_assign2_4.c $(COMMON_FILES)
	gcc -o test_assign2_4 test_assign2_4.c $(COMMON_FILES)

.PHONY: clean
clean: 
	rm test_assign2_1 test_assign2_2 test_assign2_3 test_assign2_4
