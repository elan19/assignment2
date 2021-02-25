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
#include <time.h>
#include <signal.h>
/* You will to add includes here */

// Included to get the support library
#define DEBUG
#include "calcLib.h"

#include "protocol.h"


void INThandler(int sig)
{
  exit(0);
}

int main(int argc, char *argv[])
{

  if (argc != 2)
  {
    printf("Wrong format IP:PORT\n");
    exit(0);
  }

  /* Do magic */

  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);

    if (Desthost == NULL || Destport == NULL)
  {
    printf("Wrong format.\n");
    exit(0);
  }

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
    freeaddrinfo(si);
    close(sockfd);
    exit(0);
  }

  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  ssize_t sentbytes;
  msg.type = htons(22);
  msg.message = htonl(0);
  msg.protocol = htons(17);
  msg.major_version = htons(1);
  msg.minor_version = htons(0);
  int tries = 0;
  int bytes = 0;
  socklen_t addr_len = sizeof(servaddr);

  printf("Sent message to server.\n");

  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
  signal(SIGINT, INThandler);

  while (bytes <= 0 && tries < 3)
  {

    if ((sentbytes = sendto(sockfd, &msg, sizeof(msg), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
      printf("Error: Couldnt send to the server.");
      exit(0);
    }

    bytes = recvfrom(sockfd, &protMsg, sizeof(protMsg), 0, (struct sockaddr *)&servaddr, &addr_len);

    if (bytes == -1 && tries < 2)
    {
      printf("Server did not respond, trying again.\n");
    }
    tries++;
  }
  if (bytes == -1)
  {
    printf("Error:Couldnt recieve from server.\n");
    exit(0);
  }
  if (sizeof(calcMessage) == bytes)
  {
    delete message;
    message = (calcMessage *)&protMsg;
    message->message = ntohl(message->message);
    message->type = ntohs(message->type);
    message->major_version = ntohs(message->major_version);
    message->minor_version = ntohs(message->minor_version);

    if (message->type == 2 && message->message == 2 && message->major_version == 1 &&
        message->minor_version == 0)
    {
      printf("Wrong type recieved. Expected a calcProtocol!\n");
      exit(0);
    }
    printf("Error: \n");
  }
  else if(bytes == sizeof(calcProtocol))
  {
    protMsg.arith = ntohl(protMsg.arith);
    protMsg.inValue1 = ntohl(protMsg.inValue1);
    protMsg.inValue2 = ntohl(protMsg.inValue2);
    protMsg.inResult = ntohl(protMsg.inResult);
    protMsg.id = ntohl(protMsg.id);
    protMsg.major_version = ntohs(protMsg.major_version);
    protMsg.minor_version = ntohs(protMsg.minor_version);
    protMsg.type = ntohs(protMsg.type);

    if (protMsg.arith == 1)
    {
      protMsg.inResult = protMsg.inValue1 + protMsg.inValue2;
      printf("Add %d %d, result: %d\n", protMsg.inValue1, protMsg.inValue2, protMsg.inResult);
    }
    else if (protMsg.arith == 2)
    {
      protMsg.inResult = protMsg.inValue1 - protMsg.inValue2;
      printf("Sub %d %d, result: %d\n", protMsg.inValue1, protMsg.inValue2, protMsg.inResult);
    }
    else if (protMsg.arith == 3)
    {
      protMsg.inResult = protMsg.inValue1 * protMsg.inValue2;
      printf("Mul %d %d, result: %d\n", protMsg.inValue1, protMsg.inValue2, protMsg.inResult);
    }
    else if (protMsg.arith == 4)
    {
      protMsg.inResult = protMsg.inValue1 / protMsg.inValue2;
      printf("Div %d %d, result: %d\n", protMsg.inValue1, protMsg.inValue2, protMsg.inResult);
    }
    else if (protMsg.arith == 5)
    {
      protMsg.flResult = protMsg.flValue1 + protMsg.flValue2;
      printf("fAdd %lf %lf, result: %lf\n", protMsg.flValue1, protMsg.flValue2, protMsg.flResult);
    }
    else if (protMsg.arith == 6)
    {
      protMsg.flResult = protMsg.flValue1 - protMsg.flValue2;
      printf("fSub %lf %lf, result: %lf\n", protMsg.flValue1, protMsg.flValue2, protMsg.flResult);
    }
    else if (protMsg.arith == 7)
    {
      protMsg.flResult = protMsg.flValue1 * protMsg.flValue2;
      printf("fMul %lf %lf, result: %lf\n", protMsg.flValue1, protMsg.flValue2, protMsg.flResult);
    }
    else if (protMsg.arith == 8)
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

    tries = 0;
    bytes = -1;
    while (bytes <= 0 && tries < 3)
    {

      if ((sentbytes = sendto(sockfd, &protMsg, sizeof(protMsg), 0, p->ai_addr, p->ai_addrlen)) == -1)
      {
        printf("Error: Couldnt send to the server.");
        exit(0);
      }

      bytes = recvfrom(sockfd, message, sizeof(*message), 0, (struct sockaddr *)&servaddr, &addr_len);
      if (bytes == -1 && tries < 2)
      {
        printf("Server did not respond, trying again.\n");
      }
      tries++;
    }

    if (bytes == -1)
    {
      printf("Error:Couldnt recieve from server.\n");
      exit(0);
    }
    message->message = ntohl(message->message);

    if (message->message == 1)
    {
      printf("OK!\n");
    }
    else if(message->message == 2)
    {
      printf("NOT OK!\n");
    }
    else{
      printf("N/A\n");
    }
  }
  struct sockaddr_in local;
  socklen_t addrlength = sizeof(addrlength);
  getsockname(sockfd, (struct sockaddr *)&local, &addrlength);
#ifdef DEBUG
  printf("Host %s, and port %d. Local %s, %d.\n", Desthost, port, inet_ntoa(local.sin_addr),
         (int)ntohs(local.sin_port));
#endif

  close(sockfd);
  delete message;
  return 0;
}
