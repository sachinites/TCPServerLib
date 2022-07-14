#ifndef __TCP_CONNECTOR__
#define __TCP_CONNECTOR__

#include <list>
#include <pthread.h>
#include <stdbool.h>
#include "libtimer/WheelTimer.h"

class TcpClient;
class TcpServerController;

typedef enum TcpConnectorSvcMsgRequest_ {

    CONNECTOR_SVC_CLIENT_TRY_CONNECT,
    CONNECTOR_SVC_CLIENT_CONNECT_SUCCESS,
    CONNECTOR_SVC_CLIENT_DISCONNECTED,
    CONNECTOR_SVC_CLIENT_CONNECT_FAILED,
    CONNECTOR_SVC_CLIENT_DELETE

} TcpConnectorSvcMsgRequestType;

typedef struct  TcpConnectorMgrSvcMsg_ {

    TcpConnectorSvcMsgRequestType mgs_type;
    void *data;
    sem_t *zero_sema;
}TcpConnectorMgrSvcMsg_t;

class TcpConnectorMgrSvc {

    private:
        std::list<TcpConnectorMgrSvcMsg_t *> msgQ;
        pthread_t connector_svc_thread;
        pthread_mutex_t mutex;
        pthread_cond_t cv;

        std::list<TcpClient *> establishedClient;
        std::list<TcpClient *> connectpendingClients;

        TcpServerController *tcp_ctrlr;
    public:
        TcpConnectorMgrSvc(TcpServerController *);
        ~TcpConnectorMgrSvc();
        void StartConnectorMgrServiceThread();
        void ConnectorMgrServiceThreadInternal();
        void Stop();
        void EnqueueProcessRequest (TcpConnectorSvcMsgRequestType msg_type, void *data, bool block_me);
};
#endif /* __TCP_CONNECTOR__ */
