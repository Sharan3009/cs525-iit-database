# List of common source files
COMMON_FILES = storage_mgr.c dberror.c buffer_mgr_stat.c buffer_mgr.c page_table.c linkedlist.c replacement_strategy.c

TESTS = test_assign2_1 test_assign2_2 test_assign2_3 test_assign2_4 test_assign2_5

.PHONY: all
all: $(TESTS)

test_assign2_1: test_assign2_1.c $(COMMON_FILES)
	gcc -o test_assign2_1 test_assign2_1.c $(COMMON_FILES)

test_assign2_2: test_assign2_2.c $(COMMON_FILES)
	gcc -o test_assign2_2 test_assign2_2.c $(COMMON_FILES)

test_assign2_3: test_assign2_3.c $(COMMON_FILES)
	gcc -o test_assign2_3 test_assign2_3.c $(COMMON_FILES)

test_assign2_4: test_assign2_4.c $(COMMON_FILES)
	gcc -o test_assign2_4 test_assign2_4.c $(COMMON_FILES)

test_assign2_5: test_assign2_5.c $(COMMON_FILES)
	gcc -o test_assign2_5 test_assign2_5.c $(COMMON_FILES)

test: $(TESTS)
	./test_assign2_1
	./test_assign2_2
	./test_assign2_3
	./test_assign2_4
	./test_assign2_5

.PHONY: clean
clean: 
	rm -f $(TESTS)
