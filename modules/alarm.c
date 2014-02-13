/*! \file alarm.c
    \brief alarm analysis implementation
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "bedmoni.h"
#include "dview.h"
#include "iframe.h"
#include "mframe.h"
#include "unit.h"
#include "stconf.h"
#include "crbpio.h"
#include "elog.h"
#include "alarm.h"

static alarm_cfg_t alarm_cfg;
static alarm_info_t alarm_info;
static pthread_mutex_t alarm_cfg_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t alarm_info_mutex = PTHREAD_MUTEX_INITIALIZER;

alarm_msg_t alarm_msg[NUM_ALARMS] = 
{
  // must be in order in enum declaration
  {NO_ALARMS,      NO_RISK,   MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_NONE, AlarmSound_None, -1, -1, 0, IDS_NONE},
  {T1T2_NOSENSOR,  LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, T1T2, -1, 0, IDS_ALARM_T1T2_NOSENSOR},
  {T1T2_NOCONNECT, MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_Any, T1T2, -1, 0, IDS_ALARM_T1T2_NOCONNECT},
  {SPO2_NOSENSOR,  LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, SPO2, -1, 0, IDS_ALARM_SPO2_NOSENSOR},
  {SPO2_NOPATIENT, LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, SPO2, -1, 0, IDS_ALARM_SPO2_NOPATIENT},
  {SPO2_NOPULSE,   MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_Any, SPO2, -1, 0, IDS_ALARM_SPO2_NOPULSE},
  {SPO2_RISKPULSE, MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, SPO2, UNIT_SPO2_PULSE, 0, IDS_ALARM_SPO2_RISKPULSE},
  {SPO2_RISKSAT,   MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, SPO2, UNIT_SPO2_SAT, 0, IDS_ALARM_SPO2_RISKSAT},
  {SPO2_TUNEPROC,  LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, SPO2, -1, 0, IDS_ALARM_SPO2_TUNEPROC},
  {SPO2_TUNEERR,   LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, SPO2, -1, 0, IDS_ALARM_SPO2_TUNEERR},
  {SPO2_DARKOBJ,   LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, SPO2, -1, 0, IDS_ALARM_SPO2_DARKOBJ},
  {SPO2_TRANSPOBJ, LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, SPO2, -1, 0, IDS_ALARM_SPO2_TRANSPOBJ},
  {SPO2_BADSIGNAL, LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, SPO2, -1, 0, IDS_ALARM_SPO2_BADSIGNAL},
  {NIBP_NORESP,    LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, NIBP, -1, 0, IDS_ALARM_NIBP_NORESP},
  {NIBP_RISKSS,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, NIBP, UNIT_NIBP_BP_SS, 0, IDS_ALARM_NIBP_RISKSS},
  {NIBP_RISKDD,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, NIBP, UNIT_NIBP_BP_DD, 0, IDS_ALARM_NIBP_RISKDD},
  {NIBP_RISKCC,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, NIBP, UNIT_NIBP_BP_CC, 0, IDS_ALARM_NIBP_RISKCC},
  {NIBP_RISKMEASTIME, MED_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_Any, NIBP, -1, 0, IDS_ALARM_NIBP_RISKMEASTIME},
  {ECG_NOCONNECT,  MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, ECG, -1, 0, IDS_ALARM_ECG_NOCONNECT},
  {ECG_LBREAK,     LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, ECG, -1, 0, IDS_ALARM_ECG_LBREAK},
  {ECG_RBREAK,     LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, ECG, -1, 0, IDS_ALARM_ECG_RBREAK},
  {ECG_FBREAK,     LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, ECG, -1, 0, IDS_ALARM_ECG_FBREAK},
  {ECG_CBREAK,     LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, ECG, -1, 0, IDS_ALARM_ECG_CBREAK},
  {ECG_RISKHR,     MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED ,ALR_PHYS, AlarmSound_Cardiac_MP, ECG, UNIT_ECG_HR, 0, IDS_ALARM_ECG_RISKHR},
  {ECG_RISKSTI,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_MP, ECG, -1, 0, IDS_ALARM_ECG_RISKSTI},
  {ECG_RISKSTII,   MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_MP, ECG, -1, 0, IDS_ALARM_ECG_RISKSTII},
  {ECG_RISKSTIII,  MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_MP, ECG, -1, 0, IDS_ALARM_ECG_RISKSTIII},
  {ECG_RISKSTAVR,  MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_MP, ECG, -1, 0, IDS_ALARM_ECG_RISKSTAVR},
  {ECG_RISKSTAVL,  MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_MP, ECG, -1, 0, IDS_ALARM_ECG_RISKSTAVL},
  {ECG_RISKSTAVF,  MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_MP, ECG, -1, 0, IDS_ALARM_ECG_RISKSTAVF},
  {ECG_RISKSTV,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_MP, ECG, -1, 0, IDS_ALARM_ECG_RISKSTV},
  {T1T2_RISKT1,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, T1T2, UNIT_T1T2_T1, 0, IDS_ALARM_T1T2_RISKT1},
  {T1T2_RISKT2,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, T1T2, UNIT_T1T2_T2, 0, IDS_ALARM_T1T2_RISKT2},
  {T1T2_RISKDT,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, T1T2, UNIT_T1T2_DT, 0, IDS_ALARM_T1T2_RISKDT},
  {RESP_RISKBR,    MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Any, RESP, UNIT_RESP_BR, 0, IDS_ALARM_RESP_RISKBR},
  {RESP_APNOE,     HIGH_RISK, MSG_CLR_FIXED, MSG_FIX, SND_ENABLED, ALR_PHYS, AlarmSound_General_HP, RESP, UNIT_RESP_STATE, 0, IDS_ALARM_RESP_APNOE},
  {SYS_LOWCHARGE,  MED_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_General_MP, -1, -1, 0, IDS_ALARM_SYS_LOWCHARGE},
  {SYS_EXTREMLOWCHARGE, HIGH_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_General_HP, -1, -1, 0, IDS_ALARM_SYS_EXTREMLOWCHARGE},
  {ECG_ASYSTOLIA,  HIGH_RISK, MSG_CLR_FIXED, MSG_FIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_HP, ECG, -1, 0, IDS_ALARM_ECG_ASYSTOLIA},
  {ECG_PM_NO_ANSWER, HIGH_RISK, MSG_CLR_FIXED, MSG_FIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_HP, ECG, -1, 0, IDS_ALARM_ECG_PM_NO_ANSWER},
  {ECG_TACHYCARDIA, MED_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_MP, ECG, -1, 0, IDS_ALARM_ECG_TACHYCARDIA},
  {SPO2_NOCONNECT, MED_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_Any, SPO2, -1, 0, IDS_ALARM_SPO2_NOCONNECT},
  {TPRN_OPENED, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, -1, -1, 0, IDS_ALARM_TPRN_OPENED},
  {TPRN_PAPEROUT, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, -1, -1, 0, IDS_ALARM_TPRN_PAPEROUT},
  {TPRN_OVERHEAT, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, -1, -1, 0, IDS_ALARM_TPRN_OVERHEAT},
  {TPRN_NOCONNECT, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, -1, -1, 0, IDS_ALARM_TPRN_NOCONNECT},
  {RESP_BADCONTACT_RL, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, RESP, -1, 0, IDS_ALARM_RESP_BADCONTACT_RL},
  {RESP_BADCONTACT_RF, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, RESP, -1, 0, IDS_ALARM_RESP_BADCONTACT_RF},
  {CO2_RISKETCO2, MED_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_General_MP, CO2, UNIT_CO2_ETCO2, 0, IDS_ALARM_CO2_RISKETCO2},
  {CO2_RISKICO2, MED_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_General_MP, CO2, UNIT_CO2_ICO2, 0, IDS_ALARM_CO2_RISKICO2},
  {CO2_RISKBRCO2, MED_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_General_MP, CO2, UNIT_CO2_BR, 0, IDS_ALARM_CO2_RISKBRCO2},
  {NIBP_RISKINFL, HIGH_RISK, MSG_CLR_FIXED, MSG_FIX, SND_ENABLED, ALR_TECH, AlarmSound_General_HP, NIBP, -1, 0, IDS_ALARM_NIBP_RISKINFL},
  {CO2_APNOE, HIGH_RISK, MSG_CLR_FIXED, MSG_FIX, SND_ENABLED, ALR_PHYS, AlarmSound_General_HP, CO2, UNIT_CO2_BRSTATE, 0, IDS_ALARM_RESP_APNOE},
  {SYS_NO_USBDISK, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, -1, -1, 0, IDS_ALARM_SYS_NO_USBDISK},
  {NIBP_WEAKSIGNAL, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_None, NIBP, -1, 0, IDS_ALARM_NIBP_WEAKSIGNAL},
  {NIBP_ERRSIGNAL, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_PHYS, AlarmSound_None, NIBP, -1, 0, IDS_ALARM_NIBP_ERRSIGNAL},
  {NIBP_EXCRETRYCOUNT, MED_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_General_MP, NIBP, -1, 0, IDS_ALARM_NIBP_EXCRETRYCOUNT},
  {NIBP_PNEUMBLOCK, HIGH_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_General_HP, NIBP, -1, 0, IDS_ALARM_NIBP_PNEUMBLOCK},
  {NIBP_INFLTIMEOUT, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, NIBP, -1, 0, IDS_ALARM_NIBP_INFLTIMEOUT},
  {NIBP_CALIB_EXPIRED, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, NIBP, -1, 0, IDS_ALARM_NIBP_CALIB_EXPIRED},
  {CO2_INVDATA,   LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, CO2, -1, 0, IDS_ALARM_CO2_INVDATA},
  {CO2_NOCONNECT, LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, CO2, -1, 0, IDS_ALARM_CO2_NOCONNECT},
  {CO2_OVERTEMP, LOW_RISK,  MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, CO2, -1, 0, IDS_ALARM_CO2_OVERTEMP},
  {ECG_VENTR_FIBR, HIGH_RISK, MSG_CLR_FIXED, MSG_FIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_HP, ECG, -1, 0, IDS_ALARM_ECG_VENTR_FIBR},
  {ECG_VENTR_FLUT, HIGH_RISK, MSG_CLR_FIXED, MSG_FIX, SND_ENABLED, ALR_PHYS, AlarmSound_Cardiac_HP, ECG, -1, 0, IDS_ALARM_ECG_VENTR_FLUT},
  {CO2_ZEROING, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, CO2, -1, 0, IDS_ALARM_CO2_ZEROING},
  {NIBP_INVPOWER, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, NIBP, -1, 0, IDS_ALARM_NIBP_INVPOWER},
  {CO2_CHECKSENSOR, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, CO2, -1, 0, IDS_ALARM_CO2_CHECKSENSOR},
  {CO2_WARMUP, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, CO2, -1, 0, IDS_ALARM_CO2_WARMUP},
  {CO2_STARTUP, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, CO2, -1, 0, IDS_ALARM_CO2_STARTUP},
  {RESP_BREAK, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, RESP, -1, 0, IDS_ALARM_RESP_BREAK},
  {SYS_CS_NOCONNECT, LOW_RISK, MSG_CLR, MSG_NOFIX, SND_ENABLED, ALR_TECH, AlarmSound_None, -1, -1, 0, IDS_ALARM_SYS_CS_NOCONNECT},
};

void alarm_cfg_save(void)
{
  alarm_cfg_t ac;

  alarm_cfg_get(&ac);

  if ( stconf_write(STCONF_ALARM, &ac, sizeof(alarm_cfg_t)) > 0 );
}

void alarm_ini(void)
{
  alarm_cfg_t ac;
  char s[200];

  alarm_ini_def();

  if ( stconf_read(STCONF_ALARM, &ac, sizeof(alarm_cfg_t)) > 0 )
  {
    int i;
    alarm_cfg_set(&ac);
    for (i=0; i<NUM_ALARMS; i++)
    {
     // alarm_on_off(i, ac.alarm_msg[i].enabled);
      alarm_msg[i].enabled = ac.alarm_msg[i].enabled;
      if ( (ac.alarm_msg[i].fixed && ac.alarm_msg[i].state != MSG_CLR_FIXED) ||
           (!ac.alarm_msg[i].fixed && ac.alarm_msg[i].state == MSG_SET) )
      {
       // alarm_set_clr(i, ac.alarm_msg[i].state);
        alarm_msg[i].state = ac.alarm_msg[i].state;
        if (ac.alarm_msg[i].chno != -1)
        {
          unit_ioctl(ac.alarm_msg[i].chno, SET_ALARM, i);
          elog_add(EL_ALARM, i, 1);
        }
      }
//printf("alarm ini: %d= %d %d %d\n", i, alarm_msg[i].chno, alarm_msg[i].state, alarm_msg[i].item);
    }
   // alarm_cfg_set(&ac);
  }

  alarm_info.current_sound_alarm = AlarmSound_None;
  alarm_info.current_extsound_alarm = 0x00;
  alarm_info.current_color_level = NO_RISK;

  alarm_clr(CO2_APNOE);

  sprintf(s, "%s/%s",USBDISK_PATH, DATA_DIR);
  alarm_set_clr(SYS_NO_USBDISK, access(s, R_OK) != 0);

  if (ac.ge == 0 && ac.delay_remd > 0)
    iframe_command(IFRAME_SET_TIMERVAL, (void*)((int)ac.delay_remd));
}

void alarm_deini(void)
{
  alarm_cfg_save();
}

void alarm_ini_def(void)
{
  alarm_cfg.thsh_visible = 1;
  alarm_cfg.volume = 5;
  alarm_cfg.qrs_volume = 0;//5;
  alarm_cfg.delay  = 100;
  alarm_cfg.ge = 1;
  alarm_cfg.delay_remd = 0;
  alarm_cfg.ext_bits.high = 0;
  alarm_cfg.ext_bits.medium = 0;
  alarm_cfg.ext_bits.tech = 0;
  memcpy(alarm_cfg.alarm_msg, alarm_msg, NUM_ALARMS*sizeof(alarm_msg_t));
}

int  alarm_isenabled(int alarm)
{
  return (alarm_msg[alarm].enabled);
}

int  alarm_isset(int alarm)
{
  return (alarm_msg[alarm].state == MSG_SET);
}

void alarm_set(int alarm)
{
//  if (alarm_msg[alarm].chno != -1 && alarm_msg[alarm].state == MSG_CLR && alarm_isenabled(alarm))
//  {
  if (alarm_msg[alarm].state != MSG_SET/* && alarm_msg[alarm].enabled*/)
  {
    if (alarm_msg[alarm].chno != -1)
    {
#if 0
      unit_ioctl(alarm_msg[alarm].chno, SET_ALARM, alarm);
#else
      int it = alarm_msg[alarm].item;
      unit_ioctl(alarm_msg[alarm].chno, SET_ALARM_ITEM, alarm, it);
#endif
    }
    alarm_msg[alarm].state = MSG_SET;
    if (alarm_msg[alarm].fixed)
      alarm_cfg_save();
    elog_add(EL_ALARM, alarm, 1);
  }
