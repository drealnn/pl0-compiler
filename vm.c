#include <stdio.h>
#include <stdlib.h>


#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3


typedef struct instruction {

    int op;
    int l;
    int m;
    int lineNmbr;

}Instruction;

// Linked list structure used to mark the indexes of activation records

typedef struct StackNode{

    struct StackNode* next;
    int val;

}stacknode;

//--------------GLOBAL VARIABLES----------------------------

int sp = 0;
int bp = 1;
int pc = 0;
Instruction ir;

int instructionCount = 0;
int halt = 0;
int printTrace;

FILE* out;

Instruction code[MAX_CODE_LENGTH];
int stack[MAX_STACK_HEIGHT];
int arMarkers[MAX_LEXI_LEVELS];


stacknode *arMarkerList = NULL;

const char *opStringAry[] = {"lit","opr","lod","sto","cal","inc","jmp","jpc","sio"};

//---------------FUNCTION DECLARATIONS-----------------------

void readInput(FILE* in);
void instructionDecode();
void oprExecute(int m);
void printStack();
int base(int b, int l);

// push and pop remove/create nodes at the end of the LL while peek looks at the first node
// functions used to manipulate the LL storing the activation record markers
void pushi(stacknode** head, int val);
int popi(stacknode** head);
int peeki(stacknode* head);

//---------------MAIN----------------------------------------

int vm(int v)
{
    printTrace = v;
    int j;

    FILE* fptr = fopen("mcode.txt", "r");
    out = fopen("stacktrace.txt", "w");

    stack[1] = 0;
    stack[2] = 0;
    stack[3] = 0;

    for (j = 0; j < MAX_LEXI_LEVELS; j++)
        arMarkers[j] = -1;

    if (fptr == NULL || out == NULL)
    {
        printf("Could not read file \'mcode.txt\' or could not write to \'stacktrace.txt\'.\n");
        system("PAUSE");
        return 0;
    }

    readInput(fptr);

   // printf("%24s%6s%6s%9s\n", "pc", "bp", "sp", "stack");
   // printf("%-22s%-6d%-6d%-6d\n", "Initial values", pc, bp, sp);
    fprintf(out, "%24s%6s%6s%9s\n", "pc", "bp", "sp", "stack");
    fprintf(out, "%-22s%-6d%-6d%-6d\n", "Initial values", pc, bp, sp);


    while ( !halt /*&& pc < (instructionCount + 3)*/ )
    {
        ir = code[pc];
        pc = pc + 1;
        instructionDecode();
       // if (!halt)
            printStack();
    }

    //printf("arMarkerList:%d\n", peek(arMarkerList) );

    fclose(fptr);
    fclose(out);
    free(arMarkerList);

    return 0;
}

//---------------FUNCTIONS-------------------------------------

void readInput(FILE* in)
{
    // reads in the instructions from the input file into the code array
    // writes formated instruction set onto output file
    int op, l, m;
    int i = 0;

   // printf("%-6s%-6s%-6s%-6s\n", "Line", "OP", "L", "M");
    fprintf(out, "%-6s%-6s%-6s%-6s\n", "Line", "OP", "L", "M");
   // fscanf(in, "%d %d %d", &op, &l, &m);

    while(fscanf(in, "%d %d %d", &op, &l, &m) != EOF)
    {
        code[i].op = op;
        code[i].l = l;
        code[i].m = m;
        code[i].lineNmbr = i;
        fprintf(out, "%3d%6s%4d%6d\n", i, opStringAry[op - 1], l, m);

        if (printTrace)
            printf("%3d%6s%4d%6d\n", i, opStringAry[op - 1], l, m);

        i++;
    }

    instructionCount = i - 1;



}

int base(int b, int l)
{
    // given a base pointer, returns a new base pointer l levels down
   while(l > 0)
   {
       b = stack[b + 2]; // dynamic link from current activation record
       l--;
   }

   return b;
}

void instructionDecode()
{
    int op = ir.op;
    int m = ir.m;
    int l = ir.l;

    int i = 0;

    switch(op)
    {

    //lit
    case(1):
        sp++;               // push the stack
        stack[sp] = ir.m;   // load value m onto the stack
        break;
    //opr
    case(2):
        oprExecute(ir.m);
        break;
    case(3):
    //"lod";
        sp++;                                       // push the stack
        stack[sp] = stack[base(bp, ir.l) + ir.m];   // load from l levels down at index m
        break;
    case(4):
    //"sto";
        stack[base(bp, ir.l) + ir.m] = stack[sp];   // store value l levels down to index m
        sp--;                                       // pop the stack
        break;
    case(5):
    //"cal";
        //Creating a new activation record
       // while (arMarkers[i] != -1)
       //     i++;
       // arMarkers[i] = sp+1;

        pushi(&arMarkerList, sp + 1);

        //printf("New AR at:%d\n", peek(arMarkerList));

        stack[sp+1] = 0;                            // function return
        stack[sp+2] = base(bp, ir.l);               // dynamic link
        stack[sp+3] = bp;                           // static link
        stack[sp+4] = pc;                           // return address
        bp = sp + 1;                                // base pointer of new activation record
        pc = ir.m;                                  // code block for procedure
        break;
    case(6):
    //"inc";
        sp += ir.m;                                 // increment the stack by m
        break;
    case(7):
    //"jmp";
        pc = ir.m;                                  // jump to pc address m
        break;
    case(8):
    //"jpc";
        if (stack[sp] == 0)                         // if the value at the stack is 0
            pc = ir.m;                              // jump to address m
        sp--;                                       // pop the stack
        break;
    case(9):
    //"sio";
        switch(m)
        {
                                                    // pop the stack and output to console
            case(0):
               printf("%d\n", stack[sp]);
               sp--;
               break;
            case(1):
                                                    // grab input from user and push unto stack
                sp++;
                scanf("%d", &stack[sp]);
                break;
            case(2):
                                                    // halt the program
                halt = 1;
                break;
            default:
                printf("Unknown M value for instruction \'SIO\', halting program.\n");
                halt = 1;
                break;
        }
        break;

    default:
         printf("Unknown opcode, halting program.\n");
         halt = 1;
         break;
    }
}

