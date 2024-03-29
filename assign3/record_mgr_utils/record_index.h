#include "../buffer_mgr.h"
#include "../tables.h"
#include "../dt.h"
#ifndef RECORD_INDEX_H
#define RECORD_INDEX_H
    typedef struct RecordIndexNode{
        PageNumber pageNum;
        int slot;
        char *data;
        struct RecordIndexNode *next;
    } RecordIndexNode;

    typedef struct RecordIndexLinkedList {
        int size;
        int keySize;
        RecordIndexNode* head;
    } RecordIndexLinkedList;

    extern void initRecordIndex(RM_TableData *rel);
    extern RecordIndexLinkedList *getRecordIndexList(RM_TableData *rel);
    static RecordIndexNode *createRecordIndexNode(Record *record, Schema *schema);
    extern void insertRecordIndexNode(RM_TableData *rel, Record *record);
    extern RC deleteRecordIndexNode(RM_TableData *rel, RID id);
    extern RecordIndexNode *getRecordIndexNodeByKey(RM_TableData* rel, Record *record);
    extern RecordIndexNode *getRecordIndexNodeById(RM_TableData* rel, RID id);
    extern void closeRecordIndex(RM_TableData *rel);
    extern void deleteRecordIndex(char *name);
    static RC getKeyFromRecordData(Schema *schema, char* recordData, RecordIndexNode *node);
    static int getKeySize(Schema *schema);
    static char* getIndexFileName(char *name);

#endif