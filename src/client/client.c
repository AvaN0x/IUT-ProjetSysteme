#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "../common/stream.h"
#include "client.h"

#define ADDRESS "127.0.0.1"

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
    stream_t stream = create_stream(); // received stream

    char serStream[STREAM_SIZE]; // serialized stream
    char string[BUFFER_SIZE];

    while (1) //? wait for the server to ask the user to disconnect
    {
        int bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
        if (bufSize > 0)
        {
            unserialize_stream(serStream, &stream);

            if (stream.type == END_CONNECTION)
                break;

            switch (stream.type)
            {
            case WRITE:
            case WRITE_AND_PROMPT:
                printf("%s", (char *)stream.content);
                break;
            default:
                break;
            }

            switch (stream.type)
            {
            case PROMPT:
            case WRITE_AND_PROMPT:
                enterText(string, BUFFER_SIZE);

                send(fdSocket, string, strlen(string), 0); // send buffer to server

                break;
            default:
                break;
            }
        }
    }
    destroy_stream(&stream);
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