//
//  nsl.c
//
//  Computer Science 3357a
//  Assignment 3
//
//  Author: Antoine Alarie (aalarie2@uwo.ca)
//          Omar Tobah (otobah2@uwo.ca)
//
//  DNS client that handles A, NS, CNAME, MX, & TXT queries from the command line
//

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include "udp_client.h"
#include "udp_sockets.h"
#include "dns_lib.h"

int main(int argc, char** argv)
{
  //create our input variables
  char* type = "A";
  char* dnsserver;
  char* query;
  host_t server_address;
  dns_message_t* query_msg;
  dns_message_t* response_msg;
  static struct option long_options[]=
  {
    {"type", required_argument, 0, 't'},
    {0, 0, 0, 0}
  };

  //handle inputs
  int option_index = 0, c;
  while ((c = getopt_long(argc, argv, "t:", long_options, &option_index)) != -1)
  {
    //switch cases to handle options
    switch(c)
    {
      case 't':
        type = optarg;
        break;

      case '?':
        //Error message handled, exit
        exit(EXIT_FAILURE);
        break;
    }
  }

  //handle non-option arguments
  /*if there are no arguments left, neither server nor query were given*/
  if(optind == argc || optind == argc-1)
  {
    printf("Error: server or query not specified.\n");
    printf("Expected Syntax: nsl [TYPE] DNSSERVER QUERY\n");
    exit(EXIT_FAILURE);
  }
  dnsserver = argv[optind];
  query = argv[optind+1];
  
  //Check if requested type is supported, if not print error
  int type_code = qtype_value(type);
  if (type_code != 1 && type_code != 2 && type_code != 5 && type_code != 15 && type_code != 16)
  {
    printf("Error: Unsupported record type\n");
    exit(EXIT_FAILURE);
  }
  
  //Create UDP client socket
  int sockfd = create_client_socket(dnsserver, "53", &server_address);
  
  //Send DNS query through socket
  query_msg = create_dns_query(query, type);
  int ret = send_message(sockfd, (message_t*)query_msg, &server_address);
  
  if (ret == -1)
  {
    printf("Error: Failed to send through socket\n");
    exit(EXIT_FAILURE);
  }
  
  //Receive DNS response
  response_msg = (dns_message_t*) receive_message(sockfd, &server_address);
  print_dns_response(response_msg);
  
  //Close socket
  close(sockfd);

  exit(EXIT_SUCCESS);
}

