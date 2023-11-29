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

    static RecordIndexNode *createRecordIndexNode(Record *record, Schema *schema);
    extern void insertRecordIndexNodeAtBeginning(RecordIndexLinkedList* list, Record *record, Schema *schema);
    extern RC deleteRecordIndexNode(RecordIndexLinkedList* list, Record* record, Schema *schema);
    static RC getKeyFromRecordData(Schema *schema, char* recordData, RecordIndexNode *node);

#endif