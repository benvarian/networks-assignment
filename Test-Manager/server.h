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

#define BACKLOG 10
#define MAXDATASIZE 4095
#define BSIZE 1024
#define SOCKET int

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

void connection_post(int socket, char *buf, char *args);

void received(int new_fd, int numbytes, char *buf, const char *IPv6_Address);

void manage_connection(SOCKET sockfd);

SOCKET bind_socket(struct addrinfo *servinfo);

struct addrinfo *get_info(char *port);
