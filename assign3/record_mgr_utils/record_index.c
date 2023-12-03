#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_index.h"
#include "record_mgr_serializer.h"

#define READ "r"
#define WRITE "wb+"

void initRecordIndex(RM_TableData *rel){

    // opening file
    char *fileName = getIndexFileName(rel->name);
    FILE *file = fopen(fileName, READ);

    RecordIndexLinkedList * list = (RecordIndexLinkedList *)malloc(sizeof(RecordIndexLinkedList));
    memset(list, '\0', sizeof(RecordIndexLinkedList));

    // file never existed
    if(file==NULL){
        list->size = 0;
        list->head = NULL;
        list->keySize = getKeySize(rel->schema);
        memcpy((char*)rel->mgmtData + sizeof(BM_BufferPool), list, sizeof(RecordIndexLinkedList));
    } else {
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate memory for the entire file
        char* page = (char*)malloc(fileSize); 

        fread(page, 1, fileSize, file);

        deserializeRecordIndexFromPage(list, page);

        memcpy((char*)rel->mgmtData + sizeof(BM_BufferPool), list, sizeof(RecordIndexLinkedList));

        free(page);  
        fclose(file);

    }

    free(fileName);
    free(list);
}

RecordIndexLinkedList *getRecordIndexList(RM_TableData *rel){
    return (RecordIndexLinkedList *)((char*)rel->mgmtData + sizeof(BM_BufferPool));
}

RecordIndexNode *createRecordIndexNode(Record *record, Schema *schema){
    RecordIndexNode *node = (RecordIndexNode *)malloc(sizeof(RecordIndexNode));
    node->pageNum = record->id.page;
    node->slot = record->id.slot;
    int keySize = getKeySize(schema);
    node->data = malloc(keySize);
    memset(node->data, '\0', keySize);
    if(getKeyFromRecordData(schema, record->data, node)!=RC_OK){
        return NULL;
    }
    node->next = NULL;
    return node;
}

void insertRecordIndexNode(RM_TableData *rel, Record *record) {
    RecordIndexLinkedList *list = getRecordIndexList(rel);
    list->size++;
    RecordIndexNode* newNode = createRecordIndexNode(record, rel->schema);
    if (list->head == NULL) {
        list->head = newNode;
    } else {
        newNode->next = list->head;
        list->head = newNode;
    }
}

// Delete a node with given data from the linked list
RC deleteRecordIndexNode(RM_TableData *rel, RID id) {
    RecordIndexLinkedList *list = getRecordIndexList(rel);
    RecordIndexNode* temp = list->head;
    RecordIndexNode* prev = NULL;

    while (temp != NULL && !(id.page == temp->pageNum && id.slot == temp->slot)) {
        prev = temp;
        temp = temp->next;
    }

    // If the node is not found
    if (temp == NULL) {
        return RC_IM_KEY_NOT_FOUND;
    }

    // Remove the node from the list
    if (prev == NULL) {
        list->head = temp->next;
    } else {
        prev->next = temp->next;
    }
    temp->next = NULL;
    list->size--;
    free(temp->data);
    free(temp);

    return RC_OK;
}

RecordIndexNode *getRecordIndexNodeByKey(RM_TableData* rel, Record* record) {
    RecordIndexLinkedList *list = getRecordIndexList(rel);
    RecordIndexNode *key = malloc(sizeof(RecordIndexNode));
    memset(key, '\0', sizeof(RecordIndexNode));
    key->data = malloc(list->keySize);
    memset(key->data, '\0', list->keySize);
    if(getKeyFromRecordData(rel->schema, record->data, key)!=RC_OK){
        return NULL;
    }
    RecordIndexNode* temp = list->head;
    RecordIndexNode* prev = NULL;

    while (temp != NULL && memcmp(temp->data, key->data, list->keySize)!=0) {
        prev = temp;
        temp = temp->next;
    }
    free(key->data);
    free(key);
    return temp;
}

RecordIndexNode *getRecordIndexNodeById(RM_TableData* rel, RID id) {
    RecordIndexLinkedList *list = getRecordIndexList(rel);
    RecordIndexNode* temp = list->head;
    RecordIndexNode* prev = NULL;
    while (temp != NULL && !(id.page == temp->pageNum && id.slot == temp->slot)) {
        prev = temp;
        temp = temp->next;
    }
    return temp;
}

void closeRecordIndex(RM_TableData *rel){
    RecordIndexLinkedList *list = getRecordIndexList(rel);

    char *fileName = getIndexFileName(rel->name);
    char *page;
    int size = serializeRecordIndexIntoPage(list, &page);
    
    FILE* file = fopen(fileName, WRITE);

    fwrite(page, 1, size, file);

    fclose(file);

    free(fileName);

    RecordIndexNode* temp = list->head;
    RecordIndexNode* next = NULL;

    while (temp != NULL) {
        next = temp->next;
        free(temp->data);
        free(temp);
        temp = next;
    }
    list->size = 0;

    list->head = NULL; 
    free(page);
}

void deleteRecordIndex(char *name){
    char *fileName = getIndexFileName(name);
    remove(fileName);
    free(fileName);
}

RC getKeyFromRecordData(Schema *schema, char* recordData, RecordIndexNode *node){
    for(int attr=0; attr<schema->numAttr;attr++){

        int size = 0;
        int offset = 0;
        switch(schema->dataTypes[attr]){
            case DT_INT:
                size=sizeof(int);
                break;
            case DT_FLOAT:
                size=sizeof(float);
                break;
            case DT_BOOL:
                size=sizeof(bool);
                break;
            case DT_STRING:
                size=(schema->typeLength[attr] + 1)*sizeof(char);
                break;
            default:
                size=0;
                break;
        }

        for(int key=0;key<schema->keySize;key++){
            int keyAttr = schema->keyAttrs[key];
            if(keyAttr == attr){
                memcpy(node->data + offset, recordData, size);
                offset +=size;
                break;
            }
        }
        recordData += size;
    }
    return RC_OK;
}

int getKeySize(Schema *schema){
    int size = 0;
    for(int i=0;i<schema->keySize;i++){
        switch(schema->dataTypes[schema->keyAttrs[i]]){
            case DT_INT:
                size+=sizeof(int);
                break;
            case DT_FLOAT:
                size+=sizeof(float);
                break;
            case DT_BOOL:
                size+=sizeof(bool);
                break;
            case DT_STRING:
                size+=(schema->typeLength[schema->keyAttrs[i]] + 1)*sizeof(char);
                break;
        }
    }
    return size;
}

char *getIndexFileName(char *name){
    char *fileName = malloc(strlen(name) + strlen("_index") + 1);
    strcpy(fileName, name);
    strcat(fileName, "_index");
    return fileName;
}