#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "tokenEnum.h"

typedef struct {

    int class;
    char lexeme[11];

}token_t;

// constants indicating each token's type


static token_t token;
vector codeAry; // Dynamic array structure for storing clean code input
vector aryLexTable; // For lexemetable.txt
vector aryLexList; // For lexemelist.txt


void outputCleanTxt();          // creates cleaninput.txt and puts input into code array
void outputLexTable();          // creates lexemetable.txt
void outputLexList();          // creates lexemelist.txt
void lexemeTable();             // creates lexeme table by analyzing code array character by character through deterministic finite automaton
int isLetter(char myChar);      // checks if character is [A-Ba-B]
int isDigit(char myChar);       // checks if character is [0-9]
int isSpecial(char myChar);     // checks for special characters (e.g. ' ', '\n', '\t' ...)
void cleanLexeme();
char* IntToString(int number);     // Takes an integer and returns it in string format

int hasWordAtPos(char * subStr, int index); // checks codeAry for specified token


void scanner(int printLex)
{
    vectorInit(&codeAry);
    vectorInit(&aryLexTable);
    vectorInit(&aryLexList);

    outputCleanTxt();


    //printf("%s\n", codeAry.data);
    //printf("%d\n", hasWordAtPos("var", 0));

    lexemeTable();
    outputLexTable();
    outputLexList();
   // printf("%d\n", codeAry.size);
   // printf("%d\n", codeAry.capacity);
   //vectorFree(&codeAry);

   // return 0;
  
    if (printLex > 0)
      printf("%s\n", aryLexList.data);
  
}

void cleanLexeme()
{
  int i;
  for (i = 0; i < 11; i++)
  {
    token.lexeme[i] = '\0';
  }
}

