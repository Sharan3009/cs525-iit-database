#include "../tables.h"
extern void serializeSchemaIntoPage(Schema *schema, char *page);
extern void deserializeSchemaFromPage(Schema *schema, char* page);