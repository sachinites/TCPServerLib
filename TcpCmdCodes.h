#ifndef __TCP_CMDCODES__
#define __TCP_CMDCODES__

/* TCP Server Operations */
#define TCP_SERVER_CREATE   1  // config tcp-server <name> [<ip addr>] [<port no>]
#define TCP_SERVER_ABORT    2   // config tcp-server <name> abort
#define TCP_SERVER_SET_MAX_CLIENT_LIMIT 3 // config tcp-server <name> [no] max-client-limit <N>
#define TCP_SERVER_RESET    4  // run tcp-server <name> reset
#define TCP_SERVER_STOP_CONN_ACCEPT 5   // config tcp-server <name> [no] disable-conn-accept
#define TCP_SERVER_DISCONNECT_CLIENT    6 // run tcp-server <name> client disconnect <ip-addr> <port-no>
#define TCP_SERVER_BLACK_LIST_CLIENT    7 // config tcp-server <name> [no] client black-list <ip-addr>
#define TCP_SERVER_SHOW_TCP_SERVER  8   // show tcp-server <name>
#define TCP_SERVER_ALL_CLIENTS_SET_KA_INTERVAL 9    // config tcp-server <name> [no] ka-interval <N>
#define TCP_SERVER_ONE_CLIENT_SET_KA_INTERVAL 10    // config tcp-server <name> [no] ka-interval <N> client <ip-addr> 
#define TCP_SERVER_SET_MULTITHREADED_MODE   11  // config tcp-server <name> [no] multi-threaded
#define TCP_SERVER_SET_MULTITHREADED_NODE_ONE_CLIENT    12 // config tcp-server <name> [no] client <ip-addr> multi-threaded
#define TCP_SERVER_SHOW_ONE_CLIENT_STATS    13 // show tcp-server <name> client <ip-addr> stats
#define TCP_SERVER_STOP_LISTENING_CLIENTS   14  // config tcp-server <name> [no] client-listen
#define TCP_SERVER_STOP_LISTENING_ONE_CLIENT   15  // config tcp-server <name> [no] client <ip-addr> listen
#define TCP_SERVER_START    16 // config tcp-server <name> start
#define TCP_SERVER_CONNECT_REMOTE   17 // config tcp-server s1 connect <ip-addr> <port no>

/* TCP Client Operations */
#define TCP_CLIENT_CREATE   1 // config tcp-client <name> [<src-ip-addr>] [<src-port-no>]
#define TCP_CLIENT_ABORT    2 // config tcp-client <name> abort
#define TCP_CLIENT_DISCONNECT   3 // run tcp-client <name> disconnect
#define TCP_CLIENT_SHOW_STATE   4 // show tcp-client <name>

/* Run Commands */
#define RUN_CMD_CODE_TCP_SERVER_SENDMSG 1
#endif 