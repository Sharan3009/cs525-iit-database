#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../buffer_mgr.h"
#include "../storage_mgr.h"

typedef struct DirectoryEntry {
    PageNumber pageNum;
    bool isFull;
} DirectoryEntry;

typedef struct RecordPage {
    bool isDirectory;
    PageNumber parentPageNum;
    char* data;
} RecordPage;

extern RC createDirectory(int currLevel, int totalLevels, PageNumber directoryPageNum, SM_FileHandle fh);