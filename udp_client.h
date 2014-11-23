/*header file for our udp_client.c file
 * includes all necessary prototypes
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_client_socket(char* hostname, char* port, host_t* server); 

#endif
