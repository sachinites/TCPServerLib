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
#include "TcpServerController.h"
#include "TcpClient.h"
#include "network_utils.h"

#define CLIENT_RECV_BUFFER_SIZE 1024 
static unsigned char common_recv_buffer[CLIENT_RECV_BUFFER_SIZE];

TcpClientServiceManager::TcpClientServiceManager(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;

    this->udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(this->udp_fd < 0){ 
        printf("UDP Socket Creation Failed, error = %d\n", errno);
        exit(0);
    }  

    this->max_fd = this->udp_fd;

    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port          = htons(tcp_ctrlr->port_no + 1);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);

    if (bind(this->udp_fd, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1) {
        printf("Error : UDP socket bind failed [%s(0x%x), %d], error = %d\n", 
            network_convert_ip_n_to_p(tcp_ctrlr->ip_addr, 0),
            tcp_ctrlr->ip_addr,
            tcp_ctrlr->port_no + 1, errno);
        exit(0);
    }

    FD_ZERO(&active_fd_set);
    FD_ZERO(&backup_fd_set);

    client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    sem_init(&this->wait_for_thread_operation_to_complete, 0 , 0);
    sem_init(&this->sem0_1, 0, 0);
    sem_init(&this->sem0_2, 0, 0);
    pthread_rwlock_init(&this->rwlock, NULL);
}

TcpClientServiceManager::~TcpClientServiceManager() {

    assert(this->tcp_client_db.empty());
    assert(!this->udp_fd);
    assert(!this->client_svc_mgr_thread);
}

void 
TcpClientServiceManager::RemoveClientFromDB(TcpClient *tcp_client) {

    this->tcp_client_db.remove(tcp_client);
    tcp_client->Dereference();
}

void
TcpClientServiceManager::AddClientToDB(TcpClient *tcp_client){

     this->tcp_client_db.push_back(tcp_client);
     tcp_client->Reference();
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

/* Deprecated */

void
TcpClientServiceManager::StartTcpClientServiceManagerThreadInternal2() {

    int rcv_bytes;
    TcpClient *tcp_client, *next_tcp_client;
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

        /* Iterate so that we can delete the current element while traversing */
        for (it = this->tcp_client_db.begin(), tcp_client = *it;
             it != this->tcp_client_db.end();
             tcp_client = next_tcp_client){

            next_tcp_client = *(++it);

            if (FD_ISSET(tcp_client->comm_fd, &this->active_fd_set)) {

                rcv_bytes = recvfrom(tcp_client->comm_fd,
                        tcp_client->recv_buffer,
                        sizeof(tcp_client->recv_buffer),
                        0, 
                        (struct sockaddr *)&client_addr, &addr_len);

                if (rcv_bytes == 0 || rcv_bytes == 65535 || rcv_bytes < 0) {
                    this->tcp_ctrlr->client_disconnected(this->tcp_ctrlr, tcp_client);
                    /* Remove FD from fd_set otherwise, select will go in infinite loop*/
                    FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
                    tcp_client->Reference();
                    this->RemoveClientFromDB(tcp_client);
                    tcp_client->UnSetState (TCP_CLIENT_STATE_MULTIPLEX_LISTEN);
                    tcp_client->UnSetState (TCP_CLIENT_STATE_CONNECTED);
                    this->max_fd = this->GetMaxFdSimple();
                    this->tcp_ctrlr->EnqueMsg(CTRLR_ACTION_TCP_CLIENT_DELETE, (void *)tcp_client, false);
                    tcp_client->Dereference();
                }
                else {
                    /* If client has a TcpMsgDemarcar, then push the data to Demarcar, else notify the application straightaway */
                       tcp_client->conn.bytes_recvd += rcv_bytes;
                       if (tcp_client->msgd) {
                           tcp_client->msgd->ProcessMsg(tcp_client, tcp_client->recv_buffer, rcv_bytes);
                       }
                       else if (this->tcp_ctrlr->client_msg_recvd){
                            this->tcp_ctrlr->client_msg_recvd(this->tcp_ctrlr, tcp_client, tcp_client->recv_buffer, rcv_bytes);
                       }
                }
            }
        }

        if (FD_ISSET(this->udp_fd, &this->active_fd_set)) {

            recvfrom(this->udp_fd, &dummy_msg, 1, 0 , (struct sockaddr *)&client_addr, &addr_len);
            sem_post(&this->sem0_1);
            sem_wait(&this->sem0_2);
            continue;
        }
    } // while ends
}