//  }
//  if (!alarm_isenabled(alarm)) alarm_clr(alarm);
}

void alarm_clr(int alarm)
{
  if (alarm_msg[alarm].state == MSG_SET/* && alarm_msg[alarm].enabled*/)
  {
    if (alarm_msg[alarm].chno != -1 && alarm_msg[alarm].fixed == MSG_NOFIX)
    {
#if 0
      unit_ioctl(alarm_msg[alarm].chno, CLR_ALARM, alarm);
#else
      int it = alarm_msg[alarm].item;
      unit_ioctl(alarm_msg[alarm].chno, CLR_ALARM_ITEM, alarm, it);
#endif
    }
    alarm_msg[alarm].state = MSG_CLR;
    if (alarm_msg[alarm].fixed)
      alarm_cfg_save();
   // elog_add(EL_ALARM, alarm, 0);
  }
}

void alarm_clr_fixed(int alarm)
{
  if (alarm_msg[alarm].chno != -1 && alarm_msg[alarm].state != MSG_CLR_FIXED/* && alarm_msg[alarm].enabled*/)
  {
#if 0
    unit_ioctl(alarm_msg[alarm].chno, CLR_ALARM, alarm);
#else
    int it = alarm_msg[alarm].item;
    unit_ioctl(alarm_msg[alarm].chno, CLR_ALARM_ITEM, alarm, it);
#endif
    alarm_msg[alarm].state = MSG_CLR_FIXED;
    alarm_cfg_save();
  }
}

