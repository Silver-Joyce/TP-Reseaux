#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include "cmd/cmd.h"

#define BUFFLEN 1024

void stop(char * msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if (argc != 4) {
        write(1, "Number of arguments is not confortable\n", 39);
        exit(1);
    }

    // printf("Strlen of argv[3]: %d", strlen(argv[3]));

    // argv[1] is ip addresss
    char *ip_addr = argv[1];

    // argv[2] is the port
    int PORT = atoi(argv[2]), valread;

    // check if the port is possible
    if (!PORT) {
        write(1, "Invalid port!\n", 14);
        exit(1);
    }

    char prefix[] = "speudo: "; 
    char *speudo = strcat(prefix, argv[3]); // argv[3] is the speudo 
    speudo = strcat(speudo, "\n");

    // buffer of client
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);

    // Create socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) stop("Error creating socket");

    // Create struct sockaddr_in
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(PORT);


    if (connect(socket_fd, (struct sockaddr *)(&server_addr), sizeof(server_addr)) < 0) stop("Error connecting to server");
    // Send speudo to server
    if ((send(socket_fd, speudo, strlen(speudo), 0) != strlen(speudo)) || (send(socket_fd, "\n", 1, 0) != 1)) {
        stop("Error sending speudo to server");
    }
    // Receive the greeting message
    if ((valread=recv(socket_fd, buffer, BUFFLEN-1, 0)) >= 0) {
        write(1, buffer, valread);
        memset(buffer, 0, BUFFLEN);
    } 

    fd_set readfds;
    int max_sd, activity;
    while (1) {
        FD_ZERO(&readfds);

        FD_SET(socket_fd, &readfds);
        FD_SET(0, &readfds);
        max_sd = socket_fd;

        // Use select to handle message form both socket and stdin
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
            write(1, "Select error\n", 13);

        if (FD_ISSET(socket_fd, &readfds)) {
            memset(buffer, 0, BUFFLEN);
            valread = recv(socket_fd, buffer, BUFFLEN-1, 0);
            buffer[valread] = '\0';
            write(1, "> ", 2);
            if (!msg_to_file(buffer)) write(1, "Received a new file in directory ./receivedFile\n", 48);
        } else if (FD_ISSET(0, &readfds)) {
            memset(buffer, 0, BUFFLEN);
            valread = read(0, buffer, BUFFLEN-1);
            buffer[valread-1] = '\0';
            
            int count = 0, spce_track = 0;
            int last_space = 0;
            if (strncmp(buffer, "/send\0", 5) != 0) {
                send(socket_fd, buffer, strlen(buffer), 0);
            } else {
                while (buffer[count] != '\0') {
                    if (buffer[count] == ' ') spce_track += 1;
                    if (spce_track == 2) {
                        last_space = count;
                        break;
                    }
                    count += 1;
                }
                char* file_path = malloc(strlen(buffer)-last_space);
                file_path[strlen(buffer)-last_space-1] = '\0';
                strncpy(file_path, buffer+last_space+1, strlen(buffer)-last_space-1);
                file_to_msg(file_path, buffer+last_space+1);
                send(socket_fd, buffer, strlen(buffer), 0);
            }
        }
    } 
        
}