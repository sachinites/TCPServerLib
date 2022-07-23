#ifndef __TCP_DEMARCAR_FIXED_SIZE__
#define __TCP_DEMARCAR_FIXED_SIZE__

#include <stdint.h>
class Tcpclient;


class TcpMsgFixedSizeDemarcar: public TcpMsgDemarcar {

    private:
        uint16_t msg_fixed_size;
    public:
        bool IsBufferReadyToFlush();
        void ProcessClientMsg(TcpClient *tcp_client); 

    /* Constructor */
    TcpMsgFixedSizeDemarcar(uint16_t fixed_size);
    ~TcpMsgFixedSizeDemarcar();
    void Destroy();
};

#endif