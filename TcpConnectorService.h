#ifndef __TCP_CONNECTOR__
#define __TCP_CONNECTOR__

#include <list>
#include <pthread.h>
#include <stdbool.h>
#include "libtimer/WheelTimer.h"

class TcpClient;
class TcpServerController;

typedef enum TcpConnectorSvcMsgRequest_ {

    CLIENT_TRY_CONNECT,
    CLIENT_CONNECT_SUCCESS,
    CLIENT_DISCONNECTED,
    CLIENT_CONNECT_FAILED

} TcpConnectorSvcMsgRequestType;

typedef struct  TcpConnectorMgrSvcMsg_ {

    TcpConnectorSvcMsgRequestType mgs_type;
    void *data;
}TcpConnectorMgrSvcMsg_t;

class TcpConnectorMgrSvc {

    private:
        std::list<TcpConnectorMgrSvcMsg_t *> msgQ;
        pthread_t connector_svc_thread;
        pthread_mutex_t mutex;
        pthread_cond_t cv;

        std::list<TcpClient *> connectionInProgressClients;
        std::list<TcpClient *> establishedClient;
        std::list<TcpClient *> connectpendingClients;

        TcpServerController *tcp_ctrlr;
    public:
        TcpConnectorMgrSvc(TcpServerController *);
        ~TcpConnectorMgrSvc();
        static int TryClientConnect(TcpClient *, bool);
        void StartConnectorMgrServiceThread();
        void ConnectorMgrServiceThreadInternal();
        void Stop();
        void EnqueueProcessRequest (TcpConnectorSvcMsgRequestType msg_type, void *data);
};
#endif /* __TCP_CONNECTOR__ */
