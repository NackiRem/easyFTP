#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
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

void HandleCommand(serverManager* server);
int HandleUSER(serverManager* server);
int HandlePASS(serverManager* server, char* password);
int HandlePORT(serverManager* server, char* cmdContent);
void HandlePASV(serverManager* server, char* cmdContent);
void HandleRETR(serverManager* server, char* cmdContent);
void HandleSTOR(serverManager* server, char* cmdContent);
void HandleLIST(serverManager* server, char* cmdContent);


void parseCmd(char* cmd, char* cmdType, char* cmdContent);
void clearCharNorR(char* tarString);
void getIP1(char* ip);


int main(int argc, char **argv) {
    serverManager  server;
    server.tranMode = 0;

    initClient(&server);

    if (connectServer(&server) == -1){
        printf("Connection refused.\n");
        return -1;
    }

    loginServer(&server);


    //Handle User's Command
    HandleCommand(&server);

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
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

    char buff[BUFFSIZE];
    read(server->cmdfd, buff, sizeof(buff));
    if (strstr(buff, "220") == NULL){
        printf("Failed to connect server.\n");
        return -1;
    } else {
        printf("Anonymous FTP server ready.\n");
        return 0;
    }
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


    memset(buff, 0, sizeof(buff));
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
            } else {
                memset(buff, 0, sizeof(buff));
                printf("Log in successfully.\n");
                break;
            }
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

    if (strlen(cmdContent) != 0){
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
        cmdContent = (char*)malloc(50);
        memset(cmdContent, 0, sizeof(cmdContent));
        parseCmd(buff, cmdType, cmdContent);

        //handle command
        if (strcmp(cmdType, "PORT") == 0){
            HandlePORT(server, cmdContent);
        } else if (strcmp(cmdType, "PASV") == 0){
            HandlePASV(server, cmdContent);
        } else if (strcmp(cmdType, "RETR") == 0){
            HandleRETR(server, cmdContent);
        } else if (strcmp(cmdType, "STOR") == 0){
            HandleSTOR(server, cmdContent);
        } else if (strcmp(cmdType, "QUIT") == 0){
            sleep(1);
            return;
        } else if (strcmp(cmdType, "ABOR") == 0){
            sleep(1);
            return;
        } else if (strcmp(cmdType, "EXIT") == 0){
            break;
        } else if (strcmp(cmdType, "LIST") == 0){
            HandleLIST(server, cmdContent);
        } else {
            for (int i = 0; i < strlen(buff); i++){
                if (buff[i] == '\n'){
                    buff[i] = '\r';
                    buff[i+1] = '\n';
                    break;
                }
            }
            write(server->cmdfd, buff, strlen(buff));
            memset(buff, 0, sizeof(buff));
            read(server->cmdfd, buff, sizeof(buff));
            printf(buff);
        }
        memset(buff, 0, sizeof(buff));
    }
}

int HandleUSER(serverManager* server){
    char userMess[100] = "USER anonymous\r\n";
    write(server->cmdfd, userMess, strlen(userMess));
    char response[100];
    read(server->cmdfd, response, sizeof(response));
    if (strstr(response, "331") == NULL)
        return -1;
    else
        return 0;
}

int HandlePASS(serverManager* server, char* password){
    char passMess[100] = "PASS ";
    strcat(passMess, password);
    strcat(passMess, "\r\n");
    write(server->cmdfd, passMess, strlen(passMess));
    char response[100];
    read(server->cmdfd, response, sizeof(response));
    if (strstr(response, "230") == NULL)
        return -1;
    else
        return 0;
}

int HandlePORT(serverManager* server, char* cmdContent){
    int ip[4], port[2];
    if (strlen(cmdContent) == 0){
        char strIP[20];
        getIP1(strIP);
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
            server->clientPORT = i;
            port[0] = i / 256;
            port[1] = i % 256;
            break;
        }
    }
    listen(server->filefd, 10);

    char buff[BUFFSIZE];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "PORT %d,%d,%d,%d,%d,%d\r\n", ip[0], ip[1], ip[2], ip[3], port[0], port[1]);
    write(server->cmdfd, buff, strlen(buff));
    memset(buff, 0, sizeof(buff));
    read(server->cmdfd, buff, BUFFSIZE);
    if (strstr(buff, "200") != NULL){
        printf("mode: PORT\n");
    } else {
        printf("Failed to enter port mode!\n");
        return -1;
    }
    server->tranMode = 1;
}

