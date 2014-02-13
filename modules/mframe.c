/*! \file mframe.c
    \brief Frame, containing channels data/settings/measurements
 */

#ifdef UNIX
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#ifdef WIN32
//#include <windows.h>
//#include <commctrl.h>
#endif

#include "dview.h"
#include "grph.h"
#include "mframe.h"
#include "iframe.h"
#include "cframe.h"
#include "sframe.h"
#include "uframe.h"
#include "unit.h"
#include "mainmenu.h"
#include "stconf.h"
#include "nibp.h"
#include "ep.h"
#include "ecgset.h"
#include "ecgcalc.h"
#include "resp.h"
#include "dio.h"
#include "sched.h"
#include "resp.h"
#if defined (RESPCALC_COLIBRI)
#include "respcalc.h"
#endif
#include "bedmoni.h"

#define NORM(color) color
#define DARK(color) \
((unsigned char)((unsigned char)(color >> 16) * 0.7) << 16) | ((unsigned char)((unsigned char)(color >> 8) * 0.7) << 8) | ((unsigned char)((unsigned char)(color >> 0) * 0.7) << 0)

static mframe_t mframe;

static void mframe_bitblt(int x, int y, int cx, int cy);
static void mframe_on_unit_par_ranges(mframe_t * pmframe, int v);
static void mframe_on_enumerate(mframe_t * pmframe);

static startup_info_t startup_info;

static void mframe_cfg_save(void)
{
  mframe_t * pmframe;
  mframe_cfg_t mframe_cfg;

  pmframe = &mframe;
  memset(&mframe_cfg, 0, sizeof(mframe_cfg_t));
  mframe_cfg.frame_visibility = pmframe->visible;
  mframe_cfg.maximized = pmframe->maximized;
  mframe_cfg.unit_chmask = pmframe->unit_chmask;
  if ( stconf_write(STCONF_MFRAME, &mframe_cfg, sizeof(mframe_cfg_t)) > 0 );
}

void mframe_init(void)
{
  mframe_t * pmframe;
  int i;
  char s[128];
  int v;
  ecg_data_t  ecg_data;
  spo2_data_t spo2_data;
  nibp_data_t nibp_data;
  resp_data_t resp_data;
  t1t2_data_t t1t2_data;
  co2_data_t  co2_data;
  alarm_cfg_t ac;
  unsigned short ids;

  pmframe = &mframe;

  memset(pmframe, 0, sizeof(mframe_t));

  pmframe->x    = MFRAME_X0;
  pmframe->y    = MFRAME_Y0;
  pmframe->cx   = MFRAME_CX;
  pmframe->cy   = MFRAME_CY;
  pmframe->bitblt_func = mframe_bitblt;

  pmframe->bkdc = grph_createdc(rootwfd, 0, 0, DISPLAY_CX, pmframe->cy+SFRAME_CY, 0);
  pmframe->fgdc = grph_createdc(rootwfd, 0, 0, DISPLAY_CX, pmframe->cy+SFRAME_CY, 0);
  pmframe->updc = grph_createdc(rootwfd, 0, 0, DISPLAY_CX, pmframe->cy+SFRAME_CY, 0);

  pmframe->num_cmds_in_queue = 0;

  pat_ini(NULL); // need to obtain pat data in unit_init
  unit_init(pmframe);

  units_cfg_t units_cfg;
  if ( stconf_read(STCONF_UNITS, &units_cfg, sizeof(units_cfg_t)) > 0 )
  {
    unit_set_data(ECG,  &units_cfg.ecg_data);
    unit_set_data(SPO2, &units_cfg.spo2_data);
    unit_set_data(NIBP, &units_cfg.nibp_data);
    unit_set_data(T1T2, &units_cfg.t1t2_data);
    unit_set_data(RESP, &units_cfg.resp_data);
    unit_set_data(CO2,  &units_cfg.co2_data);
  }

  unit_get_data(ECG,  &ecg_data);
  unit_get_data(SPO2, &spo2_data);
  unit_get_data(NIBP, &nibp_data);
  unit_get_data(RESP, &resp_data);
  unit_get_data(T1T2, &t1t2_data);
  unit_get_data(CO2 , &co2_data);

  memset(&startup_info, 0, sizeof(startup_info));
  startup_info.ecg_set = ecg_data.set;
 // startup_info.do_not_reset_co2 = 0;

  unit_item_add(ECG, UNIT_TYPE_VAL_LABEL, UNIT_ECG_LABEL, 1, 1, MAIN_ECG, IDS_ECG, 0);
  ids2string(IDS_HRSRC, s);
#if 0
  unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_HRSRC_CAPTION, 42, 8, 15, IDS_HRSRC, s);
  switch (ecg_data.hr_src)
  {
    case HRSRC_AUTO:
      ids = IDS_AUTO;
      break;
    case HRSRC_ECG:
      ids = IDS_ECG;
      break;
    case HRSRC_SPO2:
      ids = IDS_SPO2;
      break;
    case HRSRC_NIBP:
      ids = IDS_NIBP;
      break;
    default:
      ids = IDS_UNDEF3;
  }
  unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_HRSRC, 93, 7, 17, ids, 0);
