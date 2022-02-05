#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_CLIENT_BUFFER_SIZE 512

class TcpClientServiceManager;
class TcpServer;
class TcpClient {

    private:

    public :
        uint32_t ip_addr;
        uint16_t port_no;
        int comm_fd;
        int ref_count;
        unsigned char recv_buffer[MAX_CLIENT_BUFFER_SIZE];
        pthread_t *client_thread;
        TcpServer *tcp_server;
        sem_t wait_for_thread_operation_to_complete;
        
        TcpClient(uint32_t ip_addr, uint16_t port_no);
        TcpClient();
        TcpClient (TcpClient *);
        ~TcpClient();
        void StartThread();
        void StopThread();
        void ClientThreadFunction();
        void trace();
        void Dereference();
        void Reference();
        void Abort();
};

#endif