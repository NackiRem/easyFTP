#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <fcntl.h>
#include <getopt.h>
#define BUFFSIZE    8192

typedef struct connection{
    int     cmdfd;
    int     filefd;
    int     datafd;
    struct  sockaddr_in cmd_addr;
    struct  sockaddr_in data_addr;
    bool    isLogIn;
    bool    isPassed;
    int     dataMode;   //0-empty   1-port  2-pasv
    int     clientIpAndPort[6];
    int     serverIpAndPort[6];
    char    root[50];
    char    path[100];
}connection;

//read messages from sockfd, return a stringlist
void getSocketMessages(int sockfd, char* buff);

int sendSocketMessages(int sockfd, char* message);

int parseMessages(connection* connt, char* messages);

void ERROR(connection* connt, int errorCode);

void USER(connection* connt, char* cmdContent);

void PASS(connection* connt, char* cmdContent);

void SYST(connection* connt, const char* cmdContent);

void TYPE(connection* connt, char* cmdContent);

void QUIT(connection* connt, const char* cmdContent);

void ABOR(connection* connt, const char* cmdContent);

void PORT(connection* connt, char* cmdContent);

void PASV(connection* connt, const char* cmdContent);

void RETR(connection* connt, char* cmdContent);

void STOR(connection* connt, char* cmdContent);

void MKD(connection* connt, char* cmdContent);

void CWD(connection* connt, char* cmdContent);

void LIST(connection* connt, char* cmdContent);

void RMD(connection* connt, char* cmdContent);

void getIP(char* ip);

void parse_input(int argc, char **argv, int* listening_port, char* root);