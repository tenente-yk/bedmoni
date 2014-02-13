#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "grph.h"
#include "uframe.h"
#include "cframe.h"
#include "mframe.h"
#include "iframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "spo2set.h"

static void spo2set_none_func(void*);
static void spo2set_spo2alr_func(void*);
static void spo2set_hralr_func(void*);
static void spo2set_trshspo2h_func(void*);
static void spo2set_trshspo2l_func(void*);
static void spo2set_trshhrh_func(void*);
static void spo2set_trshhrl_func(void*);
static void spo2set_speed_func(void*);
static void spo2set_drawfpg_func(void*);
static void spo2set_exit_func(void*);

inpfoc_funclist_t inpfoc_spo2set_funclist[SPO2SET_NUM_ITEMS+1] = 
{
  { SPO2SET_NONE,      spo2set_none_func         },
  { SPO2SET_SPO2ALR,   spo2set_spo2alr_func      },
  { SPO2SET_TRSHSPO2H, spo2set_trshspo2h_func    },
  { SPO2SET_TRSHSPO2L, spo2set_trshspo2l_func    },
  { SPO2SET_HRALR,     spo2set_hralr_func        },
  { SPO2SET_TRSHHRH,   spo2set_trshhrh_func      },
  { SPO2SET_TRSHHRL,   spo2set_trshhrl_func      },
  { SPO2SET_SPEED,     spo2set_speed_func        },
  { SPO2SET_DRAWFPG,   spo2set_drawfpg_func      },
  { SPO2SET_EXIT,      spo2set_exit_func         },
  { -1       ,         spo2set_none_func         }, // -1 must be last
};

static void spo2set_none_func(void * parg)
{

}

static void spo2set_spo2alr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_SPO2SET, SPO2SET_SPO2ALR));
  alarm_on_off(SPO2_RISKSAT, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT_BELL, v);
}

static void spo2set_hralr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_SPO2SET, SPO2SET_HRALR));
  alarm_on_off(SPO2_RISKPULSE, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_HR_BELL, v);
}

static void spo2set_trshspo2h_func(void* parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  spo2_data_t spo2_data;

  if (unit_get_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHSPO2H);
  ddh = spo2_data.spo2_max;
  ddl = spo2_data.spo2_min;

  ddh += pcmd->delta*1;

  vl = 50;
  vu = 100;

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    spo2_data.spo2_max = ddh;
    unit_set_data(SPO2, &spo2_data);
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", spo2_data.spo2_min, spo2_data.spo2_max);
    unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT_RANGE, s);
  }
}

static void spo2set_trshspo2l_func(void* parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  spo2_data_t spo2_data;

  if (unit_get_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHSPO2L);
  ddh = spo2_data.spo2_max;
  ddl = spo2_data.spo2_min;

  ddl += pcmd->delta*1;

  vl = 50;
  vu = 100;

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    spo2_data.spo2_min = ddl;
    unit_set_data(SPO2, &spo2_data);
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", spo2_data.spo2_min, spo2_data.spo2_max);
    unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT_RANGE, s);
  }
}

static void spo2set_trshhrh_func(void* parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  spo2_data_t spo2_data;

  if (unit_get_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHHRH);
  ddh = spo2_data.hr_max;
  ddl = spo2_data.hr_min;

  ddh += pcmd->delta*1;

  vl = 20;
  vu = 300;

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    spo2_data.hr_max = ddh;
    unit_set_data(SPO2, &spo2_data);
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", spo2_data.hr_min, spo2_data.hr_max);
    unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_PULSE_RANGE, s);
  }
}

static void spo2set_trshhrl_func(void* parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  spo2_data_t spo2_data;

  memset(&spo2_data, 0, sizeof(spo2_data_t));
  if (unit_get_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHHRL);
  ddh = spo2_data.hr_max;
  ddl = spo2_data.hr_min;

  ddl += pcmd->delta*1;

  vl = 20;
  vu = 300;

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    spo2_data.hr_min = ddl;
    unit_set_data(SPO2, &spo2_data);
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", spo2_data.hr_min, spo2_data.hr_max);
    unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_PULSE_RANGE, s);
  }
}

