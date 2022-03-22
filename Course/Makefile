CC=g++
CFLAGS=-g
TARGET:testapp.exe
LIBS=-lpthread
OBJS=TcpClientDBManager.o 		 		\
			TcpClientServiceManager.o 		 \
			TcpNewConnectionAcceptor.o 	 \
			TcpServerController.o 				  \
			network_utils.o					

testapp.exe:testapp.o ${OBJS}
	${CC} ${CFLAGS} ${OBJS} testapp.o -o testapp.exe ${LIBS}

testapp.o:testapp.cpp
	${CC} ${CFLAGS} -c testapp.cpp -o testapp.o

TcpClientDBManager.o:TcpClientDBManager.cpp
	${CC} ${CFLAGS} -c TcpClientDBManager.cpp -o TcpClientDBManager.o

TcpClientServiceManager.o:TcpClientServiceManager.cpp
	${CC} ${CFLAGS} -c TcpClientServiceManager.cpp -o TcpClientServiceManager.o

TcpNewConnectionAcceptor.o:TcpNewConnectionAcceptor.cpp
	${CC} ${CFLAGS} -c TcpNewConnectionAcceptor.cpp -o TcpNewConnectionAcceptor.o

TcpServerController.o:TcpServerController.cpp
	${CC} ${CFLAGS} -c TcpServerController.cpp -o TcpServerController.o

network_utils.o:network_utils.cpp
	${CC} ${CFLAGS} -c network_utils.cpp -o network_utils.o

clean:
	rm -f *.o
	rm -f *exe
