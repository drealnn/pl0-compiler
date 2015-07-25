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

// looks up a symbol in the symbol table via its identifier.
//if the symbol matches the identifier and is less than(global) or equal (current scope) to the current level, return it.
symbol LookupSymbol(char* name, int level);

// if a symbol exists with the same ident and level (current scope), throw an error
void checkSymbol(char* name, int level);

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

int procStart[3];
int procCount = 0;
int numOfIns = 0;
int numOfSymbols;
int numOfVars;
int currentToken;
char ident[12];
int lexiLevel;

const char *opStringAry2[] = {"lit","opr","lod","sto","cal","inc","jmp","jpc","sio"};

int printAssembly = 0;

//-----MAIN ---------------------------------------------------
int parser(int a)
{
  printAssembly = a;
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

  printf("No errors, program is syntactically correct.\n");
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
    {
        fprintf(fpOut, "%d %d %d\n", assemblyCode[i].op, assemblyCode[i].l, assemblyCode[i].m);
        if (printAssembly > 0)
          printf("%d %d %d\n", assemblyCode[i].op, assemblyCode[i].l, assemblyCode[i].m);
    }
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

symbol LookupSymbol(char* name, int level)
{
   int i;
   int maxLevel = 0;
   symbol *tempSymbol = (symbol*)malloc(sizeof(symbol));
   tempSymbol->name[0] = '\0';
   for (i = 0; i < numOfSymbols; i++)
   {
       if (strcmp(name, symbolTable[i].name) == 0)
       {
           // check the symbol if its in a global or local domain
           if (symbolTable[i].level <= level)
           {
               // we want the symbol closest to local
               if (symbolTable[i].level >= maxLevel)
               {
                   maxLevel = symbolTable[i].level;
                   tempSymbol = &symbolTable[i];
               }
           }
         
           else
             tempSymbol = &symbolTable[i];
       }
   }


   //errorMSG("COMPILER ERROR: undeclared variable");
   //symbol newSym;
   //newSym.name[0] = '\0';
   return *tempSymbol;
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
    currentToken = getToken();

    lexiLevel = 0;

    block();

    if (currentToken != periodsym)
        errorMSG("COMPILE ERROR: \'.\' expected at end of program");

    // halt the program
    insertInst("sio", 0, 2);
}

void block()
{
    int procIns = 0;
    int currentProc = 0;
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
           newSym.level = lexiLevel;
           newSym.offset = -1;

           //if (LookupSymbol(newSym.name, lexiLevel).name[0] != '\0')
           // errorMSG("COMPILER ERROR: Symbol has already been defined");

           currentToken = getToken();
           if (currentToken != eqlsym)
            errorMSG("COMPILE ERROR: \'=\' expected after constant identifier");

           currentToken = getToken();
           if (currentToken != numbersym)
            errorMSG("COMPILE ERROR: integer value expected for constant declaration");

           newSym.val = getNumber();
           symbolTable[numOfSymbols++] = newSym; // modify insert for two-dimensional array

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
           newSym.level = lexiLevel;
           newSym.offset = 4 + numOfVars++;

           //if (LookupSymbol(newSym.name, lexiLevel).name[0] != '\0')
            //errorMSG("COMPILER ERROR: Symbol has already been defined");

           symbolTable[numOfSymbols++] = newSym; // modify insert for two-dimensional array


           currentToken = getToken();


       }while (currentToken == commasym);

       if (currentToken != semicolonsym)
             errorMSG("COMPILE ERROR: semicolon expected after variable declaration");

       currentToken = getToken();


    }


    // Nested procedure declaration
    // NOTE: code gen needed for HW4
    if (currentToken == procsym)
    {
      procCount++;
      if (numOfIns == 0)
        insertInst("jmp", 0, 0); // If m value is later corrected after procedure declarations...
      currentToken = getToken();
      if (currentToken != identsym)
        errorMSG("COMPILE ERROR: Identifier must follow procedure");

      symbol newSym;
      newSym.kind = 2;
      newSym.level = lexiLevel;
      strcpy(newSym.name, getIdentifier());
      newSym.offset = numOfIns + 1; // store the current address of the procedure here
      procIns = numOfSymbols;
      symbolTable[numOfSymbols++] = newSym;

      
      
      currentToken = getToken();
      if (currentToken != semicolonsym)
        errorMSG("COMPILE ERROR: Semicolon expected following procedure declaration");
      currentToken = getToken();
      //printf("Prior to Block Proc Count: %d for Proc %s\n", procCount, newSym.name);
      currentProc = procCount;
      lexiLevel++;

      block();
      // procCount is accurate total by here

      if (currentToken != semicolonsym)
        errorMSG("COMPILE ERROR: Semicolon expected");
      currentToken = getToken();
      
      insertInst("opr", 0, 0);
      
      lexiLevel--;
      
      // increment the stack pointer, including the initialized variables
      insertInst("inc", 0, 4 + numOfVars);
      if (assemblyCode[0].op == 7) //If there was a procedure, the first instruction is a jump,
        assemblyCode[0].m = numOfIns - 1;  //So correct it's m value to the beginning of the main procedure.
      
      if (currentProc == procCount)
      {
        symbolTable[procIns].offset = 2;
        procStart[0] = 2;
        procStart[1] = numOfIns;
      }
      else if (currentProc == procCount - 1)
      {
        procStart[2] = numOfIns;
        symbolTable[procIns].offset = procStart[1];
      } 
      else if (currentProc == procCount - 2)
      {
        symbolTable[procIns].offset = procStart[2];
      }

      statement();
      
      //printf("After Procedure: %s Offset: %d Current numOfIns: %d ProcCount: %d\n", symbolTable[procIns].name, symbolTable[procIns].offset, numOfIns, procCount);
      return;

    }

    // increment the stack pointer, including the initialized variables
    insertInst("inc", 0, 4 + numOfVars);
    if (assemblyCode[0].op == 7) //If there was a procedure, the first instruction is a jump,
      assemblyCode[0].m = numOfIns - 1;  //So correct it's m value to the beginning of the main procedure.
    statement();

}

