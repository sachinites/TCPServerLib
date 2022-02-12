/* Source : https://www.codeproject.com/Articles/11922/Solution-for-TCP-IP-client-socket-message-boundary */

#ifndef __TCP_MSG_DEMARCAR__
#define __TCP_MSG_DEMARCAR__

#include <stdint.h>

#define DEFAULT_CBC_SIZE  256

typedef enum {

        TCP_DEMARCAR_FIXED_SIZE,
        TCP_DEMARCAR_VARIABLE_SIZE,
        TCP_DEMARCAR_PATTERN,
        TCP_DEMARCAR_NONE
} TcpMsgDemarcarType;

class TcpClient;
typedef struct ByteCircularBuffer_ ByteCircularBuffer_t;

class TcpMsgDemarcar {

    private:

    public:
        ByteCircularBuffer_t *bcb;
        static TcpMsgDemarcar *InstantiateTcpMsgDemarcar(
                                                        TcpMsgDemarcarType,
                                                         uint16_t fixed_size,
                                                         unsigned char start_pattern[],
                                                         uint8_t start_pattern_size,
                                                         unsigned char end_pattern[],
                                                         uint8_t end_pattern_size);

        void CopyData(unsigned char *data, uint16_t data_size);

        /* To be Implemented by derieved classes */
        virtual bool IsBufferReadyToflush() = 0;
        virtual void NotifyMsgToClient(TcpClient *tcp_client) = 0;

        /* Constructor */
        TcpMsgDemarcar(uint16_t iovec_unit_len);

        TcpMsgDemarcar();

        /* Destructor */
        ~TcpMsgDemarcar();

        uint16_t GetTotalMsgSize();
        void Destroy();
        void ProcessMsg( TcpClient *, unsigned char *msg_recvd,  uint16_t msg_size);
};

#endif