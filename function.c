#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "function.h"
#include "preAssembler.h"

/* Constant strings for addressing methods*/
const char IMMEDIATE_ADDRESS[] = "0001";
const char DIRECT_ADDRESS[] = "0010";
const char INDIRECT_HOARD_ADDRESS[] = "0100";
const char DIRECT_HOARD_ADDRESS[] = "1000";
const char ABSOLUTE[] = "100";
const char RELOCATABLE[] = "010";
const char EXTERNAL[] = "001";
const char ZERO[] = "0000";
const char NOT_FOUND[] = "labbelnotfound!";

extern char *strdup(const char *);

Symbol *symbols = NULL;
int symbolCount = 0;

SymbolTable *symbolTable = NULL;
int labelCount = 0;

External *externs = NULL;
int externCount = 0;

L *l = NULL;

int IC = 0;
int DC = 0;

/* Adds data to the symbol table.
   Returns 1 if the data is successfully added, 0 otherwise. */

int addData(const char *line, int countLine, const char *newSymbolName)

{
    int countWord = 0;
    char codeBin[CODE_SEGMENT_SIZE];
    char *lineCopy;
    if (!line)
    {
        fprintf(stderr, "Error line %d: Num is missing\n", countLine);
        return 0;
    }
    else
    {
        lineCopy = strdup(line);
        if (lineCopy != NULL && correctCommas(lineCopy))
        {
            char *token = strtok(lineCopy, ", \t\n");
            while (token != NULL)
            {
                if (token && isNumber(token))
                {
                    int value = atoi(token);
                    strcpy(codeBin, decimalToBinary(value, CODE_SEGMENT_SIZE - 1));
                    addSymbol(newSymbolName, NULL, codeBin, 1, 0, 0); /* Add the data word*/
                    newSymbolName = NULL;
                    countWord++; /* add 1 L*/
                }
                else
                {
                    fprintf(stderr, "Error line %d: %s not a number \n", countLine, token);
                    return 0;
                }
                token = strtok(NULL, ", \t\n");
            }
        }
        else
        {
            fprintf(stderr, "Error line %d: Incorrect signs\n", countLine);
            return 0;
        }
    }
    DC += countWord;
    addL(countLine, countWord);
    return 1;
}

/* Adds a string to the symbol table.

   Returns 1 if the string is successfully added, 0 otherwise. */
int addString(const char *line, int countLine, const char *newSymbolName)
{
    int countWord = 0;
    char codeBin[CODE_SEGMENT_SIZE];
    char *lineCopy;

    if (!line)
    {
        fprintf(stderr, "Error line %d: string is missing\n", countLine);
        return 0;
    }
    else
    {
        lineCopy = strdup(line);
        char *token = strtok(lineCopy, " \t\n");
        if (isCorrectString(token) == 0)
        {
            fprintf(stderr, "Error line %d: Missing quotation marks.\n", countLine);
            return 0;
        }
        else
        {
            int length = strlen(token);
            char charToAdd = token[1];
            int value = charToAdd;                                          /* ASCII value*/
            strcpy(codeBin, decimalToBinary(value, CODE_SEGMENT_SIZE - 1)); /* Create binary code*/
            addSymbol(newSymbolName, NULL, codeBin, 1, 0, 0);               /* Add the first word*/
            countWord++;

            for (int i = 2; i < length - 1; i++)
            {
                charToAdd = token[i];
                value = charToAdd;                                              /* ASCII value*/
                strcpy(codeBin, decimalToBinary(value, CODE_SEGMENT_SIZE - 1)); /* Create binary code*/
                addSymbol(NULL, NULL, codeBin, 1, 0, 0);                        /* Add the next character*/
                countWord++;
            }

            strcpy(codeBin, decimalToBinary(0, CODE_SEGMENT_SIZE - 1)); /* Create binary code for the null terminator*/
            addSymbol(NULL, NULL, codeBin, 1, 0, 0);                    /* Add the null terminator*/
            countWord++;

            token = strtok(NULL, " \t\n");
            if (token)
            {
                fprintf(stderr, "Error line %d: Additional characters at the line.\n", countLine);
                return 0;
            }
        }
        DC += countWord;
        addL(countLine, countWord);
        return 1;
    }
}