void alarm_set_clr(int alarm, int set_clr)
{
  if (set_clr)
    alarm_set(alarm);
  else
    alarm_clr(alarm);
 // alarm_cfg_save();
}

void alarm_on_off(int alarm, int on_off)
{
  alarm_msg[alarm].enabled = on_off;
  iframe_command(IFRAME_MSG_UPDATE, NULL);
//  if (on_off == 0)
//    alarm_clr(alarm);
  alarm_cfg_save();
}

void alarm_enable(int alarm)
{
  alarm_on_off(alarm, 1);
}

void alarm_disable(int alarm)
{
  alarm_on_off(alarm, 0);
}

void alarm_ge_on(void)
{
  alarm_cfg.ge = 1;
  alarm_cfg.delay_remd = 0;
//  iframe_command(IFRAME_MSG_UPDATE, NULL);
  alarm_cfg_save();
}

void alarm_rst(void)
{
  int i;
  for (i=0; i<NUM_ALARMS; i++)
  {
    if (alarm_msg[i].type == ALR_PHYS)
    {
      if (alarm_msg[i].state == MSG_SET)
        iframe_command(IFRAME_SET_TIMERVAL, (void*)((int)180));
      else
        alarm_clr_fixed(i);
    }
    else // TECH and -1
    {
      alarm_clr(i);
    }
  }
  alarm_cfg_save();
}

