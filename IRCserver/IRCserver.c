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

#define PORT 8000
#define BUFFLEN 1024
#define N 10

void stop(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int client_number = -1;

int main(int *argc, char **argv)
{
    int opt = 1;
    int master_socket, socket_client[N];
    for (int i = 0; i < N; ++i) {
        socket_client[i] = 0;
    }
    int max_sd, sd, activity, newsocket, addrlen, valread;

    // buffer for user
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);

    // greeting message
    char *greeting_msg = "Hello Client\n\0";

    fd_set readfds;

    // The main socket that listen for all connect
    master_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (master_socket < 0)
        stop("Error creating new socket");
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)(&opt), sizeof(opt)) < 0)
        perror("Error setting socket option");

    // Sockaddr_in structure
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(master_socket, (const struct sockaddr *)(&address), sizeof(address)) < 0)
        stop("Error binding on master socket");

    if (listen(master_socket, N) < 0)
        stop("Error listening on master socket");

    write(1, "Waiting for connection ...\n", 27);

    while (1)
    {
        FD_ZERO(&readfds);

        FD_SET(master_socket, &readfds);
        FD_SET(0, &readfds);
        max_sd = master_socket;

        for (int i = 0; i < N; ++i)
        {
            sd = socket_client[i];
            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
            write(1, "Select error\n", 13);

        if (FD_ISSET(master_socket, &readfds))
        {
            if ((newsocket = accept(master_socket, (struct sockaddr *)(&address), &addrlen)) < 0)
                stop("Error connecting");
            else
            {
                user new_usr = {NULL, NULL, 0, newsocket};
                memset(buffer, 0, BUFFLEN);
                valread = recv(newsocket, buffer, BUFFLEN - 1, 0);
                buffer[valread - 2] = '\0';

                if (strncmp(buffer, "speudo: ", 8) == 0)
                {
                    new_usr.speudo = malloc(valread - 8);
                    strncpy(new_usr.speudo, buffer + 8, valread - 8);
                }
                add_usr(new_usr);
            }

            write(1, "Successfully connected!\n", 24);
            if (send(newsocket, greeting_msg, strlen(greeting_msg), 0) != strlen(greeting_msg))
            {
                perror("ERROR sending greeting message");
            }

            for (int i = 0; i < N; ++i)
            {
                if (socket_client[i] == 0)
                {
                    socket_client[i] = newsocket;
                    break;
                }
            }
        } else if (FD_ISSET(0, &readfds)) {
            memset(buffer, 0, BUFFLEN);
            valread = read(0, buffer, BUFFLEN);
            buffer[valread-1] = '\0';
            cmd newsvrCMD = cmd_regconize(buffer);
            handle_cmd(newsvrCMD, 0);
        } else {
            // init();
            // if (fork() != 0) {
                
            // }
            // else {
                for (int i = 0; i < N; ++i) {
                    sd = socket_client[i];

                    if (FD_ISSET(sd, &readfds)) {
                        memset(buffer, 0, BUFFLEN);
                        valread = recv(sd, buffer, BUFFLEN-1, 0);
                        buffer[valread] = '\0';
                        printf("Message length: %d\n", valread);
                        if (valread == 0) {
                            close(sd);
                            socket_client[i] = 0;
                            //     getpeername(sd, (struct sockaddr *)(&address), (socklen_t *)(&addrlen));
                            //     sprintf("Host disconnected, ip: %s, port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                            //     close(sd);
                            //     socket_client[i] = 0;
                        } else {
                            // if (valread >= 0) {
                            // write(1, buffer, valread);
                            fputs(buffer, stdout);
                            cmd newCMD = cmd_regconize(buffer);
                            // printf("%d\n", newCMD.cmd_id);
                            // for (int j=0; j<2; j++) {
                            //     if (newCMD.arg[j] != NULL) printf("%s\n", newCMD.arg[j]);
                            // }
                            // fputs(newCMD.arg[1], stdout);
                            handle_cmd(newCMD, sd);
                        }
                    } 
                }
            // }
        }

        // for (int i=0; i<N; ++i) {
        //     sd = socket_client[i];
        //     if (FD_ISSET(sd, &readfds)) {
        //
        //     }
        // }
    }
}