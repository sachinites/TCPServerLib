#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <list>
#include <arpa/inet.h>
#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"
#include "TcpServerController.h"
#include "TcpClient.h"
#include "TcpCmdCodes.h"
#include "network_utils.h"

void client_connected_cbk(const TcpServerController *tcp_ctrlrl, const TcpClient *tcp_client);
void client_disconnected_cbk(const TcpServerController *tcp_ctrlr, const TcpClient *tcp_client) ;
void client_msg_recvd_cbk(const TcpServerController *tcp_ctrlrl, 
                                      const TcpClient *tcp_client, 
                                      unsigned char *msg,
                                      uint16_t msg_size) ;
void client_ka_pending_cbk(const TcpServerController *tcp_ctrlr, const TcpClient *tcp_client);


std::list<TcpServerController *> tcp_server_lst;
#define TCP_SERVER_DEFAULT_PORT_NO 40000

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
config_tcp_server_handler (param_t *param,
                                  ser_buff_t *tlv_buf, 
                                  op_mode enable_or_disable) {

    int rc = 0;
    int CMDCODE;
    uint16_t ka_interval;
    tlv_struct_t *tlv = NULL;
    TcpServerController *tcp_server = NULL;
    const char *ip_addr = "127.0.0.1";
    const char *tcp_server_name = NULL;
    const char *remote_ip_addr = NULL;
    uint16_t remote_port = 0;
    uint16_t port_no = TCP_SERVER_DEFAULT_PORT_NO;
    
    CMDCODE = EXTRACT_CMD_CODE(tlv_buf);

    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if     (strncmp(tlv->leaf_id, "tcp-server-name", strlen("tcp-server-name")) ==0)
            tcp_server_name = tlv->value;
        else if (strncmp(tlv->leaf_id, "tcp-server-addr", strlen("tcp-server-addr")) ==0)
            ip_addr = tlv->value;
        else if (strncmp(tlv->leaf_id, "tcp-server-port", strlen("tcp-server-port")) ==0)
            port_no = atoi(tlv->value);
        else if (strncmp(tlv->leaf_id, "ka-interval", strlen("ka-interval")) ==0)
            ka_interval =  atoi(tlv->value);
        else if (strncmp(tlv->leaf_id, "remote-addr", strlen("remote-addr")) ==0)
            remote_ip_addr =  tlv->value;
        else if (strncmp(tlv->leaf_id, "remote-port", strlen("remote-port")) ==0)
            remote_port = atoi(tlv->value);
        else
            assert(0);
    } TLV_LOOP_END;

    switch (CMDCODE) {
        case TCP_SERVER_CREATE:
            tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (tcp_server) {
                printf ("Error : Tcp Server Already Exist\n");
                return -1;
            }
            tcp_server = new TcpServerController(
                                std::string(ip_addr),
                                port_no,
                                std::string(tcp_server_name));

            if ( !tcp_server ) {
                printf ("Error : Tcp Server Creation Failed\n");
                return -1;
            }
            
            tcp_server->SetServerNotifCallbacks( 
                client_connected_cbk,
                client_disconnected_cbk,
                client_msg_recvd_cbk,
                client_ka_pending_cbk);

            tcp_server_lst.insert(tcp_server_lst.begin(), tcp_server);
            break;
        case TCP_SERVER_START:
            tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (!tcp_server) {
                printf ("Error : Tcp Server do not Exist\n");
                return -1;
            }
            tcp_server->Start();
            break;
        case TCP_SERVER_ABORT:
            tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (!tcp_server) {
                printf("Error : Tcp Server do not Exist\n");
                return -1;
            }
            tcp_server_lst.remove(tcp_server);
            tcp_server->Stop();
            break;
        case TCP_SERVER_SET_MULTITHREADED_MODE:
             tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (!tcp_server) {
                printf("Error : Tcp Server do not Exist\n");
                return -1;
            }
            switch(enable_or_disable) {
                case CONFIG_ENABLE:
                    tcp_server->SetClientCreationMode(true);
                    break;
                case CONFIG_DISABLE:
                    tcp_server->SetClientCreationMode(false);
                    break;
                default: ;
            }
        case TCP_SERVER_ALL_CLIENTS_SET_KA_INTERVAL:
             tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (!tcp_server) {
                printf("Error : Tcp Server do not Exist\n");
                return -1;
            }
            break;
        case TCP_SERVER_CONNECT_REMOTE:
            tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (!tcp_server) {
                printf("Error : Tcp Server do not Exist\n");
                return -1;
            }
            switch(enable_or_disable) {
                case CONFIG_ENABLE:
                    tcp_server->CreateActiveClient(network_covert_ip_p_to_n(remote_ip_addr), remote_port);
                    break;
                case CONFIG_DISABLE:
                    break;
            }
            break;
        case TCP_SERVER_STOP_CONN_ACCEPT:
            tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (!tcp_server) {
                printf("Error : Tcp Server do not Exist\n");
                return -1;
            }
            switch(enable_or_disable) {
                case CONFIG_ENABLE:
                    tcp_server->StopConnectionsAcceptorSvc();
                    break;
                case CONFIG_DISABLE:
                    tcp_server->StartConnectionsAcceptorSvc();
                    break;
            }             
            break;
        case TCP_SERVER_STOP_LISTENING_CLIENTS:
            tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (!tcp_server) {
                printf("Error : Tcp Server do not Exist\n");
                return -1;
            }
            switch(enable_or_disable) {
                case CONFIG_ENABLE:
                    tcp_server->StopClientSvcMgr();
                    break;
                case CONFIG_DISABLE:
                    tcp_server->StartClientSvcMgr();
                    break;
            }                  
            break;
        default:
            ;
    }
    return rc;
}

