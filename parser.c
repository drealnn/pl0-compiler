#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenEnum.h"
#include "stack.h"

#define MAX_SYMBOL_TABLE_SIZE 100
typedef struct symbol
{
    int kind; // const = 1, var = 2, proc = 3
    char name[12];
    int val;
    int level;
    int offset;

} symbol;

typedef struct instruction
{
    int op;
    int l;
    int m;

} instruction;

//-----FUNCTION DECLARATIONS----------------------------------

// parsing functions
void program();
void block();
void statement();
void condition();
void expression();
void term();
void factor();

// get various token types from the lexeme list
int getToken();
char* getIdentifier();
int getNumber();

// looks up a symbol in the symbol table via its identifier
symbol LookupSymbol(char* name);

void cleanArray();
void insertInst(char* op, int l, int m);
void dumpSymbolTable();
void executeStackLeftovers();
void outputAssembly(const char* fileName);

//-----GLOBAL VARIABLES----------------------------------------

FILE* fp;

symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
instruction assemblyCode[500];

// stack that performs postfix processing for arithmetic expressions
stackNode * expressionStack;

int numOfIns;
int numOfSymbols;
int numOfVars;
int currentToken;
char ident[12];

const char *opStringAry2[] = {"lit","opr","lod","sto","cal","inc","jmp","jpc","sio"};

//-----MAIN ---------------------------------------------------
int parser()
{
  fp = fopen("lexemelist.txt", "r");

  int i = 1;

  if (fp == NULL)
  {
      printf("Could not open lexemelist.txt, exiting parser.\n");
      exit(1);
      return -1;

  }


  numOfSymbols = 0;
  numOfVars = 0;
  numOfIns = 0;

  program();

  printf("Program is syntactically correct.\n");
  outputAssembly("mcode.txt");

  fclose(fp);

  return 0;
}

//-----HELPER FUNCTIONS-------------------------------------------------

void outputAssembly(const char* fileName)
{
    int i;
    FILE* fpOut = fopen(fileName, "w");
    for (i = 0; i < numOfIns; i++)
        fprintf(fpOut, "%d %d %d\n", assemblyCode[i].op, assemblyCode[i].l, assemblyCode[i].m);

    fclose(fpOut);


}

int getToken()
{
    int token;
    fscanf(fp, "%d", &token);
    return token;
}

char * getIdentifier()
{
   cleanArray(ident, 12);
   fscanf(fp, "%s", ident);
   return ident;
}

void cleanArray(char* input, int size)
{
  int i;

  for (i = 0; i < size; i++)
  {
    input[i] = '\0';
  }
}

int getNumber()
{
    int token;
    fscanf(fp, "%d", &token);
    return token;
}

int errorMSG(char * str)
{
    printf("%s.\n", str);
    exit(1);
}

symbol LookupSymbol(char* name)
{
   int i;
   for (i = 0; i < numOfSymbols; i++)
   {
       if (strcmp(name, symbolTable[i].name) == 0)
        return symbolTable[i];
   }


   //errorMSG("COMPILER ERROR: undeclared variable");
   symbol newSym;
   newSym.name[0] = '\0';
   return newSym;
}

void insertInst(char* op, int l, int m)
{
    int i;

    for (i = 0; i < 9; i++)
    {
        if (strcmp(op, opStringAry2[i]) == 0)
            break;
    }

    instruction newIns;
    newIns.op = i+1;
    newIns.l = l;
    newIns.m = m;

    assemblyCode[numOfIns++] = newIns;
}

void dumpSymbolTable()
{
    int i;
    printf("Number of symbols:%d, number of vars:%d\n",numOfSymbols, numOfVars);
    for (i = 0; i < numOfSymbols; i++)
    {
        printf("%s", symbolTable[i].name);
    }
}

void executeStackLeftovers()
{
    char curOp;
   while ( peek(expressionStack) != '\0' )
    {
       curOp = pop(&expressionStack);

       if (curOp == '+')
        insertInst("opr", 0, 2);
       else if (curOp == '-')
        insertInst("opr", 0, 3);
       else if (curOp == '*')
        insertInst("opr", 0, 4);
       else if (curOp == '/')
        insertInst("opr", 0, 5);

    }// end while
}

//-----PARSING FUNCTIONS----------------------------------------------

void program()
{
    // jump instruction to main? (will be needed for hw4)

    currentToken = getToken();

    block();

    if (currentToken != periodsym)
        errorMSG("COMPILE ERROR: \'.\' expected at end of program");

    // halt the program
    insertInst("sio", 0, 2);
}

