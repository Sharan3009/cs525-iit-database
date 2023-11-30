#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr_serializer.h"

void serializeSchemaIntoPage(Schema *schema, char *page){
    char *ptr = page;

    // Serialize numAttr
    memcpy(ptr, &(schema->numAttr), sizeof(int));
    ptr += sizeof(int);

    // Serialize attrNames
    for (int i = 0; i < schema->numAttr; i++) {
        int nameLength = strlen(schema->attrNames[i]) + 1; // +1 for null character
        memcpy(ptr, schema->attrNames[i], nameLength);
        ptr += nameLength;
    }

    // Serialize dataTypes
    memcpy(ptr, schema->dataTypes, schema->numAttr * sizeof(DataType));
    ptr += schema->numAttr * sizeof(DataType);

    // Serialize typeLength
    memcpy(ptr, schema->typeLength, schema->numAttr * sizeof(int));
    ptr += schema->numAttr * sizeof(int);

    // Serialize keySize
    memcpy(ptr, &(schema->keySize), sizeof(int));
    ptr += sizeof(int);

    // Serialize keyAttrs
    memcpy(ptr, schema->keyAttrs, schema->keySize * sizeof(int));
}

void deserializeSchemaFromPage(Schema *schema, char* page){
    char *ptr = page;

    // Deserialize numAttr
    memcpy(&(schema->numAttr), ptr, sizeof(int));
    ptr += sizeof(int);

    // Deserialize attrNames
    schema->attrNames = malloc(schema->numAttr * sizeof(char*));
    for (int i = 0; i < schema->numAttr; i++) {
        int nameLength = strlen(ptr) + 1; // +1 for null character
        schema->attrNames[i] = malloc(nameLength);
        memcpy(schema->attrNames[i], ptr, nameLength);
        ptr += nameLength;
    }

    // Deserialize dataTypes
    schema->dataTypes = malloc(schema->numAttr * sizeof(DataType));
    memcpy(schema->dataTypes, ptr, schema->numAttr * sizeof(DataType));
    ptr += schema->numAttr * sizeof(DataType);

    // Deserialize typeLength
    schema->typeLength = malloc(schema->numAttr * sizeof(int));
    memcpy(schema->typeLength, ptr, schema->numAttr * sizeof(int));
    ptr += schema->numAttr * sizeof(int);

    // Deserialize keySize
    memcpy(&(schema->keySize), ptr, sizeof(int));
    ptr += sizeof(int);

    // Deserialize keyAttrs
    schema->keyAttrs = malloc(schema->keySize * sizeof(int));
    memcpy(schema->keyAttrs, ptr, schema->keySize * sizeof(int));
}

void serializeRecordIndexIntoPage(RecordIndexLinkedList *list, char* page){
    
}
void deserializeRecordIndexFromPage(RecordIndexLinkedList *list, char* page){
    
}
void serializePageDirectoryIntoPage(PageDirectory *directory, char* page){
    
}
void deserializePageDirectoryFromPage(PageDirectory *directory, char* page){
    
}