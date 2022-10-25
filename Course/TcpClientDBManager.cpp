#include <unistd.h>
#include "TcpServerController.h"
#include "TcpClientDBManager.h"
#include "TcpClient.h"

TcpClientDbManager::TcpClientDbManager(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;
    pthread_rwlock_init (&this->rwlock, NULL);
}

TcpClientDbManager::~TcpClientDbManager() {

    pthread_rwlock_destroy(&this->rwlock);
}

void
TcpClientDbManager::StartTcpClientDbMgrInit() {


}

void
TcpClientDbManager::AddClientToDB(TcpClient *tcp_client) {

    pthread_rwlock_wrlock(&this->rwlock);
    this->tcp_client_db.push_back(tcp_client);
    pthread_rwlock_unlock(&this->rwlock);
}

void
TcpClientDbManager::DisplayClientDb() {

    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client;

    pthread_rwlock_rdlock(&this->rwlock);
    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it) {
        tcp_client = *it;
        tcp_client->Display();
    }
    pthread_rwlock_unlock(&this->rwlock);
}

void 
TcpClientDbManager::CopyAllClientsTolist (std::list<TcpClient *> *list) {

    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client;

     pthread_rwlock_rdlock(&this->rwlock);
    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it) {
        tcp_client = *it;
        list->push_back(tcp_client);
    }
    pthread_rwlock_unlock(&this->rwlock);
}

void TcpClientDbManager::Purge()
{
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client, *next_tcp_client;

    pthread_rwlock_rdlock(&this->rwlock);

    for (it = this->tcp_client_db.begin(), tcp_client = *it;
         it != this->tcp_client_db.end();
         tcp_client = next_tcp_client)
    {

        next_tcp_client = *(++it);

        this->tcp_client_db.remove(tcp_client);
        tcp_client->Abort();
    }
     pthread_rwlock_unlock(&this->rwlock);
}