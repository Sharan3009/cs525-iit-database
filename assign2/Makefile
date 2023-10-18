# List of common source files
COMMON_FILES = storage_mgr.c dberror.c buffer_mgr_stat.c buffer_mgr.c page_table.c linkedlist.c replacement_strategy.c

DEFAULT_TESTS = test_assign2_1 test_assign2_2
ALL_TESTS = $(DEFAULT_TESTS) test_assign2_3 test_assign2_4 test_assign2_5 test_assign2_6

.PHONY: all
all: $(ALL_TESTS)

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

test_assign2_6: test_assign2_6.c $(COMMON_FILES)
	gcc -o test_assign2_6 test_assign2_6.c $(COMMON_FILES)

test: $(DEFAULT_TESTS)
	./test_assign2_1
	./test_assign2_2
	
test-all: $(ALL_TESTS)
	./test_assign2_1
	./test_assign2_2
	./test_assign2_3
	./test_assign2_4
	./test_assign2_5
	./test_assign2_6

.PHONY: clean
clean: 
	rm -f $(ALL_TESTS)
