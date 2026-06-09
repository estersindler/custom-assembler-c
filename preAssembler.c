#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preAssembler.h"
#include "transition.h"
#include "function.h"

Macro *macros = NULL;
int macroCount = 0;
extern char *strdup(const char *);

/* Adds a new macro to the list of macros.

   Returns 1 if the macro is successfully added, 0 otherwise. */
void addMacro(const char *name, const char *content)
{
    macroCount++;
    macros = realloc(macros, macroCount * sizeof(Macro));

    if (macros == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    macros[macroCount - 1].name = strdup(name);
    macros[macroCount - 1].content = strdup(content);

    if (macros[macroCount - 1].name == NULL || macros[macroCount - 1].content == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
}

/* Finds a macro by its name.

   Returns the content of the macro if found, or NULL if not found. */
const char *findMacro(const char *name)
{
    for (int i = 0; i < macroCount; i++)
    {
        if (strcmp(macros[i].name, name) == 0)
        {
            return macros[i].content;
        }
    }

    return NULL;
}

/* Checks if a file is open and valid.

   Returns 1 if the file is open and valid, 0 otherwise. */
int checkingWhetherTheFileIsCorrect(FILE *checkedFile)
{
    if (!checkedFile)
    {
        perror("Error opening the file");
        fclose(checkedFile);
        return 0;
    }

    return 1;
}

/* Removes a file and exits the program.

   Returns 1 if the file is removed successfully, 0 otherwise. */
void removeFileAndExit(const char *outputFilename)
{
    if (remove(outputFilename) == 0)
    {
        exit(EXIT_FAILURE);
    }
}

/* Displays an error message and exits the program.

   Closes the output file and removes the output file if specified. */
void regetError(const char *errorLine, FILE *outputFile, char *outputFilename)
{
    fprintf(stderr, "%s \n", errorLine);

    if (outputFile && outputFilename)
    {
        fclose(outputFile);
        removeFileAndExit(outputFilename);
    }
}

/* Processes an assembly language file.

   Returns 1 if the processing is successful, 0 otherwise. */
void processFile(const char *inputFilename)
{
    char outputFilename[FILENAME_MAX];
    strcpy(outputFilename, inputFilename);

    /* Change the file extension to ".am"*/
    char *dot = strrchr(outputFilename, '.');
    if (dot && strcmp(dot, ".as") == 0)
    {
        *dot = '\0';
    }
    else
    {
        regetError("Error: file name is incorrect.", NULL, NULL);
        return;
    }

    strcat(outputFilename, ".am");

    FILE *inputFile = fopen(inputFilename, "r");
    if (!checkingWhetherTheFileIsCorrect(inputFile))
    {
        return;
    }

    FILE *outputFile = fopen(outputFilename, "w");
    if (!checkingWhetherTheFileIsCorrect(outputFile))
    {
        return;
    }

    char line[MAX_LINE_LENGTH];
    char lineToCheck[MAX_LINE_LENGTH];
    char macroName[MAX_MACRO_NAME];
    char *macroContent = NULL;
    size_t macroContentSize = 0;
    int insideMacro = 0;

    /* Iterate through the lines of the input file*/
    while (fgets(line, MAX_LINE_LENGTH, inputFile))
    {
        strcpy(lineToCheck, line);
        char *token = strtok(lineToCheck, " \t\n");

        /* Check for macro definition*/
        if (token && strcmp(token, "macr") == 0)
        {
            token = strtok(NULL, " \t\n");
            if (token)
            {
                /* Check if the macro name is valid*/
                if (findOperation(token) + 1)
                {
                    regetError("Error: Macro name restricted.", outputFile, outputFilename);
                    return;
                }
                else
                {
                    strcpy(macroName, token);
                    token = strtok(NULL, " \t\n");
                    if (token)
                    {
                        regetError("Error: Additional characters in the macro definition line.", outputFile, outputFilename);
                    }
                    else
                    {
                        /* Start macro definition*/
                        free(macroContent);
                        macroContent = NULL;
                        macroContentSize = 0;
                        insideMacro = 1;
                    }
                }
            }
            else
            {
                regetError("Error: Macro name missing.", outputFile, outputFilename);
            }
        }
        else if (token && strcmp(token, "endmacr") == 0)
        { /* End of macro definition*/
            if (insideMacro == 0)
            {
                regetError("Error: End macr without undefine macr.", outputFile, outputFilename);
            }
            token = strtok(NULL, " \t\n");
            if (token)
            {
                regetError("Error: Additional characters at the end.", outputFile, outputFilename);
            }

            if (macroContent == NULL)
            {
                regetError("Error: Memory allocation failed.", outputFile, outputFilename);
            }

            /* Add the macro to the symbol table*/
            addMacro(macroName, macroContent);
            insideMacro = 0;
        }
        else if (insideMacro)
        { /* Inside a macro definition*/
            size_t lineLength = strlen(line);
            macroContent = realloc(macroContent, macroContentSize + lineLength + 1);
            if (macroContent == NULL)
            {
                regetError("Error: Memory allocation failed.", outputFile, outputFilename);
            }
            strcpy(macroContent + macroContentSize, line);
            macroContentSize += lineLength;
        }
        else if (findMacro(token))
        {                                            /* Macro call*/
            fprintf(outputFile, "%s", macroContent); /* Expand the macro*/
        }
        else
        {
            /* Regular line*/
            fprintf(outputFile, "%s", line);
        }
    }

    if (insideMacro)
    {
        regetError("Error: Macro without end.", outputFile, outputFilename);
    }

    free(macroContent);
    fclose(inputFile);
    fclose(outputFile);

    /* Perform first and second passes*/
    firstPass(outputFilename);
    secondPass(outputFilename);

    /* Build output files*/
    buildOutputFiles(outputFilename);
}