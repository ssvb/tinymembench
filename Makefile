all: ssvb-membench

ifndef CC
	CC = gcc
endif

ssvb-membench: main.c util.o util.h asm-opt.o
	${CC} -O2 ${CFLAGS} -o ssvb-membench main.c util.o asm-opt.o

util.o: util.c util.h
	${CC} -O2 ${CFLAGS} -c util.c

asm-opt.o: asm-opt.c util.h
	${CC} -O2 ${CFLAGS} -c asm-opt.c

clean:
	-rm ssvb-membench
	-rm *.o
