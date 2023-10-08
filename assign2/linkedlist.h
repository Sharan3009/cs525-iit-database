#include <stdio.h>
#include "buffer_mgr.h"
#include "page_table.h"

typedef struct Node {
    PageEntry *entry;
    struct Node* next;
    time_t priority;
    time_t time; // time of last occurence starting from k-1th
    int occurences;
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail;
} LinkedList;

extern Node* createNode(PageEntry * entry, int occurences, int priority, int time);
extern void insertAtBeginning(LinkedList* list, PageEntry * entry);
extern void insertAtEnd(LinkedList* list, PageEntry * entry);
extern Node* deleteNode(LinkedList* list, PageNumber pageNum);
extern Node* findNode(LinkedList* list, PageNumber pageNum);
extern void displayList(LinkedList* list);