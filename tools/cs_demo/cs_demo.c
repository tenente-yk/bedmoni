#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
//#include "../../modules/csio_typedefs.h"
#include "csio_typedefs.h"
#include "udp.h"
#include "cs_demo.h"

#define DIO_FIFO_SIZE  10000

#define DIN_START '{'
#define DIN_END   '}'

//#define error printf

//#define DATA_FROM_FILE

volatile int exit_flag = 0;

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
    fprintf(stderr, "data start error 0x%02X\n", b);
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
    fprintf(stderr, "invalid length %d\n", len);
   // for (i=0; i<30; i++) printf("%02X (%c)", buf[i], buf[i]);
   // fflush(stdout);
    return 0;
  }

  b = buf[16+len+1];
  if (b != DIN_END)
  {
    fprintf(stderr, "data end error 0x%02X\n", b);
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
    fprintf(stderr, "crc error: %02X %02X\n", crc_rcvd, crc);
    fprintf(stderr, "length %d\n", len);
    return 0;
  }

 // printf("id = %d,\tlen = %d \tp[0]=%02X\n", id, len, buf[16]);

#if 0
  if (id == CSIO_ID_DEBUG)
  {
    csio_debug_info_t * p = (csio_debug_info_t *) &buf[16];
    fprintf(stdout, "%s", p->payload);
  }
#endif

#if 0
  if (id == CSIO_ID_RESP_C)
  {
    csio_respdata_c_t * p = (csio_respdata_c_t*) &buf[16];
//    printf("p[0]=%02X\n", buf[16]);
    static int sync_prev = 0;
    if (p->sync-sync_prev != 1 && (sync_prev-p->sync != 255))
    {
   // printf("%02X %02X\n", buf[16], sync_prev);
      fprintf(stderr, "RESP C SYNC ERROR\n");
    }
    sync_prev = p->sync;
    fprintf(stdout, "rpg\t%d\n", p->rpg);
  }
#endif

#if 1
  if (id == CSIO_ID_ECG_C)
  {
    csio_ecgdata_c_t * p = (csio_ecgdata_c_t*) &buf[16];
//    printf("p[0]=%02X\n", buf[16]);
    static int sync_prev = 0;
    if (p->sync-sync_prev != 1 && (sync_prev-p->sync != 255))
    {
   // printf("%02X %02X\n", buf[16], sync_prev);
      fprintf(stderr, "ECG C SYNC ERROR\n");
    }
    sync_prev = p->sync;
    short i = p->data[0], ii = p->data[1];
    fprintf(stdout, "ECG: \tI %d \tII %d \tIII %d \taVR %d \taVL %d \taVF %d \n", i, ii, ii-i, (i-ii)/2, (2*i-ii)/2, i/2);
  }
#endif

#if 0
  if (id == 3)
  {
    static unsigned long ts_prev = 0;
    int dt = ts - ts_prev;
    ts_prev = ts;
    printf("ts=%08X, dt=%d\n", ts, dt);
  }
#endif

 // dproc_process_packet(id, ts, &buf[16], len);

  return 1;
}

static void dio_process_data(unsigned char *pp, int len)
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

int main(int argc, char **argv)
{
#if defined (DATA_FROM_FILE)
  FILE * f;
#endif
  unsigned char buf[500];

  fprintf(stdout, "\r\ncs_demo\r\n");
  fprintf(stdout, "server started...\r\n");

#if defined (DATA_FROM_FILE)
  f = fopen("demodata55.bin", "rb");
#else
  udp_server_init();
#endif

  while (!exit_flag)
  {
    int nr;
#if defined (DATA_FROM_FILE)
    nr = fread(buf,1,sizeof(buf),f);
    if (feof(f)) break;
#else
    nr = udp_read(buf, sizeof(buf));
#endif

    if (nr > 0) dio_process_data(buf, nr);
//    printf("nr=%d\n", nr);
#ifdef UNIX
    usleep(10*1000);
#endif
#ifdef WIN32
    Sleep(10);
#endif
  }

#if defined (DATA_FROM_FILE)
  fclose(f);
#else
  udp_server_deinit();
#endif

  return 0;
}
