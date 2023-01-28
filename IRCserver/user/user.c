#include "user.h"
#include <stdio.h>
#include <string.h>
#include "../cmd/cmd.h"

int N = 30;

extern user* users;

user* find_usr_with_sock(int sock, user* usrs) {
    for (int i=0; i<N; ++i) {
        if (usrs[i].sock == sock) return &(usrs[i]);
        else if (users[i].sock == 0) return NULL;
    }

}

user* find_usr_with_speudo(char *speudo, user* usrs) {
    for (int i=0; i<N; ++i) {
        if (usrs[i].speudo != NULL) {
            if (strcmp(usrs[i].speudo, speudo) == 0) {
                printf("%d", i);
                return &(usrs[i]);
            }
        }   
    }
    return NULL;
}

void copy_usr(user* dst, user* src) {
    dst->is_logined = src->is_logined;
    if (src->passwd == NULL) dst->passwd = NULL;
    else {
        if (dst->passwd == NULL) {
            dst->passwd = malloc(strlen(src->passwd));
        }
        strcpy(dst->passwd, src->passwd);
    }
    
    dst->sock = src->sock;

    if (src->speudo == NULL) dst->speudo = NULL;
    else {
        if (dst->speudo == NULL) {
            dst->speudo = malloc(strlen(src->speudo));
        }
        strcpy(dst->speudo, src->speudo);
    }
    
}

void add_usr(user usr) {
    if (users == NULL) {
        init();
    }
    for (int i=0; i<N; ++i) {
        if (users[i].speudo == NULL) {
            copy_usr(users+i, &usr);
            return;
        }
    }
}