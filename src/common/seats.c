#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

void dispSeats(bool *seats)
{
    int squareroot = (int)sqrt(SEAT_AMOUNT);
    for (int i = 0; i < SEAT_AMOUNT; i++)
    {
        if (seats[i] == 0)
            printf("%3d ", i + 1);
        else
            printf("  X ");

        if ((i + 1) % squareroot == 0)
            printf("\n");
    }
}