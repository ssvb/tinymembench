.PHONY : all clean

NAME = tinymembench

ifdef WINDIR
	CC = gcc
endif

override CFLAGS += -O2

ASM = aarch64-asm arm-neon mips-32 x86-sse2
SRC = asm-opt util

ASM.h = $(addsuffix .h, ${ASM})
ASM.o = $(addsuffix .o, ${ASM})
SRC.h = $(addsuffix .h, ${SRC})
SRC.o = $(addsuffix .o, ${SRC})


all : ${NAME}

${NAME} : main.c version.h ${SRC.h} ${ASM.h} ${SRC.o} ${ASM.o}
	${CC} ${CFLAGS} $< ${SRC.o} ${ASM.o} -o $@ -lm

asm-opt.o : %.o : %.c %.h ${ASM.h}
	${CC} ${CFLAGS} -c $<

util.o : %.o : %.c %.h
	${CC} ${CFLAGS} -c $<

%.o : %.S
	${CC} ${CFLAGS} -c $<

clean:
	-rm -f ${NAME}
	-rm -f ${NAME}.exe
	-rm -f ${ASM.o} ${SRC.o}
