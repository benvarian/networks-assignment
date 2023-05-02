#include <stdio.h>

#include "structs.h"

// int getaddrinfo()
// {
//     int status;
//     struct addrinfo hints;
//     struct addrinfo *servinfo;
//     memset(&hints, 0, sizeof hints); // makes sure the struct is empty
//     hints.ai_family = AF_UNSPEC; // either ipv4 or 6
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE;

//     if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0)
//     {
//         fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
//         exit(1);
//     }
//     freeaddrinfo(servinfo);
// }

void idk2()
{
    int s;
    struct addrinfo hints, *res;
    // todo error checking and walk through the res linked list looking for valid entries
    getaddrinfo("www.example.com", "http", &hints, &res);
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // connect example
    // connect(sockfd, res->ai_addr, res->ai_addrlen);
}

void idk3()
{
    struct addrinfo hints, *res;
    int sockfd;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, "3490", &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    //!  dont bind a port number lowes than 1024->65535
    bind(sockfd, res->ai_addr, res->ai_addrlen);

    // int yes = 1;
    // if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    // {
    //     perror("setsockopt");
    //     exit(1);
    // }
}

#define MYPORT "3490"
#define BACKLOG 10
int main(void)
{

    // sequence of calls
    /*
     * getaddrinfo()
     * socket()
     * bind()
     * listen()
     */
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL,MYPORT, &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    return 0;
}