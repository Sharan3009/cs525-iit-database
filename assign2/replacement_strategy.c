#include <stdio.h>
#include <stdlib.h>

#include "replacement_strategy.h"
#include "linkedlist.h"

LinkedList* list = NULL;

void initReplacementStrategy(BM_BufferPool *const bm){

    switch (bm->strategy){
        case RS_FIFO:
        case RS_LRU:
            list = (LinkedList*)realloc(list, sizeof(LinkedList));
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
        case RS_LRU:
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

static PageNumber evictLru(BM_BufferPool *const bm){

}


// generic public function to add page according to strategy
void admitPage(BM_BufferPool *const bm, PageEntry *entry){
    switch (bm->strategy){
        case RS_FIFO:
        case RS_LRU:
            admitFifo(entry);
            break;
    }
}

static void admitFifo(PageEntry *entry){
    insertAtEnd(list, entry);
}

static void admitLru(PageEntry *entry){
    insertAtEnd(list, entry);
}


// generic public function to reorder page according to strategy
void reorderPage(BM_BufferPool *const bm, PageEntry *entry){
    switch (bm->strategy){
        case RS_FIFO:
            break;
        case RS_LRU:
            reorderLru(bm, entry);
            break;
    }
}

static void reorderLru(BM_BufferPool *const bm, PageEntry *entry){
    deleteNode(list, entry->pageNum);
    insertAtEnd(list, entry);
}