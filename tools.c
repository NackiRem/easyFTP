#include "tools.h"

void getSocketMessages(int sockfd, char* buff){

    //get message from server
    int p = 0;
    while(1){
        int n = (int)read(sockfd, buff+p, (size_t)(8191-p));
        if (n < 0){
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            return ;
        } else if (n == 0){
            break;
        } else {
            p += n;
            if (buff[p-2] == '\n' || buff[p-2] == '\r')
                break;
        }
    }
    buff[p] = '\0';
}

int sendSocketMessages(int sockfd, char* message){
    int len = (int)strlen(message);
    int p = 0;
    while (p < len){
        int n = (int)write(sockfd, message+p, (size_t)(len-p));
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
    int len = (int)strlen(command);
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
        int tmp = space_tag;
        tmp += 1;
        commandType = (char*)malloc((size_t)tmp);
        for (int i = 0; i < space_tag; i++){
            commandType[i] = command[i];
        }
        commandType[space_tag] = '\0';
        if (space_tag < strlen(command)-1){
            commandContent = (char*)malloc((size_t)(len - space_tag));
            for (int i = space_tag+1; i < len; i++){
                if (command[i] != '\n')
                    commandContent[(i-space_tag-1)] = command[i];
                else
                    commandContent[(i-space_tag-1)] = '\0';
            }
        }
    }

    // clear \r \n \0 in the command
    for (int i = 0; i < strlen(commandType); i++){
        if (commandType[i] == '\r' || commandType[i] == '\n' || commandType[i] == '\0'){
            commandType[i] = '\0';
            break;
        }
    }


    if (commandContent != NULL){
        for (int i = 0; i < strlen(commandContent); i++){
            if (commandContent[i] == '\r' || commandContent[i] == '\n' || commandContent[i] == '\0'){
                commandContent[i] = '\0';
                break;
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
        return -1;
    } else if (strcmp(commandType, "ABOR") == 0){
        ABOR(connt, commandContent);
        return -1;
    } else if (strcmp(commandType, "PORT") == 0){
        PORT(connt, commandContent);
    } else if (strcmp(commandType, "PASV") == 0){
        PASV(connt, commandContent);
    } else if (strcmp(commandType, "RETR") == 0){
        RETR(connt, commandContent);
    } else if (strcmp(commandType, "STOR") == 0){
        STOR(connt, commandContent);
    } else if (strcmp(commandType, "MKD") == 0){
        MKD(connt, commandContent);
    } else if (strcmp(commandType, "CWD") == 0){
        CWD(connt, commandContent);
    } else if (strcmp(commandType, "LIST") == 0){
        LIST(connt, commandContent);
    } else if (strcmp(commandType, "RMD") == 0){
        RMD(connt, commandContent);
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
            sendSocketMessages(connt->cmdfd, "501 Syntax Error in parameters!123\r\n");
            break;
        case 550:
            sendSocketMessages(connt->cmdfd, "550 No such file or directory.\r\n");
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
        sendSocketMessages(connt->cmdfd, "230 Guest login ok.\r\n");
        connt->isPassed = true;
    }
    else
        ERROR(connt, 501);

}

void SYST(connection* connt, const char* cmdContent){
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

void QUIT(connection* connt, const char* cmdContent){
    if (cmdContent == NULL){
        connt->isLogIn = false;
        connt->isPassed = false;
        sendSocketMessages(connt->cmdfd, "221 Goodbye.\r\n");
    } else{
        ERROR(connt, 501);
    }
}

void ABOR(connection* connt, const char* cmdContent){
    if (cmdContent == NULL){
        connt->isLogIn = false;
        connt->isPassed = false;
        sendSocketMessages(connt->cmdfd, "221 Goodbye.\r\n");
    } else{
        ERROR(connt, 501);
    }
}

void PORT(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(connt, 501);
        return;
    }
    int *ip = connt->clientIpAndPort;
    sscanf(cmdContent, "%d,%d,%d,%d,%d,%d", ip, ip+1, ip+2, ip+3, ip+4, ip+5);
    for (int i = 0; i < 6; i++){
        if (ip[i] < 0 || ip[i] > 255){
            ERROR(connt, 501);
            return;
        }
    }
    connt->dataMode = 1;
    sendSocketMessages(connt->cmdfd, "200 PORT command successful.\r\n");
}

void PASV(connection* connt, const char* cmdContent){
    if (cmdContent != NULL){
        ERROR(connt, 501);
        return;
    }
    int serverPORT = 0;
    connt->dataMode = 2;

    //open a socket and listen on it
    if ((connt->filefd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return ;
    }
    memset(&(connt->data_addr), 0, sizeof(connt->data_addr));
    connt->data_addr.sin_family = AF_INET;
    connt->data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    for (int i = 20000; i < 65536; i++){
        connt->data_addr.sin_port = htons((uint16_t)i);
        if (bind(connt->filefd, (struct sockaddr*)&(connt->data_addr), sizeof(connt->data_addr)) != -1) {
            serverPORT = i;
            break;
        }
    }
    listen(connt->filefd, 10);

    //return some messages to client
    char ip[20], message[100] = "227 Entering Passive Mode (", ipandport[50];
    int *pIP = connt->serverIpAndPort;
    getIP(ip);
    sscanf(ip, "%d.%d.%d.%d", pIP, pIP+1, pIP+2, pIP+3);
    pIP[4] = serverPORT / 256;
    pIP[5] = serverPORT % 256;
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
    size_t nBytes;
    int file;
    int datafd;
    sprintf(filename, "%s%s%s", connt->root, connt->path, cmdContent);
    if (access(filename, R_OK) == -1){
        ERROR(connt, 451);
        return;
    }
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
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)(connt->clientIpAndPort[4]*256 + connt->clientIpAndPort[5]));
        char clientIP[100];
        sprintf(clientIP, "%d.%d.%d.%d", connt->clientIpAndPort[0], connt->clientIpAndPort[1],
                connt->clientIpAndPort[2], connt->clientIpAndPort[3]);
        inet_pton(AF_INET, clientIP, &addr.sin_addr);
        if (connect(datafd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            ERROR(connt, 425);
            return ;
        }
        sprintf(response, "150 Opening BINARY mode data connection for %s\r\n", filename);
        sendSocketMessages(connt->cmdfd, response);
    } else {    //MODE PASV
        if ((datafd = accept(connt->filefd, NULL, NULL)) == -1){
            ERROR(connt, 425);
            return;
        }
        sprintf(response, "150 Opening BINARY mode data connection for %s\r\n", filename);
        sendSocketMessages(connt->cmdfd, response);
    }
    connt->datafd = datafd;

    //begin transmission
    while ((nBytes = (size_t)read(file, buff, BUFFSIZE-1)) >= 0){
        write(connt->datafd, buff, nBytes);
        if (nBytes == 0)
            break;
    }

    //over the transmission
    close(file);
    close(datafd);
    if (connt->dataMode == 2)
        close(connt->filefd);
    connt->dataMode = 0;

    if (nBytes == -1){
        ERROR(connt, 426);
    } else {
        sendSocketMessages(connt->cmdfd, "226 Transfer complete.\r\n");
    }

}

void STOR(connection* connt, char* cmdContent){
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
    int file;
    size_t nBytes;
    int datafd;
    sprintf(filename, "%s%s%s", connt->root, connt->path, cmdContent);
    if ((file = open(filename, O_WRONLY | O_CREAT, S_IRWXU)) < 0){
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
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)(connt->clientIpAndPort[4]*256 + connt->clientIpAndPort[5]));
        char clientIP[100];
        sprintf(clientIP, "%d.%d.%d.%d", connt->clientIpAndPort[0], connt->clientIpAndPort[1],
                connt->clientIpAndPort[2], connt->clientIpAndPort[3]);
        inet_pton(AF_INET, clientIP, &addr.sin_addr);
        if (connect(datafd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            ERROR(connt, 425);
            return ;
        }
        sprintf(response, "150 Opening BINARY mode data connection for %s\r\n", filename);
        sendSocketMessages(connt->cmdfd, response);
    } else {    //MODE PASV
        if ((datafd = accept(connt->filefd, NULL, NULL)) == -1){
            ERROR(connt, 425);
            return;
        }
        sprintf(response, "150 Opening BINARY mode data connection for %s\r\n", filename);
        sendSocketMessages(connt->cmdfd, response);
    }
    connt->datafd = datafd;

    //begin transmission
    while ((nBytes = (size_t)read(datafd, buff, BUFFSIZE-1)) >= 0){
        write(file, buff, nBytes);
        if (nBytes == 0)
            break;
    }

    //over the transmission
    close(file);
    close(datafd);
    if (connt->dataMode == 2)
        close(connt->filefd);
    connt->dataMode = 0;

    if (nBytes == -1){
        ERROR(connt, 426);
    } else {
        sendSocketMessages(connt->cmdfd, "226 Transfer complete.\r\n");
    }
}

