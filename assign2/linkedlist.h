#include <stdio.h>
#include "buffer_mgr.h"
#include "page_table.h"

typedef struct Node {
    PageEntry *entry;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail;
} LinkedList;

extern Node* createNode(PageEntry * entry);
extern void insertAtBeginning(LinkedList* list, PageEntry * entry);
extern void insertAtEnd(LinkedList* list, PageEntry * entry);
extern void deleteNode(LinkedList* list, PageNumber pageNum);
extern void displayList(LinkedList* list);