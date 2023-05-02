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

uint32_t htonf(float f)
{
    uint32_t p;
    uint32_t sign;
    if (f < 0)
    {
        sign = 1;
        f = -f;
    }
    else
    {
        sign = 0;
    }

    p = ((((uint32_t)f) & 0x7fff) << 16 | (sign << 31));
    p |= (uint32_t)(((f - (int)f) * 65536.0f)) & 0xffff;

    return p;
}

float ntohf(uint32_t p)
{
    float f = ((p >> 16) & 0x7fff);
    f += (p & 0xffff) / 65536.0f;

    if (((p >> 31) & 0x1) == 0x1)
    {
        f = -f;
    }

    return f;
}

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

// void idk3()
// {
//     struct addrinfo hints, *res;
//     int sockfd;
//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE;

//     getaddrinfo(NULL, "3490", &hints, &res);

//     sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//     //!  dont bind a port number lowes than 1024->65535
//     bind(sockfd, res->ai_addr, res->ai_addrlen);

//     // int yes = 1;
//     // if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
//     // {
//     //     perror("setsockopt");
//     //     exit(1);
//     // }
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

    float f = 3.1415926, f2;
    uint32_t netf;
    netf = htonf(f);
    f2 = ntohf(netf);

    printf("orgin: %f\n", f);
    printf("network: 0x%08x\n", netf);
    printf("unpacket: %f\n", f2);

    return 0;
}