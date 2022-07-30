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
        int GetMaxFd();
        int GetMaxFd2();
        pthread_t *client_svc_mgr_thread;
        sem_t wait_for_thread_operation_to_complete;
        sem_t sem0_1, sem0_2;
        pthread_rwlock_t rwlock;
        bool listen_clients; 
        void ForceUnblockSelect();
        void TcpClientMigrate(TcpClient *);
        void Purge();
        void CopyClientFDtoFDSet(fd_set *fdset) ;
    public:
        TcpServerController *tcp_ctrlr;
        TcpClientServiceManager(TcpServerController *);
        ~TcpClientServiceManager();

        void StartTcpClientServiceManagerThread();
        void StartTcpClientServiceManagerThreadInternal();
        void StartTcpClientServiceManagerThreadInternal2();
        void StopTcpClientServiceManagerThread();
        void ClientFDStartListen(TcpClient *);
        void ClientFDStartListen2(TcpClient *); 
        void RemoveClientFromDB(TcpClient *);
        void AddClientToDB(TcpClient *);
        TcpClient* ClientFDStopListen(uint32_t , uint16_t);
        void ClientFDStopListen(TcpClient *);
        TcpClient* LookUpClientDB(uint32_t , uint16_t);
        void Stop();
        void SetListenAllClientsStatus(bool status);
};

#endif 
