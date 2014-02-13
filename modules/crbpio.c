/*! \file crbpio.c
    \brief CRBP board i/o functions.
*/

#ifdef UNIX
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "port.h"
#include "cframe.h"
#include "sframe.h"
#include "mframe.h"
#include "lframe.h"
#include "tframe.h"
#include "nibp.h"
#include "inpfoc.h"
#include "sched.h"
#include "bedmoni.h"
#include "uframe.h"
#include "sframe.h"
#include "iframe.h"
#include "disp.h"
#include "powerman.h"
#include "lowbatcharge.h"
#include "crbpio.h"
#if defined (FEATURE_KEYBOARD)
#include "utils.h" // kbhit function
#endif

#include "dio.h"
#include "ecgm.h"

#ifdef CRBPIO_DEBUG
#include <stdarg.h>
#endif

#ifdef CRBPIO_LOGGING
static FILE *flog = 0;
#endif

extern volatile int exit_flag;

#if !defined (CRBPIO_OUT_DIRECTLY)
static unsigned char crbpout_buf[CRBPOUT_BUF_SIZE];
int crbpout_wptr = 0;
int crbpout_rptr = 0;
int crbpout_cnt = 0;
#endif

static unsigned char crbpin_buf[CRBPIN_BUF_SIZE];
int crbpin_ptr = 0;
/*! \var static int fd
 *  \brief descriptor, associated with the CRBP board comm port.
 */
static int fd = 0;

/*! \fn int crbpio_init(void)
 *  \brief CRBP i/o interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
int crbpio_init(void)
{
#if defined(FEATURE_KEYBOARD)
  debug("crbp io - keyboard\n");
#endif
  debug("crbp io port: %s\n", CRBP_COMMPORT);
  fd = com_open(CRBP_COMMPORT, BR19200, 0);
  if (fd < 0)
  {
    error("unable to open %s\n", CRBP_COMMPORT);
    return -1;
  }

  crbpin_ptr = 0;
  memset(crbpin_buf, 0, sizeof(crbpin_buf));

#ifdef CRBPIO_LOGGING
  flog = fopen("crbpio.log", "wt");
#endif

  disp_ini();

  return 0;
}

/*! \fn void crbpio_deinit(void)
 *  \brief CRBP i/o interface deinit function.
 */
void crbpio_deinit(void)
{
#ifdef CRBPIO_LOGGING
  if (flog) fclose(flog);
#endif

  disp_deini();

  com_close(fd);
}

/*! \fn void crbpio_debug(char *fmt, ...)
 *  \brief flush CRBP debug information to console.
 *  \param fmt format string.
 *  \param ... arguments.
 */
void crbpio_debug(char *fmt, ...)
#ifdef CRBPIO_DEBUG
{
  va_list  args;
  char s[200];
  va_start(args, fmt);
  yfprintf(stdout, "crbpio: ");
  vsprintf(s, fmt, args);
  yfprintf(stdout, s);
  fflush(stdout);
#ifdef CRBPIO_LOGGING
  fprintf(flog, "crbpio: ");
  vfprintf(flog, fmt, args);
  fflush(flog);
#endif
  va_end( args );
}
#else // !CRBPIO_DEBUG
{}
#endif

/*! \fn void crbpio_send_msg(unsigned char id, unsigned char b3, unsigned char b2, unsigned char b1, unsigned char b0)
 *  \brief Send message to CRBP board.
 *  \param id Message ID.
 *  \param b3 Message byte b3.
 *  \param b2 Message byte b2.
 *  \param b1 Message byte b1.
 *  \param b0 Message byte b0.
 */
