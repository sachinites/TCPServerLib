#ifndef __TcpNewConnectionAcceptor__
#define __TcpNewConnectionAcceptor__

class TcpServerController;

class TcpNewConnectionAcceptor {

    private:

    public:
        TcpServerController *tcp_ctrlr;

        TcpNewConnectionAcceptor(TcpServerController *);
        ~TcpNewConnectionAcceptor();
};


#endif