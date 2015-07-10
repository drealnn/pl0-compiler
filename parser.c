#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenEnum.h"

#define MAX_SYMBOL_TABLE_SIZE 100
typedef struct symbol
{
    int kind; // const = 1, var = 2, proc = 3
    char name[12];
    int val;
    int level;
    int offset;

} symbol;

void program();
void block();
void statement();
void condition();
void expression();
void term();
void factor();

int getToken();
char * getIdentifier();
int getNumber();

FILE* fp;

symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];

int numOfSymbols;
int numOfVars;
int currentToken;

int parser()
{
  fp = fopen("lexemelist.txt", "r");


  if (fp == NULL)
  {
      printf("Could not open lexemelist.txt, exiting parser.\n");
      exit(1);
      return -1;

  }

  numOfSymbols = 0;
  numOfVars = 0;

  program();

  printf("Program is syntactically correct.\n");

  return 0;
}

int getToken()
{
    int token;
    fscanf(fp, "%d", &token);
    return token;
}

char * getIdentifier()
{
   char *ident;
   fscanf(fp, "%s", ident);
   return ident;
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

void program()
{
    currentToken = getToken();

    block();

    if (currentToken != periodsym)
        errorMSG("COMPILE ERROR: \'.\' expected at end of program");
}

void block()
{
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
           symbolTable[numOfSymbols++] = newSym;


           currentToken = getToken();


       }while (currentToken == commasym);

       if (currentToken != semicolonsym)
             errorMSG("COMPILE ERROR: semicolon expected after variable declaration");

       currentToken = getToken();
    }

    statement();

}

void statement()
{
    if (currentToken == identsym)
    {
        // get ident
        currentToken = getToken();
        if (currentToken != becomessym)
            errorMSG("COMPILER ERROR: assignment operator expected");
        currentToken = getToken();

        expression();

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
}
