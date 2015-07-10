#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

void vectorInit(vector* vec)
{
    vec->size = 0;
    vec->capacity = VECTOR_CAPACITY;
    vec->data = (char*)malloc(sizeof(char) * VECTOR_CAPACITY);

    if (vec->data == NULL)
    {
        printf("Error in vector initialization.\n");
    }
}

void vectorAppend(vector* vec, char val)
{
    if (vec->size >= vec->capacity)
    {
        vec->capacity *= 2;
        vec->data = (char*)realloc(vec->data, sizeof(char) * vec->capacity);

        if (vec->data == NULL)
        {
            printf("Error in vector reallocation.\n");
            return;
        }
    }

    vec->data[vec->size++] = val;

}

void vectorAppendString(vector* vec, char* val)
{
  int i;
  for (i = 0; val[i] != '\0'; i++)
  {
    vectorAppend(vec, val[i]);
  }
  
}

char vectorGet(vector* vec, int index)
{
    if (index < 0 || index >= vec->size)
    {
        printf("Invalid index\n");
        return '\0';
    }

    return vec->data[index];
}

void printVector(vector* vec)
{
    int i;
    for (i = 0; i < vec->size; i++)
    {
        printf("%c", vectorGet(vec, i));
    }
}

void vectorFree(vector* vec)
{
    vec->size = 0;
    vec->capacity = 0;
    free(vec);
}