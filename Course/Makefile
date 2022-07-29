CC=g++
CFLAGS=-g
TARGET:testapp.exe
LIBS=-lpthread -LCommandParser -lcli -lrt
OBJS=TcpClientDBManager.o 		 		\
			TcpClientServiceManager.o 		 \
			TcpNewConnectionAcceptor.o 	 \
			TcpServerController.o 				  \
			network_utils.o							   \
			TcpClient.o							 		 \
			ByteCircularBuffer.o				   \
			TcpMsgDemarcar.o					 \
			TcpMsgFixedSizeDemarcar.o	  \
			TcpMsgVariableSizeDemarcar.o \

testapp.exe:testapp.o ${OBJS} CommandParser/libcli.a
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

TcpClient.o:TcpClient.cpp
	${CC} ${CFLAGS} -c TcpClient.cpp -o TcpClient.o

ByteCircularBuffer.o:ByteCircularBuffer.cpp
	${CC} ${CFLAGS} -c ByteCircularBuffer.cpp -o ByteCircularBuffer.o

TcpMsgDemarcar.o:TcpMsgDemarcar.cpp
	${CC} ${CFLAGS} -c TcpMsgDemarcar.cpp -o TcpMsgDemarcar.o

TcpMsgFixedSizeDemarcar.o:TcpMsgFixedSizeDemarcar.cpp
	${CC} ${CFLAGS} -c TcpMsgFixedSizeDemarcar.cpp -o TcpMsgFixedSizeDemarcar.o
	
TcpMsgVariableSizeDemarcar.o:TcpMsgVariableSizeDemarcar.cpp
	${CC} ${CFLAGS} -c TcpMsgVariableSizeDemarcar.cpp -o TcpMsgVariableSizeDemarcar.o

CommandParser/libcli.a:
	(cd CommandParser; make)

clean:
	rm -f *.o
	rm -f *exe
	(cd CommandParser; make clean)
