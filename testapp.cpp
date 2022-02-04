#include <stdio.h>
#include <unistd.h>
#include "TcpServer.h"

void
client_disconnect_notif (const TcpClient *tcp_client) {
    printf ("Appln : client disconnected\n");
}

void
client_connect_notif (const TcpClient *tcp_client) {
    printf ("Appln : client connected\n");
}

void
client_recv_msg(const TcpClient *tcp_client, unsigned char *msg, uint16_t msg_size) {
    printf ("Appln : msg recvd = %d B\n", msg_size);
}

int
main(int argc, char **argv) {

    TcpServer *server1 = new TcpServer(0, 40000, "Default");
    
    server1->SetServerNotifCallbacks(
            client_connect_notif, client_disconnect_notif,client_recv_msg, NULL);

    server1->Start();
    pthread_exit(0);
    return 0;
}
