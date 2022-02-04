CC=g++
CFLAGS=-g
TARGET:testapp.exe
LIBS=-lpthread -lrt

testapp.exe:TcpClient.o testapp.o TcpClientDbManager.o TcpClientServiceManager.o TcpNewConnectionAcceptor.o TcpServer.o
	${CC} ${CFLAGS} TcpClient.o testapp.o TcpClientDbManager.o TcpClientServiceManager.o TcpNewConnectionAcceptor.o TcpServer.o -o testapp.exe ${LIBS}

TcpClient.o:TcpClient.cpp
	${CC} ${CFLAGS} -c TcpClient.cpp -o TcpClient.o

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

clean:
	rm -f *.o
	rm -f *exe