void oprExecute(int m)
{
    switch(m)
        {
            case(0):
            //return
                sp = bp - 1;
                pc = stack[sp + 4];
                bp = stack[sp + 3];
                popi(&arMarkerList);

                break;
            case(1):
            //neg
                stack[sp] = -stack[sp];
                break;
            case(2):
            //add
                sp--;
                stack[sp] = stack[sp] + stack[sp + 1];
                break;
            case(3):
            //sub
                sp--;
                stack[sp] = stack[sp] - stack[sp + 1];
                break;
            case(4):
                sp--;
                stack[sp] = stack[sp] * stack[sp + 1];
                break;
            case(5):
                sp--;
                stack[sp] = stack[sp] / stack[sp + 1];
                break;
            case(6):
                //odd
                stack[sp] = stack[sp] % 2;
                break;
            case(7):
                sp--;
                stack[sp] = stack[sp] % stack[sp + 1];
                break;
            case(8):
                sp--;
                stack[sp] = stack[sp] == stack[sp + 1];
                break;
            case(9):
                sp--;
                stack[sp] = stack[sp] != stack[sp + 1];
                break;
            case(10):
                sp--;
                stack[sp] = stack[sp] < stack[sp + 1];
                break;
            case(11):
                sp--;
                stack[sp] = stack[sp] <= stack[sp + 1];
                break;
            case(12):
                sp--;
                stack[sp] = stack[sp] > stack[sp + 1];
                break;
            case(13):
                sp--;
                stack[sp] = stack[sp] >= stack[sp + 1];
                break;
            default:
                printf("Unknown M value for instruction \'OPR\', halting program.\n");
                halt = 1;
                break;

        }
}

void printStack()
{
    int i;

    //int c = 0;
    stacknode * tmp = arMarkerList;

    if (printTrace)
        printf("%-6d%-6s%-4d%-6d%-6d%-6d%-6d", ir.lineNmbr, opStringAry[ir.op-1], ir.l, ir.m, pc, bp, sp);

    fprintf(out, "%-6d%-6s%-4d%-6d%-6d%-6d%-6d", ir.lineNmbr, opStringAry[ir.op-1], ir.l, ir.m, pc, bp, sp);
    for (i = 1; i < sp + 1; i++)
    {
        /*
        if (c < MAX_LEXI_LEVELS && arMarkers[c] == i)
        {
             printf("| ");
             fprintf(out, "| ");
             c++;
        }
        */

        if (peeki(tmp) == i)
        {
           // printf("| ");
            fprintf(out, "| ");
            tmp = tmp->next;
           // pop(&arMarkerList);
        }

        if (printTrace)
            printf("%d ", stack[i]);

        fprintf(out, "%d ", stack[i]);
    }
   if (printTrace)
        printf("\n");

    fprintf(out, "\n");
}


// create a node at the end of the list
void pushi(stacknode** head, int val)
{
    if (*head == NULL)
    {
        *head = (stacknode *)malloc(sizeof(stacknode));
        (*head)->val = val;
        (*head)->next = NULL;
        return;
    }

    stacknode *node = (stacknode *)malloc(sizeof(stacknode));

    if (node == NULL){
        printf("Error with malloc, getting null.\n");
        return;
    }

    node->next = NULL;
    stacknode * tmp = *head;

    while (tmp->next != NULL)
        tmp = tmp->next;

    node->val = val;
    tmp->next = node;
}

// remove a node from the end of the list
int popi(stacknode** head)
{
    int val;
    if (*head == NULL)
        return -1;

    if ( (*head)->next == NULL)
    {
        val = (*head)->val;
        *head = NULL;
    }
    else
    {
        stacknode* tmp = *head;
        while ( tmp->next->next != NULL)
            tmp = tmp->next;

        val = tmp->next->val;
        stacknode* tmp2 = tmp->next;
        tmp->next = NULL;
        free(tmp2);
    }

    return val;
}

// return the value stored at the beginning of the list
int peeki(stacknode* head)
{
    if (head == NULL)
        return -1;
    return head->val;
}


