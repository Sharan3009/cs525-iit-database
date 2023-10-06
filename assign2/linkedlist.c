#include <stdio.h>
#include <stdlib.h>

#include "linkedlist.h"
#include "buffer_mgr.h"

// Create a new node with given data
Node* createNode(PageEntry * entry) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->entry = entry;
    newNode->next = NULL;
    return newNode;
}

// Insert a new node at the beginning of the linked list
void insertAtBeginning(LinkedList* list, PageEntry * entry) {
    Node* newNode = createNode(entry);
    if (list->head == NULL) {
        list->head = list->tail = newNode;
    } else {
        newNode->next = list->head;
        list->head = newNode;
    }
}

// Insert a new node at the end of the linked list
void insertAtEnd(LinkedList* list, PageEntry * entry) {
    Node* newNode = createNode(entry);
    if (list->head == NULL) {
        list->head = list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}

// Delete a node with given data from the linked list
void deleteNode(LinkedList* list, PageNumber pageNum) {
    Node* temp = list->head;
    Node* prev = NULL;

    while (temp != NULL && temp->entry->pageNum != pageNum) {
        prev = temp;
        temp = temp->next;
    }

    // If the node is not found
    if (temp == NULL) {
        return;
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

    // Free the memory occupied by the deleted node
    free(temp);
}

// Display the elements of the linked list
void displayList(LinkedList* list) {
    Node* temp = list->head;
    while (temp != NULL) {
        printf("%d -> ", temp->entry->pageNum);
        temp = temp->next;
    }
    printf("NULL\n");
}