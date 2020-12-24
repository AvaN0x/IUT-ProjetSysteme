#ifndef DEF_CONCERT
#define DEF_CONCERT

#include "../common/seats.h"

#define SEAT_AMOUNT 100

typedef struct
{
    seatStruct seats[SEAT_AMOUNT];
} concertConfigStruct;

concertConfigStruct initConcert();

#endif
