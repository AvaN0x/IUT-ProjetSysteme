#ifndef DEF_SERVER
#define DEF_SERVER

typedef struct
{
    int communicationID;
    struct sockaddr_in connectedAddr;
} connectionStruct;

void *connectionThread(void *);
void UserConnected(int);
void PromptUser(int, char *);
void DisconnectUser(int, char *);

#endif
