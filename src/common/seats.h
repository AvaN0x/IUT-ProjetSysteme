#ifndef DEF_SEATS
#define DEF_SEATS

#include <stdbool.h>

#define SEAT_AMOUNT 100
#define NAME_SIZE 100
#define CODE_LENGTH 10

typedef struct
{
    bool isOccupied;
    char firstname[NAME_SIZE + 1];
    char lastname[NAME_SIZE + 1];
    char code[CODE_LENGTH + 1];
} seatStruct;

seatStruct initSeat();
bool seatIsOccupied(seatStruct *);
void dispSeats(bool *, bool);
void generateCode(char *);
int randomInt(int, int);

#endif
