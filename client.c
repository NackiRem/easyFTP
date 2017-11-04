#include "tools.h"
#define     BUFFSIZE    8192


typedef struct serverManager{
    int     tranMode;   //0-empty   1-port  2-pasv
    int     cmdfd;
    int     filefd;
    int     datafd;
    char    serverIP[50];
    char    clientIP[50];
    int     serverPORT;
    int     clientPORT;
}serverManager;


void initClient(serverManager* server);
int connectServer(serverManager* server);
void loginServer(serverManager* server);

void parseCmd(char* cmd, char* cmdType, char* cmdContent);
void HandleCommand(serverManager* server);
int HandleUSER(serverManager* server);
int HandlePASS(serverManager* server, char* password);
int HandlePORT(serverManager* server, char* cmdContent);
void HandlePASV(serverManager* server, char* cmdContent);
void HandleRETR(serverManager* server, char* cmdContent);
void HandleSTOR(serverManager* server, char* cmdContent);

void clearCharNorR(char* tarString);
void getClientIPandPORT(int* ip, int *port);


int main(int argc, char **argv) {
    serverManager  server;
    server.tranMode = 0;

    char sentence[8192];
    int len;
    int p;

    initClient(&server);

    if (connectServer(&server) == -1){
        printf("Connection refused.\n");
        return -1;
    }

    loginServer(&server);

    //get the initial message from server
    getSocketMessages(server.cmdfd, sentence);
    if (strstr(sentence, "220") != 0){
        printf("Connection refused.\n");
        return -1;
    } else {
        printf("Anonymous FTP server ready.\n");
    }

    //Handle User's Command
    HandleCommand(&server);

    while(1){
        fgets(sentence, 4096, stdin);
        len = (int)strlen(sentence);
        sentence[len] = '\0';

        char cmdType[100], *cmdContent = NULL;
        parseCmd(sentence, cmdType, cmdContent);

        //send messages
        p = 0;
        while (p < len) {
            int n = (int)write(server.cmdfd, sentence + p, (size_t)(len + 1 - p));
            if (n < 0) {
                printf("Error write(): %s(%d)\n", strerror(errno), errno);
                return 1;
            } else {
                p += n;
            }
        }

        //get messages
        getSocketMessages(server.cmdfd, sentence);
        printf(sentence);

        //handle cmd
        if (strcmp(cmdType, "PORT") == 0){
            HandlePORT(&server, cmdContent);
        } else if (strcmp(cmdType, "PASV") == 0){
            HandlePASV(&server, cmdContent, sentence);
        } else if (strcmp(cmdType, "RETR") == 0){
            HandleRETR(&server, cmdContent);
        } else if (strcmp(cmdType, "STOR") == 0){
            HandleSTOR(&server, cmdContent);
        }
    }


    return 0;
}

void clearCharNorR(char* tarString){
    if (tarString == NULL)
        return;
    for (int i = 0; i < strlen(tarString); ++i) {
        if (tarString[i] == '\r' || tarString[i] == '\n'){
            tarString[i] = '\0';
            return;
        }
    }
}

void initClient(serverManager* server){
    char    buff[BUFFSIZE];
    memset(buff, 0, sizeof(buff));

    printf("server IP:\n");
    while (1){
        printf("ftp> ");
        fgets(buff, BUFFSIZE, stdin);
        int ip[4];
        if ((sscanf(buff, "%d.%d.%d.%d", ip, ip+1, ip+2, ip+3)) != 4){
            printf("IP format error!\n");
            continue;
        }
        for (int i = 0; i < 4; i++){
            if (ip[i] < 0 || ip[i] > 255) {
                printf("IP format error!\n");
                continue;
            }
        }

        clearCharNorR(buff);
        strcpy(server->serverIP, buff);
        memset(buff, 0, sizeof(buff));
        break;
    }

    printf("server PORT:\n");
    while (1){
        printf("ftp> ");
        fgets(buff, BUFFSIZE, stdin);
        int port;
        sscanf(buff, "%d", &port);
        if (port < 0 || port > 65535){
            printf("ERROR PORT!\n");
            continue;
        }
        server->serverPORT = port;
        memset(buff, 0, sizeof(buff));
        break;
    }



}

