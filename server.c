#include "tools.h"

int main(int argc, char **argv){
    int listenfd, connfd;
    struct sockaddr_in addr;
    char sentence[8192];
    int p;
    int len;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 6789;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    if (listen(listenfd, 10) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    //loop to handle connect request
    while (1) {
        if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            continue;
        }

        bool isLogIn = false;
        bool isPassed = false;
        //send initial message to client
        sendSocketMessages(connfd, "220 Anonymous FTP server ready.\r\n");

        //loop to handle user's command
        while(1){
            getSocketMessages(connfd, sentence);
            parseMessages(connfd, sentence, &isLogIn, &isPassed);
        }


        close(connfd);
    }

    close(listenfd);
}