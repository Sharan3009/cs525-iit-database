#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record_mgr.h"

// table and manager
RC initRecordManager (void *mgmtData){
    return RC_OK;
}
RC shutdownRecordManager (){
    return RC_OK;
}

RC createTable (char *name, Schema *schema){
    return RC_OK;
}

RC openTable (RM_TableData *rel, char *name){
    return RC_OK;
}

RC closeTable (RM_TableData *rel){
    return RC_OK;
}

RC deleteTable (char *name){
    return RC_OK;
}

int getNumTuples (RM_TableData *rel){
    return 0;
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
                recordSize += schema->typeLength[i];
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
    if(numAttr<=0 || attrNames==NULL || dataTypes==NULL || typeLength == NULL
        || sizeof(attrNames)/sizeof(int)){
        return NULL;
    }

    // Check if all have numAttr number of elements in it.
    for (int i = 0; i < numAttr; i++) {
        if (attrNames[i] == NULL || dataTypes == NULL || typeLength == NULL) {
            return NULL;
        }
    }

    // allocating memory of Schema
    Schema *schema = malloc(sizeof(Schema));

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

    if(record == NULL || schema == NULL){
        return !RC_OK;
    }

    int recordSize = getRecordSize(schema);
    if(recordSize<0){
        return !RC_OK;
    }

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

    if(record == NULL || schema == NULL || attrNum<0 || attrNum>=schema->numAttr || value==NULL){
        return !RC_OK;
    }

    DataType dt =  schema->dataTypes[attrNum];

    int attrOffset = 0;
    for(int i=0;i<attrNum;i++){
        attrOffset += schema->typeLength[i];
    }

    (*value)->dt =  dt;

    switch (dt){
        case DT_INT:
            memcpy(&(*value)->v.intV, record->data + attrOffset, sizeof(int));
            break;
        case DT_FLOAT:
            memcpy(&(*value)->v.floatV, record->data + attrOffset, sizeof(float));
            break;
        case DT_BOOL:
            memcpy(&(*value)->v.boolV, record->data + attrOffset, sizeof(bool));
            break;
        case DT_STRING:
            (*value)->v.stringV = (char *)malloc(schema->typeLength[attrNum]);
            memcpy((*value)->v.stringV, record->data + attrOffset, schema->typeLength[attrNum]);
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
        attrOffset += schema->typeLength[i];
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
            memcpy(record->data + attrOffset, value->v.stringV, schema->typeLength[attrNum]);
            break;
        default:
            return RC_RM_UNKOWN_DATATYPE;
    }

    return RC_OK;
}
