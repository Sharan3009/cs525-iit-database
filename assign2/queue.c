#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

Node* initQueue(){
    Node* queue = (Node*)malloc(sizeof(Node));
    return queue;
}

void enqueue(Node* head, int data){
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;

    Node* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

void dequeue(Node* head){
    Node* temp = head->next;
    head->next = temp->next;
    free(temp);
}