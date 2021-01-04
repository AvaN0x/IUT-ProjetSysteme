#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "semaphore.h"
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

    sem_wait(&semaphore);
    for (int i = 0; i < SEAT_AMOUNT; i++)
    {
        seats[i] = config->seats[i].isOccupied;
    }
    sem_post(&semaphore);

    return seats;
}

/**
 * Return the index or -1 of the seat that have a specified code
 * @param config the concert configuration
 * @param code the code that is searched
 * @return the index
 */
int16_t getIndexWhenCode(concertConfigStruct *config, char *code)
{
    sem_wait(&semaphore);
    for (int i = 0; i < SEAT_AMOUNT; i++)
        if (config->seats[i].isOccupied == 1 && strcmp(config->seats[i].code, code) == 0)
        {
            sem_post(&semaphore);
            return i;
        }
    sem_post(&semaphore);

    return -1;
}
