#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "ByteCircularBuffer.h"

TcpMsgFixedSizeDemarcar::TcpMsgFixedSizeDemarcar (
        uint16_t fixed_size) :

    TcpMsgDemarcar (DEFAULT_CBC_SIZE) {
    this->msg_fixed_size = fixed_size;
    this->buffer = (unsigned char *)calloc(fixed_size, sizeof(char));
}

TcpMsgFixedSizeDemarcar::~TcpMsgFixedSizeDemarcar() {

    assert(!this->buffer);
}

bool 
TcpMsgFixedSizeDemarcar::IsBufferReadyToflush() {

    if ((this->TcpMsgDemarcar::GetTotalMsgSize() / this->msg_fixed_size) > 0) {
        return true;
    }
    return false;
}

void
TcpMsgFixedSizeDemarcar::Destroy() {

    free(this->buffer);
    this->buffer = NULL;
}

void
TcpMsgFixedSizeDemarcar::NotifyMsgToClient(TcpClient *tcp_client) {

    uint16_t bytes_read;
    
    if (!this->IsBufferReadyToflush()) return;

    while (bytes_read = BCBRead(
                this->TcpMsgDemarcar::bcb, 
                this->buffer, 
                this->msg_fixed_size, true)) {

        tcp_client->tcp_server->client_msg_recvd(tcp_client, this->buffer, bytes_read);
    }
}