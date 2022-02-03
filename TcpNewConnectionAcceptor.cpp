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

#include "TcpServer.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClientDbManager.h"
#include "TcpClient.h"

TcpNewConnectionAcceptor::TcpNewConnectionAcceptor(TcpServer *TcpServer) {

        this->tcp_server = TcpServer;

        this->accept_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if (this->accept_fd < 0) {
            printf ("Error : Could not create Accept FD\n");
            exit(0);
        }

        this->accept_new_conn_thread = (pthread_t *)calloc ( 1 , sizeof(pthread_t));
        
        sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);

        this->shared_binary_semaphore1 = &TcpServer->binary_semaphore_1;
}

 TcpNewConnectionAcceptor::~TcpNewConnectionAcceptor() {

        close (this->accept_fd);
        free(this->accept_new_conn_thread);
        sem_destroy(&this->wait_for_thread_operation_to_complete);
 }

void
TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThreadInternal() {

    int opt = 1;
    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(this->tcp_server->port_no);
    server_addr.sin_addr.s_addr = htonl(this->tcp_server->ip_addr);

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
        printf("Error : Master socket bind failed\n");
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

        comm_socket_fd =  accept (this->accept_fd,
                                                        (struct sockaddr *)&client_addr, &addr_len);

        if (comm_socket_fd < 0) {
            printf ("Error in Accepting New Connections\n");
            continue;
        }


        sem_wait(this->shared_binary_semaphore1);

        /* Inspect TcpServer Attributes here */

        sem_post(this->shared_binary_semaphore1);



        TcpClient tcp_client;
        tcp_client.comm_fd = comm_socket_fd;
        tcp_client.ip_addr = client_addr.sin_addr.s_addr;
        tcp_client.port_no =  client_addr.sin_port;
        this->tcp_server->tcp_client_db_mgr->NewClientCreationRequest
            (&tcp_client, TCP_DB_MGR_NEW_CLIENT_CREATE);
    }
}

static void *
tcp_listen_for_new_connections(void *arg) {

    TcpNewConnectionAcceptor *tcp_new_conn_acc = 
            (TcpNewConnectionAcceptor *)arg;

    tcp_new_conn_acc->StartTcpNewConnectionAcceptorThreadInternal();
    return NULL;
}

  void
  TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThread() {

      pthread_attr_t attr;
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

      pthread_create(this->accept_new_conn_thread, &attr, tcp_listen_for_new_connections, (void *)this);

      sem_wait (&this->wait_for_thread_operation_to_complete);
      printf ("TcpNewConnectionAcceptorThread Started\n");
  }

