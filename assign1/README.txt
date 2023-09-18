# Idea
Used page directory to track filled slots. `\0` is empty page, `1` is filled page
Writing file in binary because its faster
Only `writeBlock` and `readBlock` are the only methods handling everything.
`directory` is initialized in memory. It is stored back to file when close operation is performed

# Code structure


# Test cases

# Extras
talk about bin file
valgrind 0 leaks