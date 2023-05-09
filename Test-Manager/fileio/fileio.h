#define _POSIX_C_SOURCE     200809L
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#define	HASHTABLE_SIZE		997
#define NUM_QUESTIONS   10
#define MAX_USER_LENGTH 128 // Max name length of a user
#define str(x) #x           // used to convert enum to string

//  ENUMERATION OF QUESTION TYPE - M FOR MULTICHOICE, P FOR PROGRAMMING
//  https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
/*#define qType(Q) \
        Q(M)     \
        Q(P)     \
        Q(N)     \
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum qType {M, P, N};
static const char *qTypeString[] = { "M", "P", "N" }; */
enum qType {M, P, N};


//  STUDENT DATA STRUCTURE
typedef struct _testinfo {
    char         *user;          // Username of student
    char         *pw;            // Password of student
    enum qType   *type;          // array of question types
    char         **questions;    // array of questions assigned to the student
    char         **answers;      // array of answers assigned to student
    int          *attemptsLeft;  // array of attempts left for each question
    bool         *correct;       // array of booleans stating if student has answered correct or not
    } TESTINFO; 

//  A HELPFUL PREPROCESSOR MACRO TO CHECK IF ALLOCATIONS WERE SUCCESSFUL
#define CHECK_ALLOC(p) if(p == NULL) { perror(__func__); exit(EXIT_FAILURE); }

//  WE DEFINE A HASHTABLE AS A (WILL BE, DYNAMICALLY ALLOCATED) ARRAY OF USER INFO
typedef	TESTINFO * HASHTABLE;

//  ALLOCATE AND INITIALIZE SPACE FOR A NEW HASHTABLE (AN ARRAY OF TESTINFO)
extern HASHTABLE *hashtable_new(void);

/*  ADD A NEW USER TO A GIVEN HASHTABLE
    HASHING IS BASED ON THE USERNAME (SO IT MUST BE UNIQUE)
*/
extern void hashtable_add(HASHTABLE *hashtable, char *username, char *passw, enum qType *type, char **questions, char **answers, int *attemptsLeft, bool *correct);

/*  DETERMINE IF A USER EXISTS IN THE HASHTABLE AND
    RETURN A POINTER TO THE USER INFO STRUCTURE IF IT EXISTS 
    RETURNS NULL POINTER IF USER DOES NOT EXIST     
*/
extern TESTINFO *hashtable_get(HASHTABLE *hashtable, char *username);

// Opens the file and returns file pointer
extern FILE *openFile(char *file_path, char *mode);

// Reads content of the file into a buffer
extern char *readFile(FILE * fp);

// Uses openFile and readFile to get data and put it in the hashtable
// shouldn't need to call openFile and readFile, just this
extern void getData(HASHTABLE *hashtable, int *numStudents, char (*studentNames)[MAX_USER_LENGTH], char *filepath);

// Frees memory of a student once finished
extern void freeMemory(TESTINFO *student);

// Writes data from the hashtable into the csv at filepath, using the list of students
// as a reference for the values in the hashtable
extern void writeToCSV(HASHTABLE *hashtable, int *numStudents, char (*studentNames)[MAX_USER_LENGTH], char *filepath);