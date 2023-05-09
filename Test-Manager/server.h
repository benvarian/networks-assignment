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

#include "Data-Structures/Dictionary/Dictionary.h"

#define BACKLOG 10
#define MAXDATASIZE 4095
#define BSIZE 1024
#define SOCKET int

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

typedef struct HTTPRequest {
    struct Dictionary request_line;
    struct Dictionary header_fields;
    struct Dictionary body;
} HTTPRequest;

// define functions
void usage(void);

void drop_client(SOCKET socket);

const char *get_content_type(const char *path);

void sigchld_handler(int s);

void *get_in_addr(struct sockaddr *sa);

void send_200(SOCKET socket);

void send_400(SOCKET socket);

void send_404(SOCKET socket);

void connection_get(SOCKET socket, const char *path, const char *IPv6_Address);

HTTPRequest connection_post(int socket, char *buf, char *args);

void received(int new_fd, int numbytes, char *buf, const char *IPv6_Address);

void manage_connection(SOCKET sockfd);

SOCKET bind_socket(struct addrinfo *servinfo);

struct addrinfo *get_info(char *port);

void http_request_destructor(HTTPRequest *request);

void extract_request_line_fields(HTTPRequest *request, char *request_line);
void extract_header_fields(HTTPRequest *request, char *header_fields);
void extract_body(HTTPRequest *request, char *body);
