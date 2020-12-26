#ifndef DEF_SERVER
#define DEF_SERVER

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
void sendString(int, stream_t *, char *, char *, const char *, ...);

#endif
