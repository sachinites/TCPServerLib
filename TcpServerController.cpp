#include <assert.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "TcpServerController.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClientDbManager.h"
#include "TcpClientServiceManager.h"
#include "TcpMsgDemarcar.h"
#include "network_utils.h"
#include "TcpClient.h"

class TcpMsgDemarcar;

TcpServerController::TcpServerController(std::string ip_addr,  uint16_t port_no, std::string name) {

    this->ip_addr = network_covert_ip_p_to_n(ip_addr.c_str());
    this->port_no = port_no;
    this->name = name;
    this->tcp_new_conn_acc = new TcpNewConnectionAcceptor(this);
    this->tcp_client_db_mgr = new TcpClientDbManager(this);
    this->tcp_client_svc_mgr = new TcpClientServiceManager(this);
    this->msgd_type = TCP_DEMARCAR_FIXED_SIZE;
    pthread_mutex_init (&this->msgq_mutex, NULL);
    pthread_cond_init (&this->msgq_cv, NULL);
    pthread_rwlock_init(&this->connect_db_rwlock, NULL);
    this->state_flags = 0;
    this->SetBit(TCP_SERVER_INITIALIZED);
}

TcpServerController::~TcpServerController() {
    
    assert(!this->tcp_new_conn_acc);
    assert(!this->tcp_client_db_mgr);
    assert(!this->tcp_client_svc_mgr);
    assert (this->connectpendingClients.empty());
    assert (this->establishedClient.empty());
    printf ("TcpServerController %s Stopped\n", this->name.c_str());
}

static void *
tcp_server_msgq_thread_fn (void *arg) {

    TcpServerController *tcp_ctrlr = (TcpServerController *)arg;
    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL);
    tcp_ctrlr->MsgQProcessingThreadFn();
    return NULL;
}

void
TcpServerController::Start() {

    assert(this->tcp_new_conn_acc);
    assert(this->tcp_client_db_mgr);
    assert(this->tcp_client_svc_mgr);

    if (!this->IsBitSet (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS)) {
        this->tcp_new_conn_acc->StartTcpNewConnectionAcceptorThread();
    }
    if (!this->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        this->tcp_client_svc_mgr->StartTcpClientServiceManagerThread();
    }

    /* Initializing and Starting TCP Server Msg Q Thread */
    pthread_create (&this->msgQ_op_thread, NULL, tcp_server_msgq_thread_fn, (void *)this);

    this->SetBit(TCP_SERVER_RUNNING);

    printf ("Tcp Server is Up and Running [%s, %d]\nOk.\n", 
        network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);
}

