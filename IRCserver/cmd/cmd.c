#include "cmd.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../user/user.h"
#include <sys/socket.h>
#include <time.h>

#define N 30


char* cmds[9] = {"/nickname\0", "/register\0", "/unregister\0", "/exit\0", "/date\0", "/mp\0", "/alert\0", "/send\0", NULL};
user* users = NULL;
void init(void) {
    users = (user*)malloc(N*sizeof(user));
    user dftUsr = {NULL, NULL, 0, 0};
    for (int i=0; i<N; ++i) {
        copy_usr(&(users[i]), &dftUsr);
    }
}

int nickname(char* speudo, char* password, int sock) {
    for (int i=0; i<N; ++i) {
        if (users[i].speudo != NULL) {
            printf("%s: %d\n", users[i].speudo, strlen(users[i].speudo));
        }  
    }
    user *usr;
    if (password == NULL) {
        usr = find_usr_with_sock(sock, users);
        if (usr == NULL) {
            send(sock, "Fuck u\n", 7, 0);
            return 1;
        }
        strncpy(usr->speudo, speudo, strlen(speudo));
        char return_msg[32+strlen(speudo)];
        sprintf(return_msg, "User has changed the speudo to %s\n", speudo);
        send(sock, return_msg, strlen(return_msg), 0);
        fputs(return_msg, stdout);
        return 0;
    } else {
        usr = find_usr_with_speudo(speudo, users);
        if (usr == NULL) {
            send(sock, "Hahaha\n", 7, 0);
            return 1;
        }
        if (usr->speudo == NULL || usr->passwd == NULL) {
            send(usr->sock, "User has not enregistered!\n", 27, 0);
            return 1;
        } else if (strcmp(speudo, usr->speudo) || strcmp(password, usr->passwd)) {
            send(usr->sock, "Speudo or password is incorrect!\n", 33, 0);
            return 1;
        } else {
            usr->is_logined = 1;
            send(usr->sock, "Successfully logged in\n", 23, 0);
            return 0;
        }
        send(sock, "Error logining\n", 15, 0);
        return 1;
    } 
}

int registre(char* speudo, char* password, int sock) {
    if (find_usr_with_speudo(speudo, users) != NULL) {
        send(sock, "The user is already exist!\n", 27, 0);
        return 2;
    } else {
        int i=0;
        while (users[i].speudo != NULL) {
            i += 1;
        }
        printf("%d", i);
        // Create new user
        users[i].speudo = malloc(strlen(speudo)+1);
        users[i].speudo[strlen(speudo)] = '\0';
        strncpy(users[i].speudo, speudo, strlen(speudo));
        users[i].passwd = malloc(strlen(password)+1);
        strncpy(users[i].passwd, password, strlen(password));
        users[i].passwd[strlen(password)] = '\0';
        users[i].is_logined = 1;
        users[i].sock = sock;
        // copy_usr(&(users[i]), &(users[i])); 
        char return_msg[35+strlen(speudo)];
        return_msg[34] = '\0'; 
        // printf("%s: %s\n", users[i].speudo, users[i].passwd);
        sprintf(return_msg, "User %s has successfully registered\n", speudo);
        send(sock, return_msg, strlen(return_msg), 0);
        return 0;
    }
}

int unregistre(char* speudo, char* password, int sock) {
    user *usr = find_usr_with_speudo(speudo, users);
    if (usr == NULL) {
        send(sock, "Cannot find user\n", 17, 0);
        return 1;
    } else {
        if (usr->passwd == NULL) {
            send(sock, "User has not registered\n", 24, 0);
            return 3;
        } else {
            if (strcmp(usr->passwd, password)) {
                send(sock, "Password is not correct!\n", 25, 0);
                return 3;
            } else {
                usr->speudo = NULL;
                usr->is_logined = 0;
                send(sock, "Successfully unregistered user!\n", 32, 0);
                return 0;
            }
        }
    }
}

void date() {
    time_t t;
    time(&t);
    fputs(ctime(&t), stdout);
    fputs("\n", stdout);
}

