/*! \file upd.c
    \brief Software update interface
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#ifdef UNIX
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#endif
#ifdef WIN32
#include <stdio.h>
#include <winsock2.h>

#pragma comment (lib, "ws2_32.lib")
#endif

#include "upd.h"

#define BEDMONI
#ifdef BMONI_SERVER
#undef BEDMONI
#endif

#ifdef BEDMONI
#include "bedmoni.h"
#endif

typedef struct
{
  unsigned char   magic;
  char            name[255];
  unsigned long   size;
} upd_file_info_t;

upd_file_info_t ufi; 

static FILE * fo = NULL;
static int upd_state = UPD_STATE_IDLE;

static unsigned long actually_sent = 0;
static unsigned long need_to_send = (unsigned long)(-1);

#if 0
#define upd_debug(X) printf(X)
#else
#define upd_debug(X)
#endif

static void sleep_ms(unsigned int ms)
{
#ifdef WIN32
  Sleep(ms);
#endif
#ifdef UNIX
  usleep(ms * 1000);
#endif
}

#if 0
// timeout задается в миллисекундах
static int connectex(int s, const struct sockaddr *name, int namelen, long timeout)
{
  // As connect() but with timeout setting.
  int            rc = 0;
#if defined(WIN32)
  unsigned long  ulB;
#endif
  struct timeval Time;
  fd_set         FdSet;

#if defined (WIN32)
  ulB = TRUE; // Set socket to non-blocking mode
  ioctlsocket(s, FIONBIO, &ulB);
#else
  fcntl( s, F_SETFL, O_NDELAY );
#endif

  if (connect(s, name, sizeof(struct sockaddr_in)) == -1)
  {
#if defined (WIN32)
    if (WSAGetLastError() == WSAEWOULDBLOCK)
    {
      // now wait for the specified time
      FD_ZERO(&FdSet);
      FD_SET(s, &FdSet);

      Time.tv_sec  = timeout / 1000L;
      Time.tv_usec = (timeout % 1000) * 1000;
      rc = select(0, NULL, &FdSet, NULL, &Time);
    }
#else
    // now wait for the specified time
    FD_ZERO(&FdSet);
    FD_SET(s, &FdSet);

    Time.tv_sec  = timeout / 1000L;
    Time.tv_usec = (timeout % 1000) * 1000;
    rc = select(0, NULL, &FdSet, NULL, &Time);
#endif
  }

#if defined (WIN32)
  ulB = FALSE; // Restore socket to blocking mode
  ioctlsocket(s, FIONBIO, &ulB);
#else
  /* Return to blocked mode: this is what we are need in */
  fcntl( s, F_SETFL, fcntl(s,F_GETFL) & (~O_NDELAY));
#endif

  return (rc > 0) ? 0 : -1;
}
#endif

int upd_client_open(char * ips, int portno)
{
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;

#ifdef WIN32
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
  wVersionRequested = MAKEWORD(2,2);
  err = WSAStartup(wVersionRequested, &wsaData);

  if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
  {
    WSACleanup( );
    return -1;
  }
}
#endif

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
#ifdef BEDMONI
    error("opening socket: %s\n", strerror(errno));
#else
    fprintf(stderr, "ERROR opening socket %s\n", strerror(errno));
#endif
    return 0;
  }

  server = gethostbyname(ips);

  if (server == NULL)
  {
    close(sockfd);
#ifdef BEDMONI
    error("invalid hostname\n");
#else
    fprintf(stderr,"ERROR, specify host\n");
#endif
    return 0;
  }
  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

/*
  if (0)
  {
    int ready;
    fd_set rfds;
   // fd_set wfds;
    fd_set efds;
    struct timeval tv;    // устанавливаем время ожидания в 1 сек.
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
   // FD_ZERO(&wfds);
   // FD_SET(sockfd, &wfds);
    FD_ZERO(&efds);
    FD_SET(sockfd, &efds);
    ready = select(sockfd+1, &rfds, NULL, &efds, &tv);
   // ready = select(sockfd+1, &rfds, &wfds, NULL, &tv);
    printf("ready = %d\n", ready);
    printf("isset = %d\n", FD_ISSET(sockfd, &rfds));
    printf("isset2 = %d\n", FD_ISSET(sockfd, &efds));
  }
*/

