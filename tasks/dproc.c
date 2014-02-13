/*! \file dproc.c
    \brief Process data functionality
*/

#ifdef UNIX
#include <unistd.h>
#include <pthread.h>
#endif
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "bedmoni.h"
#include "dview.h"
#include "dio.h"
#include "dproc.h"
#include "filter.h"
#include "ecgcalc.h"
#include "resp.h"
#if defined (RESPCALC_COLIBRI)
#include "respcalc.h"
#endif
#include "ecgm.h"
#include "alarm.h"
#include "unit.h"
#include "cframe.h"
#include "mframe.h"
#include "tframe.h"
#include "lframe.h"
#include "sframe.h"
#include "sound.h"
#include "crbpio.h"
#include "nibp.h"
#include "t1t2.h"
#include "ep.h"
#include "ecgcalc.h"
#include "arrh.h"
#include "utils.h"
#include "powerman.h"
#include "tr.h"
#include "csio.h"

#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "ecgset.h"

int filt_resp_id;

static struct
{
  tr_t tr;
  int  n[NUM_PARAMS];
} trd;

#ifdef UNIX
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

int dproc_init(void)
{
  int i;

  sbdp_version[0] = 0;

  if ( tr_open() <= 0)
  {
    error("tr_open failed\n");
  }

  memset(&trd, 0, sizeof(trd));
  for (i=0; i<NUM_PARAMS; i++)
    trd.tr.vl[i] = 0x7fff;

  filt_resp_id = resp_filt_init(1.2f);

#if defined (RESPCALC_COLIBRI)
  respcalc_init();
#endif

  return 0;
}

void dproc_deinit(void)
{
#if defined (RESPCALC_COLIBRI)
  respcalc_deinit();
#endif

  resp_filt_deinit(filt_resp_id);

  tr_close();

  debug("%s\n", __FUNCTION__);
}

static int conn_cnt[NUM_VIEWS] = {0,0,0,0,0,0,0,0,0,0,0};
static void dproc_conn_ok(unsigned int id)
{
  int chno = -1;
  switch (id)
  {
    case PD_ID_ECG:
      chno = ECG;
      break;
    case PD_ID_T1T2:
      chno = T1T2;
      break;
    case PD_ID_SPO2:
      chno = SPO2;
      break;
    case PD_ID_CO2:
      chno = CO2;
      break;
    default:
      return;
  }

  if (chno < 0 || chno >= NUM_VIEWS) return;
  conn_cnt[chno] = 0;
}

