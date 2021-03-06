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

#include "semaphore.h"
#include "../common/consoleStyle.h"
#include "../common/stream.h"
#include "server.h"

sem_t semaphore;

/**
 * Main function that create the socket, create the concert, and manage client connections
 * @return exit status (EXIT_FAILURE || EXIT_SUCCESS)
 */
int main()
{
    int serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        printf(FONT_RED "Incorrect socket\n" FONT_DEFAULT);
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
        printf(FONT_RED "Bind error, PORT may already be in use.\n" FONT_DEFAULT);
        exit(EXIT_FAILURE);
    }
    if (listen(serverSocket, 5) == -1)
    {
        printf(FONT_RED "Listen error.\n" FONT_DEFAULT);
        exit(EXIT_FAILURE);
    }

    //? init the concert structure that will contain every seats
    concertConfigStruct concertConfig = initConcert();
    sem_init(&semaphore, PTHREAD_PROCESS_SHARED, 1);

    //? set the randomness of the program
    srand((unsigned int)time(NULL));

    // Main loop
    int sockaddr_in_size = sizeof(struct sockaddr_in);
    while (1)
    {
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
            printf(FONT_RED "Connection acceptation error\n" FONT_DEFAULT);
        }
    }
    close(serverSocket);
    sem_destroy(&semaphore);

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

    // function that will manage the client
    clientConnected(connection.communicationID, connection.concertConfig);

    close(connection.communicationID);
    pthread_exit(NULL);
}

/**
 * Call function that manage the client connection
 * @param communicationID the id of the communication
 * @param concertConfig address of the concert configuration
 * @return true if the seat is occupied, else false
 */