static void
tcp_build_config_cli_tree() {

    param_t *config_hook = libcli_get_config_hook();

    {
        /* config tcp-server ... */
        static param_t tcp_server;
        init_param(&tcp_server, CMD, "tcp-server", 0, 0, INVALID, 0, "config tcp-server");
        libcli_register_param(config_hook, &tcp_server);
        {
            /* config tcp-server <name> */
            static param_t tcp_server_name;
            init_param(&tcp_server_name, LEAF, 0, config_tcp_server_handler, 0, STRING, "tcp-server-name", "config tcp-server name");
            libcli_register_param(&tcp_server, &tcp_server_name);
            set_param_cmd_code(&tcp_server_name, TCP_SERVER_CREATE);
            {
                /* config tcp-server <name> start*/
                static param_t start;
                init_param(&start, CMD, "start", config_tcp_server_handler, 0, INVALID, 0, "start tcp-server");
                libcli_register_param(&tcp_server_name, &start);
                set_param_cmd_code(&start, TCP_SERVER_START);
            }
            {
                 /* config tcp-server <name> connect ...*/
                static param_t connect;
                init_param(&connect, CMD, "connect", 0, 0, INVALID, 0, "connect tcp-server");
                libcli_register_param(&tcp_server_name, &connect);
                {
                     /* config tcp-server <name> connect <ip-addr>...*/
                    static param_t remote_machine_addr;
                    init_param(&remote_machine_addr, LEAF, 0, 0, 0, IPV4, "remote-addr", "Remote IPV4 address");
                    libcli_register_param(&connect, &remote_machine_addr);
                    {
                        static param_t remote_machine_port;
                        init_param(&remote_machine_port, LEAF, 0, config_tcp_server_handler, 0, INT, "remote-port", "Remote Port Number");
                        libcli_register_param(&remote_machine_addr, &remote_machine_port);
                        set_param_cmd_code(&remote_machine_port, TCP_SERVER_CONNECT_REMOTE);
                    }
                }
            }
            {
                /* config tcp-server <name> stop*/
                static param_t stop;
                init_param(&stop, CMD, "abort", config_tcp_server_handler, 0, INVALID, 0, "stop tcp-server");
                libcli_register_param(&tcp_server_name, &stop);
                set_param_cmd_code(&stop, TCP_SERVER_ABORT);
            }
             {
                 /* config tcp-server <name>  multi-threaded-mode*/
                static param_t multi_threaded_mode;
                init_param(&multi_threaded_mode, CMD, "multi-threaded-mode", config_tcp_server_handler, 0, INVALID, 0, "create multi-threaded clients");
                libcli_register_param(&tcp_server_name, &multi_threaded_mode);
                set_param_cmd_code(&multi_threaded_mode, TCP_SERVER_SET_MULTITHREADED_MODE);
            }
            {
                 /* config tcp-server <name> [no] disable-conn-accept */
                static param_t dis_conn_accept;
                init_param(&dis_conn_accept, CMD, "disable-conn-accept", config_tcp_server_handler, 0, INVALID, 0, "Connection Accept Settings");
                libcli_register_param(&tcp_server_name, &dis_conn_accept);
                set_param_cmd_code(&dis_conn_accept, TCP_SERVER_STOP_CONN_ACCEPT);                 
            }
            {
                 /* config tcp-server <name> [no] client-listen */
                static param_t client_listen;
                init_param(&client_listen, CMD, "disable-client-listen", config_tcp_server_handler, 0, INVALID, 0, "Client Listening Settings");
                libcli_register_param(&tcp_server_name, &client_listen);
                set_param_cmd_code(&client_listen, TCP_SERVER_STOP_LISTENING_CLIENTS);                        
            }
            {
                /* config tcp-server <name> [no] ka-interval <N> */
                static param_t ka_interval;
                init_param(&ka_interval, CMD, "ka-interval", 0, 0, INVALID, 0, "config KA Interval");
                libcli_register_param(&tcp_server_name, &ka_interval);
                {
                     static param_t ka_interval_val;
                     init_param(&ka_interval_val, LEAF, 0, config_tcp_server_handler, 0,  INT, "ka-interval", "config tAK interval value in sec");
                     libcli_register_param(&ka_interval, &ka_interval_val);
                    set_param_cmd_code(&ka_interval_val, TCP_SERVER_ALL_CLIENTS_SET_KA_INTERVAL);
                }
            }
            {
                 /* config tcp-server <name> [<ip-addr>] */
                 static param_t tcp_server_addr;
                 init_param(&tcp_server_addr, LEAF, 0, config_tcp_server_handler, 0, IPV4, "tcp-server-addr", "config tcp-server address");
                 libcli_register_param(&tcp_server_name, &tcp_server_addr);
                 set_param_cmd_code(&tcp_server_name, TCP_SERVER_CREATE);
                 {
                     /* config tcp-server <name> [<ip-addr>][ <port-no> ]*/
                     static param_t tcp_server_port;
                     init_param(&tcp_server_port, LEAF, 0, config_tcp_server_handler, 0,  INT, "tcp-server-port", "config tcp-server port no");
                     libcli_register_param(&tcp_server_addr, &tcp_server_port);
                     set_param_cmd_code(&tcp_server_port, TCP_SERVER_CREATE);
                 }
            }
             support_cmd_negation(&tcp_server_name);
        }
    }
    support_cmd_negation(config_hook);
}

