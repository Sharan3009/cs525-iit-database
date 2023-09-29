GROUP 62
=========================
Sharandeep Singh

pagefile + frames = Buffer pool
1 buffer pool for read and 1 buffer pool to write
Each pool will have same kind of replacement policy. It should be thread safe.
FIFO, LRU, CLOCK, LFU, LRU-K
pin/unpin. evict with pin=0 only
page table for fast access of pages
