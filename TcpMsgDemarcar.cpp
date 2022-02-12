#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "TcpServer.h"
#include "TcpClient.h"
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"

TcpMsgDemarcar::TcpMsgDemarcar(
                        uint16_t iovec_unit_len) {

    IOVec *iovec = new IOVec();
    iovec->stream_buffer = (unsigned char *)calloc(1, iovec_unit_len);
    iovec->total_size = iovec_unit_len;
    iovec->cur_pos = 0;

    this->iovec_lst.push_back(iovec);
    this->iovec_unit_len = iovec_unit_len;
}

TcpMsgDemarcar::TcpMsgDemarcar() {

    IOVec *iovec = new IOVec();
    iovec->stream_buffer = (unsigned char *)calloc(1, DEFAULT_IOVEC_UNIT_LEN);
    iovec->total_size = iovec_unit_len;
    iovec->cur_pos = 0;

    this->iovec_lst.push_back(iovec);
    this->iovec_unit_len = DEFAULT_IOVEC_UNIT_LEN;
}

TcpMsgDemarcar::~TcpMsgDemarcar() {

    assert(this->iovec_lst.empty());
}

void
TcpMsgDemarcar::CopyData(unsigned char *data, uint16_t data_size) {

    uint16_t data_copied;
    uint16_t available_size;
     IOVec *new_iovec  = NULL;

    IOVec *iovec = this->iovec_lst.back();

    if (iovec->IsFull()) {

            new_iovec = new IOVec();
            new_iovec->stream_buffer = (unsigned char *)calloc(1, this->iovec_unit_len);
            new_iovec->total_size = this->iovec_unit_len;
            new_iovec->cur_pos = 0;
            this->iovec_lst.push_back(new_iovec);
            iovec = new_iovec;
    }

    available_size = iovec->total_size - iovec->cur_pos;
    
    if (available_size >= data_size) {

        memcpy(iovec->stream_buffer + iovec->cur_pos, data, data_size);
        iovec->cur_pos += data_size;
        this->total_iovec_msg_size += data_size;
        return;
    }

    memcpy(iovec->stream_buffer + iovec->cur_pos, data, available_size);
    iovec->cur_pos += available_size;
    this->total_iovec_msg_size += available_size;

    uint16_t data_left = data_size - available_size;
    new_iovec = new IOVec();
    new_iovec->stream_buffer = (unsigned char *)calloc(1, this->iovec_unit_len);
    new_iovec->total_size = this->iovec_unit_len;
    new_iovec->cur_pos = 0;
    this->iovec_lst.push_back(new_iovec);
    iovec = new_iovec;

    memcpy(iovec->stream_buffer + iovec->cur_pos, data + available_size, data_left);
    iovec->cur_pos += data_left;
    this->total_iovec_msg_size += data_left;
}

void
TcpMsgDemarcar::Destroy() {

    std::list<IOVec *>::iterator it;
    IOVec *curr, *next;

    for (it = this->iovec_lst.begin(),  curr = *it; 
            it != this->iovec_lst.end(); 
            curr = next) {

        next = *(++it);

        free(curr->stream_buffer);
        curr->stream_buffer = NULL;
        this->iovec_lst.remove(curr);
        delete curr;
    }
}

uint16_t
TcpMsgDemarcar::GetTotalIovecMsgSize() {

    return this->total_iovec_msg_size;
}

void
TcpMsgDemarcar::FlushIOVec() {

    std::list<IOVec *>::iterator it;
    IOVec *curr, *next;

    for (it = ++(this->iovec_lst.begin()), curr = *it; 
            it != this->iovec_lst.end(); 
            curr = next) {
        
        next = *(++it);

        free(curr->stream_buffer);
        curr->stream_buffer = NULL;
        this->iovec_lst.remove(curr);
        this->total_iovec_msg_size -= curr->cur_pos;
        delete curr;
    }

    curr = *this->iovec_lst.begin();

    if (!curr || !curr->stream_buffer) return;

    this->total_iovec_msg_size -= curr->cur_pos;
    curr->cur_pos = 0;
    assert(this->total_iovec_msg_size == 0);
}

void
TcpMsgDemarcar::AggregateIOVec(unsigned char *unified_buffer, uint16_t ubuff_size) {

    IOVec *curr;
    uint16_t pos = 0;
    std::list<IOVec *>::iterator it;

    assert(this->total_iovec_msg_size <= ubuff_size);

    for (it = this->iovec_lst.begin(); it != this->iovec_lst.end(); ++it) {

        curr = *it;
        memcpy(unified_buffer + pos , curr->stream_buffer, curr->cur_pos);
        pos += curr->cur_pos;
    }
}

void
TcpMsgDemarcar::ProcessMsg(
                    TcpClient *tcp_client,
                     unsigned char* msg_recvd, 
                     uint16_t msg_size) {

    this->CopyData(msg_recvd, msg_size);

    if (!this->IsBufferReadyToflush()) return;

    assert(this->GetTotalIovecMsgSize() <= MAX_CLIENT_BUFFER_SIZE);

    /* Avoid this step for performance reason, but makes life easy */
    this->AggregateIOVec(tcp_client->recv_buffer, MAX_CLIENT_BUFFER_SIZE);

    this->NotifyMsgToClient(tcp_client, tcp_client->recv_buffer, this->GetTotalIovecMsgSize());
    
    this->FlushIOVec();
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
            return NULL;
        case TCP_DEMARCAR_PATTERN:
            return NULL;
        case TCP_DEMARCAR_NONE:
            return NULL;
        default: ;
     }
     return NULL;
 }