
// Client side C/C++ program to demonstrate Socket
// programming
#include "QBclient.h"
#define PORT 65435
#define HANDSHAKE "are you ready to receive the questions?"
#define HANDSHAKE_FAILED "fail"
#define HANDSHAKE_ACCEPTED "yes."
#define HANDSHAKE_REJECTED "no.."
#define HANDSHAKE_VERIF_LENGTH 4
#define HANDSHAKE_LENGTH strlen(HANDSHAKE)

char* hello = "Hello from client";
int status = 0;
int valread = 0;
int client_fd = 0;

struct sockaddr_in serv_addr;
char buffer[1024] = { 0 };

void closeConnection() {
    printf("closing...");
    close(client_fd);
}

void receiveStr(int length) {
    int bytesRead = 0;
    int received = 0;
    while (bytesRead < length) {
        received = read(client_fd, &buffer[bytesRead], 1024);
        bytesRead += received;
        if(!received) {
            printf("socket connection broken. Ending process.");
            closeConnection();
            exit(EXIT_FAILURE);
        }
    }
}

int sendStr(char *str) {
    valread = send(client_fd, str, strlen(str), 0);
    printf("sent %i bytes\n", valread);

    return valread;
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
    bool run = true;
    // int count = 0;
    // char *response;
    
    while(run) {
        // send a request
        sendStr("c");

        // receive a reply of length
        receiveStr(2);
        sendStr("ack");
        uint16_t a = (uint16_t)buffer[0];
        receiveStr(a);
        printf("got: \n\n %s \n\n from server\n", buffer);
        sendStr("ack");
        break;
    }

    // closing the connected socket
    closeConnection();

    return 0;
}