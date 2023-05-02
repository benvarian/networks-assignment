#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <poll.h>

struct pollfd
{
    int fd;
    short events;
    short revents;
};

struct addrinfo
{
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    size_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
};

struct sockaddr
{
    unsigned short sa_family;
    char sa_data[14];
};

struct sockaddr_in
{
    short int sin_family;
    unsigned short int sin_port; // port number
    struct in_addr sin_addr;     // internet address
    unsigned char sin_zero[8];
};

struct in_addr
{
    uint32_t s_addr;
};

struct sockaddr_storage
{
    sa_family_t ss_family;
    // padding
    char __ss_pad1[_SS_PAD1SIZE];
    int64_t __ss_align;
    char __ss_pad2[_SS_PAD2SIZE];
};

// ipv6

struct sockaddr_in6
{
    u_int16_t sin6_family;
    u_int16_t sin6_port;
    u_int32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    u_int32_t sin6_scope_id;
};

struct in6_addr
{
    unsigned char s6_addr[16];
};

// ip stuff
struct sockaddr_in sa;   // ipv4
struct sockaddr_in6 sa6; // ipv6

inet_pton(AF_INET, char *ipv4, &(sa.sin_addr));    // ipv4
inet_pton(AF_INET6, char *ipv6, &(sa6.sin6_addr)); // ipv6

// IPv4:
char ip4[INET_ADDRSTRLEN]; // space to hold the IPv4 string
struct sockaddr_in sa;     // pretend this is loaded with something

inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);

// printf("The IPv4 address is: %s\n", ip4);

// IPv6:
char ip6[INET6_ADDRSTRLEN]; // space to hold the IPv6 string
struct sockaddr_in6 sa6;
inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET_ADDRSTRLEN);
// printf("The address is: %s\n", ip6);

// functions
int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

int socket(int domain, int type, int protocol);

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

int listen(int sockfd, int backlog);

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

// set flags to 0, see man page
int send(int sockfd, const void *msg, int len, int flags);

// return value of 0 means a connectoin was closed
int recv(int sockfd, void *buf, int len, int flags);

int sendto(int sockfd, const void *msg, int len, unsigned int flags, const struct sockaddr *to, socklen_t tolen);

int recvfrom(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr *from, int *fromlen);

int shutdown(int sockfd, int how);
// returns 0 on success
// param {how}
/*
 * 0: futher receives are disallowed
 * 1: further sends are disallowed
 * 2: further sends and receives are disallowed (like close())
 */

int getpeername(int sockfd, struct sockaddr *addr, int *addrlen);
// use inet_ntop, getnameinfo, gethostbyaddr afterwards

int gethostname(char *hostname, size_t size);
// hostname arrya of chars, size if strlen
// returns 0 on success

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
