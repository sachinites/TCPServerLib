CC=g++
CFLAGS=-g
TARGET:testapp.exe tcp_client.exe
LIBS=-lpthread -lrt -L libtimer -ltimer -L CommandParser -lcli
OBJS=TcpClient.o \
	 		TcpConn.o \
			TcpClientDbManager.o  \
			TcpClientServiceManager.o \
			TcpNewConnectionAcceptor.o \
			TcpServerController.o \
			network_utils.o	\
			TcpMsgDemarcar.o \
			TcpMsgFixedSizeDemarcar.o \
			ByteCircularBuffer.o \
			TcpMsgVariableSizeDemarcar.o \
			TcpServerCli.o \
			libtimer/libtimer.a	\
			CommandParser/libcli.a \

testapp.exe:testapp.o ${OBJS}
	${CC} ${CFLAGS} ${OBJS} testapp.o -o testapp.exe ${LIBS}

tcp_client.exe:tcp_client.o
	${CC} ${CFLAGS} tcp_client.o -o tcp_client.exe
	
TcpClient.o:TcpClient.cpp
	${CC} ${CFLAGS} -c TcpClient.cpp -o TcpClient.o

TcpConn.o:TcpConn.cpp
	${CC} ${CFLAGS} -c TcpConn.cpp -o TcpConn.o

tcp_client.o:tcp_client.cpp
	${CC} ${CFLAGS} -c tcp_client.cpp -o tcp_client.o

testapp.o:testapp.cpp
	${CC} ${CFLAGS} -c testapp.cpp -o testapp.o

TcpClientDbManager.o:TcpClientDbManager.cpp
	${CC} ${CFLAGS} -c TcpClientDbManager.cpp -o TcpClientDbManager.o

TcpClientServiceManager.o:TcpClientServiceManager.cpp
	${CC} ${CFLAGS} -c TcpClientServiceManager.cpp -o TcpClientServiceManager.o

TcpNewConnectionAcceptor.o:TcpNewConnectionAcceptor.cpp
	${CC} ${CFLAGS} -c TcpNewConnectionAcceptor.cpp -o TcpNewConnectionAcceptor.o

TcpServerController.o:TcpServerController.cpp
	${CC} ${CFLAGS} -c TcpServerController.cpp -o TcpServerController.o

network_utils.o:network_utils.cpp
	${CC} ${CFLAGS} -c network_utils.cpp -o network_utils.o

TcpMsgFixedSizeDemarcar.o:TcpMsgFixedSizeDemarcar.cpp
	${CC} ${CFLAGS} -c TcpMsgFixedSizeDemarcar.cpp -o TcpMsgFixedSizeDemarcar.o

TcpMsgDemarcar.o:TcpMsgDemarcar.cpp
	${CC} ${CFLAGS} -c TcpMsgDemarcar.cpp -o TcpMsgDemarcar.o

ByteCircularBuffer.o:ByteCircularBuffer.cpp
	${CC} ${CFLAGS} -c ByteCircularBuffer.cpp -o ByteCircularBuffer.o

TcpMsgVariableSizeDemarcar.o:TcpMsgVariableSizeDemarcar.cpp
	${CC} ${CFLAGS} -c TcpMsgVariableSizeDemarcar.cpp -o TcpMsgVariableSizeDemarcar.o

TcpServerCli.o:TcpServerCli.cpp
	${CC} ${CFLAGS} -c TcpServerCli.cpp -o TcpServerCli.o

libtimer/libtimer.a:
	(cd libtimer; make)

CommandParser/libcli.a:
	(cd CommandParser; make)
clean:
	rm -f *.o
	rm -f *exe
	(cd libtimer; make clean)
	(cd CommandParser; make clean)