#endif
  v = alarm_isenabled(ECG_RISKHR) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(ECG, UNIT_TYPE_VAL_ICON, UNIT_ECG_HR_BELL, 159, 3, v, 0);
  snprintf(s, sizeof(s), "%d..%d", ecg_data.hr_min, ecg_data.hr_max);
  unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_HR_RANGE, 180, 2, 13, UNIT_TYPE_RANGE, s);
 // unit_item_add(ECG, UNIT_TYPE_VAL_INT, UNIT_ECG_HR, 16, 5, 74, 0, (int)UNDEF_VALUE);
  unit_item_add(ECG, UNIT_TYPE_VAL_INT, UNIT_ECG_HR, 16, 14, 74, 0, (int)UNDEF_VALUE);
 // unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_ST1_CAPTION, 86, 22, 17, 0, "ST");
  ids2string(chan_ids[ecg_data.st1], s);
  unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_ST1_LEAD_CAPTION, 140, 22, 13, 0, s);
  unit_item_add(ECG, UNIT_TYPE_VAL_INT, UNIT_ECG_ST1_VALUE, 176, 22, 13, 0, ecg_data.st[ecg_data.st1-ECG_FIRST]);
  snprintf(s, sizeof(s), "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
  unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_ST1_RANGE, 160, 37, 13, UNIT_TYPE_RANGE, s);
  v = alarm_isenabled(ECG_RISKSTI+(ecg_data.st1-ECG_FIRST)) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(ECG, UNIT_TYPE_VAL_ICON, UNIT_ECG_ST1_BELL, 224, 21, v, 0);
 // unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_ST2_CAPTION, 86, 53, 17, 0, "ST");
  ids2string(chan_ids[ecg_data.st2], s);
  unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_ST2_LEAD_CAPTION, 140, 53, 13, 0, s);
  unit_item_add(ECG, UNIT_TYPE_VAL_INT, UNIT_ECG_ST2_VALUE, 176, 53, 17, 0, ecg_data.st[ecg_data.st2-ECG_FIRST]);
  snprintf(s, sizeof(s), "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
  unit_item_add(ECG, UNIT_TYPE_VAL_TEXT, UNIT_ECG_ST2_RANGE, 160, 68, 13, UNIT_TYPE_RANGE, s);
  v = alarm_isenabled(ECG_RISKSTI+(ecg_data.st2-ECG_FIRST)) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(ECG, UNIT_TYPE_VAL_ICON, UNIT_ECG_ST2_BELL, 224, 51, v, 0);
  unit_item_add(ECG, UNIT_TYPE_VAL_ICON, UNIT_ECG_HEART_IMG, 3, 40, IMAGE_BLANK, 0);
  unit_item_add(ECG, UNIT_TYPE_VAL_ICON, UNIT_ECG_PM_IMG, 3, 50, IMAGE_BLANK, 0);
  pmframe->punit[ECG]->visible = 1;

  // ECG_NEO parameters' unit
  unit_item_add(ECG_NEO, UNIT_TYPE_VAL_LABEL, UNIT_ECG_NEO_LABEL, 3, 3, MAIN_ECG_NEO, IDS_ECG, 0); // ECG Neo
  pmframe->punit[ECG_NEO]->visible = 0;
//  inpfoc_del(INPFOC_MAIN, pmframe->punit[ECG_NEO]->inpfoc_item);

  // SpO2 parameters' unit
  unit_item_add(SPO2, UNIT_TYPE_VAL_LABEL, UNIT_SPO2_LABEL, 1, 1, MAIN_SPO2, IDS_SPO2, "SpO2");
  unit_item_add(SPO2, UNIT_TYPE_VAL_TEXT, UNIT_SPO2_PULSE_CAPTION, 122, 22, 17, IDS_PULSE, 0);
  unit_item_add(SPO2, UNIT_TYPE_VAL_INT, UNIT_SPO2_PULSE, 173, 22, 27, 0, (int)UNDEF_VALUE);
  snprintf(s, sizeof(s), "%d..%d", spo2_data.hr_min, spo2_data.hr_max);
  unit_item_add(SPO2, UNIT_TYPE_VAL_TEXT, UNIT_SPO2_PULSE_RANGE, 122, 60, 13, UNIT_TYPE_RANGE, s);
  unit_item_add(SPO2, UNIT_TYPE_VAL_INT, UNIT_SPO2_SAT, 32, 12, 74, 0, (int)UNDEF_VALUE);
  snprintf(s, sizeof(s), "%d..%d", spo2_data.spo2_min, spo2_data.spo2_max);
  unit_item_add(SPO2, UNIT_TYPE_VAL_TEXT, UNIT_SPO2_SAT_RANGE, 188, 2, 13, UNIT_TYPE_RANGE, s);
  v = alarm_isenabled(SPO2_RISKSAT) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(SPO2, UNIT_TYPE_VAL_ICON, UNIT_SPO2_SAT_BELL, 159, 3, v, 0);
  v = alarm_isenabled(SPO2_RISKPULSE) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(SPO2, UNIT_TYPE_VAL_ICON, UNIT_SPO2_HR_BELL, 137, 40, v, 0);
  unit_item_add(SPO2, UNIT_TYPE_VAL_PROGRESS, UNIT_SPO2_STOLBIK, 228, 20, MAKELONG(10, 50), PBS_VERTICAL, 0);
 // unit_item_add(SPO2, UNIT_TYPE_VAL_TEXT, UNIT_SPO2_SCALE_CAPTION, 173, 52, 17, IDS_NONE, 0);
 // unit_item_add(SPO2, UNIT_TYPE_VAL_INT, UNIT_SPO2_SCALE, 194, 52, 27, 0, (int)UNDEF_VALUE);
  pmframe->punit[SPO2]->visible = 1;

  // blood pressure module measurements' data
  ids2string(IDS_NIBP, s);
  unit_item_add(NIBP, UNIT_TYPE_VAL_LABEL, UNIT_NIBP_LABEL, 1, 1, MAIN_NIBP, IDS_NIBP, s);
  unit_item_add(NIBP, UNIT_TYPE_VAL_INT, UNIT_NIBP_BP_SS, 10, 25, 30, 0, (int)UNDEF_VALUE);
  unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_BP_SEP, 67, 25, 30, 0, "/");
  unit_item_add(NIBP, UNIT_TYPE_VAL_INT, UNIT_NIBP_BP_DD, 82, 25, 30, 0, (int)UNDEF_VALUE);
  unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_BP_BRK_O, 140, 23, 30, 0, "(");
  unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_BP_BRK_C, 202, 23, 30, 0, ")");
  unit_item_add(NIBP, UNIT_TYPE_VAL_INT, UNIT_NIBP_BP_CC, 152, 25, 30, 0, (int)UNDEF_VALUE);
  unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_STIME, 110, 71, 13, 0, "- - - - - - -");
  snprintf(s, sizeof(s), "%d..%d", nibp_data.sd_min, nibp_data.sd_max);
  unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_BP_SS_RANGE, 15, 57, 13, UNIT_TYPE_RANGE, s);
  snprintf(s, sizeof(s), "%d..%d", nibp_data.dd_min, nibp_data.dd_max);
  unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_BP_DD_RANGE, 76, 57, 13, UNIT_TYPE_RANGE, s);
  snprintf(s, sizeof(s), "%d..%d", nibp_data.md_min, nibp_data.md_max);
  unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_BP_CC_RANGE, 131, 57, 13, UNIT_TYPE_RANGE, s);
  v = alarm_isenabled(NIBP_RISKSS) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(NIBP, UNIT_TYPE_VAL_ICON, UNIT_NIBP_BELL, 50, 4, v, 0);
  unit_item_add(NIBP, UNIT_TYPE_VAL_INT, UNIT_NIBP_CUFF_PRESSURE, 80, 3, 19, 0, (int)UNDEF_VALUE);
  if (nibp_data.meas_interval == NIBP_MEAS_INTERVAL_MANU)
  {
    ids = IDS_MANUALLY;
  }
  else
  {
    switch (nibp_meas_interval[nibp_data.meas_interval])
    {
      case 1:
        ids = IDS_1MIN;
        break;
      case 2:
        ids = IDS_2MIN;
        break;
      case 5:
        ids = IDS_5MIN;
        break;
      case 10:
        ids = IDS_10MIN;
        break;
      case 15:
        ids = IDS_15MIN;
        break;
      case 30:
        ids = IDS_30MIN;
        break;
      case 60:
        ids = IDS_60MIN;
        break;
      default:
       ids = IDS_UNDEF7;
    }
  }
  unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_MEAS_INTERVAL, 125, 3, 17, ids, 0);
 // unit_item_add(NIBP, UNIT_TYPE_VAL_TEXT, UNIT_NIBP_PULSE_CAPTION, 194, 56, 17, IDS_PULSE, 0);
 // unit_item_add(NIBP, UNIT_TYPE_VAL_INT, UNIT_NIBP_PULSE, 210, 71, 19, 0, (int)UNDEF_VALUE);
  pmframe->punit[NIBP]->visible = 1;

  // resp data
  unit_item_add(RESP, UNIT_TYPE_VAL_LABEL, UNIT_RESP_LABEL, 1, 1, MAIN_RESP, IDS_RESP, 0);
  unit_item_add(RESP, UNIT_TYPE_VAL_INT, UNIT_RESP_BR, 30, 8, 74, 0, (int)UNDEF_VALUE);
  snprintf(s, sizeof(s), "%d..%d", resp_data.br_min, resp_data.br_max);
  unit_item_add(RESP, UNIT_TYPE_VAL_TEXT, UNIT_RESP_BR_RANGE, 170, 2, 17, UNIT_TYPE_RANGE, s);
  unit_item_add(RESP, UNIT_TYPE_VAL_TEXT, UNIT_RESP_STATE, 160, 58, 17, IDS_UNDEF7, 0);
  unit_item_add(RESP, UNIT_TYPE_VAL_INT, UNIT_RESP_APNOE_TIME, 156, 32, 18, UNIT_TYPE_RANGE, (int)resp_data.ap_max);
  v = alarm_isenabled(RESP_RISKBR) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(RESP, UNIT_TYPE_VAL_ICON, UNIT_RESP_BR_BELL, 140, 3, v, 0);
  v = alarm_isenabled(RESP_APNOE) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(RESP, UNIT_TYPE_VAL_ICON, UNIT_RESP_APNOE_BELL, 180, 30, v, 0);
  pmframe->punit[RESP]->visible = 1;

  // T1T2 parameters' unit
  ids2string(IDS_T, s);
  unit_item_add(T1T2, UNIT_TYPE_VAL_LABEL, UNIT_T1T2_LABEL, 1, 1, MAIN_T1T2, IDS_T, s);
  unit_item_add(T1T2, UNIT_TYPE_VAL_FLOAT, UNIT_T1T2_T1, 4, 18, 27, 0, (t1t2_data.t1!=UNDEF_VALUE) ? (float)t1t2_data.t1/10 : (float)UNDEF_VALUE);
  unit_item_add(T1T2, UNIT_TYPE_VAL_FLOAT, UNIT_T1T2_T2, 4, 52, 27, 0, (t1t2_data.t2!=UNDEF_VALUE) ? (float)t1t2_data.t2/10 : (float)UNDEF_VALUE);
  unit_item_add(T1T2, UNIT_TYPE_VAL_FLOAT, UNIT_T1T2_DT, 157, 39, 27, 0, (float)UNDEF_VALUE);

  v = alarm_isenabled(T1T2_RISKT1) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(T1T2, UNIT_TYPE_VAL_ICON, UNIT_T1T2_T1_BELL, 66, 28, v, 0);
  v = alarm_isenabled(T1T2_RISKT2) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(T1T2, UNIT_TYPE_VAL_ICON, UNIT_T1T2_T2_BELL, 66, 63, v, 0);
  v = alarm_isenabled(T1T2_RISKDT) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(T1T2, UNIT_TYPE_VAL_ICON, UNIT_T1T2_DT_BELL, 201, 47, v, 0);
  snprintf(s, sizeof(s), "%.1f..%.1f", (float) (t1t2_data.t1_min) / 10.0f, (float) (t1t2_data.t1_max) / 10.0f);
  unit_item_add(T1T2, UNIT_TYPE_VAL_TEXT, UNIT_T1T2_T1_RANGE, 83, 28, 17, UNIT_TYPE_RANGE, s);
  snprintf(s, sizeof(s), "%.1f..%.1f", (float) (t1t2_data.t2_min) / 10.0f,  (float) (t1t2_data.t2_max) / 10.0f);
  unit_item_add(T1T2, UNIT_TYPE_VAL_TEXT, UNIT_T1T2_T2_RANGE, 83, 63, 17, UNIT_TYPE_RANGE, s);
  snprintf(s, sizeof(s), "%.1f", (float) (t1t2_data.dt_max) / 10.0f);
  unit_item_add(T1T2, UNIT_TYPE_VAL_TEXT, UNIT_T1T2_DT_RANGE, 221, 48, 17, UNIT_TYPE_RANGE, s);
  pmframe->punit[T1T2]->visible = 1;

  // CO2 parameters' unit
  unit_item_add(CO2, UNIT_TYPE_VAL_LABEL, UNIT_CO2_LABEL, 1, 1, MAIN_CO2, IDS_CO2, 0);
 // unit_item_add(CO2, UNIT_TYPE_VAL_TEXT, UNIT_CO2_MMHG_CAPTION, 40, 8, 15, 0, "[mmHg]");
 // unit_item_add(CO2, UNIT_TYPE_VAL_TEXT, UNIT_CO2_ETCO2_CAPTION, 5, 22, 17, 0, "EtCO2");
 // unit_item_add(CO2, UNIT_TYPE_VAL_TEXT, UNIT_CO2_BR_CAPTION, 85, 7, 17, IDS_BR, 0);
 // unit_item_add(CO2, UNIT_TYPE_VAL_TEXT, UNIT_CO2_ICO2_CAPTION, 90, 37, 17, 0, "iCO2");
  unit_item_add(CO2, UNIT_TYPE_VAL_INT, UNIT_CO2_ETCO2, 30, 8, 52, 0, (int)UNDEF_VALUE);
  snprintf(s, sizeof(s), "%d..%d", co2_data.etco2_min, co2_data.etco2_max);
  unit_item_add(CO2, UNIT_TYPE_VAL_TEXT, UNIT_CO2_ETCO2_RANGE, 70, 70, 17, UNIT_TYPE_RANGE, s);
  v = alarm_isenabled(CO2_RISKETCO2) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(CO2, UNIT_TYPE_VAL_ICON, UNIT_CO2_ETCO2_BELL, 35, 68, v, 0);

  unit_item_add(CO2, UNIT_TYPE_VAL_INT, UNIT_CO2_BR, 147, 3, 32, 0, (int)UNDEF_VALUE);
  snprintf(s, sizeof(s), "%d..%d", co2_data.br_min, co2_data.br_max);
  unit_item_add(CO2, UNIT_TYPE_VAL_TEXT, UNIT_CO2_BR_RANGE, 200, 20, 17, UNIT_TYPE_RANGE, s);
  v = alarm_isenabled(CO2_RISKBRCO2) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(CO2, UNIT_TYPE_VAL_ICON, UNIT_CO2_BR_BELL, 210, 2, v, 0);

  unit_item_add(CO2, UNIT_TYPE_VAL_INT, UNIT_CO2_ICO2, 158, 46, 17, 0, (int)UNDEF_VALUE);
  snprintf(s, sizeof(s), "%d", co2_data.ico2_max);
  unit_item_add(CO2, UNIT_TYPE_VAL_TEXT, UNIT_CO2_ICO2_RANGE, 200, 46, 17, UNIT_TYPE_RANGE, s);
  v = alarm_isenabled(CO2_RISKICO2) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(CO2, UNIT_TYPE_VAL_ICON, UNIT_CO2_ICO2_BELL, 180, 45, v, 0);

  unit_item_add(CO2, UNIT_TYPE_VAL_TEXT, UNIT_CO2_BRSTATE, 140, 69, 17, IDS_UNDEF3, 0);
  unit_item_add(CO2, UNIT_TYPE_VAL_INT, UNIT_CO2_APNOE_TIME, 218, 67, 18, UNIT_TYPE_RANGE, (int)co2_data.ap_max);
  v = alarm_isenabled(CO2_APNOE) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_item_add(CO2, UNIT_TYPE_VAL_ICON, UNIT_CO2_APNOE_BELL, 198, 67, v, 0);

  pmframe->punit[CO2]->visible = 0;

  pmframe->maximized = 0;
  pmframe->visible = 1;

  mframe_cfg_t mframe_cfg;
  if ( stconf_read(STCONF_MFRAME, &mframe_cfg, sizeof(mframe_cfg_t)) > 0 )
  {
    pmframe->visible = mframe_cfg.frame_visibility;
    pmframe->maximized = mframe_cfg.maximized;
    for (i=0; i<NUM_VIEWS; i++)
    {
      if (!pmframe->punit[i]) continue;
      if (mframe_cfg.unit_chmask & (1<<i))
      {
        pmframe->punit[i]->visible = 1;
      }
      else
      {
        pmframe->punit[i]->visible = 0;
        inpfoc_del(INPFOC_MAIN, pmframe->punit[i]->inpfoc_item);
      }
    }
  }
  else
  {
    for (i=0; i<NUM_VIEWS; i++)
    {
      if (!pmframe->punit[i]) continue;
      if (pmframe->punit[i]->visible)
      {
      }
      else
      {
        inpfoc_del(INPFOC_MAIN, pmframe->punit[i]->inpfoc_item);
      }
    }
  }

  for (i=0; i<NUM_VIEWS; i++)
  {
    if (!pmframe->punit[i]) continue;
    if (pmframe->punit[i]->visible) pmframe->unit_chmask |= (1 << i);
  }

  if (pmframe->maximized)
  {
    pmframe->x  = 0;
    pmframe->cx = DISPLAY_CX;
  }

  if (pmframe->visible)
  {
    grph_fillrect(pmframe->bkdc, 0, 0, pmframe->cx, pmframe->cy, MFRAME_BKCOLOR);
    grph_fillrect(pmframe->fgdc, 0, 0, pmframe->cx, pmframe->cy, 0x000000);
    grph_fillrect(pmframe->updc, 0, 0, pmframe->cx, pmframe->cy, 0x000000);
  }

