#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "TcpServerController.h"
#include "TcpClient.h"
#include "TcpConnectorService.h"
#include "network_utils.h"

TcpConnectorMgrSvc::TcpConnectorMgrSvc(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;
    pthread_mutex_init (&this->mutex, NULL);
    pthread_cond_init (&this->cv, NULL);
}

TcpConnectorMgrSvc::~TcpConnectorMgrSvc() {

    assert(!this->tcp_ctrlr);
}

void
TcpConnectorMgrSvc::Stop() {

    /* Terminate all threads which are busy in looking for connections */
    TcpClient *tcp_client;

    pthread_cancel(this->connector_svc_thread);
    pthread_join (this->connector_svc_thread, NULL);

    while (!this->connectpendingClients.empty()) {

        tcp_client = this->connectpendingClients.front();
        this->connectpendingClients.pop_front();

        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECT_IN_PROGRESS));
        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_ACTIVE_OPENER));
        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_NOT_CONNECTED));
        assert(tcp_client->client_thread);

        tcp_client->StopThread();
        this->tcp_ctrlr->EnqueueClientDeletionRequest(tcp_client);
        tcp_client->Dereference();
    }

    while (!this->connectionInProgressClients.empty()) {

        tcp_client = this->connectionInProgressClients.front();
        this->connectionInProgressClients.pop_front();
        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECTED));
        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_ACTIVE_OPENER));
        assert(!tcp_client->client_thread);
        this->tcp_ctrlr->EnqueueClientDeletionRequest(tcp_client);
        tcp_client->Dereference();
    }

    while (!this->connectionInProgressClients.empty()) {

        tcp_client = this->connectionInProgressClients.front();
        this->connectionInProgressClients.pop_front();
        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_ESTABLISHED));
        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_ACTIVE_OPENER));
        this->tcp_ctrlr->EnqueueClientDeletionRequest(tcp_client);
        tcp_client->Dereference();
    }


    pthread_mutex_destroy(&this->mutex);
    pthread_cond_destroy(&this->cv);
}

void
TcpConnectorMgrSvc::ConnectorMgrServiceThreadInternal() {

    TcpClient *tcp_client;
    TcpConnectorMgrSvcMsg_t *msg;

    while (true) {

        pthread_mutex_lock(&this->mutex);

        while (this->msgQ.empty()) {

            pthread_cond_wait(&this->cv, &this->mutex);
        }

        while (1) {

            msg = this->msgQ.front();
            if (!msg) break;
            switch (msg->mgs_type) {
                case CLIENT_TRY_CONNECT:
                    tcp_client = (TcpClient *)msg->data;
                    tcp_client->Dereference();
                    TcpConnectorMgrSvc::TryClientConnect(tcp_client, true);
                    break;
                case CLIENT_CONNECT_SUCCESS:
                case CLIENT_DISCONNECTED:
                case CLIENT_CONNECT_FAILED:
                default:
                    assert(0);
            }
            free(msg);
        }
        pthread_mutex_unlock(&this->mutex);
    }
}

static void *
connector_svc_thread_fn (void *arg) {

    TcpConnectorMgrSvc *conn_svc_mgr = (TcpConnectorMgrSvc *)arg;

    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL);

    conn_svc_mgr->ConnectorMgrServiceThreadInternal();
    return NULL;
}


void
TcpConnectorMgrSvc::StartConnectorMgrServiceThread() {

    pthread_create (&this->connector_svc_thread, NULL, connector_svc_thread_fn, (void *)this);
}

void 
TcpConnectorMgrSvc::EnqueueProcessRequest (
                                    TcpConnectorSvcMsgRequestType msg_type, void *data) {

    TcpConnectorMgrSvcMsg_t *msg;

    switch(msg_type) {
        case CLIENT_TRY_CONNECT:
            msg = (TcpConnectorMgrSvcMsg_t *)calloc (1, sizeof(TcpConnectorMgrSvcMsg_t));
            msg->mgs_type = msg_type;
            msg->data = data;
            pthread_mutex_lock(&this->mutex);
            this->msgQ.push_back(msg);
            pthread_cond_signal(&this->cv);
            pthread_mutex_unlock(&this->mutex);
            break;
        case CLIENT_CONNECT_SUCCESS:
            break;
        case CLIENT_DISCONNECTED:
            break;
        case CLIENT_CONNECT_FAILED:
            break;
        default:
            assert(0);
    }
}