#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "5689"  // the port users will be connecting to
#define BACKLOG 2   // how many pending connections queue will hold



int main()
{
  //Configure the network interface on the server

  //int sockfd;
  struct addrinfo hints, *serverInfo;
  int err;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP
  if((err=getaddrinfo(NULL, PORT, &hints, &serverInfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    exit(-1);
  }


  return 0;
}
