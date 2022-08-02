#ifndef __TcpNewConnectionAcceptor__
#define __TcpNewConnectionAcceptor__

#include <pthread.h>

class TcpServerController;

/* New connenctions are accepted using 'accept()' sys call
    We need to create a 'accept_fd' using socket()
*/
class TcpNewConnectionAcceptor {

    private:
        int accept_fd;
        pthread_t *accept_new_conn_thread;

    public:
        TcpServerController *tcp_ctrlr;

        TcpNewConnectionAcceptor(TcpServerController *);
        ~TcpNewConnectionAcceptor();

        void StartTcpNewConnectionAcceptorThread();
        void StartTcpNewConnectionAcceptorThreadInternal();

        void Stop();
        void StopTcpNewConnectionAcceptorThread();
};


#endif