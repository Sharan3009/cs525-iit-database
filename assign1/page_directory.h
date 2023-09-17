#include "stdio.h"
#include "storage_mgr.h"
#include "dberror.h"

extern RC readPageDirectory(SM_FileHandle *fHandle, SM_PageHandle directory);
extern RC updatePageDirectory(SM_FileHandle *fHandle, SM_PageHandle directory);
