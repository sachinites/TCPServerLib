#ifndef __TCP_DEMARCAR_FIXED_SIZE__
#define __TCP_DEMARCAR_FIXED_SIZE__

#include <stdint.h>

class TcpClient;
class TcpMsgFixedSizeDemarcar : public TcpMsgDemarcar{

    private:
        uint16_t msg_fixed_size;

    public:
        bool IsBufferReadyToflush();
        void ProcessClientMsg(TcpClient *);
        
        TcpMsgFixedSizeDemarcar(uint16_t fixed_size);

        ~TcpMsgFixedSizeDemarcar();
        void Destroy();
};

#endif