#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdarg.h> // for infinite parameters

#include "../common/seats.h"
#include "../common/stream.h"
#include "client.h"

#define ADDRESS "127.0.0.1"

/**
 * Main function that connect to the server and start the conversation
 * @return exit status (EXIT_FAILURE || EXIT_SUCCESS)
 */
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

    // call the function that manage the connection
    ConnectedToServer(fdSocket);

    close(fdSocket);
    return EXIT_SUCCESS;
}

/**
 * Function that manage the user connection
 * @param fdSocket the socket of the communication
 */
void ConnectedToServer(int fdSocket)
{
    stream_t stream = create_stream(); // received stream

    size_t serStreamSize;        // variable that will contain the size of setStream
    char serStream[STREAM_SIZE]; // serialized stream
    char string[BUFFER_SIZE];    // buffer for strings
    int8_t promptedInt;          // integer that is used when an integer is prompted to the user
    bool *seats;                 // array that will contain all seat values
    bool loop = 1;               // loop that let the user do something until he leave

    do
    {
        printf("\n*------- CONCERT -------*\n0/ Quitter\n1/ Réserver un ticket\n2/ Annuler un ticket\nChoix : ");
        promptedInt = promptInt(string, BUFFER_SIZE, 0, 2); // prompt the user an int (choice)

        switch (promptedInt)
        {
        case 0:
            loop = 0;                             // set the loop at false, will disconnect the user
            init_stream(&stream, END_CONNECTION); // tell the server that this user left
            serStreamSize = serialize_stream(&stream, serStream);
            send(fdSocket, serStream, serStreamSize, 0); // send buffer to server
            break;
        case 1:
            reserveTicket(fdSocket, &stream, string, serStream); // enter in the function that allow the user to reserve a ticket
            break;
        case 2:
            cancelTicket(fdSocket, &stream, string, serStream); // enter in the function that allow the user to cancel a ticket
            break;
        }
    } while (loop == 1);

    init_stream(&stream, STRING);
    set_content(&stream, string);
    serStreamSize = serialize_stream(&stream, serStream);

    send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

    destroy_stream(&stream);
}

/**
 * Function that manage when the user want to reserve a ticket
 * @param communicationID the id of the communication
 * @param stream the stream to send
 * @param string the buffer that contain the string
 * @param serStream the buffer that will contain the serialized stream
 */
void reserveTicket(int fdSocket, stream_t *stream, char *string, char *serStream)
{
    size_t serStreamSize; // variable that will contain the size of setStream
    int bufSize;          // contain the return of recv()
    bool loop = 1;        // loop to let the user select a seat until he/she find one that is not already reserved
    int weantedSeat;

    do
    {
        init_stream(stream, ASK_SEATS); // ask the server to send seats
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            loop = 0; // set the loop at false, this will make the user go back to the lobby
            continue; // go to the next iteration of this while loop
        }
        unserialize_stream(serStream, stream);

        dispSeats((bool *)stream->content); // display a table of all seats, it contain the header and the footer

        weantedSeat = promptInt(string, BUFFER_SIZE, 0, SEAT_AMOUNT);
        if (weantedSeat == 0)
        {
            loop = 0; // set the loop at false, this will make the user go back to the lobby
            continue; // go to the next iteration of this while loop
        }

        init_stream(stream, IS_SEAT_AVAILABLE); // check if the seat is available
        set_content(stream, &weantedSeat);
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            loop = 0; // set the loop at false, this will make the user go back to the lobby
            continue; // go to the next iteration of this while loop
        }
        unserialize_stream(serStream, stream);

        if (*(int8_t *)stream->content == 1) // if true, the seat is now available
        {
            printf("\n=> Ce siège est déjà réservé, veuillez en séléctionner un autre.\n(Appuyez sur entrer pour continuer)\n");
            promptString(string, BUFFER_SIZE);
            continue; // go to the next iteration of this while loop
        }

        char firstname[NAME_SIZE + 1];
        char lastname[NAME_SIZE + 1];

        printf("\nVeuillez saisir votre prénom : ");
        promptString(firstname, NAME_SIZE);
        printf("Veuillez saisir votre nom : ");
        promptString(lastname, NAME_SIZE);

        init_stream(stream, SET_SEAT_FIRSTNAME); // send firstname to server
        set_content(stream, firstname);
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        init_stream(stream, SET_SEAT_LASTNAME); // send lastname to server
        set_content(stream, lastname);
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        init_stream(stream, RESERVE_SEAT); // ask the server to reserve the seat
        set_content(stream, &weantedSeat); // weantedSeat still contain the value of the wanted seat
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            loop = 0; // set the loop at false, this will make the user go back to the lobby
            continue; // go to the next iteration of this while loop
        }
        unserialize_stream(serStream, stream);

        // check the answer of the server
        if (stream->type == ERROR) // ERROR if the seat got reserved from someone else while the user where sending datas
        {
            printf("\n=> Quelqu'un vient de réserver ce siège plus rapidement que vous, veuillez en séléctionner un autre.\n(Appuyez sur entrer pour continuer)\n");
            promptString(string, BUFFER_SIZE);
            continue; // go to the next iteration of this while loop
        }
        else if (stream->type == SEND_SEAT_CODE) // the seat is still available
        {
            printf("\n%s %s,\nvoici votre code (à conserver) : %s\n", lastname, firstname, (char *)stream->content);
            printf("(Appuyez sur entrer pour continuer)\n");
            promptString(string, BUFFER_SIZE);
            loop = 0; // set the loop at false, this will make the user go back to the lobby
        }
    } while (loop == 1);
}

