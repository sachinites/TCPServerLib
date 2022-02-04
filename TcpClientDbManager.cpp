#include <assert.h>
#include "TcpClientDbManager.h"
#include "TcpServer.h"
#include "TcpClient.h"

TcpClientDbManager::TcpClientDbManager(TcpServer *tcp_server) {

    this->tcp_server = tcp_server;
    sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);
    client_db_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    pthread_mutex_init (&this->request_q_mutex, NULL);
    pthread_cond_init(&this->request_q_cv, NULL);
}

TcpClientDbManager::~TcpClientDbManager() {


    assert(this->tcp_client_db.empty());
    assert(!this->client_db_mgr_thread);
    assert(!this->request_q.empty());
}

void 
TcpClientDbManager::EnqueClientProcessingRequest
            (TcpClient *tcp_client, ClientDBRequestOpnCode code) {

    TcpClientDbRequest *req = new TcpClientDbRequest();
    req->Code = code;
    req->tcp_client = tcp_client;

    /* Implement Producer Logic here : Begin */
    pthread_mutex_lock(&this->request_q_mutex);

    if (this->request_q.empty())
    {
        pthread_cond_signal(&this->request_q_cv);
    }

    this->request_q.push_back(req);
    pthread_mutex_unlock(&this->request_q_mutex);
    /* Implement Producer Logic here : End */
    }

TcpClient * 
TcpClientDbManager::LookUpClientDB(uint32_t ip_addr, uint16_t port_no) {

    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it) {

        tcp_client = *it;
        if (tcp_client->ip_addr == ip_addr &&
                tcp_client->port_no == port_no) return tcp_client;
    }
    return NULL;
}

void
TcpClientDbManager::AddNewClient(TcpClient *tcp_client) {

    assert(!this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));
    this->tcp_client_db.push_back(tcp_client);
    this->tcp_server->AddNewClientFD(tcp_client);
}

void
TcpClientDbManager::DeleteClient(TcpClient *tcp_client) {
    
    this->tcp_server->RemoveClientFD(tcp_client);
    this->tcp_client_db.remove(tcp_client);
    tcp_client->Abort();
}
void 
TcpClientDbManager::DeleteAllClients() {
    
}
void 
TcpClientDbManager::UpdateClient(TcpClient *tcp_client) {
    
}

void
TcpClientDbManager::ProcessRequest(TcpClientDbRequest *request) {

    switch (request->Code) {
        case TCP_DB_MGR_NEW_CLIENT_CREATE:
            this->AddNewClient(request->tcp_client);
            break;
        case TCP_DB_MGR_DELETE_EXISTING_CLIENT:
            this->DeleteClient(request->tcp_client);
            break;
        case TCP_DB_MGR_UPDATE_CLIENT:
            this->UpdateClient(request->tcp_client);
            break;
        case TCP_DB_MGR_DELETE_ALL_CLIENTS:
            this->DeleteAllClients();
            break;
        default:;
    }
}

void 
TcpClientDbManager::StartClientDbManagerThreadInternal() {

    sem_post(&this->wait_for_thread_operation_to_complete);

    while (true) {

        pthread_testcancel();

         /* Implement Consumer Logic here : Begin */
        pthread_mutex_lock(&this->request_q_mutex);

        while (this->request_q.empty()) {
            pthread_cond_wait(&this->request_q_cv, &this->request_q_mutex);
        }

        TcpClientDbRequest *request = (TcpClientDbRequest *) (this->request_q.front());
        this->request_q.pop_front();

        pthread_mutex_unlock(&this->request_q_mutex);
         /* Implement Consumer Logic Ends : Begin */

        this->ProcessRequest(request);
        delete request;
    }
}

static void *
tcp_client_db_manage(void *arg) {

    TcpClientDbManager *db_mgr = 
        (TcpClientDbManager *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    db_mgr->StartClientDbManagerThreadInternal();
    return NULL;
}

void 
TcpClientDbManager::StartClientDbManagerThread() {

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(this->client_db_mgr_thread, &attr, tcp_client_db_manage, (void *)this);

    sem_wait(&this->wait_for_thread_operation_to_complete);
    printf("TcpClientDbManagerThread Started\n");
}

void
TcpClientDbManager::SetClientDisconnectCbk(void (*client_disconnected)(const TcpClient *)) {

    this->client_disconnected = client_disconnected;
}

void
TcpClientDbManager::SetClientKAPending(void (*client_ka_pending)(const TcpClient *)) {

    this->client_ka_pending = client_ka_pending;
}

void
TcpClientDbManager::AbortAllClients() {


}

void
TcpClientDbManager::PurgeRequestQueue() {


}


void
TcpClientDbManager::Stop() {

    this->StopClientDbManagerThread();
    PurgeRequestQueue();
    AbortAllClients();
    sem_destroy(&this->wait_for_thread_operation_to_complete);
    sem_destroy(&this->sem0_1);
    sem_destroy(&this->sem0_2);
    pthread_mutex_destroy(&this->request_q_mutex);
    pthread_cond_destroy(&this->request_q_cv);
}

void
TcpClientDbManager::StopClientDbManagerThread() {

    pthread_cancel(*this->client_db_mgr_thread);
    pthread_join(*this->client_db_mgr_thread, NULL);
    free(this->client_db_mgr_thread);
    this->client_db_mgr_thread = NULL;
}