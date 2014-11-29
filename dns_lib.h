//
//  dns_lib.h
//
//  Computer Science 3357a
//  Assignment 3
//
//  Author: Antoine Alarie (aalarie2@uwo.ca)
//          Omar Tobah (otobah2@uwo.ca)
//
//  Library providing methods to set up DNS queries and parsing DNS responses
//

#ifndef _dns_lib_h_
#define _dns_lib_h_

#include <stdlib.h>

//This message can be cast to the more general message_t* format defined in udp_sockets library
//DNS message format
typedef struct
{
    uint32_t length;
    uint16_t ID;
    uint16_t flags;
    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;
    uint8_t buffer[65439];
} dns_message_t;

//Functions
dns_message_t* create_dns_query(char* domain_name, char* qtype);
void handle_rcode(uint8_t rcode);
void print_dns_response(dns_message_t* response);

#endif

