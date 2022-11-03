#ifndef __TcpClientServiceManager__
#define __TcpClientServiceManager__
#include <sys/select.h>

class TcpServerController;
class TcpClient;

class TcpClientServiceManager{

    private:
        int max_fd;
        fd_set active_fd_set;
        fd_set backup_fd_set;
        pthread_t *client_svc_mgr_thread;
        std::list<TcpClient *>tcp_client_db;
        int GetMaxFd();
        void CopyClientFDtoFDSet(fd_set *);

    public:
        TcpServerController *tcp_ctrlr;
        TcpClientServiceManager(TcpServerController *);
        ~TcpClientServiceManager();

        void StartTcpClientServiceManagerThread();
        void StartTcpClientServiceManagerThreadInternal();
        void ClientFDStartListen(TcpClient *);
        void StopTcpClientServiceManagerThread();
        void AddClientToDB(TcpClient *);
        void Stop();
};

#endif
