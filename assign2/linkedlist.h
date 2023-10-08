#include <stdio.h>
#include "buffer_mgr.h"
#include "page_table.h"

typedef struct Node {
    PageEntry *entry;
    struct Node* next;
    long int priority;
    long long time; // time of last occurence starting from k-1th
    int occurences;
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail;
} LinkedList;

extern Node* createNode(PageEntry * entry, int occurences, long int priority, long long time);
extern void insertAtBeginning(LinkedList* list, PageEntry * entry);
extern void insertAtEnd(LinkedList* list, PageEntry * entry);
extern Node* deleteNode(LinkedList* list, PageNumber pageNum);
extern Node* findNode(LinkedList* list, PageNumber pageNum);
extern void displayList(LinkedList* list);