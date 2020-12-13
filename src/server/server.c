#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include "server.h"

#define PORT 6000
#define BUFFER_SIZE 100

int main()
{
    int fdSocketAttente = socket(PF_INET, SOCK_STREAM, 0);
    if (fdSocketAttente < 0)
    {
        printf("Incorrect socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in coordonneesServeur;
    memset(&coordonneesServeur, 0x00, sizeof(struct sockaddr_in)); // allocate memory
    coordonneesServeur.sin_family = PF_INET;                       // Set protocal family
    coordonneesServeur.sin_addr.s_addr = htonl(INADDR_ANY);        // set address
    coordonneesServeur.sin_port = htons(PORT);                     // set address port

    if (bind(fdSocketAttente, (struct sockaddr *)&coordonneesServeur,
             sizeof(coordonneesServeur)) == -1)
    {
        printf("Bind error, PORT may already be in use.\n");
        exit(EXIT_FAILURE);
    }
    if (listen(fdSocketAttente, 5) == -1)
    {
        printf("Listen error.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in coordonneesAppelant;
    socklen_t tailleCoord = sizeof(coordonneesAppelant);

    // Main loop
    int sortie = 0;
    while (sortie != 1)
    {
        printf("Waiting for connection :\n");
        int fdSocketCommunication;
        if ((fdSocketCommunication = accept(fdSocketAttente, (struct sockaddr *)&coordonneesAppelant,
                                            &tailleCoord)) == -1)
        {
            printf("Connection acceptation error\n");
            exit(EXIT_FAILURE);
        }
        printf("Connected client : %s\n", inet_ntoa(coordonneesAppelant.sin_addr)); // display client IP

        UserConnected(fdSocketCommunication);

        close(fdSocketCommunication);
        printf("\n");
    }
    close(fdSocketAttente);

    return EXIT_SUCCESS;
}

// call function that manage the user connection
void UserConnected(int fdSocketCommunication)
{
    char buffer[BUFFER_SIZE];

    //! temporary loop to chat 2 times with the client
    for (int i = 0; i < 2; i++)
    {
        snprintf(buffer, BUFFER_SIZE, "Send me something please (%d)", i);
        send(fdSocketCommunication, buffer, strlen(buffer), 0); // send buffer to client

        PromptUser(fdSocketCommunication, buffer);
        int bufSize = recv(fdSocketCommunication, buffer, BUFFER_SIZE - 1, 0);
        if (bufSize > 0)
        {
            buffer[bufSize] = '\0'; // set last char of the buffer
            printf("Received : %s\n", buffer);
        }
    }

    DisconnectUser(fdSocketCommunication, buffer);
}

void PromptUser(int fdSocketCommunication, char *buffer)
{
    snprintf(buffer, BUFFER_SIZE, "\n");                    //? if length is equal to 1, then the user will be prompted
    send(fdSocketCommunication, buffer, strlen(buffer), 0); // send buffer to client
}

void DisconnectUser(int fdSocketCommunication, char *buffer)
{
    snprintf(buffer, BUFFER_SIZE, "END_CONNECTION");        //? this string will disconnect the user
    send(fdSocketCommunication, buffer, strlen(buffer), 0); // send buffer to client
}