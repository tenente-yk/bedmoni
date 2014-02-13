#ifdef UNIX
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#define winsize winsize1
#define termio termio1
#include <termios.h>
#include <errno.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#endif

#ifdef WIN32
#pragma warning (disable: 4311 4312)
#endif

#include "port.h"

char *cports[RATES] =
{
  "300",
  "600",
  "1200",
  "2400",
  "4800",
  "9600",
  "19200",
  "38400",
  "57600",
  "115200",
  "230400",
  "460800",
  "921600",
};

int br_rates[RATES] = 
{
  300,
  600,
  1200,
  2400,
  4800,
  9600,
  19200,
  38400,
  57600,
  115200,
  230400,
  460800,
  921600,
};

#ifdef DEB
#include "debug.h"
extern int  ffd;
extern FILE *ff;
#endif

#ifdef UNIX
static void Sleep(unsigned long dwMilliseconds)
{
  usleep(1000*dwMilliseconds);
}

int Ioctl(int rate, int fd, char *settings)
{
  int i;
  struct termios tio;
  static int rates[RATES] = { B300, B600, B1200, B2400, B4800,
                            B9600, B19200, B38400, B57600, B115200, B230400, B460800, B921600 };
  if(rate<0 || rate>=RATES) { return -1; }

  i = ioctl(fd, TCGETS, &tio);
  if(i<0) { close(fd); return -1; }
  tio.c_oflag = 0;
  tio.c_iflag = IGNBRK | IGNPAR;
  tio.c_cflag = rates[rate] | CREAD | CLOCAL | HUPCL;
  if ( settings && strlen(settings)>2)
  {
    switch( settings[0] )
    {
      case '6':
        tio.c_cflag |= CS6;
        break;
      case '7':
        tio.c_cflag |= CS7;
        break;
      default:
        tio.c_cflag |= CS8;
        break;
    }
    switch( settings[1] | 0x20 )
    {
      case 'o':
        tio.c_cflag |= PARENB | PARODD;
        break;
      case 'e':
        tio.c_cflag |= PARENB;
        break;
    }
    if ( settings[2] == '2' )
    {
      tio.c_cflag |= CSTOPB;
    }
    // CTS RTS
    if ( settings[4] == '1' )
    {
      tio.c_cflag |= CRTSCTS;
    }
    // Xon/Xoff
    if ( settings[5] == '1' )
    {
      tio.c_iflag |= IXON;
      tio.c_iflag |= IXOFF;
    }
  }
  else
  {
    tio.c_cflag |= CS8;
  }
  tio.c_lflag = 0;
  memset(tio.c_cc, 0, NCCS);
  tio.c_cc[VMIN]=1; // 250 original
  tio.c_cc[VTIME]=1;

  i = ioctl(fd, TCSETSW, &tio);

  if(i<0) { close(fd); return -1; }
  //RateSet = rate;
  return fd;
}

int Ioctl2(unsigned long oct_rate, int fd, char *settings)
{
  int i;
  struct termios tio;

  i = ioctl(fd, TCGETS, &tio);
  if(i<0) { close(fd); return -1; }
  tio.c_oflag = 0;
  tio.c_iflag = IGNBRK | IGNPAR;
  tio.c_cflag = oct_rate | CREAD | CLOCAL | HUPCL;
  if ( settings && strlen(settings)>2)
  {
    switch( settings[0] )
    {
      case '6':
        tio.c_cflag |= CS6;
        break;
      case '7':
        tio.c_cflag |= CS7;
        break;
      default:
        tio.c_cflag |= CS8;
        break;
    }
    switch( settings[1] | 0x20 )
    {
      case 'o':
        tio.c_cflag |= PARENB | PARODD;
        break;
      case 'e':
        tio.c_cflag |= PARENB;
        break;
    }
    if ( settings[2] == '2' )
    {
      tio.c_cflag |= CSTOPB;
    }
    // CTS RTS
    if ( settings[4] == '1' )
    {
      tio.c_cflag |= CRTSCTS;
    }
    // Xon/Xoff
    if ( settings[5] == '1' )
    {
      tio.c_iflag |= IXON;
      tio.c_iflag |= IXOFF;
    }
  }
  else
  {
    tio.c_cflag |= CS8;
  }
  tio.c_lflag = 0;
  memset(tio.c_cc, 0, NCCS);
  tio.c_cc[VMIN]=1; // 250 original
  tio.c_cc[VTIME]=1;

  i = ioctl(fd, TCSETSW, &tio);

  if(i<0) { close(fd); return -1; }
  //RateSet = rate;
  return fd;
}
#endif

