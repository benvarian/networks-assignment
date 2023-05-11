
// Client side C/C++ program to demonstrate Socket
// programming
#include "QBclient.h"
#define PORT 65433

char* hello = "Hello from client";
int status = 0;
int valread = 0;
int client_fd = 0;

struct sockaddr_in serv_addr;
char buffer[4096] = { 0 };

void closeConnection() {
    printf("closing...\n\n");
    close(client_fd);
}

void receive() {
    bool received = read(client_fd, buffer, 4096);
    if(!received) {
        printf("socket connection broken. Ending process.");
        closeConnection();
        exit(EXIT_FAILURE);
    }
}

int sendStr(char *str) {
    valread = send(client_fd, str, strlen(str), 0);
    printf("sent %i bytes\n", valread);

    return valread;
}

void req_questions(char type, uint16_t num) {
    char buf[] = {'q', (char)type, (char)num & 0x00FF, (char)num>>8};
    send(client_fd, buf, 4, 0);
}

int main(int argc, char const* argv[])
{
    (void) argc;
    (void) argv;

    // creating socket
    while ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error...\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
  
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
  

    while ((status
         = connect(client_fd, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed, trying again in 1 seconds.\n\n");
        // sleeps 5 s in linux or 5 ms in windows -_-
        sleep(1);
    }

    // connection is up and running -- ready to talk to the echo server.
    int count = 0;
    sleep(1);
    while(count < 3) {
        // send a request
        req_questions('c', count + 2);

        // receive a reply of length
        receive();
        uint16_t recLength = *(uint16_t *)&buffer[1];
        printf("got: type: %c \n length: %i\n with val: %s\n from server\n\n", buffer[0], recLength, buffer + 3);
        count++;
    }

    // closing the connected socket
    closeConnection();

    return 0;
}