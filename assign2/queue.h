#include "dt.h"
#include "buffer_mgr.h"

typedef struct Node {
    int data;
    struct Node* next;
} Node;

extern Node* initQueue();
extern void enqueue(Node* head, int data);
extern void dequeue(Node* head);