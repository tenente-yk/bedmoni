#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "grph.h"
#include "dproc.h"
#include "uframe.h"
#include "cframe.h"
#include "mframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "sched.h"
#include "pat.h"
#include "nibp.h"
#include "nibpset.h"

static void nibpset_none_func(void*);
static void nibpset_alr_func(void*);
static void nibpset_sdh_func(void*);
static void nibpset_sdl_func(void*);
static void nibpset_ddh_func(void*);
static void nibpset_ddl_func(void*);
static void nibpset_mdh_func(void*);
static void nibpset_mdl_func(void*);
static void nibpset_mode_func(void*);
static void nibpset_infl_func(void*);
static void nibpset_calib_func(void*);
static void nibpset_verif_func(void*);
static void nibpset_leak_func(void*);
static void nibpset_exit_func(void*);

inpfoc_funclist_t inpfoc_nibpset_funclist[NIBPSET_NUM_ITEMS+1] = 
{
  { NIBPSET_NONE,   nibpset_none_func         },
  { NIBPSET_ALR,    nibpset_alr_func          },
  { NIBPSET_SDH,    nibpset_sdh_func          },
  { NIBPSET_SDL,    nibpset_sdl_func          },
  { NIBPSET_DDH,    nibpset_ddh_func          },
  { NIBPSET_DDL,    nibpset_ddl_func          },
  { NIBPSET_MDH,    nibpset_mdh_func          },
  { NIBPSET_MDL,    nibpset_mdl_func          },
  { NIBPSET_MODE,   nibpset_mode_func         },
  { NIBPSET_INFL,   nibpset_infl_func         },
  { NIBPSET_CALIB,  nibpset_calib_func        },
  { NIBPSET_VERIF,  nibpset_verif_func        },
  { NIBPSET_LEAK,   nibpset_leak_func         },
  { NIBPSET_EXIT,   nibpset_exit_func         },
  { -1       ,      nibpset_none_func         }, // -1 must be last
};

static void nibpset_none_func(void * parg)
{

}

static void nibpset_alr_func(void * parg)
{
  int v;
  alarm_on_off(NIBP_RISKSS, !alarm_isenabled(NIBP_RISKSS));
  alarm_on_off(NIBP_RISKDD, !alarm_isenabled(NIBP_RISKDD));
  alarm_on_off(NIBP_RISKCC, !alarm_isenabled(NIBP_RISKCC));

  v = alarm_isenabled(NIBP_RISKSS) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BELL, v);
}

static void nibpset_sdh_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  nibp_data_t nibp_data;
  char s[20];
  pat_pars_t lpp;

  pat_get(&lpp);
  unit_get_data(NIBP, &nibp_data);
  ddh = nibp_data.sd_max;
  ddl = nibp_data.sd_min;

  pit = inpfoc_find(INPFOC_NIBPSET, NIBPSET_SDH);
  assert(pit);

  ddh += pcmd->delta * 1;

  switch (lpp.type)
  {
    case CHILD:
      vu = 160;
      vl = 40;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      vu = 130;
      vl = 40;
      break;
#endif
    case ADULT:
    default:
      vu = 260;
      vl = 40;
      break;
  }

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    nibp_data.sd_max = ddh;
    snprintf(s, sizeof(s), "%d..%d", nibp_data.sd_min, nibp_data.sd_max);
#if 0
    if (nibp_data.sd != UNDEF_VALUE)
    {
      nibpv_t nv;
      unsigned char st;
      int v;
      alarm_set_clr(NIBP_RISKSS, (nibp_data.sd > nibp_data.sd_max || nibp_data.sd < nibp_data.sd_min) ? 1 : 0);
      st = 0;
      st |= (nibp_data.sd > nibp_data.sd_max) ? TR_ST_U : 0;
      st |= (nibp_data.sd < nibp_data.sd_min) ? TR_ST_L : 0;
      nv.ss = nibp_data.sd;
      nv.dd = nibp_data.dd;
      nv.mm = nibp_data.md;
      v = *((int*)&nv);
      dproc_add_trv(PARAM_NIBP, v, st);
    }
#endif
    unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_SS_RANGE, s);
    unit_set_data(NIBP, &nibp_data);
  }
}

