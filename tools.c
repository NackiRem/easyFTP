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

int parseMessages(connection* connt, char* command){
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
            ERROR(connt, 500);
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
        USER(connt, commandContent);
        return 0;
    }
    if (connt->isLogIn == false){
        ERROR(connt, 332);
        return 1;
    }
    if (strcmp(commandType, "PASS") == 0){
        PASS(connt, commandContent);
        return 0;
    }
    if (connt->isPassed == false){
        ERROR(connt, 331);
        return 1;
    }
    if (strcmp(commandType, "SYST") == 0){
        SYST(connt, commandContent);
    } else if (strcmp(commandType, "TYPE") == 0){
        TYPE(connt, commandContent);
    } else if (strcmp(commandType, "QUIT") == 0){
        QUIT(connt, commandContent);
    } else if (strcmp(commandType, "PORT") == 0){
        PORT(connt, commandContent);
    } else if (strcmp(commandType, "PASV") == 0){
        PASV(connt, commandContent);
    } else if (strcmp(commandType, "RETR") == 0){
        RETR(connt, commandContent);
    } else if (strcmp(commandType, "STOR") == 0){
        STOR(connt, commandContent);
    } else {
        ERROR(connt, 500);
    }
}

void ERROR(connection* connt, int errorCode){
    switch (errorCode){
        case 331:
            sendSocketMessages(connt->sockfd, "331 Password Not Input!\r\n");
            break;
        case 332:
            sendSocketMessages(connt->sockfd, "332 Not Log In!\r\n");
            break;
        case 500:
            sendSocketMessages(connt->sockfd, "500 Command not Found!\r\n");
            break;
        case 501:
            sendSocketMessages(connt->sockfd, "501 Syntax Error in parameters!\r\n");
            break;
        default:
            break;
    }
}

void USER(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(connt, 501);
        return;
    }
    if (strcmp(cmdContent, "anonymous") == 0){
        connt->isLogIn = true;
        sendSocketMessages(connt->sockfd, "331 Guest login ok, send your complete e-mail address as password.\r\n");
    } else {
        ERROR(connt, 501);
    }
}

void PASS(connection* connt, char* cmdContent){
    if (strlen(cmdContent) > 0) {
        sendSocketMessages(connt->sockfd, "230-Welcome to School of Software\r\n230 Guest login ok.\r\n");
        connt->isPassed = true;
    }
    else
        ERROR(connt, 501);

}

void SYST(connection* connt, char* cmdContent){
    if (cmdContent == NULL)
        sendSocketMessages(connt->sockfd, "215 UNIX Type: L8\r\n");
    else
        ERROR(connt, 501);
}

void TYPE(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(connt, 501);
        return;
    }
    if (strlen(cmdContent) == 1 && cmdContent[0] == 'I')
        sendSocketMessages(connt->sockfd, "200 Type set to I.\r\n");
    else
        ERROR(connt, 501);

}

void QUIT(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        connt->isLogIn = false;
        connt->isPassed = false;
        sendSocketMessages(connt->sockfd, "221 Goodbye.\r\n");
    } else{
        ERROR(connt, 501);
    }
}

void PORT(connection* connt, char* cmdContent){
    int *ip = connt->ipAndPort;
    if (sscanf(cmdContent, "%d,%d,%d,%d,%d,%d", ip, ip+1, ip+2, ip+3, ip+4, ip+5) != 6){
        ERROR(connt, 501);
        return;
    }
    for (int i = 0; i < 6; i++){
        if (ip[i] < 0 || ip[i] > 255){
            ERROR(connt, 501);
            return;
        }
    }
    sendSocketMessages(connt->sockfd, "200 PORT command successful.\r\n");
}

void PASV(connection* connt, char* cmdContent){

}

void RETR(connection* connt, char* cmdContent){

}

void STOR(connection* connt, char* cmdContent){

}