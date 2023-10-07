#include <stdio.h>
#include <stdlib.h>

#include "replacement_strategy.h"
#include "linkedlist.h"

LinkedList* list = NULL;
int k = 1; // LRU_K's k

void initReplacementStrategy(BM_BufferPool *const bm, void* stratData){
    // setting k of LRU_K if any
    if(bm->strategy==RS_LRU_K){
        if(stratData==NULL || *((int *)stratData)<1){
            k=1;
        } else {
            k = *((int *)stratData);
        }
    }
    //
    switch (bm->strategy){
        case RS_FIFO:
        case RS_LRU:
        case RS_LRU_K:
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
            return evictFromHead(bm);
        case RS_LRU_K:
            if(k==1){
                return evictFromHead(bm);
            }
    }
    return -1;
}

static PageNumber evictFromHead(BM_BufferPool *const bm){

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
        case RS_LRU:
            insertAtEnd(list, entry);
            break;
        case RS_LRU_K:
            if(k==1){
                insertAtEnd(list, entry);
            }
    }
}


// generic public function to reorder page according to strategy
void reorderPage(BM_BufferPool *const bm, PageEntry *entry){
    switch (bm->strategy){
        case RS_FIFO:
            break;
        case RS_LRU:
            reorderLru(bm, entry);
            break;
        case RS_LRU_K:
            if(k==1){
                reorderLru(bm, entry);
            }
    }
}

static void reorderLru(BM_BufferPool *const bm, PageEntry *entry){
    deleteNode(list, entry->pageNum);
    insertAtEnd(list, entry);
    displayList(list);
}

void clearStrategyData(BM_BufferPool *const bm){
    free(list);
    list = NULL;
    k = 1;
}