static void nibpset_sdl_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  nibp_data_t nibp_data;
  char s[20];
  pat_pars_t lpp;

  pat_get(&lpp);
  unit_get_data(NIBP, &nibp_data);
  ddh = nibp_data.sd_max;
  ddl = nibp_data.sd_min;

  pit = inpfoc_find(INPFOC_NIBPSET, NIBPSET_SDL);
  assert(pit);

  ddl += pcmd->delta * 1;

  switch (lpp.type)
  {
    case CHILD:
      vu = 160;
      vl = 40;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      vu = 130;
      vl = 40;
      break;
#endif
    case ADULT:
    default:
      vu = 260;
      vl = 40;
      break;
  }

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    nibp_data.sd_min = ddl;
    snprintf(s, sizeof(s), "%d..%d", nibp_data.sd_min, nibp_data.sd_max);
#if 0
    if (nibp_data.sd != UNDEF_VALUE)
    {
      nibpv_t nv;
      unsigned char st;
      int v;
      alarm_set_clr(NIBP_RISKSS, (nibp_data.sd > nibp_data.sd_max || nibp_data.sd < nibp_data.sd_min) ? 1 : 0);
      st = 0;
      st |= (nibp_data.sd > nibp_data.sd_max) ? TR_ST_U : 0;
      st |= (nibp_data.sd < nibp_data.sd_min) ? TR_ST_L : 0;
      nv.ss = nibp_data.sd;
      nv.dd = nibp_data.dd;
      nv.mm = nibp_data.md;
      v = *((int*)&nv);
      dproc_add_trv(PARAM_NIBP, v, st);
    }
#endif
    unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_SS_RANGE, s);
    unit_set_data(NIBP, &nibp_data);
  }
}

static void nibpset_ddh_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  nibp_data_t nibp_data;
  char s[20];
  pat_pars_t lpp;

  pat_get(&lpp);
  unit_get_data(NIBP, &nibp_data);
  ddh = nibp_data.dd_max;
  ddl = nibp_data.dd_min;

  pit = inpfoc_find(INPFOC_NIBPSET, NIBPSET_DDH);
  assert(pit);

  ddh += pcmd->delta * 1;

  switch (lpp.type)
  {
    case CHILD:
      vu = 100;
      vl = 20;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      vu = 120;
      vl = 20;
      break;
#endif
    case ADULT:
    default:
      vu = 200;
      vl = 20;
      break;
  }

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    nibp_data.dd_max = ddh;
    snprintf(s, sizeof(s), "%d..%d", nibp_data.dd_min, nibp_data.dd_max);
#if 0
    if (nibp_data.dd != UNDEF_VALUE)
    {
      nibpv_t nv;
      unsigned char st;
      int v;
      alarm_set_clr(NIBP_RISKDD, (nibp_data.dd > nibp_data.dd_max || nibp_data.dd < nibp_data.dd_min) ? 1 : 0);
      st = 0;
      st |= (nibp_data.dd > nibp_data.dd_max) ? TR_ST_U : 0;
      st |= (nibp_data.dd < nibp_data.dd_min) ? TR_ST_L : 0;
      nv.ss = nibp_data.sd;
      nv.dd = nibp_data.dd;
      nv.mm = nibp_data.md;
      v = *((int*)&nv);
      dproc_add_trv(PARAM_NIBP, v, st);
    }
#endif
    unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_DD_RANGE, s);
    unit_set_data(NIBP, &nibp_data);
  }
}

static void nibpset_ddl_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  nibp_data_t nibp_data;
  char s[20];
  pat_pars_t lpp;

  pat_get(&lpp);
  unit_get_data(NIBP, &nibp_data);
  ddh = nibp_data.dd_max;
  ddl = nibp_data.dd_min;

  pit = inpfoc_find(INPFOC_NIBPSET, NIBPSET_DDL);
  assert(pit);

  ddl += pcmd->delta * 1;

  switch (lpp.type)
  {
    case CHILD:
      vu = 100;
      vl = 20;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      vu = 120;
      vl = 20;
      break;
#endif
    case ADULT:
    default:
      vu = 200;
      vl = 20;
      break;
  }

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    nibp_data.dd_min = ddl;
    snprintf(s, sizeof(s), "%d..%d", nibp_data.dd_min, nibp_data.dd_max);
#if 0
    if (nibp_data.dd != UNDEF_VALUE)
    {
      nibpv_t nv;
      unsigned char st;
      int v;
      alarm_set_clr(NIBP_RISKDD, (nibp_data.dd > nibp_data.dd_max || nibp_data.dd < nibp_data.dd_min) ? 1 : 0);
      st = 0;
      st |= (nibp_data.dd > nibp_data.dd_max) ? TR_ST_U : 0;
      st |= (nibp_data.dd < nibp_data.dd_min) ? TR_ST_L : 0;
      nv.ss = nibp_data.sd;
      nv.dd = nibp_data.dd;
      nv.mm = nibp_data.md;
      v = *((int*)&nv);
      dproc_add_trv(PARAM_NIBP, v, st);
    }
