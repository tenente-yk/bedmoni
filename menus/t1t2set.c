#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "bedmoni.h"
#include "grph.h"
#include "uframe.h"
#include "cframe.h"
#include "mframe.h"
#include "iframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "t1t2set.h"

static void t1t2set_none_func(void*);
static void t1t2set_t1alr_func(void*);
static void t1t2set_t1h_func(void*);
static void t1t2set_t1l_func(void*);
static void t1t2set_t2alr_func(void*);
static void t1t2set_t2h_func(void*);
static void t1t2set_t2l_func(void*);
static void t1t2set_dtalr_func(void*);
static void t1t2set_dt_func(void*);
static void t1t2set_exit_func(void*);

inpfoc_funclist_t inpfoc_t1t2set_funclist[T1T2SET_NUM_ITEMS+1] = 
{
  { T1T2SET_NONE,     t1t2set_none_func         },
  { T1T2SET_T1ALR,    t1t2set_t1alr_func        },
  { T1T2SET_T1H,      t1t2set_t1h_func          },
  { T1T2SET_T1L,      t1t2set_t1l_func          },
  { T1T2SET_T2ALR,    t1t2set_t2alr_func        },
  { T1T2SET_T2H,      t1t2set_t2h_func          },
  { T1T2SET_T2L,      t1t2set_t2l_func          },
  { T1T2SET_DTALR,    t1t2set_dtalr_func        },
  { T1T2SET_DT,       t1t2set_dt_func           },
  { T1T2SET_EXIT,     t1t2set_exit_func         },
  { -1       ,        t1t2set_none_func         }, // -1 must be last
};

static void t1t2set_none_func(void * parg)
{

}

static void t1t2set_t1alr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_T1T2SET, T1T2SET_T1ALR));
  alarm_on_off(T1T2_RISKT1, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1_BELL, v);
}

static void t1t2set_t1h_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  t1t2_data_t t1t2_data;

  memset(&t1t2_data, 0, sizeof(t1t2_data_t));
  if (unit_get_data(T1T2, &t1t2_data) <= 0)
  {
    debug("%s: error reading t1t2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_T1H);
  ddh = t1t2_data.t1_max;
  ddl = t1t2_data.t1_min;

  ddh += pcmd->delta*1;

  vl = 101;
  vu = 500;

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    t1t2_data.t1_max = ddh;
    unit_set_data(T1T2, &t1t2_data);
    snprintf(s, sizeof(s), "%.1f", (float)ddh/10);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%.1f..%.1f", (float)t1t2_data.t1_min/10, (float)t1t2_data.t1_max/10);
    unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1_RANGE, s);
  }
}

static void t1t2set_t1l_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  t1t2_data_t t1t2_data;

  memset(&t1t2_data, 0, sizeof(t1t2_data_t));
  if (unit_get_data(T1T2, &t1t2_data) <= 0)
  {
    debug("%s: error reading t1t2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_T1L);
  ddh = t1t2_data.t1_max;
  ddl = t1t2_data.t1_min;

  ddl += pcmd->delta*1;

  vl = 100;
  vu = 499;

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    t1t2_data.t1_min = ddl;
    unit_set_data(T1T2, &t1t2_data);
    snprintf(s, sizeof(s), "%.1f", (float)ddl/10);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%.1f..%.1f", (float)t1t2_data.t1_min/10, (float)t1t2_data.t1_max/10);
    unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1_RANGE, s);
  }
}

static void t1t2set_t2alr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_T1T2SET, T1T2SET_T2ALR));
  alarm_on_off(T1T2_RISKT2, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2_BELL, v);
}

static void t1t2set_t2h_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  t1t2_data_t t1t2_data;

  memset(&t1t2_data, 0, sizeof(t1t2_data_t));
  if (unit_get_data(T1T2, &t1t2_data) <= 0)
  {
    debug("%s: error reading t1t2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_T2H);
  ddh = t1t2_data.t2_max;
  ddl = t1t2_data.t2_min;

  ddh += pcmd->delta*1;

  vl = 101;
  vu = 500;

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    t1t2_data.t2_max = ddh;
    unit_set_data(T1T2, &t1t2_data);
    snprintf(s, sizeof(s), "%.1f", (float)ddh/10);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%.1f..%.1f", (float)t1t2_data.t2_min/10, (float)t1t2_data.t2_max/10);
    unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2_RANGE, s);
  }
}

