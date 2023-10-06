#include <stdio.h>
#include <stdlib.h>

#include "replacement_strategy.h"
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

// generic public function to get page according to strategy
PageNumber evictPage(BM_BufferPool *const bm){
    switch (bm->strategy){
        case RS_FIFO:
            return evictFifo(bm);
    }
    return -1;
}

static PageNumber evictFifo(BM_BufferPool *const bm){

    Node* temp = list->head;
    Node* prev = NULL;
    PageTable *pageTable = getPageTable(bm);

    // find page whose fixCount is 0
    while (temp != NULL && temp->entry->fixCount>0) {
        prev = temp;
        temp = temp->next;
    }

    // If the node is not found
    if (temp == NULL) {
        return -1;
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

    // store pageNum in variable
    PageNumber pageNum = temp->entry->pageNum;

    // Free the memory occupied by the deleted node
    free(temp);
    return pageNum;
}


// generic public function to add page according to strategy
void admitPage(BM_BufferPool *const bm, PageEntry *entry){
    switch (bm->strategy){
        case RS_FIFO:
            admitFifo(entry);
            break;
    }
}

static void admitFifo(PageEntry *entry){
    insertAtEnd(list, entry);
}