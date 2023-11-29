#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record_mgr.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "record_mgr_utils/record_mgr_serializer.h"
#include "record_mgr_utils/record_index.h"
#include "record_mgr_utils/page_directory.h"

// table and manager
RC initRecordManager (void *mgmtData){
    initStorageManager();
    return RC_OK;
}
RC shutdownRecordManager (){
    return RC_OK;
}

RC createTable (char *name, Schema *schema){
    // Error handling
    if (name == NULL || schema == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    RC ret = createPageFile(name);
    if (ret != RC_OK) {
        return ret;
    }

    // Open the page file
    SM_FileHandle fh;
    ret = openPageFile(name, &fh);
    if (ret != RC_OK) {
        return ret;
    }

    // Write the schema to the first page
    SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);
    memset(ph, '\0', PAGE_SIZE);
    serializeSchemaIntoPage(schema, ph);
    ret = writeBlock(0, &fh, ph);
    if (ret != RC_OK) {
        free(ph);
        closePageFile(&fh);
        return ret;
    }

    // Close the page file
    ret = closePageFile(&fh);
    free(ph);

    return ret;
}

RC openTable (RM_TableData *rel, char *name){

    if (rel == NULL || name == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    BM_BufferPool *bm = MAKE_POOL();
    
    RC ret = initBufferPool(bm, name, 100, RS_LRU, NULL);
    if (ret != RC_OK) {
        free(bm);
        return ret;
    }

    BM_PageHandle *firstPage = MAKE_PAGE_HANDLE();
    ret = pinPage(bm, firstPage, 0);
    if (ret != RC_OK) {
        free(bm);
        shutdownBufferPool(bm);
        return ret;
    }
    // setting the name
    rel->name = name;
    // setting the schema
    Schema *schema = (Schema *)malloc(sizeof(Schema));
    deserializeSchemaFromPage(schema, firstPage->data);
    rel->schema = schema;
    int sizemgmtData = sizeof(BM_BufferPool) + sizeof(RecordIndexLinkedList) + sizeof(PageDirectory);
    // setting buffer manager in mgmtData
    rel->mgmtData = (void *)malloc(sizemgmtData);
    memset(rel->mgmtData, '\0', sizemgmtData);
    memcpy(rel->mgmtData, bm, sizeof(BM_BufferPool));
    initRecordIndex(rel);
    initPageDirectory(rel);
    free(firstPage);
    free(bm);
    return ret;
}

RC closeTable (RM_TableData *rel){

    if(rel==NULL){
        return RC_FILE_HANDLE_NOT_INIT;
    }
    // getting buffer manager from mgmtData
    BM_BufferPool *bm = (BM_BufferPool *)rel->mgmtData;
    // unpinning first page
    BM_PageHandle *firstPage = MAKE_PAGE_HANDLE();
    firstPage->pageNum = 0;
    RC ret = unpinPage(bm, firstPage);
    // cleaning
    destroyRecordIndex(rel);
    destroyPageDirectory(rel);
    shutdownBufferPool(bm);
    freeSchema(rel->schema);
    free(rel->mgmtData);
    free(firstPage);
    return RC_OK;
}

RC deleteTable (char *name){
    if (name == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    return destroyPageFile(name);
}

int getNumTuples (RM_TableData *rel){
    RecordIndexLinkedList *list = getRecordIndexList(rel);
    RecordIndexNode *node = list->head;
    int count = 0;
    while(node!=NULL){
        node = node->next;
        count++;
    }
    return count;
}


// handling records in a table
RC insertRecord (RM_TableData *rel, Record *record){
    return RC_OK;
}

RC deleteRecord (RM_TableData *rel, RID id){
    return RC_OK;
}

RC updateRecord (RM_TableData *rel, Record *record){
    return RC_OK;
}

RC getRecord (RM_TableData *rel, RID id, Record *record){
    if(rel==NULL || rel->mgmtData == NULL, id.page==-1 || id.slot==-1 || record ==NULL || record->data == NULL){
        return RC_READ_NON_EXISTING_PAGE;
    }
    RC ret;
    BM_BufferPool *bm = (BM_BufferPool*)rel->mgmtData;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();
    memset(page, '\0', sizeof(BM_PageHandle));
    if((ret=pinPage(bm, page, id.page))!=RC_OK){
        return ret;
    }
    int recordSize = 0;
    for(int i=0;i<rel->schema->numAttr;i++){
        switch(rel->schema->dataTypes[i]){
            case DT_INT:
                recordSize+=sizeof(int);
            case DT_FLOAT:
                recordSize+=sizeof(float);
            case DT_BOOL:
                recordSize+=sizeof(bool);
            case DT_STRING:
                recordSize+=(rel->schema->typeLength[i] + 1);
        }
    }
    int offset = id.slot*recordSize;
    record->id.page = id.page;
    record->id.slot = id.slot;
    memcpy(record->data, page->data+offset, recordSize);
    free(page);
    return RC_OK;
}


// scans
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
    return RC_OK;
}

RC next (RM_ScanHandle *scan, Record *record){
    return RC_OK;
}

RC closeScan (RM_ScanHandle *scan){
    return RC_OK;
}


// dealing with schemas
int getRecordSize (Schema *schema){

    // error handling
    if(schema == NULL) {
        return -1;
    }

    int recordSize = 0;

    for (int i = 0; i < schema->numAttr; i++) {

        switch (schema->dataTypes[i]) {
            case DT_INT:
                recordSize += sizeof(int);
                break;
            case DT_STRING:
                recordSize += (schema->typeLength[i] + 1); // +1 for the null termination
                break;
            case DT_FLOAT:
                recordSize += sizeof(float);
                break;
            case DT_BOOL:
                recordSize += sizeof(bool);
                break;
            default:
                // do nothing
        }
    }

    return recordSize;
}

Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){

    // error handling
    if(numAttr<=0 || attrNames==NULL || dataTypes==NULL || typeLength == NULL){
        return NULL;
    }

    // allocating memory of Schema
    Schema *schema = (Schema *)malloc(sizeof(Schema));
    memset(schema, '\0', sizeof(Schema));

    // assigning parameters to schema attributes
    schema->numAttr = numAttr;
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->typeLength = typeLength;
    schema->keyAttrs = keys;
    schema->keySize = keySize;

    return schema;
}

