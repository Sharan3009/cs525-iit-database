#include "../tables.h"
#include "record_index.h"
#include "page_directory.h"

extern void serializeSchemaIntoPage(Schema *schema, char *page);
extern void deserializeSchemaFromPage(Schema *schema, char* page);
extern int serializeRecordIndexIntoPage(RecordIndexLinkedList *list, char** page);
extern void deserializeRecordIndexFromPage(RecordIndexLinkedList *list, char* page);
extern int serializePageDirectoryIntoPage(PageDirectory *directory, char** page);
extern void deserializePageDirectoryFromPage(PageDirectory *directory, char* page);
static int sizeOfRecordIndex(RecordIndexLinkedList *list);
static int sizeOfPageDirectory(PageDirectory *directory);