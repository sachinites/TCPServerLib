#include <stdio.h>
#include <unistd.h>
#include "TcpServer.h"
#include "TcpClient.h"
#include "network_utils.h"
#include "arpa/inet.h"

TcpClient gtcp_client;

static void
print_client(const TcpClient *client) {

    printf ("[%s , %d]\n", network_covert_ip_n_to_p(htonl(client->ip_addr), 0),
                htons(client->port_no));
}

void
client_disconnect_notif (const TcpClient *tcp_client) {
    printf ("Appln : client disconnected : ");
    print_client(tcp_client);
}

void
client_connect_notif (const TcpClient *tcp_client) {
    printf ("Appln : client connected : ");
    print_client(tcp_client);
    gtcp_client = *tcp_client;
}

void
client_recv_msg(const TcpClient *tcp_client, unsigned char *msg, uint16_t msg_size) {
    printf ("Appln : client msg recvd = %dB : ", msg_size);
    print_client(tcp_client);
}

int
main(int argc, char **argv) {

    //TcpServer *server1 = new TcpServer(0, 40000, "Default");
    TcpServer *server1 = new TcpServer("127.0.0.1", 40000, "Default");
    server1->SetServerNotifCallbacks(
            client_connect_notif, client_disconnect_notif,client_recv_msg, NULL);

    server1->Start();
    pthread_exit(0);
    return 0;
}
