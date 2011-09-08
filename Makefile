all: ssvb-membench

ssvb-membench: main.c util.o util.h
	gcc -static -o ssvb-membench -O2 main.c util.o

util.o: util.c util.h
	gcc -O2 -c util.c
