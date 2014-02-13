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
#include "co2iniset.h"

static void co2iniset_none_func(void*);
static void co2iniset_pressure_func(void*);
static void co2iniset_o2_compens_func(void*);
static void co2iniset_gas_balance_func(void*);
static void co2iniset_anest_agent_func(void*);
static void co2iniset_zeroing_func(void*);
static void co2iniset_exit_func(void*);

static unsigned short gas_balance_ids[NUM_CO2_BAL_GASES] = {IDS_ROOM_AIR, IDS_N2O, IDS_HELIUM};

inpfoc_funclist_t inpfoc_co2iniset_funclist[CO2INISET_NUM_ITEMS+1] = 
{
  { CO2INISET_NONE,        co2iniset_none_func         },
  { CO2INISET_PRESSURE,    co2iniset_pressure_func     },
  { CO2INISET_O2_COMPENS,  co2iniset_o2_compens_func   },
  { CO2INISET_GAS_BALANCE, co2iniset_gas_balance_func  },
  { CO2INISET_ANEST_AGENT, co2iniset_anest_agent_func  },
  { CO2INISET_ZEROING,     co2iniset_zeroing_func      },
  { CO2INISET_EXIT,        co2iniset_exit_func         },
  { -1         ,           co2iniset_none_func         }, // -1 must be the last
};

static void co2iniset_none_func(void * parg)
{

}

static void co2iniset_pressure_func(void * parg)
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

  pit = inpfoc_find(INPFOC_CO2INISET, CO2INISET_PRESSURE);
  dd = co2_data.pressure;

  dd += pcmd->delta;

  vl = 400;
  vu = 850;

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  co2_data.pressure = dd;
  unit_set_data(CO2, &co2_data);
  snprintf(s, sizeof(s), "%d", dd);
  inpfoc_wnd_setcaption(pit, s);

  co2_command(0x84, 4, 1,  (co2_data.pressure/0x80)&0x7F,co2_data.pressure&0x7F,0);

  unit_cfg_save();
}

static void co2iniset_o2_compens_func(void * parg)
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

  pit = inpfoc_find(INPFOC_CO2INISET, CO2INISET_O2_COMPENS);
  dd = co2_data.o2_compens;

  dd += pcmd->delta;

  vl = 0;
  vu = 100;

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  co2_data.o2_compens = dd;
  unit_set_data(CO2, &co2_data);
  snprintf(s, sizeof(s), "%d%%", dd);
  inpfoc_wnd_setcaption(pit, s);

  co2_command(0x84, 6, 11, co2_data.o2_compens,co2_data.gas_balance,(co2_data.anest_agent/0x80)&0x7F,co2_data.anest_agent&0x7F,0);

  unit_cfg_save();
}

static void co2iniset_gas_balance_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  int vu, vl;
  char s[200];
  co2_data_t co2_data;

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading co2 data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_CO2INISET, CO2INISET_GAS_BALANCE);
  dd = co2_data.gas_balance;

  dd += pcmd->delta;

  vl = 0;
  vu = NUM_CO2_BAL_GASES-1;

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  co2_data.gas_balance = dd;
  unit_set_data(CO2, &co2_data);
  ids2string(gas_balance_ids[dd], s);
  inpfoc_wnd_setcaption(pit, s);

  co2_command(0x84, 6, 11, co2_data.o2_compens,co2_data.gas_balance,(co2_data.anest_agent/0x80)&0x7F,co2_data.anest_agent&0x7F,0);

  unit_cfg_save();
}

static void co2iniset_anest_agent_func(void * parg)
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

  pit = inpfoc_find(INPFOC_CO2INISET, CO2INISET_ANEST_AGENT);
  dd = co2_data.anest_agent;

  dd += pcmd->delta;

  vl = 0;
  vu = 200;

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  co2_data.anest_agent = dd;
  unit_set_data(CO2, &co2_data);
  snprintf(s, sizeof(s), "%.1f%%", (float)dd/10);
  inpfoc_wnd_setcaption(pit, s);

  co2_command(0x84, 6, 11, co2_data.o2_compens,co2_data.gas_balance,(co2_data.anest_agent/0x80)&0x7F,co2_data.anest_agent&0x7F,0);

  unit_cfg_save();
}

static void co2iniset_zeroing_func(void * parg)
{
  char s[200];

  alarm_clr_fixed(CO2_APNOE);
  ids2string(IDS_UNDEF3, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BRSTATE, s);

  alarm_clr(CO2_RISKETCO2);
  alarm_clr(CO2_RISKBRCO2);
  alarm_clr(CO2_RISKICO2);
  co2_command(0x82, 1, 00); // initiate capnostat zeroing
}

static void co2iniset_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_CO2SET);
}

void menu_co2iniset_openproc(void)
{
  cframe_t * pcframe = cframe_getptr();
  co2_data_t co2_data;
  char s[128];

  assert(pcframe);

  if (unit_get_data(CO2, &co2_data) <= 0)
  {
    error("%s: reading co2 data\n", __FUNCTION__);
  }

  snprintf(s, sizeof(s), "%d", co2_data.pressure);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2INISET, CO2INISET_PRESSURE), s);

  snprintf(s, sizeof(s), "%d%%", co2_data.o2_compens);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2INISET, CO2INISET_O2_COMPENS), s);

  ids2string(gas_balance_ids[co2_data.gas_balance], s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2INISET, CO2INISET_GAS_BALANCE), s);

  snprintf(s, sizeof(s), "%.1f%%", (float)co2_data.anest_agent/10);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_CO2INISET, CO2INISET_ANEST_AGENT), s);

  inpfoc_set(INPFOC_CO2INISET, CO2INISET_EXIT);
}