/* Processes an external definition line.

   Returns 1 if the external definition is successful, 0 otherwise. */
int externDefinition(const char *line, int countLine)
{
    char *remaining = strdup(line);
    char *lineCopy;
   
        lineCopy = strdup(line);
        if (remaining && correctCommas(remaining))
        {
            char *token = strtok(lineCopy, " \t\n");
            while (token != NULL)
            {
                if (isNameRestricted(token))
                {
                    fprintf(stderr, "Error line %d:: %s is restricted name.\n", countLine, token);
                    return 0;
                }
                else
                {
                    addExtern(token); /* Add the external symbol*/
                    token = strtok(NULL, " \t\n");
                }
            }
            return 1;
        }
        else
        {
            fprintf(stderr, "Error line %d: the Extern definition is incorrect.\n", countLine);
            return 0;
        }
}

/* Processes an entry definition line.

   Returns 1 if the entry definition is successful, 0 otherwise. */
int entryDefinition(const char *line, int countLine, int isFirstPass)
{
    char *lineCopy;
    if (!line)
    {
        lineCopy = strdup(line);
        char *token = strtok(lineCopy, " \t\n");
        if (token)
        {
            if (findLabel(token) + 1)
            {
                Symbol *sym = &symbols[findLabel(token)];
                if (sym && sym->isEntry == 0)
                {
                    sym->isEntry = 1; /* Mark the symbol as an entry point*/
                }
            }
            else if (isFirstPass == 0)
            { /* Check for label existence only in the second pass*/
                fprintf(stderr, "Error line %d: label not exist.\n", countLine);
                return 0;
            }

            token = strtok(NULL, " \t\n");
            if (token)
            {
                fprintf(stderr, "Error line %d: Additional characters at the line.\n", countLine);
                return 0;
            }
        }
        token = strtok(NULL, " \t\n");
        if (token)
        {
            fprintf(stderr, "Error line %d: Additional characters at the line.\n", countLine);
            return 0;
        }
    }
    return 1;
}

/* Adds an operation to the symbol table based on the given line and operation number.

   Returns 1 if the operation is successfully added, 0 otherwise. */
int addOperation(const char *line, int numOper, int countLine, const char *newSymbolName)
{
    char *lineCopy;

    if (line)
    {
        lineCopy = strdup(line);
    }
    else
    {
        lineCopy = NULL;
    }

    if ((!line) || (lineCopy && correctCommas(lineCopy)))
    {
        if (numOper < 5)
        { /* Group 1 operations (2 operands)*/
            if (!operationWithTwoOperand(lineCopy, newSymbolName, numOper, countLine))
            {
                free(lineCopy);
                return 0;
            }
        }
        else if (numOper < 14)
        { /* Group 2 operations (1 operand)*/
            if (!operationWithOneOperand(lineCopy, newSymbolName, numOper, countLine))
            {
                free(lineCopy);
                return 0;
            }
        }
        else
        { /* Group 3 operations (0 operands)*/
            if (!operationWithoutOperand(lineCopy, newSymbolName, numOper, countLine))
            {
                free(lineCopy);
                return 0;
            }
        }
    }
    else
    {
        fprintf(stderr, "line %d:.\n", countLine);
        free(lineCopy);
        return 0;
    }
    free(lineCopy);
    return 1;
}

/* Updates the operands in the symbol table based on the given line and address.

   Returns 1 if the operands are successfully updated, 0 otherwise. */
int updateOparand(const char *line, int countLine, int countAdress)
{
    char *lineCopy;

    if (line)
    {
        lineCopy = strdup(line);
    }
    else
    {
        lineCopy = NULL;
    }

    char *token = strtok(lineCopy, " ,\t\n");
    char codeBin[CODE_SEGMENT_SIZE];
    int i = 1;

    while (token != NULL)
    {
        int symbolAddress = countAdress + i;

        if (strcmp(symbols[symbolAddress].code, NOT_FOUND) == 0)
        { /* Operand not found in the first pass*/
            Symbol *sym = &symbols[symbolAddress];
            strcpy(codeBin, miunOperand(token, 1));         /* Generate machine code for the operand*/
            strncpy(sym->code, codeBin, CODE_SEGMENT_SIZE); /* Update the symbol's code*/

            if (findExtern(token))
            {
                sym->isExtern = 1; /* Mark the symbol as external*/
                strncpy(sym->externName, token, MAX_LABEL_NAME);
            }

            if (strcmp(codeBin, NOT_FOUND) == 0)
            {
                fprintf(stderr, "Error line %d: label not found.\n", countLine);
                return 0;
            }
        }
        i++;
        token = strtok(NULL, " ,\t\n");
    }
    return 1;
}

