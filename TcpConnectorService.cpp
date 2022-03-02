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

TcpConnectorMgr::StopConnectorMgrService() {

    this->tcp_server = NULL;
    
    WheelTimerStop(this->connector_timer);
    WheelTimerDestroy(this->connector_timer);
    this->connector_timer = NULL;
}