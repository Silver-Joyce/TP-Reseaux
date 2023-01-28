#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 1234
#define BUFFLEN 1024

void stop(char * msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) stop("Error creating socket");
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(socket_fd, (const struct sockaddr *)(&server_addr), sizeof(server_addr)) == -1) {
        stop("Error connecting to server");
    } 
    char buffer[BUFFLEN];
    buffer[BUFFLEN-1] = '\0';
    for (int i=0; i<1000; ++i) {
        if (send(socket_fd, "ECHO\0", 5, 0) < 0) stop("Error sending message to server");
        if (recv(socket_fd, buffer, BUFFLEN-1, 0) < 0) stop("Error receiving message from server");
        fputs(buffer, stdout);
        fputs("\n", stdout);
        memset(buffer, 0, BUFFLEN-1);
    }
    close(socket_fd);
}