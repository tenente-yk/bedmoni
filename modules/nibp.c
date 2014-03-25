/*! \file nibp.c
    \brief NiBP module interface
 */

#include <string.h>
#ifdef WIN32
//#include <windows.h>
#endif
#ifdef UNIX
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <sys/resource.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include "mframe.h"
#include "iframe.h"
#include "uframe.h"
#include "patient.h"
#include "sched.h"
#include "port.h"
#include "dproc.h"
#include "tr.h"
#include "ecgset.h"
#include "nibp.h"
#include "nibpcal.h"
#include "crbpio.h"
#include "bedmoni.h"

#ifndef MIN
#define MIN(A,B) (A<B)?A:B
#endif

#ifndef MAX
#define MAX(A,B) (A>B)?A:B
#endif

int nibp_meas_interval[NIBP_NUM_MEAS_INTERVALS] = {0, 1, 2, 5, 10, 15, 30, 60};

static int fd = 0;
static nibp_info_t lni;
static int nibp_activity_cnt = 0;
//static int cuff_pressure_cnt = 0;
static int cuff_pressure_extr_meas_time = 0;
static int cuff_pressure_extr_delta = 0;

static int nibp_test_status = 0;
static unsigned char calibration_stat = 0;
static int service_active = 0;

#ifdef NIBP_LOGGING
static FILE *flog = 0;
#endif

static void nibp_debug(char *fmt, ...);

static void nibp_none_command(void * pArg);
static void nibp_set_initial_inflate(void * pArg);
static void nibp_start_bp(void* pArg);
static void nibp_start_peds_bp(void* pArg);
static void nibp_start_neonate_bp(void* pArg);
static void nibp_abort_bp(void* pArg);
static void nibp_get_cuff_pressure(void *pArg);
static void nibp_get_bp_data(void *pArg);
static void nibp_get_module_data(void *pArg);
static void nibp_get_return_string(void *pArg);
static void nibp_set_sleep_mode(void *pArg);
static void nibp_control_pneumatics(void * pArg);
static void nibp_calibrate_transducer(void * pArg);
static void nibp_get_return_code(void *pArg);
static void nibp_venous_stasis(void *pArg);

static nibp_cmd_func_t nibp_cmd_func[] = 
{
  nibp_none_command,         // NIBP_CMD_NONE
  nibp_set_initial_inflate,  // NIBP_CMD_SET_INITIAL_INFLATE
  nibp_start_bp,             // NIBP_CMD_START_BP
  nibp_start_peds_bp,        // NIBP_CMD_START_PEDS_BP
  nibp_start_neonate_bp,     // NIBP_CMD_START_NEONATE_BP
  nibp_abort_bp,             // NIBP_CMD_ABORT_BP
  nibp_get_cuff_pressure,    // NIBP_CMD_GET_CUFF_PRESSURE
  nibp_get_bp_data,          // NIBP_CMD_GET_BP_DATA
  nibp_get_module_data,      // NIBP_CMD_GET_MODULE_DATA
  nibp_get_return_string,    // NIBP_CMD_GET_RETURN_STRING
  nibp_set_sleep_mode,       // NIBP_CMD_SET_SLEEP_MODE
  nibp_control_pneumatics,   // NIBP_CMD_CONTROL_PNEUMATICS
  nibp_calibrate_transducer, // NIBP_CMD_CALIBRATE_TRANSDUCER
  nibp_get_return_code,      // NIBP_CMD_GET_RETURN_CODE
  nibp_venous_stasis,        // NIBP_CMD_VENOUS_STASIS
};

static struct
{
  unsigned char * sof;
  unsigned char   data[500];
  int             ptr;
  int             rdlen;
} nibp_indata;

static int bp_measurement_is_running = 0;

int nibp_init(char* commport)
{
#ifdef NIBP_DISABLED
  return -1;
#else
  time_t tt;
  struct tm * ptm;
  nibp_data_t nibp_data;
  trts_t trts_cur;

  service_active = 0;
  calibration_stat = 0x0;
  alarm_clr(NIBP_NORESP);

  fd = com_open(commport, BR9600, 0);
  if (fd <= 0)
  {
    alarm_set(NIBP_NORESP);
    error("unable to open port %s on %d\n", commport, BR9600);
    return -1;
  }

  memset(&lni, 0, sizeof(nibp_info_t));

#ifdef NIBP_LOGGING
  flog = fopen("nibp.log", "wt");
#endif

  memset(&nibp_indata, 0, sizeof(nibp_indata));

  unit_get_data(NIBP, &nibp_data);
  time(&tt);
  ptm = localtime(&tt);
  trts_cur.hour = ptm->tm_hour;
  trts_cur.min  = ptm->tm_min;
  trts_cur.day  = ptm->tm_mday;
  trts_cur.mon  = ptm->tm_mon+1;
  trts_cur.year = ptm->tm_year;

  nibp_debug("trts cal: %02d.%02d%04d\n", nibp_data.trts_cal.year, nibp_data.trts_cal.mon, nibp_data.trts_cal.day);
  nibp_debug("trts cur: %02d.%02d%04d\n", trts_cur.year, trts_cur.mon, trts_cur.day);

  calibration_stat = 0x3;
  if (trts_cur.year*365+trts_cur.mon*30+trts_cur.day - nibp_data.trts_cal.year*365+nibp_data.trts_cal.mon*30+nibp_data.trts_cal.day > 365)
  {
    error("nibp calibration is expired\n");
    if (!alarm_isset(NIBP_NORESP))
      alarm_set(NIBP_CALIB_EXPIRED);
    calibration_stat = 0x0;
  }

#ifdef NIBP_CALIB_NO_CHECK
  debug("fake!!! skip nibp calibration validity check\n");
  calibration_stat = 0x3;
  alarm_clr(NIBP_CALIB_EXPIRED);
#endif

  return NIBP_ERR_OK;
#endif // NIBP_DISABLED
}

