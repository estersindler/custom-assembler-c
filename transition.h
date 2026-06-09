/* Performs the first pass of the assembler, processing the source code and building the symbol table.*/
void firstPass(const char *sourceFile);

/* Performs the second pass of the assembler, resolving labels and updating operands.*/
void secondPass(const char *sourceFile);

/* Builds the output files (.ob, .ext, .ent) based on the symbol table and line counts.*/
void buildOutputFiles(const char *sourceFile);


