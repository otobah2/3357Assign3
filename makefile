#makefile for assignment03
#
#cs3357a
#Assignment03
#
#

all: nsl

nsl: client.o
	gcc -g -Wall $^ -o $@

client.o : client.c
	gcc -c -g -Wall -o $@ client.c

clean:
	rm -rf *o nsl
