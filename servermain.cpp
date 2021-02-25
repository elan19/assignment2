#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <vector>
#include <signal.h>

/* You will to add includes here */

// Included to get the support library
#include "calcLib.h"

#include "protocol.h"

/* Needs to be global, to be rechable by callback and main */
int loopCount = 0;
int terminate = 0;
struct cli
{
  struct sockaddr_in addr;
  calcProtocol work;
  struct timeval tid;
};
std::vector<cli> clients;

/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum)
{
  // As anybody can call the handler, its good coding to check the signal number that called it.
  struct timeval t;
  gettimeofday(&t, NULL);
  for (size_t i = 0; i < clients.size(); i++)
  {
    if (t.tv_sec - clients.at(i).tid.tv_sec > 10)
    {
      clients.erase(clients.begin() + i);
    }
  }
  printf("Let me be, I want to sleep.\n");

  if (loopCount > 20)
  {
    printf("I had enough.\n");
    terminate = 1;
  }

  return;
}

int main(int argc, char *argv[])
{

  /* Do more magic */

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

  int sockfd;
  addrinfo sa, *si, *p;
  sa.ai_family = AF_INET;
  sa.ai_socktype = SOCK_DGRAM;
  sa.ai_protocol = 17;
  initCalcLib();

  calcMessage *message = new calcMessage{};
  calcProtocol protMsg;

  if (int rv = getaddrinfo(Desthost, Destport, &sa, &si) != 0)
  {
    fprintf(stderr, "%s\n", gai_strerror(rv));
    exit(0);
  }

  struct sockaddr_in clientaddr;

  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      printf("Error: Failed to create socket.\n");
      continue;
    }
    memset(&clientaddr, 0, sizeof(clientaddr));
    if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) != 0)
    {
      printf("Error: Couldnt bind!\n");
      close(sockfd);
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
  int bytes = 0;
  ssize_t sentbytes;
  socklen_t client_len = sizeof(clientaddr);

  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

  /* 
     Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  */
  struct itimerval alarmTime;
  alarmTime.it_interval.tv_sec = 2;
  alarmTime.it_interval.tv_usec = 2;
  alarmTime.it_value.tv_sec = 2;
  alarmTime.it_value.tv_usec = 2;

  /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL, &alarmTime, NULL); // Start/register the alarm.

  bool messageChange = false;
  bool clientFound = false;
  int clientNr = 0;
  int clientInUse = -1;
  calcProtocol protSend;
  while (terminate == 0)
  {
    clientFound = false;
    printf("This is the main loop, %d time.\n", loopCount);
    bytes = recvfrom(sockfd, &protMsg, sizeof(protMsg), 0, (struct sockaddr *)&clientaddr, &client_len);
    if ((bytes) == -1)
    {
      printf("Error: Recieve timeout, sending error to client!\n");
    }

    if (sizeof(calcMessage) == bytes)
    {
      if (!messageChange)
      {
        delete message;
        messageChange = true;
      }
      message = (calcMessage *)&protMsg;
      for (size_t i = 0; i < clients.size() && clientFound != true; i++)
      {
        if (clients.at(i).addr.sin_addr.s_addr == clientaddr.sin_addr.s_addr && clients.at(i).addr.sin_port == clientaddr.sin_port)
        {
          clientFound = true;
        }
      }
      if (clientFound == false)
      {
        message->message = ntohl(message->message);
        message->type = ntohs(message->type);
        message->major_version = ntohs(message->major_version);
        message->minor_version = ntohs(message->minor_version);
        message->protocol = ntohs(message->protocol);
        if (message->type == 22 && message->message == 0 && message->protocol == 17 &&
            message->major_version == 1 && message->minor_version == 0)
        {
          protSend.arith = randomInt() % 8 + 1;
          if (protSend.arith > 4)
          {
            protSend.flValue1 = randomFloat();
            protSend.flValue2 = randomFloat();
            if (protSend.arith == 5)
            {
              protSend.flResult = protSend.flValue1 + protSend.flValue2;
            }
            else if (protSend.arith == 6)
            {
              protSend.flResult = protSend.flValue1 - protSend.flValue2;
            }
            else if (protSend.arith == 7)
            {
              protSend.flResult = protSend.flValue1 * protSend.flValue2;
            }
            else if (protSend.arith == 8)
            {
              protSend.flResult = protSend.flValue1 / protSend.flValue2;
            }
          }
          else
          {
            protSend.inValue1 = randomInt();
            protSend.inValue2 = randomInt();
            if (protSend.arith == 1)
            {
              protSend.inResult = protSend.inValue1 + protSend.inValue2;
            }
            else if (protSend.arith == 2)
            {
              protSend.inResult = protSend.inValue1 - protSend.inValue2;
            }
            else if (protSend.arith == 3)
            {
              protSend.inResult = protSend.inValue1 * protSend.inValue2;
            }
            else if (protSend.arith == 4)
            {
              protSend.inResult = protSend.inValue1 / protSend.inValue2;
            }
          }

          protSend.id = clientNr;
          clientNr++;
          protSend.type = 1;
          protSend.major_version = 1;
          protSend.minor_version = 0;
          protSend.arith = htonl(protSend.arith);
          protSend.id = htonl(protSend.id);
          protSend.type = htons(protSend.type);
          protSend.major_version = htons(protSend.major_version);
          protSend.minor_version = htons(protSend.minor_version);
          protSend.inValue1 = htonl(protSend.inValue1);
          protSend.inValue2 = htonl(protSend.inValue2);

          struct cli clientName;
          clientName.addr = clientaddr;
          gettimeofday(&clientName.tid, NULL);
          clientName.work = protSend;
          clients.push_back(clientName);
        }
      }
      if ((sentbytes = sendto(sockfd, &protSend, sizeof(protSend), 0, (struct sockaddr *)&clientaddr,
                              client_len)) == -1)
      {
        printf("Error: Couldnt send to the client.\n");
      }
    }
    else if (bytes == sizeof(calcProtocol))
    {
      for (size_t i = 0; i < clients.size() && clientFound == false; i++)
      {
        if (clients.at(i).addr.sin_addr.s_addr == clientaddr.sin_addr.s_addr && clients.at(i).addr.sin_port == clientaddr.sin_port && ntohl(clients.at(i).work.id) == ntohl(protMsg.id))
        {
          clientFound = true;
          clientInUse = i;
        }
      }
      if (clientFound == true)
      {
        clients.at(clientInUse).work.arith = ntohl(clients.at(clientInUse).work.arith);
        message->type = 2;
        message->major_version = 1;
        message->minor_version = 0;
        message->protocol = 17;
        protMsg.inResult = ntohl(protMsg.inResult);
        message->major_version = htons(message->major_version);
        message->minor_version = htons(message->minor_version);
        message->type = htons(message->type);
        message->protocol = htons(message->protocol);

        if (clients.at(clientInUse).work.arith > 4)
        {
          if (clients.at(clientInUse).work.flResult == protMsg.flResult)
          {
            message->message = 1;
          }
          else
          {
            message->message = 2;
          }
        }
        else
        {
          if (clients.at(clientInUse).work.inResult == protMsg.inResult)
          {
            message->message = 1;
          }
          else
          {
            message->message = 2;
          }
        }
      }
      else if (clientFound == false)
      {
        message->message = 0;
      }
      message->message = htonl(message->message);
      if ((sentbytes = sendto(sockfd, message, sizeof(*message), 0, (struct sockaddr *)&clientaddr,
                              client_len)) == -1)
      {
        printf("Error, couldnt send to client!\n");
      }
      else if(clientFound == true)
      {
        clients.erase(clients.begin() + clientInUse);
        clientInUse = -1;
      }
    }

    loopCount++;
  }
  return (0);
}
