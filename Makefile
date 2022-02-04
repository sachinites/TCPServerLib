CC=g++
CFLAGS=-g
TARGET:testapp.exe tcp_client.exe
LIBS=-lpthread -lrt
OBJS=TcpClient.o \
			testapp.o \
			TcpClientDbManager.o  \
			TcpClientServiceManager.o \
			TcpNewConnectionAcceptor.o \
			TcpServer.o \
			network_utils.o

testapp.exe:${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o testapp.exe ${LIBS}

tcp_client.exe:tcp_client.o
	${CC} ${CFLAGS} tcp_client.o -o tcp_client.exe
	
TcpClient.o:TcpClient.cpp
	${CC} ${CFLAGS} -c TcpClient.cpp -o TcpClient.o

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

TcpServer.o:TcpServer.cpp
	${CC} ${CFLAGS} -c TcpServer.cpp -o TcpServer.o

network_utils.o:network_utils.cpp
	${CC} ${CFLAGS} -c network_utils.cpp -o network_utils.o

clean:
	rm -f *.o
	rm -f *exe
