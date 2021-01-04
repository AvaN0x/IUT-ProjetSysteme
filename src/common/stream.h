#ifndef DEF_UTILS
#define DEF_UTILS

#include <stdint.h>
#include <stdlib.h>

#define PORT 6000
#define BUFFER_SIZE 1024
#define STREAM_SIZE BUFFER_SIZE + 128

typedef struct
{
    void *content;
    uint8_t type;
} stream_t;

enum
{
    INT,                 //? content : int
    RESERVE_SEAT,        //? content : int = seat number
    IS_SEAT_AVAILABLE,   //? content : int = seat number
    SEAT_CANCELED,       //? content : int = seat number
    STRING,              //? content : string
    SET_SEAT_LASTNAME,   //? content : string = lastname
    SET_SEAT_FIRSTNAME,  //? content : string = firstname
    SET_SEAT_CODE,       //? content : string = code
    SEND_SEAT_CODE,      //? content : string = code
    PROMPT,              //? content : int (size of the string prompted)
    PROMPT_INT_WITH_MAX, //? content : int
    SEND_SEATS,          //? content : seats[] = array of bool for seats
    ASK_SEATS,           //? content : NULL
    CANCEL_SEAT,         //? content : NULL
    ERROR,               //? content : NULL
    END_CONNECTION       //? content : NULL
};

stream_t create_stream();
void init_stream(stream_t *, uint8_t);
void set_content(stream_t *, void *);
void destroy_stream(stream_t *);

size_t serialize_stream(stream_t *, void *);
void unserialize_stream(void *, stream_t *);

#endif
