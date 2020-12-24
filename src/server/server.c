#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <pthread.h>

#include "../common/stream.h"
#include "server.h"

int main()
{
    int serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        printf("Incorrect socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0x00, sizeof(struct sockaddr_in)); // allocate memory
    serverAddr.sin_family = PF_INET;                       // Set protocal family
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);        // set address
    serverAddr.sin_port = htons(PORT);                     // set address port

    if (bind(serverSocket, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) == -1)
    {
        printf("Bind error, PORT may already be in use.\n");
        exit(EXIT_FAILURE);
    }
    if (listen(serverSocket, 5) == -1)
    {
        printf("Listen error.\n");
        exit(EXIT_FAILURE);
    }

    // Main loop
    int sockaddr_in_size = sizeof(struct sockaddr_in);
    while (1)
    {
        printf("Waiting for connection :\n");
        connectionStruct myConnectionStruct;

        //? waiting for a connection to come
        if ((myConnectionStruct.communicationID = accept(serverSocket, (struct sockaddr *)&myConnectionStruct.connectedAddr,
                                                         &sockaddr_in_size)) != -1)
        {
            //? create the thread that will manage the connection
            pthread_t thread;
            int threadReturn = pthread_create(&thread, NULL, connectionThread, (void *)&myConnectionStruct);
        }
        else
        {
            printf("Connection acceptation error\n");
        }
    }
    close(serverSocket);

    return EXIT_SUCCESS;
}

void *connectionThread(void *args)
{
    connectionStruct connection = *(connectionStruct *)args;
    printf("Connected client : %s (id : %d)\n", inet_ntoa(connection.connectedAddr.sin_addr), connection.communicationID); //? display client IP

    UserConnected(connection.communicationID);

    close(connection.communicationID);
    pthread_exit(NULL);
}

// call function that manage the user connection
void UserConnected(int communicationID)
{

    stream_t stream = create_stream(); // stream that is used with this client
    char serStream[STREAM_SIZE];       // serialized stream
    char string[BUFFER_SIZE];

    //! temporary loop ask client 2 strings
    for (int i = 0; i < 2; i++)
    {
        init_stream(&stream, WRITE_AND_PROMPT);
        snprintf(string, BUFFER_SIZE, "Send me something please (%d) : ", i);
        set_content(&stream, string);
        serialize_stream(&stream, serStream);

        send(communicationID, serStream, sizeof(serStream), 0); // send buffer to client

        // PromptUser(communicationID, buffer);
        int bufSize = recv(communicationID, string, BUFFER_SIZE, 0);
        if (bufSize > 0)
        {
            string[bufSize] = '\0'; // set last char of the buffer
            printf("%d | Received : %s\n", communicationID, string);
        }
    }

    DisconnectUser(communicationID, &stream, serStream);
    destroy_stream(&stream);
}

void DisconnectUser(int communicationID, stream_t *s, char *serStream)
{
    init_stream(s, END_CONNECTION);
    serialize_stream(s, serStream);
    send(communicationID, serStream, sizeof(serStream), 0); // send buffer to client
}