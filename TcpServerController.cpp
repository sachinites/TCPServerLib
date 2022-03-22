#include <assert.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "TcpServerController.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClientDbManager.h"
#include "TcpClientServiceManager.h"
#include "bitsop.h"
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
    SET_BIT(this->state_flags, TCP_SERVER_INITIALIZED);
}

TcpServerController::~TcpServerController() {
    
    assert(!this->tcp_new_conn_acc);
    assert(!this->tcp_client_db_mgr);
    assert(!this->tcp_client_svc_mgr);
    printf ("TcpServerController %s Stopped\n", this->name.c_str());
}

void
TcpServerController::Start() {

    assert(this->tcp_new_conn_acc);
    assert(this->tcp_client_db_mgr);
    assert(this->tcp_client_svc_mgr);

    this->tcp_new_conn_acc->StartTcpNewConnectionAcceptorThread();
    this->tcp_client_svc_mgr->StartTcpClientServiceManagerThread();

    SET_BIT(this->state_flags, TCP_SERVER_RUNNING);

    printf ("Tcp Server is Up and Running [%s, %d]\nOk.\n", 
        network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);
}

void
TcpServerController::Stop() {

    this->tcp_new_conn_acc->Stop();
    this->tcp_new_conn_acc = NULL;

    this->tcp_client_svc_mgr->Stop();
    this->tcp_client_svc_mgr = NULL;

    /* Stopping the above two services first ensures that
        now no thread is alive which could add tcpclient back into
        DB */
    this->tcp_client_db_mgr->Purge();
    delete this->tcp_client_db_mgr;
    this->tcp_client_db_mgr = NULL;

    UNSET_BIT32(this->state_flags, TCP_SERVER_RUNNING);
    delete this;
}

uint32_t
TcpServerController::GetStateFlags() {

    return this->state_flags;
}

void
TcpServerController::SetAcceptNewConnectionStatus(bool status) {

    if (status &&
        !IS_BIT_SET(this->state_flags, TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS)) {
            return;
     }

     if (!status && 
          IS_BIT_SET(this->state_flags, TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS)) {
              return;
    }

    this->tcp_new_conn_acc->SetAcceptNewConnectionStatus(status);

    if (status) {
        UNSET_BIT32(this->state_flags, TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    }
    else {
        SET_BIT(this->state_flags, TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    }
}

void
TcpServerController::SetClientCreationMode(bool status) {

    if (status &&
        IS_BIT_SET(this->state_flags, TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {
            return;
     }

     if (!status && 
          !IS_BIT_SET(this->state_flags, TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {
              return;
    }

     if (status) {
        SET_BIT(this->state_flags, TCP_SERVER_CREATE_MULTI_THREADED_CLIENT);
    }
    else {
        UNSET_BIT32(this->state_flags, TCP_SERVER_CREATE_MULTI_THREADED_CLIENT);
    }
}

void
TcpServerController::SetServerNotifCallbacks(
                void (*client_connected)(const TcpClient *), 
                void (*client_disconnected)(const TcpClient *),
                void (*client_msg_recvd)(const TcpClient *, unsigned char *, uint16_t),
                void (*client_ka_pending)(const TcpClient *) ) {

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

    if (IS_BIT_SET(this->state_flags , TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {

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
    
    tcp_client->Abort();
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
