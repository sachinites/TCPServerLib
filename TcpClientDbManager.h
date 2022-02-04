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
    
        std::list<TcpClient *> tcp_client_db;
        sem_t wait_for_thread_operation_to_complete;
        sem_t sem0_1, sem0_2;
        
        pthread_t *client_db_mgr_thread;    

        std::list<TcpClientDbRequest *> request_q;
        pthread_mutex_t request_q_mutex;
        pthread_cond_t request_q_cv;

        void ProcessRequest(TcpClientDbRequest *);
        void (*client_disconnected)(const TcpClient *);
        void (*client_ka_pending)(const TcpClient *);

        /* Client DB mgmt functions */
        void AddNewClient(TcpClient *);
        void DeleteClient(TcpClient *);
        void DeleteAllClients();
        void UpdateClient(TcpClient *);
        TcpClient * LookUpClientDB(uint32_t, uint16_t);


    public:
        TcpServer *tcp_server;  /* Back pointer to owning Server */
        TcpClientDbManager(TcpServer *);
        ~TcpClientDbManager();

        void StartClientDbManagerThread();
        void StartClientDbManagerThreadInternal();
        void StopClientDbManagerThread();

        void EnqueClientProcessingRequest
            (TcpClient *tcp_client, ClientDBRequestOpnCode code);

        void Stop();

        void
        SetClientDisconnectCbk(void (*client_disconnected)(const TcpClient *));
        void
        SetClientKAPending(void (*client_ka_pending)(const TcpClient *));

};

#endif 