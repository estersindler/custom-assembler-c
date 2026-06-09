#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preAssembler.h"
#include "transition.h"
#include "function.h"

/* Checks if the number of command-line arguments is correct.

   Returns 1 if the number of arguments is correct, 0 otherwise. */
int numFileIsCoorect(int argc, char *argv[])
{
    int count = 1;
    while (argv[count] != NULL)
    {
        count++;
    }
    if (argc != (count))
    {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 0;
    }
    else
        return 1;
}

int main(int argc, char *argv[])
{
    if (numFileIsCoorect(argc, argv))
    {
        int i;
        for (i = 1; i < argc; i++)
        {
            processFile(argv[i]);
        }
    }
    freeMemory();
    return 0;
}
