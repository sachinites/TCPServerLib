#ifndef __TCP_CONN__
#define __TCP_CONN__

typedef enum {

    tcp_conn_none,
    tcp_conn_via_accept,
    tcp_conn_via_connect
} tcp_connection_type_t;

class TcpConn {

    private:
    public:
        tcp_connection_type_t conn_type;
        TcpConn();
        ~TcpConn();
};

#endif