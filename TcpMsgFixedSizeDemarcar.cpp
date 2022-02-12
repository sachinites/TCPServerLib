#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"
#include "TcpClient.h"
#include "TcpServer.h"

TcpMsgFixedSizeDemarcar::TcpMsgFixedSizeDemarcar (
        uint16_t fixed_size) :

    TcpMsgDemarcar (DEFAULT_IOVEC_UNIT_LEN) {
    msg_fixed_size = fixed_size;
}

TcpMsgFixedSizeDemarcar::~TcpMsgFixedSizeDemarcar() {

}

bool 
TcpMsgFixedSizeDemarcar::IsBufferReadyToflush() {

    if ((this->TcpMsgDemarcar::GetTotalIovecMsgSize() % this->msg_fixed_size) == 0) {
        return true;
    }
    return false;
}

void
TcpMsgFixedSizeDemarcar::NotifyMsgToClient(
        TcpClient *tcp_client, 
        unsigned char *msg, 
        uint16_t msg_size) {

    int i, units;

    assert(!(msg_size % this->msg_fixed_size));

    if (!tcp_client->tcp_server->client_msg_recvd) return;

    units = msg_size / this->msg_fixed_size;
    for ( i = 0 ; i  < units; i++) {
        tcp_client->tcp_server->client_msg_recvd(tcp_client, msg + (i * msg_size), msg_size);
    }
}