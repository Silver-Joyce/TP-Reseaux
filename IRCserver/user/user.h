#ifndef USER_H
#define USER_H

#include <stdlib.h>

typedef struct {
    char* speudo;
    char* passwd;
    int is_logined;
    int sock;
} user;

extern int N;
extern user* users; 

user* find_usr_with_sock(int sock, user* users);
user* find_usr_with_speudo(char *speudo, user* users);
void copy_usr(user* dst, user* src);
void add_usr(user usr);

#endif