#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "crbpio.h"
#include "uframe.h"
#include "mframe.h"
#include "dio.h"
#include "ecgcalc.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "arrhset.h"

static void arrhset_none_func(void*);
static void arrhset_asys_func(void*);
static void arrhset_asysalr_func(void*);
static void arrhset_norespdly_func(void*);
static void arrhset_norespalr_func(void*);
static void arrhset_exit_func(void*);

inpfoc_funclist_t inpfoc_arrhset_funclist[ARRHSET_NUM_ITEMS+1] = 
{
  { ARRHSET_NONE,      arrhset_none_func         },
  { ARRHSET_ASYS,      arrhset_asys_func         },
  { ARRHSET_ASYSALR,   arrhset_asysalr_func      },
  { ARRHSET_NORESPDLY, arrhset_norespdly_func    },
  { ARRHSET_NORESPALR, arrhset_norespalr_func    },
  { ARRHSET_EXIT,      arrhset_exit_func         },
  { -1       ,         arrhset_none_func         }, // -1 must be last
};

static void arrhset_none_func(void * parg)
{

}

static void arrhset_asys_func(void * parg)
{
  char s[20];
  ecg_data_t ecg_data;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;

  unit_get_data(ECG, &ecg_data);

  ecg_data.asys_sec += pcmd->delta;

  while ((signed char)ecg_data.asys_sec < 1)  ecg_data.asys_sec += (9 + 1);
  while ((signed char)ecg_data.asys_sec > 10) ecg_data.asys_sec -= (9 + 1);

  unit_set_data(ECG, &ecg_data);

  sprintf(s, "%d", ecg_data.asys_sec);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ARRHSET, ARRHSET_ASYS), s);
}

static void arrhset_asysalr_func(void * parg)
{
  alarm_on_off(ECG_ASYSTOLIA, !alarm_isenabled(ECG_ASYSTOLIA));
}

static void arrhset_norespdly_func(void * parg)
{
  char s[20];
  ecg_data_t ecg_data;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;

  unit_get_data(ECG, &ecg_data);

  ecg_data.pmnr_ms += pcmd->delta;

  while (ecg_data.pmnr_ms < 20)  ecg_data.pmnr_ms += ((300-20) + 1);
  while (ecg_data.pmnr_ms > 300) ecg_data.pmnr_ms -= ((300-20) + 1);

  unit_set_data(ECG, &ecg_data);

  dio_module_cmd(PD_ID_ECS, ECS_PM_NORESP_MS, ecg_data.pmnr_ms);

  sprintf(s, "%d", ecg_data.pmnr_ms);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ARRHSET, ARRHSET_NORESPDLY), s);
}

static void arrhset_norespalr_func(void * parg)
{
  alarm_on_off(ECG_PM_NO_ANSWER, !alarm_isenabled(ECG_PM_NO_ANSWER));
}

static void arrhset_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_ECGSET);
}

void menu_arrhset_openproc(void)
{
  char s[200];
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);

  sprintf(s, "%d", ecg_data.asys_sec);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ARRHSET, ARRHSET_ASYS), s);
  sprintf(s, "%d", ecg_data.pmnr_ms);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ARRHSET, ARRHSET_NORESPDLY), s);
 // sprintf(s, "%d", ecg_data.tach_bpm);
 // inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ARRHSET, ARRHSET_TACH), s);

  inpfoc_wnd_check(inpfoc_find(INPFOC_ARRHSET, ARRHSET_ASYSALR), alarm_isenabled(ECG_ASYSTOLIA) ? IWC_CHECKED : IWC_UNCHECKED);

  inpfoc_wnd_check(inpfoc_find(INPFOC_ARRHSET, ARRHSET_NORESPALR), alarm_isenabled(ECG_PM_NO_ANSWER) ? IWC_CHECKED : IWC_UNCHECKED);

  inpfoc_disable(INPFOC_ARRHSET, ARRHSET_TACH);
 // inpfoc_disable(INPFOC_ARRHSET, ARRHSET_TACHALR);

  inpfoc_set(INPFOC_ARRHSET, ARRHSET_EXIT);
}

