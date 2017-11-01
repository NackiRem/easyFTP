#include "tools.h"

char** getSocketMessages(int sockfd, char* buff){

    //get message from server
    int p = 0;
    while(1){
        int n = read(sockfd, buff+p, 8191-p);
        if (n < 0){
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            return 1;
        } else if (n == 0){
            break;
        } else {
            p += n;
            if (buff[p-1] == '\n'){
                break;
            }
        }
    }

    //divide message into lines
    char** messages;
    int len = p;
    int temp_begin = 0, temp_end = 0;
    for (int i = 0; i < len; i++){
        if (buff[i] == '\n'){
            temp_end = i;
            char* aMessage = (char*)malloc(temp_end-temp_begin+2);
            for (int j = 0; j < (temp_end-temp_begin+1); j++)
                aMessage[j] = buff[temp_begin+j];
            aMessage[(temp_end-temp_begin+1)] = '\0';
            stringListAppend(&messages, aMessage);
            temp_begin = i+1;
        }
    }

    return messages;
}

void printSocketMessages(char** messages){
    int len = sizeof(messages) / sizeof(messages[0]);
    for (int i = 0; i < len; i++)
        printf(messages[i]);
}

void stringListAppend(char*** target, char* source){
    if (*target == NULL){
        *target = (char**)malloc(sizeof(char*));
        *target[0] = source;
    } else {
        char **temp = *target;
        int len = sizeof(*target)/sizeof((*target)[0]);
        *target = (char**)malloc(sizeof(char*)*(len+1));
        for (int i = 0; i < len; i++){
            (*target)[i] = temp[i];
        }
        (*target)[len] = source;
        free(temp);
    }
}

void freeStringList(char** tofree){
    if (tofree == NULL)
        return;
    int len = sizeof(tofree) / sizeof(tofree[0]);
    for (int i = 0; i < len; i++){
        free(tofree[i]);
    }
    free(tofree);
}