#endif
    unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_DD_RANGE, s);
    unit_set_data(NIBP, &nibp_data);
  }
}

static void nibpset_mdh_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  nibp_data_t nibp_data;
  char s[20];
  pat_pars_t lpp;

  pat_get(&lpp);
  unit_get_data(NIBP, &nibp_data);
  ddh = nibp_data.md_max;
  ddl = nibp_data.md_min;

  pit = inpfoc_find(INPFOC_NIBPSET, NIBPSET_MDH);
  assert(pit);

  ddh += pcmd->delta * 1;

  switch (lpp.type)
  {
    case CHILD:
      vu = 110;
      vl = 26;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      vu = 133;
      vl = 26;
      break;
#endif
    case ADULT:
    default:
      vu = 220;
      vl = 26;
      break;
  }

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    nibp_data.md_max = ddh;
    snprintf(s, sizeof(s), "%d..%d", nibp_data.md_min, nibp_data.md_max);
#if 0
    if (nibp_data.md != UNDEF_VALUE)
    {
      nibpv_t nv;
      unsigned char st;
      int v;
      alarm_set_clr(NIBP_RISKCC, (nibp_data.md > nibp_data.md_max || nibp_data.md < nibp_data.md_min) ? 1 : 0);
      st = 0;
      st |= (nibp_data.md > nibp_data.md_max) ? TR_ST_U : 0;
      st |= (nibp_data.md < nibp_data.md_min) ? TR_ST_L : 0;
      nv.ss = nibp_data.sd;
      nv.dd = nibp_data.dd;
      nv.mm = nibp_data.md;
      *((int*)&nv);
      dproc_add_trv(PARAM_NIBP, v, st);
    }
#endif
    unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_CC_RANGE, s);
    unit_set_data(NIBP, &nibp_data);
  }
}

static void nibpset_mdl_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  nibp_data_t nibp_data;
  char s[20];
  pat_pars_t lpp;

  pat_get(&lpp);
  unit_get_data(NIBP, &nibp_data);
  ddh = nibp_data.md_max;
  ddl = nibp_data.md_min;

  pit = inpfoc_find(INPFOC_NIBPSET, NIBPSET_MDL);
  assert(pit);

  ddl += pcmd->delta * 1;

  switch (lpp.type)
  {
    case CHILD:
      vu = 110;
      vl = 26;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      vu = 133;
      vl = 26;
      break;
#endif
    case ADULT:
    default:
      vu = 220;
      vl = 26;
      break;
  }

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    nibp_data.md_min = ddl;
    snprintf(s, sizeof(s), "%d..%d", nibp_data.md_min, nibp_data.md_max);
#if 0
    if (nibp_data.md != UNDEF_VALUE)
    {
      nibpv_t nv;
      unsigned char st;
      int v;
      alarm_set_clr(NIBP_RISKCC, (nibp_data.md > nibp_data.md_max || nibp_data.md < nibp_data.md_min) ? 1 : 0);
      st = 0;
      st |= (nibp_data.md > nibp_data.md_max) ? TR_ST_U : 0;
      st |= (nibp_data.md < nibp_data.md_min) ? TR_ST_L : 0;
      nv.ss = nibp_data.sd;
      nv.dd = nibp_data.dd;
      nv.mm = nibp_data.md;
      *((int*)&nv);
      dproc_add_trv(PARAM_NIBP, v, st);
    }
#endif
    unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_CC_RANGE, s);
    unit_set_data(NIBP, &nibp_data);
  }
}

static void nibpset_mode_func(void * parg)
{
  inpfoc_item_t *pit;
  nibp_data_t nibp_data;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[128];
  unsigned short ids;

  pit = inpfoc_find(INPFOC_NIBPSET, NIBPSET_MODE);

  assert(pit);

  unit_get_data(NIBP, &nibp_data);

  nibp_data.meas_interval += 1*pcmd->delta;

  while (nibp_data.meas_interval < 0) nibp_data.meas_interval += NIBP_NUM_MEAS_INTERVALS;
  while (nibp_data.meas_interval >= NIBP_NUM_MEAS_INTERVALS) nibp_data.meas_interval -= NIBP_NUM_MEAS_INTERVALS;

  if (nibp_data.meas_interval == NIBP_MEAS_INTERVAL_MANU)
  {
    ids = IDS_MANUALLY;
    sched_stop(SCHED_NIBP);
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
    sched_start(SCHED_NIBP, nibp_meas_interval[nibp_data.meas_interval]*60*1000, nibp_do_bp, SCHED_NORMAL);
  }

  ids2string(ids, s);
  inpfoc_wnd_setcaption(pit, s);

  unit_set_data(NIBP, &nibp_data);

  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_MEAS_INTERVAL, ids);

  unit_cfg_save();
}