int mp(char* speudo, char* msg, int sock) {
    
    if (speudo == NULL) {
        send(sock, "Speudo invalid\n", 15, 0);
        return 5;
    } else {
        
        user *src = NULL;
        if (sock>0) src = find_usr_with_sock(sock, users);
        user *dst = find_usr_with_speudo(speudo, users);
        if (dst == NULL) {
            char return_msg[21+strlen(speudo)];
            sprintf(return_msg, "User %s does not exist\n", speudo);
            send(sock, return_msg, strlen(return_msg), 0);
            return 5;
        } else {
            fputs("Hello from private message\n", stdout);
            int full_msg_len = (src!=NULL) ? 24+strlen(src->speudo)+strlen(msg) : 30+strlen(msg);
            char full_msg[full_msg_len];
            if (src != NULL) sprintf(full_msg, "private message from: %s\n%s\n", src->speudo, msg);
            else sprintf(full_msg, "private message from: server\n%s\n", src->speudo, msg);
            send(dst->sock, full_msg, strlen(full_msg), 0);
            return 0;
        }
    }
}

void alert(char* speudo, char *msg, int sock) {
    if (speudo == NULL) {
        char sent_msg[1+strlen(msg)];
        sprintf(sent_msg, "%s\n", msg);
        for (int i=0; i<N; ++i) {  
            if (i != sock) send(users[i].sock, sent_msg, strlen(sent_msg), 0);
        }
    } else {
        mp(speudo, msg, sock);
    }
    return; 
}

void sendFile(char* speudo, char* file_dir, int sock) {
    char *file_name = NULL;
    char *file_extension = NULL;
    char *path_dir = NULL;
    char *full_name = NULL;

    // Seperate the path name and file name
    int count = 0;
    int path_dir_len = 0;
    while (file_dir[count] != '\0') {
        if (file_dir[count] == '/') path_dir_len = count;
        count += 1;
    }
    
    // get the path dir if exist
    if (path_dir_len != 0) {        
        path_dir = malloc(path_dir_len+1);
        path_dir[path_dir_len] = '\0';
        strncpy(path_dir, file_dir, path_dir_len);
    }

    // get file name
    int name_length = (path_dir_len != 0) ? (strlen(file_dir)-path_dir_len-1) : strlen(file_dir);
    full_name = malloc(name_length+1);
    full_name[name_length] = '\0';

    strncpy(full_name, file_dir+strlen(file_dir)-name_length, name_length);
    

    count = 0;
    // file the margin point between file name and file extension
    while (full_name[count] != '.'  && full_name[count] != '\0') {
        count += 1;
    }

    // get file name
    file_name = malloc(count+1);
    file_name[count] = '\0';
    strncpy(file_name, full_name, count);
    
    // get file extension if there exist
    if (count != strlen(full_name)) {
        file_extension = malloc(strlen(full_name) - count);
        file_extension[strlen(full_name)-count-1] = '\0';

        strncpy(file_extension, full_name+count+1, strlen(full_name)-count-1);
    }


    int fd = open(file_dir, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    } 
    char body[1024];
    memset(body, 0, 1024);
    int n = read(fd, body, 1023);
    if (n<0) {
        perror("Error reading file");
        return;
    } else {
        user* usr = find_usr_with_speudo(speudo, users);
        if (usr == NULL) {
            char return_msg[21+strlen(speudo)];
            sprintf(return_msg, "User %s does not exist\n", speudo);
            send(sock, return_msg, strlen(return_msg), 0);
            return;
        } else {
            int sent_msg_length = 5+strlen(file_name)+1+10+strlen(file_extension)+1+strlen(body);
            char sent_msg[sent_msg_length+1];
            sprintf(sent_msg, "name:%s extension:%s %s\0", file_name, file_extension, body);
            send(usr->sock, sent_msg, sent_msg_length, 0);
            return;
        }
    }
}




