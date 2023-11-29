#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_index.h"

RecordIndexNode *createRecordIndexNode(Record *record, Schema *schema){
    RecordIndexNode *node = (RecordIndexNode *)malloc(sizeof(RecordIndexNode));
    node->pageNum = record->id.page;
    node->slot = record->id.slot;
    if(getKeyFromRecordData(schema, record->data, node)!=RC_OK){
        return NULL;
    }
    node->next = NULL;
    return node;
}

void insertRecordIndexNodeAtBeginning(RecordIndexLinkedList* list, Record *record, Schema *schema) {
    RecordIndexNode* newNode = createRecordIndexNode(record, schema);
    if (list->head == NULL) {
        list->head = newNode;
    } else {
        newNode->next = list->head;
        list->head = newNode;
    }
}

// Delete a node with given data from the linked list
RC deleteRecordIndexNode(RecordIndexLinkedList* list, Record* record, Schema *schema) {
    RecordIndexNode* temp = list->head;
    RecordIndexNode* prev = NULL;

    while (temp != NULL && record->id.page != temp->pageNum && record->id.slot != temp->slot) {
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
    free(temp);

    return RC_OK;
}

RC getKeyFromRecordData(Schema *schema, char* recordData, RecordIndexNode *node){
    if(schema->keySize==0){
        return RC_IM_KEY_NOT_FOUND;
    }
    
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
                size=(schema->typeLength[attr] + 1);
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