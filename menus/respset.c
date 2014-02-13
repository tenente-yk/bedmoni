/*! \file respset.c
 *  \brief Resp settings popup frame
 */
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
#include "cframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "ecgcalc.h"
#include "respcalc.h"
#include "resp.h"
#include "respset.h"
#include "dio.h"

static void respset_none_func(void*);
static void respset_bralr_func(void*);
static void respset_brh_func(void*);
static void respset_brl_func(void*);
static void respset_ap_func(void*);
static void respset_apalr_func(void*);
static void respset_drawrpg_func(void*);
static void respset_speed_func(void*);
static void respset_chansel_func(void*);
static void respset_exit_func(void*);

inpfoc_funclist_t inpfoc_respset_funclist[RESPSET_NUM_ITEMS+1] = 
{
  { RESPSET_NONE,    respset_none_func         },
  { RESPSET_BRALR,   respset_bralr_func        },
  { RESPSET_BRH,     respset_brh_func          },
  { RESPSET_BRL,     respset_brl_func          },
  { RESPSET_AP,      respset_ap_func           },
  { RESPSET_APALR,   respset_apalr_func        },
  { RESPSET_DRAWRPG, respset_drawrpg_func      },
  { RESPSET_SPEED,   respset_speed_func        },
  { RESPSET_CHANSEL, respset_chansel_func      },
  { RESPSET_EXIT,    respset_exit_func         },
  { -1       ,       respset_none_func         }, // -1 must be last
};

static void respset_none_func(void * parg)
{

}

static void respset_bralr_func(void * parg)
{
  int v;
  v = alarm_isenabled(RESP_RISKBR);
  v = !v;
  alarm_on_off(RESP_RISKBR, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR_BELL, v);
}

static void respset_brh_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  resp_data_t resp_data;

  memset(&resp_data, 0, sizeof(resp_data_t));
  if (unit_get_data(RESP, &resp_data) <= 0)
  {
    debug("%s: reading resp data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_RESPSET, RESPSET_BRH);
  ddh = resp_data.br_max;
  ddl = resp_data.br_min;

  ddh += pcmd->delta*1;

  vl = 0;
  vu = 120;

  while (ddh < vl) ddh += (vu-vl+1);
  while (ddh > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    resp_data.br_max = ddh;
    unit_set_data(RESP, &resp_data);
    snprintf(s, sizeof(s), "%d", ddh);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", resp_data.br_min, resp_data.br_max);
    unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR_RANGE, s);
  }
}

static void respset_brl_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  int vu, vl;
  char s[20];
  resp_data_t resp_data;

  memset(&resp_data, 0, sizeof(resp_data_t));
  if (unit_get_data(RESP, &resp_data) <= 0)
  {
    debug("%s: reading resp data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_RESPSET, RESPSET_BRL);
  ddh = resp_data.br_max;
  ddl = resp_data.br_min;

  ddl += pcmd->delta;

  vl = 0;
  vu = 120;

  while (ddl < vl) ddh += (vu-vl+1);
  while (ddl > vu) ddh -= (vu-vl+1);

  if (ddh >= ddl)
  {
    resp_data.br_min = ddl;
    unit_set_data(RESP, &resp_data);
    snprintf(s, sizeof(s), "%d", ddl);
    inpfoc_wnd_setcaption(pit, s);
    snprintf(s, sizeof(s), "%d..%d", resp_data.br_min, resp_data.br_max);
    unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR_RANGE, s);
  }
}

static void respset_ap_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  int vu, vl;
  char s[20];
  resp_data_t resp_data;

  memset(&resp_data, 0, sizeof(resp_data_t));
  if (unit_get_data(RESP, &resp_data) <= 0)
  {
    debug("%s: reading resp data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_RESPSET, RESPSET_AP);
  dd = resp_data.ap_max;

  dd += pcmd->delta;

  vl = 5;
  vu = 60;

  while (dd < vl) dd += (vu-vl+1);
  while (dd > vu) dd -= (vu-vl+1);

  resp_data.ap_max = dd;
  unit_set_data(RESP, &resp_data);
  snprintf(s, sizeof(s), "%d", dd);
  inpfoc_wnd_setcaption(pit, s);
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_APNOE_TIME, dd);

  dio_module_cmd(PD_ID_ECS, ECS_RESP_APNOE_S, dd);

  unit_cfg_save();
}

