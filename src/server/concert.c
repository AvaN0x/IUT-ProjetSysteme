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
        sendString(communicationID, stream, string, serStream, "\n*------- SALLE DE CONCERT -------*\nChaque X correspond à une place réservée. Veuillez entrer le numéro d'une place ou 0.\n0/ Quitter\n");

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
        }
        else
        {
            unserialize_stream(serStream, stream);

            if (stream->type == INT)
            {
                int receivedInt = *(int8_t *)stream->content;
                if (receivedInt == 0)
                    loop = 0;
                else
                {
                    if (concertConfig->seats[receivedInt - 1].isOccupied == 1)
                        // todo maybe use STRING_AND_PROMPT to make the user confirm that he have seen the message
                        sendString(communicationID, stream, string, serStream, "=> Ce siège est déjà réservé, veuillez en séléctionner un autre.\n");
                    else
                    {
                        printf("%d | Seat reserved  : %d\n", communicationID, receivedInt);
                        concertConfig->seats[receivedInt - 1].isOccupied = 1; //? just a test
                    }
                }
            }
        }
    }
}