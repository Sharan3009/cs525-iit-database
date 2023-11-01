#include "../dt.h"
#include "../buffer_mgr.h"
#include "page_table.h"
#include "linkedlist.h"

typedef struct ClockEntry {
    PageEntry* entry;
    short referenceBit;
} ClockEntry;

typedef struct Clock {
    ClockEntry* arr;
    int hand;
    int size;
    int capacity;
} Clock;

extern void initReplacementStrategy(BM_BufferPool *const bm, void* stratData);
extern PageNumber evictPage(BM_BufferPool *const bm);
static PageNumber evictFromHead(BM_BufferPool *const bm);
static PageNumber evictFromClock(BM_BufferPool *const bm);
extern void admitPage(BM_BufferPool *const bm, PageEntry* entry);
static void admitLruK(PageEntry* entry);
static void admitLfu(PageEntry* entry);
static void admitClock(PageEntry* entry);
extern void reorderPage(BM_BufferPool *const bm, PageEntry* entry);
static void reorderLruK(BM_BufferPool *const bm, PageEntry* entry);
static void reorderLfu(BM_BufferPool *const bm, PageEntry* entry);
static void reorderClock(BM_BufferPool *const bm, PageEntry* entry);
extern void clearStrategyData(BM_BufferPool *const bm);
static void freeClock();
static void freeLinkedList();
