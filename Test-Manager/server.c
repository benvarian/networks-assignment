/**
** @file server.c
** @brief implementation of the server required for test-manager
**/

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
#define MAXDATASIZE 100

void usage()
{
    fprintf(stderr, "usage: ./server port\n");
    exit(EXIT_FAILURE);
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

void received(int new_fd, char *buf, int size)
{
    int count = 0;
    for (int i = 0; i < size; i++) {
        if(buf[i] == '\0') {
            count++;
        }
    }
    printf("server: received '%s' with size of %d and empty bytes amount been %d\n", buf, size, count);
}


void send_message(int new_fd, char *buf, int size)
{
    if (send(new_fd, buf, size, 0) == -1)
        perror("send");

    printf("sent %s to connection %d\n", buf, new_fd);
}

void accept_connection(int sockfd)
{

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int new_fd, numbytes;
    char s[INET6_ADDRSTRLEN];
    char buf[MAXDATASIZE];

    printf("server: waiting for connections...\n");

    while (1)
    { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }
        // todo will leave in for now, but will remove later basically just debugging
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        // end
        if (!fork())
        {                  // this is the child process
            close(sockfd); // child doesn't need the listener
            if ((numbytes = recv(new_fd, buf, sizeof buf, 0)) == -1)
            {
                perror("recv");
                exit(1);
            }
            else
            {
                received(new_fd, buf, sizeof buf);
            }

            send_message(new_fd, "Hello, world!", sizeof "Hello, world!");
            
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }
}

void bind_socket(struct addrinfo *servinfo)
{
    struct addrinfo *p;
    struct sigaction sa;
    int sockfd;
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

        break;
    }

    freeaddrinfo(servinfo);

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

    accept_connection(sockfd);
}

void get_addr_ip(char *port)
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
    bind_socket(res);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage();
    }

    get_addr_ip(argv[1]);

    return 0;
}