void nibp_deinit(void)
{
#ifdef NIBP_DISABLED
  return;
#else
  nibp_command(NIBP_CMD_ABORT_BP, NULL);

  if (fd > 0)
    com_close(fd);
  fd = 0;

#ifdef NIBP_LOGGING
  if (flog) fclose(flog);
#endif
#endif // NIBP_DISABLED
}

static void nibp_debug(char *fmt, ...)
#ifdef NIBP_DEBUG
{
  va_list  args;
  char s[200];

  va_start(args, fmt);
  yfprintf(stdout, "nibp: ");
  vsprintf(s, fmt, args);
  yfprintf(stdout, s);
  fflush(stdout);
#ifdef NIBP_LOGGING
  fprintf(flog, "nibp: ");
  vfprintf(flog, fmt, args);
  fflush(flog);
#endif
  va_end( args );
}
#else // !NIBP_DEBUG
{}
#endif

void nibp_command(int ncmd, ...)
{
  int v;
  void * pArg = NULL;
  va_list argptr;

  if (fd <= 0)
  {
   // error("nibp module no init\n");
    return;
  }

  va_start(argptr, ncmd);

  switch(ncmd)
  {
    case NIBP_CMD_SET_INITIAL_INFLATE:
      v = va_arg(argptr, int);
      pArg = &v;
      break;
    case NIBP_CMD_CALIBRATE_TRANSDUCER:
      v = va_arg(argptr, int);
      pArg = &v;
      break;
    case NIBP_CMD_CONTROL_PNEUMATICS:
      v = va_arg(argptr, int);
      pArg = &v;
      break;
  }

  nibp_cmd_func[ncmd](pArg);

  va_end(argptr);
}

static void nibp_none_command(void * pArg)
{

}