void MKD(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(connt, 501);
        return;
    }
    if ((strstr(cmdContent, "..") != NULL)){
        ERROR(connt, 501);
        return;
    }
    char command[100] = "mkdir ";
    char tarPath[100] = "\0";
    strcat(tarPath, connt->path);
    strcat(command, connt->root);
    strcat(command, tarPath);
    strcat(command, cmdContent);
    if (system(command) == 0){
        char response[100] = "250 successfully created directory.\r\n";
        strcat(tarPath, "\r\n");
        strcat(response, tarPath);
        sendSocketMessages(connt->cmdfd, response);
    } else {
        sendSocketMessages(connt->cmdfd, "550 Failed to mkdir.\r\n");
    }

}

void CWD(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(connt, 501);
        return;
    }
    if ((strstr(cmdContent, "..") != NULL)){
        ERROR(connt, 501);
        return;
    }
    char command[100] = "cd ";
    char tarPath[100] = "\0";
    strcpy(tarPath, connt->path);
    if (strlen(cmdContent) == 0){
        ERROR(connt, 501);
        return;
    }
    if (cmdContent[0] == '/'){
        strcpy(tarPath, cmdContent);
    } else {
        strcat(tarPath, cmdContent);
    }
    if (tarPath[(strlen(tarPath)-1)] != '/')
        strcat(tarPath, "/");

    strcat(command, connt->root);
    strcat(command, tarPath);

    if (system(command) != 0){
        ERROR(connt, 550);
    } else {
        char response[100] = "250 okay.\r\n";
        strcat(response, tarPath);
        strcat(response, "\r\n");
        sendSocketMessages(connt->cmdfd, response);
    }
}

