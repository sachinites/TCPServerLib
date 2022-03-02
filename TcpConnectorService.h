#ifndef __TCP_CONNECTOR__
#define __TCP_CONNECTOR__

#include <list>
#include <pthread.h>
#include <stdbool.h>
#include "libtimer/WheelTimer.h"

class TcpClient;
class TcpServer;

class TcpConnectorMgr {

    private:
        list<TcpClient *> pendingClient;
        list<TcpClient *> connectedClient;
        wheel_timer_t *connector_timer;
        TcpServer *tcp_server;
        bool TryClientConnect(TcpClient *);
        void MoveClientFromPendingListtoConnectList(TcpClient *);
        void MoveClientFromConnectListtoPendingList(TcpClient *);
        void ClientProcessDisconnect(TcpClient *);
        void ClientProcessConnect(TcpClient *);
    public:
        TcpConnectorMgr(TcpServer *);
        ~TcpConnectorMgr();
        void StartConnectorMgrService();
        void StopConnectorMgrService();
};
#endif /* __TCP_CONNECTOR__ */