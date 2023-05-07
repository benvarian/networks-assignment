/**
** @file server.c
** @brief implementation of the server required for test-manager
**/

#include "server.h"

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
    // drop_client(socket);
}

void send_400(SOCKET socket)
{
    const char *c400 = "HTTP/1.1 400 Bad Request\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 11\r\n\r\nBad Request";
    send(socket, c400, strlen(c400), 0);
    // drop_client(socket);
}

void send_404(SOCKET socket)
{
    const char *c404 = "HTTP/1.1 404 Not Found\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 9\r\n\r\nPage Not Found";
    send(socket, c404, strlen(c404), 0);
    // drop_client(socket);
}

void connection_get(SOCKET socket, const char *path, const char *IPv6_Address)
{
    printf("Serving path: %s to: %s\n", path, IPv6_Address);

    if (strcmp(path, "/") == 0)
    {
        path = "/index.html";
    }
    if (strlen(path) > 100)
    {
        send_400(socket);
        return;
    }
    if (strstr(path, ".."))
    {
        send_400(socket);
        return;
    }
    char full_path[128];

    sprintf(full_path, "public%s", path);

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

    int r = fread(buffer, 1, BSIZE, fp);
    while (r)
    {
        send(socket, buffer, r, 0);
        r = fread(buffer, 1, BSIZE, fp);
    }
    fclose(fp);
}

void connection_post(int socket, char *buf, char *args)
{
    char *line;
    int index = 0;
    (void)socket;
    (void)buf;
    (void)args;
    while ((line = strtok_r(buf + index, "\r\n", &line)))
    {
        index = (line - buf) + strlen(line) + 2;
        char *key = NULL;
        char *value = NULL;
        if (strcmp(line, "") == 0)
            break;
        char *colon = strchr(line, ':');
        if (colon != NULL)
        {
            value = colon + 2;
        }
        
        // sscanf(line, "%s: %s", key, value);
        if (value != NULL)
            printf("%s:%s\n\n", key,value);
    }
}

void received(int new_fd, int numbytes, char *buf, const char *IPv6_Address)
{
    int client_received = 0;
    if (numbytes < 1)
    {
        fprintf(stderr, "Unexpected disconnect from client.\n");
        // drop_client(new_fd);
    }
    else
    {
        client_received += numbytes;
        buf[client_received] = 0;
        char *request = strstr(buf, "\r\n\r\n");

        if (request)
        {
            *request = 0;
            if (strncmp(buf, "GET", 3) == 0)
            {
                char *path = buf + 4;

                char *end_path = strstr(path, " ");
                if (!end_path)
                {
                    send_400(new_fd);
                }
                else
                {
                    *end_path = 0;
                    connection_get(new_fd, path, IPv6_Address);
                }
            }
            else if (strncmp(buf, "POST", 4) == 0)
            {
                connection_post(new_fd, buf, request + 4);
            }
            else
            {
                // unknown request figure out how to handle
                send_400(new_fd);
            }
        }
    }
}

void manage_connection(SOCKET sockfd)
{

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int new_fd, numbytes;
    char s[INET_ADDRSTRLEN];
    char buf[MAXDATASIZE + 1];

    printf("Server: waiting for connections...\n");

    while (1)
    {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            fprintf(stdout, "Client connection failed\n");
            continue;
        }
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);

        if (!fork())
        {
            close(sockfd);

            if ((numbytes = recv(new_fd, buf, sizeof buf, 0)) == -1)
            {
                perror("recv");
                exit(1);
            }
            else
            {
                received(new_fd, numbytes, buf, s);
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);
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

    SOCKET socket = bind_socket(get_info(argv[1]));

    manage_connection(socket);

    return 0;
}