/* Changes the file name extension to the specified ending.

   Returns a dynamically allocated string containing the updated file name. */
char *changeNameOfFile(const char *sourceFileName, const char *fileNameEnding)
{
    char *updateFileName = malloc(MAX_LINE_LENGTH); /* Allocate memory for the new file name*/
    char *dot = strrchr(sourceFileName, '.');       /* Find the last dot in the original file name*/
    strcpy(updateFileName, sourceFileName);         /* Copy the original file name*/
    dot = strrchr(updateFileName, '.');
    if (dot != NULL)
    {
        *dot = '\0'; /* Remove the original file name extension*/
    }
    strcat(updateFileName, fileNameEnding); /* Append the new file name extension*/
    return updateFileName;
}

/* Performs an operation with two operands.

   Returns 1 if the operation is successful, 0 otherwise. */
int operationWithTwoOperand(const char *line, const char *newSymbolName, int numOper, int countLine)
{
    char codeBin[CODE_SEGMENT_SIZE];
    char operand1[MAX_LINE_LENGTH];
    char operand2[MAX_LINE_LENGTH];
    char *lineCopy;

    if (line)
    {
        lineCopy = strdup(line);
    }
    else
    {
        lineCopy = NULL;
    }

    char *token = strtok(lineCopy, " ,\t\n");
    int countWord = 0;

    if (token)
    {
        strcpy(operand1, token);
        token = strtok(NULL, " ,\t\n");
    }
    else
    {
        fprintf(stderr, "Error line %d: Missing operand1.\n", countLine);
        return 0;
    }

    if (token)
    {
        strcpy(operand2, token);
    }
    else
    {
        fprintf(stderr, "Error line %d: Missing operand2.\n", countLine);
        return 0;
    }
    strcpy(codeBin, miunOperation(numOper, operand1, operand2)); /* Create code for the operation*/
    addSymbol(newSymbolName, NULL, codeBin, 0, 0, 0);            /* Add operation to the symbols*/
    countWord++;
    if (isRegister(operand1) + 1 && isRegister(operand2) + 1)
    {
        strcpy(codeBin, miunTwoRegister(operand1, operand2)); /* Create code for registers*/
        addSymbol(NULL, NULL, codeBin, 0, 0, 0);
        countWord++;
    }
    else
    {
        strcpy(codeBin, miunOperand(operand1, 1)); /* Create code for operand1*/
        if (findExtern(operand1))
        {
            addSymbol(NULL, operand1, codeBin, 0, 0, 1); /* Add symbol with operand1*/
            countWord++;
        }
        else
        {
            addSymbol(NULL, NULL, codeBin, 0, 0, 0); /* Add symbol with operand1*/
            countWord++;
        }
        strcpy(codeBin, miunOperand(operand2, 0)); /* Create code for operand2*/
        if (findExtern(operand2))
        {
            addSymbol(NULL, operand2, codeBin, 0, 0, 1); /* Add symbol with operand2*/
            countWord++;
        }
        else
        {
            addSymbol(NULL, NULL, codeBin, 0, 0, 0); /* Add symbol with operand2*/
            countWord++;
        }
    }

    token = strtok(NULL, " \t\n");
    if (token)
    {
        fprintf(stderr, "Error line %d: Additional operands.\n", countLine);
        return 0;
    }

    IC += countWord;
    addL(countLine, countWord);
    return 1;
}

/* Performs an operation with one operand.

   Returns 1 if the operation is successful, 0 otherwise. */
