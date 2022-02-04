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

TcpClient::TcpClient(uint32_t ip_addr, uint16_t port_no) {
    this->ip_addr = ip_addr;
    this->port_no = port_no;
}

TcpClient::TcpClient() {}
TcpClient::TcpClient(TcpClient *tcp_client) {

    this->comm_fd = tcp_client->comm_fd;
    this->ip_addr = tcp_client->ip_addr;
    this->port_no = tcp_client->port_no;
}

TcpClient::~TcpClient() {

}

void
TcpClient::Abort() {

    close(this->comm_fd);
    this->comm_fd = 0;
    delete this;
}