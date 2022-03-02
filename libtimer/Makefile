CC=g++
CFLAGS=-g
LIBNAME=libtimer.a
TARGET:${LIBNAME}
OBJS=gluethread/glthread.o timerlib.o WheelTimer.o

gluethread/glthread.o:gluethread/glthread.cpp
	${CC} ${CFLAGS} -c gluethread/glthread.cpp -o gluethread/glthread.o

timerlib.o:timerlib.cpp
	${CC} ${CFLAGS} -c timerlib.cpp -o timerlib.o

WheelTimer.o:WheelTimer.cpp
	${CC} ${CFLAGS} -c WheelTimer.cpp -o WheelTimer.o

${LIBNAME}:${OBJS}
	ar rs ${LIBNAME} ${OBJS}

clean:
	rm -f *.o
	rm -f *.a
	rm -f gluethread/*.o