static void t1t2set_t2l_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  t1t2_data_t t1t2_data;

  memset(&t1t2_data, 0, sizeof(t1t2_data_t));
  if (unit_get_data(T1T2, &t1t2_data) <= 0)
  {
    debug("%s: error reading t1t2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_T2L);
  ddh = t1t2_data.t2_max;
  ddl = t1t2_data.t2_min;

  ddl += pcmd->delta*1;

  vl = 101;
  vu = 500;

  while (ddl < vl) ddl += (vu-vl+1);
  while (ddl > vu) ddl -= (vu-vl+1);

  if (ddl <= ddh)
  {
    t1t2_data.t2_min = ddl;
    unit_set_data(T1T2, &t1t2_data);
    snprintf(s, sizeof(s), "%.1f", (float)ddl/10);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%.1f..%.1f", (float)t1t2_data.t2_min/10, (float)t1t2_data.t2_max/10);
    unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2_RANGE, s);
  }
}

static void t1t2set_dtalr_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_T1T2SET, T1T2SET_DTALR));
  alarm_on_off(T1T2_RISKDT, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT_BELL, v);
}

static void t1t2set_dt_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  int vu, vl;
  char s[20];
  t1t2_data_t t1t2_data;

  memset(&t1t2_data, 0, sizeof(t1t2_data_t));
  if (unit_get_data(T1T2, &t1t2_data) <= 0)
  {
    debug("%s: error reading t1t2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_DT);
  dd = t1t2_data.dt_max;

  dd += pcmd->delta*1;

  vl = 0;
  vu = 500;

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  t1t2_data.dt_max = dd;
  unit_set_data(T1T2, &t1t2_data);
  snprintf(s, sizeof(s), "%.1f", (float)dd/10);
  inpfoc_wnd_setcaption(pit, s);
  snprintf(s, sizeof(s), "%.1f", (float)t1t2_data.dt_max/10);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT_RANGE, s);
}

static void t1t2set_exit_func(void * parg)
{
 // cframe_t * pcframe = cframe_getptr();
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  float fl;
  t1t2_data_t t1t2_data;
  char s[20];

  memset(&t1t2_data, 0, sizeof(t1t2_data_t));

  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_T1L);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%f", &fl);
  t1t2_data.t1_min = fl*10;
  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_T1H);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%f", &fl);
  t1t2_data.t1_max = fl*10;
  snprintf(s, sizeof(s), "%.1f..%.1f", (float)t1t2_data.t1_min/10, (float)t1t2_data.t1_max/10);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1_RANGE, s);

  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_T2L);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%f", &fl);
  t1t2_data.t2_min = fl*10;
  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_T2H);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%f", &fl);
  t1t2_data.t2_max = fl*10;
  snprintf(s, sizeof(s), "%.1f..%.1f", (float)t1t2_data.t2_min/10, (float)t1t2_data.t2_max/10);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2_RANGE, s);

  pit = inpfoc_find(INPFOC_T1T2SET, T1T2SET_DT);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%f", &fl);
  t1t2_data.dt_max = fl*10;
  snprintf(s, sizeof(s), "%.1f", (float)t1t2_data.dt_max/10);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT_RANGE, s);

  if (unit_set_data(T1T2, &t1t2_data) <= 0)
  {
    error("%s: writing t1t2 data\n", __FUNCTION__);
  }

  uframe_command(UFRAME_DESTROY, NULL);

  unit_cfg_save();
}

void menu_t1t2set_openproc(void)
{
  t1t2_data_t t1t2_data;
  char s[10];

  memset(&t1t2_data, 0, sizeof(t1t2_data_t));
  if (unit_get_data(T1T2, &t1t2_data) <= 0)
  {
    debug("%s: error reading t1t2 data\n", __FUNCTION__);
  }

  inpfoc_wnd_check(inpfoc_find(INPFOC_T1T2SET, T1T2SET_T1ALR), alarm_isenabled(T1T2_RISKT1));
  inpfoc_wnd_check(inpfoc_find(INPFOC_T1T2SET, T1T2SET_T2ALR), alarm_isenabled(T1T2_RISKT2));
  inpfoc_wnd_check(inpfoc_find(INPFOC_T1T2SET, T1T2SET_DTALR), alarm_isenabled(T1T2_RISKDT));

  snprintf(s, sizeof(s), "%.1f", (float)t1t2_data.t1_min/10);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_T1T2SET, T1T2SET_T1L), s);
  snprintf(s, sizeof(s), "%.1f", (float)t1t2_data.t1_max/10);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_T1T2SET, T1T2SET_T1H), s);

  snprintf(s, sizeof(s), "%.1f", (float)t1t2_data.t2_min/10);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_T1T2SET, T1T2SET_T2L), s);
  snprintf(s, sizeof(s), "%.1f", (float)t1t2_data.t2_max/10);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_T1T2SET, T1T2SET_T2H), s);

  snprintf(s, sizeof(s), "%.1f", (float)t1t2_data.dt_max/10);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_T1T2SET, T1T2SET_DT), s);

  inpfoc_set(INPFOC_T1T2SET, T1T2SET_EXIT);
}