void HandlePASV(serverManager* server, char* cmdContent){
    int ip[4], port[2];
    char buff[BUFFSIZE];
    memset(buff, 0, sizeof(buff));
    if (strlen(cmdContent) != 0){
        printf("Error: Extra parameters.\n");
        return;
    }
    strcpy(buff, "PASV\r\n");
    write(server->cmdfd, buff, strlen(buff));
    memset(buff, 0, sizeof(buff));
    read(server->cmdfd, buff, sizeof(buff));
    if (strstr(buff, "227") != NULL){
        printf("mode: PASV\n");
    } else {
        printf("Failed to enter passive mode!\n");
        return;
    }
    sscanf(buff, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", ip, ip+1, ip+2, ip+3, port, port+1);

    for (int i = 0; i < 4; i++){
        if (ip[i] < 0 || ip[i] > 255){
            printf("Error: Server send wrong IP address.\n");
            return;
        }
    }
    for (int i = 0; i < 2; i++){
        if (port[i] < 0 || port[i] > 255) {
            printf("Error: Server send wrong Port.\n");
            return;
        }
    }
    server->serverPORT = port[0]*256 + port[1];
    sprintf(server->serverIP, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    server->tranMode = 2;
}

void HandleRETR(serverManager* server, char* cmdContent){
    if (strlen(cmdContent) == 0){
        printf("Error: Please input file name.\n");
        return;
    }
    if (server->tranMode == 0){
        printf("Error: Please select transmission mode.\n");
        return;
    }
    char buff[BUFFSIZE];
    char response1[256], response2[256];
    char filename[256];
    int file;
    ssize_t nBytes;
    int datafd;
    memset(buff, 0, sizeof(buff));
    memset(response1, 0, sizeof(response1));
    memset(response2, 0, sizeof(response2));
    sprintf(buff, "RETR %s\r\n", cmdContent);
    write(server->cmdfd, buff, strlen(buff));
    strcpy(filename, cmdContent);
    if ((file = open(filename, O_WRONLY|O_CREAT, S_IRWXU)) < 0){
        printf("Error: Failed to open local file.\n");
        return;
    }

    //establish data connection
    if (server->tranMode == 1){//PORT
        datafd = accept(server->filefd, NULL, NULL);
    } else { //PASV
        datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)server->serverPORT);
        inet_pton(AF_INET, server->serverIP, &addr.sin_addr);
        connect(datafd, (struct sockaddr*)&addr, sizeof(addr));
    }
    memset(buff, 0, sizeof(buff));
    while(1){
        read(server->cmdfd, buff, sizeof(buff));
        if (strstr(buff, "501") || strstr(buff, "425") || strstr(buff, "426") || strstr(buff, "451")){
            printf(buff);
            return;
        } else if (strstr(buff, "226")){
            break;
        }
    }

    server->datafd = datafd;

    //begin transmission
    while ((nBytes = read(datafd, buff, BUFFSIZE-1)) > 0){
        write(file, buff, nBytes);
    }


    //over transmission
    close(file);
    close(datafd);
    if (server->tranMode == 1)
        close(server->filefd);
    server->tranMode = 0;
}

void HandleSTOR(serverManager* server, char* cmdContent){
    if (strlen(cmdContent) == 0){
        printf("Error: Please input file name.\n");
        return;
    }
    if (server->tranMode == 0){
        printf("Error: Please select transmission mode.\n");
        return;
    }
    char buff[BUFFSIZE];
    char response1[256], response2[156];
    char filename[256];
    int file;
    ssize_t nBytes;
    int datafd;
    memset(buff, 0, sizeof(buff));
    memset(response1, 0, sizeof(response1));
    memset(response2, 0, sizeof(response2));
    sprintf(buff, "STOR %s\r\n", cmdContent);
    write(server->cmdfd, buff, strlen(buff));


    //open file
    strcpy(filename, cmdContent);
    if ((file = open(filename, O_RDONLY)) < 0){
        printf("Error: Failed to open local file.\n");
        return;
    }

    //establish data connection
    if (server->tranMode == 1){//PORT
        datafd = accept(server->filefd, NULL, NULL);
    } else { //PASV
        datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)server->serverPORT);
        inet_pton(AF_INET, server->serverIP, &addr.sin_addr);
        connect(datafd, (struct sockaddr*)&addr, sizeof(addr));
    }
    server->datafd = datafd;

    //begin transmission
    memset(buff, 0, sizeof(buff));
    while ((nBytes = read(file, buff, BUFFSIZE-1)) > 0){
        write(datafd, buff, nBytes);
    }

    memset(buff, 0, sizeof(buff));
    while(1){
        read(server->cmdfd, buff, sizeof(buff));
        if (strstr(buff, "501") || strstr(buff, "425") || strstr(buff, "426") || strstr(buff, "451")){
            printf(buff);
            return;
        } else if (strstr(buff, "226")){
            break;
        }
    }

    //over transmission
    close(file);
    close(datafd);
    if (server->tranMode == 1)
        close(server->filefd);
    server->tranMode = 0;
}

void HandleLIST(serverManager* server, char* cmdContent){
    char buff[BUFFSIZE];
    if (server->tranMode == 0){
        printf("Error: Please select transmission mode.\n");
        return;
    }
    if (strlen(cmdContent) != 0)
        sprintf(buff, "LIST %s\r\n", cmdContent);
    else
        strcpy(buff, "LIST\r\n");
    write(server->cmdfd, buff, strlen(buff));
    sleep(1);

    ssize_t nBytes;
    int datafd;

    if (server->tranMode == 1){//PORT
        datafd = accept(server->filefd, NULL, NULL);
    } else { //PASV
        datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)server->serverPORT);
        inet_pton(AF_INET, server->serverIP, &addr.sin_addr);
        connect(datafd, (struct sockaddr*)&addr, sizeof(addr));
    }

    memset(buff, 0, sizeof(buff));
    while(1){
        read(server->cmdfd, buff, sizeof(buff));
        if (strstr(buff, "501") || strstr(buff, "425") || strstr(buff, "426") || strstr(buff, "451")){
            printf(buff);
            return;
        } else if (strstr(buff, "226")){
            break;
        }
    }
    server->datafd = datafd;


    memset(buff, 0, sizeof(buff));
    while(1){
        if ((nBytes = read(server->datafd, buff, BUFFSIZE-1)) >= 0){
            printf("%s", buff);
            if (nBytes == 0 || (nBytes < BUFFSIZE-1))
                break;
        } else {
            printf("Error read.\n");
            return;
        }
    }

    close(datafd);
    if (server->tranMode == 1)
        close(server->filefd);
    server->tranMode = 0;
}

void getIP1(char* ip){
    system("ifconfig | grep inet[^6] | awk \'{if(NR == 2){print $2}}\' > a.txt");
    FILE *fp1 = fopen("a.txt", "r");
    fgets(ip, 1024, fp1);
    fclose(fp1);
    remove("a.txt");
}