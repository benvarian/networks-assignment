#include "fileio.h"

/**
 * Generates unsigned int for a given string
 * and returns it
*/
uint32_t hash_string(char *string)
{
    uint32_t hash = 0;

    while (*string != '\0')
    {
        hash = hash * 33 + *string;
        ++string;
    }
    return hash;
}

/**
 * Allocates and initializes space for a new hashtable
 * (an array of testing)
*/
HASHTABLE *hashtable_new(void)
{
    HASHTABLE *new = calloc(HASHTABLE_SIZE, sizeof(TESTINFO *));
    CHECK_ALLOC(new);
    return new;
}

/*  ADD A NEW USER TO A GIVEN HASHTABLE
    HASHING IS BASED ON THE USERNAME (SO IT MUST BE UNIQUE) */
void hashtable_add(HASHTABLE *hashtable, char *username, char *passw, enum qType *types, int *qid, int *attemptsLeft, bool *correct)
{
    uint32_t h = hash_string(username) % HASHTABLE_SIZE; // get index
    // allocate memory for user info
    TESTINFO *new = malloc(sizeof(TESTINFO));
    CHECK_ALLOC(new);
    hashtable[h] = new;
    // populate the user info in the hashtable entry
    hashtable[h]->user = malloc(strlen(username) * sizeof(char) + 1);
    CHECK_ALLOC(hashtable[h]->user);
    strcpy(hashtable[h]->user, username);
    hashtable[h]->pw = malloc(strlen(passw) * sizeof(char) + 1);
    CHECK_ALLOC(hashtable[h]->pw);
    strcpy(hashtable[h]->pw, passw);
    hashtable[h]->type = types;
    hashtable[h]->qid = qid;
    hashtable[h]->attemptsLeft = attemptsLeft;
    hashtable[h]->correct = correct;
    hashtable[h]->currentq = 0; // start at question 0
}

/*  DETERMINE IF A USER EXISTS IN THE HASHTABLE AND
    RETURN A POINTER TO THE USER INFO STRUCTURE IF IT EXISTS
    RETURNS NULL POINTER IF USER DOES NOT EXIST     */
TESTINFO *hashtable_get(HASHTABLE *hashtable, char *username)
{
    uint32_t h = hash_string(username) % HASHTABLE_SIZE; // get index
    if (h <= 0)
    {
        return NULL;
    }
    else
    {
        return hashtable[h];
    }
}

/* Will open a file in the specified mode, returning the file pointer */
FILE *openFile(char *file_path, char *mode)
{
    FILE *fp;
    fp = fopen(file_path, mode);
    if (fp == NULL)
    {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }
    return (fp); // Owness is on the calling function to close file when finished
}

/* Reads text from the file into the given buffer pointer */
void readFile(FILE *fp, char **buffer)
{
    // Get size of the file
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    // Allocate memory for buffer, then read into buffer
    *buffer = calloc(size + 1, sizeof(char));
    CHECK_ALLOC(buffer);
    fread(*buffer, size, 1, fp);
    return;
}

