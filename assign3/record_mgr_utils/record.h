#include "../buffer_mgr.h"

typedef struct RecordPage {
    bool isDirectory;
    PageNumber parentPageNum;
    void* data;
} RecordPage;

typedef struct RecordPageData {
    int** slotPointers;
    void* records;
} RecordEntry;

typedef struct RecordData {
    int size;
    void* data;
} RecordData;