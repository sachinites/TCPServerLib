#include <assert.h>
#include "TcpClientDbManager.h"
#include "TcpServerController.h"
#include "TcpClient.h"

TcpClientDbManager::TcpClientDbManager(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;
    pthread_rwlock_init(&this->rwlock, NULL);
}

TcpClientDbManager::~TcpClientDbManager() {

    assert(this->tcp_client_db.empty());
}

TcpClient * 
TcpClientDbManager::LookUpClientDB(uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it){

        tcp_client = *it;
        if (tcp_client->ip_addr == ip_addr &&
                tcp_client->port_no == port_no) return tcp_client;
    }
    return NULL;
}

TcpClient * 
TcpClientDbManager::LookUpClientDB_ThreadSafe(
        uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client = NULL;

    pthread_rwlock_rdlock(&this->rwlock);
    tcp_client = this->LookUpClientDB(ip_addr, port_no);
     pthread_rwlock_unlock(&this->rwlock);
    return tcp_client;
}

void
TcpClientDbManager::AddClientToDB(TcpClient *tcp_client) {

    pthread_rwlock_wrlock(&this->rwlock);
    assert(!this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));
    this->tcp_client_db.push_back(tcp_client);
    tcp_client->Reference();
    pthread_rwlock_unlock(&this->rwlock);
}

void
TcpClientDbManager::RemoveClientFromDB(TcpClient *tcp_client) {
    
    pthread_rwlock_wrlock(&this->rwlock);
    this->tcp_client_db.remove(tcp_client);
    tcp_client->Dereference();
    pthread_rwlock_unlock(&this->rwlock);
}

TcpClient *
TcpClientDbManager::RemoveClientFromDB(uint32_t ip_addr, uint16_t port_no) {
    
    TcpClient *tcp_client;

    pthread_rwlock_wrlock(&this->rwlock);

    tcp_client = this->LookUpClientDB(ip_addr, port_no);

    if (!tcp_client) {
        pthread_rwlock_unlock(&this->rwlock);
        return NULL;
    }

    this->tcp_client_db.remove(tcp_client);
    tcp_client = tcp_client->Dereference();

    pthread_rwlock_unlock(&this->rwlock);
    return tcp_client;
}

void 
TcpClientDbManager::UpdateClient(TcpClient *tcp_client) {
    
}

void
TcpClientDbManager::Purge() {

    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client, *next_tcp_client;

    pthread_rwlock_wrlock(&this->rwlock);

    for (it = this->tcp_client_db.begin(), tcp_client = *it;
         it != this->tcp_client_db.end();
         tcp_client = next_tcp_client) {

        next_tcp_client = *(++it);

      if (tcp_client->client_thread) tcp_client->StopThread();

       this->tcp_client_db.remove(tcp_client);
       tcp_client->Dereference();
    }
    pthread_rwlock_unlock(&this->rwlock);
}

void
TcpClientDbManager::DisplayClientDb() {

    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client;

    pthread_rwlock_rdlock(&this->rwlock);

    for (it = this->tcp_client_db.begin(); it !=  this->tcp_client_db.end(); ++it)
    {
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
        tcp_client->Reference();
    }
    pthread_rwlock_unlock(&this->rwlock);
}