void
TcpClientServiceManager::StartTcpClientServiceManagerThreadInternalSimple() {

    int rcv_bytes;
    TcpClient *tcp_client, *next_tcp_client;
    struct sockaddr_in client_addr;
    std::list<TcpClient *>::iterator it;

    socklen_t addr_len = sizeof(client_addr);

    if (!this->tcp_ctrlr->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        this->tcp_ctrlr->CopyAllClientsTolist(&this->tcp_client_db);
    }

    this->max_fd = this->GetMaxFdSimple();

    FD_ZERO(&this->backup_fd_set);
    this->CopyClientFDtoFDSet(&this->backup_fd_set);

    sem_post(&this->wait_for_thread_operation_to_complete);

    while(true) {

        pthread_testcancel();
        
        memcpy (&this->active_fd_set, &this->backup_fd_set, sizeof(fd_set));

        select(this->max_fd + 1 , &this->active_fd_set, NULL, NULL, NULL);

        /* Iterate so that we can delete the current element while traversing */
        for (it = this->tcp_client_db.begin(), tcp_client = *it;
             it != this->tcp_client_db.end();
             tcp_client = next_tcp_client){

            next_tcp_client = *(++it);

            if (FD_ISSET(tcp_client->comm_fd, &this->active_fd_set)) {

                rcv_bytes = recvfrom(tcp_client->comm_fd,
                        common_recv_buffer,
                        CLIENT_RECV_BUFFER_SIZE,
                        0, 
                        (struct sockaddr *)&client_addr, &addr_len);

                if (rcv_bytes == 0 || rcv_bytes == 65535 || rcv_bytes < 0) {
                    this->tcp_ctrlr->client_disconnected(this->tcp_ctrlr, tcp_client);
                    /* Remove FD from fd_set otherwise, select will go in infinite loop*/
                    FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
                    this->max_fd = this->GetMaxFdSimple();
                    this->tcp_ctrlr->EnqueMsg(CTRLR_ACTION_TCP_CLIENT_RECONNECT, (void *)tcp_client, false);
                }
                else {
                    /* If client has a TcpMsgDemarcar, then push the data to Demarcar, else notify the application straightaway */
                       tcp_client->conn.bytes_recvd += rcv_bytes;
                       if (tcp_client->msgd) {
                           tcp_client->msgd->ProcessMsg(tcp_client, common_recv_buffer, rcv_bytes);
                       }
                       else if (this->tcp_ctrlr->client_msg_recvd){
                            this->tcp_ctrlr->client_msg_recvd(this->tcp_ctrlr, tcp_client, common_recv_buffer, rcv_bytes);
                       }
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

     svc_mgr->StartTcpClientServiceManagerThreadInternalSimple();
     return NULL;
 }

void
TcpClientServiceManager::StartTcpClientServiceManagerThread() {

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (!this->client_svc_mgr_thread) {
        this->client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    }

    pthread_create(this->client_svc_mgr_thread, &attr,  
                            tcp_client_svc_manager_thread_fn, (void *)this);

    sem_wait(&this->wait_for_thread_operation_to_complete);
    printf("Service started : TcpClientServiceManagerThread\n");
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

/* Not used in the Project*/
void
TcpClientServiceManager::ClientFDStartListenAdv(TcpClient *tcp_client) {

    ForceUnblockSelect();
    
    sem_wait(&this->sem0_1);

    assert(!this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));
    this->AddClientToDB(tcp_client);
    tcp_client->SetState (TCP_CLIENT_STATE_MULTIPLEX_LISTEN);
    /* Now Update FDs */
    if (this->max_fd < tcp_client->comm_fd) {
        this->max_fd = tcp_client->comm_fd;
    }

    FD_SET(tcp_client->comm_fd, &this->backup_fd_set);
    sem_post(&this->sem0_2);
}

void
TcpClientServiceManager::ClientFDStartListenSimple(TcpClient *tcp_client) {

    if (!this->tcp_ctrlr->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        this->StopTcpClientServiceManagerThread();
        printf ("Client Svc Mgr thread Cancelled\n");
    }
    assert(!this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));
    this->AddClientToDB(tcp_client);
    tcp_client->SetState (TCP_CLIENT_STATE_MULTIPLEX_LISTEN);
    if (!this->tcp_ctrlr->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        this->StartTcpClientServiceManagerThread();
    }
}

/* Overloaded fn */
TcpClient*
TcpClientServiceManager::ClientFDStopListenAdv(uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client;

    ForceUnblockSelect();

    sem_wait(&this->sem0_1);

    tcp_client = this->LookUpClientDB(ip_addr, port_no);
    
    if (!tcp_client) {
        sem_post(&this->sem0_2);
        return NULL;
    }

    this->RemoveClientFromDB(tcp_client);
    tcp_client->UnSetState (TCP_CLIENT_STATE_MULTIPLEX_LISTEN);
    /* Now Update FDs */
    max_fd = GetMaxFdSimple();

    FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
    this->tcp_ctrlr->client_disconnected(this->tcp_ctrlr, tcp_client);
    sem_post(&this->sem0_2);
    return tcp_client;
}

void
TcpClientServiceManager::ClientFDStopListenAdv(TcpClient *tcp_client) {

    ForceUnblockSelect();

    sem_wait(&this->sem0_1);

    assert (tcp_client == 
        this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));
    
    this->RemoveClientFromDB(tcp_client);
    tcp_client->UnSetState (TCP_CLIENT_STATE_MULTIPLEX_LISTEN);

    /* Now Update FDs */
    max_fd = GetMaxFdSimple();

    FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
    this->tcp_ctrlr->client_disconnected(this->tcp_ctrlr, tcp_client);
    sem_post(&this->sem0_2);
}

void
TcpClientServiceManager::ClientFDStopListenSimple(TcpClient *tcp_client) {

    if (!this->tcp_ctrlr->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        this->StopTcpClientServiceManagerThread();
    }

    assert(this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));
    this->RemoveClientFromDB(tcp_client);
    tcp_client->UnSetState (TCP_CLIENT_STATE_MULTIPLEX_LISTEN);

    if (!this->tcp_ctrlr->IsBitSet (TCP_SERVER_NOT_LISTENING_CLIENTS)) {
        this->StartTcpClientServiceManagerThread();
    }

}

int
TcpClientServiceManager::GetMaxFdSimple() {

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

/* Deprecated */
int
TcpClientServiceManager::GetMaxFdAdv() {

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
    server_addr.sin_port          = htons(tcp_ctrlr->port_no + 1);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);
    
    int rc = sendto(fd, (unsigned char *)&dummy_data, 1,
                0, (const struct sockaddr *) &server_addr, 
                sizeof(server_addr));

    if (rc < 0) {
        printf ("%s() Sending Data to UDP Socket Failed \n", __FUNCTION__);
    }

    close (fd);
}

void
TcpClientServiceManager::Purge() {

    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client, *next_tcp_client;

    /* This fn assumes that Svc mgr thread is already cancelled,
        hence no need to lock anything */
    assert(this->client_svc_mgr_thread == NULL);

    for (it = this->tcp_client_db.begin(), tcp_client = *it;
         it != this->tcp_client_db.end();
         tcp_client = next_tcp_client) {

        next_tcp_client = *(++it);

        this->tcp_client_db.remove(tcp_client);
        tcp_client->UnSetState (TCP_CLIENT_STATE_MULTIPLEX_LISTEN);
        tcp_client->Dereference();
    }
}

void
TcpClientServiceManager::Stop() {

    this->StopTcpClientServiceManagerThread();
    close(this->udp_fd);
    this->udp_fd = 0;
    this->Purge();
    delete this;
}

void
TcpClientServiceManager::StopTcpClientServiceManagerThread() {

    if (!this->client_svc_mgr_thread) return;
    pthread_cancel(*this->client_svc_mgr_thread);
    pthread_join(*this->client_svc_mgr_thread, NULL);
    free(this->client_svc_mgr_thread);
    this->client_svc_mgr_thread = NULL;
    printf ("Service stopped : TcpClientServiceManagerThread\n");
}
