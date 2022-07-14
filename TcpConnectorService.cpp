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
        assert (!tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECTED));
        assert(tcp_client->client_thread);

        tcp_client->StopThread();
        this->tcp_ctrlr->EnqueMsg (CTRLR_ACTION_TCP_CLIENT_DELETE, (void *)tcp_client, true);
        tcp_client->Dereference();
    }

    while (!this->establishedClient.empty()) {

        tcp_client = this->establishedClient.front();
        this->establishedClient.pop_front();
        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECTED));
        assert (tcp_client->IsStateSet(TCP_CLIENT_STATE_ACTIVE_OPENER));
        this->tcp_ctrlr->EnqueMsg(CTRLR_ACTION_TCP_CLIENT_DELETE, tcp_client, false);
        tcp_client->Dereference();
    }


    pthread_mutex_destroy(&this->mutex);
    pthread_cond_destroy(&this->cv);
}

void
TcpConnectorMgrSvc::ConnectorMgrServiceThreadInternal() {

    TcpClient *tcp_client;
    sem_t *sem;
    TcpConnectorMgrSvcMsg_t *msg;

    while (true) {

        pthread_mutex_lock(&this->mutex);

        while (this->msgQ.empty()) {

            pthread_cond_wait(&this->cv, &this->mutex);
        }

        while (1) {

            if (this->msgQ.empty()) break;

            msg = this->msgQ.front();
            this->msgQ.pop_front();
            sem = msg->zero_sema;

            switch (msg->mgs_type) {
            
                case CONNECTOR_SVC_CLIENT_TRY_CONNECT:
                    tcp_client = (TcpClient *)msg->data;
                    this->connectpendingClients.push_back(tcp_client);
                    tcp_client->Reference();

                    if (tcp_client->TryClientConnect(true) == 0) {

                        this->connectpendingClients.remove(tcp_client);
                        
                         this->establishedClient.push_back(tcp_client);
                        
                        this->tcp_ctrlr->EnqueMsg (CTRLR_ACTION_TCP_CLIENT_CREATE_THREADED,
                        (void *)tcp_client, false);
                    }
                    break;

                case CONNECTOR_SVC_CLIENT_CONNECT_SUCCESS:
                    this->connectpendingClients.remove(tcp_client);
                    this->establishedClient.push_back(tcp_client);
                    this->tcp_ctrlr->EnqueMsg (CTRLR_ACTION_TCP_CLIENT_CREATE_THREADED,
                        (void *)tcp_client, false);
                        break;

                case CONNECTOR_SVC_CLIENT_DISCONNECTED:
                    break;
                case CONNECTOR_SVC_CLIENT_CONNECT_FAILED:
                    break;
                case CONNECTOR_SVC_CLIENT_DELETE:
                    if (tcp_client->IsStateSet (TCP_CLIENT_STATE_CONNECTED)) {
                        this->establishedClient.remove(tcp_client);
                        tcp_client->Dereference();
                    }
                    else {
                        this->connectpendingClients.remove(tcp_client);
                        tcp_client->Dereference();
                    }
                break;
                default:
                    assert(0);
            }
            free(msg);
            if (sem) {
                sem_post(sem);
            }
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
                                    TcpConnectorSvcMsgRequestType msg_type, void *data, bool block_me) {

    sem_t sem;
    TcpConnectorMgrSvcMsg_t *msg;

    switch(msg_type) {
        case CONNECTOR_SVC_CLIENT_TRY_CONNECT:
        case CONNECTOR_SVC_CLIENT_CONNECT_SUCCESS:
        case CONNECTOR_SVC_CLIENT_DISCONNECTED:
        case CONNECTOR_SVC_CLIENT_CONNECT_FAILED:
        case CONNECTOR_SVC_CLIENT_DELETE:
            msg = (TcpConnectorMgrSvcMsg_t *)calloc (1, sizeof(TcpConnectorMgrSvcMsg_t));
            msg->mgs_type = msg_type;
            msg->data = data;
            
            if (block_me) {
                sem_init(&sem, 0, 0);
                msg->zero_sema = &sem;
            }
            else {
                msg->zero_sema = NULL;
            }

            pthread_mutex_lock(&this->mutex);
            this->msgQ.push_back(msg);
            pthread_cond_signal(&this->cv);
            pthread_mutex_unlock(&this->mutex);
            if (block_me) {
                sem_wait(&sem);
                sem_destroy(&sem);                
            }
            break;
        default:
            assert(0);
    }
}