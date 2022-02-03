#ifndef __TcpClientDbManager__
#define __TcpClientDbManager__

#include <list>
#include <semaphore.h>
#include <stdint.h>
#include <pthread.h>
#include <vector>

class TcpServer;
class TcpClient;

typedef enum {

    TCP_DB_MGR_NEW_CLIENT_CREATE,
    TCP_DB_MGR_DELETE_EXISTING_CLIENT,
    TCP_DB_MGR_UPDATE_CLIENT,
    TCP_DB_MGR_DELETE_ALL_CLIENTS
}ClientDBRequestOpnCode;

class TcpClientDbRequest{

    public:
    TcpClient *tcp_client;
    ClientDBRequestOpnCode Code;
    TcpClientDbRequest(){}
    ~TcpClientDbRequest(){}
};

class TcpClientDbManager {

    private:
    /* A semaphore shared between TcpNewConnectionAcceptor thread and
        TcpClientDbManager thread for mutual exclusion */
        sem_t *shared_binary_semaphore1;
    /* A semaphore shared between TcpClientServiceManager thread and
        TcpClientDbManager thread for mutual exclusion */
        sem_t *shared_binary_semaphore2;

        std::list<TcpClient *> tcp_client_db;

        sem_t wait_for_thread_operation_to_complete;

        pthread_t *client_db_mgr_thread;

        std::vector<TcpClientDbRequest *> request_q;
        pthread_mutex_t request_q_mutex;
        pthread_cond_t request_q_cv;

        void ProcessRequest(TcpClientDbRequest *);

    public:
        TcpServer *tcp_server;  /* Back pointer to owning Server */
        TcpClientDbManager(TcpServer *);
        ~TcpClientDbManager();

        void StartClientDbManagerThread();
        void StartClientDbManagerThreadInternal();
        void StopClientDbManagerThread();

        void NewClientCreationRequest
            (TcpClient *tcp_client_template, ClientDBRequestOpnCode code);
};

#endif 