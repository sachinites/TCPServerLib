#ifndef __TcpClientServiceManager__
#define __TcpClientServiceManager__

class TcpServerController;
class TcpClient;

class TcpClientServiceManager{

    private:
        std::list<TcpClient *>tcp_client_db;
    public:
        TcpServerController *tcp_ctrlr;
        TcpClientServiceManager(TcpServerController *);
        ~TcpClientServiceManager();

        void StartTcpClientServiceManagerThread();
        void ClientFDStartListen(TcpClient *);
};

#endif