RC freeSchema (Schema *schema){

    if(schema == NULL){
        return !RC_OK;
    }
    // Freeing attribute
    for (int i = 0; i < schema->numAttr; i++) {
        free(schema->attrNames[i]);
    }

    // Freeing arrays
    free(schema->attrNames);
    free(schema->dataTypes);
    free(schema->typeLength);
    free(schema->keyAttrs);

    free(schema);

    return RC_OK;
}


// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema){

    if(schema == NULL){
        return !RC_OK;
    }
    
    // checking if record size is negative
    int recordSize = getRecordSize(schema);
    if(recordSize<0){
        return !RC_OK;
    }

    // allocating record struct memory
    *record = (Record *)malloc(sizeof(Record));

    Record *recordPtr = *record;

    // initializing record id as -1
    recordPtr->id.page = -1;
    recordPtr->id.slot = -1;

    // allocating memory for record data
    recordPtr->data = (char *)malloc(recordSize);

    return RC_OK;
}

RC freeRecord (Record *record){
    if(record == NULL){
        return !RC_OK;
    }

    // free record data
    free(record->data);

    // free record itself
    free(record);

    return RC_OK;
}

RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){

    if(record == NULL || schema == NULL || attrNum<0 || attrNum>=schema->numAttr){
        return !RC_OK;
    }

    *value = (Value *)malloc(sizeof(Value));

    Value* valuePtr = *value;
    DataType dt =  schema->dataTypes[attrNum];

    int attrOffset = 0;
    for(int i=0;i<attrNum;i++){
        switch (schema->dataTypes[i]){
            case DT_INT:
                attrOffset += sizeof(int);
                break;
            case DT_FLOAT:
                attrOffset += sizeof(float);
                break;
            case DT_BOOL:
                attrOffset += sizeof(bool);
                break;
            case DT_STRING:
                attrOffset += (schema->typeLength[i] + 1);
                break;
            default:
                return RC_RM_UNKOWN_DATATYPE;
        }
    }

    valuePtr->dt =  dt;

    switch (dt){
        case DT_INT:
            memcpy(&valuePtr->v.intV, record->data + attrOffset, sizeof(int));
            break;
        case DT_FLOAT:
            memcpy(&valuePtr->v.floatV, record->data + attrOffset, sizeof(float));
            break;
        case DT_BOOL:
            memcpy(&valuePtr->v.boolV, record->data + attrOffset, sizeof(bool));
            break;
        case DT_STRING:
            valuePtr->v.stringV = (char *)malloc(schema->typeLength[attrNum] + 1); // +1 for the null character
            memcpy(valuePtr->v.stringV, record->data + attrOffset, schema->typeLength[attrNum] + 1);
            break;
        default:
            return RC_RM_UNKOWN_DATATYPE;
    }

    return RC_OK;
}

RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){

    if(record == NULL || schema == NULL || attrNum<0 || attrNum>=schema->numAttr || value==NULL){
        return !RC_OK;
    }

    DataType dt =  value->dt;

    if(dt!=schema->dataTypes[attrNum]){
        return RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE;
    }

    int attrOffset = 0;
    for(int i=0;i<attrNum;i++){
        switch (schema->dataTypes[i]){
            case DT_INT:
                attrOffset += sizeof(int);
                break;
            case DT_FLOAT:
                attrOffset += sizeof(float);
                break;
            case DT_BOOL:
                attrOffset += sizeof(bool);
                break;
            case DT_STRING:
                attrOffset += (schema->typeLength[i] + 1);
                break;
            default:
                return RC_RM_UNKOWN_DATATYPE;
        }
    }

    switch (dt){
        case DT_INT:
            memcpy(record->data + attrOffset, &value->v.intV, sizeof(int));
            break;
        case DT_FLOAT:
            memcpy(record->data + attrOffset, &value->v.floatV, sizeof(float));
            break;
        case DT_BOOL:
            memcpy(record->data + attrOffset, &value->v.boolV, sizeof(bool));
            break;
        case DT_STRING:
            memcpy(record->data + attrOffset, value->v.stringV, schema->typeLength[attrNum] + 1); // for null character
            break;
        default:
            return RC_RM_UNKOWN_DATATYPE;
    }

    return RC_OK;
}