void crbpio_send_msg(unsigned char id, unsigned char b3, unsigned char b2, unsigned char b1, unsigned char b0)
{
  unsigned char buf[CRBP_MSG_SIZE];
  int i;

  buf[0] = CRBP_MSG_SOF;
  buf[1] = CRBPOUT_MSG_ID;
  buf[2] = id;
  buf[3] = b3;
  buf[4] = b2;
  buf[5] = b1;
  buf[6] = b0;

  buf[7] = 0;
  for(i=0; i<CRBP_MSG_SIZE-1; i++)
    buf[7] += buf[i];

#ifdef CRBPIO_DEBUG
  fprintf(stdout, "OUT: ");
  for (i=0; i<CRBP_MSG_SIZE; i++)
  {
    fprintf(stdout, "%02X ", buf[i]);
  }
  fprintf(stdout, "\n");
  fflush(stdout);
#endif

#if !defined (CRBPIO_OUT_DIRECTLY)
  if (id == CRBP_RESP_STAT || id == CRBP_POWEROFF_OK)
#endif
  {
    int nw_total, nw;
    for (i=0, nw_total=nw=0; i<10; i++)
    {
      nw = com_write(fd, (char*)buf+nw_total, CRBP_MSG_SIZE-nw_total);
      if (nw > 0) nw_total += nw;
      if (nw_total == CRBP_MSG_SIZE) break;
    }
    if (nw_total != CRBP_MSG_SIZE && fd > 0)
    {
      error("%s: not all bytes sent by com_write (cmd %d)\n", __FUNCTION__, id);
    }
  }

/*#if defined (CRBPIO_OUT_DIRECTLY)
  int nw_total, nw;
  for (i=0, nw_total=nw=0; i<10; i++)
  {
    nw = com_write(fd, (char*)buf+nw_total, CRBP_MSG_SIZE-nw_total);
    if (nw > 0) nw_total += nw;
    if (nw_total == CRBP_MSG_SIZE) break;
  }
  if (nw_total != CRBP_MSG_SIZE && fd > 0)
  {
    error("%s: not all bytes sent by com_write (cmd %d)\n", __FUNCTION__, id);
  }
#else
*/
#if !defined (CRBPIO_OUT_DIRECTLY)
  if (crbpout_wptr+CRBP_MSG_SIZE < CRBPOUT_BUF_SIZE)
  {
    memcpy((unsigned char*)crbpout_buf+crbpout_wptr, buf, CRBP_MSG_SIZE);
  }
  else
  {
    memcpy((unsigned char*)crbpout_buf+crbpout_wptr, buf, CRBPOUT_BUF_SIZE-crbpout_wptr);
    memcpy((unsigned char*)crbpout_buf+0, buf+CRBPOUT_BUF_SIZE-crbpout_wptr, CRBP_MSG_SIZE-(CRBPOUT_BUF_SIZE-crbpout_wptr));
  }
  crbpout_cnt += CRBP_MSG_SIZE;
  crbpout_wptr += CRBP_MSG_SIZE;
  if (crbpout_wptr >= CRBPOUT_BUF_SIZE) crbpout_wptr -= CRBPOUT_BUF_SIZE;
  if (crbpout_cnt >= CRBPOUT_BUF_SIZE)
  {
#if !defined (FEATURE_KEYBOARD)
    error("%s: queue overflow\n", __FUNCTION__);
#endif
  }
#endif
}

/*! \fn static int crbpio_process_msg(unsigned char *buf)
 *  \brief Process CRBP board message, located in <i>buf</i>.
 *  \param buf Buffer, containing incoming message data.
 *  \return 1 - if message is valid and successfully precessed, 0 - otherwise.
 */
