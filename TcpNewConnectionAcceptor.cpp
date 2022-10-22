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

#include "TcpServerController.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClientDbManager.h"
#include "TcpClient.h"
#include "network_utils.h"

TcpNewConnectionAcceptor::TcpNewConnectionAcceptor(TcpServerController *TcpServerController) {

    this->accept_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (this->accept_fd < 0) {
        printf("Error : Could not create Accept FD\n");
        exit(0);
    }

    this->accept_new_conn_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);
    pthread_rwlock_init(&this->rwlock, NULL);
    this->accept_new_conn = true;
    this->tcp_ctrlr = TcpServerController;
}

 TcpNewConnectionAcceptor::~TcpNewConnectionAcceptor() {

        assert (this->accept_fd == 0);
        assert(!this->accept_new_conn_thread);
 }

void
TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThreadInternal() {

    int opt = 1;
    bool accept_new_conn;
    bool create_multi_threaded_client;
    tcp_server_msg_code_t ctrlr_code = (tcp_server_msg_code_t)0;
    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(this->tcp_ctrlr->port_no);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);

    if (setsockopt(this->accept_fd, SOL_SOCKET,
                   SO_REUSEADDR, (char *)&opt, sizeof(opt))<0) {
        printf("setsockopt Failed\n");
        exit(0);
    }

    if (setsockopt(this->accept_fd, SOL_SOCKET,
                   SO_REUSEPORT, (char *)&opt, sizeof(opt))<0) {
        printf("setsockopt Failed\n");
       exit(0);
    }

    if (bind(this->accept_fd, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1) {
        printf("Error : Acceptor socket bind failed [%s(0x%x), %d], error = %d\n", 
            network_convert_ip_n_to_p(tcp_ctrlr->ip_addr, 0),
            tcp_ctrlr->ip_addr,
            tcp_ctrlr->port_no, errno);
        exit(0);
    }

    if (listen(this->accept_fd, 5) < 0 ) {

        printf("listen failed\n");
        exit(0);
    }
    
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int comm_socket_fd;

    sem_post (&this->wait_for_thread_operation_to_complete);

    while (true) {

        pthread_testcancel();
        comm_socket_fd =  accept (this->accept_fd,
                                                     (struct sockaddr *)&client_addr, &addr_len);

        if (comm_socket_fd < 0) {
            printf ("Error in Accepting New Connections\n");
            continue;
        }

        pthread_rwlock_rdlock(&this->rwlock);
        accept_new_conn = this->accept_new_conn;
        pthread_rwlock_unlock(&this->rwlock);

        if (!accept_new_conn) {
            close(comm_socket_fd);
            continue;
        }

        /* Send Welcome msg to client */
        TcpClient *tcp_client = new TcpClient();
        tcp_client->comm_fd = comm_socket_fd;
        tcp_client->ip_addr = htonl(client_addr.sin_addr.s_addr);
        tcp_client->port_no = htons(client_addr.sin_port);
        tcp_client->tcp_ctrlr = this->tcp_ctrlr;
        tcp_client->server_ip_addr = this->tcp_ctrlr->ip_addr;
        tcp_client->server_port_no = (this->tcp_ctrlr->port_no);
        tcp_client->SetState(TCP_CLIENT_STATE_CONNECTED | 
                                          TCP_CLIENT_STATE_PASSIVE_OPENER);
        tcp_client->SetConnectionType(tcp_conn_via_accept);

        if (this->tcp_ctrlr->client_connected) {
            this->tcp_ctrlr->client_connected(this->tcp_ctrlr, tcp_client);
        }

        tcp_client->SendMsg("Welcome\n", strlen("Welcome\n"));

        tcp_client->SetTcpMsgDemarcar(
            TcpMsgDemarcar::InstantiateTcpMsgDemarcar(
                TCP_DEMARCAR_NONE, 0, 0, 0, 0, 0));
       
       if (this->tcp_ctrlr->IsBitSet( TCP_SERVER_CREATE_MULTI_THREADED_CLIENT)) {

          ctrlr_code = CTRLR_ACTION_TCP_CLIENT_CREATE_THREADED;
      }   
      else {

        ctrlr_code = CTRLR_ACTION_TCP_CLIENT_MULTIPLEX_LISTEN;
      }

        this->tcp_ctrlr->EnqueMsg(
                (tcp_server_msg_code_t) (CTRLR_ACTION_TCP_CLIENT_PROCESS_NEW | ctrlr_code),
                tcp_client, false);
    }
}

static void *
tcp_listen_for_new_connections(void *arg) {

    TcpNewConnectionAcceptor *tcp_new_conn_acc = 
            (TcpNewConnectionAcceptor *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL);

    tcp_new_conn_acc->StartTcpNewConnectionAcceptorThreadInternal();
    return NULL;
}

  void
  TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThread() {

      pthread_attr_t attr;

      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

      if (pthread_create(this->accept_new_conn_thread, &attr,           tcp_listen_for_new_connections, (void *)this)) {
          printf ("%s() Thread Creation failed, error = %d\n", __FUNCTION__, errno);
          exit(0);
      }
      sem_wait (&this->wait_for_thread_operation_to_complete);
      printf ("Service started : TcpNewConnectionAcceptorThread\n");
  }

void
TcpNewConnectionAcceptor::SetAcceptNewConnectionStatus(bool status) {

    pthread_rwlock_wrlock(&this->rwlock);
    this->accept_new_conn = status;
    pthread_rwlock_unlock(&this->rwlock);
}

void
TcpNewConnectionAcceptor::Stop() {

    this->StopTcpNewConnectionAcceptorThread();
    close(this->accept_fd);
    this->accept_fd = 0;
    sem_destroy(&this->wait_for_thread_operation_to_complete);
    pthread_rwlock_destroy(&this->rwlock);
    delete this;
}

void
TcpNewConnectionAcceptor::StopTcpNewConnectionAcceptorThread() {

    pthread_cancel(*this->accept_new_conn_thread);
    pthread_join(*this->accept_new_conn_thread, NULL);
    free(this->accept_new_conn_thread);
    this->accept_new_conn_thread = NULL;
}
