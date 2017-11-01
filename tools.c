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
            break;
        }
    }

    if (p == 0){
        return NULL;
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

void sendSocketMessages(int sockfd, char* message){
    int len = strlen(message);
    int p = 0;
    while (p < len){
        int n = write(sockfd, message+p, len+1-p);
        if (n < 0){
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return ;
        } else {
            p += n;
        }
    }
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

int parseMessages(int sockfd, char** messages, bool* isLogIn){
    if (messages == NULL)
        return 1;
    int len = sizeof(messages) / sizeof(messages[0]);
    if (len != 1) {
        printf("Error messages from client");
        return 1;
    }
    char* command = messages[0];
    len = strlen(command);
    if (len < 4){
        //send error message to client
        ERROR(sockfd, 500);
    }
    char temp[5];
    for (int i = 0; i < 4; i++)
        temp[i] = command[i];
    temp[4] = '\0';
    if (!strcmp(temp, "USER")){
        USER(sockfd, command, isLogIn);
    }
    if (*isLogIn == false){
        ERROR(sockfd, 332);
        return 1;
    }
    if (!strcmp(temp, "PASS")){
        PASS(sockfd, command);
    } else if (!strcmp(temp, "SYST")){
        SYST(sockfd);
    } else if (!strcmp(temp, "TYPE")){
        TYPE(sockfd, command);
    } else if (!strcmp(temp, "QUIT")){
        QUIT(sockfd, isLogIn);
    }
}

void ERROR(int sockfd, int errorCode){
    switch (errorCode){
        case 332:
            sendSocketMessages(sockfd, "332 Not Log In!\r\n");
            break;
        case 500:
            sendSocketMessages(sockfd, "500 Command not Found!\r\n");
            break;
        case 501:
            sendSocketMessages(sockfd, "501 Syntax Error in parameters!\r\n");
            break;
        default:
            break;
    }
}

void USER(int sockfd, char* command, bool* isLogIn){
    int len = strlen(command);
    if (len <= 5 || command[4] != ' '){
        ERROR(sockfd, 501);
        return;
    }
    char *content = &(command[5]);
    if (strcmp(content, "anonymous")){
        *isLogIn = true;
        sendSocketMessages(sockfd, "331 Guest login ok, send your complete e-mail address as password.\r\n");
    } else {
        ERROR(sockfd, 501);
    }
}

void PASS(int sockfd, char* command){
    int len = strlen(command);
    if (len <= 5 || command[4] != ' '){
        ERROR(sockfd, 501);
        return;
    }
    sendSocketMessages(sockfd, "230 Guest login ok.\r\n");
}

void SYST(int sockfd){
    sendSocketMessages(sockfd, "215 UNIX Type: L8\r\n");
}

void TYPE(int sockfd, char* command){
    int len = strlen(command);
    if (len != 6 || command[4] != ' ' || command[5] != 'I'){
        ERROR(sockfd, 501);
        return;
    }
    sendSocketMessages(sockfd, "200 Type set to I.\r\n");

}

void QUIT(int sockfd, bool* isLogIn){
    *isLogIn = false;
    sendSocketMessages(sockfd, "221 Goodbye.\r\n");
}