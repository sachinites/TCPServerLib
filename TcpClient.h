#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_CLIENT_BUFFER_SIZE 512

class TcpClientServiceManager;
class TcpClient {

    private:
        TcpClientServiceManager *svc_mgr;
    public :
        uint32_t ip_addr;
        uint16_t port_no;
        int comm_fd;
        unsigned char recv_buffer[MAX_CLIENT_BUFFER_SIZE];
        pthread_t *client_thread;
        sem_t wait_for_thread_operation_to_complete;
        
        TcpClient(uint32_t ip_addr, uint16_t port_no);
        TcpClient();
        TcpClient (TcpClient *);
        ~TcpClient();
        void Abort();
        void StartThread();
        void StopThread();
        void ClientThreadFunction();
        void SetClientSvcMgr(TcpClientServiceManager *);
};

#endif