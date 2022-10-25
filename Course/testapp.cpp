#include "TcpServerController.h"
#include "TcpClient.h"
#include "network_utils.h"
#include <arpa/inet.h> 
#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"

#define TCP_SERVER_CREATE 1
#define TCP_SERVER_START 2
#define TCP_SERVER_SHOW_TCP_SERVER 3
#define TCP_SERVER_STOP_CONN_ACCEPT 4
#define TCP_SERVER_STOP_CLIENT_LISTEN   5
#define TCP_SERVER_STOP 6

static void
print_client(const TcpClient *tcp_client) {

    printf ("[%s , %d]\n", network_convert_ip_n_to_p((tcp_client->ip_addr), 0),
                (tcp_client->port_no));
}

static void
print_server(const TcpServerController *tcp_server) {

     printf ("%p [%s , %d]\n", tcp_server,
                network_convert_ip_n_to_p((tcp_server->ip_addr), 0),
                (tcp_server->port_no));
}

static void
appln_client_connected (const TcpServerController *tcp_server, const TcpClient *tcp_client) {

    printf ("\nTcp Server \n");
    print_server(tcp_server);
    printf("Appln : Client Connected : ");
    print_client(tcp_client);
}   

static void
appln_client_disconnected (const TcpServerController *tcp_server, const TcpClient *tcp_client) {
    
}

static void
appln_client_msg_recvd (const TcpServerController *tcp_server, const TcpClient *tcp_client, unsigned char* msg, uint16_t msg_size) {
    
    printf ("%s() Bytes recvd : %d msg : %s\n",  __FUNCTION__, msg_size, msg);
}


std::list<TcpServerController *> tcp_server_lst;
uint16_t default_port_no = 40000;
const char *default_ip_addr = "127.0.0.1";

static TcpServerController *
TcpServer_lookup (std::string tcp_sever_name) {

    TcpServerController *ctrlr;

    std::list<TcpServerController *>::iterator it;

    for (it = tcp_server_lst.begin(); it != tcp_server_lst.end(); ++it){
        
        ctrlr = *it;
        if  (ctrlr->name == tcp_sever_name) {
            return ctrlr;
        }
    }
    return NULL;
}


static int
config_tcp_server_handler (param_t *param, ser_buff_t *ser_buff, op_mode enable_or_disable) {

    int cmd_code;
    const char *server_name = NULL;
    tlv_struct_t *tlv = NULL;
    TcpServerController *tcp_server = NULL;
    char *ip_addr = (char *)default_ip_addr;
    uint16_t port_no = default_port_no;

    cmd_code = EXTRACT_CMD_CODE(ser_buff);

    TLV_LOOP_BEGIN(ser_buff, tlv) {

        if (strncmp(tlv->leaf_id, "tcp-server-name", strlen("tcp-server-name")) == 0) {
            server_name = tlv->value;
        }
        else if (strncmp(tlv->leaf_id, "tcp-server-addr", strlen("tcp-server-addr")) == 0) {
            ip_addr = tlv->value;
        }
        else if (strncmp(tlv->leaf_id, "tcp-server-port", strlen("tcp-server-port")) == 0) {
            port_no= atoi(tlv->value);
        }        
    } TLV_LOOP_END;

    switch (cmd_code) {

        case TCP_SERVER_CREATE:
            /* config tcp-server <server-name>*/
            tcp_server = TcpServer_lookup (std::string(server_name));
            if (tcp_server) {
                printf ("Error : Tcp Server Already Exist\n");
                return -1;
            }
            tcp_server = new TcpServerController (std::string(ip_addr), port_no, std::string(server_name));
            tcp_server_lst.push_back(tcp_server);
            tcp_server->SetServerNotifCallbacks (appln_client_connected, appln_client_disconnected, appln_client_msg_recvd);
        break;
        case TCP_SERVER_START:
         /* config tcp-server <server-name> start*/
            tcp_server = TcpServer_lookup (std::string(server_name));
            if (!tcp_server) {
                printf ("Error : Tcp Server do not Exist\n");
                return -1;
            }
            tcp_server->Start();
        break;
        case TCP_SERVER_STOP_CONN_ACCEPT:
             tcp_server = TcpServer_lookup (std::string(server_name));
            if (!tcp_server) {
                printf ("Error : Tcp Server do not Exist\n");
                return -1;
            }
            switch (enable_or_disable) {
                case CONFIG_ENABLE:
                    tcp_server->StopConnectionAcceptingSvc();
                break;
                case CONFIG_DISABLE:
                     tcp_server->StartConnectionAcceptingSvc();
                break;
            }
            break;
        case TCP_SERVER_STOP_CLIENT_LISTEN:
            tcp_server = TcpServer_lookup(std::string(server_name));
            if (!tcp_server)
            {
                printf("Error : Tcp Server do not Exist\n");
                return -1;
            }
            switch (enable_or_disable)
            {
            case CONFIG_ENABLE:
                tcp_server->StopClientSvcMgr();
                break;
            case CONFIG_DISABLE:
                tcp_server->StartClientSvcMgr();
                break;
            }
            break;
        case TCP_SERVER_STOP:
            tcp_server = TcpServer_lookup(std::string(server_name));
            if (!tcp_server)
            {
                printf("Error : Tcp Server do not Exist\n");
                return -1;
            }
            switch (enable_or_disable)
            {
            case CONFIG_ENABLE:
                tcp_server->Stop();
                break;
            case CONFIG_DISABLE:
                printf ("Command Negation is not supported for this CLI\n");
                return -1;
            }
            break;
        default:;
    }

    return 0;
}

