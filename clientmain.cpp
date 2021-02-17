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
  calcMessage *message = new calcMessage{};
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
  msg.type = htons(22);
  msg.message = htonl(0);
  msg.protocol = htons(17);
  msg.major_version = htons(1);
  msg.minor_version = htons(0);
  if((sentbytes = sendto(sockfd, &msg, sizeof(msg),0,p->ai_addr, p->ai_addrlen)) == -1)
  {
    printf("Error: Couldnt send to the server.");
    close(sockfd);
    exit(0);
  }

  printf("Sent message to server.\n");
  freeaddrinfo(si);

  socklen_t addr_len = sizeof(servaddr);
  int bytes = recvfrom(sockfd, &protMsg, sizeof(protMsg),0, (struct sockaddr *)&servaddr, &addr_len);
  if(bytes == -1)
  {
    printf("Error:Couldnt recieve from server.\n");
    close(sockfd);
    exit(0);
  }

  if(sizeof(calcMessage) == bytes)
  {
    delete message;
    message = (calcMessage*)&protMsg;
    message->message = ntohl(message->message);
    message->type = ntohs(message->type);
    message->major_version = ntohs(message->major_version);
    message->minor_version = ntohs(message->minor_version);

    if(message->type == 2 && message->message == 2 && message->major_version == 1 && 
    message->minor_version == 0)
    {
      printf("Wrong type recieved. Expected a calcProtocol!\n");
      close(sockfd);
      exit(0);
    }
    printf("Error: \n");


  }
  else
  {
    protMsg.arith = ntohl(protMsg.arith);
    protMsg.inValue1 = ntohl(protMsg.inValue1);
    protMsg.inValue2 = ntohl(protMsg.inValue2);
    protMsg.inResult = ntohl(protMsg.inResult);
    protMsg.id = ntohl(protMsg.id);
    protMsg.major_version = ntohs(protMsg.major_version);
    protMsg.minor_version = ntohs(protMsg.minor_version);
    protMsg.type = ntohs(protMsg.type);

  if(protMsg.arith == 1)
  {
    protMsg.inResult = protMsg.inValue1 + protMsg.inValue2;
    printf("Add %d %d, result: %d\n", protMsg.inValue1, protMsg.inValue2, protMsg.inResult);
  }
  else if(protMsg.arith == 2)
  {
    protMsg.inResult = protMsg.inValue1 - protMsg.inValue2;
    printf("Sub %d %d, result: %d\n", protMsg.inValue1, protMsg.inValue2, protMsg.inResult);
  }
  else if(protMsg.arith == 3)
  {
    protMsg.inResult = protMsg.inValue1 * protMsg.inValue2;
    printf("Mul %d %d, result: %d\n", protMsg.inValue1, protMsg.inValue2, protMsg.inResult);
  }
  else if(protMsg.arith == 4)
  {
    protMsg.inResult = protMsg.inValue1 / protMsg.inValue2;
    printf("Div %d %d, result: %d\n", protMsg.inValue1, protMsg.inValue2, protMsg.inResult);
  }
  else if(protMsg.arith == 5)
  {
    protMsg.flResult = protMsg.flValue1 + protMsg.flValue2;
    printf("fAdd %lf %lf, result: %lf\n", protMsg.flValue1, protMsg.flValue2, protMsg.flResult);
  }
  else if(protMsg.arith == 6)
  {
    protMsg.flResult = protMsg.flValue1 - protMsg.flValue2;
    printf("fSub %lf %lf, result: %lf\n", protMsg.flValue1, protMsg.flValue2, protMsg.flResult);
  }
  else if(protMsg.arith == 7)
  {
    protMsg.flResult = protMsg.flValue1 * protMsg.flValue2;
    printf("fMul %lf %lf, result: %lf\n", protMsg.flValue1, protMsg.flValue2, protMsg.flResult);
  }
  else if(protMsg.arith == 8)
  {
    protMsg.flResult = protMsg.flValue1 / protMsg.flValue2;
    printf("fDiv %lf %lf, result: %lf\n", protMsg.flValue1, protMsg.flValue2, protMsg.flResult);
  }

    protMsg.arith = htonl(protMsg.arith);
    protMsg.inValue1 = htonl(protMsg.inValue1);
    protMsg.inValue2 = htonl(protMsg.inValue2);
    protMsg.inResult = htonl(protMsg.inResult);
    protMsg.id = htonl(protMsg.id);
    protMsg.major_version = htons(protMsg.major_version);
    protMsg.minor_version = htons(protMsg.minor_version);
    protMsg.type = htons(protMsg.type);

    if((sentbytes = sendto(sockfd, &protMsg, sizeof(protMsg),0,p->ai_addr, p->ai_addrlen)) == -1)
  {
    printf("Error: Couldnt send to the server.");
    close(sockfd);
    exit(0);
  }

  bytes = recvfrom(sockfd, message, sizeof(*message),0, (struct sockaddr *)&servaddr, &addr_len);
  message->message = ntohl(message->message);

  if(message->message == 1)
  {
    printf("OK!\n");
  }
  else
  {
    printf("NOT OK!\n");
  }

  printf("%d\n", message->message);

}

  close(sockfd);
  delete message;
  return 0;
}
