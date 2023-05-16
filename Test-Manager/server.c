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
                       "Location: /\r\n"
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
void send_302(SOCKET socket, const char *path, const char *username)
{
    char c302[52 + (strlen(path) + 1) + (strlen(username) + 1)];
    sprintf(c302, "HTTP/1.1 302 Found\r\nLocation: %s\r\nSet-Cookie: %s\r\n\r\n", path, username);
    send(socket, c302, strlen(c302), 0);
    drop_client(socket);
}

/*  Gets two random integers that add up to NUM_QUESTIONS
    and requests that many questions from the two QBs
    after checking both QBs are connected

    Sends the questions to be added to the hashtable

    takes the student name who is requesting the questions

    returns 0 if successful, -1 if not
*/
int get_questions(char *student, SOCKET socket)
{
    // Check connection to both TMs
    for(int i = 0; i < NUM_QB; i++) {
        if(ping_QB(qb_info[i].socket) == -1) return -1;
    }
    // Get how many questions of each language are being asked
    int c_questions = (rand() % NUM_QUESTIONS);
    int p_questions = NUM_QUESTIONS - c_questions;
    printf("Getting %i C questions and %i Python questions", c_questions, p_questions);
    char *get_example = "QUESTIONS\r\nP:2\r\n\r\n";
    char *c_response = calloc(1, MAXDATASIZE + 1);
    CHECK_ALLOC(c_response);
    char *p_response = calloc(1, MAXDATASIZE + 1);
    CHECK_ALLOC(p_response);
    // Ask for questions from QB
    for(int i = 0; i < NUM_QB; i++) {
        if(qb_info[i].type == PYTHON) {
            if (send(qb_info[i].socket, get_example, strlen(get_example) + 1, 0) == -1)
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
        else if(qb_info[i].type == C) {
            if (send(qb_info[i].socket, get_example, strlen(get_example) + 1, 0) == -1)
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
    printf("C response: \n%s\n\n Python response:\n%s\n\n", c_response, p_response);
    free(c_response);
    free(p_response);
    send_404(socket);
    return 0;
}

int ping_QB(SOCKET socket)
{

    /*
     * first, check that the qb is still connected, if not quit under exit_failure
     * if return is say 1, then the qb is still connected, so proceed with execution,
     * add to some form of struct so can refernec later
     * send the get questions command to the qb
     * parse
     * serve html
     */
    char *ping = "TM\r\nPING\r\n\r\n";
    char response[MAXDATASIZE + 1];

    if (send(socket, ping, strlen(ping) + 1, 0) == -1)
    {
        perror("QB disconnected");
        return -1;
    }

    printf("QB is connected.\n");

    if (recv(socket, response, 4096, 0) <= 0)
    {
        perror("QB Disconnection");
        return -1;
    }
    printf("Received from QB: %s\n", response);

    return 0;
}

int connect_QB(SOCKET socket, enum QBType type) {
    // first make sure qb is alive
    if (ping_QB(socket) != 0) {
        perror("Cannot connect to QB");
        return -1;
    }
    
    // check any existing connections to ensure no QB has disconnected
    bool space_available = false;
    for(int i = 0; i < NUM_QB; i++) {
        if (ping_QB(qb_info[i].socket) == -1) {
            qb_info[i].type = NONE;
            space_available = true;
        }
    }
    // assign QB to an open space
    if(space_available) {
        for(int i = 0; i < NUM_QB; i++) {
            if(qb_info[i].type == NONE) {
                qb_info[i].socket = socket;
                qb_info[i].type = type;
                break;
            }
        }
    }
    else {
        printf("Cannot assign question bank: Already connected to 2 QBs\n");
        return -1;
    }
    printf("QB INFORMATION:\nQB SLOT 1 TYPE: %d\nQB SLOT 2 TYPE: %d\n", qb_info[0].type, qb_info[1].type);
    return 0;
}

void handle_get(SOCKET socket, HTTPRequest request)
{
    char *path = request.request_line.search(&request.request_line, "uri", strlen("uri"));
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
        char *cookie = request.header_fields.search(&request.header_fields, "Cookie", strlen("Cookie"));

        if (cookie && strcmp(path, "/login") == 0)
        {
            // makes login a protected path
            send_302(socket, "/", cookie);
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
        // ?  need this as we arent gonna dynam render pages for each user as dont have enuf time
        if (strstr(path, "/profile/") != NULL)
        {
            strtok(path, "/");
            strcat(path, "/index.html");
        }
        if (strcmp(path, "/quiz") == 0)
        {
            /* todo
            ** 1. send request to qb for question/questions
            ** 2. render page with questions
            */
            strcat(path, "/index.html");
        }
        if (strcmp(path, "/quiz/start") == 0)
        {
            // check both QBs are connected first
            if (qb_info[0].socket == 0 || qb_info[1].socket == 0)
            {
                send_400(socket);
            }

            //get_questions(qb_info.socket, socket);
            return;
            // strcat(path, ".html");
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
    char *url = (char *)response.request_line.search(&response.request_line, "uri", strlen("uri"));
    if (strcmp(url, "/login") == 0)
    {
        char *username = (char *)response.body.search(&response.body, "username", strlen("username") * sizeof(char) + 1);
        char *password = (char *)response.body.search(&response.body, "password", strlen("password") * sizeof(char) + 1);
        printf("\n%s is attempting to sign in with the password %s\n", username, password);
        TESTINFO *student = hashtable_get(hashtable, username);
        printf("Checking %s against %s and %s against %s\n", username, student->user, password, student->pw);
        if (strcmp(username, student->user) == 0 && strcmp(password, student->pw) == 0)
        {
            // ! assume that the username isnt longer than 18 characters
            // todo maybe change path to pointer and get it to work with sprintf or strcat
            printf("Sign in success\n");
            char *path = calloc(1, strlen(student->user) * sizeof(char) + sizeof("/profile/%s") + 1);
            char *cookie = calloc(1, strlen(student->user) * sizeof(char) + sizeof("user=%s") + 1);
            CHECK_ALLOC(path);
            CHECK_ALLOC(cookie);
            sprintf(path, "/profile/%s", username);
            sprintf(cookie, "user=%s", username);
            send_302(socket, path, cookie);
            free(path);
        }
        else
        {
            printf("\n\ndoenst match\n");
            send_401(socket);
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
    /*! keeping for debugging incase something happens and everything breaks
    for (int i = 0; i < response.header_fields.keys.length; i++)
    {
        printf("%s:%s\n", (char *)response.header_fields.keys.head->data, (char *)response.header_fields.search(&response.header_fields,
        (char *)response.header_fields.keys.head->data, strlen((char *)response.header_fields.keys.head->data)));
        response.header_fields.keys.head = response.header_fields.keys.head->next;
    } */
    // ! maybe get rid of
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
        // todo change to a  http response function to indicate we aint got a clue what they are doing
        // ! lmao github copilot
        printf("Method unknown\n");
    }
}

void received(int new_fd, int numbytes, char *buf)
{
    // todo maybe change up how this is dealt with as its a big messy
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
                if(strstr(buf, "PYTHON") != NULL) type = PYTHON;
                else if(strstr(buf, "C") != NULL) type = C;
                else {
                    perror("QB is of unknown language");
                }
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

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    fd_set current_sockets, ready_sockets;
    int new_fd;
    ssize_t numbytes;
    char buf[MAXDATASIZE + 1];

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
                    if ((numbytes = recv(i, buf, sizeof buf, 0)) == -1)
                    {
                        perror("recv");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        received(i, numbytes, buf);
                    }
                    FD_CLR(i, &current_sockets);
                }
            }
        }
        // sin_size = sizeof their_addr;
        // new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        // if (new_fd == -1)
        // {
        //     fprintf(stdout, "Client connection failed\n");
        //     continue;
        // }

        // if (!fork())
        // {
        //     close(sockfd);

        //     if ((numbytes = recv(new_fd, buf, sizeof buf, 0)) == -1)
        //     {
        //         perror("recv");
        //         exit(1);
        //     }
        //     else
        //     {
        //         received(new_fd, numbytes, buf);
        //     }
        //     close(new_fd);
        //     exit(0);
        // }
        // close(new_fd);
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
    qb_info = (QBInformation *) calloc(NUM_QB, sizeof(struct QBInformation));
    CHECK_ALLOC(qb_info);
    for (int i = 0; i < NUM_QB; i++) {
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
