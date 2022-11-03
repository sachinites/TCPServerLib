#ifndef __TCPSERVER__
#define __TCPSERVER__

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <list>
#include "TcpMsgDemarcar.h"

class TcpNewConnectionAcceptor;
class TcpClientServiceManager;
class TcpClientDbManager;
class TcpClient;

/* Server States */
#define TCP_SERVER_INITIALIZED (1)
#define TCP_SERVER_RUNNING (2)
#define TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS (4)
#define TCP_SERVER_NOT_LISTENING_CLIENTS (8)
#define TCP_SERVER_CREATE_MULTI_THREADED_CLIENT (16)

typedef enum tcp_server_msg_code_ {

    CTRLR_ACTION_TCP_CLIENT_PROCESS_NEW = 1,
    CTRLR_ACTION_TCP_CLIENT_MULTIPLEX_LISTEN = 2,
    CTRLR_ACTION_TCP_CLIENT_DELETE = 4,
    CTRLR_ACTION_TCP_CLIENT_MX_TO_MULTITHREADED = 8,
    CTRLR_ACTION_TCP_CLIENT_MULTITHREADED_TO_MX = 16,
    CTRLR_ACTION_TCP_CLIENT_CREATE_THREADED = 32,
    CTRLR_ACTION_TCP_CLIENT_ACTIVE_CONNECT_SUCCESS = 64,
    CTRLR_ACTION_TCP_CLIENT_RECONNECT = 128,
    CTRLR_ACTION_TCP_SERVER_OP_MAX = 256
} tcp_server_msg_code_t;

typedef struct TcpServerMsg_ {
 
    tcp_server_msg_code_t code;
    void *data;
    sem_t *zero_sema;
} TcpServerMsg_t;

class TcpServerController {

private:
    TcpNewConnectionAcceptor *tcp_new_conn_acc;
    TcpClientDbManager *tcp_client_db_mgr;
    TcpClientServiceManager *tcp_client_svc_mgr;
    uint32_t state_flags;

    pthread_rwlock_t connect_db_rwlock;
    std::list<TcpClient *> establishedClient;
    std::list<TcpClient *> connectpendingClients;

    /* Server Msg Q */
    std::list<TcpServerMsg_t *> msgQ;
    pthread_mutex_t msgq_mutex;
    pthread_cond_t msgq_cv;
    pthread_t msgQ_op_thread;
    void ProcessMsgQMsg(TcpServerMsg_t *msg);

public:
    /* State Variables */
    uint32_t ip_addr;
    uint16_t port_no;
    std::string name;
    TcpMsgDemarcarType msgd_type;


    void (*client_connected)(const TcpServerController *, const TcpClient *);
    void (*client_disconnected)(const TcpServerController *, const TcpClient *);
    void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t);
    void (*client_ka_pending)(const TcpServerController *, const TcpClient *);

    /* Constructors and Destructors */
    TcpServerController(std::string ip_addr,  uint16_t port_no, std::string name);
    ~TcpServerController();
    void Start();
    void Stop();
    uint32_t GetStateFlags();
    void SetBit (uint32_t bit);
    void UnSetBit (uint32_t bit);
    bool IsBitSet (uint32_t bit);

    void SetClientCreationMode(bool);
    void SetServerNotifCallbacks(void (*client_connected)(const TcpServerController *, const TcpClient *), 
                                                    void (*client_disconnected)(const TcpServerController *, const TcpClient *),
                                                    void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t),
                                                    void (*client_ka_pending)(const TcpServerController *, const TcpClient *) );
    
    /* Process Client Migration, used by Application */
    void ProcessClientMigrationToMultiThreaded(uint32_t ip_addr, uint16_t port_no);
    void ProcessClientMigrationToMultiplex(uint32_t ip_addr, uint16_t port_no);

    /* User by Acceptor Service */
    void ProcessNewClient(TcpClient *tcp_client);
    /* Used by Application */
    void ProcessClientDelete(uint32_t ip_addr, uint16_t port_no);
    void ProcessClientDelete(TcpClient *);

    /* Used by Multiplex Service */
    void RemoveClientFromDB(TcpClient *);

    /* To Pass the Request to Multiplex Service Mgr, this is Synchronous */
    void ClientFDStartListen(TcpClient *tcp_client);

    /* Used my Multiplex service for client migration */
    void CreateMultiThreadedClient(TcpClient *);

    /* Accept/No Accept of new Connections */
    void StopConnectionsAcceptorSvc();
    void StartConnectionsAcceptorSvc();
    /* Listen for Connected Clients */
    void StopClientSvcMgr();
    void StartClientSvcMgr();

    void SetTcpMsgDemarcar(TcpMsgDemarcarType);

    /* Print the Tcp Server Details */
    void Display();
    void MsgQProcessingThreadFn();
    void EnqueMsg (tcp_server_msg_code_t code, void *data, bool block_me);
    void CreateActiveClient (uint32_t server_ip_addr, uint16_t server_port_no);
    void CopyAllClientsTolist (std::list<TcpClient *> *list);
    TcpClient *LookupActiveOpened (uint32_t ip_addr, uint16_t port_no);
};


#endif