void clientConnected(int communicationID, concertConfigStruct *concertConfig)
{
    stream_t stream = create_stream(); // stream that is used with this client
    char serStream[STREAM_SIZE];       // serialized stream
    size_t serStreamSize;              // buffer that contain the serialized stream
    bool loop = 1;
    int clientInt;
    char firstname[NAME_SIZE + 1];   // string for the firstname
    char lastname[NAME_SIZE + 1];    // string for the lastname
    char code[CODE_LENGTH + 1];      // string for the code
    char adminCode[CODE_LENGTH + 1]; // string for the admin code

    while (loop)
    {
        // wait to receive a message from the client
        int bufSize = recv(communicationID, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            loop = 0;
            continue;
        }

        unserialize_stream(serStream, &stream);

        // check the type of the stream
        switch (stream.type)
        {
        case END_CONNECTION:
            // stop the loop that manage the client
            loop = 0;
            break;
        case ASK_SEATS:
            // the client asked for every seats status
            init_stream(&stream, SEND_SEATS);            // we send the seats array to the user
            bool *seats = getSeatsStatus(concertConfig); // create the seats array
            set_content(&stream, seats);
            free(seats); // free the seats array

            serStreamSize = serialize_stream(&stream, serStream);
            send(communicationID, serStream, serStreamSize, 0); // send buffer to client
            break;

        case IS_SEAT_AVAILABLE:
            // the client want to know if a seat is available
            clientInt = *(int8_t *)stream.content;
            init_stream(&stream, INT);
            sem_wait(&semaphore); // block the access to the concertConfig
            set_content(&stream, (int8_t *)&(concertConfig->seats[clientInt - 1].isOccupied));
            sem_post(&semaphore); // free the access to the concertConfig
            serStreamSize = serialize_stream(&stream, serStream);
            send(communicationID, serStream, serStreamSize, 0); // send buffer to server
            break;

        case SET_SEAT_LASTNAME:
            // the client want to set his/her lastname
            memcpy(lastname, (char *)stream.content, NAME_SIZE);
            break;

        case SET_SEAT_FIRSTNAME:
            // the client want to set his/her firstname
            memcpy(firstname, (char *)stream.content, NAME_SIZE);
            break;

        case SET_SEAT_CODE:
            // the client want to set his/her code
            memcpy(code, (char *)stream.content, CODE_LENGTH + 1);
            break;

        case RESERVE_SEAT:
            // the client want to reserve a seat
            if (lastname[0] != '\0' && firstname[0] != '\0')
            {
                clientInt = *(int8_t *)stream.content; // get the seat that the user have focus on

                // generate a code until we find one that doesn't already exist
                do
                {
                    generateCode(code);
                } while (getIndexWhenCode(concertConfig, code) != -1);

                sem_wait(&semaphore); // block the access to the concertConfig
                //? check if the seat is still available
                if (concertConfig->seats[clientInt - 1].isOccupied == 1) // if the seat is not available, then we send an error to the client
                {
                    sem_post(&semaphore); // free the access to the concertConfig
                    init_stream(&stream, ERROR);
                    serStreamSize = serialize_stream(&stream, serStream);
                    send(communicationID, serStream, serStreamSize, 0); // send buffer to client
                    continue;
                }
                //? if the seat is still available, we set all new values
                printf("%d | " FONT_GREEN "Seat %d reserved" FONT_DEFAULT " by : %s %s (code : %s)\n", communicationID, clientInt, firstname, lastname, code);

                concertConfig->seats[clientInt - 1].isOccupied = 1; // we set the seat as not available

                // we copy values that identify the seat
                memcpy(concertConfig->seats[clientInt - 1].firstname, firstname, strlen(firstname) + 1);
                concertConfig->seats[clientInt - 1].firstname[strlen(firstname)] = '\0';
                memcpy(concertConfig->seats[clientInt - 1].lastname, lastname, strlen(lastname) + 1);
                concertConfig->seats[clientInt - 1].lastname[strlen(lastname)] = '\0';
                memcpy(concertConfig->seats[clientInt - 1].code, code, CODE_LENGTH + 1);
                concertConfig->seats[clientInt - 1].code[strlen(code)] = '\0';

                sem_post(&semaphore); // free the access to the concertConfig

                init_stream(&stream, SEND_SEAT_CODE); // we send the client the code of the seat
                set_content(&stream, code);

                serStreamSize = serialize_stream(&stream, serStream);
                send(communicationID, serStream, serStreamSize, 0); // send buffer to client
            }
            break;
        case CANCEL_SEAT:
            // the user want to cancel a seat
            if (lastname[0] != '\0' && code[0] != '\0')
            {
                clientInt = getIndexWhenCode(concertConfig, code); // we get the index of the seat (-1 if does not exist)

                sem_wait(&semaphore);                                                                   // block the access to the concertConfig
                if (clientInt == -1 || strcmp(concertConfig->seats[clientInt].lastname, lastname) != 0) // if the seat does not exist or the lastname does not match
                {
                    // we send an error to the client
                    sem_post(&semaphore); // free the access to the concertConfig
                    init_stream(&stream, ERROR);
                    serStreamSize = serialize_stream(&stream, serStream);
                    send(communicationID, serStream, serStreamSize, 0); // send buffer to client
                }
                else
                {
                    // reinitialize the seat values
                    concertConfig->seats[clientInt].isOccupied = 0;
                    concertConfig->seats[clientInt].firstname[0] = '\0';
                    concertConfig->seats[clientInt].lastname[0] = '\0';
                    concertConfig->seats[clientInt].code[0] = '\0';
                    sem_post(&semaphore); // free the access to the concertConfig

                    printf("%d | " FONT_RED "Seat %d reservation canceled" FONT_DEFAULT " (name : %s, code : %s)\n", communicationID, clientInt + 1, lastname, code);

                    init_stream(&stream, SEAT_CANCELED); // we tell the client that the seat reservation is now canceled
                    set_content(&stream, &clientInt);
                    serStreamSize = serialize_stream(&stream, serStream);
                    send(communicationID, serStream, serStreamSize, 0); // send buffer to client
                }
            }
            break;

        case ADMIN_ASK_CODE:
            // the client want a code to be an admin
            generateCode(adminCode);
            printf("%d | Admin code to enter : " FONT_MAGENTA "%s\n" FONT_DEFAULT, communicationID, adminCode);
            init_stream(&stream, SUCCESS);
            serStreamSize = serialize_stream(&stream, serStream);
            send(communicationID, serStream, serStreamSize, 0); // send buffer to client
            break;

        case ADMIN_CHECK_CODE:
            // the client prompted a code, we check if it is the good one
            if (strcmp(adminCode, (char *)stream.content) == 0)
                init_stream(&stream, SUCCESS);
            else
                init_stream(&stream, ERROR);
            serStreamSize = serialize_stream(&stream, serStream);
            send(communicationID, serStream, serStreamSize, 0); // send buffer to client
            break;

        case ADMIN_PRINT_ALL_OCCUPIED_SEAT:
            // the client want to know the state of all seats
            printf(FONT_MAGENTA "[ADMIN]" FONT_DEFAULT " %d | Asked informations about all occupied seats.\n", communicationID);
            sem_wait(&semaphore); // block the access to the concertConfig
            for (int i = 0; i < SEAT_AMOUNT; i++)
                if (concertConfig->seats[i].isOccupied == 1) // if the seat is occupied
                    printf(FONT_MAGENTA "[ADMIN]" FONT_DEFAULT " Seat #%d is occupied by \"%s %s\" with the following code : %s\n", i + 1, concertConfig->seats[i].firstname, concertConfig->seats[i].lastname, concertConfig->seats[i].code);
            sem_post(&semaphore); // free the access to the concertConfig
            printf(FONT_MAGENTA "[ADMIN]" FONT_DEFAULT " %d | All occupied seat have been checked.\n", communicationID);
            break;

        case ADMIN_CANCEL_SEAT:
            // the client want to cancel a specified seat
            clientInt = *(int8_t *)stream.content;
            sem_wait(&semaphore); // block the access to the concertConfig
            if (concertConfig->seats[clientInt - 1].isOccupied == 0)
            {
                sem_post(&semaphore); // free the access to the concertConfig
                printf(FONT_MAGENTA "[ADMIN]" FONT_DEFAULT " %d | The seat #%d is not occupied.\n", communicationID, clientInt + 1);
            }
            else
            {
                concertConfig->seats[clientInt - 1].isOccupied = 0;
                concertConfig->seats[clientInt - 1].firstname[0] = '\0';
                concertConfig->seats[clientInt - 1].lastname[0] = '\0';
                concertConfig->seats[clientInt - 1].code[0] = '\0';
                sem_post(&semaphore); // free the access to the concertConfig
                printf(FONT_MAGENTA "[ADMIN]" FONT_DEFAULT " %d | " FONT_RED "The seat #%d reservation is now canceled.\n" FONT_DEFAULT, communicationID, clientInt);
            }
            break;
        default:
            break;
        }
    }

    printf("%d | Client disconnected\n", communicationID);
    destroy_stream(&stream);
}
