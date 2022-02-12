/* Source : https://www.codeproject.com/Articles/11922/Solution-for-TCP-IP-client-socket-message-boundary */

#ifndef __TCP_MSG_DEMARCAR__
#define __TCP_MSG_DEMARCAR__

#include <stdint.h>
#include <list>

#define DEFAULT_IOVEC_UNIT_LEN  128

typedef enum {

        TCP_DEMARCAR_FIXED_SIZE,
        TCP_DEMARCAR_VARIABLE_SIZE,
        TCP_DEMARCAR_PATTERN,
        TCP_DEMARCAR_NONE
} TcpMsgDemarcarType;

class IOVec {

    public:
        unsigned char *stream_buffer;
        uint16_t total_size;
        uint16_t cur_pos;
        bool inline IsFull() {
            return cur_pos == total_size;
        }
};

class TcpClient;
class TcpMsgDemarcar {

    private:
        std::list<IOVec *> iovec_lst;
        uint16_t total_iovec_msg_size;
        uint16_t iovec_unit_len;

    public:
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
        virtual void NotifyMsgToClient(TcpClient *, unsigned char *, uint16_t ) = 0;
        
        /* Constructor */
        TcpMsgDemarcar(uint16_t iovec_unit_len);

        TcpMsgDemarcar();

        /* Destructor */
        ~TcpMsgDemarcar();

        uint16_t GetTotalIovecMsgSize();
        void FlushIOVec();
        void AggregateIOVec(unsigned char *unified_buffer, uint16_t ubuff_size);
        void Destroy();
        void ProcessMsg( TcpClient *, unsigned char *msg_recvd,  uint16_t msg_size);
};

#endif