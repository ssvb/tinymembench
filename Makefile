all: ssvb-membench

ifndef CC
	CC = gcc
endif

ssvb-membench: main.c util.o util.h asm-opt.h asm-opt.o x86-sse2.o arm-neon.o mips-32.o
	${CC} -O2 ${CFLAGS} -o ssvb-membench main.c util.o asm-opt.o x86-sse2.o arm-neon.o mips-32.o

util.o: util.c util.h
	${CC} -O2 ${CFLAGS} -c util.c

asm-opt.o: asm-opt.c asm-opt.h x86-sse2.h arm-neon.h mips-32.h
	${CC} -O2 ${CFLAGS} -c asm-opt.c

x86-sse2.o: x86-sse2.S
	${CC} -O2 ${CFLAGS} -c x86-sse2.S

arm-neon.o: arm-neon.S
	${CC} -O2 ${CFLAGS} -c arm-neon.S

mips-32.o: mips-32.S
	${CC} -O2 ${CFLAGS} -c mips-32.S

clean:
	-rm ssvb-membench
	-rm *.o
