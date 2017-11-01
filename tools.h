#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <malloc.h>

char** getSocketMessages(int sockfd, char* buff);
void printSocketMessages(char** messages);
void stringListAppend(char*** target, char* source);
void freeStringList(char** tofree);