static int crbpio_process_msg(unsigned char *buf)
{
  unsigned char crc;
  static int bat_avtime_old = 0, mains_old = -1;
  long v;
  int i;
  alarm_info_t ai;

#ifdef CRBPIO_DEBUG
  fprintf(stdout, "IN: ");
  for (i=0; i<CRBP_MSG_SIZE; i++)
  {
    fprintf(stdout, "%02X ", buf[i]);
  }
  fprintf(stdout, "\n");
  fflush(stdout);
#endif

  if (buf[0] != CRBP_MSG_SOF)
  {
    error("crbpio_process_msg: SOF error\n");
    return 0;
  }
  if (buf[1] != CRBPIN_MSG_ID)
  {
    error("crbpio_process_msg: ID error\n");
    return 0;
  }
  crc = 0;
  for (i=0; i<CRBP_MSG_SIZE-1; i++)
  {
    crc += buf[i];
  }
  if (buf[CRBP_MSG_SIZE-1] != crc)
  {
    error("crbpio_process_msg: CRC error\n");
    return 0;
  }

#if 0
  int k;
  for (k=0; k<8; k++)
  {
    yfprintf(stdout, "0x%02X ", buf[k]);
    fflush(stdout);
  }
  yfprintf(stdout, "\n");
#endif

  // b3 - buf[3], b2 - buf[4], b1 - buf[5], b0 - buf[6]

 // crbpio_debug("buf[2] = 0x%02X\n", buf[2]);
  switch(buf[2]) // command
  {
    case CRBP_SYS_STAT:
//       power_bits_t power_bits;
      crbpio_debug("CRBP_SYS_STAT\n");
//       memcpy(&power_bits, buf[4], sizeof(unsigned char));  // read power cfg from B2
      v = buf[3]; // read power cfg from B3
#if 0
      crbpio_debug("bat: %s\n", (v & DEV_MASK_BAT) ? STR_ON : STR_OFF);
      crbpio_debug("audio: %s\n", (v & DEV_MASK_AUDIO) ? STR_ON : STR_OFF);
      crbpio_debug("usbhub: %s\n", (v & DEV_MASK_USBHUB) ? STR_ON : STR_OFF);
      crbpio_debug("ftdi: %s\n", (v & DEV_MASK_FTDI) ? STR_ON : STR_OFF);
      crbpio_debug("nibp: %s\n", (v & DEV_MASK_NIBP) ? STR_ON : STR_OFF);
      crbpio_debug("sbdp: %s\n", (v & DEV_MASK_SBDP) ? STR_ON : STR_OFF);
      crbpio_debug("printer: %s\n", (v & DEV_MASK_TPRN) ? STR_ON : STR_OFF);
      crbpio_debug("mains: %s\n", (v & DEV_MASK_MAINS) ? STR_ON : STR_OFF);
#endif
    //  powerman_boards_status(buf[3]);

      powerman_stat_get((unsigned long*)&v);
      if ( memcmp(&v, &buf[3], 4) != 0)
      {
        memcpy(&v, &buf[3], sizeof(int));
        powerman_stat(v);
        sframe_command(SFRAME_SET_BATSTAT, (void*)v);
      }

     // iframe_command(IFRAME_SET_BEDNO, (void*)((int)(buf[6]&0x7F)));

      v = (buf[4] << 8) | buf[5];

     // iframe_command(IFRAME_SET_CARDNO, (void*)v);
      if ( v != bat_avtime_old || ((buf[3] & DEV_MASK_MAINS) != mains_old) )
      {
        if ((buf[3] & DEV_MASK_MAINS) == 0)
        {
          if (v < 15) // less than 15 mins to work (shutdown in 60 s)
          {
            debug("%s: less than 0 minutes to work (%d mins) -> shut down in 60s\n", __FUNCTION__, v);
            alarm_clr(SYS_LOWCHARGE);
            alarm_set(SYS_EXTREMLOWCHARGE);
            if (!sched_is_started(SCHED_LOWBAT_SHUTDOWN))
            {
              uframe_command(UFRAME_CREATE, (void*)MENU_LOWBATSHUTDOWN);
              sched_start(SCHED_LOWBAT_SHUTDOWN, 60*1000, lowbat_shutdown, SCHED_DO_ONCE);
            }
          }
          else
          if (v < 30) // less than 30 mins to work (extreme low charge)
          {
            debug("%s: less than 15 minutes to work (%d mins)\n", __FUNCTION__, v);
           // alarm_clr(SYS_EXTREMLOWCHARGE);
            if (!alarm_isset(SYS_LOWCHARGE) && !alarm_isset(SYS_EXTREMLOWCHARGE))
              uframe_command(UFRAME_CREATE, (void*)MENU_LOWBATCHARGE);
            alarm_set(SYS_LOWCHARGE);
          }
          else
          {
           // alarm_clr(SYS_LOWCHARGE);
           // alarm_clr(SYS_EXTREMLOWCHARGE);
          }
        }
        else
        {
          sched_stop(SCHED_LOWBAT_SHUTDOWN);
          alarm_clr(SYS_LOWCHARGE);
          alarm_clr(SYS_EXTREMLOWCHARGE);
        }
        mains_old = (buf[3] & DEV_MASK_MAINS);
        bat_avtime_old = v;

       // sframe_command(SFRAME_SET_MAINS, (void*)(buf[3] & DEV_MASK_MAINS)); // mains
      }
      break;
    case CRBP_KBD_STAT:
     // crbpio_debug("kbd: %02X %02X %02X %02X\n", buf[3], buf[4], buf[5], buf[6]);
      if (buf[3] & CRBP_KBD_BPM_BMASK && buf[4] & CRBP_KBD_BPM_BMASK)
      {
        if (nibp_is_running())
          nibp_command(NIBP_CMD_ABORT_BP);
        else
        {
          nibp_do_bp();
        }
      }
      if (buf[3] & CRBP_KBD_MENU_BMASK && buf[4] & CRBP_KBD_MENU_BMASK)
      {
        uframe_command(UFRAME_CREATE, (void*)MENU_GENERAL);
      }
      if (buf[3] & CRBP_KBD_SEL_BMASK && buf[4] & CRBP_KBD_SEL_BMASK)
      {
        inpfoc_press();
      }
      if (buf[3] & CRBP_KBD_PRN_BMASK && buf[4] & CRBP_KBD_PRN_BMASK)
      {
#if 0 // screeenshot make mode
#ifndef ARM
        debug("screenshot done\n");
        fb2bmp24(0, 0, 800, 480, "./1.bmp");
        break;
#endif
#endif
        tprn_cfg_t tprn_cfg;
        tprn_cfg_get(&tprn_cfg);
        if (!tprn_cfg.prnmask.started)
        {
          debug("printer operation started\n");
          // has to be launched from lower task
          sched_start(SCHED_ANY, 0, tprn_start, SCHED_DO_ONCE);
         // tprn_start();
        }
        else
        {
          debug("printer operation aborted\n");
          tprn_stop();
        }
#ifndef ARM
        alarm_cfg_t ac;
        alarm_cfg_get(&ac);
        if (ac.ge)
        {
          debug("disable sound for %d seconds\n", ac.delay);
          iframe_command(IFRAME_SET_TIMERVAL, (void*)((int)ac.delay));
          alarm_ge_off();
        }
        else
        {
          debug("reset all set alarms\n");
          alarm_rst();
        }
#endif
      }
      if (buf[3] & CRBP_KBD_FRZ_BMASK && buf[4] & CRBP_KBD_FRZ_BMASK)
      {
        cframe_command(CFRAME_FREEZE_SCREEN, NULL);
      }
      if (buf[3] & CRBP_KBD_MSCR_BMASK && buf[4] & CRBP_KBD_MSCR_BMASK)
      {
#ifdef SFRAME_BMONI_TS
        if (buf[3] & CRBP_KBD_FRZ_BMASK && buf[4] & CRBP_KBD_FRZ_BMASK)
        {
          // if 'freeze' and 'home' buttons pressed simultaneously
          cframe_command(CFRAME_FREEZE_SCREEN, NULL);
          sframe_command(SFRAME_TOGGLE_VERMODE, NULL);
          sched_start(SCHED_ANY, 5*1000, sframe_bmoni_ts_disable, SCHED_DO_ONCE);
          dio_module_cmd(PD_ID_NONE, SBDP_QUERY_VER); // query sbdp firmware version

          char cmd[100];
          sprintf(cmd, "fw_printenv serial_no");
          system(cmd);

          unsigned long v2;
          powerman_stat_get((unsigned long*)&v2);
          debug("power_status=0x%02X\n", (unsigned char)(v2 & 0xff));
        }
#endif
#if 0
        if (buf[3] & CRBP_KBD_MENU_BMASK && buf[4] & CRBP_KBD_MENU_BMASK)
        {
          uframe_command(UFRAME_CREATE, (void*)MENU_SWUPDATE);
          break;
        }
#endif
        dview_set_state(1);
        uframe_command(UFRAME_DESTROY, NULL);
       // tframe_command(TFRAME_DESTROY, NULL);
       // lframe_command(LFRAME_DESTROY, NULL);
        dview_frame_active(CFRAME);
        cframe_command(CFRAME_RELOAD, NULL);
        if (mframe_ismaximized())
          mframe_command(MFRAME_MAXIMIZE, NULL);
        mframe_command(MFRAME_RELOAD, NULL);
        iframe_command(IFRAME_RELOAD, NULL);
        sframe_command(SFRAME_RELOAD, NULL);
      }
      if (buf[3] & CRBP_KBD_SLS_BMASK && buf[4] & CRBP_KBD_SLS_BMASK)
      {
        alarm_cfg_t ac;
        alarm_cfg_get(&ac);
        if (ac.ge)
        {
          debug("disable sound for %d seconds\n", ac.delay);
          iframe_command(IFRAME_SET_TIMERVAL, (void*)((int)ac.delay));
          alarm_ge_off();
        }
        else
        {
          debug("reset all set alarms\n");
          alarm_rst();
        }
      }
      if ((signed char)buf[6] > 0)
      {
        for (i=0; i<(signed char)buf[6]; i++)
        {
          crbpio_debug(">");
          inpfoc_shr(1);
        }
      }
      if ((signed char)buf[6] < 0)
      {
        for (i=0; i<(-(signed char)buf[6]); i++)
        {
          crbpio_debug("<");
          inpfoc_shl(1);
        }
      }
      break;
    case CRBP_VOL_STAT:
      break;
    case CRBP_ALARM_RESP:
      if (buf[4] != BeepHeartRateSound)
      {
        alarm_info_get(&ai);
        crbpio_debug("CRBP_ALARM_RESP [%d]\n", buf[4]);
        ai.current_sound_alarm = buf[4]; // sound alarm id in B2
        alarm_info_set(&ai);
      }
      break;
    case CRBP_POWEROFF_QUERY:
      // safe exit
      debug("power off query\n");
      dview_set_state(1);
      uframe_command(UFRAME_CREATE, (void*)MENU_SHUTDOWN);
      break;
    case CRBP_LIGHT_STAT:
      break;
    case CRBP_ERR_STAT:
      switch(buf[3]) // id
      {
        case 0x01: // bat
          break;
        case 0x02: // audio
          break;
        case 0x03: // flash
          break;
        case 0x04: // rs-232
          break;
        case 0x05: // nibp
          break;
        case 0x06: // 56pb
          break;
        case 0x07: // printer
          break;
        case 0x08: // ftdi
          break;
      }
      if (buf[5] == 0x01) // code
      {;} // device is not responding
      if (buf[5] == 0x02) // code
      {;} // too many errors in device
      break;
    case CRBP_DATE_STAT:
     // sprintf(s, "%02X.%02X.%04X", buf[6], buf[5], buf[4]); // dd:mm:yyyy
     // sframe_set(SFRAME_ID_DATE, s);
      debug("%s: not implemented\n", __FUNCTION__);
      break;
    case CRBP_TIME_STAT:
     // sprintf(s, "%02X:%02X", buf[4], buf[5]); // hh:mm
     // sframe_set(SFRAME_ID_TIME, s);
      debug("%s: not implemented\n", __FUNCTION__);
      break;
    default:
      error("crbpio_process_msg: INV CMD 0x%X\n", buf[2]);
      break;
  }
  return 1;
}

