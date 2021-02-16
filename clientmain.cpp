#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
/* You will to add includes here */


// Included to get the support library
#include "calcLib.h"


#include "protocol.h"

int main(int argc, char *argv[]){
  
  /* Do magic */

  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);

  int port = atoi(Destport);
  addrinfo sa, *si, *p;
  sa.ai_family = AF_INET;
  sa.ai_socktype = SOCK_DGRAM;
  sa.ai_protocol = 17;

  if (int rv = getaddrinfo(Desthost, Destport, &sa, &si) != 0)
  {
    fprintf(stderr, "%s\n", gai_strerror(rv));
    exit(0);
  }

  struct sockaddr_in servaddr;

  int sockfd;

  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      printf("Error: Failed to create socket.\n");
      continue;
    }
    break;
  }

  if (p == NULL)
  {
    printf("NULL\n");
    exit(0);
  }

  printf("hejsan\n");

  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  int n, len;
  char buffer[256];

  if(sendto(sockfd, 0, 0, NULL, p->ai_addr, p->ai_addrlen) == -1)
  {
    printf("%s\n", strerror(errno));
    close(sockfd);
    exit(0);
  }

  freeaddrinfo(si);

  socklen_t addr_len = sizeof(servaddr);
  if(n = recvfrom(sockfd, buffer, 256,0, (struct sockaddr *)&servaddr, &addr_len) == -1)
  {
    printf("Error:Couldnt recieve from server.\n");
    close(sockfd);
    exit(0);
  }


  printf("%s\n", buffer);

  printf("Test\n");

  close(sockfd);
  return 0;
}
