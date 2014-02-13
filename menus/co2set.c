#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "crbpio.h"
#include "dio.h"
#include "uframe.h"
#include "mframe.h"
#include "cframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "co2.h"
#include "co2set.h"

static void co2set_none_func(void*);
static void co2set_etco2h_func(void*);
static void co2set_etco2l_func(void*);
static void co2set_etco2alr_func(void*);
static void co2set_ico2_func(void*);
static void co2set_ico2alr_func(void*);
static void co2set_brh_func(void*);
static void co2set_brl_func(void*);
static void co2set_bralr_func(void*);
static void co2set_speed_func(void*);
static void co2set_drawco2_func(void*);
static void co2set_apnoe_func(void*);
static void co2set_apnoealr_func(void*);
static void co2set_iniset_func(void*);
static void co2set_exit_func(void*);

inpfoc_funclist_t inpfoc_co2set_funclist[CO2SET_NUM_ITEMS+1] = 
{
  { CO2SET_NONE,        co2set_none_func         },
  { CO2SET_ETCO2H,      co2set_etco2h_func       },
  { CO2SET_ETCO2L,      co2set_etco2l_func       },
  { CO2SET_ETCO2ALR,    co2set_etco2alr_func     },
  { CO2SET_ICO2,        co2set_ico2_func         },
  { CO2SET_ICO2ALR,     co2set_ico2alr_func      },
  { CO2SET_BRH,         co2set_brh_func          },
  { CO2SET_BRL,         co2set_brl_func          },
  { CO2SET_BRALR,       co2set_bralr_func        },
  { CO2SET_APNOE,       co2set_apnoe_func        },
  { CO2SET_APNOEALR,    co2set_apnoealr_func     },
  { CO2SET_SPEED,       co2set_speed_func        },
  { CO2SET_DRAWCO2,     co2set_drawco2_func      },
  { CO2SET_INISET,      co2set_iniset_func       },
  { CO2SET_EXIT,        co2set_exit_func         },
  { -1         ,        co2set_none_func         }, // -1 must be last
};

static void co2set_none_func(void * parg)
{

}

static void co2set_etco2h_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  co2_data_t co2_data;

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading co2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_CO2SET, CO2SET_ETCO2H);
  ddh = co2_data.etco2_max;
  ddl = co2_data.etco2_min;

  ddh += pcmd->delta*1;

  vl = 1;
  vu = 150;

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    co2_data.etco2_max = ddh;
    unit_set_data(CO2, &co2_data);
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", co2_data.etco2_min, co2_data.etco2_max);
    unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2_RANGE, s);
  }
}

static void co2set_etco2l_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  co2_data_t co2_data;

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_CO2SET, CO2SET_ETCO2L);
  ddh = co2_data.etco2_max;
  ddl = co2_data.etco2_min;

  ddl += pcmd->delta*1;

  vl = 0;
  vu = 150;

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    co2_data.etco2_min = ddl;
    unit_set_data(CO2, &co2_data);
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", co2_data.etco2_min, co2_data.etco2_max);
    unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2_RANGE, s);
  }
}

static void co2set_etco2alr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_CO2SET, CO2SET_ETCO2ALR));
  alarm_on_off(CO2_RISKETCO2, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2_BELL, v);
}

static void co2set_ico2_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  int vu, vl;
  char s[20];
  co2_data_t co2_data;

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading co2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_CO2SET, CO2SET_ICO2);
  dd = co2_data.ico2_max;

  dd += pcmd->delta;

  vl = 0;
  vu = 150;

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  co2_data.ico2_max = dd;
  unit_set_data(CO2, &co2_data);
  snprintf(s, sizeof(s), "%d", dd);
  inpfoc_wnd_setcaption(pit, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2_RANGE, s);

  unit_cfg_save();
}

static void co2set_ico2alr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_CO2SET, CO2SET_ICO2ALR));
  alarm_on_off(CO2_RISKICO2, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2_BELL, v);
}

static void co2set_brh_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  co2_data_t co2_data;

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading co2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_CO2SET, CO2SET_BRH);
  ddh = co2_data.br_max;
  ddl = co2_data.br_min;

  ddh += pcmd->delta*1;

  vl = 0;
  vu = 150;

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    co2_data.br_max = ddh;
    unit_set_data(CO2, &co2_data);
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", co2_data.br_min, co2_data.br_max);
    unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR_RANGE, s);
  }
}

static void co2set_brl_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  co2_data_t co2_data;

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_CO2SET, CO2SET_BRL);
  ddh = co2_data.br_max;
  ddl = co2_data.br_min;

  ddl += pcmd->delta*1;

  vl = 0;
  vu = 150;

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    co2_data.br_min = ddl;
    unit_set_data(CO2, &co2_data);
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", co2_data.br_min, co2_data.br_max);
    unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR_RANGE, s);
  }
}

