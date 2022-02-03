#ifndef __TcpClientServiceManager__
#define __TcpClientServiceManager__

#include <semaphore.h>
#include <pthread.h>
#include <list>

#define MAX_CLIENT_SUPPORTED 127

class TcpServer;
class TcpClient;
class TcpClientServiceManager{

    private:
        int max_fd;
        int udp_fd;
        std::list<TcpClient *>client_db;
        fd_set active_fd_set;
        fd_set backup_fd_set;
        /*A semaphore shared between TcpClientServiceManager thread and
        TcpClientDbManager thread for mutual exclusion */
        sem_t bin_semaphore;
        int GetMaxFd();
        pthread_t *client_svc_mgr_thread;
        sem_t wait_for_thread_operation_to_complete;
    public:
        TcpServer *tcp_server;
        TcpClientServiceManager(TcpServer *);
        ~TcpClientServiceManager();

        void StartTcpClientServiceManagerThread();
        void StartTcpClientServiceManagerThreadInternal();
        void StopTcpClientServiceManagerThread();

        void add_new_client_fd(TcpClient *);
        void remove_client_fd(TcpClient *);
};

#endif 