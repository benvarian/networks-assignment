#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <poll.h>

// #include "structs.h"

#define MYPORT "3490"
#define BACKLOG 10

// uint32_t htonf(float f)
// {
//     uint32_t p;
//     uint32_t sign;
//     if (f < 0)
//     {
//         sign = 1;
//         f = -f;
//     }
//     else
//     {
//         sign = 0;
//     }

//     p = ((((uint32_t)f) & 0x7fff) << 16 | (sign << 31));
//     p |= (uint32_t)(((f - (int)f) * 65536.0f)) & 0xffff;

//     return p;
// }

// float ntohf(uint32_t p)
// {
//     float f = ((p >> 16) & 0x7fff);
//     f += (p & 0xffff) / 65536.0f;

//     if (((p >> 31) & 0x1) == 0x1)
//     {
//         f = -f;
//     }

//     return f;
// }

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

// void idk2()
// {
//     int s;
//     struct addrinfo hints, *res;
//     // todo error checking and walk through the res linked list looking for valid entries
//     getaddrinfo("www.example.com", "http", &hints, &res);
//     s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

//     // connect example
//     // connect(sockfd, res->ai_addr, res->ai_addrlen);
// }


int main(void)
{

    // sequence of calls
    /*
     * getaddrinfo()
     * socket()
     * bind()
     * listen()
     */
    // struct sockaddr_storage their_addr;
    // socklen_t addr_size;
    // struct addrinfo hints, *res;
    // int sockfd, new_fd;

    // memset(&hints, 0, sizeof hints);
    // hints.ai_family = AF_UNSPEC;
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE;

    // getaddrinfo(NULL, MYPORT, &hints, &res);

    // sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    // bind(sockfd, res->ai_addr, res->ai_addrlen);
    // listen(sockfd, BACKLOG);

    // addr_size = sizeof their_addr;
    // new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    // return 0;

    return 0;
}