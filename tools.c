#include "tools.h"

void getSocketMessages(int sockfd, char* buff){

    //get message from server
    int p = 0;
    while(1){
        int n = read(sockfd, buff+p, 8191-p);
        if (n < 0){
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            return ;
        } else if (n == 0){
            break;
        } else {
            p += n;
            if (buff[p-1] == '\n')
                break;
        }
    }
    buff[p] = '\0';
}

void sendSocketMessages(int sockfd, char* message){
    int len = strlen(message);
    int p = 0;
    while (p < len){
        int n = write(sockfd, message+p, len-p);
        if (n < 0){
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return ;
        } else {
            p += n;
        }
    }
}

int parseMessages(int sockfd, char* command, bool* isLogIn, bool* isPassed){
    if (command == NULL)
        return 1;
    char* commandType = NULL;
    char* commandContent = NULL;
    int len = strlen(command);
    int space_tag = -1;
    for (int i = 0; i < len; i++){
        if (command[i] == ' ')
            space_tag = i;
    }
    if (space_tag == -1){
        if (strlen(command) > 10){
            ERROR(sockfd, 500);
            return 1;
        } else {
            commandType = command;
            for (int i = 0; i < len; i++){
                if (command[i] != '\n')
                    commandType[i] = command[i];
                else
                    commandType[i] = '\0';
            }
        }
    } else {
        commandType = (char*)malloc(space_tag+1);
        for (int i = 0; i < space_tag; i++){
            commandType[i] = command[i];
        }
        commandType[space_tag] = '\0';
        if (space_tag < strlen(command)-1){
            commandContent = (char*)malloc(len - space_tag);
            for (int i = space_tag+1; i < len; i++){
                if (command[i] != '\n')
                    commandContent[(i-space_tag-1)] = command[i];
                else
                    commandContent[(i-space_tag-1)] = '\0';
            }
        }
    }


    if (strcmp(commandType, "USER") == 0){
        USER(sockfd, commandContent, isLogIn);
        return 0;
    }
    if (*isLogIn == false){
        ERROR(sockfd, 332);
        return 1;
    }
    if (strcmp(commandType, "PASS") == 0){
        PASS(sockfd, commandContent, isLogIn, isPassed);
        return 0;
    }
    if (*isPassed == false){
        ERROR(sockfd, 331);
        return 1;
    }
    if (strcmp(commandType, "SYST") == 0){
        SYST(sockfd, commandContent);
    } else if (strcmp(commandType, "TYPE") == 0){
        TYPE(sockfd, commandContent);
    } else if (strcmp(commandType, "QUIT") == 0){
        QUIT(sockfd, commandContent, isLogIn, isPassed);
    } else {
        ERROR(sockfd, 500);
    }
}

void ERROR(int sockfd, int errorCode){
    switch (errorCode){
        case 331:
            sendSocketMessages(sockfd, "331 Password Not Input!\r\n");
            break;
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

void USER(int sockfd, char* cmdContent, bool* isLogIn){
    if (cmdContent == NULL){
        ERROR(sockfd, 501);
        return;
    }
    if (strcmp(cmdContent, "anonymous") == 0){
        *isLogIn = true;
        sendSocketMessages(sockfd, "331 Guest login ok, send your complete e-mail address as password.\r\n");
    } else {
        ERROR(sockfd, 501);
    }
}

void PASS(int sockfd, char* cmdContent, bool* isLogIn, bool* isPassed){
    if (strlen(cmdContent) > 0) {
        sendSocketMessages(sockfd, "230-Welcome to School of Software\r\n230 Guest login ok.\r\n");
        *isPassed = true;
    }
    else
        ERROR(sockfd, 501);

}

void SYST(int sockfd, char* cmdContent){
    if (cmdContent == NULL)
        sendSocketMessages(sockfd, "215 UNIX Type: L8\r\n");
    else
        ERROR(sockfd, 501);
}

void TYPE(int sockfd, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(sockfd, 501);
        return;
    }
    if (strlen(cmdContent) == 1 && cmdContent[0] == 'I')
        sendSocketMessages(sockfd, "200 Type set to I.\r\n");
    else
        ERROR(sockfd, 501);

}

void QUIT(int sockfd, char* cmdContent, bool* isLogIn, bool* isPassed){
    if (cmdContent == NULL){
        *isLogIn = false;
        *isPassed = false;
        sendSocketMessages(sockfd, "221 Goodbye.\r\n");
    } else{
        ERROR(sockfd, 501);
    }
}