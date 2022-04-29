#include "TcpServerController.h"
#include "TcpClient.h"
#include "network_utils.h"
#include <arpa/inet.h> 

static void
print_client(const TcpClient *tcp_client) {

    printf ("[%s , %d]\n", network_convert_ip_n_to_p(htonl(tcp_client->ip_addr), 0),
                htons(tcp_client->port_no));
}


static void
print_server(const TcpServerController *tcp_server) {

     printf ("[%s , %d]\n", network_convert_ip_n_to_p((tcp_server->ip_addr), 0),
                htons(tcp_server->port_no));
}

static void
appln_client_connected (const TcpServerController *tcp_server, const TcpClient *tcp_client) {

    print_server(tcp_server);
    printf("Appln : Client Connected : ");
    print_client(tcp_client);
}

static void
appln_client_disconnected (const TcpServerController *tcp_server, const TcpClient *tcp_client) {
    
}

static void
appln_client_msg_recvd (const TcpServerController *tcp_server, const TcpClient *tcp_client, unsigned char* msg, uint16_t msg_size) {
    
}

int
main(int argc, char **argv) {

    TcpServerController *server1 = new TcpServerController("127.0.0.1", 40000, "Default TCP Server");
    server1->SetServerNotifCallbacks(appln_client_connected, appln_client_disconnected, appln_client_msg_recvd);
    server1->Start();
    scanf("\n");
    return 0;
}