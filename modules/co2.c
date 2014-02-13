#include <stdarg.h>
#include <string.h>
#include "bedmoni.h"
#include "mframe.h"
#include "dproc.h"
#include "csio.h"
#include "dio.h"
#include "co2.h"

//typedef struct
//{
//  unsigned char cmd;
//  unsigned char nbf;
//  unsigned char data[20];
//  unsigned char retry_count;
//  unsigned char running;
//} co2_cmd_ex_t;

//static co2_cmd_ex_t co2_cmd_ex[CO2_MAX_NUM_CMDS_IN_QUEUE];
//static int co2_num_cmds_running = 0;
static co2_errstat_bits_t co2_errstat;

void co2_command_queue_init(void)
{
//  memset(&co2_cmd_ex, 0, sizeof(co2_cmd_ex));
}

void co2_command(int cmd, int nbf, ...)
{
  unsigned char buf[20], i, crc;
  va_list arg;

  if (nbf >= 20-3) return;
  va_start(arg, nbf);
  buf[0] = cmd;
  buf[1] = nbf;

  for (i=0,crc=0; i<nbf+2-1; i++)
  {
    if (i>=2)
    {
      buf[i] = (unsigned char)va_arg(arg, int);
    }
    crc = (signed char) (crc + buf[i]);
  }
  crc = (signed char) ((-crc) & 0x7F);
  buf[nbf+2-1] = crc;

#if 0
  if (co2_num_cmds_running >= CO2_MAX_NUM_CMDS_IN_QUEUE)
  {
    error("co2 cmds overflow\n");
    return;
  }
  for (i=0; i<CO2_MAX_NUM_CMDS_IN_QUEUE; i++)
  {
    if (memcmp(&co2_cmd_ex[i].cmd, buf, nbf+2) == 0)
    {
      co2_cmd_ex[i].running = 0;
      break;
    }
  }
  if (i == CO2_MAX_NUM_CMDS_IN_QUEUE)
  {
    for (i=0; i<CO2_MAX_NUM_CMDS_IN_QUEUE; i++)
    {
      if (co2_cmd_ex[i].running == 0) break;
    }
  }
  if (i >= CO2_MAX_NUM_CMDS_IN_QUEUE)
  {
    // overflow
    return;
  }
  memcpy(&co2_cmd_ex[i].cmd, buf, nbf+2);
  co2_cmd_ex[i].retry_count = 0;
  co2_cmd_ex[i].running = 1;
  co2_num_cmds_running ++;
#endif
  dio_module_cmd(PD_ID_CO2, cmd, buf, nbf+2);
}

static void co2_process_set(unsigned char * buf, int nbf)
{
#if 0
  int i;
  for (i=0; i<CO2_MAX_NUM_CMDS_IN_QUEUE; i++)
  {
    if (co2_cmd_ex[i].running)
    {

    }
  }
#endif
}

void co2_debug(char *fmt, ...)
#ifdef CO2_DEBUG
{
  va_list  args;
  char s[200];
  va_start(args, fmt);
  yfprintf(stdout, "co2: ");
  vsprintf(s, fmt, args);
  yfprintf(stdout, s);
  fflush(stdout);
  va_end( args );
}
#else // !CO2_DEBUG
{}
#endif

