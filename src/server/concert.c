#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "concert.h"
#include "../common/seats.h"

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
