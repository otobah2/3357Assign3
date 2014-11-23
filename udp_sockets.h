/*header file for udp_sockets.c
 * contains all necessary prototypes and includes
 */

#ifndef SOCKETS_H
#define SOCKETS_H

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

//Maximum amount of data per UDP datagram
#define UDP_MSS 65535

typedef struct
{
  int length;
  uint8_t buffer[UDP_MSS];
}message_t;

typedef struct
{
  struct sockaddr_in addr;
  socklen_t addr_len;
  char friendly_ip[INET_ADDRSTRLEN];
}host_t;

struct addrinfo* get_udp_sockaddr(const char* node, const char* port, int flags);

message_t* create_message();

message_t* receive_message(int sockfd, host_t* source);

int send_message(int sockfd, message_t* msg, host_t* dest);

#endif
