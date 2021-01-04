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
    concertConfigStruct concertConfig = *(concertConfigStruct *)malloc(sizeof(concertConfigStruct)); // allocate the size of a concertConfigStruct
    for (int i = 0; i < SEAT_AMOUNT; i++)                                                            // init every seat of the array
        concertConfig.seats[i] = initSeat();

    return concertConfig;
}

/**
 * Create an array (bool[]) of occupied seats from the concert configuration
 * @param config the concert configuration
 * @return the seats array
 */
bool *getSeatsStatus(concertConfigStruct *config)
{
    bool *seats = malloc(SEAT_AMOUNT * sizeof(bool)); // allocate memory for an array of bool for each seats

    sem_wait(&semaphore); // block the access to the concertConfig
    for (int i = 0; i < SEAT_AMOUNT; i++)
    {
        seats[i] = config->seats[i].isOccupied; // set the boolean value of the new array to the value of the seat
    }
    sem_post(&semaphore); // free the access to the concertConfig

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
    sem_wait(&semaphore); // block the access to the concertConfig
    for (int i = 0; i < SEAT_AMOUNT; i++)
        if (config->seats[i].isOccupied == 1 && strcmp(config->seats[i].code, code) == 0) // if the seat is occupied and the code match
        {
            sem_post(&semaphore); // free the access to the concertConfig
            return i;             // we return the number of the seat
        }
    sem_post(&semaphore); // free the access to the concertConfig

    return -1; // default return
}
