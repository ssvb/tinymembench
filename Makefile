all: ssvb-membench

ifndef CC
	CC = gcc
endif

ssvb-membench: main.c util.o util.h
	${CC} -O2 ${CFLAGS} -o ssvb-membench main.c util.o

util.o: util.c util.h
	${CC} -O2 ${CFLAGS} -c util.c

clean:
	-rm ssvb-membench
	-rm *.o
