#ifndef DEF_CONCERT
#define DEF_CONCERT

#include "../common/seats.h"

typedef struct
{
    seatStruct seats[SEAT_AMOUNT];
} concertConfigStruct;

concertConfigStruct initConcert();
bool *getSeatsStatus(concertConfigStruct *);

#endif
