#ifndef __TCP_CONN__
#define __TCP_CONN__

#include <stdint.h>

typedef enum {

    tcp_conn_none,
    tcp_conn_via_accept,
    tcp_conn_via_connect
} tcp_connection_type_t;

class TcpConn {

    private:
    public:
        tcp_connection_type_t conn_type;
        uint32_t ka_sent;
        uint32_t ka_recvd;
        uint32_t bytes_sent;
        uint32_t bytes_recvd;
        TcpConn();
        ~TcpConn();
};

#endif