static void co2set_bralr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_CO2SET, CO2SET_BRALR));
  alarm_on_off(CO2_RISKBRCO2, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR_BELL, v);
}

static void co2set_apnoe_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  int vu, vl;
  char s[20];
  co2_data_t co2_data;

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading spo2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_CO2SET, CO2SET_APNOE);
  dd = co2_data.ap_max;

  dd += pcmd->delta*1;

  vl = 0;
  vu = 60;

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  co2_data.ap_max = dd;
  unit_set_data(CO2, &co2_data);
  snprintf(s, sizeof(s), "%d", dd);
  inpfoc_wnd_setcaption(pit, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_APNOE_TIME, dd);
  co2_command(0x84, 3, 6,co2_data.ap_max,0);
  alarm_clr(CO2_APNOE); // TODO: maybe not needed
}

static void co2set_apnoealr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_CO2SET, CO2SET_APNOEALR));
  alarm_on_off(CO2_APNOE, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_APNOE_BELL, v);
}

static void co2set_speed_func(void * parg)
{
  cframe_command(CFRAME_CHANSPD, (void*)MAKELONG(CO2, ((inpfoc_cmd_t*)parg)->delta));
  // string caption will be updated in cframe_on_chanspd
}

static void co2set_drawco2_func(void * parg)
{
  inpfoc_item_t *pit;
  cframe_t * pcframe = cframe_getptr();
  char s[128];

  pit = inpfoc_find(INPFOC_CO2SET, CO2SET_DRAWCO2);

  assert(pit);
  assert(pcframe);

  if (pcframe->chanview[CO2].visible)
  {
    ids2string(IDS_OFF, s);
    inpfoc_wnd_setcaption(pit, s);
    cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(CO2, 0));
  }
  else
  {
    ids2string(IDS_ON, s);
    inpfoc_wnd_setcaption(pit, s);
    cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(CO2, 1));
  }
}

static void co2set_iniset_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_CO2INISET);
}


static void co2set_exit_func(void * parg)
{
  uframe_command(UFRAME_DESTROY, (void*)NULL);
}

void menu_co2set_openproc(void)
{
  inpfoc_item_t *pit;
  cframe_t * pcframe = cframe_getptr();
  co2_data_t co2_data;
  char s[200], s2[200];

  assert(pcframe);

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading co2 data\n", __FUNCTION__);
  }

  inpfoc_wnd_check(inpfoc_find(INPFOC_CO2SET, CO2SET_ETCO2ALR), alarm_isenabled(CO2_RISKETCO2));
  inpfoc_wnd_check(inpfoc_find(INPFOC_CO2SET, CO2SET_ICO2ALR), alarm_isenabled(CO2_RISKICO2));
  inpfoc_wnd_check(inpfoc_find(INPFOC_CO2SET, CO2SET_BRALR), alarm_isenabled(CO2_RISKBRCO2));
  inpfoc_wnd_check(inpfoc_find(INPFOC_CO2SET, CO2SET_APNOEALR), alarm_isenabled(CO2_APNOE));

  if (pcframe->chanview[CO2].visible)
    ids2string(IDS_ON, s);
  else
    ids2string(IDS_OFF, s);

  pit = inpfoc_find(INPFOC_CO2SET, CO2SET_DRAWCO2);

  assert(pit);
  inpfoc_wnd_setcaption(pit, s);

  snprintf(s, sizeof(s), "%d", co2_data.etco2_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2SET, CO2SET_ETCO2H), s);
  snprintf(s, sizeof(s), "%d", co2_data.etco2_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2SET, CO2SET_ETCO2L), s);

  snprintf(s, sizeof(s), "%d", co2_data.ico2_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2SET, CO2SET_ICO2), s);

  snprintf(s, sizeof(s), "%d", co2_data.br_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2SET, CO2SET_BRH), s);
  snprintf(s, sizeof(s), "%d", co2_data.br_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2SET, CO2SET_BRL), s);

  snprintf(s, sizeof(s), "%d", co2_data.ap_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2SET, CO2SET_APNOE), s);

  if (pcframe->chanview[CO2].chandata.mmps == 6.25f)
    sprintf(s, "%.2f ", (float)pcframe->chanview[CO2].chandata.mmps);
  else
  if (pcframe->chanview[CO2].chandata.mmps == 12.5f)
    sprintf(s, "%.1f ", (float)pcframe->chanview[CO2].chandata.mmps);
  else
    sprintf(s, "%d ", (int)pcframe->chanview[CO2].chandata.mmps);
  ids2string(IDS_MMPS, s2);
  strcat(s, " ");
  strcat(s, s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2SET, CO2SET_SPEED), s);

  inpfoc_set(INPFOC_CO2SET, CO2SET_EXIT);
}

