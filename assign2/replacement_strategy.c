#include <stdio.h>
#include <stdlib.h>

#include "replacement_strategy.h"
#include "linkedlist.h"

LinkedList* list = NULL;
int k = 1; // LRU_K's k
long long timer = 0;
Clock* clock = NULL;

void initReplacementStrategy(BM_BufferPool *const bm, void* stratData){
    // setting k of LRU_K if any
    if(bm->strategy==RS_LRU_K){
        if(stratData==NULL || *((int *)stratData)<1){
            k=1;
        } else {
            k = *((int *)stratData);
        }
    }

    switch (bm->strategy){
        case RS_FIFO:
        case RS_LRU:
        case RS_LRU_K:
        case RS_LFU:
            list = (LinkedList*)realloc(list, sizeof(LinkedList));
            list->head = NULL;
            list->tail = NULL;
            break;
        case RS_CLOCK:
            clock = (Clock *)realloc(clock, sizeof(Clock));
            clock->hand = 0;
            clock->size = 0;
            clock->capacity = bm->numPages;
            clock->arr =  (ClockEntry*)malloc(clock->capacity*sizeof(ClockEntry));
            // Initialize clock entries to NULL
            for (int i = 0; i < clock->capacity; i++) {
                clock->arr[i].entry = NULL;
                clock->arr[i].referenceBit = 0;
            }
            break;
        default:
            printf("Replacement Strategy not defined=%d\n", bm->strategy);
            exit(1);
            break;
    }
}

// generic public function to get page according to strategy
PageNumber evictPage(BM_BufferPool *const bm){
    switch (bm->strategy){
        case RS_FIFO:
        case RS_LRU:
        case RS_LRU_K:
        case RS_LFU:
            return evictFromHead(bm);
        case RS_CLOCK:
            return evictFromClock(bm);
    }
    return -1;
}

static PageNumber evictFromHead(BM_BufferPool *const bm){

    Node* temp = list->head;
    Node* prev = NULL;
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

static PageNumber evictFromClock(BM_BufferPool *const bm){
    while(true){
        if(clock->arr[clock->hand].entry->fixCount==0 && clock->arr[clock->hand].referenceBit==0){
            PageNumber pageNum = clock->arr[clock->hand].entry->pageNum;
            clock->arr[clock->hand].entry = NULL;
            clock->hand = (clock->hand + 1) % clock->capacity;
            return pageNum;
        }
        clock->arr[clock->hand].referenceBit = 0;
        clock->hand = (clock->hand + 1) % clock->capacity;
    }
    return -1;
}


// generic public function to add page according to strategy
void admitPage(BM_BufferPool *const bm, PageEntry *entry){
    switch (bm->strategy){
        case RS_FIFO:
            insertAtEnd(list, entry);
            break;
        case RS_LRU:
        case RS_LRU_K:
            admitLruK(entry);
            break;
        case RS_LFU:
            admitLfu(entry);
            break;
        case RS_CLOCK:
            admitClock(entry);
            break;
    }
}

static void admitLruK(PageEntry* entry){
    long long currTime = -1;
    if(k-1==1)
        currTime = timer++;
    Node* node = createNode(entry);
    node->bp = LONG_MAX;
    node->time = currTime;

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
    while (temp != NULL && temp->bp == LONG_MAX) {
        prev = temp;
        temp = temp->next;
    }

    // Insert at the beginning
    if (prev == NULL) {
        node->next = list->head;
        list->head = node;
    }  else {  // Insert in between or at the end
        prev->next = node;
        node->next = temp;
    }

}

static void admitLfu(PageEntry* entry){
    Node* node = createNode(entry);
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
    while (temp != NULL && temp->frequency <= node->frequency) {
        prev = temp;
        temp = temp->next;
    }

    // Insert at the beginning
    if (prev == NULL) {
        node->next = list->head;
        list->head = node;
    }  else {  // Insert in between or at the end
        prev->next = node;
        node->next = temp;
    }
}

static void admitClock(PageEntry* entry){
    if(clock->size < clock->capacity){
        for(int i=0;i<clock->capacity;i++){
            if(clock->arr[i].entry==NULL){
                clock->arr[i].entry = entry;
                clock->arr[i].referenceBit = 1;
                clock->size++;
                return;
            }
        }
    }
    clock->arr[clock->hand].entry = entry;
    clock->arr[clock->hand].referenceBit = 1;
}


// generic public function to reorder page according to strategy
void reorderPage(BM_BufferPool *const bm, PageEntry *entry){
    switch (bm->strategy){
        case RS_FIFO:
            break;
        case RS_LRU:
        case RS_LRU_K:
            reorderLruK(bm, entry);
            break;
        case RS_LFU:
            reorderLfu(bm, entry);
            break;
        case RS_CLOCK:
            reorderClock(bm, entry);
            break;
    }
}

static void reorderLruK(BM_BufferPool *const bm, PageEntry *entry){
    Node* node = deleteNode(list, entry->pageNum);
    node->occurences++;
    if(node->occurences>k)
        node->occurences = k;

    if(node->occurences==k-1){
        // storing time for the first time, but bp still stays MAXIMUM
        node->time = timer++;
    } else if(node->occurences==k){
        long long prevTime = node->time; // it had a previous time
        if(prevTime!=-1){
            node->time = timer++;
            node->bp = node->time - prevTime;
        }
    }

    Node* temp = list->head;
    Node* prev = NULL;

    while (temp != NULL && temp->bp >= node->bp) {
        prev = temp;
        temp = temp->next;
    }

    // Insert at the beginning
    if (prev == NULL) {
        node->next = list->head;
        list->head = node;
    }  else {  // Insert in between or at the end
        prev->next = node;
        node->next = temp;
    }
}

static void reorderLfu(BM_BufferPool *const bm, PageEntry *entry){
    Node* node = deleteNode(list, entry->pageNum);
    node->frequency++;

    Node* temp = list->head;
    Node* prev = NULL;

    while (temp != NULL && temp->frequency <= node->frequency) {
        prev = temp;
        temp = temp->next;
    }

    // Insert at the beginning
    if (prev == NULL) {
        node->next = list->head;
        list->head = node;
    }  else {  // Insert in between or at the end
        prev->next = node;
        node->next = temp;
    }
}

static void reorderClock(BM_BufferPool *const bm, PageEntry *entry){
    for(int i=0;i<clock->capacity;i++){
        if(clock->arr[i].entry->pageNum == entry->pageNum){
            clock->arr[i].referenceBit = 1;
            break;
        }
    }
}

void clearStrategyData(BM_BufferPool *const bm){
    switch (bm->strategy){
        case RS_FIFO:
        case RS_LRU:
        case RS_LRU_K:
        case RS_LFU:
            // free list
            free(list);
            list = NULL;
            // reset k for lru-k
            k = 1;
            // reset timer for lfu
            timer = 0;
            break;
        case RS_CLOCK:
            // reset clock array
            free(clock);
            clock = NULL;
            break;
        default:
            printf("Replacement Strategy not defined=%d\n", bm->strategy);
            exit(1);
            break;
    }

}