static void dproc_check_conns(void)
{
  int i;

  for (i=0; i<NUM_VIEWS; i++) conn_cnt[i] = conn_cnt[i]+1;

  // ECG
  if (conn_cnt[ECG] > (1500 / NP_UPD_DELAY_MS))
  {
    if (unit_isenabled(ECG))
    {
      error("reset ECG\n");
      conn_cnt[ECG] = 20;
      dio_module_cmd(PD_ID_ECG, SBDP_RST_MOD);

      // set startup info for further restore after reset
      startup_info_t si;
      ecg_data_t ecg_data;
      memset(&si, 0, sizeof(startup_info_t));
      unit_get_data(ECG, &ecg_data);
      si.ecg_set = ecg_data.set;
      mframe_command(MFRAME_SET_STARTUP_CFG, (void*)&si);

      mframe_command(MFRAME_STARTUP, NULL);

#if defined (RESPCALC_COLIBRI)
      respcalc_reset();
#endif

      alarm_set(ECG_NOCONNECT);
      alarm_clr(ECG_RBREAK);
      alarm_clr(ECG_LBREAK);
      alarm_clr(ECG_FBREAK);
      alarm_clr(ECG_CBREAK);
      alarm_clr(RESP_BADCONTACT_RL);
      alarm_clr(RESP_BADCONTACT_RF);
    }
  }
  else
  {
    if (conn_cnt[ECG] < 20) alarm_clr(ECG_NOCONNECT);
  }
  // SpO2
  if (conn_cnt[SPO2] > (3000 / NP_UPD_DELAY_MS))
  {
    if (unit_isenabled(SPO2))
    {
      alarm_set(SPO2_NOCONNECT);
      error("reset SPO2\n");
      dio_module_cmd(PD_ID_SPO2, SBDP_RST_MOD);
      alarm_clr(SPO2_NOSENSOR);
     // alarm_clr(SPO2_RISKSAT);
     // alarm_clr(SPO2_RISKPULSE);
      alarm_clr(SPO2_TUNEPROC);
      alarm_clr(SPO2_TUNEERR);
      alarm_clr(SPO2_DARKOBJ);
      alarm_clr(SPO2_TRANSPOBJ);
      alarm_clr(SPO2_BADSIGNAL);
    }
    conn_cnt[SPO2] = 20;
  }
  else
  {
    if (conn_cnt[SPO2] < 20) alarm_clr(SPO2_NOCONNECT);
  }
  // T1T2
  if (conn_cnt[T1T2] > (3000 / NP_UPD_DELAY_MS))
  {
    alarm_set(T1T2_NOCONNECT);

   // conn_cnt[T1T2] = 0;
    conn_cnt[T1T2] -= conn_cnt[T1T2]/2;
    dio_module_cmd(PD_ID_T1T2, SBDP_RST_MOD);
  }
  else
  {
    if (conn_cnt[T1T2] < 20) alarm_clr(T1T2_NOCONNECT);
  }
  // CO2
  if (conn_cnt[CO2] > (3000 / NP_UPD_DELAY_MS))
  {
    if (unit_isenabled(CO2))
    {
      debug("reset CO2\n");
      alarm_set(CO2_NOCONNECT);

      co2_undef_state();

      conn_cnt[CO2] -= conn_cnt[CO2]/2;
      dio_module_cmd(PD_ID_CO2, SBDP_RST_MOD);
    }
  }
  else
  {
    if (conn_cnt[CO2] < 20) alarm_clr(CO2_NOCONNECT);
  }
}

