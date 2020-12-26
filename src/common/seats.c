#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "seats.h"

/**
 * Initialize a seatStruct instance
 * @return the seat
 */
seatStruct initSeat()
{
    seatStruct seat = *(seatStruct *)malloc(sizeof(seatStruct));
    seat.isOccupied = false;

    return seat;
}

/**
 * Check whether a seat is occupied or not
 * @param seat the seat
 * @param buffer the buffer to fill in
 * @return true if the seat is occupied, else false
 */
bool seatIsOccupied(seatStruct *seat)
{
    return seat->isOccupied;
}

/**
 * Display a table of seats
 * @param seats the array (bool[]) of seats
 */
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
    printf("Choix : ");
}