void
TcpServerController::Stop() {

    TcpClient *tcp_client;

    if (this->tcp_new_conn_acc) {
        this->tcp_new_conn_acc->Stop();
        this->tcp_new_conn_acc = NULL;
        this->SetBit (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    }

    if (this->tcp_client_svc_mgr) {
        this->tcp_client_svc_mgr->Stop();
        this->tcp_client_svc_mgr = NULL;
        this->SetBit (TCP_SERVER_NOT_LISTENING_CLIENTS);
    }

    /* Stopping the above two services first ensures that
        now no thread is alive which could add tcpclient back into
        DB */
    this->tcp_client_db_mgr->Purge();
    delete this->tcp_client_db_mgr;
    this->tcp_client_db_mgr = NULL;

    pthread_rwlock_wrlock (&this->connect_db_rwlock);

    while (!this->connectpendingClients.empty()) {
        
        tcp_client = this->connectpendingClients.front();
        assert (tcp_client->IsStateSet (TCP_CLIENT_STATE_CONNECT_IN_PROGRESS));
        tcp_client->StopConnectorThread();
        this->connectpendingClients.pop_front();
        tcp_client->Dereference();
    }

    while (!this->establishedClient.empty()) {

        tcp_client = this->establishedClient.front();
        assert (tcp_client->IsStateSet (TCP_CLIENT_STATE_CONNECTED));
        tcp_client->StopThread();
        this->establishedClient.pop_front();
        tcp_client->Dereference();
    }

    pthread_rwlock_unlock (&this->connect_db_rwlock);
    pthread_rwlock_destroy (&this->connect_db_rwlock);

    this->UnSetBit(TCP_SERVER_RUNNING);
    delete this;
}

uint32_t
TcpServerController::GetStateFlags() {

    return this->state_flags;
}


void
TcpServerController::SetClientCreationMode(bool status) {

    if (status &&
        this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {
            return;
     }

     if (!status && 
          !this->IsBitSet( TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {
              return;
    }

     if (status) {
        this->SetBit( TCP_SERVER_CREATE_MULTI_THREADED_CLIENT);
    }
    else {
        this->UnSetBit(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT);
    }
}

void
TcpServerController::SetServerNotifCallbacks(
                void (*client_connected)(const TcpServerController *, const TcpClient *), 
                void (*client_disconnected)(const TcpServerController *, const TcpClient *),
                void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t),
                void (*client_ka_pending)(const TcpServerController *, const TcpClient *) ) {

    /* Should be called before invoking Start() on TCP Server */
    assert (this->state_flags == TCP_SERVER_INITIALIZED);
    this->client_connected = client_connected;
    this->client_ka_pending = client_ka_pending;
    this->client_msg_recvd = client_msg_recvd;
    this->client_disconnected = client_disconnected;
}

void
TcpServerController::CreateMultiThreadedClient(TcpClient *tcp_client) {

    assert(tcp_client->client_thread == NULL);
    tcp_client->StartThread();
}

void
TcpServerController::ProcessNewClient(TcpClient *tcp_client) {

    this->tcp_client_db_mgr->AddClientToDB(tcp_client);

    if (this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {

        this->CreateMultiThreadedClient(tcp_client);
    }
    else {

        this->tcp_client_svc_mgr->ClientFDStartListen(tcp_client);
    }
}

void
TcpServerController::ProcessClientDelete(uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client = 
        this->tcp_client_db_mgr->RemoveClientFromDB(ip_addr, port_no);
    
    if (!tcp_client) {
        printf ("Error : Such a client dont exist \n");
        return;
    }

    if (!tcp_client->client_thread) {
        this->tcp_client_svc_mgr->ClientFDStopListen(tcp_client);
    }
}

void
TcpServerController::ProcessClientDelete(TcpClient *tcp_client) {

    tcp_client->Reference();

    if (tcp_client->IsStateSet (TCP_CLIENT_STATE_PASSIVE_OPENER)) {
        this->tcp_client_db_mgr->RemoveClientFromDB(tcp_client);
    }
    
    if (tcp_client->IsStateSet (TCP_CLIENT_STATE_MULTIPLEX_LISTEN)) {
        this->tcp_client_svc_mgr->ClientFDStopListen(tcp_client);
    }

    if (tcp_client->client_thread) {
        tcp_client->StopThread();
    }

    if (tcp_client->IsStateSet (TCP_CLIENT_STATE_ACTIVE_OPENER)) {
        pthread_rwlock_wrlock (&this->connect_db_rwlock);
        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECTED))
        {
            this->establishedClient.remove(tcp_client);
            tcp_client->Dereference();
        }
        else
        {
            this->connectpendingClients.remove(tcp_client);
            tcp_client->Dereference();
        }
        pthread_rwlock_unlock (&this->connect_db_rwlock);
    }

    tcp_client->Dereference();
}

void
TcpServerController::RemoveClientFromDB(TcpClient *tcp_client) {

    this->tcp_client_db_mgr->RemoveClientFromDB(tcp_client);
}

void
TcpServerController::ClientFDStartListen(TcpClient *tcp_client) {

    this->tcp_client_svc_mgr->ClientFDStartListen(tcp_client);
}

void
TcpServerController::ProcessClientMigrationToMultiThreaded(uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client = 
        this->tcp_client_db_mgr->LookUpClientDB_ThreadSafe(ip_addr, port_no);
    
    if (!tcp_client) return;

    if (tcp_client->client_thread) {
        printf ("Error : Client is already Multi-Threaded\n");
        return;
    }

    this->tcp_client_svc_mgr->ClientFDStopListen(tcp_client);
    this->CreateMultiThreadedClient(tcp_client);
}

void
TcpServerController::ProcessClientMigrationToMultiplex(uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client = 
        this->tcp_client_db_mgr->LookUpClientDB_ThreadSafe(ip_addr, port_no);
    
    if (!tcp_client) return;

    if (!tcp_client->client_thread) {
        printf ("Error : Client is already Multiplexed\n");
        return;
    }

    tcp_client->StopThread();
    this->tcp_client_svc_mgr->ClientFDStartListen(tcp_client);
}

void 
TcpServerController::SetTcpMsgDemarcar(TcpMsgDemarcarType msgd_type) {

    this->msgd_type = msgd_type;
}

void
TcpServerController::Display() {

    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    printf ("Server Name : %s\n", this->name.c_str());

    if (!this->IsBitSet (TCP_SERVER_RUNNING)) {
        printf ("Not Running\n");
        return;
    }

    printf ("Listening on : [%s, %d]\n", network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);

    printf ("Flags :  ");

    if (this->IsBitSet (TCP_SERVER_INITIALIZED)) {
        printf ("I");
    }
    else {
        printf ("Un-I");
    }

    if (this->IsBitSet (TCP_SERVER_RUNNING)) {
        printf (" R");
    }
    else {
        printf (" Not-R");
    }

    if (this->IsBitSet (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS)) {
        printf (" Not-Acc");
    }
    else {
        printf (" Acc");
    }    

    if (this->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        printf (" Not-L");
    }
    else {
        printf (" L");
    }        

    if (this->IsBitSet (TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {
        printf (" M");
    }
    else {
        printf (" Not-M");
    }    

    printf ("\n");

    this->tcp_client_db_mgr->DisplayClientDb();

    pthread_rwlock_rdlock (&this->connect_db_rwlock);

    for (it = this->connectpendingClients.begin(); it != this->connectpendingClients.end(); ++it) {

        tcp_client = *it;
        tcp_client->Display();
    }

    for (it = this->establishedClient.begin(); it != this->establishedClient.end(); ++it) {

        tcp_client = *it;
        tcp_client->Display();
    }
    pthread_rwlock_unlock (&this->connect_db_rwlock);
}

void 
TcpServerController::ProcessMsgQMsg(TcpServerMsg_t *msg) {

    TcpClient *tcp_client;

    tcp_client = (TcpClient *)msg->data;
    msg->data = NULL;

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_DELETE) {

        this->ProcessClientDelete(tcp_client);
        return;
     }

     if (msg->code &  CTRLR_ACTION_TCP_CLIENT_RECONNECT) {

        uint32_t server_ip_addr = tcp_client->server_ip_addr;
        uint16_t server_port_no = tcp_client->server_port_no;
        this->ProcessClientDelete(tcp_client);
        this->CreateActiveClient(server_ip_addr, server_port_no);
     }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_PROCESS_NEW) {

        this->tcp_client_db_mgr->AddClientToDB(tcp_client);
    }
    
    if (msg->code & CTRLR_ACTION_TCP_CLIENT_MULTIPLEX_LISTEN) {

            assert (!tcp_client->IsStateSet (TCP_CLIENT_STATE_MULTIPLEX_LISTEN));
            this->tcp_client_svc_mgr->ClientFDStartListen(tcp_client);
    }
    
    if (msg->code & CTRLR_ACTION_TCP_CLIENT_MX_TO_MULTITHREADED) {

        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_THREADED)) return;

        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_MULTIPLEX_LISTEN)) {

            this->tcp_client_svc_mgr->ClientFDStopListen(tcp_client);
        }

        this->CreateMultiThreadedClient(tcp_client);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_MULTITHREADED_TO_MX) {

        if (tcp_client->IsStateSet (TCP_CLIENT_STATE_MULTIPLEX_LISTEN)) return;

         if (tcp_client->IsStateSet(TCP_CLIENT_STATE_THREADED)) {

            tcp_client->StopThread();
         }

        this->tcp_client_svc_mgr->ClientFDStartListen(tcp_client);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_CREATE_THREADED) {

            assert (!tcp_client->IsStateSet (TCP_CLIENT_STATE_THREADED));
            this->CreateMultiThreadedClient(tcp_client);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_ACTIVE_CONNECT_SUCCESS) {

        assert (!tcp_client->IsStateSet (TCP_CLIENT_STATE_CONNECT_IN_PROGRESS));
        assert (tcp_client->IsStateSet (TCP_CLIENT_STATE_CONNECTED));
        assert (!tcp_client->IsStateSet (TCP_CLIENT_STATE_PASSIVE_OPENER));
        assert (tcp_client->IsStateSet (TCP_CLIENT_STATE_ACTIVE_OPENER));
        assert (!tcp_client->IsStateSet (TCP_CLIENT_STATE_MULTIPLEX_LISTEN));
        assert (!tcp_client->IsStateSet (TCP_CLIENT_STATE_THREADED));
        pthread_rwlock_wrlock (&this->connect_db_rwlock);
        this->connectpendingClients.remove(tcp_client);
        this->establishedClient.push_back(tcp_client);
        pthread_rwlock_unlock (&this->connect_db_rwlock);
        this->CreateMultiThreadedClient(tcp_client);
    }
}

