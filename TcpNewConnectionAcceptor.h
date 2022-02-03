#ifndef __TcpNewConnectionAcceptor__
#define __TcpNewConnectionAcceptor__

#include <pthread.h>
#include <semaphore.h>

class TcpServer;
class TcpNewConnectionAcceptor {

    private:
        int accept_fd;
        pthread_t *accept_new_conn_thread;
        sem_t wait_for_thread_operation_to_complete;

        /* A semaphore shared between TcpNewConnectionAcceptor thread and
        TcpClientDbManager thread for mutual exclusion */
        sem_t *shared_binary_semaphore1;

    public:
        TcpServer *tcp_server;  /* Back pointer to owning Server */

        TcpNewConnectionAcceptor(TcpServer *);
        ~TcpNewConnectionAcceptor();

        void StartTcpNewConnectionAcceptorThread();
        void StopTcpNewConnectionAcceptorThread();

        void SetSharedSemaphore(sem_t *);

        void StartTcpNewConnectionAcceptorThreadInternal();
};

#endif 