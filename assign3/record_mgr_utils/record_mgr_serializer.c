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

int serializeRecordIndexIntoPage(RecordIndexLinkedList *list, char** page){

    int size = sizeof(int) + sizeof(int) + list->size*(sizeof(PageNumber) + sizeof(int) + list->keySize);

    *page = (char *)malloc(size);
    memset(*page, '\0', size);

    char *ptr = *page;

    memcpy(ptr, &(list->size), sizeof(int));
    ptr += sizeof(int);

    memcpy(ptr, &(list->keySize), sizeof(int));
    ptr += sizeof(int);

    RecordIndexNode *node = list->head;
    // Serialize attrNames
    for (int i = 0; i < list->size; i++) {
        memcpy(ptr, &(node->pageNum), sizeof(PageNumber));
        ptr += sizeof(PageNumber);

        memcpy(ptr, &(node->slot), sizeof(int));
        ptr += sizeof(int);

        memcpy(ptr, node->data, list->keySize);
        ptr += list->keySize;

        node = node->next;
    }

    return size;
    
}

void deserializeRecordIndexFromPage(RecordIndexLinkedList *list, char* page){
    char *ptr = page;

    memcpy(&(list->size), ptr, sizeof(int));
    ptr += sizeof(int);

    memcpy(&(list->keySize), ptr, sizeof(int));
    ptr += sizeof(int);

    for (int i=0; i<list->size;i++){
        RecordIndexNode *node = (RecordIndexNode *)malloc(sizeof(RecordIndexNode));
        memset(node, '\0', sizeof(RecordIndexNode));

        memcpy(&(node->pageNum), ptr, sizeof(PageNumber));
        ptr += sizeof(PageNumber);

        memcpy(&(node->slot), ptr, sizeof(int));
        ptr += sizeof(int);

        node->data = (char *)malloc(list->keySize);
        memset(node->data, '\0', list->keySize);
        memcpy(node->data, ptr, list->keySize);
        ptr += list->keySize;

        if (list->head == NULL) {
            list->head = node;
        } else {
            node->next = list->head;
            list->head = node;
        }
    }

}

int serializePageDirectoryIntoPage(PageDirectory *directory, char** page){

    int size = sizeof(int) + directory->capacity*sizeof(bool);
    *page = (char *)malloc(size);
    memset(*page, '\0', size);

    char *ptr = *page;

    memcpy(ptr, &(directory->capacity), sizeof(int));
    ptr += sizeof(int);

    for(int i=0;i<directory->capacity;i++){
        memcpy(ptr, &(directory->isFull[i]), sizeof(bool));
        ptr += sizeof(bool);
    }

    return size;
    
}

void deserializePageDirectoryFromPage(PageDirectory *directory, char* page){
    char *ptr = page;

    memcpy(&(directory->capacity), ptr, sizeof(int));
    ptr += sizeof(int);

    directory->isFull = (bool *)malloc(directory->capacity * sizeof(bool));
    memset(directory->isFull, '\0', directory->capacity * sizeof(bool));

    for(int i=0;i<directory->capacity;i++){
        memcpy(&(directory->isFull[i]), ptr, sizeof(bool));
        ptr += sizeof(bool);
    }
}