static void nibp_process_response(unsigned char *pd, int len)
{
  unsigned char crc;
  int i;
  int v;
  char s[200];
  nibp_data_t nibp_data;
  ecg_data_t ecg_data;
  spo2_data_t spo2_data;

//for (i=0; i<len; i++)
//  printf("%02X ", pd[i]);
//printf("\n");

  if (pd[0] != NIBP_ST_MSTART)
  {
    error("nibp_process_response: pd[0] = 0x%02X\n", pd[0]);
    return;
  }
  if (len != pd[1])
  {
    error("nibp_process_response: len %d != %d\n", pd[1], len);
    return;
  }
  crc = 0;
  for (i=0; i<len-1; i++)
  {
    crc += pd[i];
  }
  crc = 0x100 - crc;
  if (crc != pd[len-1])
  {
    error("nibp_process_response: crc 0x%02X != 0x%02X\n", crc, pd[len-1]);
    return;
  }

//printf("nibp_process_response %d\n", len);
  switch(len)
  {
    case 0x04:
      nibp_debug("%c\n", pd[2]);
//printf("%c\n", pd[2]);
      switch (pd[2])
      {
        case NIBP_RESP_O:
          break;
        case NIBP_RESP_K:
          nibp_test_status ++;
          break;
        case NIBP_RESP_B:
          nibp_debug("module is busy\n");
          break;
        case NIBP_RESP_A:
          nibp_debug("abort measurement\n");
          break;
        default:
          nibp_debug("unknown resp %c (0x%02X)\n", pd[2], pd[2]);
      }
      break;
    case 0x05: // get_cuff_pressure
      if (!nibp_calibration_stat())
      {
       // alarm_set(NIBP_CALIB_EXPIRED);
        break;
      }
      v = (pd[3] << 8) | pd[2];
      lni.cuff = v;
      nibp_debug("cuff pressure = %d\n", lni.cuff);
      unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_CUFF_PRESSURE, (lni.cuff < 0) ? (int)UNDEF_VALUE : (int)lni.cuff);
#if 0
      if (lni.cuff > cuff_pressure_extr_delta)
      {
        cuff_pressure_cnt ++;
        if (cuff_pressure_cnt > cuff_pressure_extr_meas_time / NIBP_UPDATE_CUFFDATA_PERIOD)
        {
          alarm_set(NIBP_RISKMEASTIME);
          cuff_pressure_cnt = 0;
        }
      }
      else
      {
        cuff_pressure_cnt = 0;
        alarm_clr(NIBP_RISKMEASTIME);
      }
#endif
      break;
    case 0x06: // get_return_code
      if (pd[4] == 0x00)
        nibp_test_status += 1;
      else
        nibp_test_status = -1;
      break;
    case 0x18: // get_bp_data
     // lni.data = *((nibp_suntech_data_t*)&pd[2]);
      memcpy(&lni.data, &pd[2], sizeof(nibp_suntech_data_t));

      if (unit_get_data(NIBP, &nibp_data) <= 0)
      {
        error("%s: reading nibp data\n", __FUNCTION__);
      }
      if (unit_get_data(ECG, &ecg_data) <= 0)
      {
        error("%s: reading ecg data\n", __FUNCTION__);
      }
      if (unit_get_data(SPO2, &spo2_data) <= 0)
      {
        error("%s: reading spo2 data\n", __FUNCTION__);
      }

      if (ecg_data.hr_src_curr != NIBP && ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr == UNDEF_VALUE && spo2_data.hr == UNDEF_VALUE)
      {
        unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[NIBP]);
        ecg_data.hr_src_curr = NIBP;
        unit_set_data(ECG, &ecg_data);
      }

      if (ecg_data.hr_src == HRSRC_NIBP || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == NIBP))
      {
        nibp_data.hr = (lni.data.rate != 0) ? lni.data.rate : UNDEF_VALUE;
        unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, nibp_data.hr);
        unit_set_data(NIBP, &nibp_data);
      }

      if (lni.data.bps)
      {
        bp_measurement_is_running = 0;
        nibp_debug("bp_data = %d / %d\n", lni.data.sss, lni.data.ddd);

        if (nibp_data.meas_interval != NIBP_MEAS_INTERVAL_MANU)
        {
          sched_start(SCHED_NIBP, nibp_meas_interval[nibp_data.meas_interval]*60*1000, nibp_do_bp, SCHED_NORMAL);
        }

        if (lni.data.sss != 0 && lni.data.ddd != 0)
        {
          time_t tt;
          struct tm * ptm;
          nibpv_t nv;
          int v;
          unsigned char st;

          if (lni.data.sss < nibp_data.sd_min || lni.data.sss > nibp_data.sd_max)
          {
            alarm_set(NIBP_RISKSS);
          }
          else
          {
            alarm_clr(NIBP_RISKSS);
          }

          if (lni.data.ddd < nibp_data.dd_min || lni.data.ddd > nibp_data.dd_max)
          {
            alarm_set(NIBP_RISKDD);
          }
          else
          {
            alarm_clr(NIBP_RISKDD);
          }

          if (lni.data.map < nibp_data.md_min || lni.data.map > nibp_data.md_max)
          {
            alarm_set(NIBP_RISKCC);
          }
          else
          {
            alarm_clr(NIBP_RISKCC);
          }

          nibp_data.sd = lni.data.sss;
          nibp_data.dd = lni.data.ddd;
          nibp_data.md = lni.data.map;
          unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_SS, lni.data.sss);
          unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_DD, lni.data.ddd);
          unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_CC, lni.data.map);
         // unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_PULSE, lni.data.rate);

          time(&tt);
          ptm = localtime(&tt);
          nibp_data.trts_meas.hour = ptm->tm_hour;
          nibp_data.trts_meas.min  = ptm->tm_min;
          nibp_data.trts_meas.day  = ptm->tm_mday;
          nibp_data.trts_meas.mon  = ptm->tm_mon+1;
          nibp_data.trts_meas.year = ptm->tm_year;

          // no errors, clear alarms
          alarm_clr(NIBP_RISKMEASTIME);
          alarm_clr(NIBP_WEAKSIGNAL);
          alarm_clr(NIBP_ERRSIGNAL);
          alarm_clr(NIBP_EXCRETRYCOUNT);
          alarm_clr(NIBP_PNEUMBLOCK);
          alarm_clr(NIBP_INFLTIMEOUT);
          alarm_clr(NIBP_RISKINFL);

          nibp_data.trts_meas.hour = ptm->tm_hour;
          nibp_data.trts_meas.min  = ptm->tm_min;
          nibp_data.trts_meas.day  = ptm->tm_mday;
          nibp_data.trts_meas.mon  = ptm->tm_mon+1;
          nibp_data.trts_meas.year = ptm->tm_year;

          nibp_data.hwerr_ack = 0;

          if (unit_set_data(NIBP, &nibp_data) <= 0)
          {
            error("%s: writing nibp data\n", __FUNCTION__);
          }

          if (unit_set_data(ECG, &ecg_data) <= 0)
          {
            error("%s: writing ecg data\n", __FUNCTION__);
          }

          sprintf(s, "%02d:%02d %02d.%02d.%04d", ptm->tm_hour, ptm->tm_min, ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900);
          unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_STIME, s);

          st = 0;
          st |= (lni.data.sss > nibp_data.sd || lni.data.ddd > nibp_data.dd || lni.data.map > nibp_data.md) ? TR_ST_U : 0;
          st |= (lni.data.sss < nibp_data.sd || lni.data.ddd < nibp_data.dd || lni.data.map < nibp_data.md) ? TR_ST_L : 0;

          nv.ss = lni.data.sss;
          nv.dd = lni.data.ddd;
          nv.mm = lni.data.map;
          memcpy(&v, &nv, 4);
          dproc_add_trv(NIBP, PARAM_NIBP, v, st);
        }
      }
      else
      {
        nibp_data_t nibp_data;
        unit_get_data(NIBP, &nibp_data);
        if (/*lni.data.ec > 0 && */lni.data.ec != nibp_data.ec)
        {
          if (lni.data.ec > 0)
          {
            nibp_debug("exited with error code %d\n", lni.data.ec);

#if 0
            nibp_data.sd = UNDEF_VALUE;
            nibp_data.dd = UNDEF_VALUE;
            nibp_data.md = UNDEF_VALUE;
            nibp_data.hr = UNDEF_VALUE;
            unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_SS, UNDEF_VALUE);
            unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_DD, UNDEF_VALUE);
            unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_CC, UNDEF_VALUE);
            unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_PULSE, UNDEF_VALUE);
#endif
          }

          switch (lni.data.ec)
          {
            case 0:
              // no errors
              alarm_clr(NIBP_RISKMEASTIME);
              alarm_clr(NIBP_WEAKSIGNAL);
              alarm_clr(NIBP_ERRSIGNAL);
              alarm_clr(NIBP_EXCRETRYCOUNT);
              alarm_clr(NIBP_PNEUMBLOCK);
              alarm_clr(NIBP_INFLTIMEOUT);
              alarm_clr(NIBP_INVPOWER);
              alarm_clr(NIBP_RISKINFL);
              break;
            case 1:
              alarm_set(NIBP_WEAKSIGNAL);
              break;
            case 2:
              alarm_set(NIBP_ERRSIGNAL);
              break;
            case 3:
              alarm_set(NIBP_EXCRETRYCOUNT);
              break;
            case 4:
              alarm_set(NIBP_RISKMEASTIME);
              break;
            case 85:
              alarm_set(NIBP_PNEUMBLOCK);
              break;
            case 87:
              alarm_set(NIBP_INFLTIMEOUT);
              break;
            case 89:
              if (!nibp_data.hwerr_ack)
              {
                alarm_set(NIBP_RISKINFL);
                alarm_clr(NIBP_RISKINFL);
              }
              break;
            case 90:
              alarm_set(NIBP_INVPOWER);
              break;
          }
          nibp_data.hwerr_ack = 1;
          nibp_data.ec = lni.data.ec;
          unit_set_data(NIBP, &nibp_data);
        }
      }
