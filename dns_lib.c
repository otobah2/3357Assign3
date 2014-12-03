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
            printf("\nError: Format Error\n\n");
            exit(EXIT_FAILURE);
        //Server failure
        case 2:
            printf("\nError: Server Failure.\n\n");
            exit(EXIT_FAILURE);
        //Name error
        case 3:
            printf("\nError: Name Error\n\n");
            exit(EXIT_FAILURE);
        //Not implemented
        case 4:
            printf("\nError: Not implemented\n\n");
            exit(EXIT_FAILURE);
        //Refused
        case 5:
            printf("\nError: Refused\n\n");
            exit(EXIT_FAILURE);
        //Undefined rcode
        default:
            printf("\nError: Undefined Error\n\n");
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
char* format_dns_name(uint8_t* dns_name, int* offset)
{
    char* formatted = malloc(256 * sizeof(char));
    int i, k = 0, length;
    
    //Read length octets and copy labels to formatted. Seperate labels by periods.
    length = dns_name[k];
    
    //If name is only a pointer, return offset from beginning of message
    if (length >> 6 == 3 && offset != NULL)
    {
        memcpy(offset, &dns_name[k], 2);
        *offset = ntohs(*offset);
        *offset = *offset & 0x3FFF;
        formatted[0] = '\0';
        return formatted;
    }
    
    //Parse domain name until a null character or pointer is encountered
    do
    {
        for (i = 0; i < length; ++i)
            formatted[i+k] = dns_name[i+k+1];
        
        k += length + 1;
        length = dns_name[k];
        formatted[k-1] = '.';
    }
    while (length != 0 && length >> 6 != 3);
    
    //If a pointer was encountered replace it by a period and return offset
    if (length >> 6 == 3 && offset != NULL)
    {
        memcpy(offset, &dns_name[k], 2);
        *offset = ntohs(*offset);
        *offset = *offset & 0x3FFF;
        formatted[k-1] = '.';
        formatted[k] = '\0';
        return formatted;
    }
    
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
    int i, pointer = 0;
    char* pointer_string;
    uint16_t flags = ntohs(response->flags);
    uint8_t qr = flags >> 15;
    uint8_t rcode = flags & 15;
    uint16_t an_count = ntohs(response->an_count);
    
    //Handle RCODE
    handle_rcode(rcode);
    
    //Read QNAME
    char* qname = format_dns_name(response->buffer, NULL);
    uint8_t name_length = strlen(qname);
    
    ////////Parse first RR//////////
    //Read NAME
    int offset_from_name;
    char* name = format_dns_name(&(response->buffer[name_length+6]), &pointer);
    //If a pointer was encountered in name, replace it with corresponding string
    if (pointer != 0)
    {
        pointer_string = format_dns_name(&(response->buffer[pointer-HEADER_OFFSET]), NULL);
        strcat(name, pointer_string);
        offset_from_name = name_length + 7;
    }
    //Save appropriate offset from name
    else
        offset_from_name = name_length + 6 + (int)strlen(name);
    
    //Read TYPE
    uint16_t type;
    memcpy(&type, &(response->buffer[offset_from_name + 1]), 2);
    type = ntohs(type);
    
    //Read TTL
    uint32_t ttl;
    memcpy(&ttl, &(response->buffer[offset_from_name + 5]), 4);
    ttl = ntohl(ttl);
    
    //Read RDLENGTH
    uint16_t rdlength;
    memcpy(&rdlength, &(response->buffer[offset_from_name + 9]), 2);
    rdlength = ntohs(rdlength);
    
    //Read RDATA
    uint8_t* rdata = malloc(rdlength * sizeof(char) + 1);
    for (i = 0; i < rdlength; ++i)
        rdata[i] = response->buffer[offset_from_name+11+i];
    rdata[i] = '\0';
    
    //Print server response
    printf("\nqr: %d\tancount: %d\ttype: %s \tttl: %d\n\n", qr, an_count, qtype_name(type), ttl);
    printf("Name: %s\n", name);
    print_rdata(rdata, type, response->buffer);
    printf("\n");
}

//  Print the provided rdata according to its type. A pointer to the buffer must be provided
//  to handle pointers in rdata names.
//
void print_rdata(uint8_t* rdata, uint8_t type, uint8_t* buffer)
{
    int pointer;
    uint16_t pref;
    char* processed_rdata = NULL;
    char* pointer_string;
    
    //If type is not A, must check for pointers in domain name
    if (type != A)
    {
        //MX is a special case preceded by a 2 byte integer
        if (type == MX)
            processed_rdata = format_dns_name(&rdata[2], &pointer);
        else
            processed_rdata = format_dns_name(rdata, &pointer);
        
        //If a pointer was present, replace it with corresponding string
        if (pointer != 0)
        {
            pointer_string = format_dns_name(&(buffer[pointer-HEADER_OFFSET]), NULL);
            strcat(processed_rdata, pointer_string);
        }
    }
    
    //Print processed rdata according to its type
    switch (type) {
        case A:
            printf("Address: %d.%d.%d.%d\n", rdata[0], rdata[1], rdata[2], rdata[3]);
            break;
        case NS:
            printf("Authoritative Host: %s\n", processed_rdata);
            break;
        case CNAME:
            printf("Canonical Name: %s\n", processed_rdata);
            break;
        case MX:
            //Obtain preference of server
            memcpy(&pref, &rdata[0], 2);
            pref = ntohs(pref);
            printf("Mail Server: %s\tpref: %d\n", processed_rdata, pref);
            break;
        case TXT:
            printf("Text: %s\n", processed_rdata);
            break;
        default:
            break;
    }
}

