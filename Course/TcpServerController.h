#ifndef __TCPSERVERCONTROLLER__
#define __TCPSERVERCONTROLLER__

#include <stdint.h>
#include <string>

class TcpNewConnectionAcceptor;
class TcpClientServiceManager;
class TcpClientDbManager;

class TcpServerController {

    private:
        TcpNewConnectionAcceptor *tcp_new_conn_acc;
        TcpClientDbManager *tcp_client_db_mgr;
        TcpClientServiceManager *tcp_client_svc_mgr;

    public:
        uint32_t ip_addr;
        uint16_t port_no;
        std::string name;

    /* Constructors and Destructors */
    TcpServerController(std::string ip_addr,  uint16_t port_no, std::string name);
    ~TcpServerController();
    void Start();
    void Stop();
};


#endif 