#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "ByteCircularBuffer.h"
#include "TcpClient.h"

TcpMsgDemarcar::TcpMsgDemarcar( uint16_t circular_buffer_len) {

    this->bcb = BCBCreateNew(circular_buffer_len);
    this->buffer = (unsigned char *)calloc (MAX_CLIENT_BUFFER_SIZE,  sizeof(unsigned char));
}

TcpMsgDemarcar::TcpMsgDemarcar() {

    this->bcb = BCBCreateNew(DEFAULT_CBC_SIZE);
    this->buffer = (unsigned char *)calloc (MAX_CLIENT_BUFFER_SIZE, sizeof(unsigned char));
}

TcpMsgDemarcar::~TcpMsgDemarcar() {

    assert(!this->bcb);
    assert(!this->buffer);
}

void
TcpMsgDemarcar::Destroy() {

    if (this->bcb ){
        BCBFree(this->bcb);
        this->bcb = NULL;
    }
    if (this->buffer) {
        free(this->buffer);
        this->buffer = NULL;
    }
}

void
TcpMsgDemarcar::ProcessMsg (TcpClient *tcp_client, unsigned char *msg_recvd, uint16_t msg_size) {

    assert(BCBWrite (this->bcb, msg_recvd, msg_size));

    if (!this->IsBufferReadyToFlush()) return;

    this->ProcessClientMsg (tcp_client);
}