//  mframe_on_unit_par_ranges(pmframe, lac.thsh_visible);

  switch (ecg_data.hr_src)
  {
    case HRSRC_AUTO:
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, RGB(0xff,0xff,0xff));
      break;
    case HRSRC_ECG:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[ECG]);
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[ECG]);
      break;
    case HRSRC_SPO2:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[SPO2]);
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[SPO2]);
      break;
    case HRSRC_NIBP:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[NIBP]);
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[NIBP]);
      break;
    default:
      break;
  }

  alarm_ini();
  alarm_cfg_get(&ac);

  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR_BELL, alarm_isenabled(ECG_RISKHR) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, alarm_isenabled(ECG_RISKSTI+(ecg_data.st1-ECG_FIRST)) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, alarm_isenabled(ECG_RISKSTI+(ecg_data.st2-ECG_FIRST)) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_HR_BELL, alarm_isenabled(SPO2_RISKPULSE) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT_BELL, alarm_isenabled(SPO2_RISKSAT) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BELL, alarm_isenabled(NIBP_RISKSS) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR_BELL, alarm_isenabled(RESP_RISKBR) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_APNOE_BELL, alarm_isenabled(RESP_APNOE) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2_BELL, alarm_isenabled(CO2_RISKETCO2) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2_BELL, alarm_isenabled(CO2_RISKICO2) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR_BELL, alarm_isenabled(CO2_RISKBRCO2) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_APNOE_BELL, alarm_isenabled(CO2_APNOE) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1_BELL, alarm_isenabled(T1T2_RISKT1) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2_BELL, alarm_isenabled(T1T2_RISKT2) ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT_BELL, alarm_isenabled(T1T2_RISKDT) ? IMAGE_BELL : IMAGE_NOBELL);

  if (unit_isenabled(CO2))
  {
    dio_module_cmd(PD_ID_CO2, SBDP_RST_MOD);
  }

  mframe_command(MFRAME_UNIT_PAR_RANGES, (void*)((int)(ac.thsh_visible))); // enumerate inside on_func
  mframe_command(MFRAME_ENUMERATE, NULL); // not 'on_enumerate' as params thresholds' visibility will not updated
  mframe_command(MFRAME_STARTUP, NULL); // calls only once at startup
}

