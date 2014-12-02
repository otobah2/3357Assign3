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

// Returns the value of the specified qtype name
//
uint16_t qtype_value(char* qtype)
{
    if (strcmp(qtype, "A") == 0)
        return 1;
    if (strcmp(qtype, "NS") == 0)
        return 2;
    if (strcmp(qtype, "CNAME") == 0)
        return 5;
    if (strcmp(qtype, "MX") == 0)
        return 15;
    if (strcmp(qtype, "TXT") == 0)
        return 16;
    return 0;
}

// Returns the name of the specified qtype value
//
char* qtype_name(int qtype)
{
    if (qtype == 1)
        return "A";
    if (qtype == 2)
        return "NS";
    if (qtype == 5)
        return "CNAME";
    if (qtype == 15)
        return "MX";
    if (qtype == 16)
        return "TXT";
    return NULL;
}

//  Handles the error for the passed DNS RCODE value. If no error, the function returns
//
void handle_rcode(uint8_t rcode)
{
    //Check rcode and print appropriate error message
    switch (rcode)
    {
            //No error
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
            //Refused
        case 5:
            printf("Error: Refused\n");
            exit(EXIT_FAILURE);
            //Undefined rcode
        default:
            printf("Error: Undefined Error\n");
            exit(EXIT_FAILURE);
    }
}

//  Formats the specified domain name into the correct label format for DNS messages
//
uint8_t* format_domain_name(char* domain_name)
{
    char* label;
    uint8_t* formatted = malloc(256 * sizeof(char));
    int i, k = 0, length;
    
    //Read labels (delimitted by .) and concatenate them to formatted preceded by their length octet
    label = strtok(domain_name, ".");
    do
    {
        length = (int) strlen(label);
        formatted[k] = length;
        
        for (i = 0; i < formatted[k]; ++i)
            formatted[k+i+1] = label[i];
        
        k += length + 1;
        label = strtok(NULL, ".");
    }
    while (label != NULL);
    
    //Append null character to end of formatted string
    formatted[k] = '\0';
    
    return formatted;
}

//  Formats the specified DNS name into a readable domain name
//
char* format_dns_name(uint8_t* dns_name)
{
    char* formatted = malloc(256 * sizeof(char));
    int i, k = 0, length;
    
    //Read length octets and copy labels to formatted. Seperate labels by periods.
    length = dns_name[k];
    do
    {
        for (i = 0; i < length; ++i)
            formatted[i+k] = dns_name[i+k+1];
        
        k += length + 1;
        length = dns_name[k];
        formatted[k-1] = '.';
    }
    while (length != 0);
    
    //Append null character to end of formatted string
    formatted[k-1] = '\0';
    
    return formatted;
}

//  Initializes a DNS query message with specified question
//
dns_message_t* create_dns_query(char* domain_name, char* qtype_name)
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
    uint8_t* qname = format_domain_name(domain_name);
    memcpy(&(msg->buffer[0]), qname, qname_length+2);
    
    //Set qtype
    uint16_t qtype = qtype_value(qtype_name);
    memcpy(&(msg->buffer[qname_length+3]), &qtype, 2);
    printf("qtype-test[0]: %d\n", msg->buffer[qname_length+3]);
    printf("qtype-test[1]: %d\n\n", msg->buffer[qname_length+4]);
    
    //Set qclass
    uint16_t qclass = 1;
    memcpy(&(msg->buffer[qname_length+5]), &qclass, 2);
    
    //Update length
    msg->length = 18 + (uint32_t)qname_length;
    return msg;
}

//  Parses a DNS response message and prints it
//
void print_dns_response(dns_message_t* response)
{
    int i;
    uint8_t qr = response->flags >> 15;
    uint8_t rcode = response->flags & 15;
    uint16_t an_count = ntohs(response->an_count);
    uint16_t flags = response->flags;
    
    printf("ancount: %d\n", an_count);
    printf("rcode: %d\n", rcode);
    printf("flags: %d\n", flags);
    
    //Handle RCODE
    handle_rcode(rcode);
    
    //Read NAME of record
    char* name = format_dns_name(response->buffer);
    uint8_t name_length = strlen(name);
    
    //Read TYPE of record
    uint16_t type;
    memcpy(&type, &(response->buffer[name_length+3]), 2);
    
    //Read RDLENGTH
    uint32_t ttl;
    memcpy(&ttl, &(response->buffer[name_length+7]), 4);
    
    //Read RDLENGTH
    uint16_t rdlength;
    memcpy(&rdlength, &(response->buffer[name_length+11]), 2);
    
    //Read RDATA
    char* rdata = malloc(rdlength * sizeof(char) + 1);
    for (i = 0; i < rdlength; ++i)
        rdata[i] = response->buffer[name_length+13+i];
    rdata[rdlength] = '\0';
    
    //Print response (will be interpreted by a function taking TYPE, which will determine RDATA)
    for (i = 0; i < 50; ++i)
        printf("buffer[%d]: %d\n",i, response->buffer[i]);
    printf("Name: %s\n", name);
    printf("ttl: %d\n", ttl);
    printf("Type: %s\n", qtype_name(type));
    printf("rdlength: %d\n", rdlength);
    printf("RDATA: %s\n", rdata);
}


