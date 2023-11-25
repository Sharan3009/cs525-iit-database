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

extern RC createDirectories(int currLevel, int totalLevels, PageNumber directoryPageNum, SM_FileHandle *fh);
static RC createDirectory(int level, PageNumber directoryPageNum, PageNumber parentPageNum, SM_FileHandle *fh);