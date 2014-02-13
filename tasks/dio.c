/*! \file dio.c
    \brief SBDP board and other fast periphery i/o functions.
*/

#ifdef UNIX
#include <unistd.h>
#include <termios.h>
#endif
#ifdef WIN32
//#include <windows.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <memory.h>
#include <assert.h>
#include "dio.h"
#include "dproc.h"
#include "port.h"
#include "cframe.h"
#include "tprn.h"
#include "demo.h"
#include "ecgm.h"
#include "ecgcalc.h"
#include "co2.h"
#include "bedmoni.h"

#include "errno.h"

/*! \var sbdp_version
 *  \brief SBDP board version string.
 */
char sbdp_version[10];

static unsigned char rx_buf[DIO_FIFO_SIZE];
static int wptr = 0, rptr = 0;
static unsigned char tx_buf[DIO_TXFIFO_SIZE];
static int owptr = 0, orptr = 0;

static int fd;
static FILE * fdemo = NULL;

static volatile int dio_update_suspended = 0;

/*! \fn int dio_init(void)
 *  \brief SBDP i/o interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
int dio_init(void)
{
  demo_cfg_t dcfg;

  demo_cfg_get(&dcfg);
  demo_mode = dcfg.demo;

  if (demo_mode == 0)
  {
    fd = com_open(DEV, BR115200, 0);
    if (fd <= 0)
    {
      error("unable to open sbdp in %s\n", DEV);
      info("run in demo mode\n");
      demo_mode = 1;
    }
  }

  if (demo_mode)
  {
    char s[255];

    sprintf(s, "%s/%s/demodata%d.bin", USBDISK_PATH, DEMOFILES_DIR, dcfg.fno);
    fdemo = fopen(s, "rb");
    if (!fdemo)
    {
      error("Unable to open demo file %s\n", s);
    }
  }
  else // if (!demo_mode)
  {
#ifdef UNIX
    tcflush(fd, TCIOFLUSH);
    tcdrain(fd);
#endif
#ifdef WIN32
    com_purge(fd);
#endif
  }

  dcfg.demo = demo_mode;
  demo_cfg_set(&dcfg);

  ecgm_command_queue_init();
  co2_command_queue_init();

  return 0;
}

/*! \fn void dio_deinit(void)
 *  \brief SBDP i/o interface deinit function.
 */
void dio_deinit(void)
{
  if (fdemo)
  {
    fclose(fdemo);
    fdemo = NULL;
  }
  if (fd > 0) com_close(fd);
}

/*! \fn void dio_module_cmd(unsigned short id, unsigned char cmd, ...)
 *  \brief Send message to SBDP board.
 *  \param id Module identificator.
 *  \param id Command.
 *  \param ... Arguments.
 */
