/*client.c
 *
 * CS3357a
 * Assignment03
 *
 * Authors: Omar Tobah and Antoine Alarie
 * otobah2@uwo.ca
 * aalarie2@uwo.ca
 *
 * our DNS client for Assignment 03
 */

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
  
  /*//print options
  printf("Type       : %s\n", type);
  printf("DNS Server : %s\n", dnsserver);
  printf("Query      : %s\n", query);*/
  
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