void mframe_deinit(void)
{
  mframe_t * pmframe;

  pmframe = &mframe;

  pat_deini(); // need to save pat data into stconf

  mframe_cfg_save();
  unit_cfg_save();

  unit_deinit();

  inpfoc_rm(INPFOC_MAIN);

  grph_releasedc(pmframe->bkdc);
  grph_releasedc(pmframe->fgdc);
  grph_releasedc(pmframe->updc);
}

void mframe_on_resize(const int x0, const int y0, const int cx, const int cy)
{
#if 1
  mframe_t *pmframe;

  pmframe = mframe_getptr();
  assert(pmframe);

  pmframe->x    = x0;
  pmframe->y    = y0;
  pmframe->cx   = cx;
  pmframe->cy   = cy;

  if (pmframe->visible)
    mframe_on_enumerate(pmframe);
#else
  error("%s, %d - disabled for test by YK (05.02.2011)\n", __FILE__, __LINE__);
#endif
}

void mframe_maximize(mframe_t *pmframe)
{
  uframe_t * puframe = uframe_getptr();

  assert(pmframe);
  assert(puframe);

  if (!pmframe->visible) return;

  pmframe->maximized = 1;

  pmframe->x  = 0;
  pmframe->x += (puframe->visible) ? UFRAME_CX : 0;
  pmframe->cx = DISPLAY_CX - pmframe->x;
  cframe_on_hide();

  unit_align_all();

  mframe_cfg_save();
}