static void spo2set_speed_func(void* parg)
{
  cframe_command(CFRAME_CHANSPD, (void*)MAKELONG(SPO2, ((inpfoc_cmd_t*)parg)->delta));
  // string caption will be updated in cframe_on_chanspd
}

static void spo2set_drawfpg_func(void* parg)
{
  inpfoc_item_t *pit;
  cframe_t * pcframe = cframe_getptr();
  char s[128];

  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_DRAWFPG);

  assert(pit);
  assert(pcframe);

  if (pcframe->chanview[SPO2].visible)
  {
    ids2string(IDS_OFF, s);
    inpfoc_wnd_setcaption(pit, s);
    cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(SPO2, 0));
  }
  else
  {
    ids2string(IDS_ON, s);
    inpfoc_wnd_setcaption(pit, s);
    cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(SPO2, 1));
  }
}

static void spo2set_exit_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  int v;
  spo2_data_t spo2_data;
  char s[10];

  memset(&spo2_data, 0, sizeof(spo2_data_t));

  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHSPO2H);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%d", &v);
  spo2_data.spo2_max = v;
  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHSPO2L);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%d", &v);
  spo2_data.spo2_min = v;
  snprintf(s, sizeof(s), "%d..%d", spo2_data.spo2_min, spo2_data.spo2_max);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT_RANGE, s);

  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHHRH);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%d", &v);
  spo2_data.hr_max = v;
  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHHRL);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%d", &v);
  spo2_data.hr_min = v;
  snprintf(s, sizeof(s), "%d..%d", spo2_data.hr_min, spo2_data.hr_max);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_PULSE_RANGE, s);

  if (unit_set_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: writing spo2 data\n", __FUNCTION__);
  }

  uframe_command(UFRAME_DESTROY, NULL);

  unit_cfg_save();
}

void menu_spo2set_openproc(void)
{
  inpfoc_item_t *pit;
  cframe_t * pcframe = cframe_getptr();
  spo2_data_t spo2_data;
  char s[200], s2[200];

  inpfoc_set(INPFOC_SPO2SET, SPO2SET_EXIT);

  assert(pcframe);

  memset(&spo2_data, 0, sizeof(spo2_data_t));
  if (unit_get_data(SPO2, &spo2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }

  inpfoc_wnd_check(inpfoc_find(INPFOC_SPO2SET, SPO2SET_SPO2ALR), alarm_isenabled(SPO2_RISKSAT));
  inpfoc_wnd_check(inpfoc_find(INPFOC_SPO2SET, SPO2SET_HRALR), alarm_isenabled(SPO2_RISKPULSE));

  if (pcframe->chanview[SPO2].visible)
    ids2string(IDS_ON, s);
  else
    ids2string(IDS_OFF, s);

  pit = inpfoc_find(INPFOC_SPO2SET, SPO2SET_DRAWFPG);

  assert(pit);
  inpfoc_wnd_setcaption(pit, s);

  snprintf(s, sizeof(s), "%d", spo2_data.spo2_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHSPO2H), s);
  snprintf(s, sizeof(s), "%d", spo2_data.spo2_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHSPO2L), s);

  snprintf(s, sizeof(s), "%d", spo2_data.hr_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHHRH), s);
  snprintf(s, sizeof(s), "%d", spo2_data.hr_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SPO2SET, SPO2SET_TRSHHRL), s);

  if (pcframe->chanview[SPO2].chandata.mmps == 6.25f)
    sprintf(s, "%.2f ", (float)pcframe->chanview[SPO2].chandata.mmps);
  else
  if (pcframe->chanview[SPO2].chandata.mmps == 12.5f)
    sprintf(s, "%.1f ", (float)pcframe->chanview[SPO2].chandata.mmps);
  else
    sprintf(s, "%d ", (int)pcframe->chanview[SPO2].chandata.mmps);
  ids2string(IDS_MMPS, s2);
  strcat(s, " ");
  strcat(s, s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SPO2SET, SPO2SET_SPEED), s);
}

