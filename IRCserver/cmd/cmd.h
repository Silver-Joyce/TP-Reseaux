#ifndef CMD_H
#define CMD_H

#include "../user/user.h"

typedef struct {
    int cmd_id;
    char* arg[2];
} cmd;


// Convert cmd to id and vice versa
int cmdToID(char* command);
char* IDTocmd(int id);


int nickname(char* speudo, char* password, int sock);
int registre(char* speudo, char* password, int sock);
int mp(char* speudo, char* msg, int sock);
void alert(char* speudo, char *msg, int sock);
void sendFile(char* speudo, char* file_dir, int sock);

int handle_cmd(cmd a, int sock);

// function to regconize a command from a buffer
cmd cmd_regconize(char *buff);

void init(void);

int msg_to_file(char *msg);
void file_to_msg(char* file_path, char* buffer);

#endif