static void respset_apalr_func(void * parg)
{
  int v;
  v = alarm_isenabled(RESP_APNOE);
  v = !v;
  alarm_on_off(RESP_APNOE, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_APNOE_BELL, v);

  unit_cfg_save();
}

static void respset_drawrpg_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  cframe_t * pcframe = cframe_getptr();

  pit = inpfoc_find(INPFOC_RESPSET, RESPSET_DRAWRPG);
  pifw = (inpfoc_wnd_t*) pit->parg;

  assert(pit);
  assert(pifw);
  assert(pcframe);

  if (pcframe->chanview[RESP].visible)
  {
    inpfoc_wnd_setids(pit, IDS_OFF);
    cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(RESP, 0));
  }
  else
  {
    inpfoc_wnd_setids(pit, IDS_ON);
    cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(RESP, 1));
  }
}

static void respset_speed_func(void * parg)
{
  cframe_command(CFRAME_CHANSPD, (void*)MAKELONG(RESP, ((inpfoc_cmd_t*)parg)->delta));
  // string caption will be updated in cframe_on_chanspd
}

static void respset_chansel_func(void * parg)
{
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  // set RL if RF now is set and otherwise
  ecgm_command((ecg_data.set_bits.breath_ch) ? ECGM_RESP_CHAN_RL : ECGM_RESP_CHAN_RF);
  // when channel changes window updates automatically
  // no need calling inpfoc_wnd_setcaption

  // RL and RF bit fields are otherwise here
  unsigned char st;
  st = ((ecg_data.break_byte & 0x3) != 0 && !ecg_data.set_bits.breath_ch == 0) || // RL
       ((ecg_data.break_byte & 0x5) != 0 && !ecg_data.set_bits.breath_ch == 1);   // RF
  alarm_set_clr(RESP_BREAK, st);

#if defined (RESPCALC_COLIBRI)
  respcalc_reset(); // do it instantly and after soft reset
#endif

  resp_soft_hw_reset();
}

static void respset_exit_func(void * parg)
{
  uframe_command(UFRAME_DESTROY, NULL);

  unit_cfg_save();
}

void menu_respset_openproc(void)
{
  cframe_t * pcframe = cframe_getptr();
  resp_data_t resp_data;
  ecg_data_t ecg_data;
  char s[200], s2[200];

  memset(&resp_data, 0, sizeof(resp_data_t));
  if (unit_get_data(RESP, &resp_data) <= 0)
  {
    debug("%s: error reading resp data\n", __FUNCTION__);
  }
  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  inpfoc_wnd_check(inpfoc_find(INPFOC_RESPSET, RESPSET_BRALR),  alarm_isenabled(RESP_RISKBR));
  inpfoc_wnd_check(inpfoc_find(INPFOC_RESPSET, RESPSET_APALR),  alarm_isenabled(RESP_APNOE));

  inpfoc_wnd_setids(inpfoc_find(INPFOC_RESPSET, RESPSET_DRAWRPG), (pcframe->chanview[RESP].visible)?IDS_ON:IDS_OFF);

  snprintf(s, sizeof(s), "%d", resp_data.br_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_RESPSET, RESPSET_BRH), s);
  snprintf(s, sizeof(s), "%d", resp_data.br_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_RESPSET, RESPSET_BRL), s);
  snprintf(s, sizeof(s), "%d", resp_data.ap_max);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_RESPSET, RESPSET_AP), s);

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_RESPSET, RESPSET_CHANSEL), (ecg_data.set_bits.breath_ch) ? "RF" : "RL");

  if (pcframe->chanview[RESP].chandata.mmps == 6.25f)
    sprintf(s, "%.2f ", (float)pcframe->chanview[RESP].chandata.mmps);
  else
  if (pcframe->chanview[RESP].chandata.mmps == 12.5f)
    sprintf(s, "%.1f ", (float)pcframe->chanview[RESP].chandata.mmps);
  else
    sprintf(s, "%d ", (int)pcframe->chanview[RESP].chandata.mmps);
  ids2string(IDS_MMPS, s2);
  strcat(s, " ");
  strcat(s, s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_RESPSET, RESPSET_SPEED), s);

  inpfoc_set(INPFOC_RESPSET, RESPSET_EXIT);
}