/**
 * Function that manage when the user want to reserve a ticket
 * @param communicationID the id of the communication
 * @param stream the stream to send
 * @param string the buffer that contain the string
 * @param serStream the buffer that will contain the serialized stream
 */
void cancelTicket(int fdSocket, stream_t *stream, char *string, char *serStream)
{
    size_t serStreamSize; // variable that will contain the size of setStream
    int bufSize;          // contain the return of recv()
    int promptedInt;

    char lastname[NAME_SIZE + 1];
    char code[CODE_LENGTH + 1];
    printf("\nVeuillez saisir votre nom : ");
    promptString(lastname, NAME_SIZE + 1);
    printf("Veuillez saisir votre numéro de dossier : ");
    promptString(code, CODE_LENGTH + 1);

    init_stream(stream, SET_SEAT_LASTNAME); // send leastname to server
    set_content(stream, lastname);
    serStreamSize = serialize_stream(stream, serStream);
    send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

    init_stream(stream, SET_SEAT_CODE); // send code to server
    set_content(stream, code);
    serStreamSize = serialize_stream(stream, serStream);
    send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

    if (!promptConfirmation("Voulez vous vraiment supprimer votre réservation? (O ou N)\n"))
        return;

    init_stream(stream, CANCEL_SEAT);  // ask the server to cancel the seat
    set_content(stream, &promptedInt); // promptedInt still contain the value of the wanted seat
    serStreamSize = serialize_stream(stream, serStream);
    send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

    bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
    if (bufSize < 1)
        return;

    unserialize_stream(serStream, stream);

    // check the answer of the server
    if (stream->type == ERROR) // ERROR if values does not match with any seat
    {
        printf("\n=> Les informations saisient ne correspondant à aucunes places.\n(Appuyez sur entrer pour continuer)\n");
        promptString(string, BUFFER_SIZE);
        return;
    }
    else if (stream->type == SEAT_CANCELED) // the seat exist and everything match
    {
        printf("\n=> Votre réservation du siège numéro %d est annulée.\n", *(int *)stream->content + 1);
        printf("(Appuyez sur entrer pour continuer)\n");
        promptString(string, BUFFER_SIZE);
    }
}

/**
 * Allow to enter text, and if the text entered is too long, then it will clear the buffer
 * @param buffer the buffer to fill
 * @param length the max length of the string
 * @return exit status (EXIT_FAILURE || EXIT_SUCCESS)
 */
int promptString(char *buffer, int length)
{
    if (fgets(buffer, length, stdin) != NULL)
    {
        char *lastCharPos = strchr(buffer, '\n');
        if (lastCharPos != NULL)
            *lastCharPos = '\0';
        else
            clearBuffer();
        return EXIT_SUCCESS;
    }
    else
    {
        clearBuffer();
        return EXIT_FAILURE;
    }
}

/**
 * Ask user to confirm a choice
 * @param format the formated string to display
 * @return bool
 */
int promptConfirmation(const char *format, ...)
{
    char answer;

    // this allow us to use infinite paramaters and access them
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr); // print a formated string
    va_end(argptr);

    scanf("%c", &answer); // prompt a char to the client

    clearBuffer();                                                        // clear the buffer to get rid of unwanted char
    if (answer == 'Y' || answer == 'y' || answer == 'O' || answer == 'o') // check if the char match a confirmation
        return 1;
    else // else we just don't confirm
        return 0;
}

/**
 * Allow to enter an int
 * @param buffer the buffer to fill
 * @param length the max length of the string
 * @param min min value
 * @param max max value
 * @return the prompted int
 */
int promptInt(char *buffer, int length, int min, int max)
{
    int ret;      // the variable that will contain the prompted int
    char *endPtr; // this will allow us to check if the user specified a number or not
    while (1)     // infinite loop until there is a return
    {
        promptString(buffer, length); // prompt a string to the buffer

        ret = (int)strtol(buffer, &endPtr, 10); // convert to int
        // if strtol did not found a matching int, or if the int is now in the interval, then we ask the client to prompt a new int
        if (buffer == endPtr || ret < min || ret > max)
            printf("Veuillez rentrer un entier entre %d et %d : ", min, max);
        else // else we return the good int
            return ret;
    }
}

/**
 * Function that clear the buffer of its content
 */
void clearBuffer()
{
    int c = 0;
    while (c != '\n' && c != EOF)
        c = getchar();
}