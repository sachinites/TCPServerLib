#ifndef __TCPSERVER__
#define __TCPSERVER__

#include <stdint.h>
#include <string>
#include <pthread.h>

class TcpNewConnectionAcceptor;
class TcpClientServiceManager;
class TcpClientDbManager;
class TcpClient;

/* Server States */
#define TCP_SERVER_INITIALIZED (0)
#define TCP_SERVER_RUNNING (1)
#define TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS (2)
#define TCP_SERVER_NOT_LISTENING_CLIENTS (3)
#define TCP_SERVER_CREATE_MULTI_THREADED_CLIENT (4)
#define TCP_SERVER_STATE_MAX (5)

class TcpServer {

private:
    TcpNewConnectionAcceptor *tcp_new_conn_acc;
    TcpClientDbManager *tcp_client_db_mgr;
    TcpClientServiceManager *tcp_client_svc_mgr;
    uint32_t state_flags;

public:
    /* State Variables */
    uint32_t ip_addr;
    uint16_t port_no;
    std::string name;

    void (*client_connected)(const TcpClient *);
    void (*client_disconnected)(const TcpClient *);
    void (*client_msg_recvd)(const TcpClient *, unsigned char *, uint16_t);
    void (*client_ka_pending)(const TcpClient *);

    /* Constructors and Destructors */
    TcpServer(std::string ip_addr,  uint16_t port_no, std::string name);
    ~TcpServer();
    void Start();
    void Stop();
    uint32_t GetStateFlags();
    void SetAcceptNewConnectionStatus(bool);
    void SetClientCreationMode(bool);
    void SetServerNotifCallbacks(void (*client_connected)(const TcpClient *), 
                                                    void (*client_disconnected)(const TcpClient *),
                                                    void (*client_msg_recvd)(const TcpClient *, unsigned char *, uint16_t),
                                                    void (*client_ka_pending)(const TcpClient *) );
    
    /* Process Client Migration, used by Application */
    void ProcessClientMigrationToMultiThreaded(uint32_t ip_addr, uint16_t port_no);
    void ProcessClientMigrationToMultiplex(uint32_t ip_addr, uint16_t port_no);

    /* User by Acceptor Service */
    void ProcessNewClient(TcpClient *tcp_client);
    /* Used by Application */
    void ProcessClientDelete(uint32_t ip_addr, uint16_t port_no);
    /* Used by Multiplex Service */
    void RemoveClientFromDB(TcpClient *);

    /* To Pass the Request to Multiplex Service Mgr, this is Synchronous */
    void ClientFDStartListen(TcpClient *tcp_client);
    void StopListeningAllClients();

    /* Used my Multiplex service for client migration */
    void CreateMultiThreadedClient(TcpClient *);
};


#endif