void lexemeTable()
{
    int i, j;
    int state = 0;
    int last_index;

    i = 0;
    token.class = -1;
    cleanLexeme();

    // Setting up the header for lexemetable.txt
    vectorAppendString(&aryLexTable, "lexeme\t\ttoken type\n");
    // At the end of this function (lexemeTable()) the vector aryLexTable contains the character data for lexemetable.txt completely and correctly (as far as I know)

    //Something needs to be done to check that codeAry.data[i+n] isn't index out of bounds
    //before attempting to access it to avoid Index Out of Bounds exceptions or seg faults.


    vectorAppend(&codeAry, ' ');

    while (i < codeAry.size - 1)
    {
      last_index = i+1;

      // State 0: reserved tokens, State 1: identifiers, State 2: numbers
      switch (state)
      {
        case(0):

      // check for rel-ops-------------------------------

          if (codeAry.data[i] == '=')
          {
            token.class = eqlsym;
            token.lexeme[strlen(token.lexeme)] = '=';
          }
          else if (codeAry.data[i] == '<')
          {
            token.class = lessym;
            token.lexeme[strlen(token.lexeme)] = '<';

            // check next character for additional rel-ops
            if (codeAry.data[last_index] == '=')
            {
              token.class = leqsym;
              token.lexeme[strlen(token.lexeme)] = '=';
              i += 1;
            }
            else  if (codeAry.data[last_index] == '>')
            {
              token.class = neqsym;
              token.lexeme[strlen(token.lexeme)] = '>';
              i += 1;
            }

          }
          else if (codeAry.data[i] == '>')
          {
            token.class = gtrsym;
            token.lexeme[strlen(token.lexeme)] = '>';

            // check next character for additional rel-ops
            if (codeAry.data[last_index] == '=')
            {
              token.class = geqsym;
              token.lexeme[strlen(token.lexeme)] = '=';
              i += 1;
            }
          }

          // Check for Special Symbols -----------------------

          else if (codeAry.data[i] == ';')
          {
            token.class = semicolonsym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == ',')
          {
            token.class = commasym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == '.')
          {
            token.class = periodsym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == '(')
          {
            token.class = lparentsym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == ')')
          {
            token.class = rparentsym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == '+')
          {
            token.class = plussym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == '-')
          {
            token.class = minussym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == '*')
          {
            token.class = multsym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == '/')
          {
            token.class = slashsym;
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
          }
          else if (codeAry.data[i] == ':')
          {
            token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
            if (codeAry.data[last_index] == '=')
            {
              token.class = becomessym;
              token.lexeme[strlen(token.lexeme)] = codeAry.data[last_index];
              i += 1;
            }
          }

          else if (hasWordAtPos("begin", i))
          {
              strcat(token.lexeme, "begin");
              token.class = beginsym;
              i += 4;
          }
          else if (hasWordAtPos("call", i))
          {
              strcat(token.lexeme, "call");
              token.class = callsym;
              i += 3;
          }
          else if (hasWordAtPos("const", i))
          {
              strcat(token.lexeme, "const");
              token.class = constsym;
              i += 4;
          }
          else if (hasWordAtPos("do", i))
          {
              strcat(token.lexeme, "do");
              token.class = dosym;
              i += 1;
          }
          else if (hasWordAtPos("else", i))
          {
              strcat(token.lexeme, "else");
              token.class = elsesym;
              i += 3;
          }
          else if (hasWordAtPos("end", i))
          {
              strcat(token.lexeme, "end");
              token.class = endsym;
              i += 2;
          }
          else if (hasWordAtPos("if", i))
          {
              strcat(token.lexeme, "if");
              token.class = ifsym;
              i += 1;
          }
          else if (hasWordAtPos("odd", i))
          {
              strcat(token.lexeme, "odd");
              token.class = oddsym;
              i += 2;
          }
          else if (hasWordAtPos("procedure", i))
          {
              strcat(token.lexeme, "procedure");
              token.class = procsym;
              i += 8;
          }
          else if (hasWordAtPos("read", i))
          {
              strcat(token.lexeme, "read");
              token.class = readsym;
              i += 3;
          }
          else if (hasWordAtPos("then", i))
          {
              strcat(token.lexeme, "then");
              token.class = thensym;
              i += 3;
          }
          else if (hasWordAtPos("var", i))
          {
              strcat(token.lexeme, "var");
              token.class = varsym;
              i += 2;
          }
          else if (hasWordAtPos("while", i))
          {
              strcat(token.lexeme, "while");
              token.class = whilesym;
              i += 4;
          }
          else if (hasWordAtPos("write", i))
          {
              strcat(token.lexeme, "write");
              token.class = writesym;
              i += 4;
          }

          // Identifier---------------------------------------
          else if (isLetter(codeAry.data[i]))
          {
              token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
              token.class = identsym;
              state = 1;
              i++;
              break;
          }
          // Number-------------------------------------------
          else if (isDigit(codeAry.data[i]))
          {
              token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
              token.class = numbersym;
              state = 2;
              i++;
              break;
          }

          i++;

          //Token complete by here only when token.class != -1.

          //if (token.class > -1)
          //  printf("%d %s | ", token.class, token.lexeme);

        if (token.class > -1)
        {
          vectorAppendString(&aryLexTable, token.lexeme);
          vectorAppendString(&aryLexTable, "\t\t\t\t\t");
          vectorAppendString(&aryLexTable, IntToString(token.class));
          vectorAppend(&aryLexTable, '\n');

          vectorAppendString(&aryLexList, IntToString(token.class));
          vectorAppendString(&aryLexList, " ");
          if (token.class == 2 || token.class == 3)
          {
            vectorAppendString(&aryLexList, token.lexeme);
            vectorAppendString(&aryLexList, " ");
          }
        }
          token.class = -1;
          cleanLexeme();

          break;

        case(1):

            // check if identifier is too long
            if ( strlen(token.lexeme) >= 11 )
            {
                // print error message
                printf("COMPILE ERROR: Identifier too long.\n");
                exit(1);
                token.class = -1;
                cleanLexeme();
            }
             // if the next character is a letter or a digit, proceed to next character.
            else if ( isLetter(codeAry.data[i]) || isDigit(codeAry.data[i]) )
            {
                token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
                i++;
                break;
            }


        if (token.class > -1)
        {
          vectorAppendString(&aryLexTable, token.lexeme);
          vectorAppendString(&aryLexTable, "\t\t\t\t\t");
          vectorAppendString(&aryLexTable, IntToString(token.class));
          vectorAppend(&aryLexTable, '\n');

          vectorAppendString(&aryLexList, IntToString(token.class));
          vectorAppendString(&aryLexList, " ");
          if (token.class == 2 || token.class == 3)
          {
            vectorAppendString(&aryLexList, token.lexeme);
            vectorAppendString(&aryLexList, " ");
          }
        }
          token.class = -1;
          cleanLexeme();

          state = 0;

          break;

        case(2):
            if (isLetter(codeAry.data[i]))
            {
                // print error message
                printf("Letter in number.\n");
                token.class = -1;
                cleanLexeme();
            }
            else if (isDigit(codeAry.data[i]))
            {
                token.lexeme[strlen(token.lexeme)] = codeAry.data[i];
                i++;
                break;
            }

          //Token complete by here only when token.class != -1.

          //if (token.class > -1)
          //  printf("%d %s | ", token.class, token.lexeme);

        if (token.class > -1)
        {
          vectorAppendString(&aryLexTable, token.lexeme);
          vectorAppendString(&aryLexTable, "\t\t\t\t\t");
          vectorAppendString(&aryLexTable, IntToString(token.class));
          vectorAppend(&aryLexTable, '\n');

          vectorAppendString(&aryLexList, IntToString(token.class));
          vectorAppendString(&aryLexList, " ");
          if (token.class == 2 || token.class == 3)
          {
            vectorAppendString(&aryLexList, token.lexeme);
            vectorAppendString(&aryLexList, " ");
          }
        }


          token.class = -1;
          cleanLexeme();

          state = 0;

          break;


      } //End Switch

    } //End For

    vectorAppend(&aryLexList, '\0');
    vectorAppend(&aryLexTable, '\0');

}  //End Function