void mframe_normalize(mframe_t *pmframe)
{
  uframe_t * puframe = uframe_getptr();
  assert(pmframe);
  assert(puframe);


  if (!pmframe->visible) return;

  pmframe->maximized = 0;

  pmframe->x  = MFRAME_X0;
  pmframe->cx = MFRAME_CX;
  cframe_on_show();

  pmframe->visible = 1;

  unit_align_all();

  mframe_cfg_save();
}

void mframe_on_enumerate(mframe_t *pmframe)
{
  int i;

  mframe_cfg_save(); // save unit_chmask field, called from unit_ioctl

  assert(pmframe);
  if (!pmframe->visible) return;

  // first, disable all units, then enable all visible
  // to make right order of cursor
  for (i=0; i<NUM_VIEWS; i++)
  {
    unit_on_show(i, SW_HIDE);
  }
  for (i=0; i<NUM_VIEWS; i++)
  {
    unit_on_show(i, (pmframe->unit_chmask & (1 << i)) ? SW_SHOW : SW_HIDE);
  }

  unit_align_all();
}

static void mframe_on_hide(mframe_t *pmframe)
{
  pmframe->visible = 0;
  mframe_cfg_save();
}

static void mframe_on_reload(mframe_t *pmframe)
{
  assert(pmframe);

  pmframe->visible = 1;
  if (pmframe->maximized)
  {
    mframe_maximize(pmframe);
  }
  else
  {
    mframe_normalize(pmframe);
  }

  mframe_cfg_save();
}