static int
show_tcp_server_handler (param_t *param, ser_buff_t *ser_buff, op_mode enable_or_disable) {

    int cmd_code;
    char *server_name = NULL;
    tlv_struct_t *tlv = NULL;
    TcpServerController *tcp_server = NULL;

    cmd_code = EXTRACT_CMD_CODE(ser_buff);

        TLV_LOOP_BEGIN(ser_buff, tlv) {

            if (strncmp(tlv->leaf_id, "tcp-server-name", strlen("tcp-server-name")) == 0) {
                server_name = tlv->value;
             }

        }  TLV_LOOP_END;

    switch(cmd_code) {
       case TCP_SERVER_SHOW_TCP_SERVER:
            tcp_server = TcpServer_lookup (std::string(server_name));
            if (!tcp_server) {
                printf ("Error : Tcp Server do not Exist\n");
                return -1;
            }
            tcp_server->Display();
        default:;
        break;
    }
    return 0;
}


static void
tcp_build_config_cli_tree() {

    /* config tcp-server <name> */
    param_t *config_hook = libcli_get_config_hook();
    {
        /* config tcp-server ...*/
        static param_t tcp_server;
        init_param (&tcp_server, CMD, "tcp-server", NULL, NULL, INVALID, NULL, "config tcp-server");
        libcli_register_param (config_hook, &tcp_server);
        {
            /* config tcp-server <name> */
            static param_t tcp_server_name;
            init_param(&tcp_server_name, LEAF, NULL, config_tcp_server_handler, NULL, STRING, "tcp-server-name", "Tcp Server Name");
            libcli_register_param (&tcp_server, &tcp_server_name);
            set_param_cmd_code(&tcp_server_name, TCP_SERVER_CREATE);
            {
                /* config tcp-server <name> [no] disable-conn-accept */
                static param_t dis_conn_accept;
                init_param(&dis_conn_accept, CMD, "disable-conn-accept", config_tcp_server_handler, 0, INVALID, 0, "Connection Accept Settings");
                libcli_register_param(&tcp_server_name, &dis_conn_accept);
                set_param_cmd_code(&dis_conn_accept, TCP_SERVER_STOP_CONN_ACCEPT);                     
            }
            {
                /* config tcp-server <name> [no] disable-client-listen */
                static param_t disable_client_listen;
                init_param(&disable_client_listen, CMD, "disable-client-listen", config_tcp_server_handler, 0, INVALID, 0, "Listening  Settings");
                libcli_register_param(&tcp_server_name, &disable_client_listen);
                set_param_cmd_code(&disable_client_listen, TCP_SERVER_STOP_CLIENT_LISTEN);
            }
            {
                /* config tcp-server <name> stop */
                static param_t stop;
                init_param(&stop, CMD, "stop", config_tcp_server_handler, 0, INVALID, 0, "Stop TCP Server");
                libcli_register_param(&tcp_server_name, &stop);
                set_param_cmd_code(&stop, TCP_SERVER_STOP);
            }
            {
                /* config tcp-server <name> [<ip-addr>] ...*/
                static param_t tcp_server_addr;
                init_param (&tcp_server_addr, LEAF, 0, NULL, NULL, IPV4, "tcp-server-addr", "Tcp Server Address");
                libcli_register_param (&tcp_server_name, &tcp_server_addr);
                {
                     /* config tcp-server <name> [<ip-addr>] [<port-no>]*/
                     static param_t tcp_server_port;
                     init_param (&tcp_server_port, LEAF, 0, config_tcp_server_handler, 0, INT, "tcp-server-port", "Tcp Server Port Number");
                     libcli_register_param (&tcp_server_addr , &tcp_server_port);
                     set_param_cmd_code(&tcp_server_port, TCP_SERVER_CREATE);
                }
            }
            {
                /* config tcp-server <name> start */
                static param_t start;
                init_param (&start, CMD, "start", config_tcp_server_handler, NULL, INVALID, NULL, "Start");
                libcli_register_param (&tcp_server_name, &start);
                set_param_cmd_code(&start, TCP_SERVER_START);
            }
            support_cmd_negation(&tcp_server_name);
            /* do not add any param_t here */
        }
    }
    support_cmd_negation(config_hook);
}

static void
tcp_build_show_cli_tree() {
    
    param_t *show_hook = libcli_get_show_hook();

       /* show tcp-server ...*/
        static param_t tcp_server;
        init_param (&tcp_server, CMD, "tcp-server", NULL, NULL, INVALID, NULL, "show tcp-server");
        libcli_register_param (show_hook, &tcp_server);
        {
            /* show tcp-server <name> */
            static param_t tcp_server_name;
            init_param(&tcp_server_name, LEAF, NULL, show_tcp_server_handler, NULL, STRING, "tcp-server-name", "Tcp Server Name");
            libcli_register_param (&tcp_server, &tcp_server_name);
            set_param_cmd_code(&tcp_server_name, TCP_SERVER_SHOW_TCP_SERVER);
        }
}

static void
tcp_build_cli() {

    tcp_build_config_cli_tree();
    tcp_build_show_cli_tree();
}

int
main(int argc, char **argv) {

#if 0
    TcpServerController *server1 = new TcpServerController("127.0.0.1", 40000, "Default TCP Server");
    server1->SetServerNotifCallbacks(appln_client_connected, appln_client_disconnected, appln_client_msg_recvd);
    server1->Start();
    scanf("\n");
    server1->Display();
#endif

    init_libcli();

    tcp_build_cli();

    start_shell();
    /* dead code */
    return 0;
}
