#ifndef RINGBUF_H 
#define RINGBUF_H

#include <stdint.h>

//Defines a ring buffer with the first parameter as its name and the second parameter as its size
#define RINGBUF_DEF(x,y) uint8_t x##_space[y]; ringBuf_t x = { x##_space,0,0,y}

/**
 * Simple implementation of a ring buffer which does not overwrite old data when it gets full. 
 * The head variable indicates where we should insert a new element, and the tail variable
 * indicates where we should pop. Note with this implementation, the last byte in the buffer 
 * is never used. 
 */

typedef struct {
    uint8_t *const buffer;
    int head;
    int tail;
    const int maxLen;
} ringBuf_t;
/**
 * Pushes an unsigned 8 bit int to the buffer of a ringBuf_t object,
 * returns -1 if the ring buffer is full and will not push the data 
 * on. User should check and handle this appropriately.  
 */
int ringBufPush(ringBuf_t *buf, uint8_t data);

/**
 * Pops an unsigned 8 bit in to the location specified by *data. 
 * returns -1 if the ring buffer is empty.
 */
int ringBufPop(ringBuf_t *buf, uint8_t *data);
#endif
