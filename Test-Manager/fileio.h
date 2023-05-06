#include "hashtable/hashtable.h"
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>

extern FILE *openFile(char *file_path, char *mode);

extern char *readFile(FILE * fp);

extern void getData(HASHTABLE *hashtable, char *filepath);