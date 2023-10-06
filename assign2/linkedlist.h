#include <stdio.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

extern Node* createNode(int data);
extern void insertAtBeginning(Node** head, int data);
extern void deleteNode(Node** head, int data);
extern void displayList(Node* head);