void statement()
{
    if (currentToken == identsym)
    {

        symbol newSym = LookupSymbol(getIdentifier(), lexiLevel);

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
        // NOTE:
        // The closer the variable is to the procedure, the smaller the "l" parameter is.
        // for example, accessing a variable within the same activation record will have an "l" level of 0.
        insertInst("sto", lexiLevel - newSym.level, newSym.offset);


    }
    // NOTE: code gen needed for HW4
    else if (currentToken == callsym)
    {
      currentToken = getToken();
      if (currentToken != identsym)
        errorMSG("COMPILER ERROR: Identification symbol expected.");
      
      symbol newSym = LookupSymbol(getIdentifier(), lexiLevel);
      //if (newSym.name[0] == '\0')
      //  errorMSG("COMPILER ERROR: Procedure not identified");

      //printf("\nCalling Symbol %s val: %d level: %d offtset: %d \n", newSym.name, newSym.val, newSym.level, newSym.offset);
        
        /*
        int kind; // const = 1, var = 2, proc = 3
        char name[12];
        int val;
        int level;
        int offset;
        */
      
      // the procedure symbol's lexi level should point to the correct static link (hopefully)
      insertInst("cal", newSym.level, newSym.offset - 1);



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

        if (currentToken != endsym){
          printf("Token supposed to be end: %d\n", currentToken);
          errorMSG("COMPILER ERROR: \'END\' not found");}

        currentToken = getToken();
    }

    else if (currentToken == ifsym)
    {
      currentToken = getToken();
      condition();
      if (currentToken != thensym)
        errorMSG("COMPILER ERROR: THEN not found");
      currentToken = getToken();

     // if conditional fails, execute jump instruction skipping if body
      int jmpInsIndex = numOfIns;
      insertInst("jpc", 0, -1);

      statement();

      // update the jump instruction with the correct PC value pointing to the end of the "if" body.
      instruction jump = assemblyCode[jmpInsIndex];
      jump.m = numOfIns;
      assemblyCode[jmpInsIndex] = jump;

      // NOTE: code gen needed for HW4
      if (currentToken == elsesym)
      {
        currentToken = getToken();
        statement();
      }

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

      symbol newSymbol = LookupSymbol(getIdentifier(), lexiLevel); //Do something with it.

      // if write, output load variable to top of the stack, then output to console
      if (write)
      {
          if (newSymbol.kind == 1)
            insertInst("lit", lexiLevel - newSymbol.level, newSymbol.val);
          else
            insertInst("lod", lexiLevel - newSymbol.level, newSymbol.offset);

          insertInst("sio", 0, 0);
      }
      // if read, ask the user for input, then store that value to the location of the variable on the stack
      else
      {
          if (newSymbol.kind == 1)
            errorMSG("COMPILER ERROR: Cannot write to constant");

          insertInst("sio", 0, 1);
          insertInst("sto", lexiLevel - newSymbol.level, newSymbol.offset);
      }

      currentToken = getToken();
    }
    //TODO:
    else if (currentToken == whilesym)
    {
      currentToken = getToken();

      int conditionIndex = numOfIns; // index pointing to top of while loop

      condition();
      if (currentToken != dosym)
        errorMSG("COMPILER ERROR: DO expected.");
      currentToken = getToken();

      // if conditional fails, execute jump instruction skipping "while" body
      int jmpInsIndex = numOfIns;
      insertInst("jpc", 0, -1);

      statement(); // body of loop

      // add an unconditional jump to loop back to the start of the "while" body
      insertInst("jmp", 0, conditionIndex);

      // update the conditional jump instruction with the correct PC value pointing to the end of the "while" body.
      instruction jump = assemblyCode[jmpInsIndex];
      jump.m = numOfIns;
      assemblyCode[jmpInsIndex] = jump;
    }

}

void condition()
{
  if (currentToken == oddsym)
  {
    currentToken = getToken();
    expression();
    executeStackLeftovers();


    insertInst("opr", 0, 6);
    // if odd->1 on top of stack, else 0

  }
  else
  {
    expression();
    executeStackLeftovers();

    if (relation(currentToken) < 1)
      errorMSG("COMPILER ERROR: Relation symbol expected.");

    int currRelation = currentToken;

    currentToken = getToken();
    expression();
    executeStackLeftovers();

    // compare top two items (calculated expressions) in the stack
    switch (currRelation)
    {
    case(eqlsym):
        insertInst("opr", 0, 8);
        break;
    case(neqsym):
        insertInst("opr", 0, 9);
        break;
    case(lessym):
        insertInst("opr", 0, 10);
        break;
    case(leqsym):
        insertInst("opr", 0, 11);
        break;
    case(gtrsym):
        insertInst("opr", 0, 12);
        break;
    case(geqsym):
        insertInst("opr", 0, 13);
        break;
    }// end switch

  }// end if

}//end function

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
    symbol newSym = LookupSymbol(getIdentifier(), lexiLevel); //Do Something with this.

    if (newSym.name[0] == '\0')
        {
            errorMSG("COMPILER ERROR: Undefined symbol");
        }

    if (newSym.kind == 1)
        insertInst("lit", lexiLevel - newSym.level, newSym.val);
    else if (newSym.kind == 2)
        insertInst("lod", lexiLevel - newSym.level, newSym.offset);

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
