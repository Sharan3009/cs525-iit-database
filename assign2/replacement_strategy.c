#include <stdio.h>
#include <stdlib.h>

#include "replacement_strategy.h"
#include "page_table.h"
#include "linkedlist.h"

LinkedList* list = NULL;

void initReplacementStrategy(BM_BufferPool *const bm){

    switch (bm->strategy){
        case RS_FIFO:
            list = (LinkedList*)malloc(sizeof(LinkedList));
            list->head = NULL;
            list->tail = NULL;
            break;
        default:
            break;
    }
}

int evictPage(BM_BufferPool *const bm){
    switch (bm->strategy){
        case RS_FIFO:
            return evictFifo(bm);
    }
    return -1;
}

static int evictFifo(BM_BufferPool *const bm){
    Node* temp = list->head;
    Node* prev = NULL;
    PageTable *pageTable = getPageTable(bm);
    int index = hasPage(bm, temp->data);

    jump:
    while (temp != NULL && pageTable->table[index].fixCount>0) {
        prev = temp;
        temp = temp->next;
        if(temp!=NULL)
            index = hasPage(bm, temp->data);
    }

    // If the node is not found
    if (temp == NULL) {
        goto jump;
    }

    // Remove the node from the list
    if (prev == NULL) {
        list->head = temp->next;
        if (list->head == NULL) {
            list->tail = NULL;
        }
    } else {
        prev->next = temp->next;
        if (prev->next == NULL) {
            list->tail = prev;
        }
    }
    PageNumber pageNum = temp->data;
    // Free the memory occupied by the deleted node
    free(temp);
    return pageNum;
}


void admitPage(BM_BufferPool *const bm, PageNumber pageNum){
    switch (bm->strategy){
        case RS_FIFO:
            admitFifo(pageNum);
            break;
    }
}

static void admitFifo(PageNumber pageNum){
    insertAtEnd(list, pageNum);
}