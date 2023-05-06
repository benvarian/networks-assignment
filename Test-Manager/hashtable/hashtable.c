// SHAMELESSLY ADAPTED FROM CHRIS CITS2002 WORKSHOP 7
#include "hashtable.h"

//  --------------------------------------------------------------------

//  FUNCTION hash_string() ACCEPTS A STRING PARAMETER,
//  AND RETURNS AN UNSIGNED 32-BIT INTEGER AS ITS RESULT
//
//  see:  https://en.cppreference.com/w/c/types/integer

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