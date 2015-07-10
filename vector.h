#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED


#define VECTOR_CAPACITY 100

typedef struct
{
    int size;
    int capacity;
    char *data;

}vector;

void vectorInit(vector* vec);
void vectorAppend(vector* vec, char val);
void vectorAppendString(vector* vec, char* val);
char vectorGet(vector* vec, int index);
void vectorFree(vector* vec);
void printVector(vector* vec);

#endif // VECTOR_H_INCLUDED