void dproc_add_frame(unsigned int id, unsigned int ts, int len, unsigned char *pp)
{
  ecg_data_t   ecg_data;
  alarm_info_t ai;
  alarm_cfg_t ac;
  char *ps;

  switch (id)
  {
    case PD_ID_NONE:
     // if (strncmp(pp, "ALIVE", 4) != 0) // do not display ALIVE messages
      ps = strstr((char*)pp, "sbdp");
      if (ps)
      {
        // retrieve SBDP version
        ps += strlen("sbdp ");
        strcpy(sbdp_version, ps);
      }
      if (memcmp(pp, "ALIVE", 5) != 0)
        info("%s\n", pp);
      sched_start(SCHED_SBDP, 5000, sbdp_reset, SCHED_NORMAL); // 5s

      csio_debug_info_t csio_debug_info;
      csio_debug_info.len = len;
      memset(&csio_debug_info, 0, sizeof(csio_debug_info_t));
      memcpy(csio_debug_info.payload, pp, (len<sizeof(csio_debug_info.payload))?len:sizeof(csio_debug_info.payload));
      csio_send_msg(CSIO_ID_DEBUG, &csio_debug_info, sizeof(csio_debug_info_t));
      break;
    case PD_ID_ECG:
      ecgm_process_packet(pp, len);
      break;
    case PD_ID_T1T2:
      sched_start(SCHED_SBDP, 5000, sbdp_reset, SCHED_NORMAL); // 5s
      t1t2_process_packet(pp, len);
     // csio_send_msg(CSIO_ID_T1T2, pp, len);
      break;
    case PD_ID_CO2:
      co2_process_packet(pp, len);
      break;
    case PD_ID_SPO2:
      spo2_process_packet(pp, len);
      break;
    case PD_ID_ED:
#if 0
      if ( ((ecg_pars_t*)pp)->hr == ECGCALC_UNDEF_VALUE)
        debug("PD_ID_ED: HR ---\n");
      else
        debug("PD_ID_ED: HR %d\n", ((ecg_pars_t*)pp)->hr);
#endif
      ep_process_packet(pp, len);
      sched_start(SCHED_EP_WAIT, 5000, ecg_on_no_ep, SCHED_NORMAL);
      break;
    case PD_ID_RPEAK:
     // debug("qQRS: HR %d\n", (pp[2]<<8) | pp[1]);
     // play_sound(HEART_BEAT);
      unit_get_data(ECG, &ecg_data);
      if (ecg_data.hr_src == HRSRC_ECG || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == ECG))
      {
        respcalc_pulse_beep();

        unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HEART_IMG, IMAGE_HEART);
        sched_start(SCHED_HEARTBEAT, 100, unit_hide_heart, SCHED_NORMAL);

        alarm_cfg_get(&ac);
        alarm_info_get(&ai);
        if (ai.current_sound_alarm == 0 || ac.volume == 0)
        {
          unsigned char b;
          b = 0x00;   // none
          if (ai.current_color_level == LOW_RISK)
            b = 0x01; // blue
          else
          if (ai.current_color_level == MED_RISK)
            b = 0x02; // yellow
          else
          if (ai.current_color_level == HIGH_RISK)
            b = 0x04; // red
       // debug("beep\n");
          crbpio_send_msg( CRBP_SOUND_ALARM, b, BeepHeartRateSound, 0x00, (int)(exp(ac.qrs_volume*log(256)/5) - 1) );
        }
      }
      if (!ep_is_ok() && (ecg_data.hr_src == HRSRC_ECG /*|| ecg_data.hr_src == HRSRC_AUTO*/))
      {
        unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (pp[2]<<8) | pp[1]);
        ecg_data.hr = (pp[2]<<8) | pp[1];
      }
      unit_set_data(ECG, &ecg_data);
      // reload asystolia detector
      // ((ecg_data.break_byte & 0xF) == 0) // no breaks
      alarm_clr(ECG_ASYSTOLIA);
      sched_start(SCHED_QRS_WAIT, ecg_data.asys_sec*1000, ecg_on_no_qrs, SCHED_NORMAL);
      sched_stop(SCHED_PM_NORESP);
      csio_rpeak_t csio_rpeak;
      memset(&csio_rpeak, 0, sizeof(csio_rpeak));
      csio_rpeak.id = pp[0];
      csio_rpeak.hr = (pp[2]<<8) | pp[1];
      csio_send_msg(CSIO_ID_RPEAK, &csio_rpeak, sizeof(csio_rpeak_t));
      break;
    case PD_ID_ECS:
     // debug("PD_ID_ECS: 0x%02X 0x%02X\n", pp[0], pp[1]);
      if (pp[0] == ECS_ARRH)
      {
        int alarm = NO_ALARMS;
        switch(pp[1])
        {
          case ARRH_VENTR_FIBR:
            alarm = ECG_VENTR_FIBR;
            break;
          case ARRH_VENTR_FLUT:
            alarm = ECG_VENTR_FLUT;
            break;
          case ARRH_PM_NORESP:
           // debug("ARRH_PM_NORESP disabled\n");
#if 0
            alarm = ECG_PM_NO_ANSWER;
#if 1
            unit_get_data(ECG, &ecg_data);
            if (ecg_data.break_byte)
              alarm = NO_ALARMS;
#endif
#endif
            break;
          default:
           // alarm = NO_ALARMS;
            break;
        }
        if (alarm != NO_ALARMS)
        {
         // debug("arrhythmia %d\n", pp[1]);
          alarm_set(alarm);
          alarm_clr(alarm);
        }
      }
      csio_ecs_t csio_ecs;
      memset(&csio_ecs, 0, sizeof(csio_ecs));
      memcpy(&csio_ecs, pp, len<sizeof(csio_ecs) ? len : sizeof(csio_ecs));
      csio_send_msg(CSIO_ID_ECS, &csio_ecs, sizeof(csio_ecs_t));
      break;
    case PD_ID_RESP:
     // debug("RESP BR = %d, %s\n", (pp[0]>>1)&0x7F, (pp[0]&0x1)?"apnoe":"norm");
