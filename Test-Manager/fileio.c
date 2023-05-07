#include "fileio.h"
#define NUM_QUESTIONS   10

/* Will open a file in the specified mode, returning the file pointer */
FILE* openFile(char *file_path, char *mode) {
	FILE *fp;
	//char fullpath[1024];
	//realpath(file_path, fullpath);
    fp = fopen(file_path, mode);
	if(fp == NULL) {
		perror("Failed to open file");
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	return(fp); // Owness is on the calling function to close file when finished
}

/* Reads text from the file into a buffer then closes the file, returning the buffer */
char *readFile(FILE *fp) {
    // Get size of the file
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    // Allocate memory for buffer, then read into buffer
    char *buffer = calloc(size, sizeof(char));
    CHECK_ALLOC(buffer);
    fread(buffer, size, 1, fp);
    fclose(fp);
    return buffer;
}

void getData(HASHTABLE *hashtable, char *filepath) {
    // get data into a buffer
    FILE *fp = openFile(filepath, "r");
    char *buffer = readFile(fp);
    // split data into rows for each user
    char *saverow;
    char *row = strtok_r(buffer, "\n", &saverow);
    // strtok again to skip first row
    row = strtok_r(NULL, "\n", &saverow);
    while (row != NULL) {
        char *saveentry, *savetype, *savequestions, *saveanswers, *saveattempts, *savecorrect;
        // parse username
        char *entries = strtok_r(row, ",", &saveentry);
        char *user = calloc(sizeof(entries), sizeof(char));
        CHECK_ALLOC(user);
        strcpy(user, entries);

        // parse password
        entries = strtok_r(NULL, ",", &saveentry);
        char *password = calloc(sizeof(entries), sizeof(char));
        CHECK_ALLOC(password);
        strcpy(password, entries);

        // parse enumerated types
        entries = strtok_r(NULL, ",", &saveentry);
        enum qType types[NUM_QUESTIONS];
        char *typetok = strtok_r(entries, "$", &savetype);
        for(int i = 0; i < NUM_QUESTIONS; i++) {
            if(strcmp(typetok, "M") == 0) types[i] = (enum qType) M;
            else if (strcmp(typetok, "P") == 0) types[i] = (enum qType) P;
            else types[i] = (enum qType) NULL;   // ERROR: SHOULD BE M OR P NOT NOTHING
            typetok = strtok_r(NULL, "$", &savetype);
        }

        // parse questions
        entries = strtok_r(NULL, ",", &saveentry);
        char *questions[NUM_QUESTIONS];
        char *questionstok = strtok_r(entries, "$", &savequestions);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            questions[i] = calloc(sizeof(questionstok), sizeof(char));
            CHECK_ALLOC(questions[i]);
            strcpy(questions[i], questionstok);
            questionstok = strtok_r(NULL, "$", &savequestions);
        }

        // parse answers
        entries = strtok_r(NULL, ",", &saveentry);
        char *answers[NUM_QUESTIONS];
        CHECK_ALLOC(answers);
        char *answerstok = strtok_r(entries, "$", &saveanswers);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            answers[i] = calloc(sizeof(answerstok), sizeof(char));
            CHECK_ALLOC(answers[i]);
            strcpy(answers[i], answerstok);
            answerstok = strtok_r(NULL, "$", &saveanswers);
        }

        // parse attempts left
        entries = strtok_r(NULL, ",", &saveentry);
        int attemptsLeft[NUM_QUESTIONS];
        char *attemptstok = strtok_r(entries, "$", &saveattempts);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            attemptsLeft[i] = atoi(attemptstok);
            attemptstok = strtok_r(NULL, "$", &saveattempts);
        }

        // parse correct answers
        entries = strtok_r(NULL, ",", &saveentry);
        bool correct[NUM_QUESTIONS];
        char *correcttok = strtok_r(entries, "$", &savecorrect);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            if(strcmp(correcttok, "T")) correct[i] = true;
            else if (strcmp(correcttok, "F")) correct[i] = false;
            else correct[i] = (bool) NULL;     // ERROR: SHOULD BE T OR F NOT NOTHING
            correcttok = strtok_r(entries, "$", &savecorrect);
        }

        // add user to hashtable
        hashtable_add(hashtable, user, password, types, questions, answers, attemptsLeft, correct);
        row = strtok_r(NULL, "\n", &saverow);
    }
    free(buffer);
}

void freeMemory(TESTINFO *student) {
    /* NEED TO FIX UP FREEING QUESTIONS AND ANSWERS, MEMORY LEAKS RIGHT NOW
    for(int i= 0; i < NUM_QUESTIONS; i++) {
        free(student->answers[i]);
        free(student->questions[i]);
    }
    */
    free(student->user);
    free(student->pw);
}

int main(int argc, char *argv[]) {
    HASHTABLE *hashtable = hashtable_new();
    getData(hashtable, "./userdata.csv");
    TESTINFO *mitch = hashtable_get(hashtable, "mitch");
    printf("Username: %s\nPassword: %s\nQuestion 1: %s\nAnswer 1: %s\n", mitch->user, mitch->pw, mitch->questions[0], mitch->answers[0]);
    freeMemory(mitch);
}