void dio_module_cmd(unsigned short id, unsigned char cmd, ...)
{
  unsigned char buf[100], crc;
  int len, i, v;
  va_list arg;
  unsigned char data[20];

  len = 0;
  va_start(arg, cmd);

  if (id == PD_ID_CO2 && cmd != SBDP_RST_MOD && cmd != SBDP_OFF_MOD)
  {
#if 1
    unsigned char *bptr;
    bptr = (unsigned char*)va_arg(arg, int*);
    memcpy(data, bptr, 20);
    len = va_arg(arg, int);
   // error("%s: change all old calls dio__ with co2_command\n", __FUNCTION__);
#else
    int nsb;

    nsb = 0;

    data[0] = cmd;
    switch(cmd)
    {
      case CO2_CMD_SETGET:
        nsb = (unsigned char)va_arg(arg, int);
        data[1] = nsb;
        for (i=0; i<nsb-1; i++)
        {
          data[2+i] = (unsigned char)va_arg(arg, int);
        }
        break;
    }
    crc = 0;
    for (i=0; i<2+nsb-1; i++)
    {
      crc += data[i];
    }
    crc = (~crc + 1) & 0x7F;
    data[2+nsb-1] = crc;

    len = 2+nsb;
#endif
  } // if (id == PD_ID_CO2)

  switch(cmd)
  {
    case SPO2_SET_SCALE:
      len = 5;
      data[0] = 0xF0;
      data[1] = 0x08;
      data[2] = 0x01;
      data[3] = (unsigned char)va_arg(arg, int);
      data[4] = 0x100 - (data[0]+data[1]+data[2]+data[3]);
      break;
    case ECGM_N_TO_LINE:
    case ECGM_N_TO_GND:
    case ECGM_ECG_TAU_3200:
    case ECGM_ECG_TAU_320:
    case ECGM_ECG_TAU_160:
    case ECGM_RESP_TAU_6800:
    case ECGM_RESP_TAU_320:
   // case ECGM_PM_RL:
   // case ECGM_PM_RF:
   // case ECGM_PM_RC:
    case ECGM_RESP_ON:
    case ECGM_RESP_OFF:
    case ECGM_DHF_ON:
    case ECGM_DHF_OFF:
    case ECGM_DATA_OFF:
    case ECGM_DATA_RESET:
    case ECGM_RESP_CHAN_RL:
    case ECGM_RESP_CHAN_RF:
    case SBDP_RST_MOD:
    case SBDP_OFF_MOD:
    case ECS_RESTART_EP:
    case ECS_RESPCALC_RESET:
    case SBDP_QUERY_VER:
      len = 1;
      data[0] = cmd;
      break;
    case ECS_JPOINT_SHIFT:
    case ECS_PM_NORESP_MS:
    case ECS_RESP_APNOE_S:
      len = 3;
      data[0] = cmd;
      v = va_arg(arg, int);
      data[1] = (unsigned char)v;
      data[2] = (unsigned char)(v>>8);
      break;
    default:
      if (id == PD_ID_CO2) break; // CO2 cmd has been parsed above, leave switch()
      error("cmd 0x%02X for module (0x%X) is not supported\n", cmd, id);
      return;
  } // switch (cmd)
  buf[0] = DOUT_START;
  buf[1] = PD_RESERVED;
  *((unsigned short*)&buf[2]) = id;
  *((unsigned short*)&buf[4]) = len;
  for (i=0; i<len; i++)
  {
    buf[6+i] = data[i];
  }
  crc = 0x00;
  for (i=1; i<6+len; i++)
    crc ^= buf[i];
  buf[6+len] = crc;
  buf[6+len+1] = DOUT_END;
 // nw = com_write(fd, (char*)buf, 8+len);
  len = 8+len;

  if (owptr+len < DIO_TXFIFO_SIZE)
  {
    memcpy(&tx_buf[owptr], buf, len);
    owptr += len;
  }
  else
  {
    memcpy(&tx_buf[owptr], buf, DIO_TXFIFO_SIZE-owptr);
    memcpy(&tx_buf[0], buf+DIO_TXFIFO_SIZE-owptr, len-(DIO_TXFIFO_SIZE-owptr));
    owptr = len-(DIO_TXFIFO_SIZE-owptr);
  }

/*
printf("[");
  for (i=0; i<len+8; i++)
    printf(" %02X ", buf[i]);
printf("]\n");*/

  va_end(arg);
}

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
  unsigned short len;

  b = buf[0];
  if (b != DIN_START)
  {
    error("data start error 0x%02X\n", b);
    return 0;
  }

  id = (buf[3]<<8) | buf[2];
  len = (buf[5]<<8) | buf[4];

  if (len > 40)
  {
    error("invalid length %d\n", len);
   // for (i=0; i<30; i++) printf("%02X (%c)", buf[i], buf[i]);
   // fflush(stdout);
    return 0;
  }

  b = buf[2+2+2+len+2-1];
  if (b != DIN_END)
  {
    error("data end error 0x%02X\n", b);
   // for (i=0; i<30; i++) printf("%02X (%c)", buf[i], buf[i]);
   // fflush(stdout);
    return 0;
  }

  crc = 0;
  for (i=1; i<2+2+2+len+2-2; i++)
    crc ^= buf[i];
  crc_rcvd = buf[2+2+2+len+2-2];

  if (crc != crc_rcvd)
  {
    error("crc error: %02X %02X\n", crc_rcvd, crc);
    error("length %d\n", len);
    return 0;
  }

 // yprintf("%d %d %d\n", id, len, buf[2+2+2]);
  dproc_add_frame(id, 0, len, &buf[2+2+2]);
 // dproc_check_frame(id, 0, len, &buf[2+2+2]);
  return 1;
}

