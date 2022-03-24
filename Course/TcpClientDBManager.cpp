#include "TcpServerController.h"
#include "TcpClientDBManager.h"
#include "TcpClient.h"

TcpClientDbManager::TcpClientDbManager(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;
}

TcpClientDbManager::~TcpClientDbManager() {


}

void
TcpClientDbManager::StartTcpClientDbMgrInit() {


}

void
TcpClientDbManager::AddClientToDB(TcpClient *tcp_client) {

    this->tcp_client_db.push_back(tcp_client);
}