void outputCleanTxt()
{
    FILE* fptr = fopen("input.txt", "r");
    FILE* outPtr = fopen("cleaninput.txt", "w");

    char first, second;

    // error checking
    if (fptr == NULL || outPtr == NULL)
    {
        printf("Error reading input.txt or writing cleaninput.txt, shutting program down.\n");
        exit(1);
    }

    int commentFlag = 0;

    while (!feof(fptr)) // tests end of file
    {
        first = (char)fgetc(fptr);

        // if we encounter a forward slash, look ahead one character for comment indicator
        if (first == '/' && commentFlag == 0)
        {
            second = (char)fgetc(fptr);

            // if the second character is a star, we have a comment declaration.
            if (second == '*')
                commentFlag = 1;

            // else, append one or two characters to the char array
            else if (!feof(fptr))
            {
                vectorAppend(&codeAry, first);
                vectorAppend(&codeAry, second);
            }
            else
            {
                vectorAppend(&codeAry, first);
            }
        }

        else if (first == '*' && commentFlag == 1)
        {
            second = (char)fgetc(fptr);

            if (second == '/')
                commentFlag = 0;

        }

        else if (commentFlag == 0 && !feof(fptr))
        {
            vectorAppend(&codeAry, first);

        }

    }

    fprintf(outPtr, "%s", codeAry.data); // put our code data into the cleaninput text file

    fclose(fptr);
    fclose(outPtr);
}

void outputLexTable()
{
  FILE* outPtr = fopen("lexemetable.txt", "w");

  fprintf(outPtr, "%s", aryLexTable.data);

}

void outputLexList()
{
  FILE* outPtr = fopen("lexemelist.txt", "w");

  fprintf(outPtr, "%s", aryLexList.data);
  
  fclose(outPtr);
}

int hasWordAtPos(char * subStr, int index)
{
    int i = 0;
    while (subStr[i] != '\0')
    {
        if (subStr[i] != codeAry.data[index++])
            return 0;
        i++;
    }

    if ( isSpecial(codeAry.data[index]) )
        return 1;
    else
        return 0;

}

int isSpecial(char myChar)
{
    if (myChar >= 48 && myChar <= 58)
        return 0;
    else if (myChar >= 65 && myChar <= 90)
        return 0;
    else if (myChar >= 97 && myChar <= 122)
        return 0;

    else
        return 1;

}

int isDigit(char myChar)
{
    if (myChar >= 48 && myChar <= 57)
        return 1;
    else
        return 0;
}

int isLetter(char myChar)
{
    if ( (myChar >= 65 && myChar <= 90) || (myChar >= 97 && myChar <= 122) )
        return 1;
    else
        return 0;
}


char* IntToString(int number)
{
  char* str = malloc(sizeof(char) * 15);
  sprintf(str, "%d", number);

  return str;
}
