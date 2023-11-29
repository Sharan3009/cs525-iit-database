STORAGE_MGR = storage_mgr.c dberror.c
BUFFER_MGR_UTILS = buffer_mgr_utils/page_table.c buffer_mgr_utils/linkedlist.c buffer_mgr_utils/replacement_strategy.c
RECORD_MGR_UTILS = record_mgr_utils/record_index.c record_mgr_utils/record_mgr_serializer.c
BUFFER_MGR = buffer_mgr_stat.c buffer_mgr.c $(BUFFER_MGR_UTILS)
RECORD_MGR = rm_serializer.c expr.c record_mgr.c $(RECORD_MGR_UTILS)

# List of common source files
COMMON_FILES = $(STORAGE_MGR) $(BUFFER_MGR) $(RECORD_MGR)

DEFAULT_TESTS = test_assign3_1 test_expr
ALL_TESTS = $(DEFAULT_TESTS)

.PHONY: all
all: $(ALL_TESTS)

test_assign3_1: test_assign3_1.c $(COMMON_FILES)
	gcc -g -o test_assign3_1 test_assign3_1.c $(COMMON_FILES) -lm

test_expr: test_expr.c $(COMMON_FILES)
	gcc -g -o test_expr test_expr.c $(COMMON_FILES) -lm

test: $(DEFAULT_TESTS)
	./test_assign3_1
	./test_expr
	
test-all: $(ALL_TESTS)
	./test_assign3_1
	./test_expr

.PHONY: clean
clean: 
	rm -f $(ALL_TESTS)