void block()
{
    // constant declarations

    if (currentToken == constsym)
    {
       do
       {
           symbol newSym;

           currentToken = getToken();
           if (currentToken != identsym)
            errorMSG("COMPILE ERROR: identifier expected after constant declaration");

           newSym.kind = 1;
           strcpy(newSym.name, getIdentifier());
           newSym.level = 0;
           newSym.offset = -1;

           if (LookupSymbol(newSym.name).name[0] != '\0')
            errorMSG("COMPILER ERROR: Symbol has already been defined");

           currentToken = getToken();
           if (currentToken != eqlsym)
            errorMSG("COMPILE ERROR: \'=\' expected after constant identifier");

           currentToken = getToken();
           if (currentToken != numbersym)
            errorMSG("COMPILE ERROR: integer value expected for constant declaration");

           newSym.val = getNumber();
           symbolTable[numOfSymbols++] = newSym;

           currentToken = getToken();

       }while (currentToken == commasym);

       if (currentToken != semicolonsym)
             errorMSG("COMPILE ERROR: semicolon expected after constant declaration");

       currentToken = getToken();
    }

    // variable declarations

    if (currentToken == varsym)
    {
       do
       {
           symbol newSym;

           currentToken = getToken();
           if (currentToken != identsym)
            errorMSG("COMPILE ERROR: identifier expected after \'var\'");

           strcpy(newSym.name, getIdentifier());
           newSym.kind = 2;
           newSym.level = 0;
           newSym.offset = 4 + numOfVars++;

           if (LookupSymbol(newSym.name).name[0] != '\0')
            errorMSG("COMPILER ERROR: Symbol has already been defined");

           symbolTable[numOfSymbols++] = newSym;


           currentToken = getToken();


       }while (currentToken == commasym);

       if (currentToken != semicolonsym)
             errorMSG("COMPILE ERROR: semicolon expected after variable declaration");

       currentToken = getToken();


    }

    //dumpSymbolTable();

    // increment the stack pointer, including the initialized variables
       insertInst("inc", 0, 4 + numOfVars);

    // Nested procedure declaration
    // NOTE: code gen needed for HW4
    if (currentToken == procsym)
    {
      currentToken = getToken();
      if (currentToken != identsym)
        errorMSG("COMPILE ERROR: Identifier must follow procedure");
      getIdentifier();  //Do something with this
      currentToken = getToken();
      if (currentToken != semicolonsym)
        errorMSG("COMPILE ERROR: Semicolon expected following procedure declaration");
      currentToken = getToken();
      block();
      if (currentToken != semicolonsym)
        errorMSG("COMPILE ERROR: Semicolon expected");
      currentToken = getToken();
    }

    statement();

}

void statement()
{
    if (currentToken == identsym)
    {

        symbol newSym = LookupSymbol(getIdentifier());

        if (newSym.name[0] == '\0')
        {
            errorMSG("COMPILER ERROR: Undefined symbol");
        }

        if (newSym.kind != 2)
            errorMSG("COMPILER ERROR: Assignment to constant or procedure is not allowed");

        currentToken = getToken();
        if (currentToken != becomessym)
            errorMSG("COMPILER ERROR: assignment operator expected");
        currentToken = getToken();

        expression();
        executeStackLeftovers();

        //printf("successful expression calculation\n");

        // execute each operator left on the stack

        // store the value of the expression (at top of the stack) to the offset of newsym (STO)
        insertInst("sto", newSym.level, newSym.offset);


    }
    // NOTE: code gen needed for HW4
    else if (currentToken == callsym)
    {
      currentToken = getToken();
      if (currentToken != identsym)
        errorMSG("COMPILER ERROR: Identification symbol expected.");

      getIdentifier();  //Do something with this.

      currentToken = getToken();
    }
    else if (currentToken == beginsym)
    {
        currentToken = getToken();
        statement();

        while (currentToken == semicolonsym)
        {
            currentToken = getToken();
            statement();
        }

        if (currentToken != endsym)
            errorMSG("COMPILER ERROR: \'END\' not found");

        currentToken = getToken();
    }
    //TODO:
    else if (currentToken == ifsym)
    {
      currentToken = getToken();
      condition();
      if (currentToken != thensym)
        errorMSG("COMPILER ERROR: THEN not found");
      currentToken = getToken();
      statement();
    }
    // NOTE: code gen needed for HW4
    else if (currentToken == elsesym)
    {
      currentToken = getToken();
      statement();
    }

    else if (currentToken == writesym || currentToken == readsym)
    {
      int read = 0, write = 0;

      if (currentToken == writesym)
        write = 1;
      else
        read = 1;

      currentToken = getToken();
      if (currentToken != identsym)
        errorMSG("COMPILER ERROR: Read/Write must be followed by identifier.");

      symbol newSymbol = LookupSymbol(getIdentifier()); //Do something with it.

      // if write, output load variable to top of the stack, then output to console
      if (write)
      {
          if (newSymbol.kind == 1)
            insertInst("lit", 0, newSymbol.val);
          else
            insertInst("lod", 0, newSymbol.offset);

          insertInst("sio", 0, 0);
      }
      // if read, ask the user for input, then store that value to the location of the variable on the stack
      else
      {
          if (newSymbol.kind == 1)
            errorMSG("COMPILER ERROR: Cannot write to constant");

          insertInst("sio", 0, 1);
          insertInst("sto", 0, newSymbol.offset);
      }

      currentToken = getToken();
    }
    //TODO:
    else if (currentToken == whilesym)
    {
      currentToken = getToken();
      condition();
      if (currentToken != dosym)
        errorMSG("COMPILER ERROR: DO expected.");
      currentToken = getToken();
      statement();
    }

}

