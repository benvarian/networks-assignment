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
    char *saverow, *saveentry, *savetype, *savequestions, *saveanswers, *saveattempts, *savecorrect;
    char *row = strtok_r(buffer, "\n", &saverow);
    // strtok again to skip first row
    row = strtok_r(NULL, "\n", &saverow);
    while (row != NULL) {
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
        enum qType *types = calloc(sizeof(NUM_QUESTIONS), sizeof(enum qType));
        //CHECK_ALLOC(*types);
        char *typetok = strtok_r(entries, ",", &savetype);
        for(int i = 0; i < NUM_QUESTIONS; i++) {
            CHECK_ALLOC(types);
            if(i == 0) typetok = &typetok[1]; // if at start or end, remove the "
            if(i == NUM_QUESTIONS - 1) typetok = &typetok[0];
            if(strcmp(typetok, "M") == 0) types[i] = (enum qType) M;
            else if (strcmp(typetok, "P")) types[i] = (enum qType) P;
            else types[i] = (enum qType) NULL;   // ERROR: SHOULD BE M OR P NOT NOTHING
            strtok_r(NULL, ",", &savetype);
        }

        // parse questions
        entries = strtok_r(NULL, ",", &saveentry);
        char *questions[NUM_QUESTIONS];
        char *questionstok = strtok_r(entries, ",", &savequestions);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            questions[i] = calloc(sizeof(questionstok), sizeof(char));
            CHECK_ALLOC(questions[i]);
            strcpy(questionstok, questions[i]);
            strtok_r(NULL, ",", &savequestions);
        }

        // parse answers
        entries = strtok_r(NULL, ",", &saveentry);
        char *answers[NUM_QUESTIONS];
        CHECK_ALLOC(answers);
        char *answerstok = strtok_r(entries, ",", &saveanswers);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            answers[i] = calloc(sizeof(answerstok), sizeof(char));
            CHECK_ALLOC(answers[i]);
            strcpy(answerstok, answers[i]);
            strtok_r(NULL, ",", &saveanswers);
        }

        // parse attempts left
        entries = strtok_r(NULL, ",", &saveentry);
        int *attemptsLeft = calloc(sizeof(NUM_QUESTIONS), sizeof(int));
        CHECK_ALLOC(attemptsLeft);
        char *attemptstok = strtok_r(entries, ",", &saveattempts);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            attemptsLeft[i] = atoi(attemptstok);
            strtok_r(NULL, ",", &saveattempts);
        }

        // parse correct answers
        entries = strtok_r(NULL, ",", &saveentry);
        bool *correct = calloc(sizeof(NUM_QUESTIONS), sizeof(bool));
        CHECK_ALLOC(correct);
        char *correcttok = strtok_r(entries, ",", &savecorrect);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            if(correcttok[i] == 'T') correct[i] = true;
            else if (correcttok[i] == 'F') correct[i] = false;
            else correct[i] = (bool) NULL;     // ERROR: SHOULD BE T OR F NOT NOTHING
            strtok_r(entries, ",", &savecorrect);
        }

        // add user to hashtable
        hashtable_add(hashtable, user, password, types, questions, answers, attemptsLeft, correct);
        // NEED TO FREE THE MEMORY OF THE HASHTABLE AT THE END
        free(user);
        free(password);
        free(types);
        for(int i = 0; i < NUM_QUESTIONS; i++) {
            free(questions[i]);
            free(answers[i]);
        }
        free(attemptsLeft);
        free(correct);
        strtok_r(NULL, "\n", &saverow);
    }
    free(buffer);
}

int main(int argc, char *argv[]) {
    HASHTABLE *hashtable = hashtable_new();
    getData(hashtable, "./userdata.csv");
}