int handle_cmd(cmd a, int sock) {
    if (users == NULL) {
        init();
    }
    switch (a.cmd_id) {
        // Handle /nickname command
        case 0:
            if (nickname(a.arg[0], a.arg[1], sock) == 0) {
                printf("Successfully changed speudo for %d\n", sock);
            } else {
                printf("Unsuccessfully changed speudo\n");
            }
            break;

        // Handle /register command
        case 1:
            if (registre(a.arg[0], a.arg[1], sock) == 0) {
                printf("Successfully registered new user %s\n", a.arg[0]);
            } else {
                printf("Unsuccessfully registered new user\n");
            }
            break;

        // Handle /unregister command
        case 2:
            if (unregistre(a.arg[0], a.arg[1], sock) == 0) {
                printf("Successfully unregistered the user %s\n", a.arg[0]);
            } else {
                printf("Unsuccessfully unregister user");
            }
            break;

        // Handle /exit
        case 3:
            exit(0);
            break;

        // Handle /date
        case 4:
            date();
            break;

        // Handle /mp 
        case 5:
            mp(a.arg[0], a.arg[1], sock);
        
        // Handle alert
        case 6:
            if (a.arg[1] == NULL) alert(NULL, a.arg[0], sock);
            else alert(a.arg[0], a.arg[1], sock);
        case 7: 
            // if the sender is server
            if (sock == 0) {
                sendFile(a.arg[0], a.arg[1], sock);
            }
            // if the sender is a client 
            else {
                user* usr = find_usr_with_speudo(a.arg[0], users);
                if (usr == NULL) {
                    char return_msg[21];
                    sprintf(return_msg, "User %s does not exist\n", a.arg[0]);
                    send(sock, return_msg, strlen(return_msg), 0);
                } else {
                    send(usr->sock, a.arg[1], strlen(a.arg[1]), 0);
                }
            }
            
            
        default:
            break;
    }
}


// Function to regconize of the command from the buffer
cmd cmd_regconize(char *buff) {
    cmd newCMD = {-1 , {NULL, NULL}};
    
    int c = 0;
    int count = 0;
    char *tmp = buff;
    while (1) {
        if ((*(tmp+count) == ' ') || (*(tmp+count) == '\0')) {

            // recognize command and arguments
            if (c == 0) {

                // Convert string command to ID
                char* tmp_cmd = malloc(count+1);
                tmp_cmd[count] = '\0';
                strncpy(tmp_cmd, tmp, count);
                newCMD.cmd_id = cmdToID(tmp_cmd);


                if (newCMD.cmd_id == -1) {
                    write(1, "Command unvalid\n", 16);
                    return newCMD;
                }

            } else if (c == 1) {
                newCMD.arg[c-1] = malloc(count+1);
                newCMD.arg[c-1][count] = '\0';
                strncpy(newCMD.arg[c-1], tmp, count);
                
            } else {
                int rest_length = strlen(buff) - (tmp - buff) + 1;
                newCMD.arg[c-1] = malloc(rest_length+1);
                newCMD.arg[c-1][rest_length] = '\0';
                strncpy(newCMD.arg[c-1], tmp, rest_length);
                return newCMD;
            }
            if (*(tmp+count) == '\0') {
                return newCMD;
            } 
            

            c++;


            tmp += (count+1);
            count = 0;
            
        } else {
            count ++;
        }
    }
}

// Convert cmd to id and vice versa
int cmdToID(char* command) {
    for (int i=0; i<8; ++i) {
        if (strcmp(command, cmds[i]) == 0) return i;
    }
    return -1;
}

char* IDTocmd(int id) {
    return cmds[id];
}

// function to convert the full path name to the file name
void path_to_name(char* path, char* path_dir, char* file_name) {
    int count = 0;
    int path_dir_len = 0;
    while (path[count] != '\0') {
        if (path[count] == '/') path_dir_len = count;
        count += 1;
    }

    // get the path dir if exist
    if (path_dir_len != 0) {
        if (path_dir == NULL) {
            path_dir = malloc(path_dir_len+1);
            path_dir[path_dir_len] = '\0';
        }
        strncpy(path, path_dir, path_dir_len);
    }

    // get file name
    int name_length = strlen(path)-path_dir_len-1;
    if (file_name == NULL) {
        file_name = malloc(name_length+1);
        file_name[name_length] = '\0';
    }
    strncpy(file_name, path+path_dir_len+1, name_length);
}

// funtion to get file name and file extension from full name of file
void get_file_properties(char *full_name, char *file_name, char *file_extension) {
    int count = 0;
    // file the margin point between file name and file extension
    while (full_name[count] != '.'  && full_name[count] != '\0') {
        count += 1;
    }

    // get file name
    if (file_name == NULL) {
        file_name = malloc(count+1);
        file_name[count] = '\0';
    }
    strncpy(file_name, full_name, count);
    
    // get file extension if there exist
    if (count != strlen(full_name)) {
        if (file_extension == NULL) {
            file_extension = malloc(strlen(full_name) - count);
            file_extension[strlen(full_name)-count-1] = '\0';
        }
        strncpy(file_extension, full_name+count+1, strlen(full_name)-count-1);
    }
}



