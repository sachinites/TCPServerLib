#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "TcpServerController.h"
#include "TcpClientServiceManager.h"
#include "TcpClient.h"
#include "TcpMsgDemarcar.h"

unsigned char client_recv_buffer[MAX_CLIENT_BUFFER_SIZE];
                                                        
TcpClientServiceManager::TcpClientServiceManager(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;
    this->max_fd = 0;
    FD_ZERO(&this->active_fd_set);
    FD_ZERO(&this->backup_fd_set);
    client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
}

TcpClientServiceManager::~TcpClientServiceManager() {
    
}

int
TcpClientServiceManager::GetMaxFd() {

    int max_fd_lcl = 0;

    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it) {

        tcp_client = *it;
        if (tcp_client->comm_fd > max_fd_lcl ) {
            max_fd_lcl = tcp_client->comm_fd;
        }
    }
    return max_fd_lcl;
}

void
TcpClientServiceManager::CopyClientFDtoFDSet(fd_set *fdset) {

    TcpClient *tcp_client;
     std::list<TcpClient *>::iterator it;

      for (it = this->tcp_client_db.begin(); 
            it != this->tcp_client_db.end();
            ++it) {

        tcp_client = *it;
        FD_SET(tcp_client->comm_fd, fdset);
    }
}

void
TcpClientServiceManager::AddClientToDB(TcpClient *tcp_client){

     this->tcp_client_db.push_back(tcp_client);
}

void
TcpClientServiceManager::StartTcpClientServiceManagerThreadInternal() {

    /* Invoke select system call on all Clients present in Client DB */
    int rcv_bytes;
    TcpClient *tcp_client, *next_tcp_client;
    struct sockaddr_in client_addr;
    std::list<TcpClient *>::iterator it;

    socklen_t addr_len = sizeof(client_addr);

    if (this->tcp_ctrlr->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENTS)) {

        this->tcp_ctrlr->CopyAllClientsTolist(&this->tcp_client_db);
    }

    this->max_fd = this->GetMaxFd();

    FD_ZERO(&this->backup_fd_set);
    this->CopyClientFDtoFDSet(&this->backup_fd_set);

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
                                                        MAX_CLIENT_BUFFER_SIZE,
                                                        0,
                                                        (struct sockaddr *)&client_addr, &addr_len);

                    if (rcv_bytes == 0 ) {
                        printf ("error no = %d\n", errno);
                        sleep(1);
                    }

                    if (tcp_client->msgd) {
                        tcp_client->msgd->ProcessMsg (tcp_client, client_recv_buffer, rcv_bytes);
                    }
                    else if  (this->tcp_ctrlr->client_msg_recvd) {
                        this->tcp_ctrlr->client_msg_recvd(this->tcp_ctrlr, tcp_client,
                                        client_recv_buffer, rcv_bytes);
                    }         
                }
                memset (client_recv_buffer, 0, rcv_bytes);
        }
    }
}

void *
tcp_client_svc_manager_thread_fn(void *arg) {

    TcpClientServiceManager *svc_mgr = 
        (TcpClientServiceManager *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    svc_mgr->StartTcpClientServiceManagerThreadInternal();
    return NULL;
}

void
TcpClientServiceManager::StartTcpClientServiceManagerThread() {

    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_create(this->client_svc_mgr_thread, &attr, 
                            tcp_client_svc_manager_thread_fn, (void *)this);
    sleep(1); // Why this sleep is required !!
    printf("Service started : TcpClientServiceManagerThread\n");
}

void
TcpClientServiceManager::StopTcpClientServiceManagerThread() {

    pthread_cancel(*this->client_svc_mgr_thread);
    pthread_join(*this->client_svc_mgr_thread, NULL);
    free(this->client_svc_mgr_thread);
    this->client_svc_mgr_thread = NULL;
}


void TcpClientServiceManager::ClientFDStartListen(TcpClient *tcp_client) {

    this->StopTcpClientServiceManagerThread();
    printf ("Client Svc Mgr Thread is cancelled\n");

    this->AddClientToDB(tcp_client);
    
    this->client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    this->StartTcpClientServiceManagerThread();
}

void
TcpClientServiceManager::Stop() {

    this->StopTcpClientServiceManagerThread();

    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client, *next_tcp_client;

    /* This fn assumes that Svc mgr thread is already cancelled,
        hence no need to lock anything */
    assert(this->client_svc_mgr_thread == NULL);

    for (it = this->tcp_client_db.begin(), tcp_client = *it;
         it != this->tcp_client_db.end();
         tcp_client = next_tcp_client)
    {
        next_tcp_client = *(++it);
        this->tcp_client_db.remove(tcp_client);
        /* Note that these clients are still present in ClientDBMgr DB */
    }

    delete this;
}
