#include "../tables.h"
#include "record_index.h"
#include "page_directory.h"

extern void serializeSchemaIntoPage(Schema *schema, char *page);
extern void deserializeSchemaFromPage(Schema *schema, char* page);
extern void serializeRecordIndexIntoPage(RecordIndexLinkedList *list, char* page);
extern void deserializeRecordIndexFromPage(RecordIndexLinkedList *list, char* page);
extern void serializePageDirectoryIntoPage(PageDirectory *directory, char* page);
extern void deserializePageDirectoryFromPage(PageDirectory *directory, char* page);