#if 0
      alarm_clr(NIBP_RISKINFL);
#endif
      break;
    case 0x43:
     // lni.fw_info = *((nibp_fwinfo_t*)&pd[2]);
      memcpy(&lni.fw_info, &pd[2], sizeof(nibp_fwinfo_t));
      nibp_debug("bp_mfw: %s\n", lni.fw_info.bp_fw);
      nibp_debug("s_fw: %s\n", lni.fw_info.s_fw);
      // nibp_debug("pr_code: %s\n", lni.fw_info.pr_code);
      // nibp_debug("mod_id: %s\n", lni.fw_info.mod_id);
      nibp_debug("cyc: %s\n", lni.fw_info.cyc);
      break;
    default:
      error("nibp_process_response: inv len %d\n", len);
      break;
  }
}

void nibp_process_input(void)
{
  int nr;
  int i;
  int packets_in_queue;

 // nibp_debug("nibp_process_input\n");

 // do
  {
    nr = com_read(fd, (char*)&nibp_indata.data[nibp_indata.ptr], 500-nibp_indata.ptr);
//printf("nr = %d\n", nr);
//if (nr < 0) printf("%s\n", strerror(errno));
//printf("----------------------------\n");
//for (i=0; i<nr; i++)
//printf("%02X ", nibp_indata.data[nibp_indata.ptr + i]);
//printf("\n");
    if (nr > 0)
    {
      nibp_indata.ptr += nr;
      nibp_activity_cnt = 0;
    }
  }// while (nr > 0 && nibp_indata.ptr < 500);

  do
  {
    packets_in_queue = 0;
    if (nibp_indata.sof == NULL)
    {
      for (i=0; i<nibp_indata.ptr; i++)
      {
        if (nibp_indata.data[i] == NIBP_ST_MSTART)
        {
          nibp_indata.sof = &nibp_indata.data[i];
          break;
        }
      }
    }
    if (nibp_indata.sof != NULL)
    {
      if ( (nibp_indata.rdlen == 0) && (nibp_indata.data+nibp_indata.ptr - nibp_indata.sof > 1) )
      {
       nibp_indata.rdlen = *(nibp_indata.sof+1);
     }
//printf("sof %d %d\n", nibp_indata.rdlen, nibp_indata.data+nibp_indata.ptr - nibp_indata.sof);
      if (nibp_indata.rdlen != 0 && nibp_indata.data+nibp_indata.ptr - nibp_indata.sof >= nibp_indata.rdlen)
      {
        nibp_process_response(nibp_indata.sof, nibp_indata.rdlen);
        nibp_indata.ptr -= (nibp_indata.rdlen+nibp_indata.sof-&nibp_indata.data[0]);
        memcpy(nibp_indata.data, nibp_indata.sof+nibp_indata.rdlen, nibp_indata.ptr);
        nibp_indata.rdlen = 0;
        nibp_indata.sof = (nibp_indata.data[0] == NIBP_ST_MSTART) ? &nibp_indata.data[0] : NULL;
        packets_in_queue = 1;
        nibp_activity_cnt = 0;
      }
    }
  } while (packets_in_queue);

  nibp_activity_cnt += 1;

 // debug("nibp_activity_cnt = %d\n", nibp_activity_cnt);

  //if (nibp_activity_cnt * HP_UPD_DELAY_MS >= NIBP_ACTIVITY_TIMEOUT)
  if (nibp_activity_cnt >= NIBP_INACTIVITY_COUNT_LIMIT && service_active == 0)
  {
    //if (!alarm_isset(NIBP_NORESP)) {}
    alarm_set(NIBP_NORESP);
#ifdef NIBP_CALIB_NO_CHECK
    alarm_clr(NIBP_CALIB_EXPIRED);
#endif
    unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_CUFF_PRESSURE, (int)UNDEF_VALUE);
    if (sched_is_started(SCHED_NIBP))
      nibp_debug("nibp is not responding\n");
    nibp_activity_cnt -= 1;
  }
  else
  {
    alarm_clr(NIBP_NORESP);
  }

 //printf("->%d\n", bp_measurement_is_running);
}

static void nibp_set_initial_inflate(void * pArg)
{
  unsigned char d[5];
  int i;
  int mmHg = 150;

  if (pArg)
    mmHg = *((int*)pArg);
  else
    error("nibp_set_initial_inflate: pArg==NULL, use default init inflate %d\n", mmHg);

  nibp_debug("nibp_set_initial_inflate with %d mmHg\n", mmHg);

  if (fd <= 0) return;
  d[0] = NIBP_ST_HSTART;
  d[1] = 0x17;
  d[2] = (mmHg>>0) & 0xFF;
  d[3] = (mmHg>>8) & 0xFF;
  d[4] = 0;
  for (i=0; i<4; i++)
    d[4] += d[i];
  d[4] = 0x100-d[4];
  com_write(fd, (char*)d, 5);
}