int operationWithOneOperand(const char *line, const char *newSymbolName, int numOper, int countLine)
{
    char codeBin[CODE_SEGMENT_SIZE];
    char operand1[MAX_LINE_LENGTH];
    char *lineCopy;

    if (line)
    {
        lineCopy = strdup(line);
    }
    else
    {
        lineCopy = NULL;
    }

    char *token = strtok(lineCopy, " ,\t\n");
    int countWord = 0;

    if (token)
    {
        strcpy(operand1, token);
    }
    else
    {
        fprintf(stderr, "Error line %d: Missing operand.\n", countLine);
        return 0;
    }

    strcpy(codeBin, miunOperation(numOper, NULL, operand1)); /* Create code for the operation*/
    addSymbol(newSymbolName, NULL, codeBin, 0, 0, 0);        /* Add operation to the symbols*/
    countWord++;
    strcpy(codeBin, miunOperand(operand1, 1)); /* Create code for operand1*/
    if (findExtern(operand1))
    {
        addSymbol(NULL, operand1, codeBin, 0, 0, 1); /* Add symbol with operand1*/
        countWord++;
    }
    else
    {
        addSymbol(NULL, NULL, codeBin, 0, 0, 0); /* Add symbol with operand1*/
        countWord++;
    }

    token = strtok(NULL, " \t\n");
    if (token)
    {
        fprintf(stderr, "Error line %d: Additional operands.\n", countLine);
        return 0;
    }

    IC += countWord;
    addL(countLine, countWord);
    return 1;
}

/* Performs an operation without operands.

   Returns 1 if the operation is successful, 0 otherwise. */
int operationWithoutOperand(const char *line, const char *newSymbolName, int numOper, int countLine)
{
    char codeBin[CODE_SEGMENT_SIZE];
    int countWord = 0;

    strcpy(codeBin, miunOperation(numOper, NULL, NULL)); /* Create code for the operation*/
    addSymbol(newSymbolName, NULL, codeBin, 0, 0, 0);    /* Add operation to the symbols*/
    countWord++;
    if (line)
    {
        fprintf(stderr, "Error line %d: Additional operands.\n", countLine);
        return 0;
    }

    IC += countWord;
    addL(countLine, countWord);
    return 1;
}

/* Checks if a label refers to a data location.

   Returns 1 if the label is data, 0 otherwise. */
int checkIfLabelIsData(const char *label)
{
    int labelAddress = findLabel(label);
    if (labelAddress + 1)
    {
        Symbol *sym = &symbols[labelAddress];
        if (sym->isData == 0)
        {
            return 0;
        }
    }
    return 1;
}

/* Checks if a name is a restricted keyword.

   Returns 1 if the name is restricted, 0 otherwise. */
int isNameRestricted(const char *name)
{
    if (isRegister(name) + 1)
    {
        return 1; /* Register*/
    }
    else if (findOperation(name) + 1)
    {
        return 1; /* Operation*/
    }
    else if (findMacro(name))
    {
        return 1; /* Macro*/
    }
    else if (findLabel(name) + 1)
    {
        return 1; /* Label*/
    }
    return 0; /* Not restricted*/
}

/* Finds the index of a given operation in the list of operations.

   Returns the index of the operation, or -1 if not found. */
