#include <stdio.h>
#include <stdlib.h>

#include "linkedlist.h"

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void insertAtBeginning(Node** head, int data) {
    Node* newNode = createNode(data);
    newNode->next = *head;
    *head = newNode;
}

void deleteNode(Node** head, int data) {
    Node* temp = *head;
    Node* prev = NULL;

    // Traverse the list to find the node to be deleted
    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    // If the node is not found
    if (temp == NULL) {
        return;
    }

    // Remove the node from the list
    if (prev == NULL) {
        *head = temp->next;
    } else {
        prev->next = temp->next;
    }

    // Free the memory occupied by the deleted node
    free(temp);
}

// Display the elements of the linked list
void displayList(Node* head) {
    while (head != NULL) {
        printf("%d -> ", head->data);
        head = head->next;
    }
    printf("NULL\n");
}