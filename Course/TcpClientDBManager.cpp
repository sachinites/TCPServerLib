#include "TcpServerController.h"
#include "TcpClientDBManager.h"

TcpClientDbManager::TcpClientDbManager(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;
}

TcpClientDbManager::~TcpClientDbManager() {


}

void
TcpClientDbManager::StartTcpClientDbMgrInit() {


}