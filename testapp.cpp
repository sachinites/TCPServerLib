#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "CommandParser/libcli.h"
#include "TcpServerController.h"
#include "TcpClient.h"
#include "network_utils.h"
#include "arpa/inet.h"

extern void tcp_build_cli();

static void
print_client(const TcpClient *client) {

    printf ("[%s , %d]\n", network_convert_ip_n_to_p(htonl(client->ip_addr), 0),
                htons(client->port_no));
}

static void
client_disconnect_notif (const TcpClient *tcp_client) {
    printf ("Appln : client disconnected : ");
    print_client(tcp_client);
}

static void
client_connect_notif (const TcpClient *tcp_client) {
    printf ("Appln : client connected : ");
    print_client(tcp_client);
    //gtcp_client = *tcp_client;
}
#pragma pack (push,1)
typedef struct _test_struct{

    uint16_t msg_size;
    unsigned int a;
    unsigned int b;
} test_struct_t;
#pragma pack(pop)

typedef struct result_struct_{

    unsigned int c;

} result_struct_t;

static void
client_recv_msg(const TcpClient *tcp_client, unsigned char *msg, uint16_t msg_size) {

    printf ("Appln : client msg recvd = %dB : ", msg_size);
    print_client(tcp_client);
	
    test_struct_t *data = (test_struct_t *)msg;
    result_struct_t res;
    res.c = data->a + data->b;
    //tcp_client->SendMsg((char *)&res, sizeof(res));
}

int
main(int argc, char **argv) {

    init_libcli();
    tcp_build_cli();
    start_shell();
    return 0;
}
