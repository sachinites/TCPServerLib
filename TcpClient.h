#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include "TcpConn.h"

#define MAX_CLIENT_BUFFER_SIZE 1024

class TcpClientServiceManager;
class TcpServerController;
class TcpMsgDemarcar;

class TcpClient {

    private:
        pthread_rwlock_t rwlock;
        void Abort();
        ~TcpClient();
        
    public :
        uint32_t ip_addr;
        uint16_t port_no;
        int comm_fd;
        int ref_count;
        unsigned char recv_buffer[MAX_CLIENT_BUFFER_SIZE];
        pthread_t *client_thread;
        TcpServerController *tcp_ctrlr;
        sem_t wait_for_thread_operation_to_complete;
        TcpMsgDemarcar *msgd;
        TcpConn conn;

        TcpClient(uint32_t, uint16_t);
        TcpClient();
        TcpClient (TcpClient *);
        
	    int SendMsg(char *, uint32_t);
        void StartThread();
        void StopThread();
        void ClientThreadFunction();
        void trace();
        TcpClient * Dereference();
        void Reference();
        void Display();
        void SetTcpMsgDemarcar(TcpMsgDemarcar  *);
        void SetConnectionType(tcp_connection_type_t);
};

#endif
