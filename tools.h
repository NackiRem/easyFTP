#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>

typedef struct connection{
    int     sockfd;
    bool    isLogIn;
    bool    isPassed;
    int     dataMode;   //0-empty   1-port  2-pasv
    int     ipAndPort[6];
}connection;

//read messages from sockfd, return a stringlist
void getSocketMessages(int sockfd, char* buff);

void sendSocketMessages(int sockfd, char* message);

int parseMessages(connection* connt, char* messages);

void ERROR(connection* connt, int errorCode);

void USER(connection* connt, char* cmdContent);

void PASS(connection* connt, char* cmdContent);

void SYST(connection* connt, char* cmdContent);

void TYPE(connection* connt, char* cmdContent);

void QUIT(connection* connt, char* cmdContent);

void PORT(connection* connt, char* cmdContent);

void PASV(connection* connt, char* cmdContent);

void RETR(connection* connt, char* cmdContent);

void STOR(connection* connt, char* cmdContent);