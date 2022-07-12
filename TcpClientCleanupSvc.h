#ifndef __TCP_CLIENT_CLEANUP_SVC__
#define __TCP_CLIENT_CLEANUP_SVC__

#include <pthread.h>
#include <list>

class TcpServerController;
class TcpClient;

class TcpClientCleanupSvc
{
    private:
    pthread_t client_cleanup_thread;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
    std::list<TcpClient *> cleanup_lst;
    TcpServerController *tcp_ctrlr;
    void StopTcpServerClientCleanupThread ();

    public:
    TcpClientCleanupSvc(TcpServerController *);
    ~TcpClientCleanupSvc();

    void StartTcpServerClientCleanupThread ();
    void Stop();
    void EnqueueClientDeletionRequest(TcpClient *);
    void TcpServerClientCleanupThreadInternal();
};

#endif