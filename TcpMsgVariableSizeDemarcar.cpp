#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "ByteCircularBuffer.h"
#include "TcpMsgVariableSizeDemarcar.h"

TcpMsgVariableSizeDemarcar::TcpMsgVariableSizeDemarcar () :

    TcpMsgDemarcar (DEFAULT_CBC_SIZE) {
    this->buffer = (unsigned char *)calloc(VARIABLE_SIZE_MAX_BUFFER, sizeof(char));
}

TcpMsgVariableSizeDemarcar::~TcpMsgVariableSizeDemarcar() {

    assert(!this->buffer);
}

void
TcpMsgVariableSizeDemarcar::Destroy() {

    free(this->buffer);
    this->buffer = NULL;
}

bool 
TcpMsgVariableSizeDemarcar::IsBufferReadyToflush() {

    uint16_t msg_size;
   ByteCircularBuffer_t *bcb = this->TcpMsgDemarcar::bcb;
   
   if (bcb->current_size <= sizeof(uint16_t)) return false;

    BCBRead(bcb, (unsigned char *)&msg_size, sizeof(uint16_t), false);

    if (msg_size <= bcb->current_size) return true;
    
    return false;
}

void
TcpMsgVariableSizeDemarcar::NotifyMsgToClient(TcpClient *tcp_client) {

    uint16_t msg_size;
    ByteCircularBuffer_t *bcb = this->TcpMsgDemarcar::bcb;

    while (this->IsBufferReadyToflush()) {

        BCBRead(bcb, (unsigned char *)&msg_size, sizeof(uint16_t), false);
        assert(msg_size == BCBRead(bcb, this->buffer, msg_size, true));

        tcp_client->tcp_server->client_msg_recvd(tcp_client,  this->buffer, msg_size);
    }
}