#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "TcpClient.h"
#include "TcpServerController.h"
#include "ByteCircularBuffer.h"
#include "TcpMsgVariableSizeDemarcar.h"

#define HDR_MSG_SIZE    2

TcpMsgVariableSizeDemarcar::TcpMsgVariableSizeDemarcar () :

    TcpMsgDemarcar (DEFAULT_CBC_SIZE) {
}

TcpMsgVariableSizeDemarcar::~TcpMsgVariableSizeDemarcar() {

}

void
TcpMsgVariableSizeDemarcar::Destroy() {

    this->TcpMsgDemarcar::Destroy();
}

bool 
TcpMsgVariableSizeDemarcar::IsBufferReadyToflush() {

    uint16_t msg_size;
   ByteCircularBuffer_t *bcb = this->TcpMsgDemarcar::bcb;
   
   if (bcb->current_size <= HDR_MSG_SIZE) return false;

    BCBRead(bcb, (unsigned char *)&msg_size, HDR_MSG_SIZE, false);

    if (msg_size <= bcb->current_size) return true;
    
    return false;
}

void
TcpMsgVariableSizeDemarcar::ProcessClientMsg(TcpClient *tcp_client) {

    uint16_t msg_size;
    ByteCircularBuffer_t *bcb = this->TcpMsgDemarcar::bcb;

    while (this->IsBufferReadyToflush()) {

        BCBRead(bcb, (unsigned char *)&msg_size, HDR_MSG_SIZE, false);
        assert(msg_size == BCBRead(bcb, this->TcpMsgDemarcar::buffer, msg_size, true));

        tcp_client->tcp_ctrlr->client_msg_recvd(tcp_client->tcp_ctrlr, tcp_client,  this->TcpMsgDemarcar::buffer, msg_size);
    }
}
