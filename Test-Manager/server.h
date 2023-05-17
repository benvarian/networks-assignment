#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>
#include <time.h>

#include "Data-Structures/Dictionary/Dictionary.h"

#define BACKLOG 10
#define MAXDATASIZE 4095
#define BSIZE 1024
#define SOCKET int
#define NUM_QB  2

enum HTTPMethods
{
    GET,
    POST,
    PUT,
    HEAD,
    PATCH,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
};

enum QBType
{
    NONE,
    PYTHON,
    C
};

typedef struct HTTPRequest
{
    struct Dictionary request_line;
    struct Dictionary header_fields;
    struct Dictionary body;
} HTTPRequest;

typedef struct QBInformation
{
    int socket;
    enum QBType type;
} QBInformation;

// define functions
void usage(void);

void drop_client(SOCKET socket);

const char *get_content_type(const char *path);

void sigchld_handler(int s);

void *get_in_addr(struct sockaddr *sa);

void send_200(SOCKET socket);

void send_400(SOCKET socket);

void send_404(SOCKET socket);

void handle_get(SOCKET socket, HTTPRequest request);

// todo change back to HTTPRequest
void handle_post(HTTPRequest response, SOCKET socket);

void parse_request(char *response_string, SOCKET socket);

void received(int new_fd, int numbytes, char *buf);

void manage_connection(SOCKET sockfd);

SOCKET bind_socket(struct addrinfo *servinfo);

struct addrinfo *get_info(char *port);

void http_request_destructor(HTTPRequest *request);

void extract_request_line_fields(HTTPRequest *request, char *request_line);

void extract_header_fields(HTTPRequest *request, char *header_fields);

void extract_body(HTTPRequest *request, char *body);

void handle_post(HTTPRequest response, SOCKET socket);

void addq_to_hashtable(char *student_name, int qnum, char *qid, char *type);

void ping_QB(SOCKET socket, int qb_num);

int connect_QB(SOCKET socket, enum QBType type);

int get_questions(char *student);