static int
run_tcp_server_handler (param_t *param,
                                             ser_buff_t *tlv_buf, 
                                             op_mode enable_or_disable) {

    const char *tcp_server_name = NULL;
    char *msg = NULL;
    tlv_struct_t *tlv = NULL;
    const char *remote_ip_addr = NULL;
    uint16_t remote_port = 0;
    TcpServerController *tcp_server = NULL;

    int CMDCODE = EXTRACT_CMD_CODE(tlv_buf);

     TLV_LOOP_BEGIN(tlv_buf, tlv){

        if     (strncmp(tlv->leaf_id, "tcp-server-name", strlen("tcp-server-name")) ==0)
            tcp_server_name = tlv->value;
        else if (strncmp(tlv->leaf_id, "msg", strlen("msg")) ==0)
            msg = tlv->value;
        else if (strncmp(tlv->leaf_id, "remote-addr", strlen("remote-addr")) ==0)
            remote_ip_addr =  tlv->value;
        else if (strncmp(tlv->leaf_id, "remote-port", strlen("remote-port")) ==0)
            remote_port = atoi(tlv->value);
        else
            assert(0);
     } TLV_LOOP_END;

    switch (CMDCODE) {
        case RUN_CMD_CODE_TCP_SERVER_SENDMSG:
            tcp_server = TcpServer_lookup(std::string(tcp_server_name));
            if (!tcp_server) {
                printf ("Error : Tcp Server do not Exist\n");
                return -1;
            }
            TcpClient *tcp_client ;
            tcp_client = tcp_server->LookupActiveOpened(
                network_covert_ip_p_to_n(remote_ip_addr), remote_port);
            if (!tcp_client) {
                printf ("Error : Remote Machine Connection not found\n");
                return -1;
            }
            tcp_client->SendMsg(msg, strlen(msg));
            break;
        default: ;
    }
    return 0;
}