static void mframe_on_units_alarm(mframe_t *pmframe)
{
  unit_alarm_stepit();
}

static void mframe_on_unit_change(mframe_t *pmframe, int v)
{
  unit_on_change(LOWORD(v), HIWORD(v));
}

static void mframe_on_unit_par_ranges(mframe_t * pmframe, int v)
{
  int i, j;
  unit_t * punit;

  for (i=0; i<NUM_VIEWS; i++)
  {
    punit = pmframe->punit[i];
    for (j=0; j<punit->nitems; j++)
    {
      if (punit->item[j].type.range)
        punit->item[j].visible = (v) ? 1 : 0;
    }
  }
  mframe_on_enumerate(pmframe);
}

static void mframe_on_update(mframe_t *pmframe)
{
  unit_update_data_all();
}

static void mframe_on_startup(mframe_t * pmframe)
{
  ecg_data_t ecg_data;
  resp_data_t resp_data;
 // co2_data_t co2_data;

  switch (startup_info.ecg_set & 0x3)
  {
    case 0:
      ecgm_command(ECGM_ECG_TAU_3200);
      break;
    case 1:
      ecgm_command(ECGM_ECG_TAU_320);
      break;
    case 2:
      ecgm_command(ECGM_ECG_TAU_160);
      break;
    default:
      break;
  }
  if (startup_info.ecg_set & (1<<6))
  {
   // ecgm_command(...); // breath scan off
  }
  ecgm_command((startup_info.ecg_set & (1<<5)) ? ECGM_RESP_CHAN_RF : ECGM_RESP_CHAN_RL); // breath chan RL : RF

#if 0
  ecgm_command(ECGM_RESP_TAU_320); // make a soft reset of RESP
#endif
  resp_soft_hw_reset();

  unit_get_data(ECG, &ecg_data);
  dio_module_cmd(PD_ID_ECS, ECS_JPOINT_SHIFT, ecg_data.j_shift);
  dio_module_cmd(PD_ID_ECS, ECS_PM_NORESP_MS, ecg_data.pmnr_ms);

  unit_get_data(RESP, &resp_data);
  dio_module_cmd(PD_ID_ECS, ECS_RESP_APNOE_S, resp_data.ap_max);
  dio_module_cmd(PD_ID_ECS, ECS_RESPCALC_RESET);

  // clear NiBP alarms
  alarm_clr(NIBP_RISKSS);
  alarm_clr(NIBP_RISKDD);
  alarm_clr(NIBP_RISKCC);
  alarm_clr(NIBP_RISKMEASTIME);
  alarm_clr(NIBP_WEAKSIGNAL);
  alarm_clr(NIBP_ERRSIGNAL);
  alarm_clr(NIBP_EXCRETRYCOUNT);
  alarm_clr(NIBP_PNEUMBLOCK);
  alarm_clr(NIBP_INFLTIMEOUT);
  alarm_clr(NIBP_INVPOWER);

  alarm_clr(RESP_RISKBR);

  ecgm_command((ecg_data.num_leads == 5) ? ECGM_N_TO_LINE: ECGM_N_TO_GND);

#if defined (RESPCALC_COLIBRI)
  sched_start(SCHED_ANY, 4*1000, respcalc_reset, SCHED_DO_ONCE);
#endif
}