void co2_process_packet(unsigned char *data, int len)
{
  co2_data_t co2_data;
  unsigned char crc, st;
  static unsigned char old_sync = 0;
 // short w;
  int v;
  int i;
  static int startup_counter = 0;

  startup_counter ++;
  if (startup_counter > 60*100) // 60 sec = 1 min
  {
    alarm_clr(CO2_STARTUP);
    startup_counter--;
  }

#if 0
  for (i=0; i<len; i++)
  {
    printf("%02X ", data[i]);
  }
  printf("\n");
  dview_chandata_add(CO2, data[2]);
  return;
#endif

  if ((data[0] & 0x80) == 0x00)
  {
    error("CO2 SOF error\n");
    return;
  }
  if (data[1] > 30)
  {
    error("CO2 length (%d) error\n", data[1]);
    return;
  }
  crc = 0x00;
  for (i=0; i<2+data[1]-1; i++)
  {
    crc += data[i];
  }
  crc = (~crc + 1) & 0x7F;
  if (crc != data[2+data[1]-1])
  {
    error("CO2 crc error 0x%02X 0x%02X\n", data[2+data[1]-1], crc);
    return;
  }
  for (i=1; i<2+data[1]-1; i++)
  {
    if (data[i] & 0x80)
    {
      error("CO2 format data error\n");
      return;
    }
  }

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading co2 data\n", __FUNCTION__);
  }

  st = 0;
  switch (data[0])
  {
    case CO2_CMD_WAVEFORM: //0x80:
      if (data[2] - old_sync != 0x01 && data[2] - old_sync != -0x7F)
      {
        error("CO2 sync error 0x%02X 0x%02X\n", old_sync, data[2]);
        old_sync = data[2];
        return;
      }
      v = ((128*data[3]) + data[4]) - 1000;
     // if (v < -900) v += 0x4000;
     // if (data[3] & 0x40) v -= 0x4000;
//printf("v=%d %02X (%d %d), %02X%02X\n",v, data[2], data[1], data[5], data[3], data[4]);
      if (v == -1000)
      {
        alarm_set_clr(CO2_INVDATA, (co2_errstat.calib_stat = 0x00 && data[7] == 0) ? 1 : 0);
       // v = 0;
      }
      dview_chandata_add(CO2, v/100);

      csio_co2data_c_t csio_co2data_c;
      memset(&csio_co2data_c, 0, sizeof(csio_co2data_c_t));
      csio_co2data_c.sync = data[2];
      csio_co2data_c.data = v/100;
      csio_send_msg(CSIO_ID_CO2_C, &csio_co2data_c, sizeof(csio_co2data_c_t));

      old_sync = data[2];
      if (data[1] > 4) // check packet for dpi data
      {
        switch(data[5])
        {
          case 1: // CO2 status/errors
            co2_debug("co2 err/stat case %02X %02X %02X %02X  %02X\n", data[6], data[7], data[8], data[9], data[10]);
            memcpy(&co2_errstat, &data[6], sizeof(co2_errstat_bits_t));
           /* if ((data[7] & 0x0C) == 0x04)
            {
              alarm_set(CO2_ZEROING);
            }
            else
              alarm_clr(CO2_ZEROING);*/
            alarm_set_clr(CO2_WARMUP, co2_errstat.temper_stat == 0x01 ? 1 : 0);
            if (co2_errstat.temper_stat == 0x01)
            {
              co2_undef_state();
            }
            if (co2_errstat.check_adapter)
            {
              alarm_set(CO2_CHECKSENSOR);
              co2_undef_state();
            }
            else
              alarm_clr(CO2_CHECKSENSOR);
            alarm_set_clr(CO2_ZEROING, co2_errstat.calib_stat == 0x01);
            if (!alarm_isset(CO2_APNOE) && co2_errstat.apnoe && co2_errstat.calib_stat == 0x00 && co2_errstat.check_adapter == 0 && co2_errstat.temper_stat == 0 && startup_counter >= 60*100)
            {
              alarm_set(CO2_APNOE);
              unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BRSTATE, IDS_APNOE);
              unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR, (int)UNDEF_VALUE);
              unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2, (int)UNDEF_VALUE);
              unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2, (int)UNDEF_VALUE);
            }
            if ( co2_errstat.calib_stat > 0x01/* || (co2_errstat.check_adapter && co2_errstat.calib_stat != 0x01)*/ )
            {
//printf("CO2 ZERO\n");
              co2_undef_state();
              co2_command(0x82, 1, 0); // initiate capnostat zeroing
            }
            alarm_set_clr(CO2_OVERTEMP, (data[7] & 0x03) == 0x2);
            break;
          case 2: // ETCO2 x10
            if (startup_counter < 60*100) // wait 1 min
            {
              co2_undef_state();
              break;
            }
            v = (data[6] * 128 + data[7]) / 10;
            if (co2_errstat.breath_ok && v <= 150 && v > 0)
            {
              co2_data.etco2 = v;
              st = 0;
              if      (co2_data.etco2 < co2_data.etco2_min) st |= TR_ST_L;
              else if (co2_data.etco2 > co2_data.etco2_max) st |= TR_ST_U;
              dproc_add_trv(CO2, PARAM_ETCO2, v, st);
            }
            else
            {
              co2_data.etco2 = UNDEF_VALUE;
            }
            if (co2_errstat.calib_stat != 0x00)
            {
              co2_data.etco2 = UNDEF_VALUE;
              st = 0;
            }
            alarm_set_clr(CO2_RISKETCO2, st);
            unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2, co2_data.etco2);
            break;
          case 3: // respiration rate
            if (startup_counter < 60*100) // wait 1 min
            {
              co2_undef_state();
              break;
            }
            v = data[6] * 128 + data[7];
            st = 0;
            if (v == 0)
            {
              // one more check for apnoea
            /*  if (!alarm_isset(CO2_APNOE) && co2_errstat.calib_stat != 0x01)
              {
                alarm_set(CO2_APNOE);
                ids2string(IDS_APNOE, s);
                unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BRSTATE, s);
                unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR, (int)UNDEF_VALUE);
                unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2, (int)UNDEF_VALUE);
                unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2, (int)UNDEF_VALUE);
              }*/
              co2_data.br = UNDEF_VALUE;
            }
            else
            if (co2_errstat.breath_ok && v <= 150 && !alarm_isset(CO2_APNOE))
            {
              co2_data.br = v;
              if      (co2_data.br < co2_data.br_min) st |= TR_ST_L;
              else if (co2_data.br > co2_data.br_max) st |= TR_ST_U;
              dproc_add_trv(CO2, PARAM_BRC, v, st);
            }
            else
            {
              co2_data.br = UNDEF_VALUE;
            }
            if (co2_errstat.calib_stat != 0x00)
            {
              co2_data.br = UNDEF_VALUE;
              st = 0;
            }
            unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR, co2_data.br);
            alarm_set_clr(CO2_RISKBRCO2, st);
            break;
          case 4: // inspired CO2 x10
            if (startup_counter < 60*100) // wait 1 min
            {
              co2_undef_state();
              break;
            }
            v = (data[6] * 128 + data[7]) / 10;
            if (co2_errstat.breath_ok && v <= 150 && v > 0) // v<=100?
            {
              co2_data.ico2 = v;
              st = 0;
              if (co2_data.ico2 > co2_data.ico2_max) st |= TR_ST_U;
              dproc_add_trv(CO2, PARAM_ICO2, v, st);
            }
            else
            {
              co2_data.ico2 = UNDEF_VALUE;
            }
            if (co2_errstat.calib_stat != 0x00)
            {
              co2_data.etco2 = UNDEF_VALUE;
              st = 0;
            }
            unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2, co2_data.ico2);
            alarm_set_clr(CO2_RISKICO2, st);
            break;
          case 5: // breath detected flag
            unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BRSTATE, IDS_NORM);
            alarm_clr(CO2_APNOE);
            break;
          case 7: // hardware status
            error("co2 hw error 0x%02X\n", data[6]);
            break;
        }
      }
      break;
    case CO2_CMD_ZERO:   //0x82:
      co2_debug("co2 zero cmd\n");
      switch (data[2])
      {
        case 0:
          co2_debug("co2 zero started\n");
          break;
        case 1:
          co2_debug("co2 not ready for zeroing\n");
          break;
        case 2:
          co2_debug("co2 zeroing in progress\n");
         // alarm_clr_fixed(CO2_APNOE); // TODO: fake!! - replace with alarm_clr
          co2_undef_state();
          alarm_set(CO2_ZEROING);
          break;
        case 3:
          co2_debug("breath detected during co2 zeroing\n");
          break;
      }
      break;
    case CO2_CMD_SETGET: //0x84:
      co2_process_set(data, data[1]);
