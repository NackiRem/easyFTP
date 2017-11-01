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
char** getSocketMessages(int sockfd, char* buff);

void sendSocketMessages(int sockfd, char* message);

void printSocketMessages(char** messages);

void stringListAppend(char*** target, char* source);

void freeStringList(char** tofree);

int parseMessages(int sockfd, char** messages, bool* isLogIn);

void ERROR(int sockfd, int errorCode);

void USER(int sockfd, char* command, bool* isLogIn);

void PASS(int sockfd, char* command);

void SYST(int sockfd);

void TYPE(int sockfd, char* command);

void QUIT(int sockfd, bool* isLogIn);