static void nibp_get_cuff_pressure(void *pArg)
{
  unsigned char d[5] = {NIBP_ST_HSTART, 0x79, 0x05, 0x00, 0x48};

  //nibp_debug("nibp_get_cuff_pressure\n");

  if (fd <= 0) return;

  com_write(fd, (char*)d, 5);
}

static void nibp_start_bp(void* pArg)
{
  unsigned char d[3] = {NIBP_ST_HSTART, 0x20, 0xA6};

  nibp_debug("nibp_start_bp\n");

  if (fd <= 0) return;
  com_write(fd, (char*)d, 3);

  bp_measurement_is_running = 1;
}

static void nibp_start_peds_bp(void * pArg)
{
  unsigned char d[3] = {NIBP_ST_HSTART, 0x87, 0x3F};

  nibp_debug("nibp_start_peds_bp\n");

  if (fd <= 0) return;
  com_write(fd, (char*)d, 3);

  bp_measurement_is_running = 1;
}

static void nibp_start_neonate_bp(void * pArg)
{
  unsigned char d[3] = {NIBP_ST_HSTART, 0x28, 0x9E};

  nibp_debug("nibp_start_neonate_bp\n");

  if (fd <= 0) return;
  com_write(fd, (char*)d, 3);

  bp_measurement_is_running = 1;
}

static void nibp_abort_bp(void * pArg)
{
  unsigned char d[5] = {NIBP_ST_HSTART, 0x79, 0x01, 0x00, 0x4C};

  nibp_debug("nibp_abort_bp\n");

  if (fd <= 0) return;
  com_write(fd, (char*)d, 5);

  bp_measurement_is_running = 0;
}

static void nibp_get_bp_data(void *pArg)
{
  unsigned char d[5] = {NIBP_ST_HSTART, 0x79, 0x03, 0x00, 0x4A};

  nibp_debug("nibp_get_bp_data\n");

  if (fd <= 0) return;
  com_write(fd, (char*)d, 5);
}

static void nibp_get_module_data(void * pArg)
{
  unsigned char d[3] = {NIBP_ST_HSTART, 0x00, 0xC6};

  nibp_debug("nibp_get_module_data\n");

  if (fd <= 0) return;
  com_write(fd, (char*)d, 3);
}

static void nibp_get_return_string(void * pArg)
{
  unsigned char d[5] = {NIBP_ST_HSTART, 0x79, 0x02, 0x40, 0x0B};

  nibp_debug("nibp_get_return_string\n");

  if (fd <= 0) return;

  nibp_get_module_data(NULL);

  com_write(fd, (char*)d, 5);
}

static void nibp_set_sleep_mode(void * pArg)
{
  int mode;
  unsigned char d[7];

  nibp_debug("nibp_set_sleep_mode\n");

  mode = *((int*)pArg);

  if (fd <= 0) return;
  switch(mode)
  {
    case IMMEDIATE:
      d[2] = 0x53;
      d[3] = 0x54;
      d[4] = 0x4F;
      d[5] = 0x50;
      d[6] = 0xFF;
      break;
    case AUTOSLEEP:
      d[2] = d[3] = d[4] = 0x00;
      d[5] = 0x01;
      d[6] = 0x44;
      break;
    case NO_AUTOSLEEP:
      d[2] = d[3] = d[4] = d[5] = 0x00;
      d[6] = 0x45;
      break;
    default:
      nibp_debug("nibp_set_sleep_mode: invalid mode\n");
      return;
  }
  d[0] = NIBP_ST_HSTART;
  d[1] = 0x81;

  com_write(fd, (char*)d, 7);

#if 0
  res = nibp_waitfor_resp(NIBP_RESP_O);
  if (res != NIBP_ERR_OK) return res;

  if (mode == AUTOSLEEP || mode == NO_AUTOSLEEP)
  {
    res = nibp_waitfor_resp(NIBP_RESP_K);
    if (res != NIBP_ERR_OK) return res;
  }
  if (mode == IMMEDIATE)
  {
    res = nibp_waitfor_resp(NIBP_RESP_S);
    if (res != NIBP_ERR_OK) return res;
  }
#endif
}

static void nibp_control_pneumatics(void * pArg)
{
  char pump, control_valve, dump_valve;
  unsigned int v;
  unsigned char d[6];
  int i;

  v = *((int*)pArg);
  pump = (char)((v >> 0) & 0xff);
  control_valve = (char)((v >> 8) & 0xff);
  dump_valve = (char)((v >> 16) & 0xff);

  nibp_debug("nibp_control_pneumatics\n");

  if (fd <= 0) return;
  d[0] = NIBP_ST_HSTART;
  d[1] = 0x0C;
  d[2] = (pump == OFF) ? 0x00 : 0x01;
  d[3] = (control_valve == CLOSE) ? 0x01 : 0x00;
  d[4] = (dump_valve == CLOSE) ? 0x01 : 0x00;
  d[5] = 0;
  for (i=0; i<5; i++)
    d[5] += d[i];
  d[5] = 0x100-d[5];

  com_write(fd, (char*)d, 6);

#if 0
  res = nibp_waitfor_resp(NIBP_RESP_O);
  if (res != NIBP_ERR_OK) return res;

  res = nibp_waitfor_resp(NIBP_RESP_K);
  if (res != NIBP_ERR_OK) return res;
#endif
}

