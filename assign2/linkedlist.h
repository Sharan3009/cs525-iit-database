#include <stdio.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail;
} LinkedList;

extern Node* createNode(int data);
extern void insertAtBeginning(LinkedList* list, int data);
extern void insertAtEnd(LinkedList* list, int data);
extern void deleteNode(LinkedList* list, int data);
extern void displayList(LinkedList* list);