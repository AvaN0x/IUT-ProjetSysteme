#ifndef DEF_SERVER
#define DEF_SERVER

#include <netinet/in.h>
#include "concert.h"

typedef struct
{
    int communicationID;
    struct sockaddr_in connectedAddr;
    concertConfigStruct *concertConfig;
} connectionStruct;

void *connectionThread(void *);
void clientConnected(int, concertConfigStruct *);
void disconnectUser(int, stream_t *, char *);
int sendString(int, stream_t *, char *, char *, bool, const char *, ...);
void promptUser(int, stream_t *, char *, int);

#endif
