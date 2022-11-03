#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h> // for IPPROTO_TCP
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "TcpServerController.h"
#include "TcpNewConnectionAcceptor.h"
#include "network_utils.h"
#include "TcpClient.h"
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"
#include "TcpMsgVariableSizeDemarcar.h"

TcpNewConnectionAcceptor::TcpNewConnectionAcceptor(TcpServerController *tcp_ctrlr) {

    this->accept_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

     if (this->accept_fd < 0) {
        printf("Error : Could not create Accept FD\n");
        exit(0);
    }

    this->accept_new_conn_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    this->tcp_ctrlr = tcp_ctrlr;
}

TcpNewConnectionAcceptor::~TcpNewConnectionAcceptor(){

}

/*
    2. Create an infinite loop
    3. invoke accept() to accept new connections
    4. Notify the application for new connections */
void
TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThreadInternal() {

    int opt = 1;
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->tcp_ctrlr->port_no);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);

    if (setsockopt(this->accept_fd, SOL_SOCKET,
                SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {

        printf ("setsockopt Failed\n");
        exit(0);
    }

    if (setsockopt(this->accept_fd, SOL_SOCKET,
                SO_REUSEPORT, (char *)&opt, sizeof(opt)) < 0) {

        printf ("setsockopt Failed\n");
        exit(0);
    }

    /* Let us bind the socket now */

    if (bind(this->accept_fd, (struct sockaddr *)&server_addr, 
                    sizeof(struct sockaddr)) == -1) {

        printf("Error : Acceptor socket bind failed [%s(0x%x), %d], error = %d\n", 
            network_convert_ip_n_to_p(this->tcp_ctrlr->ip_addr, 0),
            this->tcp_ctrlr->ip_addr,
            this->tcp_ctrlr->port_no, errno);
        exit(0);
    }
                
    if (listen(this->accept_fd, 5) < 0 ) {

        printf("listen failed\n");
        exit(0);
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int comm_sock_fd;

    while(true) {

        comm_sock_fd = accept (this->accept_fd, 
                        (struct sockaddr *)&client_addr, &addr_len);

        if (comm_sock_fd < 0) {

            printf ("Error in Accepting New Connections\n");
            continue;
        }

        TcpClient *tcp_client = new TcpClient (
            htonl(client_addr.sin_addr.s_addr),
            htons(client_addr.sin_port));

        tcp_client->server_ip_addr = this->tcp_ctrlr->ip_addr;
        tcp_client->server_port_no = this->tcp_ctrlr->port_no;
        
        tcp_client->tcp_ctrlr = this->tcp_ctrlr;
        tcp_client->comm_fd = comm_sock_fd;

        if (this->tcp_ctrlr->client_connected) {
            this->tcp_ctrlr->client_connected(this->tcp_ctrlr, tcp_client);
        }
        
        tcp_client->msgd = NULL;

        /* Tell the TCP Controller, to further process the Client */
        this->tcp_ctrlr->ProcessNewClient(tcp_client);
        
        printf ("Connection Accepted from Client [%s, %d]\n",
                network_convert_ip_n_to_p(htonl(client_addr.sin_addr.s_addr) , 0),
                htons(client_addr.sin_port));
    }
}

static void *
tcp_listen_for_new_connections(void *arg) {

    TcpNewConnectionAcceptor *tcp_new_conn_acc = 
        (TcpNewConnectionAcceptor *)arg;

    tcp_new_conn_acc->StartTcpNewConnectionAcceptorThreadInternal();
    return NULL;
}
/* 1. Start a thread
    2. Create an infinite loop
    3. invoke accept() to accept new connections
    4. Notify the application for new connections
*/
void
TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThread() {

    if (pthread_create(this->accept_new_conn_thread, NULL, tcp_listen_for_new_connections, (void *)this)) {

        printf ("%s() Thread Creation failed, error = %d\n", __FUNCTION__, errno);
        exit(0);
    }

    printf ("Service Started : TcpNewConnectionAcceptorThread\n");
}

void
TcpNewConnectionAcceptor::StopTcpNewConnectionAcceptorThread() {

    if (!this->accept_new_conn_thread) return;
    pthread_cancel (*this->accept_new_conn_thread);
    /* wait until the thread is cancelled successfully */
    pthread_join (*this->accept_new_conn_thread, NULL);
    free(this->accept_new_conn_thread);
    this->accept_new_conn_thread = NULL;
}

void 
TcpNewConnectionAcceptor::Stop() {
/* 1. Stop the CAS thread if it is running 
    2. Release the resource ( accept_fd)
    3. delete this instance of CAS */

    this->StopTcpNewConnectionAcceptorThread();
    close (this->accept_fd);
    this->accept_fd = 0;
    delete this;
}
