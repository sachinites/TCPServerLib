#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "ByteCircularBuffer.h"

ByteCircularBuffer_t *
BCBCreateNew(uint16_t size) {

    ByteCircularBuffer_t *bcb = (ByteCircularBuffer_t *)calloc(1, sizeof(ByteCircularBuffer_t));
    bcb->buffer_size = size;
    bcb->buffer = (unsigned char *)calloc(size, sizeof(unsigned char));
    bcb->current_size = 0;
    bcb->front = 0;
    bcb->rear = 0;
    return bcb;
}

void
BCBFree(ByteCircularBuffer_t *bcb) {

    free(bcb->buffer);
    free(bcb);
}

uint16_t
BCBAvailableSize(ByteCircularBuffer_t *bcb) {

    return bcb->buffer_size - bcb->current_size;
}

uint16_t
BCBWrite(ByteCircularBuffer_t *bcb, unsigned char *data, uint16_t data_size) {

    if (BCBIsFull(bcb)) return 0;
    if (BCBAvailableSize(bcb) < data_size) return 0;
    
    if (bcb->front < bcb->rear) {
        memcpy(BCB(bcb, bcb->front), data, data_size);
        bcb->front += data_size;
        if (bcb->front == bcb->buffer_size) bcb->front = 0;
        bcb->current_size += data_size;
        return data_size;
    }

    uint16_t leading_space = bcb->buffer_size - bcb->front;

    if (data_size <= leading_space) {
        memcpy(BCB(bcb, bcb->front), data, data_size);
        bcb->front += data_size;
        if (bcb->front == bcb->buffer_size) bcb->front = 0;
        bcb->current_size += data_size;
        return data_size;
    }

    memcpy(BCB(bcb, bcb->front), data,  leading_space);
    memcpy(BCB(bcb, 0), data + leading_space ,  data_size - leading_space);
    bcb->front = data_size - leading_space;
    bcb->current_size += data_size;
    return data_size;
}

uint16_t
BCBRead(ByteCircularBuffer_t *bcb,
                 unsigned char* buffer, uint16_t data_size,
                 bool remove_read_bytes) {

    if (bcb->current_size < data_size) return 0;

    if (bcb->rear < bcb->front) {
        memcpy(buffer, BCB(bcb, bcb->rear), data_size);
        if (remove_read_bytes) {
            bcb->rear += data_size;
            if (bcb->rear == bcb->buffer_size) bcb->rear = 0;
            bcb->current_size -= data_size;
        }
        return data_size;
    }

    uint16_t leading_space = bcb->buffer_size - bcb->rear;

    if (data_size <= leading_space) {

        memcpy(buffer, BCB(bcb, bcb->rear), data_size);

        if (remove_read_bytes) {
            bcb->rear += data_size;
            if (bcb->rear == bcb->buffer_size) bcb->rear = 0;
            bcb->current_size -= data_size;
        }
        return data_size;
    }

    memcpy(buffer, BCB(bcb, bcb->rear), leading_space);
    memcpy(buffer, BCB(bcb, 0), data_size - leading_space);
     if (remove_read_bytes) {
         bcb->rear = (data_size - leading_space);
         bcb->current_size -= data_size;
     }
    return data_size;
}

bool
BCBIsFull(ByteCircularBuffer_t *bcb) {

    return bcb->current_size == bcb->buffer_size;
}

void
BCBReset(ByteCircularBuffer_t *bcb) {

    bcb->current_size = 0;
    bcb->front = 0;
    bcb->rear = 0;
}