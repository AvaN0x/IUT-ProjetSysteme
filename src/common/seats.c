#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "seats.h"

seatStruct initSeat()
{
    seatStruct seat = *(seatStruct *)malloc(sizeof(seatStruct));
    seat.isOccupied = false;

    return seat;
}

bool seatIsOccupied(seatStruct *seat)
{
    return seat->isOccupied;
}