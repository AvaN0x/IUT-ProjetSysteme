#ifndef DEF_SERVER
#define DEF_SERVER

typedef struct
{
    int communicationID;
    struct sockaddr_in connectedAddr;
} connectionStruct;

void *connectionThread(void *);
void UserConnected(int);
void DisconnectUser(int, stream_t *, char *);

#endif