int com_write(int fd, const char *arr, int size)
{
#ifdef WIN32
  unsigned long dwWritten = 0;
  if ( !WriteFile((void*)fd, arr, size, &dwWritten, NULL) ) return -1;
  return dwWritten;
#endif
#ifdef UNIX
  return write(fd, arr, size);
#endif
}

int com_read(int fd, char *arr, int size)
{
#ifdef WIN32
  unsigned long dwRead = 0;
  if ( !ReadFile((void*)fd, arr, size, &dwRead, NULL) ) return -1;
  return dwRead;
#endif
#ifdef UNIX
  return read(fd, arr, size);
#endif
}

int com_open( char *port_name, br_t port_rate, char *settings)
{
#ifdef UNIX
  int fd = 0; int i;
  int mode = O_NOCTTY | O_RDWR;
#endif
#ifdef WIN32
  HANDLE hPort;
  DCB dcbState;
  hPort = CreateFile ( port_name
                      , GENERIC_READ | GENERIC_WRITE
                      , 0
                      , NULL
                      , OPEN_EXISTING
                      , 0
                      , NULL
                      );

#ifdef DEB
  myprintf(ffd, 0, ff, "hPort = 0x%X, br_rates[port_rate] = %d\r\n", hPort, br_rates[port_rate]);
#endif

  if (hPort == INVALID_HANDLE_VALUE) 
  {
//    fprintf(stderr, "Open failed, ERRNO=%d\r\n", GetLastError());
#ifdef DEB
    myprintf(ffd, 0, ff, "Open failed, ERRNO=%d\r\n", GetLastError());
#endif
    return 0;
  }

  memset(&dcbState, 0, sizeof(DCB));
  GetCommState(hPort, &dcbState);

  dcbState.BaudRate = br_rates[port_rate];
  dcbState.Parity   = NOPARITY;
  dcbState.ByteSize = 8;
  dcbState.StopBits = ONESTOPBIT;

  if (SetCommState(hPort, &dcbState))
  {
   // SetupComm(m_hCOM, 1024, 1024);
    COMMTIMEOUTS CommTimeouts;
    CommTimeouts.ReadIntervalTimeout = MAXDWORD;
    CommTimeouts.ReadTotalTimeoutConstant = 0;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant = 10;
    CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
    if (!SetCommTimeouts(hPort, &CommTimeouts))
      return FALSE;

    PurgeComm(hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
  }
  else
  {
//    fprintf(stderr, "SetCommState failed, ERRNO=%d\r\n", GetLastError());
#ifdef DEB
    myprintf(ffd, 0, ff, "SetCommState failed, ERRNO=%d\r\n", GetLastError());
#endif
    return 0;
  }

  return (int)hPort;
#endif
#ifdef UNIX
  fd = open(port_name, mode | O_NONBLOCK); /* To open port in any case */
 // fd = open(port_name, mode | O_NDELAY); /* To open port in any case */

  if (fd < 0) 
  {
//    fprintf(stderr, "Open failed, ERRNO=%d: ", errno);
//    fprintf(stderr, "%s\r\n", strerror(errno));
#ifdef DEB
    myprintf(ffd, 0, ff, "Open failed for %s, ERRNO=%d: ", port_name, errno);
    myprintf(ffd, 0, ff, "%s\r\n", strerror(errno));
#endif
    return fd;
  }

  /* Return to blocked mode: this is what we are need in */
 // fcntl(fd,F_SETFL,fcntl(fd,F_GETFL) & (~O_NONBLOCK));

  if(fd <= 0) 
  {
    return fd; 
  }
  i = Ioctl (port_rate, fd, settings);
  if (i < 0) close (fd);
  return i;
#endif
}

#ifdef UNIX
int com_open_2(char *port_name, unsigned long tio_rate)
{
  int fd = 0; int i;
  int mode = O_NOCTTY | O_RDWR;
  fd = open(port_name, mode | O_NONBLOCK); /* To open port in any case */
 // fd = open(port_name, mode | O_NDELAY); /* To open port in any case */

  if (fd < 0)
  {
//    fprintf(stderr, "Open failed, ERRNO=%d: ", errno);
//    fprintf(stderr, "%s\r\n", strerror(errno));
#ifdef DEB
    myprintf(ffd, 0, ff, "Open failed for %s, ERRNO=%d: ", port_name, errno);
    myprintf(ffd, 0, ff, "%s\r\n", strerror(errno));
#endif
    return fd;
  }

  /* Return to blocked mode: this is what we are need in */
 // fcntl(fd,F_SETFL,fcntl(fd,F_GETFL) & (~O_NONBLOCK));

  if(fd <= 0)
  {
    return fd;
  }
  i = Ioctl2 (tio_rate, fd, 0);
  if (i < 0) close (fd);
  return i;
}
#endif

int com_close(int fd)
{
#ifdef WIN32
  return CloseHandle((void*)fd);
#endif
#ifdef UNIX
  return close(fd);
#endif
}

int com_setbaud(int fd, br_t port_rate)
{
#ifdef WIN32
  DCB m_dcb;
  DWORD old_br;
  BOOL fSuccess = FALSE;
  DWORD  Errors;
  COMSTAT  Stat;

  FlushFileBuffers((void*)fd);
  Sleep(10);
  PurgeComm( (void*)fd, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT );

  GetCommState((void*)fd,&m_dcb);
  old_br = m_dcb.BaudRate;
  //if(br == old)return true;
  m_dcb.BaudRate = br_rates[port_rate];
  fSuccess = SetCommState( (void*)fd, &m_dcb);
  if (!fSuccess )
  {
    m_dcb.BaudRate = old_br;
  }
  ClearCommError((void*)fd,&Errors,&Stat);
  PurgeComm((void*)fd,PURGE_RXCLEAR | PURGE_TXCLEAR);
  PurgeComm((void*)fd,PURGE_TXABORT | PURGE_TXABORT);

  return fSuccess;
#endif
#ifdef UNIX
  return (Ioctl(port_rate, fd, 0) > 0);
#endif
}

int com_write_ch(int fd, char ch)
{
  char arr[1];
  arr[0] = ch;
  return com_write(fd, arr, 1);
}

int com_waitfor_ch(int fd, char *s, char inp_c)
{
  int i;
  long bytes;
  for(i=0; i<10; i++) // 1 second
  {
    bytes = com_read(fd, s, 1);
    if(bytes)
    {
      if(bytes > 0)
      {
        return 1;
      }
      else
      {
        if( inp_c )
        {
          com_write_ch(fd, inp_c);
        }
      }
    }
    Sleep(100);
  }
  return 0;
}

int com_recv(int fd, char *buf, int len)
{
  unsigned long numBytes;
  numBytes = com_read(fd, buf, len);
  if(numBytes)
  {
    buf[numBytes] = 0;
  }
  return numBytes;
}

int com_purge(int fd)
{
#ifdef UNIX
//  fprintf(stderr, "PurgeComm not supported\r\n");
  return 0;
#endif
#ifdef WIN32
  return PurgeComm((void*)fd, PURGE_TXCLEAR | PURGE_RXCLEAR);
#endif
}

int com_flush(int fd)
{
#ifdef UNIX
  int ret;
  ret = ioctl(fd, TCFLSH, TCIOFLUSH);
  ret |= tcdrain(fd);
  return ret;
#endif
#ifdef WIN32
  return FlushFileBuffers((void*)fd);
#endif
}

int com_setdataline(int fd, dl_t dl)
{
#ifdef WIN32
  if (dl.clrbreak) EscapeCommFunction((void*)fd, CLRBREAK);
  if (dl.clrdtr)   EscapeCommFunction((void*)fd, CLRDTR);
  if (dl.clrrts)   EscapeCommFunction((void*)fd, CLRRTS);
  if (dl.setbreak) EscapeCommFunction((void*)fd, SETBREAK);
  if (dl.setdtr)   EscapeCommFunction((void*)fd, SETDTR);
  if (dl.setrts)   EscapeCommFunction((void*)fd, SETRTS);
  if (dl.setxoff)  EscapeCommFunction((void*)fd, SETXOFF);
  if (dl.setxon)   EscapeCommFunction((void*)fd, SETXON);
  return 0;
#endif
#ifdef UNIX
  int status;
  if (ioctl(fd, TIOCMGET, &status) != 0)
  {
//    fprintf(stderr, "Fail in %s: %d\n", __FILE__, __LINE__);
    return -1;
  }
  if (dl.setdtr) status |=  TIOCM_DTR;
  if (dl.clrdtr) status &= ~TIOCM_DTR;
  if (dl.setrts) status |=  TIOCM_RTS;
  if (dl.clrrts) status &= ~TIOCM_RTS;

  if (ioctl(fd, TIOCMSET, &status) != 0)
  {
//    fprintf(stderr, "Fail in %s: %d\n", __FILE__, __LINE__);
    return -1;
  }
  return 0;
#endif
}


#ifdef WIN32
int com_enum(int *pnums, int size)
{
  int i, nports=0;
  char str[20];
  COMMCONFIG cc;
  unsigned int sz=sizeof(COMMCONFIG);
  if (!pnums) return -1;
  for(i=1;i<100;i++)
  {
    sprintf(str,"COM%i",i);
    if (GetDefaultCommConfig(str,&cc,&sz))
    {
      *pnums++ = i;
      size--;
      nports++;
      if (size == 0) break;
    }
  }
  return nports;
}
#endif

#ifdef WIN32
#pragma warning (default: 4311 4312)
#endif
