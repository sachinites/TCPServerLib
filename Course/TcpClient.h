#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <stdint.h>

class TcpServerController;

class TcpClient {

    private:
    public:
        uint32_t ip_addr;
        uint16_t port_no;
        int comm_fd;
        TcpServerController *tcp_ctrlr;
        TcpClient(uint32_t ip_addr, uint16_t port_no);
} ;


#endif