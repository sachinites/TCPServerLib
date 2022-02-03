#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <stdint.h>

class TcpClient {

    public :
        uint32_t ip_addr;
        uint16_t port_no;
        int comm_fd;
        TcpClient(uint32_t ip_addr, uint16_t port_no);
        TcpClient();
        TcpClient (TcpClient *);
        ~TcpClient();
};

#endif