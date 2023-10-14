#include <stdio.h>
#include "buffer_mgr.h"
#include "page_table.h"
#include <limits.h>

typedef struct Node {
    PageEntry *entry;
    struct Node* next;
    long int bp; // backward k distance
    long long time; // time of last occurence starting from k-1th
    int occurences; // the k in LRU_K
    int frequency; // for LFU
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail;
} LinkedList;

extern Node* createNode(PageEntry * entry);
extern void insertAtBeginning(LinkedList* list, PageEntry * entry);
extern void insertAtEnd(LinkedList* list, PageEntry * entry);
extern Node* deleteNode(LinkedList* list, PageNumber pageNum);
extern Node* findNode(LinkedList* list, PageNumber pageNum);
extern void displayList(LinkedList* list);