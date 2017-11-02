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

int sendSocketMessages(int sockfd, char* message){
    int len = strlen(message);
    int p = 0;
    while (p < len){
        int n = write(sockfd, message+p, len-p);
        if (n < 0){
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return -1;
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
            sendSocketMessages(connt->cmdfd, "331 Password Not Input!\r\n");
            break;
        case 332:
            sendSocketMessages(connt->cmdfd, "332 Not Log In!\r\n");
            break;
        case 425:
            sendSocketMessages(connt->cmdfd, "425 no TCP connection was established\r\n");
            break;
        case 426:
            sendSocketMessages(connt->cmdfd, "426 TCP connection was broken by the client or by network failure\r\n");
            break;
        case 451:
            sendSocketMessages(connt->cmdfd, "451 Can not read file from disk.\r\n");
            break;
        case 500:
            sendSocketMessages(connt->cmdfd, "500 Command not Found!\r\n");
            break;
        case 501:
            sendSocketMessages(connt->cmdfd, "501 Syntax Error in parameters!\r\n");
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
        sendSocketMessages(connt->cmdfd, "331 Guest login ok, send your complete e-mail address as password.\r\n");
    } else {
        ERROR(connt, 501);
    }
}

void PASS(connection* connt, char* cmdContent){
    if (strlen(cmdContent) > 0) {
        sendSocketMessages(connt->cmdfd, "230-Welcome to School of Software\r\n230 Guest login ok.\r\n");
        connt->isPassed = true;
    }
    else
        ERROR(connt, 501);

}

void SYST(connection* connt, char* cmdContent){
    if (cmdContent == NULL)
        sendSocketMessages(connt->cmdfd, "215 UNIX Type: L8\r\n");
    else
        ERROR(connt, 501);
}

void TYPE(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(connt, 501);
        return;
    }
    if (strlen(cmdContent) == 1 && cmdContent[0] == 'I')
        sendSocketMessages(connt->cmdfd, "200 Type set to I.\r\n");
    else
        ERROR(connt, 501);

}

void QUIT(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        connt->isLogIn = false;
        connt->isPassed = false;
        sendSocketMessages(connt->cmdfd, "221 Goodbye.\r\n");
    } else{
        ERROR(connt, 501);
    }
}

void PORT(connection* connt, char* cmdContent){
    int *ip = connt->clientIpAndPort;
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
    connt->dataMode = 1;
    sendSocketMessages(connt->cmdfd, "200 PORT command successful.\r\n");
    if (connt->dataMode != 0){
        close(connt->datafd);
    }
}

void PASV(connection* connt, char* cmdContent){
    if (cmdContent != NULL){
        ERROR(connt, 501);
        return;
    }
    connt->dataMode = 2;

    //open a socket and listen on it
    if ((connt->filefd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return ;
    }
    memset(&(connt->data_addr), 0, sizeof(connt->data_addr));
    connt->data_addr.sin_family = AF_INET;
    connt->data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    for (connt->data_addr.sin_port = 20000; connt->data_addr.sin_port < 65536; connt->data_addr.sin_port++){
        if (bind(connt->filefd, (struct sockaddr*)&(connt->data_addr), sizeof(connt->data_addr)) != -1)
            break;
    }
    listen(connt->filefd, 10);

    //return some messages to client
    char ip[20], message[100] = "227 Entering Passive Mode (", ipandport[50];
    int *pIP = connt->serverIpAndPort;
    getIP(ip);
    sscanf(ip, "%d.%d.%d.%d", pIP, pIP+1, pIP+2, pIP+3);
    pIP[4] = connt->data_addr.sin_port / 256;
    pIP[5] = connt->data_addr.sin_port % 256;
    sprintf(ipandport, "%d,%d,%d,%d,%d,%d", pIP[0], pIP[1], pIP[2], pIP[3], pIP[4], pIP[5]);
    strcat(message, ipandport);
    strcat(message, ")\r\n");
    sendSocketMessages(connt->cmdfd, message);
}

void RETR(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(connt, 501);
        return;
    }
    if (connt->dataMode == 0){
        ERROR(connt, 425);
        return;
    }
    char buff[BUFFSIZE];
    char filename[256], response[256];
    int file, nBytes;
    int datafd;
    sscanf(filename, "%s", cmdContent);
    if ((file = open(filename, O_RDONLY)) < 0){
        ERROR(connt, 451);
        return;
    }

    //establish data connection
    if (connt->dataMode == 1){ //MODE PORT
        if ((datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
            ERROR(connt, 425);
            return;
        }
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = connt->clientIpAndPort[4]*256 + connt->clientIpAndPort[5];
        char clientIP[100];
        sprintf(clientIP, "%d.%d.%d.%d", connt->clientIpAndPort[0], connt->clientIpAndPort[1],
                connt->clientIpAndPort[2], connt->clientIpAndPort[3]);
        inet_pton(AF_INET, clientIP, &addr.sin_addr);
        if (connect(datafd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            ERROR(connt, 425);
            return ;
        }
        sprintf(response, "50 Opening BINARY mode data connection for %s\r\n", filename);
        sendSocketMessages(connt->cmdfd, response);
    } else {    //MODE PASV
        if (datafd = accept(connt->filefd, NULL, NULL) == -1){
            ERROR(connt, 425);
            return;
        }
        sprintf(response, "150 Opening BINARY mode data connection for %s\r\n", filename);
        sendSocketMessages(connt->cmdfd, response);
    }
    connt->datafd = datafd;

    //begin transmission
    while ((nBytes = read(file, buff, BUFFSIZE-1)) > 0){
        if (sendSocketMessages(connt->datafd, buff) == -1){
            ERROR(connt, 426);
            return;
        }
    }

    if (nBytes < 0){
        ERROR(connt, 426);
    } else {
        sendSocketMessages(connt->cmdfd, "226 Transfer complete.\r\n");
    }

}

void STOR(connection* connt, char* cmdContent){

}

void getIP(char* ip){
    system("ifconfig | grep inet[^6] | awk \'{if(NR == 2){print $2}}\' > a.txt");
    FILE *fp1 = fopen("a.txt", "r");
    fgets(ip, 1024, fp1);
    fclose(fp1);
    remove("a.txt");
}