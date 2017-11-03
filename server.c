#include "tools.h"
#define default_root "/tmp"

int main(int argc, char **argv){
    int listenfd;
    struct sockaddr_in addr;
    char sentence[BUFFSIZE];
    int p;
    int len;
    int listen_port = 6789;
    char working_dir[100] = default_root;
    parse_input(argc, argv, &listen_port, working_dir);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)listen_port);
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
        connection connt;
        connt.dataMode = 0;
        connt.isLogIn = false;
        connt.isPassed = false;
        strcpy(connt.root, working_dir);
        strcpy(connt.path, "/\0");

        if ((connt.cmdfd = accept(listenfd, NULL, NULL)) == -1){
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            continue;
        }

        int pid = fork();
        if (pid < 0){
            printf("Error fork()\n");
        } else if (pid == 0){
            //send initial message to client
            sendSocketMessages(connt.cmdfd, "220 Anonymous FTP server ready.\r\n");
            //loop to handle user's command
            while(1){
                getSocketMessages(connt.cmdfd, sentence);
                if (parseMessages(&connt, sentence) == -1)
                    break;
            }
            close(connt.cmdfd);
            break;
        }
    }

    close(listenfd);
}