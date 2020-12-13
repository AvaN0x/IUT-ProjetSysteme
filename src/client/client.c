#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "client.h"

#define PORT 6000
#define ADDRESS "127.0.0.1"
#define BUFFER_SIZE 100

int main()
{
    // Get the socket
    int fdSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (fdSocket < 0)
    {
        printf("Incorrect socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverCoords;
    memset(&serverCoords, 0x00, sizeof(struct sockaddr_in)); // allocate memory
    serverCoords.sin_family = PF_INET;                       // Set protocal family
    inet_aton(ADDRESS, &serverCoords.sin_addr);              // put server address to our struct
    serverCoords.sin_port = htons(PORT);                     // set address port

    if (connect(fdSocket, (struct sockaddr *)&serverCoords, sizeof(serverCoords)) == -1)
    {
        printf("Connection failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected to %s:%d\n", ADDRESS, PORT);

    // call function that manage the user connection
    ConnectedToServer(fdSocket);

    close(fdSocket);
    return EXIT_SUCCESS;
}

void ConnectedToServer(int fdSocket)
{
    char buffer[BUFFER_SIZE];
    char END_CONNECTION[] = "END_CONNECTION\0";

    while (1) //? wait for the server to ask the user to disconnect
    {
        int bufSize = recv(fdSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bufSize > 1) //? > 1 mean that the server send a string
        {
            buffer[bufSize] = '\0';                  // set last char of the buffer
            if (strcmp(buffer, END_CONNECTION) == 0) //! will close the loop and disconnect the user
                break;
            printf("Received : %s\n", buffer);
        }
        else //? <= 1 mean that the server prompted the user
        {
            printf("Enter some text : ");
            enterText(buffer, BUFFER_SIZE);

            send(fdSocket, buffer, strlen(buffer), 0); // send buffer to server
        }
    }
}

/*
    allow to enter text, and if the text entered is too long, then it will clear the buffer
*/
int enterText(char *buffer, int length)
{
    if (fgets(buffer, length, stdin) != NULL)
    {
        char *lastCharPos = strchr(buffer, '\n');
        if (lastCharPos != NULL)
            *lastCharPos = '\0';
        else
            clearBuffer();
        return 1;
    }
    else
    {
        clearBuffer();
        return EXIT_FAILURE;
    }
}

void clearBuffer()
{
    int c = 0;
    while (c != '\n' && c != EOF)
        c = getchar();
}