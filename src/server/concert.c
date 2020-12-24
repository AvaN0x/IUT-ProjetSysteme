#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "concert.h"
#include "../common/seats.h"

concertConfigStruct initConcert()
{
    concertConfigStruct concertConfig = *(concertConfigStruct *)malloc(sizeof(concertConfigStruct));
    for (int i = 0; i < SEAT_AMOUNT; i++)
    {
        concertConfig.seats[i] = initSeat();
    }

    return concertConfig;
}