//printf("RESP BR = %d, %s\n", (pp[0]>>1)&0x7F, (pp[0]&0x1)?"apnoe":"norm");
#if !defined (RESPCALC_COLIBRI)
      resp_process_packet(pp, len);
#endif
      break;
    default:
      error("unknown packet (id = %d)\n", id);
      break;
  }
  dproc_conn_ok(id);
}

void dproc_add_trv(int chno, int param, int v, unsigned char st)
{
  assert (param < NUM_PARAMS);
  if (v == UNDEF_VALUE) return;

  if (chno >= 0)
  {
    if (!unit_isenabled(chno)) return;
  }

#ifdef UNIX
  pthread_mutex_lock(&mutex);
#endif

  if (param == PARAM_NIBP && trd.n[param] > 0)
  {
    // TODO: calc average nibp value
    trd.n[param] -= 1;
    trd.tr.v[param] = 0;
  }

  trd.tr.v[param] += v;
  trd.tr.vu[param] = max(v, trd.tr.vu[param]);
  trd.tr.vl[param] = min(v, trd.tr.vl[param]);
  trd.n[param] += 1;
  trd.tr.st[param] |= st;

#ifdef UNIX
  pthread_mutex_unlock(&mutex);
#endif
}

void dproc_process_tr(void)
{
  tr_t ltr;
  time_t tt;
  struct tm * ptm;
  trts_t trts;
  int v, i;

  time(&tt);
  ptm = localtime(&tt);
  ptm->tm_min = ptm->tm_min-1;
  mktime(ptm);
  memset(&trts, 0, sizeof(trts_t));
  trts.hour = ptm->tm_hour;
  trts.min = ptm->tm_min;
  trts.day = ptm->tm_mday;
  trts.mon = ptm->tm_mon+1;
  trts.year = ptm->tm_year;
  v = 0;
  memcpy(&v, &trts, sizeof(int));
  dproc_add_trv(-1, PARAM_TS, v, 0);

#ifdef UNIX
  pthread_mutex_lock(&mutex);
#endif

  memset(&ltr, 0, sizeof(tr_t));
  for (i=0; i<NUM_PARAMS; i++)
  {
    ltr.v[i]  = (trd.n[i] != 0) ? trd.tr.v[i] / trd.n[i] : TR_UNDEF_VALUE;
    ltr.vl[i] = (trd.n[i] != 0) ? trd.tr.vl[i] : TR_UNDEF_VALUE;
    ltr.vu[i] = (trd.n[i] != 0) ? trd.tr.vu[i] : TR_UNDEF_VALUE;
    ltr.st[i] = (trd.n[i] != 0) ? trd.tr.st[i] : 0;
  }

  memset(&trd, 0, sizeof(trd));
  for (i=0; i<NUM_PARAMS; i++)
  {
    trd.tr.vl[i] = 0x00007fff;
  }
#ifdef UNIX
  pthread_mutex_unlock(&mutex);
#endif

#if 1
  tr_append(&ltr);
#else
  debug("%s: tr_append disabled\n", __FUNCTION__);
#endif

  tframe_command(TFRAME_NEWDATA, NULL);
  lframe_command(LFRAME_NEWDATA, NULL);
}

void dproc_retrieve_sno(void)
{
 // static int already_here = 0; <- not needed as called from the same thread
 // if (already_here) return;
 // already_here = 1;

#if 1
  static int retry_attempt = 0;
  if (serial_no == SERIAL_NO_UNINIT && retry_attempt < 3)
  {
    sched_start(SCHED_ANY, 0, dproc_retrieve_sno, SCHED_DO_ONCE);
    retry_attempt ++;
  }
#endif

 // already_here = 0;
}

void dproc_update(void)
{
  dproc_check_conns();
}