static void nibp_calibrate_transducer(void * pArg)
{
  unsigned char d[4];
  unsigned int v;
  unsigned char b;
  int i;

  nibp_debug("%s\n", __FUNCTION__);

  if (fd <= 0) return;

#if 0
  v = (OFF<<0) | (CLOSE<<8) | (CLOSE<<16);
  nibp_control_pneumatics(&v);
#endif

  v = *((int*)pArg);
  if (v == 0)
    b = 0x00;
  else
  if (v == 250)
    b = 0x01;
  else
  {
    nibp_debug("%s: invalid calib pressure %d\n", __FUNCTION__, v);
    return;
  }
  nibp_debug("%s: %d mm Hg\n", __FUNCTION__, v);

  d[0] = NIBP_ST_HSTART;
  d[1] = 0x04;
  d[2] = b;

  d[3] = 0;
  for (i=0; i<3; i++)
    d[3] += d[i];
  d[3] = 0x100-d[3];

  assert (com_write(fd, (char*)d, 4) == 4);
}

static void nibp_get_return_code(void *pArg)
{
  unsigned char d[5] = {NIBP_ST_HSTART, 0x79, 0x02, 0x03, 0x48};

  nibp_debug("nibp_get_return_code\n");

  if (fd <= 0) return;

  com_write(fd, (char*)d, 5);
}

static void nibp_venous_stasis(void * pArg)
{
  unsigned char d[6];
  int i;
  unsigned int v;
  unsigned char pressure, dt, mode;

  nibp_debug("nibp_venous_stasis\n");

  if (fd <= 0) return;

  v = *((int*)pArg);
  pressure = (unsigned char)((v>>0) & 0xff);
  dt = (unsigned char)((v>>8) & 0xff);
  mode = (unsigned char)((v>>16) & 0xff);

  d[0] = NIBP_ST_HSTART;
  d[1] = 0x86;
  d[2] = pressure;
  d[3] = dt;
  d[4] = (mode == ADULT) ? 0x00 : 0x01;
  if (d[4] == 0x00) // adult
  {
    d[2] = MAX(30, d[2]);
    d[2] = MIN(140, d[2]);
    d[3] = MAX(10, d[3]);
    d[3] = MIN(20, d[3]);
  }
  else              // neonate
  {
    d[2] = MAX(10, d[2]);
    d[2] = MIN(100, d[2]);
    d[3] = MAX(1, d[3]);
    d[3] = MIN(70, d[3]);
  }
  d[5] = 0;
  for (i=0; i<5; i++)
    d[5] += d[i];
  d[5] = 0x100-d[5];

  com_write(fd, (char*)d, 6);

#if 0
  res = nibp_waitfor_resp(NIBP_RESP_O);
  if (res != NIBP_ERR_OK) return res;

  res = nibp_waitfor_resp(NIBP_RESP_K);
  if (res != NIBP_ERR_OK) return res;
#endif
}

void nibp_get_last_data(nibp_info_t *pni)
{
  if (pni) *pni = lni;
}

int nibp_is_running(void)
{
  return (bp_measurement_is_running);
}

void nibp_reqdata(void)
{
  static unsigned long cnt = 0;
  int n;

 // if (!nibp_calibration_stat()) return;

  nibp_command(NIBP_CMD_GET_CUFF_PRESSURE); // get_cuff_pressure

  n = (NIBP_UPDATE_DATA_PERIOD / NIBP_UPDATE_CUFFDATA_PERIOD) + 1;
  if (cnt % n == 0)
  {
    nibp_command(NIBP_CMD_GET_BP_DATA);
  }
  cnt ++;
}

void nibp_do_bp(void)
{
  nibp_data_t nibp_data;
  pat_pars_t lpp;
  int nibp_cmd;

  if (!nibp_calibration_stat() && !alarm_isset(NIBP_NORESP))
  {
    alarm_set(NIBP_CALIB_EXPIRED);
    return;
  }

  if (unit_get_data(NIBP, &nibp_data) <= 0)
  {
    error("%s error reading nibp data\n", __FUNCTION__);
  }

  nibp_data.ec = 0;

  nibp_command(NIBP_CMD_SET_INITIAL_INFLATE, nibp_data.infl);

  pat_get(&lpp);

  switch(lpp.type)
  {
    case ADULT:
      nibp_cmd = NIBP_CMD_START_BP;
      cuff_pressure_extr_delta = 15;
      cuff_pressure_extr_meas_time = 130;
      break;
    case CHILD:
      nibp_cmd = NIBP_CMD_START_PEDS_BP;
      cuff_pressure_extr_delta = 15;
      cuff_pressure_extr_meas_time = 130;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      nibp_cmd = NIBP_CMD_START_NEONATE_BP;
      cuff_pressure_extr_delta = 5;
      cuff_pressure_extr_meas_time = 65;
      break;
#endif
    default:
      nibp_cmd = NIBP_CMD_NONE;
      error("%s: unexpected type of pat %d\n", __FUNCTION__, lpp.type);
      break;
  }

  unit_set_data(NIBP, &nibp_data);

  nibp_command(nibp_cmd);
}

static int calib_pressure = 0;
static struct
{
  unsigned long  t0;
  unsigned long  t1;
  unsigned long  t2;
  unsigned short p1;
  unsigned short p2;
} leaktst_data;

