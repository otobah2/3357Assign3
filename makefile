#makefile for assignment03
#
#cs3357a
#Assignment03
#
#

all: nsl

nsl: nsl.o udp_client.o udp_sockets.o dns_lib.o
	gcc -g -Wall $^ -o $@

nsl.o: nsl.c dns_lib.c udp_sockets.h udp_client.h
	gcc -c -g -Wall -o $@ $<

udp_client.o: udp_client.c udp_client.h udp_sockets.h
	gcc -c -g -Wall -o $@ $<

udp_sockets.o: udp_sockets.c udp_sockets.h
	gcc -c -g -Wall -o $@ $<

dns_lib.o: dns_lib.c dns_lib.h
	gcc -c -g -Wall -o $@ $<

clean:
	rm -rf *o nsl