/*! \fn static void crbpio_add_byte(unsigned char b)
 *  \brief Send one more incoming character from CRBP board to message parser.
 *  \param b incoming character.
 */
static void crbpio_add_byte(unsigned char b)
{
  crbpin_buf[crbpin_ptr] = b;
  if (crbpin_buf[0] == CRBP_MSG_SOF)
    crbpin_ptr ++;

  if (crbpin_ptr == CRBP_MSG_SIZE)
  {
    if ( crbpio_process_msg(crbpin_buf) )
    {
      crbpin_ptr = 0;
    }
    else
    {
      int i;
      for (i=0+1; i<CRBP_MSG_SIZE; i++)
      {
        if (crbpin_buf[i] == CRBP_MSG_SOF) break;
      }
      if (i != CRBP_MSG_SIZE)
      {
        memmove(crbpin_buf, &crbpin_buf[i], CRBP_MSG_SIZE-i);
      }
      crbpin_ptr = CRBP_MSG_SIZE-i;
    }
  }

  if (crbpin_ptr >= CRBP_MSG_SIZE) // never true
  {
    error("crbpio_add_byte: overflow %d\n", crbpin_ptr);
    crbpin_ptr = 0;
  }
}

/*! \fn void crbp_resp_stat(void)
 *  \brief Send alive status massage to CRBP board to prevent CRBP watchdog reset.
 */
