#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "transition.h"
#include "function.h"

/* Performs the first pass of the assembler, processing the source code and building the symbol table.

   Returns 1 if the first pass is successful, 0 otherwise. */
void firstPass(const char *sourceFilename)
{
    FILE *sourceFile = fopen(sourceFilename, "r");

    if (!sourceFile)
    {
        perror("Error opening input file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char lineToCheck[MAX_LINE_LENGTH];
    int uncorrect = 0;
    int countLine = 0;
    char *newSymbolName = NULL;

    while (fgets(line, MAX_LINE_LENGTH, sourceFile))
    {
        countLine++;
        strcpy(lineToCheck, line);
        char *token = strtok(lineToCheck, " \t\n");
        if (token && (token[0] != ';')) /* No comment*/
        {
            newSymbolName = labelDefinition(token); /* Check for label definition*/

            if (newSymbolName)
            { /* Label definition found*/
                if (isNameRestricted(newSymbolName))
                { /* Check if label name is restricted*/
                    fprintf(stderr, "Error line %d: %s is restricted name.\n", countLine, newSymbolName);
                    uncorrect = 1;
                }
                else
                {
                    token = strtok(NULL, " \t\n"); /* Get the next token*/
                }
            }

            if (!token && newSymbolName)
            {
                fprintf(stderr, "Error line %d: %s is empty label.\n", countLine, newSymbolName);
            }
            else if (token)
            {
                /* Data directive*/
                if (strcmp(token, ".data") == 0)
                {
                    char *remaining = strtok(NULL, "");
                    if (addData(remaining, countLine, newSymbolName) == 0)
                    {
                        uncorrect = 1;
                    }
                }
                else if (strcmp(token, ".string") == 0)
                { /* String directive*/
                    char *remaining = strtok(NULL, "");
                    if (addString(remaining, countLine, newSymbolName) == 0)
                    {
                        uncorrect = 1;
                    }
                }
                else if (strcmp(token, ".extern") == 0)
                { /* External directive*/
                    char *remaining = strtok(NULL, "");
                    if (!remaining)
                    {
                        fprintf(stderr, "Error line %d: Extern is missing.\n", countLine);
                    }
                    else if (externDefinition(remaining, countLine) == 0)
                    {
                        uncorrect = 1;
                    }
                    addL(countLine, 0); /* Add line count for external definition*/
                }
                else if (strcmp(token, ".entry") == 0)
                { /* Entry directive*/
                    char *remaining = strtok(NULL, "");
                    if (!remaining)
                    {
                        fprintf(stderr, "Error line %d: Entry is missing.\n", countLine);
                    }
                    else if (entryDefinition(remaining, countLine, 1) == 0)
                    {
                        uncorrect = 1;
                    }
                    addL(countLine, 0); /* Add line count for entry definition*/
                }
                else if (findOperation(token) + 1)
                { /* Operation*/
                    int numOper = findOperation(token);
                    char *remaining = strtok(NULL, "");
                    if (addOperation(remaining, numOper, countLine, newSymbolName) == 0)
                    {
                        uncorrect = 1;
                    }
                }
            }
            /* Uncorrect introduction*/
            else
            {
                fprintf(stderr, "Error line %d: introduction name incorrect.\n", countLine);
                uncorrect = 1;
            }
        }

        free(newSymbolName);
        newSymbolName = NULL;
    }

    fclose(sourceFile);

    if (uncorrect == 1)
    {
        fprintf(stderr, "Error Incorrect file was received.\n");
        exit(EXIT_FAILURE);
    }
}

/* Performs the second pass of the assembler, resolving labels and updating operands.

   Returns 1 if the second pass is successful, 0 otherwise. */
void secondPass(const char *sourceFilename)
{
    FILE *sourceFile = fopen(sourceFilename, "r");

    if (!sourceFile)
    {
        perror("Error opening input file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char lineToCheck[MAX_LINE_LENGTH];
    int uncorrect = 0;
    int countLine = 0;
    char *newSymbolName = NULL;
    int countAdress = 0;

    while (fgets(line, MAX_LINE_LENGTH, sourceFile))
    {
        strcpy(lineToCheck, line);
        countLine++;
        char *token = strtok(lineToCheck, " \t\n");

        /* Check for label definition*/
        if (token)
        {
            newSymbolName = labelDefinition(token);
            if (newSymbolName)
            {
                token = strtok(NULL, " \t\n"); /* Get the next token*/
            }
        }

        /* Check the first operation*/
        if (token)
        {
            /* Check if the line is a directive or comment*/
            if ((token[0] == ';') || (strcmp(token, ".data") == 0) || (strcmp(token, ".string") == 0) || (strcmp(token, ".extern") == 0))
            {
                continue;
            }
            else if (strcmp(token, ".entry") == 0)
            { /* Entry directive*/
                char *remaining = strtok(NULL, "");
                if (entryDefinition(remaining, countLine, 0) == 0)
                {
                    uncorrect = 1;
                }
            }
            else if (findOperation(token) + 1)
            { /* Operation*/
                char *remaining = strtok(NULL, "");
                if (updateOparand(remaining, countLine, countAdress) == 0)
                {
                    uncorrect = 1;
                }
            }
        }

        countAdress += l[countLine - 1].wordCounter;
    }

    if (uncorrect == 1)
    {
        fprintf(stderr, "Error Incorrect file was received.\n");
        exit(EXIT_FAILURE);
    }

    fclose(sourceFile);
}

/* Builds the output files (.ob, .ext, .ent) based on the symbol table and line counts.

   Returns 1 if the output files are created successfully, 0 otherwise. */
void buildOutputFiles(const char *sourceFilename)
{
    FILE *sourceFile = fopen(sourceFilename, "r");

    if (!sourceFile)
    {
        perror("Error opening input file");
        return;
    }

    /* Create .ob file*/
    char objectFilename[MAX_LINE_LENGTH];
    strcpy(objectFilename, changeNameOfFile(sourceFilename, ".ob"));

    FILE *obFile = fopen(objectFilename, "w");
    if (obFile == NULL)
    {
        perror("Error creating .o file");
        return;
    }

    /* Create .ext file*/
    char externFilename[MAX_LINE_LENGTH];
    strcpy(externFilename, changeNameOfFile(sourceFilename, ".ext"));

    FILE *extFile = fopen(externFilename, "w");
    if (extFile == NULL)
    {
        perror("Error creating .ext file");
        fclose(obFile);
        return;
    }

    int externExist = 0;

    /* Create .ent file*/
    char entryFilename[MAX_LINE_LENGTH];
    strcpy(entryFilename, changeNameOfFile(sourceFilename, ".ent"));

    FILE *entFile = fopen(entryFilename, "w");
    if (entFile == NULL)
    {
        perror("Error creating .ent file");
        return;
    }

    int entryExist = 0;

    /* Write to .ob file and .ext file*/
    fprintf(obFile, "%d \t %d \n", IC, DC);

    for (int i = 0; i < symbolCount; i++)
    {
        Symbol *sym = &symbols[i];
        int codeInDecimal = binaryToDecimal(sym->code);
        fprintf(obFile, "%04d \t %05o \n", sym->address, codeInDecimal);

        if (sym->isExtern)
        {
            externExist = 1;
            fprintf(extFile, "%s \t %04d \n", sym->externName, sym->address);
            fflush(extFile);
        }
    }

    /* Write to .ent file*/
    for (int i = 0; i < labelCount; i++)
    {
        Symbol *sym = &symbols[symbolTable[i].count];
        if (sym->isEntry)
        {
            entryExist = 1;
            fprintf(entFile, "%s \t %04d \n", sym->symbol, sym->address);
            fflush(entFile);
        }
    }

    fprintf(stderr, "The files were created successfully.\n");
    fclose(extFile);
    fclose(entFile);
    fclose(obFile);

    /* Remove .ent file if no entries were found*/
    if (!entryExist)
    {
        remove(entryFilename);
    }

    /* Remove .ext file if no externals were found*/
    if (!externExist)
    {
        remove(externFilename);
    }
}