static void nibpset_infl_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd, vu, vl;
  char s[10];
  nibp_data_t nibp_data;
  pat_pars_t lpp;

  pat_get(&lpp);
  unit_get_data(NIBP, &nibp_data);
  pit = inpfoc_find(INPFOC_NIBPSET, NIBPSET_INFL);

  dd = nibp_data.infl;

  dd += pcmd->delta * 1;

  switch (lpp.type)
  {
    case CHILD:
      vu = 170;
      vl = 80;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      vu = 140;
      vl = 60;
      break;
#endif
    case ADULT:
    default:
      vu = 280;
      vl = 120;
      break;
  }

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  nibp_data.infl = dd;
  unit_set_data(NIBP, &nibp_data);

  snprintf(s, sizeof(s), "%d", dd);
  inpfoc_wnd_setcaption(pit, s);
}

static void nibpset_calib_func(void * parg)
{
  nibp_calib();
  uframe_command(UFRAME_CHANGE, (void*)MENU_NIBPCAL);
}

static void nibpset_verif_func(void * parg)
{
  nibp_verif();
  uframe_command(UFRAME_CHANGE, (void*)MENU_VERIF);
}

static void nibpset_leak_func(void * parg)
{
  nibp_leak_test();
  uframe_command(UFRAME_CHANGE, (void*)MENU_LEAKTEST);
}

static void nibpset_exit_func(void * parg)
{
  nibp_data_t nibp_data;

  unit_get_data(NIBP, &nibp_data);

  nibp_command(NIBP_CMD_SET_INITIAL_INFLATE, nibp_data.infl); // re set initial inflate
  if (nibp_data.meas_interval == NIBP_MEAS_INTERVAL_MANU)
  {
    sched_stop(SCHED_NIBP);
  }
  else
  {
    sched_start(SCHED_NIBP, nibp_meas_interval[nibp_data.meas_interval]*60*1000, nibp_do_bp, SCHED_NORMAL);
  }

//  sched_stop(SCHED_NIBP_TEST);

#if 0
  if (unit_set_data(NIBP, &nibp_data) <= 0)
  {
    debug("%s: error writing nibp data\n", __FUNCTION__);
  }
#endif

  uframe_command(UFRAME_DESTROY, NULL);

  unit_cfg_save();
}

void menu_nibpset_openproc(void)
{
  nibp_data_t nibp_data;
  char s[128];
  unsigned short ids;

  inpfoc_set(INPFOC_NIBPSET, NIBPSET_EXIT);

  memset(&nibp_data, 0, sizeof(nibp_data_t));
  if (unit_get_data(NIBP, &nibp_data) <= 0)
  {
    debug("%s: error reading nibp data\n", __FUNCTION__);
  }

  inpfoc_wnd_check(inpfoc_find(INPFOC_NIBPSET, NIBPSET_ALR), alarm_isenabled(NIBP_RISKSS));

  if (nibp_data.meas_interval == NIBP_MEAS_INTERVAL_MANU)
    ids = IDS_MANUALLY;
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
  ids2string(ids, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NIBPSET, NIBPSET_MODE), s);

  snprintf(s, sizeof(s), "%d", nibp_data.sd_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NIBPSET, NIBPSET_SDH), s);
  snprintf(s, sizeof(s), "%d", nibp_data.sd_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NIBPSET, NIBPSET_SDL), s);

  snprintf(s, sizeof(s), "%d", nibp_data.dd_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NIBPSET, NIBPSET_DDH), s);
  snprintf(s, sizeof(s), "%d", nibp_data.dd_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NIBPSET, NIBPSET_DDL), s);

  snprintf(s, sizeof(s), "%d", nibp_data.md_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NIBPSET, NIBPSET_MDH), s);
  snprintf(s, sizeof(s), "%d", nibp_data.md_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NIBPSET, NIBPSET_MDL), s);

  snprintf(s, sizeof(s), "%d", nibp_data.infl);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NIBPSET, NIBPSET_INFL), s);

  inpfoc_set(INPFOC_NIBPSET, NIBPSET_EXIT);
}

