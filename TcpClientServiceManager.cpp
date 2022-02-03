#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> // for IPPROTO_TCP
#include <unistd.h>
#include <memory.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "TcpClientServiceManager.h"
#include "TcpServer.h"
#include "TcpClient.h"

TcpClientServiceManager::TcpClientServiceManager(TcpServer *tcp_server) {

    this->udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(this->udp_fd == -1){ 
        printf("UDP Socket Creation Failed\n");
        exit(0);
    }  

    this->max_fd = this->udp_fd;

    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port          = htons(tcp_server->port_no + 1);
    server_addr.sin_addr.s_addr = htonl(this->tcp_server->ip_addr);

    if (bind(this->udp_fd, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1) {
        printf("Error : UDP socket bind failed\n");
        exit(0);
    }

    FD_ZERO(&active_fd_set);
    FD_ZERO(&backup_fd_set);

    sem_init(&bin_semaphore, 0 , 1);
    client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
}

TcpClientServiceManager::~TcpClientServiceManager() {

    assert(this->client_db.empty());
    assert(!this->udp_fd);
    assert(!this->client_svc_mgr_thread);
}

void
TcpClientServiceManager::StartTcpClientServiceManagerThreadInternal() {

    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    this->max_fd = this->udp_fd;
    
    FD_SET(this->udp_fd , &this->backup_fd_set);

    sem_post(&this->wait_for_thread_operation_to_complete);

    while(true) {

        memcpy (&this->active_fd_set, &this->backup_fd_set, sizeof(fd_set));\
        
        printf ("TcpClientServiceManager blocked on select\n");
        select(this->max_fd + 1 , &this->active_fd_set, NULL, NULL, NULL);
        printf ("TcpClientServiceManager unblocked from select\n");

        if (FD_ISSET(this->udp_fd, &this->active_fd_set)) {

            printf ("UDP FD invoked\n");
        }

        for (it = this->client_db.begin(); it != this->client_db.end(); ++it){
                tcp_client = *it;

                if (FD_ISSET(tcp_client->comm_fd, &this->active_fd_set)) {


                }
        }

    } // while ends
}

static void *
 tcp_client_svc_manager_thread_fn(void *arg) {

     TcpClientServiceManager *svc_mgr = 
        (TcpClientServiceManager *)arg;

    svc_mgr->StartTcpClientServiceManagerThreadInternal();
    return NULL;
 }

void
TcpClientServiceManager::StartTcpClientServiceManagerThread() {

    pthread_attr_t attr;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

     pthread_create(this->client_svc_mgr_thread, &attr,  
                            tcp_client_svc_manager_thread_fn, (void *)this);

    sem_wait(&this->wait_for_thread_operation_to_complete);
    printf("TcpClientServiceManagerThread Started\n");
}