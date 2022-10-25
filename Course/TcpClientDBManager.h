#ifndef __TcpClientDbManager__
#define __TcpClientDbManager__

#include <list>
#include <pthread.h>

class TcpClient;
class TcpServerController;

class TcpClientDbManager {

    private:
    std::list<TcpClient *> tcp_client_db;
    pthread_rwlock_t rwlock;

    public:
    TcpServerController *tcp_ctrlr;

    TcpClientDbManager(TcpServerController *);
    ~TcpClientDbManager();

    void StartTcpClientDbMgrInit();
    void AddClientToDB(TcpClient *tcp_client);\
    void CopyAllClientsTolist (std::list<TcpClient *> *list);
    void DisplayClientDb();
    void Purge();
};

#endif