void LIST(connection* connt, char* cmdContent){
    if (connt->isLogIn == false){
        ERROR(connt, 332);
        return;
    }
    char command[100];
    if (cmdContent != NULL){
        sprintf(command, "ls %s", cmdContent);
    } else {
        strcpy(command, "ls ");
    }
    strcat(command, " ");
    strcat(command, connt->root);
    strcat(command, connt->path);
    char response[100];
    memset(response, 0, 100);
    if (connt->dataMode == 0){
        strcpy(response, "Please choose transmission mode.\r\n");
        sendSocketMessages(connt->cmdfd, response);
    }

    int datafd;
    if (connt->dataMode == 1){
        sendSocketMessages(connt->cmdfd, "150 Opening BINARY mode data connection.\r\n");

        if ((datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
            ERROR(connt, 425);
            return;
        }
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)(connt->clientIpAndPort[4]*256 + connt->clientIpAndPort[5]));
        char clientIP[100];
        sprintf(clientIP, "%d.%d.%d.%d", connt->clientIpAndPort[0], connt->clientIpAndPort[1],
                connt->clientIpAndPort[2], connt->clientIpAndPort[3]);
        inet_pton(AF_INET, clientIP, &addr.sin_addr);
        if (connect(datafd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            ERROR(connt, 425);
            return ;
        }
    } else{
        sendSocketMessages(connt->cmdfd, "150 Opening BINARY mode data connection.\r\n");

        if ((datafd = accept(connt->filefd, NULL, NULL)) == -1) {
            ERROR(connt, 425);
            return;
        }
    }

    FILE* fp;
    if ((fp = popen(command, "r")) == NULL){
        sendSocketMessages(connt->cmdfd, "550 Failed to List.\r\n");
        return;
    }

    char buff[BUFFSIZE];
    while(1){
        size_t n = fread(buff, 1, BUFFSIZE, fp);
        if ((write(datafd, buff, n)) < 0){
            sendSocketMessages(connt->cmdfd, "426 Failed to List.\r\n");
            pclose(fp);
            break;
        }
        if (n <= 0)
            break;
    }
    pclose(fp);
    sendSocketMessages(connt->cmdfd, "226 List successfully.\r\n");

    if (connt->dataMode == 2)
        close(connt->filefd);
    connt->dataMode = 0;
}

void RMD(connection* connt, char* cmdContent){
    if (cmdContent == NULL){
        ERROR(connt, 501);
        return;
    }
    if ((strstr(cmdContent, "..") != NULL)){
        ERROR(connt, 501);
        return;
    }
    char command[100] = "rm -rf ";
    char tarPath[100] = "\0";

    strcat(tarPath, connt->path);

    if (strlen(cmdContent) == 0){
        ERROR(connt, 501);
        return;
    }
    if (cmdContent[0] == '/'){
        ERROR(connt, 501);
    } else {
        strcat(tarPath, cmdContent);
    }

    strcat(command, connt->root);
    strcat(command, tarPath);
    if (system(command) == 0){
        char response[100] = "250 okay.\r\n";
        strcat(tarPath, "\r\n");
        strcat(response, tarPath);
        sendSocketMessages(connt->cmdfd, response);
    } else {
        sendSocketMessages(connt->cmdfd, "550 Failed to mkdir.\r\n");
    }
}

void getIP(char* ip){
    system("ifconfig | grep inet[^6] | awk \'{if(NR == 2){print $2}}\' > a.txt");
    FILE *fp1 = fopen("a.txt", "r");
    fgets(ip, 1024, fp1);
    fclose(fp1);
    remove("a.txt");
}

void parse_input(int argc, char **argv, int* listening_port, char* working_dir){
    struct option long_options[] = {
            {"root", required_argument, NULL, 'r'},
            {"port", required_argument, NULL, 'p'},
            {NULL, 0, NULL, 0}
    };
    while (1){
        switch (getopt_long_only(argc, argv, "r:p:", long_options, NULL)){
            case 'r':
                strcpy(working_dir, optarg);
                break;
            case 'p':
                *listening_port = atoi(optarg);
                break;
            default:
                return;
        }
    }
}