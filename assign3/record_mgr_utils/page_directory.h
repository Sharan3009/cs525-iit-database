#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../buffer_mgr.h"
#include "../storage_mgr.h"
#include "record.h"

typedef struct DirectoryEntry {
    PageNumber pageNum;
    bool isFull;
} DirectoryEntry;

extern RC createDirectories(int currLevel, int totalLevels, PageNumber directoryPageNum, SM_FileHandle *fh);
static RC createDirectory(PageNumber directoryPageNum, PageNumber parentPageNum, PageNumber firstEntryPageNum, SM_FileHandle *fh);
static int getNumEntries();