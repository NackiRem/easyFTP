
#include "tools.h"
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


int inputServerInfo(char** targetIP, int* targetPORT);
void parseCmd(char* cmd, char* cmdType, char* cmdContent);
void HandlePORT(serverManager* server, char* cmdContent);
void HandlePASV(serverManager* server, char* cmdContent, char* buff);
void HandleRETR(serverManager* server, char* cmdContent);
void HandleSTOR(serverManager* server, char* cmdContent);

int main(int argc, char **argv) {
    serverManager  server;
    server.tranMode = 0;

    struct sockaddr_in addr;
    char sentence[8192];
    int len;
    int p;
    char *targetIP = NULL;
    char debugIP[] = "127.0.0.1";
    int targetPort = 6789;

    if ((server.cmdfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    //get target info
//    if (inputServerInfo(&targetIP, &targetPort)){
//        printf("Failed to getServerInfo!");
//        return 1;
//    }
    targetIP = debugIP;


    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)targetPort);
    if (inet_pton(AF_INET, targetIP, &addr.sin_addr) <= 0) {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
//    free(targetIP);

    if (connect(server.cmdfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    //get the initial message from server
    getSocketMessages(server.cmdfd, sentence);
    printf(sentence);


    //loop to get command from user
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

int inputServerInfo(char** targetIP, int* targetPORT){
    printf("please input target IP address:(divided by \".\")\n");
    char temp[20];
    fgets(temp, 20, stdin);
    int len = strlen(temp);
    *targetIP = (char*)malloc((len));
    if (!(*targetIP)) {
        printf("Not enough memory!");
        return 1;
    } else {
        for (int i = 0; i < len; i++){
            (*targetIP)[i] = temp[i];
        }
        (*targetIP)[len-1] = '\0';
    }

    printf("please input target PORT address:\n");
    scanf("%d", targetPORT);
    return 0;
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
        if (cmdType[i] == '\r' || cmdType[i] == '\n') {
            cmdType[i] = '\0';
            break;
        }
    }
    if (cmdContent != NULL){

    }

}

void HandlePORT(serverManager* server, char* cmdContent){
    int ip[4], port[2];
    if (sscanf(cmdContent, "%d,%d,%d,%d,%d,%d", ip, ip+1, ip+2, ip+3, port, port+1) != 6)
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
    strcpy(server->clientIP, strIP);
    server->clientPORT = numPORT;

    //open a socket and listen on it
    if ((server->filefd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return ;
    }
    struct sockaddr_in addr;
    memset(&(addr), 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = numPORT;
    if( bind(server->filefd, (struct sockaddr*)&(addr), sizeof(addr)) == -1){
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return ;
    }
    server->tranMode = 1;
    listen(server->filefd, 10);
}

void HandlePASV(serverManager* server, char* cmdContent, char* buff){
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