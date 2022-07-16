#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include "TcpConn.h"

#define MAX_CLIENT_BUFFER_SIZE 1024

/* TCP Client States */
#define TCP_CLIENT_STATE_CONNECT_IN_PROGRESS  1
#define TCP_CLIENT_STATE_CONNECTED    2
#define TCP_CLIENT_STATE_PASSIVE_OPENER    8
#define TCP_CLIENT_STATE_ACTIVE_OPENER 16
#define TCP_CLIENT_STATE_KA_BASED 32
#define TCP_CLIENT_STATE_KA_EXPIRED   64
#define TCP_CLIENT_STATE_MULTIPLEX_LISTEN 128
#define TCP_CLIENT_STATE_THREADED 256

typedef uint32_t client_state_bit;

class TcpClientServiceManager;
class TcpServerController;
class TcpMsgDemarcar;

class TcpClient {

    private:
        pthread_rwlock_t rwlock;
        void Abort();
        ~TcpClient();
        uint32_t state_flags;

    public :
        uint32_t ip_addr;
        uint16_t port_no;
        uint32_t server_ip_addr;
        uint16_t server_port_no;
        int comm_fd;
        int ref_count;
        unsigned char recv_buffer[MAX_CLIENT_BUFFER_SIZE];
        pthread_t *client_thread;
        pthread_t *active_connect_thread;
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
        void StopConnectorThread ();
        void ClientThreadFunction();
        TcpClient * Dereference();
        void Reference();
        void Display();
        void SetTcpMsgDemarcar(TcpMsgDemarcar  *);
        void SetConnectionType(tcp_connection_type_t);
        int TryClientConnect (bool) ;
        void SetState(client_state_bit flag_bit);
        void UnSetState(client_state_bit flag_bit);
        bool IsStateSet (client_state_bit flag_bit);
};

#endif