/**
 * Gets the data from the csv pointed at filepath and for each student
 * adds it to the hashtable for later usage
 * increments the number of students each time a line is read in, and stores
 * the student name inside studentNames array provided
*/
void getData(HASHTABLE *hashtable, int *numStudents, char ***studentNames, char *filepath)
{
    // get data into a buffer
    FILE *fp = openFile(filepath, "r");
    char *buffer;
    readFile(fp, &buffer);
    fclose(fp);
    // split data into rows for each user
    char *saverow;
    char *user;
    char *password;
    char *row = strtok_r(buffer, "\n", &saverow);
    char **studentNamestmp;
    // strtok again to skip first row
    row = strtok_r(NULL, "\n", &saverow);
    while (row != NULL)
    {
        char *saveentry, *savetype, *saveqid, *saveattempts, *savecorrect;
        // parse username
        char *entries = strtok_r(row, ",", &saveentry);
        user = malloc(strlen(entries) * sizeof(char) + 1);
        CHECK_ALLOC(user);
        strcpy(user, entries);
        // Add student name to the list of names
        if (*numStudents == 0)
        {
            *studentNames = realloc(NULL, sizeof(char *)); // allocate memory for first string
            CHECK_ALLOC(*studentNames);
        }
        else
        {
            studentNamestmp = (char **)realloc(*studentNames, *numStudents * sizeof(char *) + sizeof(char *)); // realloc if studentNames already has memory
            CHECK_ALLOC(studentNamestmp);
            *studentNames = studentNamestmp;
        }
        (*studentNames)[*numStudents] = calloc(1, strlen(user) * sizeof(char) + 1); // allocate memory for string
        CHECK_ALLOC((*studentNames)[*numStudents]);
        strcpy((*studentNames)[*numStudents], user);
        *numStudents = *numStudents + 1;

        // parse password
        entries = strtok_r(NULL, ",", &saveentry);
        password = malloc(strlen(entries) * sizeof(char) + 1);
        CHECK_ALLOC(password);
        strcpy(password, entries);

        // parse enumerated types
        entries = strtok_r(NULL, ",", &saveentry);
        enum qType *types = calloc(sizeof(int), NUM_QUESTIONS); // enums take same size as int
        char *typetok = strtok_r(entries, "$", &savetype);
        for (int i = 0; i < NUM_QUESTIONS; i++)
        {
            if (strcmp(typetok, "M") == 0)
                types[i] = (enum qType)M;
            else if (strcmp(typetok, "P") == 0)
                types[i] = (enum qType)P;
            else
                types[i] = (enum qType)N; // ERROR: SHOULD BE M OR P NOT NOTHING
            typetok = strtok_r(NULL, "$", &savetype);
        }

        // parse question IDs
        entries = strtok_r(NULL, ",", &saveentry);
        int *qid = (int *)calloc(sizeof(int), NUM_QUESTIONS);
        CHECK_ALLOC(qid);
        char *qidtok = strtok_r(entries, "$", &saveqid);
        for (int i = 0; i < NUM_QUESTIONS; i++)
        {
            qid[i] = atoi(qidtok);
            qidtok = strtok_r(NULL, "$", &saveqid);
        }

        // parse attempts left
        entries = strtok_r(NULL, ",", &saveentry);
        int *attemptsLeft;
        attemptsLeft = (int *)calloc(sizeof(int), NUM_QUESTIONS);
        CHECK_ALLOC(attemptsLeft);
        char *attemptstok = strtok_r(entries, "$", &saveattempts);
        for (int i = 0; i < NUM_QUESTIONS; i++)
        {
            attemptsLeft[i] = atoi(attemptstok);
            attemptstok = strtok_r(NULL, "$", &saveattempts);
        }

        // parse correct answers
        entries = strtok_r(NULL, ",", &saveentry);
        bool *correct;
        correct = (bool *)calloc(sizeof(bool *), NUM_QUESTIONS);
        CHECK_ALLOC(correct);
        char *correcttok = strtok_r(entries, "$", &savecorrect);
        for (int i = 0; i < NUM_QUESTIONS; i++)
        {
            if (strcmp(correcttok, "T") == 0) correct[i] = true;
            else correct[i] = false;
            correcttok = strtok_r(entries, "$", &savecorrect);
        }

        // add user to hashtable
        hashtable_add(hashtable, user, password, types, qid, attemptsLeft, correct);
        // free the allocated memory
        free(user);
        free(password);
        row = strtok_r(NULL, "\n", &saverow);
    }
    if (*numStudents == 0)
    {
        printf("Error: No student data in database\n");
        exit(EXIT_FAILURE);
    }
    free(buffer);
}

/**
 * Writes the data inside the hashtable to the csv, based on each student name
 * csv is saved to the filepath
*/
void writeToCSV(HASHTABLE *hashtable, int *numStudents, char **studentNames, char *filepath)
{
    FILE *fp = openFile(filepath, "w");
    TESTINFO *entry;
    fprintf(fp, "user,pw,qtype,qid,attemptsLeft,correct,questiontracker\n");
    for (int i = 0; i < *numStudents; i++)
    {
        entry = hashtable_get(hashtable, studentNames[i]);
        char *types;
        char *qid;
        char *attempts;
        char *correct;
        // allocate space for each question and a space in between
        types = calloc(sizeof(char) * 2, NUM_QUESTIONS);
        CHECK_ALLOC(types);
        qid = calloc(sizeof(int) * 2, NUM_QUESTIONS);
        CHECK_ALLOC(qid);
        attempts = calloc(sizeof(int) * 2, NUM_QUESTIONS);
        CHECK_ALLOC(attempts);
        correct = calloc(sizeof(char) * 2, NUM_QUESTIONS);
        CHECK_ALLOC(correct);

        // Concatenate the data into strings to add to the csv
        for (int j = 0; j < NUM_QUESTIONS; j++)
        {
            if (j != NUM_QUESTIONS - 1)
            {
                sprintf(types + strlen(types), "%s$", (entry->type[j] == P) ? "P" : "M");
                sprintf(qid + strlen(qid), "%i$", entry->qid[j]);
                sprintf(attempts + strlen(attempts), "%i$", entry->attemptsLeft[j]);
                sprintf(correct + strlen(correct), "%s$", (entry->correct[j] == true) ? "T" : "F");
            }
            else
            { // if it is the final question data, dont add $
                sprintf(types + strlen(types), "%s", (entry->type[j] == P) ? "P" : "M");
                sprintf(qid + strlen(qid), "%i", entry->qid[j]);
                sprintf(attempts + strlen(attempts), "%i", entry->attemptsLeft[j]);
                sprintf(correct + strlen(correct), "%s", (entry->correct[j] == true) ? "T" : "F");
            }
        }
        // NOTE: Question tracker is always set to 0, because a student should always start at q1
        if (i != *numStudents - 1)
            fprintf(fp, "%s,%s,%s,%s,%s,%s,0\n", entry->user, entry->pw, types, qid, attempts, correct);
        else
            fprintf(fp, "%s,%s,%s,%s,%s,%s,0", entry->user, entry->pw, types, qid, attempts, correct);
        free(types);
        free(qid);
        free(attempts);
        free(correct);
    }
    fclose(fp);
}