void crbp_resp_stat(void)
{
#if 0
  static long t = 0, t0 = 0;
  t = gettimemillis();
  debug("shake %d\n", t-t0);
  t0 = t;
#endif
  crbpio_send_msg(CRBP_RESP_STAT, 0x00, 0x00, 0x00, 0x00);
}

/*! \fn void crbpio_update(void)
 *  \brief CRBP i/o interface update function.
 */
void crbpio_update(void)
{
  unsigned char buf[300];
  int nr, i;

#if defined (FEATURE_KEYBOARD)
  if (kbhit())
  {
    unsigned char msg[8];
    msg[0] = CRBP_MSG_SOF;
    msg[1] = CRBPIN_MSG_ID;
    int c = getchar();
   // printf("PRESSED %c %X\n", c, c);
    switch(c)
    {
      case 'q': // power off
        msg[2] = CRBP_POWEROFF_QUERY;
        msg[3] = msg[4] = msg[5] = msg[6] = 0x00;
        break;
      case 'm': // menu
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = CRBP_KBD_MENU_BMASK;
        msg[5] = msg[6] = 0x00;
        break;
      case 'h': // home
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = CRBP_KBD_MSCR_BMASK;
        msg[5] = msg[6] = 0x00;
        break;
      case 'f': // freeze
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = CRBP_KBD_FRZ_BMASK;
        msg[5] = msg[6] = 0x00;
        break;
      case 's': // silence
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = CRBP_KBD_SLS_BMASK;
        msg[5] = msg[6] = 0x00;
        break;
      case 'p': // print
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = CRBP_KBD_PRN_BMASK;
        msg[5] = msg[6] = 0x00;
        break;
      case 'b': // blood pressure measurement
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = CRBP_KBD_BPM_BMASK;
        msg[5] = msg[6] = 0x00;
        break;
      case 0x0A: // select
      case 0x20: // "enter" or "space"
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = CRBP_KBD_SEL_BMASK;
        msg[5] = msg[6] = 0x00;
        break;
      case 0x2C: // <-
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = msg[5] = 0x00;
        msg[6] = -1;
        break;
      case 0x2E: // ->
        msg[2] = CRBP_KBD_STAT;
        msg[3] = msg[4] = msg[5] = 0x00;
        msg[6] = +1;
        break;
    }
    msg[7] = 0x00;
    for (i=0; i<7; i++)
      msg[7] += msg[i];
    crbpio_process_msg(msg);
  }
#endif

  do
  {
    nr = com_read(fd, (char*)buf, sizeof(buf));
    if (nr > 0)
    {
      for (i=0; i<nr; i++)
      {
        crbpio_add_byte(buf[i]);
     // crbpio_debug("<- 0x%X\n", buf[i]);
      }
    }
  }
  while (nr > 0);

#if !defined(CRBPIO_OUT_DIRECTLY)
  while (crbpout_cnt > 0)
  {
    int nw, n;
    if (crbpout_wptr > crbpout_rptr)
    {
      n = crbpout_wptr-crbpout_rptr;
      memcpy(buf, &crbpout_buf[crbpout_rptr], crbpout_wptr-crbpout_rptr);
    }
    else
    {
      n = CRBPOUT_BUF_SIZE+crbpout_wptr-crbpout_rptr;
      memcpy(&buf[0], &crbpout_buf[crbpout_rptr], CRBPOUT_BUF_SIZE-crbpout_rptr);
      memcpy(&buf[CRBPOUT_BUF_SIZE-crbpout_rptr], &crbpout_buf[0], n-(CRBPOUT_BUF_SIZE-crbpout_rptr));
    }
    nw = com_write(fd, (char*)buf, n);
    if (nw <= 0)
    {
#if !defined (FEATURE_KEYBOARD)
      error("%s: write port error (%d sent)\n", __FUNCTION__, nw);
#endif
      break;
    }
    crbpout_rptr += nw;
    if (crbpout_rptr >= CRBPOUT_BUF_SIZE) crbpout_rptr -= CRBPOUT_BUF_SIZE;
    crbpout_cnt -= nw;
  }
#endif
}