/*
#if defined(WIN32)
  u_long FAR cmd = 1;
  if( ioctlsocket( sockfd , FIONBIO, &cmd ) != 0 )
#else
  if( fcntl( sockfd , F_SETFL, O_NDELAY ) == -1 )
#endif
  {
#ifdef BEDMONI
    error("setting non-blocking mode!\n");
#else
    fprintf(stderr, "ERROR setting non-blocking mode!\n");
#endif
  }
*/

#if 1
  // this code checks is connection is established (in order to prevent hang in connect(..) call)
  FILE * pipe;
  char buf[BUFSIZ];
  char cmd[100];
  int server_is_running = 0;

  strcpy(cmd, "ifconfig");

  if ((pipe = popen(cmd, "r")) != NULL)
  {
    while (!feof(pipe))
    {
      if (fgets(buf, BUFSIZ, pipe) != NULL)
      {
        if (strstr(buf, "RUNNING")) server_is_running = 1;
      }
      else
      {
        break;
      }
    }
    pclose(pipe);
  }

  if (!server_is_running)
  {
    if (sockfd > 0)
      close(sockfd);

#ifdef BEDMONI
    error("server is not running\n");
#endif

    return 0;
  }
#endif

  if ( connect(sockfd, (struct sockaddr *) &serv_addr, (socklen_t)sizeof(serv_addr)) < 0)
 // if ( connectex(sockfd, (struct sockaddr *) &serv_addr, (socklen_t)sizeof(serv_addr), 1000) < 0)
  {
#ifdef BEDMONI
    error("connecting: %s\n", strerror(errno));
#else
    fprintf(stderr, "ERROR connecting %s\n", strerror(errno));
#endif

    if (sockfd > 0)
      close(sockfd);

    return 0;
  }

  upd_state = UPD_STATE_INPROGRESS;

  send(sockfd, UPD_MSG_START, sizeof(UPD_MSG_START), 0);

  memset(&ufi, 0, sizeof(ufi));

  return sockfd;
}

void upd_client_close(int sockfd)
{
#ifdef UNIX
  if (sockfd > 0)
    close(sockfd);
#endif

#ifdef WIN32
  if (sockfd > 0)
    closesocket(sockfd);
  WSACleanup( );
#endif

  upd_state = UPD_STATE_IDLE;
}

static unsigned char rxbuf[1200];
static int rxptr = 0;

static int upd_proc_recv_data(const void * data, int len, void * odata, int * nr)
{
  assert(len >= 0);

  memcpy(rxbuf+rxptr, data, len);
  rxptr += len;

  if (rxptr >= 4+1+1)
  {
    int pclen;
    int k;

    k = 0;
    while (rxbuf[k] != UPD_BYTE_START && k<rxptr) k++;
    if (rxptr - k >= 1+4+1)
    {
//printf("%02X %02X %02X %02X\n", rxbuf[k+4], rxbuf[k+3], rxbuf[k+2], rxbuf[k+1]);
      pclen = ( (rxbuf[k+4] << 24) | (rxbuf[k+3] << 16) | (rxbuf[k+2] << 8) | rxbuf[k+1] );
//printf("pclen = %X\n", pclen);
      upd_set_progress((pclen >> 24) % 101);
      pclen &= 0x00ffffff;
//printf("pclen = %d\n", pclen);
      if (rxptr - k >= pclen + 4 + 2)
      {
        // process message
       // printf("MSG!!! len %d, k %d\n", pclen, k);
       // fwrite(&rxbuf[k+1+4], 1, pclen, fo);
        memcpy(odata, &rxbuf[k+1+4], pclen);
        *nr = pclen;
        rxptr -= k+2+4+pclen;
        memmove(rxbuf, &rxbuf[k+2+4+pclen], rxptr);
        return (pclen) ? UPD_CODE_OK : UPD_CODE_END;
      }
    }
  }

  return UPD_CODE_BUSY;
}

