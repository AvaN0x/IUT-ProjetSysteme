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

/**
 * Main function that create the socket, create the concert, and manage client connections
 * @return exit status (EXIT_FAILURE || EXIT_SUCCESS)
 */
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

    //? init the concert structure that will contain every seats
    concertConfigStruct concertConfig = initConcert();

    // Main loop
    int sockaddr_in_size = sizeof(struct sockaddr_in);
    while (1)
    {
        printf("Waiting for connection :\n");
        connectionStruct myConnectionStruct;
        myConnectionStruct.concertConfig = &concertConfig; // set the address of the concertConfig variable

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

/**
 * The thread that will manage the connection
 * @param args instance of connectionStruct as (void *)
 */
void *connectionThread(void *args)
{
    connectionStruct connection = *(connectionStruct *)args;
    printf("Connected client : %s (id : %d)\n", inet_ntoa(connection.connectedAddr.sin_addr), connection.communicationID); //? display client IP

    UserConnected(connection.communicationID, connection.concertConfig);

    close(connection.communicationID);
    pthread_exit(NULL);
}

/**
 * Call function that manage the user connection
 * @param communicationID the id of the communication
 * @param concertConfig address of the concert configuration
 * @return true if the seat is occupied, else false
 */
void UserConnected(int communicationID, concertConfigStruct *concertConfig)
{
    stream_t stream = create_stream(); // stream that is used with this client
    char serStream[STREAM_SIZE];       // serialized stream
    char string[BUFFER_SIZE];

    // concertConfig->seats[communicationID].isOccupied = 1; //? just a test

    //! sending client the list of available seats
    init_stream(&stream, PROMPT_WANTED_SEAT);
    bool *seats = getSeatsStatus(concertConfig);
    set_content(&stream, seats);
    free(seats);
    serialize_stream(&stream, serStream);
    send(communicationID, serStream, sizeof(serStream), 0); // send buffer to client

    //! temporary loop ask client 2 strings
    for (int i = 0; i < 2; i++)
    {
        init_stream(&stream, STRING_AND_PROMPT);
        snprintf(string, BUFFER_SIZE, "Send me something please (%d) : ", i);
        set_content(&stream, string);
        serialize_stream(&stream, serStream);

        send(communicationID, serStream, sizeof(serStream), 0); // send buffer to client

        int bufSize = recv(communicationID, serStream, STREAM_SIZE, 0);
        if (bufSize > 0)
        {
            unserialize_stream(serStream, &stream);

            printf("%d | Received : %s\n", communicationID, (char *)stream.content);
        }
    }

    DisconnectUser(communicationID, &stream, serStream);
    destroy_stream(&stream);
}

/**
 * Disconnect an user from the server
 * @param communicationID the id of the communication
 * @param s the stream to send
 * @param serStream the buffer that will contain the serialized stream
 */
void DisconnectUser(int communicationID, stream_t *s, char *serStream)
{
    init_stream(s, END_CONNECTION);
    serialize_stream(s, serStream);
    send(communicationID, serStream, sizeof(serStream), 0); // send buffer to client
}