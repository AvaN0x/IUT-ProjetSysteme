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
int16_t getIndexWhenCode(concertConfigStruct *, char *);
void reserveTicket(bool *, int, concertConfigStruct *, stream_t *, char *, char *);
void cancelTicket(bool *, int, concertConfigStruct *, stream_t *, char *, char *);

#endif