void 
TcpServerController::MsgQProcessingThreadFn() {

    TcpServerMsg_t *msg;

    while (true) {

        pthread_mutex_lock(&this->msgq_mutex);

        while (this->msgQ.empty()) {
            pthread_cond_wait(&this->msgq_cv, &this->msgq_mutex);
        }

        while (1) {
            if (this->msgQ.empty()) break;
            msg = this->msgQ.front();
            this->msgQ.pop_front();
            this->ProcessMsgQMsg(msg);
            if (msg->zero_sema) {
                sem_post(msg->zero_sema);
            }
            free(msg);
        }

        pthread_mutex_unlock(&this->msgq_mutex);
    }
}

void 
TcpServerController::EnqueMsg (tcp_server_msg_code_t code, void *data, bool block_me) {

        sem_t sem;
        TcpServerMsg_t *msg = (TcpServerMsg_t *)calloc (1, sizeof (TcpServerMsg_t));
        msg->code = code;
        msg->data = data;

        if (block_me) {
            sem_init (&sem, 0, 0);
            msg->zero_sema = &sem;
        }
        else {
            msg->zero_sema = NULL;
        }
        
        pthread_mutex_lock (&this->msgq_mutex);

        if (block_me) {
            this->msgQ.push_front(msg);
        } 
        else {
            this->msgQ.push_back(msg);
        }
        pthread_cond_signal (&this->msgq_cv);
        pthread_mutex_unlock (&this->msgq_mutex);

        if (block_me) {
            sem_wait(&sem);
            sem_destroy(&sem);
        }
}

