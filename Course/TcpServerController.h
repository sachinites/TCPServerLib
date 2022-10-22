#ifndef __TCPSERVERCONTROLLER__
#define __TCPSERVERCONTROLLER__

#include <stdint.h>
#include <string>
#include <list>

/* Server States */
#define TCP_SERVER_INITIALIZED (1)
#define TCP_SERVER_RUNNING (2)
#define TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS (4)
#define TCP_SERVER_NOT_LISTENING_CLIENTS (8)
#define TCP_SERVER_CREATE_MULTI_THREADED_CLIENT (16)

class TcpNewConnectionAcceptor;
class TcpClientServiceManager;
class TcpClientDbManager;
class TcpClient;

class TcpServerController {

    private:
        TcpNewConnectionAcceptor *tcp_new_conn_acc;
        TcpClientDbManager *tcp_client_db_mgr;
        TcpClientServiceManager *tcp_client_svc_mgr;
        uint32_t state_flags;

     public:
        uint32_t ip_addr;
        uint16_t port_no;
        std::string name;

        void (*client_connected)(const TcpServerController *, const TcpClient *);
        void (*client_disconnected)(const TcpServerController *, const TcpClient *);
        void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t);

        void SetServerNotifCallbacks(
                void (*client_connected)(const TcpServerController *, const TcpClient *),
                void (*client_disconnected)(const TcpServerController *, const TcpClient *),
                void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t)
        );

    /* Constructors and Destructors */
    TcpServerController(std::string ip_addr,  uint16_t port_no, std::string name);
    ~TcpServerController();
    void SetBit (uint32_t bit);
    void UnSetBit (uint32_t bit);
    bool IsBitSet (uint32_t bit);
    void Start();
    void Stop();
    void ProcessNewClient(TcpClient *tcp_client) ;
    void Display();
    void StopConnectionAcceptingSvc();
    void StartConnectionAcceptingSvc();
    /* Listen for Connected Clients */
    void StopClientSvcMgr();
    void StartClientSvcMgr();
    void CopyAllClientsTolist (std::list<TcpClient *> *list);
};


#endif 