int connectServer(serverManager* server){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)server->serverPORT);

    if ((server->cmdfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    if (inet_pton(AF_INET, server->serverIP, &addr.sin_addr) <= 0) {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    if (connect(server->cmdfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    return 0;
}

void loginServer(serverManager* server){
    char    buff[BUFFSIZE];
    memset(buff, 0, sizeof(buff));

    printf("Login(anonymous only):\n");
    while(1){
        printf("ftp> ");
        fgets(buff, BUFFSIZE, stdin);
        if (strcmp(buff, "anonymous\n") != 0){
            printf("User not found!\n");
            continue;
        } else {
            if (HandleUSER(server) == -1){
                printf("User not found.\n");
                continue;
            }
            memset(buff, 0, sizeof(buff));
            break;
        }
    }

    printf("Input your password(email):\n");
    while (1){
        printf("ftp> ");
        fgets(buff, BUFFSIZE, stdin);
        clearCharNorR(buff);
        if (strlen(buff) == 0){
            printf("Password can not be empty!\n");
            continue;
        } else {
            if(HandlePASS(server, buff) == -1){
                printf("Password can not be empty!\n");
                continue;
            }
            memset(buff, 0, sizeof(buff));
            break;
        }
    }
}

void parseCmd(char* cmd, char* cmdType, char* cmdContent){
    int len = strlen(cmd);
    int space_tag = -1;
    for (int i = 0; i < len; i++){
        if (cmd[i] == ' ')
            space_tag = i;
    }
    if (space_tag == -1){
        strcpy(cmdType, cmd);
    } else {
        for (int i = 0; i < space_tag; i++){
            cmdType[i] = cmd[i];
        }
        cmdType[space_tag] = '\0';
        if (space_tag < strlen(cmd)-1){
            cmdContent = (char*)malloc(len - space_tag);
            for (int i = space_tag+1; i < len; i++){
                if (cmd[i] != '\n')
                    cmdContent[(i-space_tag-1)] = cmd[i];
                else
                    cmdContent[(i-space_tag-1)] = '\0';
            }
        }
    }

    for (int i = 0; i < strlen(cmdType); i++){
        if (cmdType[i] == '\r' || cmdType[i] == '\n' || cmdType == '\0') {
            cmdType[i] = '\0';
            break;
        }
    }

    if (cmdContent != NULL){
        for (int i = 0; i < strlen(cmdContent); i++){
            if (cmdContent[i] == '\r' || cmdContent[i] == '\n' || cmdContent == '\0'){
                cmdContent[i] = '\0';
                break;
            }
        }
    }

}

void HandleCommand(serverManager* server){
    char buff[BUFFSIZE];
    memset(buff, 0, sizeof(buff));
    while (1){
        printf("ftp> ");
        fgets(buff, BUFFSIZE, stdin);

        char cmdType[50], *cmdContent = NULL;
        parseCmd(buff, cmdType, cmdContent);

        memset(buff, 0, sizeof(buff));

        //handle command
        if (strcmp(cmdType, "PORT") == 0){
            HandlePORT(server, cmdContent);
        } else if (strcmp(cmdType, "PASV") == 0){
            HandlePASV(server, cmdContent);
        } else if (strcmp(cmdType, "RETR") == 0){
            HandleRETR(server, cmdContent);
        } else if (strcmp(cmdType, "STOR") == 0){
            HandleSTOR(server, cmdContent);
        } else if (strcmp(cmdType, "EXIT") == 0){
            break;
        } else {
            write(server->cmdfd, buff, strlen(buff));
            read(server->cmdfd, buff, sizeof(buff));
            printf(buff);
        }
    }
}

int HandleUSER(serverManager* server){
    char userMess[100] = "USER anonymous";
    write(server->cmdfd, userMess, strlen(userMess));
    char response[100];
    read(server->cmdfd, response, sizeof(response));
    if (strstr(response, "331") != 0)
        return -1;
    else
        return 0;
}

int HandlePASS(serverManager* server, char* password){
    char passMess[100] = "PASS ";
    strcat(passMess, password);
    char response[100];
    read(server->cmdfd, response, sizeof(response));
    if (strstr(response, "230") != 0)
        return -1;
    else
        return 0;
}

int HandlePORT(serverManager* server, char* cmdContent){
    int ip[4], port[2];
    if (cmdContent == NULL){
        char strIP[20];
        getIP(strIP);
        sscanf(strIP, "%d.%d.%d.%d", ip, ip+1, ip+2, ip+3);
        strcpy(server->clientIP, strIP);
        server->clientPORT = 20000;
    } else {
        sscanf(cmdContent, "%d,%d,%d,%d,%d,%d", ip, ip+1, ip+2, ip+3, port, port+1);
        for (int i = 0; i < 4; i++){
            if (ip[i] < 0 || ip[i] > 255)
                return -1;
        }
        for (int i = 0; i < 2; i++){
            if (port[i] < 0 || port[i] > 255)
                return -1;
        }
        char strIP[50];
        int numPORT = port[0]*256 + port[1];
        sprintf(strIP, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        strcpy(server->clientIP, strIP);
        server->clientPORT = numPORT;
    }



    //open a socket and listen on it
    if ((server->filefd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    struct sockaddr_in addr;
    memset(&(addr), 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)server->clientPORT);
    for (int i = 20000; i < 65536; i++){
        if( bind(server->filefd, (struct sockaddr*)&(addr), sizeof(addr)) == -1){
            addr.sin_port = htons((uint16_t)i);
        } else {
            break;
        }
    }
    server->tranMode = 1;
    listen(server->filefd, 10);
}

void HandlePASV(serverManager* server, char* cmdContent){
    int ip[4], port[2];
    if (sscanf(buff, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", ip, ip+1, ip+2, ip+3, port, port+1) != 6)
        return;
    for (int i = 0; i < 4; i++){
        if (ip[i] < 0 || ip[i] > 255)
            return;
    }
    for (int i = 0; i < 2; i++){
        if (port[i] < 0 || port[i] > 255)
            return;
    }
    char strIP[50];
    int numPORT = port[0]*256 + port[1];
    sprintf(strIP, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    strcpy(server->serverIP, strIP);
    server->serverPORT = numPORT;
    server->tranMode = 2;
}

void HandleRETR(serverManager* server, char* cmdContent){
    if (server->tranMode == 0)
        return;
    char buff[BUFFSIZE];
    char filename[256];
    FILE* file;
    int nBytes;
    int datafd;
    strcpy(filename, cmdContent);
    file = fopen(filename, "wb");

    //establish data connection
    if (server->tranMode == 1){//PORT
        datafd = accept(server->filefd, NULL, NULL);
    } else { //PASV
        datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = server->serverPORT;
        inet_pton(AF_INET, server->serverIP, &addr.sin_addr);
        connect(datafd, (struct sockaddr*)&addr, sizeof(addr));
    }
    server->datafd = datafd;

    //begin transmission
    while ((nBytes = read(datafd, buff, BUFFSIZE-1)) > 0){
        write(file, buff, BUFFSIZE-1);
    }

    //over transmission
    close(file);
    close(datafd);
    if (server->tranMode == 1)
        close(server->filefd);
    server->tranMode = 0;
}

void HandleSTOR(serverManager* server, char* cmdContent){
    if (server->tranMode == 0)
        return;
    char buff[BUFFSIZE];
    char filename[256];
    FILE* file;
    int nBytes;
    int datafd;
    strcpy(filename, cmdContent);
    file = fopen(filename, "rb+");

    //establish data connection
    if (server->tranMode == 1){//PORT
        datafd = accept(server->filefd, NULL, NULL);
    } else { //PASV
        datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = server->serverPORT;
        inet_pton(AF_INET, server->serverIP, &addr.sin_addr);
        connect(datafd, (struct sockaddr*)&addr, sizeof(addr));
    }
    server->datafd = datafd;

    //begin transmission
    while ((nBytes = read(file, buff, BUFFSIZE-1)) > 0){
        write(datafd, buff, BUFFSIZE-1);
    }

    //over transmission
    close(file);
    close(datafd);
    if (server->tranMode == 1)
        close(server->filefd);
    server->tranMode = 0;
}

void getClientIPandPORT(int* ip, int *port){
    char strIP[20];
    getIP(strIP);
    sscanf(strIP, "%d.%d.%d.%d");

}