static void nibp_calib_stepit(void)
{
  unsigned int v;

//printf("nibp_test_status = %d\n", nibp_test_status);

  switch (nibp_test_status)
  {
    case -1:
     // error("calibration failed\n");
      if (calib_pressure == 0)
        uframe_printbox(155, 40, -1, -1, "X", RGB(0xff,0x00,0x00));
      if (calib_pressure == 250)
        uframe_printbox(155, 70, -1, -1, "X", RGB(0xff,0x00,0x00));
      sched_stop(SCHED_NIBP);
      break;
    case 0:
      uframe_clearbox(155, 40, 40, 20);
      uframe_clearbox(155, 70, 40, 20);
      v = (OFF<<0) | (CLOSE<<8) | (CLOSE<<16);
      nibp_control_pneumatics(&v);
      calib_pressure = -1;
     // nibp_test_status ++;
      break;
    case 1:
      if (calib_pressure != 0)
      {
        inpfoc_enable(INPFOC_NIBPCAL, NIBPCAL_SET0);
        inpfoc_set(INPFOC_NIBPCAL, NIBPCAL_SET0);
      }
      calib_pressure = 0;
      break;
    case 2:
      unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_CUFF_PRESSURE, UNDEF_VALUE);
      nibp_calibrate_transducer(&calib_pressure);
      break;
    case 3:
      nibp_get_return_code(NULL);
      break;
    case 4:
      if (calib_pressure == 0)
      {
        uframe_printbox(155, 40, -1, -1, "OK", RGB(0x00,0xff,0x00));
        calibration_stat |= (1 << 0);
        if (calibration_stat == 0x3)
        {
          time_t tt;
          struct tm * ptm;
          nibp_data_t nibp_data;
          char s[200];

          unit_get_data(NIBP, &nibp_data);
          time(&tt);
          ptm = localtime(&tt);
          nibp_data.trts_cal.hour = ptm->tm_hour;
          nibp_data.trts_cal.min  = ptm->tm_min;
          nibp_data.trts_cal.day  = ptm->tm_mday;
          nibp_data.trts_cal.mon  = ptm->tm_mon+1;
          nibp_data.trts_cal.year = ptm->tm_year;
          unit_set_data(NIBP, &nibp_data);
          alarm_clr(NIBP_CALIB_EXPIRED);

          sprintf(s, "%02d.%02d.%04d", nibp_data.trts_cal.day, nibp_data.trts_cal.mon, nibp_data.trts_cal.year+1900);
          uframe_printbox(20, 437, -1, -1, s, UFRAME_STATICTEXT_COLOR);

          inpfoc_set(INPFOC_NIBPCAL, NIBPCAL_SET250);
         // ids2string(IDS_CALIB_FINISHED, s);
         // uframe_printbox(10, 130, -1, -1, s, UFRAME_STATICTEXT_COLOR);
        }
      }
      nibp_test_status ++;
      break;
    case 5:
      calib_pressure = 250;
      break;
    case 6:
      unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_CUFF_PRESSURE, UNDEF_VALUE);
      nibp_calibrate_transducer(&calib_pressure);
      break;
    case 7:
      nibp_get_return_code(NULL);
      break;
    case 8:
      if (calib_pressure == 250)
      {
        uframe_printbox(155, 70, -1, -1, "OK", RGB(0x00,0xff,0x00));
        calibration_stat |= (1 << 1);
        if (calibration_stat == 0x3)
        {
          time_t tt;
          struct tm * ptm;
          nibp_data_t nibp_data;
          char s[200];

          unit_get_data(NIBP, &nibp_data);
          time(&tt);
          ptm = localtime(&tt);
          nibp_data.trts_cal.hour = ptm->tm_hour;
          nibp_data.trts_cal.min  = ptm->tm_min;
          nibp_data.trts_cal.day  = ptm->tm_mday;
          nibp_data.trts_cal.mon  = ptm->tm_mon+1;
          nibp_data.trts_cal.year = ptm->tm_year;
          unit_set_data(NIBP, &nibp_data);
          alarm_clr(NIBP_CALIB_EXPIRED);

          sprintf(s, "%02d.%02d.%04d", nibp_data.trts_cal.day, nibp_data.trts_cal.mon, nibp_data.trts_cal.year+1900);
          uframe_clearbox(20, 437, 140, 20);
          uframe_printbox(20, 437, -1, -1, s, UFRAME_STATICTEXT_COLOR);

          inpfoc_set(INPFOC_NIBPCAL, NIBPCAL_EXIT);
         // ids2string(IDS_CALIB_FINISHED, s);
         // uframe_printbox(10, 100, -1, -1, s, UFRAME_STATICTEXT_COLOR);
        }
      }
      v = (ON<<0) | (OPEN<<8) | (OPEN<<16);
      nibp_control_pneumatics(&v);
      sched_stop(SCHED_NIBP);
      break;
    default:
      error("%s: invalid case %d\n", nibp_test_status);
      sched_stop(SCHED_NIBP);
      break;
  }
}

