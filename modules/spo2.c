/*! \file spo2.c
    \brief Pulsoximetry interface
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bedmoni.h"
#include "iframe.h"
#include "unit.h"
#include "mframe.h"
#include "alarm.h"
#include "crbpio.h"
#include "sched.h"
#include "dproc.h"
#include "csio.h"
#include "ecgset.h"
#include "respcalc.h"
#include "spo2.h"

void spo2_process_packet(unsigned char *data, int len)
{
  spo2_packet_t spo2_packet, *pspo2_packet;
  spo2_data_t spo2_data;
  ecg_data_t ecg_data;
  static unsigned char old_sync = 0;
  static int spo2_entrance_counter = 0;
//  static unsigned char old_spo2_stat = 0xFF;
//  static int spo2, pulse;

  if (unit_get_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    error("%s: reading ecg data\n", __FUNCTION__);
  }

  if ((unsigned long)data & 0x1)
  {
    pspo2_packet = &spo2_packet;
    memcpy(pspo2_packet, data, sizeof(spo2_packet_t));
  }
  else
    pspo2_packet = (spo2_packet_t*)data;

  if ((unsigned char)(pspo2_packet->sync - old_sync) != 1)
  {
    error("SpO2 sync error: %02X %02X\n", old_sync, pspo2_packet->sync);
  }
  old_sync = pspo2_packet->sync;

  if (pspo2_packet->stat_bits.pulse_bip)
  {
    if (ecg_data.hr_src == HRSRC_SPO2 || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == SPO2))
    {
      respcalc_pulse_beep();

      alarm_info_t ai;
      alarm_cfg_t ac;

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
        crbpio_send_msg( CRBP_SOUND_ALARM, b, BeepHeartRateSound, 0x00, (int)(exp(ac.qrs_volume*log(256)/5) - 1) );
      }
    }
  }

  if (pspo2_packet->hr_mode.hr >= spo2_data.hr_min && pspo2_packet->hr_mode.hr <= spo2_data.hr_max && pspo2_packet->hr_mode.hr != spo2_data.hr)
  {
    unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_PULSE, pspo2_packet->hr_mode.hr);
    if (ecg_data.hr_src_curr != SPO2 && ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr == UNDEF_VALUE)
    {
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[SPO2]);
      ecg_data.hr_src_curr = SPO2;
    }
    if (ecg_data.hr_src == HRSRC_SPO2 || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == SPO2))
    {
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, pspo2_packet->hr_mode.hr);
     // ecg_data.hr = pspo2_packet->hr_mode.hr;
    }
    alarm_clr(SPO2_RISKPULSE);
    dproc_add_trv(SPO2, PARAM_PULSE, (int)(pspo2_packet->hr_mode.hr), 0);
    spo2_data.hr = pspo2_packet->hr_mode.hr;
  }
  if (pspo2_packet->hr_mode.hr == 0)
  {
    if (spo2_data.hr != UNDEF_VALUE)
      unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_PULSE, (int)UNDEF_VALUE);
#if 1
    if (ecg_data.hr_src == HRSRC_SPO2 /*|| (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == SPO2)*/)
    {
     // ecg_data.hr_src_curr = ECG;
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[ECG]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int)UNDEF_VALUE);
    }
