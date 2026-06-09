#define MAX_LINE_LENGTH 81
#define MAX_MACRO_NAME 77

typedef struct
{
    char *name;
    char *content;
} Macro;

extern Macro *macros;
extern int macroCount;

/* Adds a new macro to the list of macros.*/
void addMacro(const char *name, const char *content) ;

/* Finds a macro by its name.*/
const char* findMacro(const char *name);

/* Checks if a file is open and valid.*/
int checkingWhetherTheFileIsCorrect(FILE * checkedFile);

/* Removes a file and exits the program.*/
void removeFileAndExit(const char *outputFilename);

/* Displays an error message and exits the program.*/
void regetError(const char *errorLine, FILE *outputFile, char *outputFilename);

/* Processes an assembly language file.*/
void processFile(const char *inputFilename);
