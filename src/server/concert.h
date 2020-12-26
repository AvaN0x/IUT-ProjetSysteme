#ifndef DEF_CONCERT
#define DEF_CONCERT

#include "../common/seats.h"
#include "../common/stream.h"

typedef struct
{
    seatStruct seats[SEAT_AMOUNT];
} concertConfigStruct;

concertConfigStruct initConcert();
bool *getSeatsStatus(concertConfigStruct *);
void reserveTicket(bool *, int, concertConfigStruct *, stream_t *, char *, char *);

#endif
