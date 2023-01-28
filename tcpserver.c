#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 1234
#define BUFFLEN 1024

void stop(char * msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int *argc, char **argv) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (bind(socket_fd, (const struct sockaddr *)(&server_addr), sizeof(server_addr)) < 0) {
        stop("Error binding");
    }
    if (listen(socket_fd, 5) < 0) {
        stop("Error listening");
    }
    int addrlen;
    int newsocket = accept(socket_fd, (struct sockaddr *)(&server_addr), &addrlen);
    if (newsocket < 0) {
        stop("Error accepting connection");
    }
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN-1);
    buffer[BUFFLEN-1]='\0';
    for (int i=0; i<1000; ++i) {
        int n;
        if ((n = recv(newsocket, buffer, BUFFLEN-1, 0)) < 0) stop("Error receiving");
        
        fputs("Received message: ", stdout);
        fputs(buffer, stdout);
        fputs("\n", stdout);
        if (send(newsocket, buffer, BUFFLEN-1, 0) < 0) stop("Error sending message");
        /* memset(buffer, 0, BUFFLEN-1); */

    }
    close(newsocket);
    close(socket_fd);
}
