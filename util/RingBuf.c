#include "RingBuf.h"

int ringBufPush(ringBuf_t *buf, uint8_t data)
{
    int next = buf->head + 1;
    if (next == buf->maxLen)
        next = 0;
 
    // Cicular buffer is full
    if (next == buf->tail)
        return -1;  // quit with an error
 
    buf->buffer[buf->head] = data;
    buf->head = next;
    return 0;
}

int ringBufPop(ringBuf_t *buf, uint8_t *data)
{
    // if the head isn't ahead of the tail, we don't have any characters
    if (buf->head == buf->tail)
        return -1;  // quit with an error
 
    *data = buf->buffer[buf->tail];
    buf->buffer[buf->tail] = 0;  // clear the data (optional)
 
    int next = buf->tail + 1;
    if(next == buf->maxLen)
        next = 0;
 
    buf->tail = next;
 
    return 0;
}
