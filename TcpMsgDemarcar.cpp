#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "TcpClient.h"
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"
#include "TcpMsgVariableSizeDemarcar.h"
#include "ByteCircularBuffer.h"

TcpMsgDemarcar::TcpMsgDemarcar(
                        uint16_t circular_buffer_size) {

    this->bcb = BCBCreateNew(circular_buffer_size);
    this->buffer = (unsigned char *)calloc (MAX_CLIENT_BUFFER_SIZE, sizeof(unsigned char));
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

uint16_t
TcpMsgDemarcar::GetTotalMsgSize() {

    return this->bcb->current_size;
}

void
TcpMsgDemarcar::ProcessMsg(
                    TcpClient *tcp_client,
                     unsigned char* msg_recvd, 
                     uint16_t msg_size) {

    assert ( BCBWrite (this->bcb, msg_recvd, msg_size) );

    if (!this->IsBufferReadyToflush()) return;

   this->ProcessClientMsg(tcp_client);
}

 TcpMsgDemarcar*
 TcpMsgDemarcar::InstantiateTcpMsgDemarcar(
        TcpMsgDemarcarType msgd_type,
        uint16_t fixed_size,
        unsigned char  start_pattern[],
        uint8_t start_pattern_size,
        unsigned char end_pattern[],
        uint8_t end_pattern_size) {

     switch(msgd_type) {

         case TCP_DEMARCAR_FIXED_SIZE:
            return new TcpMsgFixedSizeDemarcar(fixed_size);
        case TCP_DEMARCAR_VARIABLE_SIZE:
            return new TcpMsgVariableSizeDemarcar();
        case TCP_DEMARCAR_PATTERN:
            return NULL;
        case TCP_DEMARCAR_NONE:
            return NULL;
        default: ;
     }
     return NULL;
 }
