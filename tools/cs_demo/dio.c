#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "dproc.h"
#include "dio.h"

extern volatile int exit_flag;

static unsigned char rx_buf[DIO_FIFO_SIZE];
static int wptr = 0, rptr = 0;
static FILE * fcfg = NULL;

//static unsigned char tx_buf[DIO_TXFIFO_SIZE];
//static int owptr = 0, orptr = 0;

/*! \fn static int process_packet(unsigned char *buf)
 *  \brief Process incoming data packet, located in <i>buf</i>.
 *  \param buf Buffer, containing incoming data.
 *  \return 1 - if message is valid and successfully precessed, 0 - otherwise.
 */
static int process_packet(unsigned char *buf)
{
  int i;
  unsigned char b, crc, crc_rcvd;
  unsigned short id;
  short len;
  unsigned long  sno;
  unsigned long ts;

  b = buf[0];
  if (b != DIN_START)
  {
    error("data start error 0x%02X\n", b);
    return 0;
  }

  sno = (buf[5]<<24) | (buf[4]<<16) | (buf[3]<<8) | buf[2];
  b = buf[6]; // 0x77
  b = buf[7]; // 0x88
  ts = (buf[11]<<24) | (buf[10]<<16) | (buf[9]<<8) | buf[8];

  id = (buf[13]<<8) | buf[12];
  len = (buf[15]<<8) | buf[14];
  if (len > 240)
  {
    error("invalid length %d\n", len);
   // for (i=0; i<30; i++) printf("%02X (%c)", buf[i], buf[i]);
   // fflush(stdout);
    return 0;
  }

  b = buf[16+len+1];
  if (b != DIN_END)
  {
    error("data end error 0x%02X\n", b);
#if 1
    for (i=0; i<16+len+2; i++)
      printf("%02X * ", buf[i], buf[i]);
    printf("\n");
    fflush(stdout);
#endif
    return 0;
  }

  crc = 0;
  for (i=1; i<16+len; i++)
    crc += buf[i];
  crc_rcvd = buf[16+len];

  if (crc != crc_rcvd)
  {
    error("crc error: %02X %02X\n", crc_rcvd, crc);
    error("length %d\n", len);
    return 0;
  }

 // printf("id = %d,\tlen = %d \tp[0]=%02X\n", id, len, buf[16]);


  dproc_packet(id, ts, &buf[16], len);

  return 1;
}

void dio_process_data(unsigned char *pp, int len)
{
  unsigned char b;
  static unsigned char packet[300];
  static int packet_ptr = 0;
  static short packet_len = -1;

  if (len <= 0) return;

  if (wptr+len < DIO_FIFO_SIZE)
  {
    memcpy(&rx_buf[wptr], pp, len);
    wptr += len;
  }
  else
  {
    assert(wptr+len < 2*DIO_FIFO_SIZE);
    memcpy(&rx_buf[wptr], pp, DIO_FIFO_SIZE-wptr);
    memcpy(&rx_buf[0], pp+DIO_FIFO_SIZE-wptr, len-(DIO_FIFO_SIZE-wptr));
    wptr = len-(DIO_FIFO_SIZE-wptr);
  }

#if 0
  for (i=0; i<len; i++)
  {
    rx_buf[wptr++] = pp[i]; // TODO: replace with memcpy function call
    if (wptr >= DIO_FIFO_SIZE) wptr -= DIO_FIFO_SIZE;
  }
#endif

  while (rptr != wptr && !exit_flag)
  {
    while (rptr >= DIO_FIFO_SIZE) rptr -= DIO_FIFO_SIZE;
    if (rptr == wptr) break;

    b = rx_buf[rptr];

    if (packet_ptr == 0 && b == DIN_START)
    {
      packet[packet_ptr++] = b;
      rptr ++;
      continue;
    }
    if (packet_ptr > 0) packet[packet_ptr++] = b;
    if (packet_ptr == 16)
    {
      packet_len = *((unsigned short*)&packet[14]);
      if (packet_len > 240)
      {
        fprintf(stderr, "invalid length %d\n", packet_len);
        packet_ptr = 0;
        packet_len = -1;
        continue;
      }
      rptr ++;
      continue;
    }
    if (packet_len > 0 && packet_ptr == 16+packet_len+2)
    {
      process_packet(packet);
      packet_ptr = 0;
      packet_len = -1;
      rptr ++;
      continue;
    }
    if (packet_ptr > (300-1))
    {
      packet_ptr = 0;
      packet_len = -1;
      continue;
    }
    rptr ++;
  }
}
