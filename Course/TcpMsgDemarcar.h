#ifndef __TCP_MSG_DEMARCAR__
#define __TCP_MSG_DEMARCAR__

#include <stdint.h>
#define DEFAULT_CBC_SIZE    10240

typedef enum TcpMsgDemarcarType_ {

    TCP_DEMARCAR_NONE,
    TCP_DEMARCAR_FIXES_SIZE,
    TCP_DEMARCAR_VARIABLE_SIZE,
    TCP_DEMARCAR_PATTERN
} TcpMsgDemarcarType;

typedef struct ByteCircularBuffer_ ByteCircularBuffer_t;
class TcpClient;

class TcpMsgDemarcar {

    private:
    protected:
        ByteCircularBuffer_t *bcb;
        unsigned char *buffer;
    public:
        virtual bool IsBufferReadyToFlush() = 0;
        virtual void ProcessClientMsg(TcpClient *) = 0;        

    /* Constructor */
    TcpMsgDemarcar(uint16_t circular_buffer_len);
    TcpMsgDemarcar();
    ~TcpMsgDemarcar();

    void Destroy();
    void ProcessMsg (TcpClient *, unsigned char *msg_recvd, uint16_t msg_size);
};


#endif 