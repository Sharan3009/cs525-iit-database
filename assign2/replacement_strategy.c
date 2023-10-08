#include <stdio.h>
#include <stdlib.h>

#include "replacement_strategy.h"
#include "linkedlist.h"

LinkedList* list = NULL;
int k = 1; // LRU_K's k
long long timer = 0;

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
        case RS_LRU_K:
            return evictFromHead(bm);
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
            admitLruK(entry);
    }
}

void admitLruK(PageEntry* entry){
    long long currTime = -1;
    if(k-1==1)
        currTime = timer++;
    Node* node = createNode(entry, 1, -1, currTime);

    Node* temp = list->head;
    Node* prev = NULL;

    if(temp==NULL){
        if (list->head == NULL) {
            list->head = list->tail = node;
        } else {
            node->next = list->head;
            list->head = node;
        }
        return;
    }
    while (temp != NULL && temp->priority ==-1) {
        prev = temp;
        temp = temp->next;
    }

    // reached end
    if(temp==NULL){
        prev->next = node;
        node->next = NULL;
    } else { // insert in between
        node->next = prev->next;
        prev->next = node;
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
            reorderLruK(bm, entry);
    }
}

static void reorderLru(BM_BufferPool *const bm, PageEntry *entry){
    free(deleteNode(list, entry->pageNum));
    insertAtEnd(list, entry);
}

static void reorderLruK(BM_BufferPool *const bm, PageEntry *entry){
    Node* node = deleteNode(list, entry->pageNum);
    node->occurences++;
    printf("pageNum=%d and occurences=%d and time=%lld\n",node->entry->pageNum, node->occurences, node->time);
    if(node->occurences>k)
        node->occurences = k;

    if(node->occurences==k-1){
        // storing time for the first time, but priority still stays -1
        node->time = timer++;
    } else if(node->occurences==k){
        long long prevTime = node->time; // it had a previous time
        if(prevTime!=-1){
            node->time = timer++;
            node->priority = node->time - prevTime;
        }
    }

    Node* temp = list->head;
    Node* prev = NULL;

    while (temp != NULL && temp->priority <= node->priority) {
        prev = temp;
        temp = temp->next;
    }

    // reached end
    if(temp==NULL){
        prev->next = node;
        node->next = NULL;
    } else { // insert in between
        node->next = prev->next;
        prev->next = node;
    }
}

void clearStrategyData(BM_BufferPool *const bm){
    free(list);
    list = NULL;
    k = 1;
    timer = 0;
}