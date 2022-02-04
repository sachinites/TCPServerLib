#include <assert.h>
#include "TcpServer.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClientDbManager.h"
#include "TcpClientServiceManager.h"
#include "bitsop.h"

TcpServer::TcpServer(uint16_t ip_addr,  uint16_t port_no, std::string name) {

    this->ip_addr = ip_addr;
    this->port_no = port_no;
    this->name = name;
    this->tcp_new_conn_acc = new TcpNewConnectionAcceptor(this);
    this->tcp_client_db_mgr = new TcpClientDbManager(this);
    this->tcp_client_svc_mgr = new TcpClientServiceManager(this);
    SET_BIT(this->state_flags, TCP_SERVER_INITIALIZED);
}

TcpServer::~TcpServer() {
    
    assert(!this->tcp_new_conn_acc);
    assert(!this->tcp_client_db_mgr);
    assert(!this->tcp_client_svc_mgr);
}

void
TcpServer::Start() {

    assert(this->tcp_new_conn_acc);
    assert(this->tcp_client_db_mgr);
    assert(this->tcp_client_svc_mgr);

    this->tcp_new_conn_acc->StartTcpNewConnectionAcceptorThread();
    this->tcp_client_db_mgr->StartClientDbManagerThread();
    this->tcp_client_svc_mgr->StartTcpClientServiceManagerThread();

    SET_BIT(this->state_flags, TCP_SERVER_RUNNING);
    printf ("Tcp Server is Up and Running\nOk.\n");
}

void
TcpServer::Stop() {

    this->tcp_new_conn_acc->Stop();
    this->tcp_new_conn_acc = NULL;

    this->tcp_client_db_mgr->Stop();
    this->tcp_client_db_mgr = NULL;

    this->tcp_client_svc_mgr->Stop();
    this->tcp_client_svc_mgr = NULL;
    UNSET_BIT32(this->state_flags, TCP_SERVER_RUNNING);

    delete this;
}

uint32_t
TcpServer::GetStateFlags() {

    return this->state_flags;
}

void
TcpServer::SetAcceptNewConnectionStatus(bool status) {

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
TcpServer::SetClientCreationMode(bool status) {

    if (status &&
        IS_BIT_SET(this->state_flags, TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {
            return;
     }

     if (!status && 
          !IS_BIT_SET(this->state_flags, TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {
              return;
    }

    this->tcp_client_db_mgr ->SetClientCreationMode(status);

    if (status) {
        SET_BIT(this->state_flags, TCP_SERVER_CREATE_MULTI_THREADED_CLIENT);
    }
    else {
        UNSET_BIT32(this->state_flags, TCP_SERVER_CREATE_MULTI_THREADED_CLIENT);
    }
}

void
TcpServer::SetListenAllClientsStatus(bool status) {

    if (status &&
        !IS_BIT_SET(this->state_flags, TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        return;
    }

    if (!status &&
        IS_BIT_SET(this->state_flags, TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        return;
    }

    this->tcp_client_svc_mgr->SetListenAllClientsStatus(status);

    if (status) {
        UNSET_BIT32(this->state_flags, TCP_SERVER_NOT_LISTENING_CLIENTS);
    }
    else {
        SET_BIT(this->state_flags, TCP_SERVER_NOT_LISTENING_CLIENTS);
    }
}

void
TcpServer::SetServerNotifCallbacks(
                void (*client_connected)(const TcpClient *), 
                void (*client_disconnected)(const TcpClient *),
                void (*client_msg_recvd)(const TcpClient *, unsigned char *, uint16_t),
                void (*client_ka_pending)(const TcpClient *) ) {

    /* Should be called before invoking Start() on TCP Server */
    assert (this->state_flags == TCP_SERVER_INITIALIZED);
    this->tcp_new_conn_acc->SetClientConnectCbk(client_connected);
    this->tcp_client_db_mgr->SetClientKAPending(client_ka_pending);
    this->tcp_client_svc_mgr->SetClientMsgRecvCbk(client_msg_recvd);
    this->tcp_client_svc_mgr->SetClientDisconnectCbk(client_disconnected);
}

void
TcpServer::CreateNewClientRequestSubmission(TcpClient *tcp_client) {

    this->tcp_client_db_mgr->EnqueClientProcessingRequest (
                tcp_client, TCP_DB_MGR_NEW_CLIENT_CREATE);
}

void
TcpServer::CreateDeleteClientRequestSubmission(TcpClient *tcp_client) {

    this->tcp_client_db_mgr->EnqueClientProcessingRequest(
            tcp_client, TCP_DB_MGR_DELETE_EXISTING_CLIENT);
}

void
TcpServer::ClientFDStartListen(TcpClient *tcp_client) {

    this->tcp_client_svc_mgr->ClientFDStartListen(tcp_client);
}

TcpClient* 
TcpServer::ClientFDStopListen(uint32_t ip_addr, uint16_t port_no) {

    return this->tcp_client_svc_mgr->ClientFDStopListen(ip_addr, port_no);
}

void
TcpServer::StopListeningAllClients() {

    this->tcp_client_svc_mgr->StopListeningAllClients();
}

void 
TcpServer::AbortClient(uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client = this->tcp_client_svc_mgr->ClientFDStopListen(ip_addr, port_no);
    if (!tcp_client) {
        printf ("Error : Client do not exist\n");
        return;
    }
    this->CreateDeleteClientRequestSubmission(tcp_client);
}