/*! \fn static void dio_process_data(unsigned char *pp, int len)
 *  \brief Send incoming data from SBDP board to parser.
 *  \param pp incoming data buffer.
 *  \param len incoming data length.
 */
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
    if (packet_ptr == 2+2+2)
    {
      packet_len = *((unsigned short*)&packet[4]);
      if (packet_len > 40)
      {
        error("invalid length %d\n", packet_len);
        packet_ptr = 0;
        packet_len = -1;
        continue;
      }
      rptr ++;
      continue;
    }
    if (packet_len > 0 && packet_ptr == 2+2+2+packet_len+2)
    {
      process_packet(packet);
      packet_ptr = 0;
      packet_len = -1;
      rptr ++;
      continue;
    }
    if (packet_ptr > (100-1))
    {
      packet_ptr = 0;
      packet_len = -1;
      continue;
    }
    rptr ++;
  }
}

/*! \fn void dio_suspend(void)
 *  \brief Suspend SBDP i/o.
 */
void dio_suspend(void)
{
  dio_update_suspended = 1;
}

/*! \fn void dio_resume(void)
 *  \brief Resume SBDP i/o.
 */
void dio_resume(void)
{
  cframe_command(CFRAME_RESET, NULL);
  dio_update_suspended = 0;
}

/*! \fn void dio_update(void)
 *  \brief SBDP i/o interface update function.
 */
