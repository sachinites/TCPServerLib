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
#include "TcpClient.h"
#include "TcpClientServiceManager.h"
#include "TcpServer.h"

TcpClient::TcpClient(uint32_t ip_addr, uint16_t port_no) {

    this->ip_addr = ip_addr;
    this->port_no = port_no;
    sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);
}

TcpClient::TcpClient() {

    this->ip_addr = 0;
    this->port_no = 0;
    sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);
}
TcpClient::TcpClient(TcpClient *tcp_client) {

    this->comm_fd = tcp_client->comm_fd;
    this->ip_addr = tcp_client->ip_addr;
    this->port_no = tcp_client->port_no;
    sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);
}

TcpClient::~TcpClient() {

    assert(!this->comm_fd);
    assert(!this->client_thread);
}

void
TcpClient::Abort() {

    if (this->client_thread) {
        this->StopThread();
    }
    close(this->comm_fd);
    this->comm_fd = 0;
    sem_destroy(&this->wait_for_thread_operation_to_complete);
    delete this;
}

void 
TcpClient::SetClientSvcMgr(TcpClientServiceManager *svc_mgr) {

    this->svc_mgr = svc_mgr;
}

void
TcpClient::ClientThreadFunction() {

    int rcv_bytes;
    struct sockaddr_in client_addr;

    socklen_t addr_len = sizeof(client_addr);

    while (1)
    {
        pthread_testcancel();
        
        rcv_bytes = recvfrom(this->comm_fd,
                             this->recv_buffer,
                             sizeof(this->recv_buffer),
                             0,
                             (struct sockaddr *)&client_addr, &addr_len);

        if (rcv_bytes == 0) {
            this->svc_mgr->client_disconnected(this);
            this->svc_mgr->tcp_server->CreateDeleteClientRequestSubmission(this);
        }
        else {
            this->svc_mgr->client_msg_recvd(this, this->recv_buffer, rcv_bytes);
        }
    }
}

static void *
client_listening_thread(void *arg) {

    TcpClient *tcp_client = (TcpClient *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    sem_post(&tcp_client->wait_for_thread_operation_to_complete);

    tcp_client->ClientThreadFunction();
    return NULL;
}

void
TcpClient::StartThread() {

    pthread_attr_t attr;

    assert (!this->client_thread );
    this->client_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(this->client_thread, &attr, client_listening_thread, (void *)this);
    sem_wait(&this->wait_for_thread_operation_to_complete);
}

void
TcpClient::StopThread() {

    assert(this->client_thread);
    pthread_cancel(*this->client_thread);
    pthread_join(*this->client_thread, NULL);
    free(this->client_thread);
    this->client_thread = NULL;
}