void alarm_ge_off(void)
{
  alarm_info_t ai;
  unsigned char b;

  alarm_cfg.ge = 0;

  alarm_info_get(&ai);
  ai.current_sound_alarm = AlarmSound_None;
  ai.current_extsound_alarm = 0x00;
  alarm_info_set(&ai);

  b = 0x00;   // none
  if (ai.current_color_level == LOW_RISK)
    b = 0x01; // blue
  else
  if (ai.current_color_level == MED_RISK)
    b = 0x02; // yellow
  else
  if (ai.current_color_level == HIGH_RISK)
    b = 0x04; // red

  crbpio_send_msg(CRBP_SOUND_ALARM, b, 0x00, 0x00, 0x00);
  alarm_cfg_save();
}

void alarm_update(void)
{
  mframe_process_alarms();
  iframe_process_msgs();
}

void alarm_crbp(int color_level, int alarm_sound)
{
 // static int alarm_sound_current = AlarmSound_None;
  unsigned char b, bb;
  alarm_info_t ai;

  alarm_info_get(&ai);

  if (ai.current_color_level == color_level)
  {
    bb = 0x00 | ((color_level==LOW_RISK) & alarm_cfg.ext_bits.tech) | ((color_level==MED_RISK) & alarm_cfg.ext_bits.medium) | ((color_level==HIGH_RISK) & alarm_cfg.ext_bits.high);
    bb = bb ? 0x01 : 0x00;
    if (ai.current_sound_alarm == alarm_sound && ai.current_extsound_alarm == bb) return;
    if (alarm_cfg.ge == 0) return;
  }

  b = 0x00;   // none
  if (color_level == LOW_RISK)
    b = 0x01; // blue
  else
  if (color_level == MED_RISK)
    b = 0x02; // yellow
  else
  if (color_level == HIGH_RISK)
    b = 0x04; // red

  if (alarm_cfg.ge == 0)
  {
    if (ai.current_color_level != color_level)
    {
      debug("alarm ge disabled, no alarm beep (color %d)\n", color_level);
      crbpio_send_msg(CRBP_SOUND_ALARM, b, 0x00, 0x00, 0x00);
      ai.current_color_level = color_level;
    }
  }
  else
  {
    bb = 0x00 | ((color_level==LOW_RISK) & alarm_cfg.ext_bits.tech) | ((color_level==MED_RISK) & alarm_cfg.ext_bits.medium) | ((color_level==HIGH_RISK) & alarm_cfg.ext_bits.high);
    bb = bb ? 0x01 : 0x00;
    debug("alarm beep, color %d, sound %d, ext %s\n", color_level, alarm_sound, (bb) ? "ON" : "OFF");
    crbpio_send_msg(CRBP_SOUND_ALARM, b, alarm_sound, bb, exp(alarm_cfg.volume*log(256)/SOUND_MAX_VOLUME) -
 1);
    ai.current_color_level = color_level;
    ai.current_sound_alarm = alarm_sound;
    ai.current_extsound_alarm = bb;
  }

  alarm_info_set(&ai);
}