void upd_stepit(int sockfd)
{
  int n, c, nr, nw;
  unsigned char buf[1040], obuf[1040];
  static int file_nr = 0;
  char s[255];

  do
  {
    if (upd_state == UPD_STATE_FINISHED) return;

    n = read(sockfd, buf, sizeof(buf));

    if (n <= 0) return;

    c = upd_proc_recv_data(buf, n, obuf, &nr);

   // printf("recv %d, c = %c\n", nr, c);

    if (c == UPD_CODE_OK)
    {
      if (ufi.magic != 0xAA)
      {
        assert(nr == sizeof(ufi));
        memcpy(&ufi, obuf, nr);
        strcpy(s, "");
#ifdef BEDMONI
        strcat(s, USBDISK_PATH);
        strcat(s, "/");
        debug("receiving file %s (%d bytes)\n", ufi.name, ufi.size);
#endif
        strcat(s, ufi.name);
        fo = fopen(s, "wb");
      }
      else
      {
        fwrite(obuf, 1, nr, fo);
        if (nr > 0) file_nr += nr;
        if (file_nr >= ufi.size)
        {
          file_nr = 0;
          memset(&ufi, 0, sizeof(ufi));
          fclose(fo);
          fo = NULL;
        }
      }
    }

    if (c == UPD_CODE_END)
    {
      upd_state = UPD_STATE_FINISHED;
    }

    nw = write(sockfd, &c, 4);
    if (nw != 4)
    {
      error("write error %s\n", __FUNCTION__);
    }
  } while (0);
}

static int upd_read_code(int sockfd)
{
  int nr, nr_total, c;
  nr_total = 0;
  do
  {
    nr = recv(sockfd, (unsigned char*)&c+nr_total, 4-nr_total, 0);
    if (nr > 0) nr_total += nr;
  } while (nr_total != 4);
 // printf("recv code 0x%X\n", c);
  return c;
}

static int upd_send_data(int sockfd, const void * data, int len, int pos)
{
  const int packet_size = 1024;
 // unsigned char b;
  int i, c, nw, nw_total, nout;
  unsigned char buf[packet_size+8]; // len could be <= 1024

 /* b = UPD_BYTE_START;
  send(sockfd, &b, 1, 0);
  len &= 0x00ffffff;
  len |= (pos % 101) << 24;
  send(sockfd, &len, 4, 0);
  if ((len & 0x00ffffff) && data)
    send(sockfd, data, len & 0x00ffffff, 0);
  b = UPD_BYTE_END;
  send(sockfd, &b, 1, 0);
*/
  nout = 0;
  buf[0] = UPD_BYTE_START;
  len &= 0x00ffffff;
  len |= (pos % 101) << 24;
  memcpy(&buf[1], &len, 4);
  if ((len & 0x00ffffff) && data)
  {
    memcpy(&buf[5], data, len & 0x00ffffff);
  }
  nout = 5 + (len & 0x00ffffff);
  buf[nout++] = UPD_BYTE_END;

  for (i=0,nw_total=0;i<5;i++)
  {
    nw = send(sockfd, buf+nw_total, nout-nw_total, 0);
    if (nw > 0) nw_total += nw;
    if (nw_total == nout) break;
  }
  assert(i<5);

  do
  {
    c = upd_read_code(sockfd);
  } while (c == UPD_CODE_BUSY);

  actually_sent += nout;

  return c;
}

static int upd_send_file_info(int sockfd, upd_file_info_t * pufi)
{
  const int packet_size = 1024;
  int rc = -1;
  int c;
  unsigned char buf[packet_size];

  assert(sizeof(upd_file_info_t) < packet_size);

  memcpy(buf, pufi, sizeof(upd_file_info_t));
  c = upd_send_data(sockfd, buf, sizeof(upd_file_info_t), 100*actually_sent / need_to_send);
  if (c == UPD_CODE_OK)
  {
    rc = 1;
  }
  upd_set_progress(100*actually_sent / need_to_send);

  return rc;
}

static int upd_send_file_packet(int sockfd, char * filename)
{
  const int packet_size = 1024;
  int rc = -1;
  static FILE * f = 0;
  static unsigned int pos = 0;
  int nr, c;
  unsigned char buf[packet_size];
  static int len = 0;

  if (!f)
  {
    f = fopen(filename, "rb");
    if (!f)
    {
#ifdef BEDMONI
      error("%s: unable to open %s\n", __FUNCTION__, filename);
#else
      fprintf(stderr, "Unable to open %s\n", filename);
#endif
      return 0;
    }
    fseek(f, 0, SEEK_SET);
    fseek(f, 0, SEEK_END);
    len = ftell(f);
   // upd_send_data(sockfd, &len, 4);
  }
  fseek(f, pos, SEEK_SET);
  nr = fread(buf, 1, packet_size, f);
  c = upd_send_data(sockfd, buf, nr, 100*pos / len);
  if (nr > 0 && c == UPD_CODE_OK)
  {
    pos += nr;
    rc = 1;
  }
  upd_set_progress(100*pos / len);
  if (feof(f))
  {
    c = upd_send_data(sockfd, NULL, 0, 100);
    if (c == UPD_CODE_END)
    {
      rc = 0;
      fclose(f);
      f = 0;
      pos = 0;
      len = 0;
     // fprintf(stdout, "| OK\n");
      return rc; // 0
    }
    else
      rc = -1;
  }
  return rc;
}

