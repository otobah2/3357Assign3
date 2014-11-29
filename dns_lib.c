//
//  dns_lib.c
//
//  Computer Science 3357a
//  Assignment 3
//
//  Author: Antoine Alarie (aalarie2@uwo.ca)
//          Omar Tobah (otobah2@uwo.ca)
//
//  Library providing methods to set up DNS queries and parsing DNS responses
//

#include <string.h>
#include <unistd.h>
#include "dns_lib.h"
#include "udp_sockets.h"

//  Initializes a DNS query message with specified question
//
dns_message_t* create_dns_query(char* domain_name, char* qtype)
{
    dns_message_t* msg = (dns_message_t*) create_message();
    
    //Setup Header
    msg->ID = 0;
    msg->flags = 0;
    msg->qd_count = htons(1);
    msg->an_count = 0;
    msg->ns_count = 0;
    msg->ar_count = 0;
    
    //Get length of domain name
    unsigned long qname_length = strlen(domain_name);
    
    if (qname_length > 255)
    {
        printf("Error: Question name too long (max 255 characters)\n");
        exit(EXIT_FAILURE);
    }
    
    //Copy domain name in buffer
    msg->buffer[0] = qname_length;
    for (int i = 1; i <= qname_length + 1; ++i)
        msg->buffer[i] = domain_name[i-1];
    
    //Set qtype
    msg->buffer[qname_length+3] = qtype[0];
    msg->buffer[qname_length+4] = qtype[1];
    
    //Set qclass
    msg->buffer[qname_length+5] = 'I';
    msg->buffer[qname_length+6] = 'N';
    
    //Update length
    msg->length = 18 + (uint32_t)qname_length;
    
    return msg;
}

void print_dns_response(dns_message_t* response)
{
    uint8_t qr = response->flags >> 7;
    uint8_t rcode = response->flags & 00001111;
    uint16_t an_count = ntohs(response->an_count);
    
    //Check qr to ensure message is a response
    if (qr != 1)
    {
        printf("Error: Invalid Response\n");
        exit(EXIT_FAILURE);
    }
    
    //Check rcode for error
    switch (rcode)
    {
        //No error condition
        case 0:
            break;
        //Format error
        case 1:
            printf("Error: Format Error\n");
            exit(EXIT_FAILURE);
        //Server failure
        case 2:
            printf("Error: Server Failure.\n");
            exit(EXIT_FAILURE);
        //Name error
        case 3:
            printf("Error: Name Error\n");
            exit(EXIT_FAILURE);
        //Not implemented
        case 4:
            printf("Error: Not implemented\n");
            exit(EXIT_FAILURE);
        case 5:
            printf("Error: Refused\n");
            exit(EXIT_FAILURE);
        //Undefined rcode
        default:
            printf("Erro: Undefined Error\n");
            exit(EXIT_FAILURE);
    }
}