void alarm_on_off_all_phys(int enabled)
{
  int i;

  for (i=0; i<NUM_ALARMS; i++)
  {
    if (alarm_msg[i].type == ALR_PHYS)
    {
      alarm_msg[i].enabled = enabled;
    }
  }
  iframe_command(IFRAME_MSG_UPDATE, NULL);
  alarm_cfg_save();
}

void alarm_on_off_all_tech(int enabled)
{
  int i;

  for (i=0; i<NUM_ALARMS; i++)
  {
    if (alarm_msg[i].type == ALR_TECH)
    {
      alarm_msg[i].enabled = enabled;
    }
  }
  iframe_command(IFRAME_MSG_UPDATE, NULL);
  alarm_cfg_save();
}

void alarm_cfg_get(alarm_cfg_t * pcfg)
{
  pthread_mutex_lock(&alarm_cfg_mutex);
  if (pcfg)
  {
    *pcfg = alarm_cfg;
    memcpy(pcfg->alarm_msg, alarm_msg, NUM_ALARMS*sizeof(alarm_msg_t));
  }
  pthread_mutex_unlock(&alarm_cfg_mutex);
}

void alarm_cfg_set(alarm_cfg_t * pcfg)
{
  pthread_mutex_lock(&alarm_cfg_mutex);
  if (pcfg)
  {
    memcpy(&alarm_cfg, pcfg, sizeof(alarm_cfg_t));
    memcpy(alarm_msg, pcfg->alarm_msg, NUM_ALARMS*sizeof(alarm_msg_t));
  }
  pthread_mutex_unlock(&alarm_cfg_mutex);
}

void alarm_info_get(alarm_info_t * pai)
{
  pthread_mutex_lock(&alarm_info_mutex);
  if (pai) *pai = alarm_info;
  pthread_mutex_unlock(&alarm_info_mutex);
}

void alarm_info_set(alarm_info_t * pai)
{
  pthread_mutex_lock(&alarm_info_mutex);
  if (pai) memcpy(&alarm_info, pai, sizeof(alarm_info_t));
  pthread_mutex_unlock(&alarm_info_mutex);
}
