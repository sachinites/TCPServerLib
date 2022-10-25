#include <stdio.h>
#include <assert.h>
#include "TcpServerController.h"
#include "TcpClientDBManager.h"
#include "TcpClientServiceManager.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClient.h"
#include "network_utils.h"

TcpServerController::TcpServerController(
        std::string ip_addr, uint16_t port_no, std::string name) {

    this->ip_addr = network_covert_ip_p_to_n(ip_addr.c_str());
    this->port_no = port_no;
    this->name = name;

    this->tcp_new_conn_acc = new TcpNewConnectionAcceptor(this);
    this->tcp_client_db_mgr = new TcpClientDbManager(this);
    this->tcp_client_svc_mgr = new TcpClientServiceManager(this);

    this->SetBit (TCP_SERVER_INITIALIZED);
}

TcpServerController::~TcpServerController() {

    assert(!this->tcp_new_conn_acc);
    assert(!this->tcp_client_db_mgr);
    assert(!this->tcp_client_svc_mgr);
}

void
TcpServerController::Start() {

    /* Start the CRS thread
        Start the DRS thread
        initialize the DBMS */

    this->tcp_new_conn_acc->StartTcpNewConnectionAcceptorThread();
    this->tcp_client_svc_mgr->StartTcpClientServiceManagerThread();
    this->tcp_client_db_mgr->StartTcpClientDbMgrInit();

    printf ("Tcp Server is Up and Running [%s, %d]\nOk.\n",
        network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);
    
    this->SetBit (TCP_SERVER_RUNNING);
}

void
TcpServerController::ProcessNewClient(TcpClient *tcp_client) {

    this->tcp_client_db_mgr->AddClientToDB(tcp_client);
    this->tcp_client_svc_mgr->ClientFDStartListen(tcp_client);
}

 void TcpServerController::SetServerNotifCallbacks(
                void (*client_connected)(const TcpServerController *, const TcpClient *),
                void (*client_disconnected)(const TcpServerController *, const TcpClient *),
                void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t)) {

        this->client_connected = client_connected;
        this->client_disconnected = client_disconnected;
        this->client_msg_recvd = client_msg_recvd;
}

void
TcpServerController::Display() {

    printf ("Server Name : %s\n", this->name.c_str());

    if (!this->IsBitSet (TCP_SERVER_RUNNING)) {
        printf ("Tcp Server Not Running\n");
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
TcpServerController::StopConnectionAcceptingSvc() {

    if (this->IsBitSet (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS)) return;
    this->SetBit (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    this->tcp_new_conn_acc->Stop();
    this->tcp_new_conn_acc = NULL;
}

void
TcpServerController::StartConnectionAcceptingSvc() {

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
    this->tcp_client_svc_mgr = NULL;
}

void
TcpServerController::StartClientSvcMgr() {

    if (!this->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) return;
    this->tcp_client_svc_mgr = new TcpClientServiceManager(this);
    this->tcp_client_svc_mgr->StartTcpClientServiceManagerThread();
    this->UnSetBit (TCP_SERVER_NOT_LISTENING_CLIENTS);
}

void 
TcpServerController::CopyAllClientsTolist (std::list<TcpClient *> *list) {

    this->tcp_client_db_mgr->CopyAllClientsTolist(list);
}

void
TcpServerController::Stop() {
 
    TcpClient *tcp_client;
 
    if (this->tcp_new_conn_acc) {
        this->StopConnectionAcceptingSvc();
        this->SetBit (TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    }
 
    if (this->tcp_client_svc_mgr) {
        this->StopClientSvcMgr();
        this->SetBit (TCP_SERVER_NOT_LISTENING_CLIENTS);
    }
 
    /* Stopping the above two services first ensures that
        now no thread is alive which could add tcpclient back into
        DB */
    this->tcp_client_db_mgr->Purge();
    delete this->tcp_client_db_mgr;
    this->tcp_client_db_mgr = NULL;
 
    this->UnSetBit(TCP_SERVER_RUNNING);
    delete this;
}