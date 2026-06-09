#define MAX_LINE_LENGTH 81
#define MAX_LABEL_NAME 32
#define CODE_SEGMENT_SIZE 16
#define CODE_EE_SIZE 13
#define CODE_SIZE 4
#define ARE_SIZE 3

/* Constant strings for addressing methods*/
extern const char IMMEDIATE_ADDRESS[];
extern const char DIRECT_ADDRESS[];
extern const char INDIRECT_HOARD_ADDRESS[];
extern const char DIRECT_HOARD_ADDRESS[];
extern const char ABSOLUTE[];
extern const char RELOCATABLE[];
extern const char EXTERNAL[];
extern const char ZERO[];
extern const char NOT_FOUND[];

typedef struct
{
    char symbol[MAX_LINE_LENGTH];
    char code[CODE_SEGMENT_SIZE];
    char externName[MAX_LABEL_NAME];
    int address;
    int isData;
    int isEntry;
    int isExtern;
} Symbol;

extern Symbol *symbols;
extern int symbolCount;

typedef struct
{
    char *name;
    int count;
} SymbolTable;

extern SymbolTable *symbolTable;
extern int labelCount;

typedef struct
{
    char *name;
} External;

External *externs;
extern int externCount;

typedef struct
{
    int wordCounter;
} L;
extern L *l;

extern int IC;
extern int DC;

/* Adds data to the symbol table.*/
int addData(const char *line, int countLine, const char *newSymbolName);

/* Adds a string to the symbol table*/
int addString(const char *line, int countLine, const char *newSymbolName);

/* Processes an external definition line.*/
int externDefinition(const char *line, int countLine);

/* Processes an entry definition line.*/
int entryDefinition(const char *line, int countLine, int isFirstPass);

/* Adds an operation to the symbol table based on the given line and operation number.*/
int addOperation(const char *line, int numOper, int countLine, const char *newSymbolName);

/* Updates the operands in the symbol table based on the given line and address.*/
int updateOparand(const char *line, int countLine, int countAdress);

/* Changes the file name extension to the specified ending.*/
char *changeNameOfFile(const char *sourceFileName, const char *fileNameEnding);

/* Performs an operation with two operands.*/
int operationWithTwoOperand(const char *line, const char *newSymbolName, int numOper, int countLine);

/* Performs an operation with one operand.*/
int operationWithOneOperand(const char *line, const char *newSymbolName, int numOper, int countLine);

/* Performs an operation without operands.*/
int operationWithoutOperand(const char *line, const char *newSymbolName, int numOper, int countLine);

/*int checkIfLabbleIsData(const char* label);*/

/* Checks if a label refers to a data location. */
int checkIfLabelIsData(const char *label);

/* Checks if a name is a restricted keyword.*/
int isNameRestricted(const char *name);

/* Finds the index of a given operation in the list of operations.*/
int findOperation(const char *name);

/* Checks if a name is a register.*/
int isRegister(const char *name);

/* Determines the addressing method for a given operand.*/
char *findsMethodOfAddressing(const char *operand);

/* Performs a subtraction operation on two operands and returns the corresponding machine code.*/
char *miunOperation(int num, const char *operand1, const char *operand2);

/* Generates machine code for a subtraction operation with a single operand.*/
char *miunOperand(const char *operand, int firstOperand);

/* Subtracts two registers and returns the corresponding machine code.*/
char *miunTwoRegister(const char *operand1, const char *operand2);

/* Adds a symbol to the symbol table.*/
void addSymbol(const char *symbolName, const char *externName, const char *code, const int isData, const int isEntry, const int isExtern);

/* Adds a label to the symbol table.*/
void addLabel(const char *name, const int count);

/* Adds an external symbol to the list of external symbols.*/
void addExtern(const char *name);

/* Adds a line count to the list of line counts.*/
void addL(int counteLine, int wordCounter);

/*check if it a labbel definition*/
char *labelDefinition(const char *token);

/* Finds a label in the symbol table by its name.*/
int findLabel(const char *token);

/* Finds an external symbol by its name.*/
char *findExtern(const char *externName);

/* Checks if a string is enclosed in double quotes.*/
int isCorrectString(const char *name);

/* Checks if a string represents a valid number.*/
int isNumber(const char *str);

/* Converts a binary string to its decimal equivalent.*/
int binaryToDecimal(const char *binary);

/* Converts a decimal number to its binary representation.*/
char *decimalToBinary(int n, int numOfChar);

/* Correct commas in a string representing a list of objects.*/
int correctCommas(char *line);

/* The function frees the defined memory */
void freeMemory();