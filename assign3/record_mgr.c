#include <stdio.h>
#include <stdlib.h>

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
    return 0;
}

Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){

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
    return RC_OK;
}

RC freeRecord (Record *record){
    return RC_OK;
}

RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
    return RC_OK;
}

RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
    return RC_OK;
}
