#include "TcpClientDbManager.h"
#include "TcpServer.h"
#include "TcpClient.h"

TcpClientDbManager::TcpClientDbManager(TcpServer *tcp_server) {

    this->tcp_server = tcp_server;
    this->shared_binary_semaphore1 = &tcp_server->binary_semaphore_1;
    this->shared_binary_semaphore2 = &tcp_server->binary_semaphore_2;
    sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);
    client_db_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    pthread_mutex_init (&this->request_q_mutex, NULL);
    pthread_cond_init(&this->request_q_cv, NULL);
}

TcpClientDbManager::~TcpClientDbManager() {

    sem_destroy(&this->wait_for_thread_operation_to_complete);
    free(client_db_mgr_thread);
    pthread_mutex_destroy(&this->request_q_mutex);
    pthread_cond_destroy(&this->request_q_cv);
}

void 
TcpClientDbManager::NewClientCreationRequest
            (TcpClient *tcp_client_template, ClientDBRequestOpnCode code) {

    TcpClient *tcp_client;
    switch (code) {

    case TCP_DB_MGR_NEW_CLIENT_CREATE:
        tcp_client = new TcpClient(tcp_client_template);
        this->tcp_client_db.push_back(tcp_client);
        this->tcp_server->tcp_client_svc_mgr->add_new_client_fd(tcp_client);
        break
    case TCP_DB_MGR_DELETE_EXISTING_CLIENT:
    case TCP_DB_MGR_UPDATE_CLIENT:
    case TCP_DB_MGR_DELETE_ALL_CLIENTS:
    default:;
    }
}

void
TcpClientDbManager::ProcessRequest(TcpClientDbRequest *request) {
    
        
}

void 
TcpClientDbManager::StartClientDbManagerThreadInternal() {


    sem_post(&this->wait_for_thread_operation_to_complete);

    while (true) {

        pthread_mutex_lock(&this->request_q_mutex);

        while (this->request_q.empty()) {
            pthread_cond_wait(&this->request_q_cv, &this->request_q_mutex);
        }

        TcpClientDbRequest *request = (TcpClientDbRequest *) (this->request_q.pop_back());

        pthread_mutex_unlock(&this->request_q_mutex);

        this->ProcessRequest(request);
    }
}

static void *
tcp_client_db_manage(void *arg) {

    TcpClientDbManager *db_mgr = 
        (TcpClientDbManager *)arg;
    
    db_mgr->StartClientDbManagerThreadInternal();
    return NULL;
}

void 
TcpClientDbManager::StartClientDbManagerThread() {

    pthread_attr_t attr;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

     pthread_create(this->client_db_mgr_thread, &attr,  tcp_client_db_manage, (void *)this);

      sem_wait (&this->wait_for_thread_operation_to_complete);
      printf ("ClientDbManagerThread Started\n");
}

void
TcpClientDbManager::StopClientDbManagerThread() {

}