int findOperation(const char *name)
{
    const char *operations[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"};
    int restrictedNamesCount = 16;
    for (int i = 0; i < restrictedNamesCount; i++)
    {
        if (strcmp(name, operations[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

/* Checks if a name is a register.

   Returns the index of the register in the list of registers, or -1 if not found. */
int isRegister(const char *name)
{
    const char *registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    int registerNum = 8;
    char temp[strlen(name) + 1];
    strcpy(temp, name);

    if (temp[0] == '*')
    {
        memmove((char *)temp, temp + 1, strlen(temp));
    }
    for (int i = 0; i < registerNum; i++)
    {
        if (strcmp(temp, registers[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

/* Determines the addressing method for a given operand.

   Returns a string representing the addressing method, or NULL if the operand is invalid. */
char *findsMethodOfAddressing(const char *operand)
{
    if (operand)
    {
        if (operand[0] == '#')
        { /* Immediate value*/
            if (!isNumber(operand + 1))
            {
                return NULL; /* Invalid immediate value*/
            }
            return strdup(IMMEDIATE_ADDRESS);
        }
        else if (findLabel(operand) + 1)
        { /* Label*/
            return strdup(DIRECT_ADDRESS);
        }
        else if (findExtern(operand))
        { /* External label*/
            return strdup(DIRECT_ADDRESS);
        }
        else if ((isRegister(operand) + 1) && (operand[0] == '*'))
        { /* Indirect addressing through a register*/
            return strdup(INDIRECT_HOARD_ADDRESS);
        }
        else if (isRegister(operand) + 1)
        { /* Register*/
            return strdup(DIRECT_HOARD_ADDRESS);
        }
        else
        { /* Label not defined yet (assuming it will be defined later)*/
            return strdup(DIRECT_ADDRESS);
        }
    }
    return strdup(ZERO); /* Operand is NULL*/
}

/* Performs a subtraction operation on two operands and returns the corresponding machine code.

   The operands can be registers, labels, or immediate values. */
char *miunOperation(int num, const char *operand1, const char *operand2)
{
    static char codeBin[CODE_SEGMENT_SIZE];
    char are[ARE_SIZE];

    char *opCode = decimalToBinary(num, CODE_SIZE);
    strcpy(are, ABSOLUTE);
    char *addressingMethod1 = findsMethodOfAddressing(operand1);
    char *addressingMethod2 = findsMethodOfAddressing(operand2);

    snprintf(codeBin, CODE_SEGMENT_SIZE, "%s%s%s%s", opCode, addressingMethod1, addressingMethod2, are);
    return codeBin;
}

/* Generates machine code for a subtraction operation with a single operand.

   The operand can be a register, label, or immediate value. */
char *miunOperand(const char *operand, int firstOperand)
{
    static char codeBin[CODE_SEGMENT_SIZE];
    char address[CODE_EE_SIZE];
    char addressingMethod[CODE_SIZE];
    char are[ARE_SIZE];

    if (operand[0] == '#')
    { /* Immediate value*/
        char num[strlen(operand) - 1];
        strncpy(num, &operand[1], strlen(operand) - 1);
        num[strlen(operand) - 1] = '\0';
        int value = atoi(num);
        strcpy(address, decimalToBinary(value, CODE_SIZE * 3));
        strcpy(are, ABSOLUTE);
    }
    else if (findLabel(operand) + 1)
    { /* Label*/
        Symbol *sym = &symbols[findLabel(operand)];
        int value = sym->address;
        strcpy(address, decimalToBinary(value, CODE_SIZE * 3));
        strcpy(are, RELOCATABLE);
    }
    else if (findExtern(operand))
    { /* External label*/
        strcpy(address, decimalToBinary(0, CODE_SIZE * 3));
        strcpy(are, EXTERNAL);
    }
    else if (isRegister(operand) + 1)
    { /* Register*/
        int num = isRegister(operand);
        strcpy(addressingMethod, decimalToBinary(num, CODE_SIZE));
        if (firstOperand)
        {
            snprintf(address, sizeof(address), "%s%s%s", ZERO, addressingMethod, ZERO);
        }
        else
        {
            snprintf(address, sizeof(address), "%s%s%s", ZERO, ZERO, addressingMethod);
        }
        strcpy(are, ABSOLUTE);
    }
    else
    { /* Invalid operand*/
        return strdup(NOT_FOUND);
    }

    snprintf(codeBin, CODE_SEGMENT_SIZE, "%s%s", address, are);
    return codeBin;
}

/* Subtracts two registers and returns the corresponding machine code.

   The operands are assumed to be register names. */
char *miunTwoRegister(const char *operand1, const char *operand2)
{
    static char codeBin[CODE_SEGMENT_SIZE];
    char addressingMethod1[CODE_SIZE + 1];
    char addressingMethod2[CODE_SIZE + 1];

    int num = isRegister(operand1);
    strcpy(addressingMethod1, decimalToBinary(num, CODE_SIZE));
    num = isRegister(operand2);
    strcpy(addressingMethod2, decimalToBinary(num, CODE_SIZE));

    snprintf(codeBin, CODE_SEGMENT_SIZE, "%s%s%s%s", ZERO, addressingMethod1, addressingMethod2, ABSOLUTE);
    return codeBin;
}

/* Adds a symbol to the symbol table.

   The symbol's name, external name, machine code, data flag, entry flag, and external flag are stored. */
void addSymbol(const char *symbolName, const char *externName, const char *codeBin, const int isData, const int isEntry, const int isExtern)
{
    Symbol *temp = realloc(symbols, (symbolCount + 1) * sizeof(Symbol));
    if (!temp)
    {
        fprintf(stderr, "Error Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    symbols = temp;
    symbolCount++;

    if (symbolName != NULL)
    {
        strncpy(symbols[symbolCount - 1].symbol, symbolName, MAX_LINE_LENGTH - 1);
        symbols[symbolCount - 1].symbol[MAX_LINE_LENGTH - 1] = '\0';
    }
    else
    {
        symbols[symbolCount - 1].symbol[0] = '\0';
    }

    if (externName != NULL)
    {
        strncpy(symbols[symbolCount - 1].externName, externName, MAX_LABEL_NAME - 1);
        symbols[symbolCount - 1].externName[MAX_LABEL_NAME - 1] = '\0';
    }
    else
    {
        symbols[symbolCount - 1].externName[0] = '\0';
    }

    strncpy(symbols[symbolCount - 1].code, codeBin, CODE_SEGMENT_SIZE - 1);
    symbols[symbolCount - 1].code[CODE_SEGMENT_SIZE - 1] = '\0';

    /* The address calculation might need to be adjusted based on the specific assembly language and memory model*/
    symbols[symbolCount - 1].address = symbolCount + 99;

    symbols[symbolCount - 1].isData = isData;
    symbols[symbolCount - 1].isEntry = isEntry;
    symbols[symbolCount - 1].isExtern = isExtern;

    if (symbolName != NULL)
    {
        addLabel(symbolName, symbolCount - 1); /* Add the label to the symbol table*/
    }
}

/* Adds a label to the symbol table.

   The label's name and its count are stored in the symbol table. */
void addLabel(const char *name, const int count)
{
    SymbolTable *temp = realloc(symbolTable, (labelCount + 1) * sizeof(SymbolTable));
    if (!temp)
    {
        fprintf(stderr, "Error Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    symbolTable = temp;
    labelCount++;

    symbolTable[labelCount - 1].name = strdup(name);
    if (!symbolTable[labelCount - 1].name)
    {
        fprintf(stderr, "Error Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    symbolTable[labelCount - 1].count = count;
}

/* Adds an external symbol to the list of external symbols.

   The external symbol's name is stored in the list. */
void addExtern(const char *name)
{
    External *temp = realloc(externs, (externCount + 1) * sizeof(External));
    if (!temp)
    {
        fprintf(stderr, "Error Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    externs = temp;
    externCount++;

    externs[externCount - 1].name = strdup(name);
    if (!externs[externCount - 1].name)
    {
        fprintf(stderr, "Error Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
}

/* Adds a line count to the list of line counts.

   The line count's word count is stored in the list. */
void addL(int counteLine, int wordCounter)
{
    L *temp = realloc(l, counteLine * sizeof(L));
    if (!temp)
    {
        fprintf(stderr, "Error Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    l = temp;
    l[counteLine - 1].wordCounter = wordCounter;
}

/* Checks if a token represents a label definition and returns the label name if so.

   Returns a dynamically allocated string containing the label name, or NULL if the token is not a label definition. */
char *labelDefinition(const char *token)
{
    int length = strlen(token);

    if (token[length - 1] == ':')
    {
        /* Create a new string for the label name */
        char *newSymbolName = malloc(length);
        if (!newSymbolName)
        {
            fprintf(stderr, "Error Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        /* Copy the label name without the colon */
        strncpy(newSymbolName, token, length - 1);
        newSymbolName[length - 1] = '\0';
        return newSymbolName;
    }
    return NULL;
}

/* Finds a label in the symbol table by its name.

   Returns the index of the label in the symbol table, or -1 if not found. */
int findLabel(const char *symbolName)
{
    for (int i = 0; i < symbolCount; i++)
    {
        if (strcmp(symbols[i].symbol, symbolName) == 0)
        {
            return i;
        }
    }
    return -1;
}

/* Finds an external symbol by its name.

   Returns the name of the external symbol if found, or NULL if not found. */
char *findExtern(const char *externName)
{
    for (int i = 0; i < externCount; i++)
    {
        if (strcmp(externs[i].name, externName) == 0)
        {
            return externs[i].name;
        }
    }
    return NULL;
}

/* Checks if a string is enclosed in double quotes.

   Returns 1 if the string is enclosed in double quotes, 0 otherwise. */
int isCorrectString(const char *name)
{
    if (name[0] == '"' && name[strlen(name) - 1] == '"')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* Checks if a string represents a valid number.

   Returns 1 if the string is a number, 0 otherwise.

   Handles both positive and negative numbers. */
int isNumber(const char *str)
{
    /* Check for empty string */
    if (*str == '\0')
    {
        return 0;
    }

    /* Optional: Handle negative numbers */
    if (*str == '-' || *str == '+')
    {
        str++;
    }

    /* Iterate through the string, checking if each character is a digit */
    while (*str != '\0')
    {
        if (!isdigit(*str))
        {
            return 0;
        }
        str++;
    }

    return 1;
}

/* Converts a decimal number to its binary representation.

   Returns a dynamically allocated string containing the binary representation.
   The length parameter specifies the desired number of bits in the binary representation. */
char *decimalToBinary(int decimal, int length)
{
    char *binary = (char *)malloc(length + 1); /* Allocate memory for the binary string */

    if (!binary)
    {
        fprintf(stderr, "Error Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    binary[length] = '\0'; /* Null-terminate the string */

    unsigned int mask = 1U << (length - 1); /* Create a mask for extracting bits */

    /* Handle negative numbers by converting to two's complement */
    if (decimal < 0)
    {
        decimal = (1 << length) + decimal;
    }

    /* Iterate through the bits and set the corresponding characters in the binary string */
    for (int i = 0; i < length; i++)
    {
        binary[i] = (decimal & mask) ? '1' : '0';
        mask >>= 1; /* Shift the mask to the right */
    }

    return binary;
}

/* Converts a binary string to its decimal equivalent.

   Returns the decimal value. */
int binaryToDecimal(const char *binary)
{
    int decimal = 0;
    int length = strlen(binary);

    /* Iterate through the binary string, starting from the least significant bit */
    for (int i = 0; i < length; i++)
    {
        /* If the current bit is 1, add its corresponding power of 2 to the decimal value */
        if (binary[length - 1 - i] == '1')
        {
            decimal += pow(2, i);
        }
    }
    return decimal;
}

/* Correct commas in a string representing a list of objects.
   Returns 1 if the commas are correctly placed, 0 otherwise.

   Assumes that:
   - Objects are separated by commas.
   - There are no spaces within objects.
   - The string does not end with a comma. */
int correctCommas(char *str)
{
    int expecting_object = 1; /* Flag to indicate whether we expect to find a new object */

    while (*str)
    {
        if (*str == ',' && expecting_object)
        {
            /* Encountered a comma before finding an object, which is an error */
            return 0;
        }
        else if (*str == ',' && !expecting_object)
        {
            /* Encountered a comma after finding an object, which is valid.
               Now expect to find a new object. */
            expecting_object = 1;
        }
        else if (!isspace(*str))
        {
            /* Found a non-whitespace character, which is part of an object.
               Now expect to find a comma. */
            expecting_object = 0;
        }
        str++;
    }

    /* If we reach the end of the string and are still expecting an object,
       it means there was an extra comma at the end, which is an error. */
    return 1;
}

/* The function frees the defined memory */
void freeMemory()
{
    /* Free memory for macros */
    free(macros);

    /* Free allocated memory for symbols */
    free(symbolTable);

    /* Free allocated memory for externs */
    free(externs);

    /* Free allocated memory for symbols (assuming symbols is a pointer to a data structure) */
    free(symbols);

    /* Free allocated memory for the number lines counter each symbole take */
    free(l);
}