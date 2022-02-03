#ifndef __TCPSERVER__
#define __TCPSERVER__

#include <stdint.h>
#include <string>

class TcpNewConnectionAcceptor;
class TcpClientServiceManager;
class TcpClientDbManager;

class TcpServer {

    private:

    public:
        TcpNewConnectionAcceptor *tcp_new_conn_acc;
        TcpClientDbManager *tcp_client_db_mgr;
        TcpClientServiceManager *tcp_client_svc_mgr;
        /* State Variables */
        uint16_t ip_addr;
        uint16_t port_no;
        std::string name;
        
        /* A semaphore shared between TcpNewConnectionAcceptor thread and
        TcpClientDbManager thread for mutual exclusion */
        sem_t binary_semaphore_1;
        /* A semaphore shared between TcpClientServiceManager thread and
        TcpClientDbManager thread for mutual exclusion */
        sem_t binary_semaphore_2;

        /* Constructors and Destructors */
        TcpServer(uint16_t ip_addr,  uint16_t port_no, std::string name);
        ~TcpServer();
};


#endif