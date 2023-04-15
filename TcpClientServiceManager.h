#ifndef __TcpClientServiceManager__
#define __TcpClientServiceManager__

#include <semaphore.h>
#include <pthread.h>
#include <list>

#define MAX_CLIENT_SUPPORTED 127

class TcpServerController;
class TcpClient;
class TcpClientServiceManager{

    private:
        int max_fd;
        int udp_fd;
        std::list<TcpClient *>tcp_client_db;
        fd_set active_fd_set;
        fd_set backup_fd_set;
        int GetMaxFdSimple();
        int GetMaxFdAdv();
        pthread_t *client_svc_mgr_thread;
        sem_t wait_for_thread_operation_to_complete;
        sem_t sem0_1, sem0_2;
        pthread_rwlock_t rwlock;
        void ForceUnblockSelect();
        void TcpClientMigrate(TcpClient *);
        void Purge();
        void CopyClientFDtoFDSet(fd_set *fdset) ;
    public:
        TcpServerController *tcp_ctrlr;
        TcpClientServiceManager(TcpServerController *);
        ~TcpClientServiceManager();

        void StartTcpClientServiceManagerThread();
        void StartTcpClientServiceManagerThreadInternalSimple();
        void StartTcpClientServiceManagerThreadInternal2();
        void StopTcpClientServiceManagerThread();
        void ClientFDStartListenSimple(TcpClient *);
        void ClientFDStartListenAdv(TcpClient *); 
        void RemoveClientFromDB(TcpClient *);
        void AddClientToDB(TcpClient *);
        TcpClient* ClientFDStopListenAdv(uint32_t , uint16_t);
        void ClientFDStopListenAdv(TcpClient *);
        void ClientFDStopListenSimple(TcpClient *);
        TcpClient* LookUpClientDB(uint32_t , uint16_t);
        void Stop();
};

#endif 