void 
TcpServerController::CreateActiveClient (uint32_t server_ip_addr, uint16_t server_port_no) {

    TcpClient *tcp_client = new TcpClient ();
    tcp_client->server_ip_addr = server_ip_addr;
    tcp_client->server_port_no = server_port_no;
    tcp_client->ip_addr = this->ip_addr;
    tcp_client->port_no = 0; // Dynamically allocated
    tcp_client->tcp_ctrlr = this;
    sem_init(&tcp_client->wait_for_thread_operation_to_complete, 0, 0);
    tcp_client->SetTcpMsgDemarcar(NULL);
    tcp_client->SetState(TCP_CLIENT_STATE_ACTIVE_OPENER);
    tcp_client->conn.conn_type = tcp_conn_via_connect;
    pthread_rwlock_wrlock (&this->connect_db_rwlock);
    this->connectpendingClients.push_back(tcp_client);
    pthread_rwlock_unlock (&this->connect_db_rwlock);
    tcp_client->Reference();

    if (tcp_client->TryClientConnect(true) == 0) {
        pthread_rwlock_wrlock (&this->connect_db_rwlock);
        this->connectpendingClients.remove(tcp_client);
        this->establishedClient.push_back(tcp_client);
        pthread_rwlock_unlock (&this->connect_db_rwlock);
        assert (!tcp_client->IsStateSet (TCP_CLIENT_STATE_THREADED));
        this->CreateMultiThreadedClient(tcp_client);
    }
}

void
TcpServerController::SetBit(uint32_t bit) {

    this->state_flags |= bit;
}

void
TcpServerController::UnSetBit(uint32_t bit) {

     this->state_flags &= ~bit;
}

bool
TcpServerController::IsBitSet(uint32_t bit) {

    return (this->state_flags & bit);
}

void 
TcpServerController::StopAcceptingNewConnections() {

    if (this->IsBitSet (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS)) return;
    this->SetBit (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    this->tcp_new_conn_acc->Stop();
}

void
TcpServerController::StartAcceptingNewConnections() {

    if (!this->IsBitSet (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS)) return;
    this->UnSetBit (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    this->tcp_new_conn_acc = new TcpNewConnectionAcceptor(this);
    this->tcp_new_conn_acc->StartTcpNewConnectionAcceptorThread();
}

void
TcpServerController::StopClientSvcMgr() {

    if (this->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) return;
    this->tcp_client_svc_mgr->Stop();
    this->SetBit (TCP_SERVER_NOT_LISTENING_CLIENTS);
}

void
TcpServerController::StartClientSvcMgr() {

    if (!this->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) return;
    this->tcp_client_svc_mgr = new TcpClientServiceManager(this);
    this->tcp_client_svc_mgr->StartTcpClientServiceManagerThread();
    this->UnSetBit (TCP_SERVER_NOT_LISTENING_CLIENTS);
}