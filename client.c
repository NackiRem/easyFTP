#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <malloc.h>

int inputServerInfo(char** targetIP, int* targetPORT);
char** getSocketMessages(int sockfd, char* buff);
void printServerMessages(char** messages);

void charAppend(char*** target, char* source);
void freeStringList(char** tofree);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in addr;
    char sentence[8192];
    int len;
    int p;
    char *targetIP = NULL;
    char debugIP[] = "127.0.0.1";
    int targetPort = 6789;
    char** serverMessages = NULL;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
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
    addr.sin_port = targetPort;
    if (inet_pton(AF_INET, targetIP, &addr.sin_addr) <= 0) {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
//    free(targetIP);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    //get the initial message from server
    serverMessages = getSocketMessages(sockfd, sentence);
    printServerMessages(serverMessages);
    freeStringList(serverMessages);


    //loop to get command from user
    while(1){
        fgets(sentence, 4096, stdin);
        len = strlen(sentence);
        sentence[len] = '\n';
        sentence[len + 1] = '\0';

        p = 0;
        while (p < len) {
            int n = write(sockfd, sentence + p, len + 1 - p);
            if (n < 0) {
                printf("Error write(): %s(%d)\n", strerror(errno), errno);
                return 1;
            } else {
                p += n;
            }
        }

        serverMessages = getSocketMessages(sockfd, sentence);
        printServerMessages(serverMessages);
        freeStringList(serverMessages);
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
            charAppend(&messages, aMessage);
            temp_begin = i+1;
        }
    }

    return messages;
}


void printServerMessages(char** messages){
    int len = sizeof(messages) / sizeof(messages[0]);
    for (int i = 0; i < len; i++)
        printf(messages[i]);
}

void charAppend(char*** target, char* source){
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