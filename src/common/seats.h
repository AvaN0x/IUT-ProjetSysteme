#ifndef DEF_SEATS
#define DEF_SEATS

#include <stdbool.h>

#define SEAT_AMOUNT 100
#define NAME_SIZE 100
#define CODE_LENGTH 10

typedef struct
{
    bool isOccupied;
    char firstname[NAME_SIZE];
    char lastname[NAME_SIZE];
    char code[CODE_LENGTH]; // todo compare string with this to check if the user prompted the good code
} seatStruct;

seatStruct initSeat();
bool seatIsOccupied(seatStruct *);
void dispSeats(bool *);

#endif
