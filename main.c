#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int i;
    int l = 0;
    int a = 0;
    int v = 0;

    for (i = 0; i < argc; i++)
    {
      if (strcmp(argv[i], "-l") == 0)
        l = 1;
      if (strcmp(argv[i], "-a") == 0)
        a = 1;
      if (strcmp(argv[i], "-v") == 0)
        v = 1;
    }

    scanner(l);
    parser(a);
    vm(v);



    return 0;
}