static void nibp_leak_stepit(void)
{
  unsigned int v;
  unsigned long t;
  nibp_info_t ni;
  static int cnt = 0;
  static int y = 150;
  char s[200];

  switch (nibp_test_status)
  {
    case -1:
      error("leak test failed\n");
     // nibpset_leak_done(FALSE);
      sched_stop(SCHED_NIBP);
      break;
    case 0:
      v = (OFF<<0) | (CLOSE<<8) | (CLOSE<<16);
      nibp_control_pneumatics(&v);
      break;
    case 1:
      debug("wait for start leak test\n");
      y = 150;
      uframe_clearbox(10, y, 190, 60);
      break;
    case 2:
     // debug("leak test in progress\n");
      memset(s, '.', cnt % 5);
      s[cnt % 5] = 0;
      cnt ++;
      uframe_clearbox(160, y, 40, 20);
      uframe_printbox(160, y, -1, -1, s, UFRAME_STATICTEXT_COLOR);
      if (leaktst_data.t0 == 0)
      {
        ids2string(IDS_STAGE, s);
        strcat(s, " 1/2");
        uframe_printbox(10, y, -1, -1, s, UFRAME_STATICTEXT_COLOR);
        leaktst_data.t0 = gettimemillis();
      }
      else
      {
        t = gettimemillis();
        if (t - leaktst_data.t0 > 60*1000)
        {
          if (leaktst_data.t1 == 0)
          {
            leaktst_data.t1 = t;
            nibp_get_last_data(&ni);
            leaktst_data.p1 = ni.cuff;
            debug("leak test: p1 = %d\n", leaktst_data.p1);
            uframe_clearbox(160, y, 40, 20);
            uframe_printbox(160, y, -1, -1, "OK", RGB(0x00,0xff,0x00));
            ids2string(IDS_STAGE, s);
            strcat(s, " 2/2");
            y += 20;
            uframe_printbox(10, y, -1, -1, s, UFRAME_STATICTEXT_COLOR);
          }
          else
          if (t - leaktst_data.t1 > 60*1000)
          {
            nibp_get_last_data(&ni);
            leaktst_data.p2 = ni.cuff;
            debug("leak test: p2 = %d\n", leaktst_data.p2);
            uframe_clearbox(160, y, 40, 20);
            uframe_printbox(160, y, -1, -1, "OK", RGB(0x00,0xff,0x00));
            y += 20;
            nibp_test_status ++; // finish leak test
          }
        }
      }
      break;
    case 3:
      debug("leak test: p1 = %d, p2 = %d\n", leaktst_data.p1, leaktst_data.p2);
      debug("leak test finished\n", nibp_test_status);
      if (leaktst_data.p1 - leaktst_data.p2 < 6)
      {
        ids2string(IDS_LEAKTEST_FINISHED_OK, s);
        uframe_printbox(10, y, -1, -1, s, UFRAME_STATICTEXT_COLOR);
        uframe_printbox(175, y, -1, -1, "OK", RGB(0x00,0xff,0x00));
      }
      else
      {
        ids2string(IDS_LEAKTEST_FINISHED_FAIL, s);
        uframe_printbox(10, y, -1, -1, s, UFRAME_STATICTEXT_COLOR);
        uframe_printbox(175, y, -1, -1, "X", RGB(0xff,0x00,0x00));
      }
      nibp_test_status ++; // perform next case: finishing
      v = (ON<<0) | (OPEN<<8) | (OPEN<<16);
      nibp_control_pneumatics(&v);
      break;
    default:
     // debug("nibp_test_status = %d\n", nibp_test_status);
      sched_stop(SCHED_NIBP);
      break;
  }
}

static void nibp_verif_stepit(void)
{
  unsigned int v;
  nibp_info_t ni;
  char s[200];

  switch (nibp_test_status)
  {
    case -1:
      error("verif test failed\n");
      sched_stop(SCHED_NIBP);
      break;
    case 0:
      v = (OFF<<0) | (CLOSE<<8) | (CLOSE<<16);
      nibp_control_pneumatics(&v);
      break;
    case 1:
      debug("wait for start verif test\n");
      unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_CUFF_PRESSURE, UNDEF_VALUE);
      break;
    case 2:
      nibp_get_last_data(&ni);
      nibp_debug("cuff pressure = %d\n", ni.cuff);
      if (nibp_calibration_stat())
        sprintf(s, "%d\n", ni.cuff);
      else
        sprintf(s, "--\n");
      uframe_clearbox(170, 170, 40, 20);
      uframe_printbox(170, 170, -1, -1, s, UFRAME_STATICTEXT_COLOR);
      // update window in uframe
      if (1)
      {
        static int cnt = 0;
        cnt ++;
        if (cnt == 120) // we have to finish before Suntech SW abort)
        {
          debug("verification finished\n");
          nibp_test_status++;
        }
      }
      break;
    case 3:
      ids2string(IDS_VERIF_FINISHED, s);
      uframe_clearbox(160, 150, 40, 20);
      uframe_printbox(20, 170, -1, -1, s, UFRAME_STATICTEXT_COLOR);
      v = (ON<<0) | (OPEN<<8) | (OPEN<<16);
      nibp_control_pneumatics(&v);
      sched_stop(SCHED_NIBP);
      break;
    default:
     // debug("nibp_test_status = %d\n", nibp_test_status);
      sched_stop(SCHED_NIBP);
      break;
  }
}

int  nibp_get_status(void)
{
  return (nibp_test_status);
}

void nibp_calib(void)
{
  sched_stop(SCHED_NIBP);
  sched_stop(SCHED_NIBP_REQDATA);

  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_MEAS_INTERVAL, IDS_STOPPED);

  nibp_test_status = 0;
  sched_start(SCHED_NIBP, 1000, nibp_calib_stepit, SCHED_NORMAL);
}

void nibp_leak_test(void)
{
  sched_stop(SCHED_NIBP);
 // sched_stop(SCHED_NIBP_REQDATA);

  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_MEAS_INTERVAL, IDS_STOPPED);

  memset(&leaktst_data, 0, sizeof(leaktst_data));
  nibp_test_status = 0;
  sched_start(SCHED_NIBP, 1000, nibp_leak_stepit, SCHED_NORMAL);
}

void nibp_verif(void)
{
  sched_stop(SCHED_NIBP);
  sched_stop(SCHED_NIBP_REQDATA);

  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_MEAS_INTERVAL, IDS_STOPPED);

  nibp_test_status = 0;
  sched_start(SCHED_NIBP, 1000, nibp_verif_stepit, SCHED_NORMAL);
}

void nibp_reset(void)
{
  debug("nibp_reset\n");
  crbpio_send_msg(CRBP_SET_POWER, DEV_MASK_NIBP, 0x00, 0x00, 0x00); // nibp off
  crbpio_send_msg(CRBP_SET_POWER, DEV_MASK_NIBP, 0x00, 0x01, 0x00); // nibp on
}

int nibp_calibration_stat(void)
{
 // return 1;
  return ((calibration_stat & 0x3) == 0x3) ? 1 : 0;
}

void nibp_service(int on_off)
{
  service_active = on_off;
}

