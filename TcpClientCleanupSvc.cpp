#include <stdio.h>
#include <stdlib.h>
#include "TcpClientCleanupSvc.h"
#include "TcpServerController.h"
#include "TcpClient.h"
#include "network_utils.h"

TcpClientCleanupSvc::TcpClientCleanupSvc(TcpServerController *tcp_ctrlr) {

    pthread_mutex_init (&this->mutex, NULL);
    pthread_cond_init (&this->cv, NULL);
    this->tcp_ctrlr = tcp_ctrlr;
}

TcpClientCleanupSvc::~TcpClientCleanupSvc() {
    
    /* Nothing to do */
}

void
TcpClientCleanupSvc::TcpServerClientCleanupThreadInternal() {

    TcpClient *tcp_client;

    while (true) {

        pthread_mutex_lock(&this->mutex);

        while (this->cleanup_lst.empty()) {

            pthread_cond_wait(&this->cv, &this->mutex);

        }

        while (1) {

            tcp_client = this->cleanup_lst.front();
            if (!tcp_client) break;
            this->cleanup_lst.pop_front();
            tcp_client->Dereference();
            this->tcp_ctrlr->ProcessClientDelete(tcp_client);
        }

        pthread_mutex_unlock(&this->mutex);
    }
}

static void *
tcp_server_cleanup_fn (void *arg) {

    TcpClientCleanupSvc *cleanup_svc = (TcpClientCleanupSvc *)arg;

    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL);

    cleanup_svc->TcpServerClientCleanupThreadInternal();
    return NULL;
}

void 
TcpClientCleanupSvc::StartTcpServerClientCleanupThread() {

        pthread_create (&this->client_cleanup_thread,
                              NULL,
                              tcp_server_cleanup_fn, (void *) this);
}

void 
TcpClientCleanupSvc::StopTcpServerClientCleanupThread() {

    pthread_cancel(this->client_cleanup_thread);
    pthread_join (this->client_cleanup_thread, NULL);
}

void
TcpClientCleanupSvc::Stop() {

    TcpClient *tcp_client;

    this->StopTcpServerClientCleanupThread();

    while (1) {

        tcp_client = this->cleanup_lst.front();
        if (!tcp_client)
            break;
        this->cleanup_lst.pop_front();
        tcp_client->Dereference();
        this->tcp_ctrlr->ProcessClientDelete(tcp_client);
    }

    pthread_mutex_destroy(&this->mutex);
    pthread_cond_destroy(&this->cv);    
}

void 
TcpClientCleanupSvc::EnqueueClientDeletionRequest(TcpClient *tcp_client) {

    pthread_mutex_lock (&this->mutex);
    this->cleanup_lst.push_back(tcp_client);
    tcp_client->Reference();
    pthread_cond_signal(&this->cv);
    pthread_mutex_unlock (&this->mutex);
}