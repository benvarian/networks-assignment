/**
** @file server.c
** @brief implementation of the server required for test-manager
**/

#include "server.h"
#include "fileio/fileio.h"
#include "Data-Structures/Queue/Queue.h"

HASHTABLE *hashtable;
QBInformation *qb_info;
int numStudents = 0;
char **studentNames;

void usage(void)
{
    fprintf(stderr, "usage: ./server port\n");
    exit(EXIT_FAILURE);
}

void drop_client(SOCKET socket)
{
    close(socket);
}

const char *get_content_type(const char *path)
{
    const char *last_dot = strrchr(path, '.');
    if (last_dot)
    {
        if (strcmp(last_dot, ".css") == 0)
            return "text/css";
        if (strcmp(last_dot, ".csv") == 0)
            return "text/csv";
        if (strcmp(last_dot, ".gif") == 0)
            return "image/gif";
        if (strcmp(last_dot, ".htm") == 0)
            return "text/html";
        if (strcmp(last_dot, ".html") == 0)
            return "text/html";
        if (strcmp(last_dot, ".ico") == 0)
            return "image/x-icon";
        if (strcmp(last_dot, ".jpeg") == 0)
            return "image/jpeg";
        if (strcmp(last_dot, ".jpg") == 0)
            return "image/jpeg";
        if (strcmp(last_dot, ".js") == 0)
            return "application/javascript";
        if (strcmp(last_dot, ".json") == 0)
            return "application/json";
        if (strcmp(last_dot, ".png") == 0)
            return "image/png";
        if (strcmp(last_dot, ".pdf") == 0)
            return "application/pdf";
        if (strcmp(last_dot, ".svg") == 0)
            return "image/svg+xml";
        if (strcmp(last_dot, ".txt") == 0)
            return "text/plain";
    }

    return "application/octet-stream";
}

void http_request_destructor(HTTPRequest *request)
{
    dictionary_destructor(&request->request_line);
    dictionary_destructor(&request->header_fields);
    dictionary_destructor(&request->body);
}

void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void send_200(SOCKET socket)
{
    const char *c200 = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html\r\n";
    send(socket, c200, strlen(c200), 0);
    drop_client(socket);
}

void send_400(SOCKET socket)
{
    const char *c400 = "HTTP/1.1 400 Bad Request\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 11\r\n\r\nBad Request";
    send(socket, c400, strlen(c400), 0);
    drop_client(socket);
}

void send_QB_disconnected(SOCKET socket)
{
    const char *c400 = "HTTP/1.1 400 Bad Request\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 35\r\n\r\nCannot Start Quiz: QB Not Connected";
    send(socket, c400, strlen(c400), 0);
    drop_client(socket);
}

void send_404(SOCKET socket)
{
    const char *c404 = "HTTP/1.1 404 Not Found\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 9\r\n\r\nPage Not Found";
    send(socket, c404, strlen(c404), 0);
    drop_client(socket);
}

void send_201(SOCKET socket)
{
    const char *c201 = "HTTP/1.1 201 Created\r\n"
                       "Content-Type: text/html\r\n\r\n"
                       "<div><h1>CONGRATS</h1></div";
    send(socket, c201, strlen(c201), 0);
    drop_client(socket);
}

void send_401(SOCKET socket)
{
    const char *c401 = "HTTP/1.1 401 Unauthorized\r\n\r\n";
    send(socket, c401, strlen(c401), 0);
    drop_client(socket);
}

void send_403(SOCKET socket)
{
    const char *c403 = "HTTP/1.1 403 Forbidden\r\n\r\n";
    send(socket, c403, strlen(c403), 0);
    drop_client(socket);
}

void send_307(SOCKET socket)
{
    const char *c307 = "HTTP/1.1 307 Temporary Redirect\r\nLocation: /quiz\r\n\r\n";
    send(socket, c307, strlen(c307), 0);
    drop_client(socket);
}

void send_302(SOCKET socket, const char *path, const char *username)
{
    char c302[52 + (strlen(path) + 1) + (strlen(username) + 1)];
    sprintf(c302, "HTTP/1.1 302 Found\r\nLocation: %s\r\nSet-Cookie: %s\r\n\r\n", path, username);
    send(socket, c302, strlen(c302), 0);
    drop_client(socket);
}