// -------------------------------------- For client -------------------------------------- //
// The encoded message of a file is in the form: name:{file_name} extension:{file_extesion} {file_body}
// received file is always saved in directory receivedFile

// This function is used to convert a message to a file
int msg_to_file(char *msg) {
    // Handle received file

    char *name = NULL;
    char *extension = NULL;

    int count = 0;
    while (msg[count] != '\0') {
        if (msg[count] == ' ') {
            if (name == NULL) {
                name = malloc(count+1);
                name[count] = '\0';
                if (strncmp(msg, "name:", 5)) write(1, msg, strlen(msg));
                else strncpy(name, msg+5, count-5);
            } else if (extension == NULL) {
                int extension_length = count-5-strlen(name)-1-10;
                extension = malloc(extension_length+1);
                extension[extension_length] = '\0';
                if (strncmp(msg+6+strlen(name), "extension:", 10)) write(1, msg, strlen(msg));
                else strncpy(extension, msg+16+strlen(name), extension_length);
            } else {
                break;
            }
        }
        count += 1;
    }
    int body_length = strlen(msg)-5-strlen(name)-1-10-strlen(extension)-1;
    char *body = malloc(body_length+1);
    strncpy(body, msg+strlen(msg)-body_length, body_length);

    char full_name[15+strlen(name)+strlen(extension)];
    sprintf(full_name, "./receivedFile/%s.%s\0", name, extension);

    int fd = open(full_name, O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        perror("ERROR open file");
        return -1;
    }
    write(fd, body, body_length);
    return 0;
}

// This function is used to convert a file to its encoded message
// To be honest, I am so sorry because i can not find the way to modulerize the path of converting a full file path name to {path dir, file name, file extenstion}
// If you have any way to do it, please contact me via tienduy0123@gmail.com. I will be so happy if you do that.
void file_to_msg(char* file_dir, char* buffer) {
    char *file_name = NULL;
    char *file_extension = NULL;
    char *path_dir = NULL;
    char *full_name = NULL;

    // Seperate the path name and file name
    int count = 0;
    int path_dir_len = 0;
    while (file_dir[count] != '\0') {
        if (file_dir[count] == '/') path_dir_len = count;
        count += 1;
    }
    
    // get the path dir if exist
    if (path_dir_len != 0) {        
        path_dir = malloc(path_dir_len+1);
        path_dir[path_dir_len] = '\0';
        strncpy(path_dir, file_dir, path_dir_len);
    }

    // get file name
    int name_length = (path_dir_len != 0) ? (strlen(file_dir)-path_dir_len-1) : strlen(file_dir);
    full_name = malloc(name_length+1);
    full_name[name_length] = '\0';

    strncpy(full_name, file_dir+strlen(file_dir)-name_length, name_length);
    

    count = 0;
    // file the margin point between file name and file extension
    while (full_name[count] != '.'  && full_name[count] != '\0') {
        count += 1;
    }

    // get file name
    file_name = malloc(count+1);
    file_name[count] = '\0';
    strncpy(file_name, full_name, count);
    
    // get file extension if there exist
    if (count != strlen(full_name)) {
        file_extension = malloc(strlen(full_name) - count);
        file_extension[strlen(full_name)-count-1] = '\0';

        strncpy(file_extension, full_name+count+1, strlen(full_name)-count-1);
    }

    int fd = open(file_dir, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    } 
    
    char body[1024];
    memset(body, 0, 1024);
    int n = read(fd, body, 1023);
    if (n<0) {
        perror("Error reading file");
        return;
    } else {
        int sent_msg_length = 5+strlen(file_name)+1+10+strlen(file_extension)+1+strlen(body);
        char sent_msg[sent_msg_length+1];
        sprintf(sent_msg, "name:%s extension:%s %s\0", file_name, file_extension, body);
        strncpy(buffer, sent_msg, sent_msg_length);
    }
}

