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

        if ((i + 1) % squareroot == 0 || i == (SEAT_AMOUNT - 1))
            printf("\n");
    }
    printf("Choix : ");
}

/**
 * Generate a code for a buffer
 * @param code the buffer that will contain the code string
 */
void generateCode(char *code)
{
    for (int i = 0; i < CODE_LENGTH; i++)
    {
        code[i] = randomInt(48, 57);
    }
    code[CODE_LENGTH] = '\0';
}

/**
 * Generate a random int inside of an interval
 * @param min interval min for the int
 * @param max interval max for the int
 */
int randomInt(int min, int max)
{
    return rand() % (max - min + 1) + min;
}