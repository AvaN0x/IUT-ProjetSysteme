#include <string.h>
#include "stream.h"
#include "../server/concert.h"

stream_t create_stream()
{
    stream_t s;
    s.content = NULL;
    s.type = -1;

    return s;
}

void init_stream(stream_t *s, uint8_t type)
{
    if (s->content != NULL)
        free(s->content);

    s->content = NULL;
    s->type = type;
}

void set_content(stream_t *s, void *content)
{
    if (s->content != NULL)
        free(s->content);

    size_t len;
    switch (s->type)
    {
    case PROMPT:
    case END_CONNECTION:
        s->content = NULL;
        break;

    case STRING:
    case STRING_AND_PROMPT:
        len = strlen((char *)content);
        s->content = malloc(len * sizeof(char));
        memcpy(s->content, content, len);
        break;

    case PROMPT_WANTED_SEAT:
        s->content = malloc(SEAT_AMOUNT * sizeof(bool));
        memcpy(s->content, content, SEAT_AMOUNT);
        break;

    default:
        s->content = NULL;
    }
}

void destroy_stream(stream_t *s)
{
    if (s->content != NULL)
        free(s->content);
}

size_t serialize_stream(stream_t *s, void *buffer)
{
    *((uint8_t *)buffer) = s->type;
    buffer += sizeof(uint8_t);

    size_t len;
    switch (s->type)
    {
    case PROMPT:
    case END_CONNECTION:
        return sizeof(uint8_t);

    case STRING:
    case STRING_AND_PROMPT:
        len = strlen((char *)s->content);
        *((uint64_t *)buffer) = len;
        buffer += sizeof(uint64_t);
        memcpy(buffer, s->content, len);
        return sizeof(uint8_t) + sizeof(uint64_t) + len;

    case PROMPT_WANTED_SEAT:
        memcpy(buffer, s->content, SEAT_AMOUNT);
        return sizeof(uint8_t) + SEAT_AMOUNT;

    default:
        return 0;
    }
}

void unserialize_stream(void *buffer, stream_t *s)
{
    init_stream(s, *((uint8_t *)buffer));

    buffer += sizeof(uint8_t);
    size_t len;
    switch (s->type)
    {
    case STRING:
    case STRING_AND_PROMPT:
        len = *((uint64_t *)buffer);
        buffer += sizeof(uint64_t);
        s->content = malloc((len + 1) * sizeof(char));
        memcpy(s->content, buffer, len);
        ((char *)s->content)[len] = '\0';
        break;

    case PROMPT_WANTED_SEAT:
        s->content = malloc(SEAT_AMOUNT * sizeof(bool));
        memcpy(s->content, buffer, SEAT_AMOUNT);
        break;

    default:
        break;
    }
}
