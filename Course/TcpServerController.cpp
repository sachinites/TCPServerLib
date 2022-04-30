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
    printf ("Listening on : [%s, %d]\n", network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);

    this->tcp_client_db_mgr->DisplayClientDb();
}
