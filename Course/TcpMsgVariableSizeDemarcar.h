#ifndef __TCP_DEMARCAR_VARIABLE_SIZE__
#define __TCP_DEMARCAR_VARIABLE_SIZE__

#include <stdint.h>

#define VARIABLE_SIZE_MAX_BUFFER    256

class TcpClient;
class TcpMsgVariableSizeDemarcar : public TcpMsgDemarcar{

    private:
    public:
        bool IsBufferReadyToFlush();
        void ProcessClientMsg(TcpClient *);
        
        TcpMsgVariableSizeDemarcar();
        ~TcpMsgVariableSizeDemarcar();
        void Destroy();
};

#endif