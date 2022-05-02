#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include "TcpServerController.h"
#include "TcpClientServiceManager.h"
#include "TcpClient.h"

#define TCP_CLIENT_RECV_BUFFER_SIZE 1024
unsigned char client_recv_buffer[TCP_CLIENT_RECV_BUFFER_SIZE];
                                                        
TcpClientServiceManager::TcpClientServiceManager(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;
    this->max_fd = 0;
    FD_ZERO(&this->active_fd_set);
    FD_ZERO(&this->backup_fd_set);
    client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
}

TcpClientServiceManager::~TcpClientServiceManager() {
    
}

void
TcpClientServiceManager::StartTcpClientServiceManagerThreadInternal() {

    /* Invoke select system call on all Clients present in Client DB */
    int rcv_bytes;
    TcpClient *tcp_client, *next_tcp_client;
    struct sockaddr_in client_addr;
    std::list<TcpClient *>::iterator it;

    socklen_t addr_len = sizeof(client_addr);

    while(true) {

        memcpy(&this->active_fd_set, &this->backup_fd_set, sizeof(fd_set)); 
        select(this->max_fd + 1, &this->active_fd_set, 0, 0, 0);

        for (it = this->tcp_client_db.begin(), tcp_client = *it;
                it!= this->tcp_client_db.end(); 
                tcp_client = next_tcp_client) {

                next_tcp_client = *(++it);

                if (FD_ISSET(tcp_client->comm_fd, &this->active_fd_set)) {

                    rcv_bytes = recvfrom(tcp_client->comm_fd,
                                                        client_recv_buffer, 
                                                        TCP_CLIENT_RECV_BUFFER_SIZE,
                                                        0,
                                                        (struct sockaddr *)&client_addr, &addr_len);

                    if (this->tcp_ctrlr->client_msg_recvd) {
                        this->tcp_ctrlr->client_msg_recvd(this->tcp_ctrlr, tcp_client,
                                        client_recv_buffer, rcv_bytes);
                    }
                }
        }
    }
}

void *
tcp_client_svc_manager_thread_fn(void *arg) {

    TcpClientServiceManager *svc_mgr = 
        (TcpClientServiceManager *)arg;

    svc_mgr->StartTcpClientServiceManagerThreadInternal();
    return NULL;
}

void
TcpClientServiceManager::StartTcpClientServiceManagerThread() {

    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_create(this->client_svc_mgr_thread, &attr, 
                            tcp_client_svc_manager_thread_fn, (void *)this);
    printf("Service started : TcpClientServiceManagerThread\n");
}

void TcpClientServiceManager::ClientFDStartListen(TcpClient *tcp_client) {

}