static void
tcp_build_run_cli_tree() {

    param_t *run_hook = libcli_get_run_hook();
    /* run tcp-server ... */
    static param_t tcp_server;
    init_param(&tcp_server, CMD, "tcp-server", 0, 0, INVALID, 0, "run tcp-server");
    libcli_register_param(run_hook, &tcp_server);
    {
        /*run  tcp-server <name> */
        static param_t tcp_server_name;
        init_param(&tcp_server_name, LEAF, 0, NULL, 0, STRING, "tcp-server-name", "TCP Server name");
        libcli_register_param(&tcp_server, &tcp_server_name);
        {
            /*run  tcp-server <name> send-msg ...*/
            static param_t send_msg;
            init_param(&send_msg, CMD, "send-msg", 0, 0, INVALID, 0, "Send Msg");
            libcli_register_param(&tcp_server_name, &send_msg);
            {
                /*run  tcp-server <name> send-msg <test-msg> <remote-ip-addr> <remote-port>*/
                static param_t remote_machine_addr;
                init_param(&remote_machine_addr, LEAF, 0, 0, 0, IPV4, "remote-addr", "Remote IPV4 address");
                libcli_register_param(&send_msg, &remote_machine_addr);
                {
                    static param_t remote_machine_port;
                    init_param(&remote_machine_port, LEAF, 0, NULL, 0, INT, "remote-port", "Remote Port Number");
                    libcli_register_param(&remote_machine_addr, &remote_machine_port);
                    {
                        static param_t msg;
                        init_param(&msg, LEAF, 0, run_tcp_server_handler, 0, STRING, "msg", "Message to be Sent");
                        libcli_register_param(&remote_machine_port, &msg);
                        set_param_cmd_code(&msg, RUN_CMD_CODE_TCP_SERVER_SENDMSG);
                    }
                }
            }
        }
    }
}

static int
show_tcp_server_handler (param_t *param,
                                  ser_buff_t *tlv_buf, 
                                  op_mode enable_or_disable) {

    int rc = 0;
    int CMDCODE;
    tlv_struct_t *tlv = NULL;
    TcpServerController *tcp_server = NULL;
    const char *tcp_server_name = NULL;

    CMDCODE = EXTRACT_CMD_CODE(tlv_buf);

    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if     (strncmp(tlv->leaf_id, "tcp-server-name", strlen("tcp-server-name")) ==0)
            tcp_server_name = tlv->value;
        else
            assert(0);
    } TLV_LOOP_END;

    switch (CMDCODE) {
        case TCP_SERVER_SHOW_TCP_SERVER:
            tcp_server = TcpServer_lookup(std::string(tcp_server_name));  
            if (!tcp_server) {
                printf ("Error : Tcp Server do not Exist\n");
                return -1;
            }
            tcp_server->Display();
            break;
        default:
            ;
    }
    return rc;
}

static void
tcp_build_show_cli_tree() {

    param_t *show_hook = libcli_get_show_hook();

    /* show tcp-server ... */
        static param_t tcp_server;
        init_param(&tcp_server, CMD, "tcp-server", 0, 0, INVALID, 0, "config tcp-server");
        libcli_register_param(show_hook, &tcp_server);
        {
            /*show  tcp-server <name> */
            static param_t tcp_server_name;
            init_param(&tcp_server_name, LEAF, 0, show_tcp_server_handler, 0, STRING, "tcp-server-name", "config tcp-server name");
            libcli_register_param(&tcp_server, &tcp_server_name);
            set_param_cmd_code(&tcp_server_name, TCP_SERVER_SHOW_TCP_SERVER);
        }
}

void
tcp_build_cli() {

    tcp_build_config_cli_tree();
    tcp_build_run_cli_tree();
    tcp_build_show_cli_tree();
}

/* Default Callbacks, used when TCPServerLib is run via CLI instead of application */
static void
print_client(const TcpClient *client) {

    printf ("[%s , %d]\n", network_convert_ip_n_to_p((client->ip_addr), 0),
                (client->port_no));
}

static void
print_server(const TcpServerController *tcp_server) {

     printf ("[%s , %d]\n", network_convert_ip_n_to_p((tcp_server->ip_addr), 0),
                (tcp_server->port_no));
}


void
client_connected_cbk(const TcpServerController *tcp_ctrlr, const TcpClient *tcp_client) {

    print_server(tcp_ctrlr);
    printf ("Appln : client connected : ");
    print_client(tcp_client);
}

void
client_disconnected_cbk(const TcpServerController *tcp_ctrlr, const TcpClient *tcp_client) {

    print_server(tcp_ctrlr);
    printf ("Appln : client disconnected : ");
    print_client(tcp_client);
}

void
client_msg_recvd_cbk(const TcpServerController *tcp_ctrlr, const TcpClient *tcp_client, 
                               unsigned char *msg, uint16_t msg_size) {

    print_server(tcp_ctrlr);
    printf ("Appln : client msg recvd = %dB : ", msg_size);
    print_client(tcp_client);
    printf ("msg = %s\n", msg);
}

void client_ka_pending_cbk(const TcpServerController *tcp_ctrlr, const TcpClient *tcp_client) {

    print_server(tcp_ctrlr);
    printf ("Appln : client KA Pending :  ");
    print_client(tcp_client);
}