void send_302_quiz_finish(SOCKET socket)
{
    const char *c301 = "HTTP/1.1 301 Found\r\nLocation: /quiz\r\n\r\n";
    send(socket, c301, strlen(c301), 0);
    drop_client(socket);
}

void send_418(SOCKET socket)
{
    const char *c418 = "HTTP/1.1 418 I'm a teapot\r\n\r\n";
    send(socket, c418, strlen(c418), 0);
    drop_client(socket);
}

void send_501(SOCKET socket)
{
    const char *c501 = "HTTP/1.1 501 Not Implemented\r\n\r\n";
    send(socket, c501, strlen(c501), 0);
    drop_client(socket);
}

/*  Concatenates a static 'start' and 'end' html page (found in server.h)
 * with the dynamic 'centre' and send it to the socket
 *  Used for Summary page and displaying each question
 */

void send_webpage(SOCKET socket, char *centre, const char *start, const char *end)
{
    char *web_page = calloc(1, 10013 + 1);
    CHECK_ALLOC(web_page);

    // Concatenate the webpage HTML together
    strcat(web_page, start);
    strcat(web_page, centre);
    strcat(web_page, end);
    // printf("%s:", web_page)

    char buffer[BSIZE];

    sprintf(buffer, "HTTP/1.1 200 ok\r\n");
    send(socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Connection: close\r\n");
    send(socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Length: %zu\r\n", strlen(web_page) + 1);
    send(socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Type: %s\r\n", "text/html");
    send(socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "\r\n");
    send(socket, buffer, strlen(buffer), 0);

    send(socket, web_page, strlen(web_page) + 1, 0);

    free(web_page);
}

// Parses out the request line to retrieve the method, uri, and http version.
void extract_request_line_fields(struct HTTPRequest *request, char *request_line)
{
    // Copy the string literal into a local instance. MITCH ADDED +1 FOR \0
    char fields[strlen(request_line) + 1];
    strcpy(fields, request_line);
    // Separate the string on spaces for each section.
    char *method = strtok(fields, " ");
    char *uri = strtok(NULL, " ");
    char *http_version = strtok(NULL, "\0");
    // Insert the results into the request object as a dictionary
    struct Dictionary request_line_dict = dictionary_constructor((compare_string_keys));
    request_line_dict.insert(&request_line_dict, "method", sizeof("method") + 1, method, strlen(method) * sizeof(char) + 1);
    request_line_dict.insert(&request_line_dict, "uri", sizeof("uri") + 1, uri, strlen(uri) * sizeof(char) + 1);
    request_line_dict.insert(&request_line_dict, "http_version", sizeof("http_version") + 1, http_version, strlen(http_version) * sizeof(char) + 1);
    // Save the dictionary to the request object.
    request->request_line = request_line_dict;
    if (request->request_line.search(&request->request_line, "GET", sizeof("GET")))
    {
        extract_body(request, (char *)request->request_line.search(&request->request_line, "uri", sizeof("uri")));
    }
}

// Parses out the header fields.
void extract_header_fields(struct HTTPRequest *request, char *header_fields)
{
    // Copy the string literal into a local instance.
    char fields[strlen(header_fields) + 1];
    strcpy(fields, header_fields);
    // Save each line of the input into a queue.
    struct Queue headers = queue_constructor();
    char *field = strtok(fields, "\n");
    while (field)
    {
        headers.push(&headers, field, strlen(field) * sizeof(char) + 1);
        field = strtok(NULL, "\r\n");
    }
    // Initialize the request's header_fields dictionary.
    request->header_fields = dictionary_constructor(compare_string_keys);
    // Use the queue to further extract key value pairs.
    char *header = (char *)headers.peek(&headers);
    while (header)
    {
        char *key = strtok(header, ":");
        char *value = strtok(NULL, "\0");
        if (value)
        {
            // Remove leading white spaces.
            if (value[0] == ' ')
            {
                value++;
            }
            // printf(":%s\n", value);
            // Push the key value pairs into the request's header_fields dictionary.
            request->header_fields.insert(&request->header_fields, key, strlen(key) * sizeof(char) + 1, value, strlen(value) * sizeof(char) + 1);
            // Collect the next field from the queue.
        }
        headers.pop(&headers);
        header = (char *)headers.peek(&headers);
    }
    // Destroy the queue.
    queue_destructor(&headers);
}

// Parses the body according to the content type specified in the header fields.
void extract_body(struct HTTPRequest *request, char *body)
{
    // Check what content type needs to be parsed
    char *content_type = (char *)request->header_fields.search(&request->header_fields, "Content-Type", sizeof("Content-Type"));
    if (content_type)
    {
        // Initialize the body_fields dictionary.
        struct Dictionary body_fields = dictionary_constructor(compare_string_keys);
        if (strcmp(content_type, "application/x-www-form-urlencoded") == 0)
        {
            // Collect each key value pair as a set and store them in a queue.
            struct Queue fields = queue_constructor();
            char *field = strtok(body, "&");
            while (field)
            {
                fields.push(&fields, field, strlen(field) * sizeof(char) + 1);
                // loop over to the next field that is parsed otherwise itll cause a hang as its the same string
                field = strtok(NULL, "&");
            }
            // Iterate over the queue to further separate keys from values.
            field = fields.peek(&fields);
            while (field)
            {
                char *key = strtok(field, "=");
                char *value = strtok(NULL, "\0");
                // Remove unnecessary leading white space.
                if (value[0] == ' ')
                {
                    value++;
                }
                // Insert the key value pair into the dictionary.
                body_fields.insert(&body_fields, key, strlen(key) * sizeof(char) + 1, value, strlen(value) * sizeof(char) + 1);
                // Collect the next item in the queue.
                fields.pop(&fields);
                field = fields.peek(&fields);
            }
            // Destroy the queue.
            queue_destructor(&fields);
        }
        else
        {
            // Save the data as a single key value pair.
            body_fields.insert(&body_fields, "data", sizeof("data"), body, strlen(body) * sizeof(char) + 1);
        }
        // Set the request's body dictionary.
        request->body = body_fields;
    }
}

void addq_to_hashtable(char *student_name, int qnum, char *qid, char *type)
{
    // get hash of the student
    uint32_t hash = hash_string(student_name) % HASHTABLE_SIZE;
    // overwrite information by adding a new entry to the hashtable with same username
    hashtable[hash]->qid[qnum] = atoi(qid);
    // convert type to enum
    if (strcmp(type, "P") == 0)
        hashtable[hash]->type[qnum] = P;
    else
        hashtable[hash]->type[qnum] = M;
}

/*  Gets two random integers that add up to NUM_QUESTIONS
    and requests that many questions from the two QBs
    after checking both QBs are connected

    Sends the questions to be added to the hashtable

    takes the student name who is requesting the questions

    returns 0 if successful, -1 if not
*/
int populate_questions(char *student_name)
{
    // Check connection to both TMs
    for (int i = 0; i < NUM_QB; i++)
    {
        ping_QB(qb_info[i].socket, i);
        if (qb_info[i].type == NONE)
            return -1;
    }
    // Get how many questions of each language are being asked
    srand(time(NULL));
    int c_questions = (rand() % NUM_QUESTIONS);
    int p_questions = NUM_QUESTIONS - c_questions;
    char *c_response = calloc(1, MAXDATASIZE + 1);
    CHECK_ALLOC(c_response);
    char *p_response = calloc(1, MAXDATASIZE + 1);
    CHECK_ALLOC(p_response);
    // Ask for questions from QB
    for (int i = 0; i < NUM_QB; i++)
    {
        if (qb_info[i].type == PYTHON && p_questions != 0) // if no questions needed dont ask python qb
        {
            // create the request string, with the language and number of questions needed
            char p_request[64];
            sprintf(p_request, "QUESTIONS\r\n%s:%i\r\n\r\n", "P", p_questions);
            // send/receive request
            if (send(qb_info[i].socket, p_request, strlen(p_request) + 1, 0) == -1)
            {
                perror("send");
                exit(EXIT_FAILURE);
            }
            if (recv(qb_info[i].socket, p_response, 4096, 0) <= 0)
            {
                perror("recv");
                exit(EXIT_FAILURE);
            }
        }
        else if (qb_info[i].type == C && c_questions != 0) // if no quesitons needed dont ask c qb
        {
            // create the request string, with the language and number of questions needed
            char c_request[64];
            sprintf(c_request, "QUESTIONS\r\n%s:%i\r\n\r\n", "C", c_questions);
            if (send(qb_info[i].socket, c_request, strlen(c_request) + 1, 0) == -1)
            {
                perror("send");
                exit(EXIT_FAILURE);
            }
            if (recv(qb_info[i].socket, c_response, 4096, 0) <= 0)
            {
                perror("recv");
                exit(EXIT_FAILURE);
            }
        }
    }
    // SEND C RESPONSE OFF TO ADD TO HASHTABLE
    char *savequestion;
    char *qid;
    char *type;
    int qnum = 0;
    char *question = strtok_r(c_response, "&", &savequestion);
    question = strtok_r(NULL, "&", &savequestion); // parse again to get rid of response header
    while (question != NULL)
    {
        qid = strtok(question, ":");
        type = strtok(NULL, ":");
        addq_to_hashtable(student_name, qnum, qid, type);
        qnum++;
        question = strtok_r(NULL, "&", &savequestion);
    }
    free(c_response);
    // SEND P RESPONSE OFF TO PARSE AND ADD TO HASHTABLE
    question = strtok_r(p_response, "&", &savequestion);
    question = strtok_r(NULL, "&", &savequestion); // parse again to get rid of response header
    while (question != NULL)
    {
        qid = strtok(question, ":");
        type = strtok(NULL, ":");
        addq_to_hashtable(student_name, qnum, qid, type);
        qnum++;
        question = strtok_r(NULL, "&", &savequestion);
    }
    free(p_response);
    return 0;
}

/*  Increments the tracker of the current question a student
 *  is viewing on their webpage, takes student name as parameter
 */
void increment_question(char *student_name)
{
    uint32_t h = hash_string(student_name) % HASHTABLE_SIZE;
    hashtable[h]->currentq += 1;
}

/*  If question was marked correct, call this function to update the
    hashtable (and call write to CSV to make sure data is saved)
    Takes the student name and the question ID of the correct answer
*/
void answer_correct(char *student_name, int qid)
{
    uint32_t h = hash_string(student_name) % HASHTABLE_SIZE;
    for (int i = 0; i < NUM_QUESTIONS; i++)
        if (hashtable[h]->qid[i] == qid)
            hashtable[h]->correct[i] = true;
    writeToCSV(hashtable, &numStudents, studentNames, FILEPATH);
}

/*  If question was marked incorrect, call this function to update the
    hashtable (and call write to CSV to make sure data is saved)
    Takes the student name and the question ID of the incorrect answer
*/
void answer_incorrect(char *student_name, int qid)
{
    uint32_t h = hash_string(student_name) % HASHTABLE_SIZE;
    for (int i = 0; i < NUM_QUESTIONS; i++) {
        if (hashtable[h]->qid[i] == qid) {
            hashtable[h]->attemptsLeft[i] = hashtable[h]->attemptsLeft[i] - 1;
            break;
        }
        else continue;
    }
    writeToCSV(hashtable, &numStudents, studentNames, FILEPATH);
}

/**
 * Pings the QB specified at the socket, if it is still connected
 * it wont change anything, but if it has disconnected it will
 * update the qb_info array to say there is a slot available
 * for a new QB to connect
*/
void ping_QB(SOCKET socket, int qb_num)
{
    char *ping = "TM\r\nPING\r\n\r\n";
    char response[MAXDATASIZE + 1];

    if (send(socket, ping, strlen(ping) + 1, 0) == -1)
    {
        perror("QB disconnected");
        qb_info[qb_num].type = NONE;
        return;
    }

    printf("QB is connected.\n");

    if (recv(socket, response, 4096, 0) <= 0)
    {
        perror("QB Disconnection");
        qb_info[qb_num].type = NONE;
        return;
    }
    printf("Received from QB: %s\n", response);

    return;
}

/**
 * Connects to a QB at socket, of type (PYTHON or C)
 * if there is space to connect to it (as can only connect
 * to a specified number of QBs at a time)
 * 
 * Pings both QB slots to make sure there is space available
 * Returns -1 if cannot connect, 0 if it can
*/
int connect_QB(SOCKET socket, enum QBType type)
{
    // check any existing connections to ensure no QB has disconnected
    bool space_available = false;
    for (int i = 0; i < NUM_QB; i++)
    {
        ping_QB(qb_info[i].socket, i);
        if (qb_info[i].type == NONE)
            space_available = true;
    }

    // assign QB to an open space
    if (space_available)
    {
        for (int i = 0; i < NUM_QB; i++)
        {
            if (qb_info[i].type == NONE)
            {
                qb_info[i].socket = socket;
                qb_info[i].type = type;
                break;
            }
        }
    }
    else
    {
        printf("Cannot assign question bank: Already connected to 2 QBs\n");
        return -1;
    }
    printf("QB INFORMATION:\nQB SLOT 1 TYPE: %d\nQB SLOT 2 TYPE: %d\n", qb_info[0].type, qb_info[1].type);
    return 0;
}

/**
 * Queries the QB for the question specified by qid from the
 * corresponding QB - and returns the string
 * Odd qid = Python question, even qid = C question
*/
char *get_question(int qid)
{
    char request[64];
    char *response = calloc(1, MAXDATASIZE + 1);
    CHECK_ALLOC(response);
    sprintf(request, "GETQUESTION\r\n%i", qid);
    if (qid % 2 == 1)
    {
        // Question is a Python question, so ask from a Python QB
        for (int i = 0; i < NUM_QB; i++)
        {
            ping_QB(qb_info[i].socket, i);
            if (qb_info[i].type == PYTHON)
            {
                // send/receive request
                if (send(qb_info[i].socket, request, strlen(request) + 1, 0) == -1)
                {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                if (recv(qb_info[i].socket, response, 4096, 0) <= 0)
                {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    else
    {
        // Question is a C question, so ask from a C QB
        for (int i = 0; i < NUM_QB; i++)
        {
            ping_QB(qb_info[i].socket, i);
            if (qb_info[i].type == C)
            {
                // send/receive request
                if (send(qb_info[i].socket, request, strlen(request) + 1, 0) == -1)
                {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                if (recv(qb_info[i].socket, response, 4096, 0) <= 0)
                {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    // Handle Response - strtok twice to get question
    char *question = strtok(response, "\r\n");
    strtok(NULL, "\r\n");
    question = strtok(NULL, "\0");
    return question;
}

/**
 * Gets the answer to a question specified by qid
 * from the corresponding QB, returns a char * to
 * that question
*/
char *get_answer(int qid)
{
    char request[64];
    char *response = calloc(1, MAXDATASIZE + 1);
    CHECK_ALLOC(response);
    sprintf(request, "ANSWER\r\n%i", qid);
    if (qid % 2 == 1)
    {
        // Question is a Python question, so ask from a Python QB
        for (int i = 0; i < NUM_QB; i++)
        {
            ping_QB(qb_info[i].socket, i);
            if (qb_info[i].type == PYTHON)
            {
                // send/receive request
                if (send(qb_info[i].socket, request, strlen(request) + 1, 0) == -1)
                {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                if (recv(qb_info[i].socket, response, 4096, 0) <= 0)
                {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    else
    {
        // Question is a C question, so ask from a C QB
        for (int i = 0; i < NUM_QB; i++)
        {
            ping_QB(qb_info[i].socket, i);
            if (qb_info[i].type == C)
            {
                // send/receive request
                if (send(qb_info[i].socket, request, strlen(request) + 1, 0) == -1)
                {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                if (recv(qb_info[i].socket, response, 4096, 0) <= 0)
                {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    // Handle Response - strtok twice to get question

    char *answer = strstr(response, "\r\n\r\n") + 4;
    // remove final padding
    answer[strlen(answer) - 3] = 0;
    // printf("GOT answer FOR QID %i: %s\n", qid, answer);
    return answer;
}

/**
 * Queries the correct QB if the student answer ans
 * for the question qid is correct or not
 * Returns char 1 if correct, char 0 if incorrect
*/
char get_mark(int qid, char *ans)
{
    char request[MAXDATASIZE];
    char *response = calloc(1, MAXDATASIZE + 1);
    CHECK_ALLOC(response);
    sprintf(request, "MARK\r\n%i:%s", qid, ans);
    if (qid % 2 == 1)
    {
        // Question is a Python question, so ask from a Python QB
        for (int i = 0; i < NUM_QB; i++)
        {
            ping_QB(qb_info[i].socket, i);
            if (qb_info[i].type == PYTHON)
            {
                // send/receive request
                if (send(qb_info[i].socket, request, strlen(request) + 1, 0) == -1)
                {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                if (recv(qb_info[i].socket, response, 4096, 0) <= 0)
                {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    else
    {
        // Question is a C question, so ask from a C QB
        for (int i = 0; i < NUM_QB; i++)
        {
            ping_QB(qb_info[i].socket, i);
            if (qb_info[i].type == C)
            {
                // send/receive request
                if (send(qb_info[i].socket, request, strlen(request) + 1, 0) == -1)
                {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                if (recv(qb_info[i].socket, response, 4096, 0) <= 0)
                {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    char *mark = strtok(response, "\r\n");
    mark = strtok(NULL, "\r\n");
    mark = strtok(NULL, "\r\n");
    char score = *(mark + 5);

    return score;
}

void handle_question_increase(SOCKET socket, char *student_name)
{
    // check both QBs are connected first
    if (qb_info[0].socket == 0 || qb_info[1].socket == 0)
    {
        send_QB_disconnected(socket);
    }
    // Get first question of the student's test
    TESTINFO *student = hashtable_get(hashtable, student_name);
    char *next_question = get_question(student->qid[student->currentq]);
    char *format_question = calloc(1, (strlen(next_question) + 5) * sizeof(char)); // + 4 for question number, +1 for null terminator
    CHECK_ALLOC(format_question);
    sprintf(format_question, "%i. %s", student->currentq + 1, next_question);
    if (student->currentq <= NUM_QUESTIONS && student->currentq != NUM_QUESTIONS - 1)
    {
        increment_question(student_name);
    }
    else
        send_307(socket);

    student = hashtable_get(hashtable, student_name);

    if (student->qid[student->currentq - 1] >= 100)
    {
        // Programming Question
        send_webpage(socket, format_question, first_input, last_input);
    }
    else
    {
        // Multi Choice Question
        send_webpage(socket, format_question, first_multi, last_multi);
    }
}

void handle_get(SOCKET socket, HTTPRequest request)
{
    char *path = request.request_line.search(&request.request_line, "uri", strlen("uri") + 1);
    if (strstr(path, ".."))
    {
        send_400(socket);
        return;
    }
    if (strlen(path) > 100)
    {
        send_400(socket);
        return;
    }

    if (strcmp(path, "/") == 0)
    {
        path = "/index.html";
    }
    else
    {
        char *cookie = request.header_fields.search(&request.header_fields, "Cookie", strlen("Cookie") + 1);
        char *student_name;
        if (cookie != NULL)
            student_name = cookie + 5; // plus 5 because cookie starts with 'user=XXXXX'
        if (cookie && strcmp(path, "/login") == 0)
        {
            char new_path[124];
            sprintf(new_path, "/profile/%s", cookie + 5);
            strcat(cookie, ";");
            send_302(socket, new_path, cookie);
        }
        else if (strcmp(path, "/login") == 0 && cookie == NULL)
        {
            path = "/login.html";
        }

        if (strcmp(path, "/logout") == 0 && cookie != NULL)
        {
            strcat(cookie, "; expires=Thu, 01 Jan 1970 00:00:00 GMT");
            send_302(socket, "/", cookie);
            return;
        }
        else if (strcmp(path, "/logout") == 0 && cookie == NULL)
        {
            send_302(socket, "/", "user=; expires=Thu, 01 Jan 1970 00:00:00 GMT");
            return;
        }
        if (strstr(path, "/profile/") != NULL)
        {
            strtok(path, "/");
            strcat(path, "/index.html");
        }
        if (strcmp(path, "/quiz") == 0 && cookie != NULL)
        {
            // CHECK IF STUDENT HAS QUESTIONS OR NOT YET
            TESTINFO *student = hashtable_get(hashtable, student_name);
            if (student->qid[0] == 0)
            {
                if (populate_questions(student_name) == -1)
                { // populate student qids and types
                    printf("Cannot retrieve questions: Missing QB Connection\n");
                    send_QB_disconnected(socket);
                }
                else
                    writeToCSV(hashtable, &numStudents, studentNames, FILEPATH); // update csv with info
            }
            student = hashtable_get(hashtable, student_name); // regenerate information again
            char *summary_centre = calloc(1, MAXDATASIZE - strlen(summary_start) - strlen(summary_end));
            CHECK_ALLOC(summary_centre);
            // calculate current total marks of student
            // Total marks is score of attempts left if they have it correct
            // 3 attempts left, when question is correct, is 3 marks
            int totalMarks = 0;
            for (int i = 0; i < NUM_QUESTIONS; i++)
                if (student->correct[i] == true)
                    totalMarks += student->attemptsLeft[i];
            // create summary screen text
            sprintf(summary_centre, "<h1>Marks Summary</h1><hr><h2>Total Marks: %i</h2><hr>", totalMarks);
            char questionstring[128];
            for (int i = 0; i < NUM_QUESTIONS; i++)
            {
                sprintf(questionstring, "<h2>Question %i:</h2><div><p>Attempts Left: %i</p><p>Answered Correctly?   %s</p></div><hr>",
                        i + 1, student->attemptsLeft[i], (student->correct[i] == true) ? "True" : "False");
                strcat(summary_centre, questionstring);
            }
            send_webpage(socket, summary_centre, summary_start, summary_end);
            free(summary_centre);
            return;
        }
        if (strcmp(path, "/quiz/start") == 0)
        {
            handle_question_increase(socket, student_name);
            return;
        }
    }
    char full_path[128];
    sprintf(full_path, "public%s", path);
    printf("\nfull path: %s\n", full_path);
    FILE *fp = fopen(full_path, "rb");

    if (!fp)
    {
        send_404(socket);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    size_t cl = ftell(fp);
    rewind(fp);
    const char *ct = get_content_type(full_path);

    char buffer[BSIZE];

    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    send(socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Connection: close\r\n");
    send(socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Length: %zu\r\n", cl);
    send(socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Type: %s\r\n", ct);
    send(socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "\r\n");
    send(socket, buffer, strlen(buffer), 0);

    size_t r = fread(buffer, 1, BSIZE, fp);
    while (r)
    {
        send(socket, buffer, r, 0);
        r = fread(buffer, 1, BSIZE, fp);
    }
    fclose(fp);

    // http_request_destructor(&request);
}

void handle_post(HTTPRequest response, SOCKET socket)
{
    char *url = (char *)response.request_line.search(&response.request_line, "uri", strlen("uri") + 1);
    if (strcmp(url, "/login") == 0)
    {
        char *username = (char *)response.body.search(&response.body, "username", strlen("username") + 1);
        char *password = (char *)response.body.search(&response.body, "password", strlen("password") + 1);
        printf("\n%s is attempting to sign in with the password %s\n", username, password);
        TESTINFO *student = hashtable_get(hashtable, username);
        // stops segfault
        if (student == NULL)
        {
            printf("Student doesnt exist\n");
            send_418(socket);
            return;
        }
        else
        {
            if (strcmp(username, student->user) == 0 && strcmp(password, student->pw) == 0)
            {
                printf("Sign in success\n");
                char *path = calloc(1, strlen(student->user) + 1 + strlen("/profile/") + 1);
                char *cookie = calloc(1, strlen(student->user) + 1 + strlen("user=") + 1);
                CHECK_ALLOC(path);
                CHECK_ALLOC(cookie);
                sprintf(path, "/profile/%s", username);
                sprintf(cookie, "user=%s", username);
                send_302(socket, path, cookie);
                free(path);
                free(cookie);
            }
            else
            {
                printf("\n\ndoenst match\n");
                send_418(socket);
            }
        }
    }
    if (strcmp(url, "/quiz/start") == 0)
    {
        char *cookie = response.header_fields.search(&response.header_fields, "Cookie", strlen("Cookie") + 1);
        char *user = cookie + 5;
        TESTINFO *student = hashtable_get(hashtable, user);
        int current_question = student->currentq;
        char *student_answer = response.body.search(&response.body, "sans", strlen("sans") + 1);
        char *actual = student_answer;
        if (student->correct[student->currentq - 1] != true && student->attemptsLeft[student->currentq - 1] > 0)
        {
            if (get_mark(student->qid[current_question - 1], actual) == '1')
            {
                printf("Student answered correctly\n");
                answer_correct(student->user, student->qid[current_question - 1]);
            }
            else
            {
                answer_incorrect(student->user, student->qid[current_question - 1]);
                send_418(socket);
            }
        }
        else
        { 
            /* TO DO BEN FINISH */
            // char *answer = get_answer(student->qid[student->currentq]);
            printf("triggered\n\n\n");
            send_201(socket);
            return;
        }
    }
    // http_request_destructor(&response);
}

void parse_request(char *response_string, SOCKET socket)
{
    HTTPRequest response;

    char duplicate[strlen(response_string) + 1];
    strcpy(duplicate, response_string);

    for (int i = 0; i < (int)strlen(response_string) - 1; i++)
    {
        if (response_string[i] == '\n' && response_string[i + 1] == '\n')
        {
            response_string[i + 1] = '|';
        }
    }
    char *request_line = strtok(response_string, "\r\n");
    char *header_fields = strtok(NULL, "|");
    char *body = strstr(duplicate, "\r\n\r\n") + 4;

    extract_header_fields(&response, header_fields);
    extract_request_line_fields(&response, request_line);
    extract_body(&response, body);
    char *method = (char *)response.request_line.search(&response.request_line, "method", strlen("method"));
    if (strcmp(method, "GET") == 0)
    {
        handle_get(socket, response);
    }
    else if (strcmp(method, "POST") == 0)
    {
        handle_post(response, socket);
    }
    else
    {
        send_501(socket);
    }
}

void received(int new_fd, int numbytes, char *buf)
{
    int client_received = 0;
    if (numbytes < 1)
    {
        fprintf(stderr, "Unexpected disconnect from client.\n");
        drop_client(new_fd);
    }
    else
    {
        char original[strlen(buf) + 1];
        strcpy(original, buf);
        client_received += numbytes;
        buf[client_received] = 0;
        char *res = strstr(buf, "\r\n\r\n");
        // ! double check importance of this if statements and nested stuff later
        if (res)
        {
            *res = 0;
            if (strncmp(buf, "GET", 3) == 0)
            {
                char *path = buf + 4;
                char *end_path = strstr(path, " ");
                // printf("%s", end_path);
                if (!end_path)
                {
                    send_400(new_fd);
                }
                else
                {
                    *end_path = 0;
                    parse_request(original, new_fd);
                }
            }
            else if (strncmp(buf, "POST", 4) == 0)
            {
                parse_request(original, new_fd);
            }
            else if (strncmp(buf, "QB", 2) == 0)
            {
                enum QBType type;
                // See if QB is Python or C
                if (strstr(buf, "PYTHON") != NULL)
                    type = PYTHON;
                else
                    type = C;
                // Try to connect to QB
                if (connect_QB(new_fd, type) == -1)
                {
                    perror("Failed to connect to new QB");
                }
            }
            else
            {
                // todo  unknown request figure out how to handle
                send_400(new_fd);
            }
        }
    }
}

void manage_connection(SOCKET sockfd)
{

    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    fd_set current_sockets, ready_sockets;
    int new_fd;
    ssize_t numbytes;
    char *buf;

    FD_ZERO(&current_sockets);
    FD_SET(sockfd, &current_sockets);

    printf("Server: waiting for connections...\n\n");

    while (1)
    {
        ready_sockets = current_sockets;
        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {
                if (i == sockfd)
                {
                    sin_size = sizeof their_addr;
                    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
                    if (new_fd == -1)
                    {
                        fprintf(stdout, "Client connection failed\n");
                        continue;
                    }
                    FD_SET(new_fd, &current_sockets);
                }
                else
                {
                    buf = calloc(1, MAXDATASIZE + 1);
                    CHECK_ALLOC(buf);
                    if ((numbytes = recv(i, buf, MAXDATASIZE + 1, 0)) == -1)
                    {
                        perror("recv");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        received(i, numbytes, buf);
                    }
                    FD_CLR(i, &current_sockets);
                    free(buf);
                }
            }
        }
    }
}

SOCKET bind_socket(struct addrinfo *servinfo)
{
    struct addrinfo *p;
    struct sigaction sa;
    SOCKET sockfd;
    int yes = 1;
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        freeaddrinfo(servinfo);

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    return sockfd;
}

struct addrinfo *get_info(char *port)
{
    int status;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    return res;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage();
    }
    // Read in the data of students from a csv
    hashtable = hashtable_new();
    getData(hashtable, &numStudents, &studentNames, FILEPATH);
    // Allocation for Question Banks
    qb_info = (QBInformation *)calloc(NUM_QB, sizeof(struct QBInformation));
    CHECK_ALLOC(qb_info);
    for (int i = 0; i < NUM_QB; i++)
    {
        qb_info[i].socket = 0;
        qb_info[i].type = NONE;
    }
    for (int i = 0; i < numStudents; i++)
    {
        TESTINFO *student = hashtable_get(hashtable, studentNames[i]);
        printf("Student data for %s loaded in\n", student->user);
    }
    printf("\n");
    // Server stuff idk
    SOCKET socket = bind_socket(get_info(argv[1]));

    manage_connection(socket);
    // writes any changed data back to the csv when finished
    writeToCSV(hashtable, &numStudents, studentNames, FILEPATH);
    return 0;
}
