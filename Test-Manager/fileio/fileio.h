#define _POSIX_C_SOURCE     200809L
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define	HASHTABLE_SIZE		997
#define NUM_QUESTIONS       10
#define MAX_USER_LENGTH     256
#define FILEPATH            "./fileio/userdata.csv"

//  ENUMERATION OF QUESTION TYPE - M FOR MULTICHOICE, P FOR PROGRAMMING, N FOR NONE
enum qType {M, P, N};

//  STUDENT DATA STRUCTURE
typedef struct _testinfo {
    char         *user;          // Username of student
    char         *pw;            // Password of student
    enum qType   *type;          // array of question types
    int          *qid;           // array of question IDs (unsigned int)
    int          *attemptsLeft;  // array of attempts left for each question (unsigned int)
    bool         *correct;       // array of booleans stating if student has answered correct or not
    int          currentq;       // current question a student is viewing in their test
    } TESTINFO; 

//  A HELPFUL PREPROCESSOR MACRO TO CHECK IF ALLOCATIONS WERE SUCCESSFUL
#define CHECK_ALLOC(p) if(p == NULL) { perror(__func__); exit(EXIT_FAILURE); }

//  WE DEFINE A HASHTABLE AS A (WILL BE, DYNAMICALLY ALLOCATED) ARRAY OF USER INFO
typedef	TESTINFO * HASHTABLE;

extern uint32_t hash_string(char *string);

extern HASHTABLE *hashtable_new(void);

extern void hashtable_add(HASHTABLE *hashtable, char *username, char *passw, enum qType *types, int *qid, int *attemptsLeft, bool *correct);

extern TESTINFO *hashtable_get(HASHTABLE *hashtable, char *username);

extern FILE *openFile(char *file_path, char *mode);

extern void readFile(FILE *fp, char **buffer);

extern void getData(HASHTABLE *hashtable, int *numStudents, char ***studentNames, char *filepath);

extern void writeToCSV(HASHTABLE *hashtable, int *numStudents, char **studentNames, char *filepath);
