#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

void push(stackNode** head, char val)
{
    if (*head == NULL)
    {
        *head = (stackNode *)malloc(sizeof(stackNode));

        if (*head == NULL){
        printf("Error creating stack, getting null node.\n");
        exit(1);
        }

        (*head)->val = val;
        (*head)->next = NULL;
        return;
    }

    stackNode *node = (stackNode *)malloc(sizeof(stackNode));

    if (node == NULL){
        printf("Error adding to stack, getting null node.\n");
        exit(1);
    }

    node->val = val;
    node->next = *head;
    (*head) = node;
}

// remove a node from the beginning of the list
char pop(stackNode** head)
{
    char val;
    if (*head == NULL)
        return '\0';

    val = (*head)->val;
    if ( (*head)->next == NULL)
    {
        *head = NULL;
    }
    else
    {
        *head = (*head)->next;
    }

    return val;
}

// return the value stored at the beginning of the list
char peek(stackNode* head)
{
    if (head == NULL)
        return '\0';
    return head->val;
}
