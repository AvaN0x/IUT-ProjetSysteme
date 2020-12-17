#ifndef DEF_UTILS
#define DEF_UTILS

#include <stdint.h>
#include <stdlib.h>

#define PORT 6000
#define BUFFER_SIZE 256
#define STREAM_SIZE BUFFER_SIZE + 128
typedef struct
{
    void *content;
    uint8_t type;
} stream;

enum
{
    WRITE,
    WRITE_AND_PROMPT,
    PROMPT,
    END_CONNECTION
};

void init_stream(stream *, uint8_t);
void set_content(stream *, void *);
void destroy_stream(stream *);

size_t serialize_stream(stream *, void *);
void unserialize_stream(void *, stream *);

#endif