static void mframe_on_set_startup_cfg(mframe_t * pmframe, void * arg)
{
  startup_info_t * psi = arg;
  assert(psi);

  memcpy(&startup_info, psi, sizeof(startup_info_t));
}

void mframe_process_alarms(void)
{
 // int i;
 // for (i=0; i<NUM_VIEWS; i++)
 // {
 //   mframe_command(MFRAME_UNIT_ALARM, (void*)i);
 // }
  mframe_command(MFRAME_UNITS_ALARM, NULL);
}

static void mframe_bitblt(int x, int y, int cx, int cy)
{
  mframe_t * pmframe;
  dc_list_t dclist;

  pmframe = &mframe;

  dclist.xres = ((PGDC)(pmframe->bkdc))->xres;
  dclist.yres = ((PGDC)(pmframe->bkdc))->yres;
  dclist.p_bk = (unsigned char*)((PGDC)(pmframe->bkdc))->addr;
  dclist.p_fg = (unsigned char*)((PGDC)(pmframe->fgdc))->addr;
  dclist.p_up = (unsigned char*)((PGDC)(pmframe->updc))->addr;

//  if (pmframe->x+x + cx >= pmframe->cx) cx = pmframe->cx - (pmframe->x+x) -1;
//  if (pmframe->y+y + cy >= pmframe->cy) cy = pmframe->cy - (pmframe->y+y) -1;

//printf("1: %d %d %d %d %d %d %d %d \n", pmframe->x+x, pmframe->y+y, cx, cy, x, y, pmframe->cx, pmframe->cy);

 // if (pmframe->x+x + cx >= DISPLAY_CX) cx = DISPLAY_CX - (pmframe->x+x);
  if (pmframe->x+x >= DISPLAY_CX) return;

  if (x + cx >= pmframe->cx) cx = pmframe->cx-x;
  if (y + cy >= pmframe->cy) cy = pmframe->cy-y;

//printf("2: %d %d %d %d %d %d \n", pmframe->x+x, pmframe->y+y, cx, cy, x, y);

  dview_bitblt(pmframe->x+x, pmframe->y+y, cx, cy, &dclist, x, y);
}

