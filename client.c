
#include "tools.h"

int inputServerInfo(char** targetIP, int* targetPORT);

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
    getSocketMessages(sockfd, sentence);
    printf(sentence);


    //loop to get command from user
    while(1){
        fgets(sentence, 4096, stdin);
        len = strlen(sentence);
        sentence[len] = '\n';
        sentence[len + 1] = '\0';

        //send messages
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

        //get messages
        getSocketMessages(sockfd, sentence);
        printf(sentence);
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