void condition()
{
  if (currentToken == oddsym)
  {
    currentToken = getToken();
    expression();
  }
  else
  {
    expression();
    if (relation(currentToken) < 1)
      errorMSG("COMPILER ERROR: Relation symbol expected.");
    currentToken = getToken();
    expression();
  }
}

int relation(int token)
{
  if (token > 8 && token < 15)
    return 1;
  else
    return 0;
}

void expression()
{
  char curOp;
  if (currentToken == plussym || currentToken == minussym)
    currentToken = getToken();

  term();

  while(currentToken == plussym || currentToken == minussym)
  {
    // while plus/minus/multi/divide is on stack, perform the operators on stack until we reach lower precedence '(' or '', then place current plus/minus on stack
    while ( peek(expressionStack) != '\0' && (peek(expressionStack) != '(' ) )
    {
       curOp = pop(&expressionStack);

       if (curOp == '+')
        insertInst("opr", 0, 2);
       else if (curOp == '-')
        insertInst("opr", 0, 3);
       else if (curOp == '*')
        insertInst("opr", 0, 4);
       else if (curOp == '/')
        insertInst("opr", 0, 5);

    }// end while

    if (currentToken == plussym)
        push(&expressionStack, '+');
    else
        push(&expressionStack, '-');

    currentToken = getToken();
    term();
  } // end while

}

void term()
{
  char curOp;
  factor();
  while (currentToken == multsym || currentToken == slashsym)
  {
    // while multi/divide is on stack, perform the operators, then place multi/divide on stack
    while ( peek(expressionStack) != '\0' && (peek(expressionStack) == '*'  || peek(expressionStack) == '/') )
    {
       curOp = pop(&expressionStack);

       if (curOp == '+')
        insertInst("opr", 0, 2);
       else if (curOp == '-')
        insertInst("opr", 0, 3);
       else if (curOp == '*')
        insertInst("opr", 0, 4);
       else if (curOp == '/')
        insertInst("opr", 0, 5);

    }// end while

    if (currentToken == multsym)
        push(&expressionStack, '*');
    else
        push(&expressionStack, '/');
    currentToken = getToken();
    factor();
  }
}

void factor()
{
  char curOp;
  if (currentToken == identsym)
  {
    // load variable or constant into the stack from the symbol table
    symbol newSym = LookupSymbol(getIdentifier()); //Do Something with this.

    if (newSym.name[0] == '\0')
        {
            errorMSG("COMPILER ERROR: Undefined symbol");
        }

    if (newSym.kind == 1)
        insertInst("lit", 0, newSym.val);
    else if (newSym.kind == 2)
        insertInst("lod", 0, newSym.offset);

    currentToken = getToken();
  }

  else if (currentToken == numbersym)
  {
    // load constant numerical value to the top of the stack
    insertInst("lit", 0, getNumber()); //Do something with this.
    currentToken = getToken();
  }
  else if (currentToken == lparentsym)
  {
    // push left paren onto the stack
    push(&expressionStack, '(');

    currentToken = getToken();
    expression();

    if (currentToken != rparentsym)
      errorMSG("COMPILER ERROR: Right parenthesis expected.");

    // pop the stack performing each operation until left paren is found
    while (peek(expressionStack) != '(')
    {
        curOp = pop(&expressionStack);

       if (curOp == '+')
        insertInst("opr", 0, 2);
       else if (curOp == '-')
        insertInst("opr", 0, 3);
       else if (curOp == '*')
        insertInst("opr", 0, 4);
       else if (curOp == '/')
        insertInst("opr", 0, 5);

    }// end while


    pop(&expressionStack);
    currentToken = getToken();

  }
  else
    errorMSG("COMPILER ERROR: Factor expected.");
}
