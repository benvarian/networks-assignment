#include "fileio.h"



uint32_t hash_string(char *string) {
    uint32_t hash = 0;

    while(*string != '\0') {
        hash = hash*33 + *string;
        ++string;
    }
    return hash;
}

//  ALLOCATE AND INITIALIZE SPACE FOR A NEW HASHTABLE (AN ARRAY OF TESTINFO)
HASHTABLE *hashtable_new(void) {
    HASHTABLE   *new = calloc(HASHTABLE_SIZE, sizeof(TESTINFO *));
    CHECK_ALLOC(new);
    return new;
}

/*  ADD A NEW USER TO A GIVEN HASHTABLE
    HASHING IS BASED ON THE USERNAME (SO IT MUST BE UNIQUE) */
void hashtable_add(HASHTABLE *hashtable, char *username, char *passw, enum qType *types, char **questions, char **answers, int *attemptsLeft, bool *correct) {
    uint32_t h   = hash_string(username) % HASHTABLE_SIZE;  // get index
    // allocate memory for user info
    TESTINFO *new = calloc(1, sizeof(TESTINFO));
    CHECK_ALLOC(new);
    hashtable[h] = new;
    // populate the user info in the hashtable entry
    hashtable[h]->user = username;
    hashtable[h]->pw = passw;
    hashtable[h]->type = types;
    hashtable[h]->questions = questions;
    hashtable[h]->answers = answers;
    hashtable[h]->attemptsLeft = attemptsLeft;
    hashtable[h]->correct = correct;
}

/*  DETERMINE IF A USER EXISTS IN THE HASHTABLE AND
    RETURN A POINTER TO THE USER INFO STRUCTURE IF IT EXISTS 
    RETURNS NULL POINTER IF USER DOES NOT EXIST     */
TESTINFO *hashtable_get(HASHTABLE *hashtable, char *username) {
    uint32_t h	= hash_string(username) % HASHTABLE_SIZE;   // get index
    return hashtable[h];
}

/* Will open a file in the specified mode, returning the file pointer */
FILE* openFile(char *file_path, char *mode) {
	FILE *fp;
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

void getData(HASHTABLE *hashtable, int *numStudents, char (*studentNames)[MAX_USER_LENGTH], char *filepath) {
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
        studentNames = realloc(studentNames, sizeof(user));
        CHECK_ALLOC(studentNames);
        strcpy(studentNames[*numStudents], user);
        *numStudents = *numStudents + 1;

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
            else types[i] = (enum qType) N;   // ERROR: SHOULD BE M OR P NOT NOTHING
            typetok = strtok_r(NULL, "$", &savetype);
        }

        // parse questions
        entries = strtok_r(NULL, ",", &saveentry);
        char **questions;
        questions = (char **)calloc(sizeof(char *), NUM_QUESTIONS);
        char *questionstok = strtok_r(entries, "$", &savequestions);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            questions[i] = calloc(sizeof(questionstok), sizeof(char));
            CHECK_ALLOC(questions[i]);
            strcpy(questions[i], questionstok);
            questionstok = strtok_r(NULL, "$", &savequestions);
        }

        // parse answers
        entries = strtok_r(NULL, ",", &saveentry);
        char **answers;
        answers = (char **)calloc(sizeof(char *), NUM_QUESTIONS);
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
        int *attemptsLeft;
        attemptsLeft = (int *)calloc(sizeof(int *), NUM_QUESTIONS);
        char *attemptstok = strtok_r(entries, "$", &saveattempts);
        for (int i = 0; i < NUM_QUESTIONS; i++) {
            attemptsLeft[i] = atoi(attemptstok);
            attemptstok = strtok_r(NULL, "$", &saveattempts);
        }

        // parse correct answers
        entries = strtok_r(NULL, ",", &saveentry);
        bool *correct;
        correct = (bool *)calloc(sizeof(bool *), NUM_QUESTIONS);
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

void writeToCSV(HASHTABLE *hashtable, int *numStudents, char (*studentNames)[MAX_USER_LENGTH], char *filepath) {
    FILE *fp = openFile(filepath, "w");
    TESTINFO *entry;
    fprintf(fp, "user,pw,qtype,questions,answers,attemptsLeft,correct\n");
    for (int i = 0; i < *numStudents; i++) {
        entry = hashtable_get(hashtable, studentNames[i]);
        char *types;
        char *attempts;
        char *correct;
        types = malloc(NUM_QUESTIONS * sizeof(char) * 2); // allocate space for each question and a space in between
        CHECK_ALLOC(types);
        attempts = malloc(NUM_QUESTIONS * sizeof(int) * 2);
        CHECK_ALLOC(attempts);
        correct = malloc(NUM_QUESTIONS * sizeof(char) * 2);
        CHECK_ALLOC(correct);
        // Calculate size of questions/answers then allocate memory for a string to hold it
        int qsize = 0; 
        int asize = 0;
        for (int j = 0; j < NUM_QUESTIONS; j++) {
            qsize += strlen(entry->questions[j]) + 1; // +1 for $ delimiter or '\0'
            asize += strlen(entry->answers[j]) + 1;
        }
        char *questions = malloc(qsize);
        CHECK_ALLOC(questions);
        char *answers = malloc(asize);
        CHECK_ALLOC(answers);
        // Concatenate the data into strings to add to the csv
        for (int j = 0; j < NUM_QUESTIONS; j++) {
            if(j != NUM_QUESTIONS-1) {
                sprintf(questions + strlen(questions), "%s$", entry->questions[j]);
                sprintf(answers + strlen(answers), "%s$", entry->answers[j]);
                sprintf(types + strlen(types), "%s$", (entry->type[j] == P) ? "P" : "M");
                sprintf(attempts + strlen(attempts), "%i$", entry->attemptsLeft[j]);
                sprintf(correct + strlen(correct), "%s$", (entry->correct[j] == true) ? "T" : "F");
            }
            else { // if it is the final question data, dont add $
                sprintf(questions + strlen(questions), "%s", entry->questions[j]);
                sprintf(answers + strlen(answers), "%s", entry->answers[j]);
                sprintf(types + strlen(types), "%s", (entry->type[j] == P) ? "P" : "M");
                sprintf(attempts + strlen(attempts), "%i", entry->attemptsLeft[j]);
                sprintf(correct + strlen(correct), "%s", (entry->correct[j] == true) ? "T" : "F");
            }
        }
        fprintf(fp, "%s,%s,%s,%s,%s,%s,%s\n", entry->user, entry->pw, types, questions, answers, attempts, correct);
    }
    fclose(fp);
}

// TESTING FILE IO
int main(void) {
    // HASHTABLE *hashtable = hashtable_new();
    // getData(hashtable, &numStudents, studentNames, "./userdata.csv");
    // TESTINFO *mitch = hashtable_get(hashtable, "mitch");
    // printf("Username: %s\nPassword: %s\nQuestion 1: %s\nAnswer 1: %s\n", mitch->user, mitch->pw, mitch->questions[0], mitch->answers[0]);
    // writeToCSV(hashtable, &numStudents, studentNames, "./userdata.csv");
}   
