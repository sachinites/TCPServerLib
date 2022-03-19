#include <assert.h>
#include "TcpServerController.h"
#include "TcpClient.h"
#include "TcpConnectorService.h"

TcpConnectorMgr::TcpConnectorMgr(TcpServerController *tcp_ctrlr) {

    this->tcp_ctrlr = tcp_ctrlr;
    this->connector_timer = init_wheel_timer(60, 1, TIMER_SECONDS);
}

TcpConnectorMgr::~TcpConnectorMgr() {

    assert(!this->tcp_ctrlr);
    assert(!this->connector_timer);
}

void
TcpConnectorMgr::StopConnectorMgrService() {

    this->tcp_ctrlr = NULL;
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
