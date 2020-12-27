#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "concert.h"
#include "server.h"
#include "../common/seats.h"
#include "../common/stream.h"

/**
 * Initialize a concertConfigStruct instance
 * @return the concert configuration
 */
concertConfigStruct initConcert()
{
    concertConfigStruct concertConfig = *(concertConfigStruct *)malloc(sizeof(concertConfigStruct));
    for (int i = 0; i < SEAT_AMOUNT; i++)
    {
        concertConfig.seats[i] = initSeat();
    }

    return concertConfig;
}

/**
 * Create an array (bool[]) of occupied seats from the concert configuration
 * @param config the concert configuration
 * @return the seats array
 */
bool *getSeatsStatus(concertConfigStruct *config)
{
    bool *seats = malloc(SEAT_AMOUNT * sizeof(bool));
    for (int i = 0; i < SEAT_AMOUNT; i++)
    {
        seats[i] = config->seats[i].isOccupied;
    }

    return seats;
}

/**
 * Function that manage when the user want to reserve a ticket
 * @param parentLoop the main loop state to stop connection if user leave
 * @param communicationID the id of the communication
 * @param stream the stream to send
 * @param string the buffer that contain the string
 * @param serStream the buffer that will contain the serialized stream
 */
void reserveTicket(bool *parentLoop, int communicationID, concertConfigStruct *concertConfig, stream_t *stream, char *string, char *serStream)
{
    size_t serStreamSize;
    bool loop = 1;
    while (loop)
    {
        sendString(communicationID, stream, string, serStream, 0, "\n*------- SALLE DE CONCERT -------*\nChaque X correspond à une place réservée. Veuillez entrer le numéro d'une place ou 0.\n0/ Quitter\n");

        init_stream(stream, PROMPT_WANTED_SEAT);
        bool *seats = getSeatsStatus(concertConfig);
        set_content(stream, seats);
        free(seats);
        serialize_stream(stream, serStream);
        send(communicationID, serStream, STREAM_SIZE, 0); // send buffer to client

        int bufSize = recv(communicationID, serStream, STREAM_SIZE, 0);
        if (bufSize < 1)
        {
            *parentLoop = 0;
            loop = 0;
            continue;
        }
        unserialize_stream(serStream, stream);

        if (stream->type == INT)
        {
            int receivedInt = *(int8_t *)stream->content;
            if (receivedInt == 0)
            {
                loop = 0;
                continue;
            }

            if (concertConfig->seats[receivedInt - 1].isOccupied == 1)
                sendString(communicationID, stream, string, serStream, 1, "\n=> Ce siège est déjà réservé, veuillez en séléctionner un autre. \n(Appuyez sur une touche pour passer à la suite)\n");
            else
            {
                printf("%d | Seat reserved  : %d\n", communicationID, receivedInt);
                concertConfig->seats[receivedInt - 1].isOccupied = 1; //? just a test

                // init_stream(&stream, STRING_AND_PROMPT);
                // snprintf(string, BUFFER_SIZE, "Send me something please (%d) : ", i);
                // set_content(&stream, string);
                // serialize_stream(&stream, serStream);

                // send(communicationID, serStream, sizeof(serStream), 0); // send buffer to client
                // send(communicationID, serStream, STREAM_SIZE, 0);       // send buffer to client

                // int bufSize = recv(communicationID, serStream, STREAM_SIZE, 0);
                // if (bufSize > 0)
                // {
                //     unserialize_stream(serStream, &stream);
                //     printf("%d | Received : %s\n", communicationID, (char *)stream.content);
                // }
            }
        }
    }
}