/*header file for udp_server.c
 * contains all necessary function prototypes
 */
#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "udp_sockets.h"

int bind_socket(struct addrinfo* addr_list);

int create_server_socket(char* port);

#endif
