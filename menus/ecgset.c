/*! \file ecgset.c
 *  \brief ECG settings popup frame
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "grph.h"
#include "alarm.h"
#include "mframe.h"
#include "unit.h"
#include "uframe.h"
#include "cframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "ecgcalc.h"
#include "respcalc.h"
#include "ecgset.h"
#include "dio.h"
#include "ecgm.h"
#include "tprn.h"
#include "mainmenu.h"

static void ecgset_none_func(void*);
static void ecgset_hralr_func(void*);
static void ecgset_hrl_func(void*);
static void ecgset_hrh_func(void*);
static void ecgset_numchans_func(void*);
static void ecgset_speed_func(void*);
static void ecgset_stsetproc_func(void*);
static void ecgset_arrhsetproc_func(void*);
static void ecgset_hrsrc_func(void*);
static void ecgset_dhf_func(void*);
static void ecgset_qrssigvol_func(void*);
static void ecgset_cabletype_func(void*);
static void ecgset_exit_func(void*);

inpfoc_funclist_t inpfoc_ecgset_funclist[ECGSET_NUM_ITEMS+1] = 
{
  { ECGSET_NONE,       ecgset_none_func         },
  { ECGSET_HRL,        ecgset_hrl_func          },
  { ECGSET_HRH,        ecgset_hrh_func          },
  { ECGSET_HRALR,      ecgset_hralr_func        },
  { ECGSET_EXIT,       ecgset_exit_func         },
  { ECGSET_QRSSIGVOL,  ecgset_qrssigvol_func    },
  { ECGSET_HRCALCSRC,  ecgset_none_func         },
  { ECGSET_NUMCHANS,   ecgset_numchans_func     },
  { ECGSET_SPEED,      ecgset_speed_func        },
  { ECGSET_STCALCPROC, ecgset_none_func         },
  { ECGSET_STSETPROC,  ecgset_stsetproc_func    },
  { ECGSET_ARRHPROC,   ecgset_arrhsetproc_func  },
  { ECGSET_HRSRC,      ecgset_hrsrc_func        },
  { ECGSET_DHF,        ecgset_dhf_func          },
  { ECGSET_CABLETYPE,  ecgset_cabletype_func    },
  { -1       ,         ecgset_none_func         }, // -1 must be last
};

static void ecgset_none_func(void * parg)
{

}

static void ecgset_hralr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_ECGSET, ECGSET_HRALR));
  alarm_on_off(ECG_RISKHR, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR_BELL, v);
}

static void ecgset_hrl_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[10];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_ECGSET, ECGSET_HRL);
  ddh = ecg_data.hr_max;
  ddl = ecg_data.hr_min;

  ddl += pcmd->delta*1;
  while (ddl < 30) ddl += (300-30);
  ddl %= 300;

  if (ddl <= ddh)
  {
    ecg_data.hr_min = ddl;
    unit_set_data(ECG, &ecg_data);
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", ecg_data.hr_min, ecg_data.hr_max);
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR_RANGE, s);
  }
}

static void ecgset_hrh_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[10];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_ECGSET, ECGSET_HRH);
  ddh = ecg_data.hr_max;
  ddl = ecg_data.hr_min;

  ddh += pcmd->delta*1;
  while (ddh < 30) ddh += (300-30);
  ddh %= 300;

  if (ddh >= ddl)
  {
    ecg_data.hr_max = ddh;
    unit_set_data(ECG, &ecg_data);
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", ecg_data.hr_min, ecg_data.hr_max);
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR_RANGE, s);
  }
}

static void ecgset_numchans_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int i, n;
  cframe_t * pcframe;
  char s[20];
  ecg_data_t ecg_data;
  int maxnum_ecg_channels;

  pcframe = cframe_getptr();
  assert(pcframe);

  unit_get_data(ECG, &ecg_data);

  n = 0;
  for (i=0; i<NUM_ECG; i++)
  {
    n += (pcframe->chanview[ECG_1+i].visible) ? 1 : 0;
  }

  switch (ecg_data.num_leads)
  {
    case 1:
      maxnum_ecg_channels = 1;
      break;
    case 3:
      maxnum_ecg_channels = 6;
      break;
    case 5:
      maxnum_ecg_channels = NUM_ECG;
      break;
    default:
      maxnum_ecg_channels = 0;
      break;
  }

  n += pcmd->delta;
  while (n < 0)
    n += (maxnum_ecg_channels+1);
  while (n > maxnum_ecg_channels)
    n -= (maxnum_ecg_channels+1);

  for (i=0; i<NUM_ECG; i++)
  {
    cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(ECG_1+i, (i<n)?1:0));
  }

  snprintf(s, sizeof(s), "%d", n);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_NUMCHANS), s);
}

static void ecgset_speed_func(void * parg)
{
  cframe_command(CFRAME_CHANSPD, (void*)MAKELONG(ECG, ((inpfoc_cmd_t*)parg)->delta));
  // string caption will be updated in cframe_on_chanspd
}

static void ecgset_stsetproc_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_STSET);
}

static void ecgset_arrhsetproc_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_ARRHSET);
}

static void ecgset_qrssigvol_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int v;
  char s[20];
  alarm_cfg_t lac;

  alarm_cfg_get(&lac);

  pit = inpfoc_find(INPFOC_ECGSET, ECGSET_QRSSIGVOL);
  v = lac.qrs_volume;

  v += pcmd->delta*1;
  while (v > 5) v -= (5+1);
  while (v < 0) v += (5+1);

  lac.qrs_volume = v;

  sprintf(s, "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  alarm_cfg_set(&lac);
  alarm_cfg_save();
}

static void ecgset_hrsrc_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[200];
  int v;
  unsigned short ids;
  ecg_data_t ecg_data;
  spo2_data_t spo2_data;
  nibp_data_t nibp_data;

  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    error("%s: reading ecg data\n", __FUNCTION__);
  }
  if (unit_get_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }
  if (unit_get_data(NIBP, &nibp_data) <= 0)
  {
    error("%s: reading nibp data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_ECGSET, ECGSET_HRSRC);
  v = ecg_data.hr_src;

  v += pcmd->delta*1;
  while (v >= HRSRC_NUM) v -= HRSRC_NUM;
  while (v < 0) v += HRSRC_NUM;

  ecg_data.hr_src = v;
  unit_set_data(ECG, &ecg_data);
  switch(v)
  {
    case HRSRC_AUTO:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[ECG]);
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, RGB(0xff,0xff,0xff));
      ids = IDS_AUTO;
      if (ecg_data.hr_src_curr == ECG)
        unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int)ecg_data.hr);
      if (ecg_data.hr_src_curr == SPO2)
        unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int)spo2_data.hr);
      if (ecg_data.hr_src_curr == NIBP)
        unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int)nibp_data.hr);
      break;
    case HRSRC_ECG:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[ECG]);
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[ECG]);
      ids = IDS_ECG;
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int)ecg_data.hr);
      break;
    case HRSRC_SPO2:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[SPO2]);
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[SPO2]);
      ids = IDS_SPO2;
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int)spo2_data.hr);
      break;
    case HRSRC_NIBP:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[NIBP]);
     // unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[NIBP]);
      ids = IDS_NIBP;
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int)nibp_data.hr);
      break;
    default:
      ids = IDS_UNDEF3;
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int)UNDEF_VALUE);
      break;
  }
  ids2string(ids, s);
  if (pit) inpfoc_wnd_setcaption(pit, s);
 // unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HRSRC, ids);

//#if defined (RESPCALC_CUBIC_SPLINE)
//  respcalc_reset();
//#endif

  unit_cfg_save();
}

static void ecgset_dhf_func(void * parg)
{
#if 0
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  dio_module_cmd(PD_ID_ECG, (ecg_data.set_bits.dhf) ? ECGM_DHF_OFF : ECGM_DHF_ON);

  // when dhf changes window updates automatically
  // no need calling inpfoc_wnd_setcaption
#endif
}

static void ecgset_cabletype_func(void * parg)
{
  ecg_data_t ecg_data;
  char s[200];
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;

  unit_get_data(ECG, &ecg_data);

  ecg_data.num_leads += pcmd->delta*2; // possible values: 3, 5
 // if ((signed char)ecg_data.num_leads < 1) ecg_data.num_leads += 5+1;
  while ((signed char)ecg_data.num_leads < 3) ecg_data.num_leads += (5-3+1)+1;
  while ((signed char)ecg_data.num_leads > 5) ecg_data.num_leads -= (5-3+1)+1;

  switch (ecg_data.num_leads)
  {
    case 1:
      ids2string(IDS_3LEADS_1CHAN, s);
      break;
    case 3:
      ids2string(IDS_3LEADS, s);
      break;
    case 5:
      ids2string(IDS_5LEADS, s);
      break;
    default:
      ids2string(IDS_UNDEF3, s);
      break;
  }

  ecgm_command((ecg_data.num_leads == 5) ? ECGM_N_TO_LINE: ECGM_N_TO_GND);

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_CABLETYPE), s);

  if (ecg_data.num_leads == 1)
  {
    int i, n;
    cframe_t * pcframe;

    pcframe = cframe_getptr();
    assert(pcframe);

    n = 0;
    for (i=0; i<NUM_ECG; i++)
    {
      n += (pcframe->chanview[ECG_1+i].visible) ? 1 : 0;
    }

    n -= 1;
    for (i=ECG_LAST; i>=ECG_FIRST && n>0; i--)
    {
      if (pcframe->chanview[i].visible)
      {
        cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(i, 0));
        n --;
      }
    }
    n += 1;

    snprintf(s, sizeof(s), "%d", n);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_NUMCHANS), s);

    ecg_data.st1 = ECG_I;
    ids2string(chan_ids[ecg_data.st1], s);
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_LEAD_CAPTION, s);

    ecg_data.st2 = ECG_II;
    ids2string(chan_ids[ecg_data.st2], s);
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_LEAD_CAPTION, s);

    // switch Resp to RF channel
    ecgm_command(ECGM_RESP_CHAN_RF);
  } // 3-lead ecg, 1 channel

  if (ecg_data.num_leads == 3)
  {
    int i, n;
    cframe_t * pcframe;

    pcframe = cframe_getptr();
    assert(pcframe);

    n = 0;
    for (i=0; i<NUM_ECG; i++)
    {
      n += (pcframe->chanview[ECG_1+i].visible) ? 1 : 0;
    }

    n -= 3;
    for (i=ECG_LAST; i>=ECG_FIRST && n>0; i--)
    {
      if (pcframe->chanview[i].visible)
      {
        cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(i, 0));
        n --;
      }
    }
    n += 3;

    snprintf(s, sizeof(s), "%d", n);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_NUMCHANS), s);

    if (ecg_data.st1 > ECG_III)
    {
      ecg_data.st1 = ECG_I;
      for (i=ECG_FIRST; i<=ECG_III; i++)
      {
        if (i != ecg_data.st2) break;
      }
      ecg_data.st1 = i;
      ids2string(chan_ids[ecg_data.st1], s);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_LEAD_CAPTION, s);
    }
    if (ecg_data.st2 > ECG_III)
    {
      ecg_data.st2 = ECG_I;
      for (i=ECG_FIRST; i<=ECG_III; i++)
      {
        if (i != ecg_data.st1) break;
      }
      ecg_data.st2 = i;
      ids2string(chan_ids[ecg_data.st2], s);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_LEAD_CAPTION, s);
    }
  } // 3-lead ecg

  cframe_command(CFRAME_ECGLEAD_DEFAULT, NULL);

  tprn_cfg_t tprn_cfg;
  tprn_cfg_get(&tprn_cfg);
  while ( (tprn_cfg.c[0] == tprn_cfg.c[1]) || tprn_cfg.c[0] == NIBP || tprn_cfg.c[0] == T1T2 || tprn_cfg.c[0] == ECG_NEO ||
  ( (tprn_cfg.c[0] == ECG_I || tprn_cfg.c[0] == ECG_III || tprn_cfg.c[0] == ECG_aVR || tprn_cfg.c[0] == ECG_aVL || tprn_cfg.c[0] == ECG_aVF || tprn_cfg.c[0] == ECG_V) && ecg_data.num_leads == 1 ) ||
  ( tprn_cfg.c[0] == ECG_V && ecg_data.num_leads == 3 )
        )

  {
    tprn_cfg.c[0] += 1;
    while ((signed char)tprn_cfg.c[0] >= NUM_VIEWS)  tprn_cfg.c[0] -= NUM_VIEWS;
    while ((signed char)tprn_cfg.c[0] < 0)           tprn_cfg.c[0] += NUM_VIEWS;
  }

  while ( (tprn_cfg.c[0] == tprn_cfg.c[1]) || tprn_cfg.c[1] == NIBP || tprn_cfg.c[1] == T1T2 || tprn_cfg.c[1] == ECG_NEO ||
  ( (tprn_cfg.c[1] == ECG_I || tprn_cfg.c[1] == ECG_III || tprn_cfg.c[1] == ECG_aVR || tprn_cfg.c[1] == ECG_aVL || tprn_cfg.c[1] == ECG_aVF || tprn_cfg.c[1] == ECG_V) && ecg_data.num_leads == 1 ) ||
  ( tprn_cfg.c[1] == ECG_V && ecg_data.num_leads == 3 )
        )

  {
    tprn_cfg.c[1] += 1;
    while ((signed char)tprn_cfg.c[1] >= NUM_VIEWS)  tprn_cfg.c[1] -= NUM_VIEWS;
    while ((signed char)tprn_cfg.c[1] < 0)           tprn_cfg.c[1] += NUM_VIEWS;
  }

  tprn_cfg_set(&tprn_cfg);

  unit_set_data(ECG, &ecg_data);
}

static void ecgset_exit_func(void * parg)
{
  uframe_command(UFRAME_DESTROY, NULL);

  // menu_ecgset_closeproc will be called by UFRAME
}

void menu_ecgset_openproc(void)
{
  ecg_data_t ecg_data;
  alarm_cfg_t ac;
  cframe_t * pcframe;
  char s[200], s2[200];
  int i, n, ids;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }
  alarm_cfg_get(&ac);

  pcframe = cframe_getptr();
  assert(pcframe);

  inpfoc_wnd_check(inpfoc_find(INPFOC_ECGSET, ECGSET_HRALR), alarm_isenabled(ECG_RISKHR));

  snprintf(s, sizeof(s), "%d", ecg_data.hr_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_HRH), s);
  snprintf(s, sizeof(s), "%d", ecg_data.hr_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_HRL), s);

  n = 0;
  for (i=0; i<NUM_ECG; i++)
  {
    n += (pcframe->chanview[ECG_1+i].visible) ? 1 : 0;
  }
  snprintf(s, sizeof(s), "%d", n);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_NUMCHANS), s);

#if 0
  ids2string((ecg_data.set_bits.dhf) ? IDS_ON : IDS_OFF, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_DHF), s);
#endif

  snprintf(s, sizeof(s), "%d", ac.qrs_volume);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_QRSSIGVOL), s);

  snprintf(s, sizeof(s), "%d", ecg_data.hr_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_HRL), s);

  switch (ecg_data.num_leads)
  {
    case 1:
      ids2string(IDS_3LEADS_1CHAN, s);
      break;
    case 3:
      ids2string(IDS_3LEADS, s);
      break;
    case 5:
      ids2string(IDS_5LEADS, s);
      break;
    default:
      ids2string(IDS_UNDEF3, s);
      break;
  }
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_CABLETYPE), s);

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
  ids2string(ids, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_HRSRC), s);

  inpfoc_disable(INPFOC_ECGSET, ECGSET_PRNALR);
  inpfoc_disable(INPFOC_ECGSET, ECGSET_STCALCPROC);

  if (pcframe->chanview[ECG].chandata.mmps == 6.25f)
    sprintf(s, "%.2f ", (float)pcframe->chanview[ECG].chandata.mmps);
  else
  if (pcframe->chanview[ECG].chandata.mmps == 12.5f)
    sprintf(s, "%.1f ", (float)pcframe->chanview[ECG].chandata.mmps);
  else
    sprintf(s, "%d ", (int)pcframe->chanview[ECG].chandata.mmps);
  ids2string(IDS_MMPS, s2);
  strcat(s, " ");
  strcat(s, s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ECGSET, ECGSET_SPEED), s);

  inpfoc_set(INPFOC_ECGSET, ECGSET_EXIT);
}

void menu_ecgset_closeproc(void)
{
  unit_cfg_save();
}
