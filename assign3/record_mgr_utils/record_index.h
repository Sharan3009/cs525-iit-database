#include "../buffer_mgr.h"
#include "../tables.h"
#include "../dt.h"
#ifndef RECORD_INDEX_H

    typedef struct RecordIndexNode{
        PageNumber pageNum;
        int slot;
        char *data;
        struct RecordIndexNode *next;
    } RecordIndexNode;

    typedef struct RecordIndexLinkedList {
        RecordIndexNode* head;
    } RecordIndexLinkedList;

    extern void initRecordIndex(void* mgmtData);
    extern RecordIndexLinkedList *getRecordIndexList(void* mgmtData);
    static RecordIndexNode *createRecordIndexNode(Record *record, Schema *schema);
    extern void insertRecordIndexNodeAtBeginning(RM_TableData *rel, Record *record);
    extern RC deleteRecordIndexNode(RM_TableData *rel, Record* record);
    extern RecordIndexNode *getRecordIndexNode(RM_TableData* rel, char* recordData);
    extern void destroyRecordIndex(void* mgmtData);
    static RC getKeyFromRecordData(Schema *schema, char* recordData, RecordIndexNode *node);
    static int getKeySize(Schema *schema);

#endif