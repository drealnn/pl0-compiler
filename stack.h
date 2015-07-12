#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

typedef struct StackNode{

    struct StackNode* next;
    char val;

}stackNode;

void push(stackNode** head, char val);
char pop(stackNode** head);
char peek(stackNode* head);

#endif // STACK_H_INCLUDED
