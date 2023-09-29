.PHONY: all
all: test_assign2_1 test_assign2_2

test_assign2_1: test_assign2_1.c storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c
	gcc -o test_assign2_1 test_assign2_1.c storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c

test_assign2_2: test_assign2_2.c storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c
	gcc -o test_assign2_2 test_assign2_2.c storage_mgr.c dberror.c page_directory.c buffer_mgr_stat.c

.PHONY: clean
clean: 
	rm test_assign2_1 rm test_assign2_2
