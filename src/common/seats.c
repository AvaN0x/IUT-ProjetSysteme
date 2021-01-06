#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "consoleStyle.h"
#include "seats.h"

/**
 * Initialize a seatStruct instance
 * @return the seat
 */
seatStruct initSeat()
{
    seatStruct seat = *(seatStruct *)malloc(sizeof(seatStruct)); // allocate the size of a seatStruct
    seat.isOccupied = false;                                     // set the seat as not occupied

    return seat;
}

/**
 * Display a table of seats
 * @param seats the array (bool[]) of seats
 */
void dispSeats(bool *seats, bool cancel)
{
    printf("  0/ Quitter\n");
    // find the squareroot of SEAT_AMOUNT to know how many columns we need to display
    int squareroot = (int)sqrt(SEAT_AMOUNT);
    for (int i = 0; i < SEAT_AMOUNT; i++)
    {
        if ((seats[i] == 0 && !cancel) || (seats[i] != 0 && cancel)) // check if the seat is available
            printf(FONT_GREEN "%3d ", i + 1);                        // display the available seat number as a 3 digit numbers
        else
            printf(FONT_RED "  X "); // display a cross a reserved seat

        if ((i + 1) % squareroot == 0 || i == (SEAT_AMOUNT - 1)) // if we are at the end of a line, or at the end of the array
            printf(FONT_DEFAULT "\n");                           // then we print a line break
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
        code[i] = randomInt(48, 57); // 48 is '0' and 57 is '9'
    }
    code[CODE_LENGTH] = '\0'; // set the end of the string
}

/**
 * Generate a random int inside of an interval
 * @param min interval min for the int
 * @param max interval max for the int
 */
int randomInt(int min, int max)
{
    return rand() % (max - min + 1) + min; // return a number between min and max
}