.PHONY: all
all: test_assign2_1 test_assign2_2

test_assign2_1: test_assign2_1.c storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c buffer_mgr.c page_table.c linkedlist.c replacement_strategy.c
	gcc -o test_assign2_1 test_assign2_1.c storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c buffer_mgr.c page_table.c linkedlist.c replacement_strategy.c

test_assign2_2: test_assign2_2.c storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c buffer_mgr.c page_table.c linkedlist.c replacement_strategy.c
	gcc -o test_assign2_2 test_assign2_2.c storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c buffer_mgr.c page_table.c linkedlist.c replacement_strategy.c

.PHONY: clean
clean: 
	rm test_assign2_1 test_assign2_2
