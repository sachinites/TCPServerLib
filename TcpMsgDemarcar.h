/* Source : https://www.codeproject.com/Articles/11922/Solution-for-TCP-IP-client-socket-message-boundary */

#ifndef __TCP_MSG_DEMARCAR__
#define __TCP_MSG_DEMARCAR__

#include <stdint.h>

#define DEFAULT_CBC_SIZE  (10240)

typedef enum {

        TCP_DEMARCAR_NONE,
        TCP_DEMARCAR_FIXED_SIZE,
        TCP_DEMARCAR_VARIABLE_SIZE,
        TCP_DEMARCAR_PATTERN
} TcpMsgDemarcarType;

class TcpClient;
typedef struct ByteCircularBuffer_ ByteCircularBuffer_t;

class TcpMsgDemarcar {

    private:
    protected:
        ByteCircularBuffer_t *bcb;
        unsigned char *buffer;

    public:

        static TcpMsgDemarcar *InstantiateTcpMsgDemarcar(
                                                        TcpMsgDemarcarType,
                                                         uint16_t fixed_size,
                                                         unsigned char start_pattern[],
                                                         uint8_t start_pattern_size,
                                                         unsigned char end_pattern[],
                                                         uint8_t end_pattern_size);

        /* To be Implemented by derieved classes */
        virtual bool IsBufferReadyToflush() = 0;
        virtual void ProcessClientMsg(TcpClient *tcp_client) = 0;

        /* Constructor */
        TcpMsgDemarcar(uint16_t circular_buffer_len);

        TcpMsgDemarcar();

        /* Destructor */
        ~TcpMsgDemarcar();

        uint16_t GetTotalMsgSize();
        void Destroy();
        void ProcessMsg( TcpClient *, unsigned char *msg_recvd,  uint16_t msg_size);
};

#endif