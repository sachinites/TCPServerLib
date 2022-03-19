#include <assert.h>
#include "TcpServer.h"
#include "TcpClient.h"
#include "TcpConnectorService.h"

TcpConnectorMgr::TcpConnectorMgr(TcpServer *tcp_server) {

    this->tcp_server = tcp_server;
    this->connector_timer = init_wheel_timer(60, 1, TIMER_SECONDS);
}

TcpConnectorMgr::~TcpConnectorMgr() {

    assert(!this->tcp_server);
    assert(!this->connector_timer);
}

void
TcpConnectorMgr::StopConnectorMgrService() {

    this->tcp_server = NULL;
    if (this->connector_timer) {
        cancel_wheel_timer(this->connector_timer);
        free(this->connector_timer);
        this->connector_timer = NULL;
    }
}

void
TcpConnectorMgr::StartConnectorMgrService() {

   start_wheel_timer(this->connector_timer);
}