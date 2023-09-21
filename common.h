#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define TRAIL_UP 4

#define SERV_PORT 5555
#define XMAX 44
#define YMAX 155
#define TIE -2
#define WIN 2
#define LOSE 1

#define TRAIL_INDEX_SHIFT 50

#define SA struct sockaddr
#define SAI struct sockaddr_in
#define TV struct timeval

#define WALL 111
#define VOID 32

#define TRUE 1
#define FALSE 0

#define STDIN 0

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

void raler(char *msg)
{
    perror(msg);
    exit(1);
}

#define CHECK(op)               \
    do                          \
    {                           \
        if ((op) == -1)         \
        {                       \
            perror(#op);        \
            exit(EXIT_FAILURE); \
        }                           \
    } while (0)

typedef struct display_info
{
    char board[XMAX][YMAX];
    int winner;
} display_info;

struct client_input
{
    int id;
    char input;
};

struct client_init_infos
{                                      
    int nb_players;
};

#endif