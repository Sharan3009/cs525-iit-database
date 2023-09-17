.PHONY: all
all: test_assign1

test_assign1: test_assign1_1.c storage_mgr.c dberror.c page_directory.c
	gcc -o test_assign1 test_assign1_1.c storage_mgr.c dberror.c page_directory.c

.PHONY: clean
clean: 
	rm test_assign1
