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
#include <stdarg.h> // for infinite parameters
#include <time.h>   // for random functions

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

    //? set the randomness of the program
    srand((unsigned int)time(NULL));

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
    printf("%d | Connected client : %s\n", connection.communicationID, inet_ntoa(connection.connectedAddr.sin_addr)); //? display client IP

    clientConnected(connection.communicationID, connection.concertConfig);

    close(connection.communicationID);
    pthread_exit(NULL);
}

/**
 * Call function that manage the user connection
 * @param communicationID the id of the communication
 * @param concertConfig address of the concert configuration
 * @return true if the seat is occupied, else false
 */
void clientConnected(int communicationID, concertConfigStruct *concertConfig)
{
    stream_t stream = create_stream(); // stream that is used with this client
    char serStream[STREAM_SIZE];       // serialized stream
    size_t serStreamSize;
    char string[BUFFER_SIZE];
    int8_t maxIntValue;
    bool loop = 1;

    while (loop)
    {
        sendString(communicationID, &stream, string, serStream, 0, "\n*------- CONCERT -------*\n0/ Quitter\n1/ Réserver un ticket\n2/ Annuler un ticket\nChoix : ");

        init_stream(&stream, PROMPT_INT_WITH_MAX);
        maxIntValue = 2;
        set_content(&stream, &maxIntValue);
        serStreamSize = serialize_stream(&stream, serStream);
        send(communicationID, serStream, serStreamSize, 0); // send buffer to client

        int bufSize = recv(communicationID, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            loop = 0;
            continue;
        }

        unserialize_stream(serStream, &stream);

        if (stream.type == INT)
        {
            int receivedInt = *(int8_t *)stream.content;
            switch (receivedInt)
            {
            case 0:
                loop = 0; //? stop the loop, which will disconnect the user
                sendString(communicationID, &stream, string, serStream, 0, "Passez une bonne journée, aurevoir !\n");
                break;

            case 1:
                reserveTicket(&loop, communicationID, concertConfig, &stream, string, serStream);
                break;

            default:
                break;
            }
        }
    }

    disconnectUser(communicationID, &stream, serStream);
    destroy_stream(&stream);
}

/**
 * Disconnect an user from the server
 * @param communicationID the id of the communication
 * @param s the stream to send
 * @param serStream the buffer that will contain the serialized stream
 */
void disconnectUser(int communicationID, stream_t *s, char *serStream)
{
    init_stream(s, END_CONNECTION);
    size_t serStreamSize = serialize_stream(s, serStream);
    send(communicationID, serStream, serStreamSize, 0); // send buffer to client
    printf("%d | Client disconnected\n", communicationID);
}

/**
 * Send user a string
 * @param communicationID the id of the communication
 * @param stream the stream to send
 * @param string the buffer that contain the string
 * @param serStream the buffer that will contain the serialized stream
 * @param shouldWait should ask the client to press enter to continue
 * @param format the formated string
 */
void sendString(int communicationID, stream_t *stream, char *string, char *serStream, bool shouldWait, const char *format, ...)
{
    init_stream(stream, shouldWait ? STRING_AND_WAIT : STRING);

    va_list argptr;
    va_start(argptr, format);
    vsprintf(string, format, argptr);
    va_end(argptr);

    set_content(stream, string);

    size_t serStreamSize = serialize_stream(stream, serStream);
    send(communicationID, serStream, serStreamSize, 0); // send buffer to client

    if (shouldWait)
        recv(communicationID, serStream, STREAM_SIZE, 0);
}

/**
 * Prompt an user
 * @param communicationID the id of the communication
 * @param s the stream to send
 * @param serStream the buffer that will contain the serialized stream
 */
void promptUser(int communicationID, stream_t *s, char *serStream)
{
    init_stream(s, PROMPT);
    size_t serStreamSize = serialize_stream(s, serStream);
    send(communicationID, serStream, serStreamSize, 0); // send buffer to client
}
