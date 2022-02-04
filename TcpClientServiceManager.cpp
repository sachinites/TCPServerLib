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

    this->tcp_server = tcp_server;

    this->udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(this->udp_fd < 0){ 
        printf("UDP Socket Creation Failed, error = %d\n", errno);
        exit(0);
    }  

    this->max_fd = this->udp_fd;

    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port          = htons(tcp_server->port_no + 1);
    server_addr.sin_addr.s_addr = htonl(this->tcp_server->ip_addr);

    if (bind(this->udp_fd, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1) {
        printf("Error : UDP socket bind failed, error = %d\n", errno);
        exit(0);
    }

    FD_ZERO(&active_fd_set);
    FD_ZERO(&backup_fd_set);

    client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    sem_init(&this->wait_for_thread_operation_to_complete, 0 , 0);
    sem_init(&this->sem0_1, 0, 0);
    sem_init(&this->sem0_2, 0, 0);
    listen_clients = true;
    pthread_rwlock_init(&this->rwlock, NULL);
}

TcpClientServiceManager::~TcpClientServiceManager() {

    assert(this->tcp_client_db.empty());
    assert(!this->udp_fd);
    assert(!this->client_svc_mgr_thread);
}

void
TcpClientServiceManager::StartTcpClientServiceManagerThreadInternal() {

    int rcv_bytes;
    TcpClient *tcp_client;
    struct sockaddr_in client_addr;
    unsigned char dummy_msg;
    std::list<TcpClient *>::iterator it;

    socklen_t addr_len = sizeof(client_addr);

    this->max_fd = this->udp_fd;
    
    FD_SET(this->udp_fd , &this->backup_fd_set);

    sem_post(&this->wait_for_thread_operation_to_complete);

    while(true) {

        pthread_testcancel();

        memcpy (&this->active_fd_set, &this->backup_fd_set, sizeof(fd_set));

        select(this->max_fd + 1 , &this->active_fd_set, NULL, NULL, NULL);

        if (FD_ISSET(this->udp_fd, &this->active_fd_set)) {

            recvfrom(this->udp_fd, &dummy_msg, 1, 0 , (struct sockaddr *)&client_addr, &addr_len);
            sem_post(&this->sem0_1);
            sem_wait(&this->sem0_2);
            continue;
        }

        for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it){

                tcp_client = *it;

                if (FD_ISSET(tcp_client->comm_fd, &this->active_fd_set)) {
                    
                    rcv_bytes = recvfrom(tcp_client->comm_fd,
                                                        tcp_client->recv_buffer,
                                                        sizeof(tcp_client->recv_buffer),
                                                        0, 
                                                        (struct sockaddr *)&client_addr, &addr_len);

                    if (rcv_bytes == 0) {
                        this->client_disconnected(tcp_client);
                        /* Remove FD from fd_set otherwise, select will go in infinite loop*/
                        FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
                        this->tcp_server->CreateDeleteClientRequestSubmission(tcp_client);
                    }
                    else {
                        this->client_msg_recvd(tcp_client, tcp_client->recv_buffer, rcv_bytes);
                    }
                }
        }
    } // while ends
}

static void *
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

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

     pthread_create(this->client_svc_mgr_thread, &attr,  
                            tcp_client_svc_manager_thread_fn, (void *)this);

    sem_wait(&this->wait_for_thread_operation_to_complete);
    printf("TcpClientServiceManagerThread Started\n");
}

void
TcpClientServiceManager::SetClientMsgRecvCbk(
    void (*client_msg_recvd)(const TcpClient *, unsigned char *, uint16_t))
{
    this->client_msg_recvd = client_msg_recvd;
}

void
TcpClientServiceManager::SetClientDisconnectCbk(
        void (*client_disconnected)(const TcpClient *)) {

    this->client_disconnected = client_disconnected;
}

void
TcpClientServiceManager::SetListenAllClientsStatus(bool status) {

    pthread_rwlock_wrlock(&this->rwlock);
    this->listen_clients = status;
    pthread_rwlock_unlock(&this->rwlock);
}

TcpClient * 
TcpClientServiceManager::LookUpClientDB(uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it) {

        tcp_client = *it;
        if (tcp_client->ip_addr == ip_addr &&
                tcp_client->port_no == port_no) return tcp_client;
    }
    return NULL;
}

void
TcpClientServiceManager::AddNewClientFD(TcpClient *tcp_client) {

    ForceUnblockSelect();
    
    sem_wait(&this->sem0_1);

    assert(!this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));
    this->tcp_client_db.push_back(tcp_client);

    /* Now Update FDs */
    if (this->max_fd < tcp_client->comm_fd) {
        this->max_fd = tcp_client->comm_fd;
    }

    FD_SET(tcp_client->comm_fd, &this->backup_fd_set);
    sem_post(&this->sem0_2);
}

void
TcpClientServiceManager::RemoveClientFD(TcpClient *tcp_client) {

    ForceUnblockSelect();

    sem_wait(&this->sem0_1);

    assert(this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));
    this->tcp_client_db.remove(tcp_client);

    /* Now Update FDs */
    max_fd = GetMaxFd();

    FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
    sem_post(&this->sem0_2);
}

void
TcpClientServiceManager::StopListeningAllClients() {


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

    if (max_fd_lcl < this->udp_fd) {
        max_fd_lcl = this->udp_fd;
    }

    return max_fd_lcl;
}

/* Send 1B of dummy data to UDP FD*/
void
TcpClientServiceManager::ForceUnblockSelect() {

    int fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (fd < 0) {
        printf ("%s() Socket Creation Failed \n", __FUNCTION__);
        return;
    }

    unsigned char dummy_data;
    struct sockaddr_in server_addr;

    server_addr.sin_family      = AF_INET;
    server_addr.sin_port          = htons(tcp_server->port_no + 1);
    server_addr.sin_addr.s_addr = htonl(this->tcp_server->ip_addr);
    
    int rc = sendto(fd, (unsigned char *)&dummy_data, 1,
                0, (const struct sockaddr *) &server_addr, 
                sizeof(server_addr));

    if (rc < 0) {
        printf ("%s() Sending Data to UDP Socket Failed \n", __FUNCTION__);
    }

    close (fd);
}

void
TcpClientServiceManager::Stop() {

    this->StopTcpClientServiceManagerThread();
    /* Free other Resources */
}

void
TcpClientServiceManager::StopTcpClientServiceManagerThread() {

    pthread_cancel(*this->client_svc_mgr_thread);
    pthread_join(*this->client_svc_mgr_thread, NULL);
    free(this->client_svc_mgr_thread);
    this->client_svc_mgr_thread = NULL;
}