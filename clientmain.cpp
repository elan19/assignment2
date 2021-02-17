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

  calcMessage msg;
  calcProtocol protMsg;

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
    close(sockfd);
    exit(0);
  }

  ssize_t sentbytes;
  msg.type = 22;
  msg.message = 0;
  msg.protocol = 17;
  msg.major_version = 1;
  msg.minor_version = 0;
  if((sentbytes = sendto(sockfd, &msg, sizeof(msg),0,p->ai_addr, p->ai_addrlen)) == -1)
  {
    printf("Error: Couldnt send to the server.");
    close(sockfd);
    exit(0);
  }

  freeaddrinfo(si);

  socklen_t addr_len = sizeof(servaddr);
  if(recvfrom(sockfd, &protMsg, sizeof(protMsg),0, (struct sockaddr *)&servaddr, &addr_len) == -1)
  {
    printf("Error:Couldnt recieve from server.\n");
    close(sockfd);
    exit(0);
  }

  if(protMsg.arith == 1)
  {
    protMsg.inResult = protMsg.inValue1 + protMsg.inValue2;
    printf("Add %d %d\n", protMsg.inValue1, protMsg.inValue2);
  }
  else if(protMsg.arith == 2)
  {
    protMsg.inResult = protMsg.inValue1 - protMsg.inValue2;
    printf("Sub %d %d\n", protMsg.inValue1, protMsg.inValue2);
  }
  else if(protMsg.arith == 3)
  {
    protMsg.inResult = protMsg.inValue1 * protMsg.inValue2;
    printf("Mul %d %d\n", protMsg.inValue1, protMsg.inValue2);
  }
  else if(protMsg.arith == 4)
  {
    protMsg.inResult = protMsg.inValue1 / protMsg.inValue2;
    printf("Div %d %d\n", protMsg.inValue1, protMsg.inValue2);
  }
  else if(protMsg.arith == 5)
  {
    protMsg.flResult = protMsg.flValue1 + protMsg.flValue2;
    printf("fAdd %lf %lf\n", protMsg.flValue1, protMsg.flValue2);
  }
  else if(protMsg.arith == 6)
  {
    protMsg.flResult = protMsg.flValue1 - protMsg.flValue2;
    printf("fSub %lf %lf\n", protMsg.flValue1, protMsg.flValue2);
  }
  else if(protMsg.arith == 7)
  {
    protMsg.flResult = protMsg.flValue1 * protMsg.flValue2;
    printf("fMul %lf %lf\n", protMsg.flValue1, protMsg.flValue2);
  }
  else if(protMsg.arith == 8)
  {
    protMsg.flResult = protMsg.flValue1 / protMsg.flValue2;
    printf("fDiv %lf %lf\n", protMsg.flValue1, protMsg.flValue2);
  }


  printf("Test\n");

  close(sockfd);
  return 0;
}
