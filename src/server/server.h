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
void UserConnected(int);
void DisconnectUser(int, stream_t *, char *);

#endif