#ifdef CO2_DEBUG
      yprintf("isb %d:\t", data[2]);
      for (i=3; i<2+data[1]-1; i++)
      {
        yprintf("0x%02X ", data[i]);
      }
      yprintf("\n");
#endif
      break;
    case 0xC8: // nack error
      if (data[1] == 0x2 && data[2] == 0x00) // boot code
      {
        co2_debug("co2 boot code\n");
        startup_counter = 0;
        alarm_set(CO2_STARTUP);
        unit_get_data(CO2, &co2_data);
        co2_command(0x84, 4, 1,  (co2_data.pressure/0x80)&0x7F,co2_data.pressure&0x7F,0);
        co2_command(0x84, 6, 11, co2_data.o2_compens,co2_data.gas_balance,(co2_data.anest_agent/0x80)&0x7F,co2_data.anest_agent&0x7F,0);
        co2_command(0x84, 3, 7,0x00,0);
        co2_command(0x84, 3, 6,co2_data.ap_max,0);
        co2_command(0x80, 2, 0x00,0);
      }
      break;
    default:
      debug("unhandled co2 cmd: 0x%02X\n", data[0]);
      break;
  }

  if (unit_set_data(CO2, &co2_data) <= 0)
  {
    error("%s: writing co2 data\n", __FUNCTION__);
  }
}

void co2_undef_state(void)
{
  char s[200];

  alarm_clr(CO2_RISKETCO2);
  alarm_clr(CO2_RISKBRCO2);
  alarm_clr(CO2_RISKICO2);
  alarm_clr(CO2_APNOE);
 // alarm_clr_fixed(CO2_APNOE); // TODO: may be need only alarm_clr

  ids2string(IDS_UNDEF3, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BRSTATE, s);

  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR, (int)UNDEF_VALUE);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2, (int)UNDEF_VALUE);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2, (int)UNDEF_VALUE);
}