#endif
    alarm_clr(SPO2_RISKPULSE);
    spo2_data.hr = UNDEF_VALUE;
  }
  else if (pspo2_packet->hr_mode.hr < spo2_data.hr_min || pspo2_packet->hr_mode.hr > spo2_data.hr_max)
  {
    unsigned char st = 0;
    if      (pspo2_packet->hr_mode.hr > spo2_data.hr_max) st |= TR_ST_U;
    else if (pspo2_packet->hr_mode.hr < spo2_data.hr_min) st |= TR_ST_L;
    if (pspo2_packet->hr_mode.hr != spo2_data.hr)
      unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_PULSE, pspo2_packet->hr_mode.hr);
    if (ecg_data.hr_src_curr != SPO2 && ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr == UNDEF_VALUE)
    {
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[SPO2]);
      ecg_data.hr_src_curr = SPO2;
    }
    if ( (ecg_data.hr_src == HRSRC_SPO2 || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == SPO2)) && pspo2_packet->hr_mode.hr != ecg_data.hr)
    {
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, pspo2_packet->hr_mode.hr);
      ecg_data.hr = pspo2_packet->hr_mode.hr;
    }
    alarm_set(SPO2_RISKPULSE);
    dproc_add_trv(SPO2, PARAM_PULSE, (int)(pspo2_packet->hr_mode.hr), st);
    spo2_data.hr = pspo2_packet->hr_mode.hr;
  }

  if (pspo2_packet->sat == 100) pspo2_packet->sat = 99;
  if (pspo2_packet->sat >= spo2_data.spo2_min && pspo2_packet->sat <= spo2_data.spo2_max && pspo2_packet->sat != spo2_data.spo2)
  {
    unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT, pspo2_packet->sat);
    alarm_clr(SPO2_RISKSAT);
    dproc_add_trv(SPO2, PARAM_SPO2, pspo2_packet->sat, 0);
    spo2_data.spo2 = pspo2_packet->sat;
  }
  if (pspo2_packet->sat > 100)
  {
    if (spo2_data.spo2 != UNDEF_VALUE)
      unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT, (int)UNDEF_VALUE);
    alarm_clr(SPO2_RISKSAT);
    spo2_data.spo2 = UNDEF_VALUE;
  }
  else if (pspo2_packet->sat < spo2_data.spo2_min || pspo2_packet->sat > spo2_data.spo2_max)
  {
    unsigned char st = 0;
    if      (pspo2_packet->sat > spo2_data.spo2_max) st |= TR_ST_U;
    else if (pspo2_packet->sat < spo2_data.spo2_min) st |= TR_ST_L;
    if (pspo2_packet->sat != spo2_data.spo2)
      unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT, pspo2_packet->sat);
    alarm_set(SPO2_RISKSAT);
    dproc_add_trv(SPO2, PARAM_SPO2, pspo2_packet->sat, st);
    spo2_data.spo2 = pspo2_packet->sat;
  }

  if (pspo2_packet->stolb & 0x80)
  {
    unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SCALE, pspo2_packet->stolb*0x7F);
    spo2_data.scale = pspo2_packet->stolb*0x7F;
  }
  else
  {
    unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_STOLBIK, pspo2_packet->stolb*100/64);
//printf("stolbik = %d\n", pspo2_packet->stolb*100/30);
    spo2_data.stolb = pspo2_packet->stolb;
  }

  if (unit_set_data(ECG, &ecg_data) <= 0)
  {
    error("%s: writing ecg data\n", __FUNCTION__);
  }
  if (unit_set_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: writing spo2 data\n", __FUNCTION__);
  }

  if ((++spo2_entrance_counter & 63) == 63/* && old_spo2_stat != pspo2_packet->stat*/)
  {
    alarm_set_clr(SPO2_NOSENSOR,  pspo2_packet->stat_bits.no_sensor);
    alarm_set_clr(SPO2_NOPATIENT, pspo2_packet->stat_bits.no_patient);
    alarm_set_clr(SPO2_TUNEPROC,  pspo2_packet->stat_bits.tune_proc);
    alarm_set_clr(SPO2_TUNEERR,   pspo2_packet->stat_bits.tune_err);
    alarm_set_clr(SPO2_DARKOBJ,   pspo2_packet->stat_bits.dark_obj);
    alarm_set_clr(SPO2_TRANSPOBJ, pspo2_packet->stat_bits.transp_obj);
    alarm_set_clr(SPO2_BADSIGNAL, pspo2_packet->stat_bits.bad_signal);

//    old_spo2_stat = pspo2_packet->stat;
  }

  dview_chandata_add(SPO2, pspo2_packet->fpg);

  csio_spo2data_c_t csio_spo2data_c;
  memset(&csio_spo2data_c, 0, sizeof(csio_spo2data_c_t));
  csio_spo2data_c.sync = pspo2_packet->sync;
  csio_spo2data_c.mode = pspo2_packet->hr_mode.mode;
  csio_spo2data_c.mode |= (1 << pspo2_packet->stat_bits.pulse_bip);
  csio_spo2data_c.fpg = pspo2_packet->fpg;
  csio_send_msg(CSIO_ID_SPO2_C, &csio_spo2data_c, sizeof(csio_spo2data_c_t));

/*
#if !defined (ARM)
unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, 80);
unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_VALUE, -29);
unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_VALUE, +28);

unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_HR, 78);
unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT, 98);

unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_SS, 120);
unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_DD, 80);
unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_CC, 104);

unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_CUFF_PRESSURE, 30);
//unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_PULSE, 200);
char s[200];
sprintf(s, "%02d:%02d %02d.%02d.%04d", 12, 34, 02, 02, 102+1900);
unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_STIME, s);

unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR, 19);
unit_ioctl(RESP, SET_VALUE, UNIT_RESP_STATE, IDS_NORM);

unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1, (float)36.6);
unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2, (float)38.6);
unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT, (float)2.0);

unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2, 40);
unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR, 20);
unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2, 0);
unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BRSTATE, IDS_NORM);

alarm_clr(SPO2_NOSENSOR);
alarm_clr(T1T2_NOSENSOR);
alarm_clr(CO2_NOCONNECT);
#endif
*/
}

