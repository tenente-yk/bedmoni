#ifdef UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#ifdef WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include <string.h>
//#include "networkset.h"
//#include "bedmoni.h"
#include "udp.h"

#ifdef WIN32
#pragma comment (lib, "ws2_32.lib")
#endif

static int sockfd = 0;
static struct sockaddr_in * si_other_ptr = NULL;

#if defined (UDP_BEDMONI_SERVER)
static struct sockaddr_in si_client;
int udp_server_init(void)
{
  struct sockaddr_in serv_addr;
//  networkset_t nws;
  int portno;

#ifdef WIN32
  int err;
  WORD wVersionRequested;
  WSADATA wsaData;
  wVersionRequested = MAKEWORD(2,2);
  err = WSAStartup(wVersionRequested, &wsaData);
  assert(err == 0);
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
  {
    WSACleanup();
    assert(0);
  }
#endif

  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sockfd <= 0)
  {
    error("%s: socket error\n", __FUNCTION__);
    return -1;
  }

 // memset(&nws, 0, sizeof(networkset_t));
 // network_get(&nws);

 // portno = UDP_WORKSTATION_COMM_PORT;
  portno = 9999;//nws.port;
  si_other_ptr = NULL;

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  serv_addr.sin_port=htons(portno);
  if ( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0 )
  {
    error("%s: bind error\n", __FUNCTION__);
    close(sockfd);
    sockfd = 0;
    return -1;
  }

  if (sockfd > 0)
    si_other_ptr = &si_client;

  return 0;
}

void udp_server_deinit(void)
{
#ifdef UNIX
  if (sockfd > 0) close(sockfd);
#endif
#ifdef WIN32
  if (sockfd > 0) closesocket(sockfd);
  WSACleanup();
#endif
}
#endif // if defined (UDP_BEDMONI_SERVER)

#if defined (UDP_BEDMONI_CLIENT)
static struct sockaddr_in serv_addr;
int udp_client_init(void)
{
  char s[200], buf[100];
  networkset_t nws;
  int portno;

  si_other_ptr = NULL;

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);

  snprintf(s, sizeof(s), "%d.%d.%d.%d", (nws.serverip & 0xff000000) >> 24, (nws.serverip & 0x00ff0000) >> 16, (nws.serverip & 0x0000ff00) >> 8, (nws.serverip & 0x000000ff) >> 0);
 // portno = UDP_WORKSTATION_COMM_PORT;
  portno = nws.portno;
#ifndef ARM
 // strcpy(s, "10.1.2.16");
#endif

  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sockfd <= 0)
  {
    error("%s: socket error\n", __FUNCTION__);
    return -1;
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);
  serv_addr.sin_addr.s_addr = inet_addr(s);
  if (serv_addr.sin_addr.s_addr == INADDR_NONE)
  {
    struct hostent * host = NULL;
    host = gethostbyname(s);
    if (host == NULL)
    {
      error("%s: no such host\n", __FUNCTION__);
      sockfd = 0;
      return -1;
    }
    memcpy(&serv_addr.sin_addr, host->h_addr_list[0], host->h_length);
  }

  debug("udp: %s:%d, state: %s\n", s, portno, sockfd>0?"ok":"failed");

  strcpy(buf, "hello from bedmoni");
  if (sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
  {
    error("sendto in %s\n", __FUNCTION__);
    return -1;
  }

  if (sockfd > 0)
    si_other_ptr = &serv_addr;

  return 0;
}

void udp_client_deinit(void)
{
  if (sockfd > 0) close(sockfd);
  si_other_ptr = NULL;
}
#endif // if defined (UDP_BEDMONI_CLIENT)

int udp_write(const void * pbuf, int len)
{
  if (!si_other_ptr) return -1;
  return (sendto(sockfd, pbuf, len, 0, (const struct sockaddr *)si_other_ptr, sizeof(struct sockaddr)) );
}

int udp_read(void * pbuf, int len)
{
 // int si_len = sizeof(struct sockaddr);
 // if (!si_other_ptr) return -1;
 // return ( recvfrom(sockfd, pbuf, len, 0, (struct sockaddr *)si_other_ptr, (socklen_t *)&si_len) );
  struct sockaddr sock_addr;
  int si_len = sizeof(struct sockaddr);
  return ( recvfrom(sockfd, pbuf, len, 0, (struct sockaddr *)&sock_addr, (socklen_t *)&si_len) );
  // parse sock_addr structure
}