int  upd_server(int portno)
{
  int sockfd, newsockfd, clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n, rc;
  time_t tt;
  struct tm * ptm;

#ifdef WIN32
  if (1)
  {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2,2);
    err = WSAStartup(wVersionRequested, &wsaData);

    if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
    {
      WSACleanup( );
      return -1;
    }
  }
#endif

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
  {
//#ifdef BEDMONI
//    error("openning socket %s\n", strerror(errno));
//#else
    fprintf(stderr, "ERROR openning socket %s\n", strerror(errno));
//#endif
    return -1;
  }

  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  upd_state = UPD_STATE_WAITFORCONN;

  upd_debug("UPD_STATE_WAITFORCONN\n");

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
  {
//#ifdef BEDMONI
//    error("binding %s\n", strerror(errno));
//#else
    fprintf(stderr, "ERROR on binding: %s\n", strerror(errno));
//#endif
    return -1;
  }

  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
  if (newsockfd < 0)
  {
//#ifdef BEDMONI
//    error("accept %s\n", strerror(errno));
//#else
    fprintf(stderr, "ERROR on accept: %s\n", strerror(errno));
//#endif
    return -1;
  }

  time(&tt);
  ptm = localtime(&tt);
//#ifdef BEDMONI
//  info("update %s %02d:%02d %02d.%02d.%04d\n", inet_ntoa(cli_addr.sin_addr), ptm->tm_hour, ptm->tm_min, ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900);
//#else
  fprintf(stdout, "update %s %02d:%02d %02d.%02d.%04d\n", inet_ntoa(cli_addr.sin_addr), ptm->tm_hour, ptm->tm_min, ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900);
//#endif

  upd_debug("UPD_STATE_WAITFORSTART\n");

  memset(buffer, 0, sizeof(buffer));
 // n = read(newsockfd,buffer,255);

  do
  {
    n = recv(newsockfd, buffer, 255, 0);
    sleep_ms(100);
    if (n > 0)
    {
      if (strcmp(buffer, UPD_MSG_START) == 0)
      {
        upd_state = UPD_STATE_INPROGRESS;
        break;
      }
    }
  }
  while (n == 0);

  upd_debug("UPD_STATE_INPROGRESS\n");

  do
  {
    char fname[255];
    FILE *f;

    strcpy(fname, "1.tar.gz");

    // TODO: splitpath ufi.name, extract from fname
    strcpy(ufi.name, fname);

    f = fopen(fname, "rb");
    if(f)
    {
      fseek(f, 0, SEEK_END);

      ufi.size = ftell(f);
      fclose(f);
      ufi.magic = 0xAA;

      rc = upd_send_file_info(newsockfd, &ufi);

      do
      {
       // fprintf(stdout, ">");
       // fflush(stdout);
       // upd_debug(">\n");
        fprintf(stdout, "\r%d%%", upd_get_progress());
        fflush(stdout);

        rc = upd_send_file_packet(newsockfd, ufi.name);
        sleep_ms(100);
      }
      while (rc != 0);

      fprintf(stdout, "\r%d%%", upd_get_progress());
    } // if (f)
  }
  while (0); // repeat for all files

  upd_set_progress(100);
  fprintf(stdout, "\r%d%%", upd_get_progress());
  fprintf(stdout, "\n");
  upd_state = UPD_STATE_FINISHED;

#ifdef UNIX
  if (sockfd > 0)
    close(sockfd);
  if (newsockfd > 0)
    close(newsockfd);
#endif
#ifdef WIN32
  if (sockfd > 0)
    closesocket(sockfd);
  if (newsockfd > 0)
    closesocket(newsockfd);
#endif

#ifdef WIN32
  WSACleanup();
#endif

  upd_state = UPD_STATE_IDLE;

  return 0;
}

int  upd_get_state(void)
{
  return upd_state;
}

static int progress;

void upd_set_progress(int pos)
{
  progress = pos;
}

int  upd_get_progress(void)
{
  return progress;
}