void mframe_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case MFRAME_RESIZE:
      dview_command(MFRAME, cmd, sizeof(rect_t), arg);
      break;
    case MFRAME_SET_STARTUP_CFG:
      dview_command(MFRAME, cmd, sizeof(startup_info_t), arg);
      break;
    default:
      dview_command(MFRAME, cmd, 0, arg);
      break;
  }
}

void mframe_on_command(int cmd, void * arg)
{
  mframe_t * pmframe;
  pmframe = &mframe;

  switch (cmd)
  {
    case MFRAME_MAXIMIZE:
      mframe_maximize(pmframe);
      break;
    case MFRAME_NORMALIZE:
      mframe_normalize(pmframe);
      break;
    case MFRAME_HIDE:
      mframe_on_hide(pmframe);
      break;
    case MFRAME_RESIZE:
      mframe_on_resize(((rect_t*)arg)->x0, ((rect_t*)arg)->y0, ((rect_t*)arg)->x1-((rect_t*)arg)->x0, ((rect_t*)arg)->y1-((rect_t*)arg)->y0);
      break;
    case MFRAME_ENUMERATE:
      mframe_on_enumerate(pmframe);
      break;
    case MFRAME_UNITS_ALARM:
      mframe_on_units_alarm(pmframe);
      break;
    case MFRAME_UNIT_CHANGE:
      mframe_on_unit_change(pmframe, *((int*)arg));
      break;
    case MFRAME_RELOAD:
      mframe_on_reload(pmframe);
      break;
    case MFRAME_UNIT_PAR_RANGES:
      mframe_on_unit_par_ranges(pmframe, *((int*)arg));
      break;
    case MFRAME_UPDATE:
      mframe_on_update(pmframe);
      break;
    case MFRAME_STARTUP:
      mframe_on_startup(pmframe);
      break;
    case MFRAME_SET_STARTUP_CFG:
      mframe_on_set_startup_cfg(pmframe, (void*)arg);
      break;
    default:
      error("%s: unknown cmd %d\n", __FUNCTION__, cmd);
      break;
  }
}

int mframe_ismaximized(void)
{
  mframe_t * pmframe = mframe_getptr();
  assert(pmframe);
  return pmframe->maximized;
}

mframe_t * mframe_getptr(void)
{
  return &mframe;
}
