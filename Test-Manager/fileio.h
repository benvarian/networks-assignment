#include "hashtable/hashtable.h"
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>

// Opens the file and returns file pointer
extern FILE *openFile(char *file_path, char *mode);

// Reads content of the file into a buffer
extern char *readFile(FILE * fp);

// Uses openFile and readFile to get data and put it in the hashtable
// shouldn't need to call openFile and readFile, just this
extern void getData(HASHTABLE *hashtable, char *filepath);

// Frees memory of a student once finished
extern void freeMemory(TESTINFO *student);