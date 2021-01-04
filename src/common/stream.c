#include <string.h>
#include "stream.h"
#include "../server/concert.h"

/**
 * Create a stream, initialize it, and return it
 * @return the stream
 */
stream_t create_stream()
{
    stream_t s;
    s.content = NULL;
    s.type = -1;

    return s;
}

/**
 * Reinitialize a stream with a type, and the content set to null
 * @param s the stream to reset
 * @param type the new type
 */
void init_stream(stream_t *s, uint8_t type)
{
    if (s->content != NULL)
        free(s->content);

    s->content = NULL;
    s->type = type;
}

/**
 * Se the content of a stream
 * @param s the stream
 * @param content the new content
 */
void set_content(stream_t *s, void *content)
{
    if (s->content != NULL)
        free(s->content);

    size_t len;
    switch (s->type)
    {
    case PROMPT:
    case INT:
    case IS_SEAT_AVAILABLE:
    case RESERVE_SEAT:
    case PROMPT_INT_WITH_MAX:
    case SEAT_CANCELED:
        s->content = malloc(sizeof(int8_t));
        memcpy(s->content, content, 1);
        break;

    case STRING:
    case SET_SEAT_LASTNAME:
    case SET_SEAT_FIRSTNAME:
    case SET_SEAT_CODE:
    case SEND_SEAT_CODE:
        len = strlen((char *)content);
        s->content = malloc(len * sizeof(char));
        memcpy(s->content, content, len);
        ((char *)s->content)[len] = '\0';
        break;

    case SEND_SEATS:
        s->content = malloc(SEAT_AMOUNT * sizeof(bool));
        memcpy(s->content, content, SEAT_AMOUNT);
        break;

    default:
        s->content = NULL;
    }
}

/**
 * Free the memory used by the content of a stream
 * @param s the stream
 */
void destroy_stream(stream_t *s)
{
    if (s->content != NULL)
        free(s->content);
}

/**
 * Serialize a stream into a buffer
 * @param s the stream
 * @param buffer the buffer to fill in
 * @return the size of the buffer
 */
size_t serialize_stream(stream_t *s, void *buffer)
{
    *((uint8_t *)buffer) = s->type;
    buffer += sizeof(uint8_t);

    size_t len;
    switch (s->type)
    {
    case END_CONNECTION:
    case ASK_SEATS:
    case ERROR:
    case CANCEL_SEAT:
        return sizeof(uint8_t);

    case PROMPT:
    case INT:
    case IS_SEAT_AVAILABLE:
    case RESERVE_SEAT:
    case PROMPT_INT_WITH_MAX:
    case SEAT_CANCELED:
        memcpy(buffer, s->content, 1);
        return sizeof(uint8_t) + sizeof(uint8_t);

    case STRING:
    case SET_SEAT_LASTNAME:
    case SET_SEAT_FIRSTNAME:
    case SET_SEAT_CODE:
    case SEND_SEAT_CODE:
        len = strlen((char *)s->content);
        *((uint64_t *)buffer) = len;
        buffer += sizeof(uint64_t);
        memcpy(buffer, s->content, len);
        return sizeof(uint8_t) + sizeof(uint64_t) + len;

    case SEND_SEATS:
        memcpy(buffer, s->content, SEAT_AMOUNT);
        return sizeof(uint8_t) + SEAT_AMOUNT;

    default:
        return 0;
    }
}

/**
 * Unserialize a buffer into a stream
 * @param buffer the buffer
 * @param s the stream to fill in
 */
void unserialize_stream(void *buffer, stream_t *s)
{
    init_stream(s, *((uint8_t *)buffer));

    buffer += sizeof(uint8_t);
    size_t len;
    switch (s->type)
    {
    case PROMPT:
    case INT:
    case IS_SEAT_AVAILABLE:
    case RESERVE_SEAT:
    case PROMPT_INT_WITH_MAX:
    case SEAT_CANCELED:
        s->content = malloc(sizeof(int8_t));
        memcpy(s->content, buffer, 1);
        break;

    case STRING:
    case SET_SEAT_LASTNAME:
    case SET_SEAT_FIRSTNAME:
    case SET_SEAT_CODE:
    case SEND_SEAT_CODE:
        len = *((uint64_t *)buffer);
        buffer += sizeof(uint64_t);
        s->content = malloc((len + 1) * sizeof(char));
        memcpy(s->content, buffer, len);
        ((char *)s->content)[len] = '\0';
        break;

    case SEND_SEATS:
        s->content = malloc(SEAT_AMOUNT * sizeof(bool));
        memcpy(s->content, buffer, SEAT_AMOUNT);
        break;

    default:
        break;
    }
}
