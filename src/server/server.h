#ifndef DEF_SERVER
#define DEF_SERVER

typedef struct
{
    int fdSocketCommunication;
    struct sockaddr_in coordonneesAppelant;
} connectionStruct;

void *connectionThread(void *);
void UserConnected(int);
void PromptUser(int, char *);
void DisconnectUser(int, char *);

#endif
