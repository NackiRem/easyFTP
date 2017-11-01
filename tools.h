#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>

//read messages from sockfd, return a stringlist
void getSocketMessages(int sockfd, char* buff);

void sendSocketMessages(int sockfd, char* message);

void stringListAppend(char*** target, char* source);

void freeStringList(char** tofree);

int parseMessages(int sockfd, char* messages, bool* isLogIn, bool* isPassed);

void ERROR(int sockfd, int errorCode);

void USER(int sockfd, char* cmdContent, bool* isLogIn);

void PASS(int sockfd, char* cmdContent, bool* isLogIn, bool* isPassed);

void SYST(int sockfd, char* cmdContent);

void TYPE(int sockfd, char* cmdContent);

void QUIT(int sockfd, char* cmdContent, bool* isLogIn, bool* isPassed);