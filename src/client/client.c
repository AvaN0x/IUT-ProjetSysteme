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

    char serStream[STREAM_SIZE]; // serialized stream
    size_t serStreamSize;
    char string[BUFFER_SIZE];
    int8_t promptedInt;
    bool *seats;
    bool loop = 1;

    do
    {

        printf("\n*------- CONCERT -------*\n0/ Quitter\n1/ Réserver un ticket\n2/ Annuler un ticket\nChoix : ");
        promptedInt = promptInt(string, BUFFER_SIZE, 0, 2);
        switch (promptedInt)
        {
        case 0:
            loop = 0;
            init_stream(&stream, END_CONNECTION);
            serStreamSize = serialize_stream(&stream, serStream);
            send(fdSocket, serStream, serStreamSize, 0); // send buffer to server
            break;
        case 1:
            reserveTicket(fdSocket, &stream, string, serStream);
            break;
        case 2:
            cancelTicket(fdSocket, &stream, string, serStream);
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
    size_t serStreamSize;
    int bufSize;
    bool loop = 1;
    int promptedInt;

    do
    {
        init_stream(stream, ASK_SEATS);
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            loop = 0;
            continue;
        }
        unserialize_stream(serStream, stream);

        dispSeats((bool *)stream->content);

        promptedInt = promptInt(string, BUFFER_SIZE, 0, SEAT_AMOUNT);
        if (promptedInt == 0)
        {
            loop = 0;
            continue;
        }

        init_stream(stream, IS_SEAT_STILL_AVAILABLE);
        set_content(stream, &promptedInt);
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            loop = 0;
            continue;
        }
        unserialize_stream(serStream, stream);

        if (*(int8_t *)stream->content == 1)
        {
            printf("\n=> Ce siège est déjà réservé, veuillez en séléctionner un autre.\n(Appuyez sur entrer pour continuer)\n");
            promptString(string, BUFFER_SIZE);
            continue;
        }

        char firstname[NAME_SIZE + 1];
        char lastname[NAME_SIZE + 1];
        printf("\nVeuillez saisir votre prénom : ");
        promptString(firstname, NAME_SIZE);
        printf("Veuillez saisir votre nom : ");
        promptString(lastname, NAME_SIZE);

        init_stream(stream, SET_SEAT_FIRSTNAME);
        set_content(stream, firstname);
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        init_stream(stream, SET_SEAT_LASTNAME);
        set_content(stream, lastname);
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        init_stream(stream, RESERVE_SEAT);
        set_content(stream, &promptedInt); // promptedInt still contain the value of the wanted seat
        serStreamSize = serialize_stream(stream, serStream);
        send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

        bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            loop = 0;
            continue;
        }
        unserialize_stream(serStream, stream);

        if (stream->type == ERROR)
        {
            printf("\n=> Quelqu'un vient de réserver ce siège plus rapidement que vous, veuillez en séléctionner un autre.\n(Appuyez sur entrer pour continuer)\n");
            promptString(string, BUFFER_SIZE);
            continue;
        }
        else if (stream->type == SEND_SEAT_CODE)
        {
            printf("\n%s %s,\nvoici votre code (à conserver) : %s\n", lastname, firstname, (char *)stream->content);
            printf("(Appuyez sur entrer pour continuer)\n");
            promptString(string, BUFFER_SIZE);
            loop = 0;
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
    size_t serStreamSize;
    int bufSize;
    int promptedInt;

    char lastname[NAME_SIZE + 1];
    char code[CODE_LENGTH + 1];
    printf("\nVeuillez saisir votre nom : ");
    promptString(lastname, NAME_SIZE + 1);
    printf("Veuillez saisir votre numéro de dossier : ");
    promptString(code, CODE_LENGTH + 1);

    init_stream(stream, SET_SEAT_LASTNAME);
    set_content(stream, lastname);
    serStreamSize = serialize_stream(stream, serStream);
    send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

    init_stream(stream, SET_SEAT_CODE);
    set_content(stream, code);
    serStreamSize = serialize_stream(stream, serStream);
    send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

    if (!promptConfirmation("Voulez vous vraiment supprimer votre réservation? (O ou N)\n"))
        return;

    init_stream(stream, CANCEL_SEAT);
    set_content(stream, &promptedInt); // promptedInt still contain the value of the wanted seat
    serStreamSize = serialize_stream(stream, serStream);
    send(fdSocket, serStream, serStreamSize, 0); // send buffer to server

    bufSize = recv(fdSocket, serStream, STREAM_SIZE, 0);
    if (bufSize < 1)
        return;

    unserialize_stream(serStream, stream);

    if (stream->type == ERROR)
    {
        printf("\n=> Les informations saisient ne correspondant à aucunes places.\n(Appuyez sur entrer pour continuer)\n");
        promptString(string, BUFFER_SIZE);
        return;
    }
    else if (stream->type == SEAT_CANCELED)
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

    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);

    scanf("%c", &answer);

    clearBuffer();
    if (answer == 'Y' || answer == 'y' || answer == 'O' || answer == 'o')
        return 1;
    else
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
    int ret;
    char *endPtr;
    while (1)
    {
        promptString(buffer, length);

        ret = (int)strtol(buffer, &endPtr, 10); // convert to int
        if (buffer == endPtr || ret < min || ret > max)
            printf("Veuillez rentrer un entier entre %d et %d : ", min, max);
        else
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