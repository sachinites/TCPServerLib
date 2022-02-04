#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <stdint.h>

#define MAX_CLIENT_BUFFER_SIZE 512

class TcpClient {

    public :
        uint32_t ip_addr;
        uint16_t port_no;
        int comm_fd;
        TcpClient(uint32_t ip_addr, uint16_t port_no);
        TcpClient();
        TcpClient (TcpClient *);
        ~TcpClient();
        void Abort();
        unsigned char recv_buffer[MAX_CLIENT_BUFFER_SIZE];
};

#endif