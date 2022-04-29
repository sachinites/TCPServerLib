#include <stdio.h>
#include "TcpClient.h"
#include "network_utils.h"
#include <arpa/inet.h>

TcpClient::TcpClient(uint32_t ip_addr, uint16_t port_no) {

    this->ip_addr = ip_addr;
    this->port_no = port_no;
}