void dio_update(void)
{
  unsigned char buf[5000];
  int nr, nw;

  if (dio_update_suspended) return;

  if (demo_mode)
  {
    if (fdemo)
    {
#ifdef ARM
      int nblock = ((cframe_t*)(cframe_getptr())->visible) ? 700 : 550;
#else
      const int nblock = 200;
#endif
      nr = fread((char*)buf, 1, nblock, fdemo);
      if (nr > 0) dio_process_data(buf, nr);
      if (feof(fdemo)) fseek(fdemo, 0, SEEK_SET);
    }
    else
    {
      unsigned char crc = 0;
      static char   c = 0;
      ecgm_packet_t ep;
      int i;

      buf[0] = DIN_START; // start
      buf[1] = 0;         // reserved
      *((unsigned short*)&buf[2]) = PD_ID_ECG; // id
      *((unsigned short*)&buf[4]) = ECGM_PACKET_SIZE;
      memset(&ep, 0, ECGM_PACKET_SIZE);
      ep.sync = c;
      ep.break_bits.r = ep.break_bits.l = ep.break_bits.f = ep.break_bits.c = 1;
      *((short*)(ep.data+0)) = c;
      *((short*)(ep.data+2)) = c;
      *((short*)(ep.data+4)) = c;
      memcpy(&buf[6], &ep, ECGM_PACKET_SIZE);
      c ++;
      crc = 0;
      for (i=1; i<6+ECGM_PACKET_SIZE; i++)
      {
        crc ^= buf[i];
      }
      buf[6+ECGM_PACKET_SIZE] = crc;
      buf[6+ECGM_PACKET_SIZE+1] = DIN_END;
      nr = 8+ECGM_PACKET_SIZE;
      dio_process_data(buf, nr);
      if (c % 5 == 0)
      {
        static unsigned char sync = 0;
        buf[0] = DIN_START; // start
        buf[1] = 0;         // reserved
        *((unsigned short*)&buf[2]) = PD_ID_SPO2; // id
        *((unsigned short*)&buf[4]) = SPO2_PACKET_SIZE;
        buf[6] = sync++; // pila
        buf[7] = 0x40;   // stat
        buf[8] = 101;    // spo2
        buf[9] = 0;      // hr lo
        buf[10] = 0;     // hr hi
        buf[11] = 0;     // stolbik
        buf[12] = 0;     // fpg
        buf[13] = 0;     // reserved
        buf[14] = 0;     // crc
        for (i=6; i<14; i++)
        {
          buf[14] += buf[i];
        }
        buf[14] = 0x100 - buf[14];
        crc = 0;
        for (i=1; i<6+SPO2_PACKET_SIZE; i++)
        {
          crc ^= buf[i];
        }
        buf[6+SPO2_PACKET_SIZE] = crc; // packet crc
        buf[6+SPO2_PACKET_SIZE+1] = DIN_END;
        nr = 8+SPO2_PACKET_SIZE;
        dio_process_data(buf, nr);
      }

      // send T1 T2 packet instead of ECG packet
      if (c == 0)
      {
        buf[0] = DIN_START; // start
        buf[1] = 0;         // reserved
        *((unsigned short*)&buf[2]) = PD_ID_T1T2; // id
        *((unsigned short*)&buf[4]) = THERME_PACKET_SIZE;
        buf[6] = 0x7E;  // SOF
        buf[7] = 0xF0;  // M_T1_T2
        buf[8] = 4;     // len
        buf[9] = 0x6E;  // T1 lo
        buf[10] = 0x01; // T1 hi
        buf[11] = 0x6E; // T2 lo
        buf[12] = 0x01; // T2 hi
        buf[13] = 0x00; // therme crc
        for (i=6; i<13; i++)
        {
          buf[13] += buf[i];
        }
        crc = 0;
        for (i=1; i<6+THERME_PACKET_SIZE; i++)
        {
          crc ^= buf[i];
        }
        buf[6+THERME_PACKET_SIZE] = crc; // packet crc
        buf[6+THERME_PACKET_SIZE+1] = DIN_END;
        nr = 8+THERME_PACKET_SIZE;
        dio_process_data(buf, nr);
      }
    }
  }
  else // device mode (normal)
  {
//static int t0 = 0;
//printf("dt=%d\n", gettimemillis()-t0);
//t0 = gettimemillis();
    do
    {
#if 0 //def UNIX
      fcntl(fd, F_SETFL, FNDELAY);
#endif
      if (exit_flag) break;

      nr = com_read(fd, (char*)buf, sizeof(buf));
     // debug("nr=%d (%d)\n", nr, sizeof(buf));
      if (demo_is_recording())
      {
        if (nr > 0)
        {
          demo_record_append(buf, nr);
        }
      }

      dio_process_data(buf, nr);
    //  if (nr > 0) csio_write((void*)buf, nr);
    } while (nr > 0);

    if (orptr != owptr)
    {
      if (owptr > orptr)
      {
        nw = com_write(fd, (char*)&tx_buf[orptr], owptr-orptr);
        if (nw > 0) orptr += nw;
      }
      else
      {
        nw = com_write(fd, (char*)&tx_buf[orptr], DIO_TXFIFO_SIZE-orptr);
        if (nw > 0) orptr += nw;
        if (orptr == DIO_TXFIFO_SIZE)
        {
          orptr = 0;
          nw = com_write(fd, (char*)&tx_buf[0], owptr);
          if (nw > 0) orptr += nw;
        }
      }
    }
  }
#if 0
  static unsigned char cnt = 0;
  sprintf(buf, "msg %d\0", cnt++);
  csio_write((void*)buf, strlen(buf)+1);
#endif
}
