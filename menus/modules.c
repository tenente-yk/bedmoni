/*! \file modules.c
    \brief Popup frame with modules enablity settings
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "dview.h"
#include "dio.h"
#include "mframe.h"
#include "uframe.h"
#include "cframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "powerman.h"
#include "crbpio.h"
#include "modules.h"

static void mod_none_func(void*);
static void mod_ecg_func(void*);
static void mod_spo2_func(void*);
static void mod_id1_func(void*);
static void mod_co2_func(void*);
static void mod_id2_func(void*);
static void mod_nad_func(void*);
static void mod_resp_func(void*);
static void mod_t_func(void*);
static void mod_save_func(void*);
static void mod_exit_func(void*);

static unsigned int chmask = 0;

inpfoc_funclist_t inpfoc_mod_funclist[MOD_NUM_ITEMS+1] = 
{
  { MOD_NONE,         mod_none_func         },
  { MOD_ECG,          mod_ecg_func          },
  { MOD_SPO2,         mod_spo2_func         },
  { MOD_NAD,          mod_nad_func          },
  { MOD_ID1,          mod_id1_func          },
  { MOD_CO2,          mod_co2_func          },
  { MOD_ID2,          mod_id2_func          },
  { MOD_RESP,         mod_resp_func         },
  { MOD_T,            mod_t_func            },
  { MOD_SAVE,         mod_save_func         },
  { MOD_EXIT,         mod_exit_func         },
  { -1      ,         mod_none_func         }, // -1 must be last
};

static void mod_none_func(void * parg)
{

}

static void mod_ecg_func(void * parg)
{
  int r;
  uframe_clearbox(15, 360, 180, 40);
  r = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_MOD, MOD_ECG));
  if (r)
    chmask |= (1 << ECG);
  else
  {
    chmask &= ~(1 << ECG);
    chmask &= ~(1 << RESP);
    inpfoc_wnd_check(inpfoc_find(INPFOC_MOD, MOD_RESP), IWC_UNCHECKED);
  }
}

static void mod_spo2_func(void * parg)
{
  int r;
  uframe_clearbox(15, 360, 180, 40);
  r = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_MOD, MOD_SPO2));
  if (r)
    chmask |= (1 << SPO2);
  else
    chmask &= ~(1 << SPO2);
}

static void mod_id1_func(void * parg)
{
  int r;
  uframe_clearbox(15, 360, 180, 40);
  r = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_MOD, MOD_ID1));
  if (r)
    chmask |= (1 << ECG_NEO);
  else
    chmask &= ~(1 << ECG_NEO);
}

static void mod_id2_func(void * parg)
{
  int r;
  uframe_clearbox(15, 360, 180, 40);
  r = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_MOD, MOD_ID2));
  if (r)
    chmask |= (1 << ECG_NEO);
  else
    chmask &= ~(1 << ECG_NEO);
}

static void mod_co2_func(void * parg)
{
  int r;
  uframe_clearbox(15, 360, 180, 40);
  r = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_MOD, MOD_CO2));
  if (r)
    chmask |= (1 << CO2);
  else
    chmask &= ~(1 << CO2);
}

static void mod_nad_func(void * parg)
{
  int r;
  uframe_clearbox(15, 360, 180, 40);
  r = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_MOD, MOD_NAD));
  if (r)
    chmask |= (1 << NIBP);
  else
    chmask &= ~(1 << NIBP);
}

static void mod_resp_func(void * parg)
{
  int r;
  uframe_clearbox(15, 360, 180, 40);
  r = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_MOD, MOD_RESP));
  if (r)
    chmask |= (1 << RESP);
  else
    chmask &= ~(1 << RESP);
}

static void mod_t_func(void * parg)
{
  int r;
  uframe_clearbox(15, 360, 180, 40);
  r = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_MOD, MOD_T));
  if (r)
    chmask |= (1 << T1T2);
  else
    chmask &= ~(1 << T1T2);
}

static void mod_save_func(void * parg)
{
  int i;
  int r, n;
  char s[200];

 // debug("%s: chmask = 0x%X\n", __FUNCTION__, chmask);

  n = 0;
  for (i=0; i<NUM_VIEWS; i++)
  {
    n += ((chmask >> i) & 0x1);
  }
  if (n > 6 || (n > 5 && (!mframe_ismaximized())))
  {
    unsigned short ids2;
    uframe_clearbox(15, 360, 180, 40);
    ids2 = (mframe_ismaximized()) ? IDS_MORE6MODULES_2 : IDS_MORE5MODULES_2;
    ids2string(IDS_MORE5MODULES_1, s); // IDS_MORE5MODULES_1 == IDS_MORE6MODULES_1
    uframe_printbox(17, 360, -1, -1, s, RGB(0xff,0x00,0x00));
    ids2string(ids2, s);
    uframe_printbox(17, 380, -1, -1, s, RGB(0xff,0x00,0x00));
    return;
  }

  for (i=0; i<NUM_VIEWS; i++)
  {
    r = unit_isenabled(i);
    if (r < 0) continue;
    if ( r != ((chmask >> i) & 0x1) )
    {
      int k;
      r = !r;
      unit_ioctl(i, (r) ? UNIT_CMD_SHOW : UNIT_CMD_HIDE);
      switch (i)
      {
        case ECG:
          if (r)
          {
            for (k=0; k<3; k++)
              cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(ECG_1+k, 1));
          }
          else
          {
            for (k=0; k<NUM_ECG; k++)
              cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(ECG_1+k, 0));
          }
#ifndef ARM // not works on ARM, on Desktop it works fine
          if (r)
          {
            dio_module_cmd(PD_ID_ECG, SBDP_RST_MOD);
            // set startup info for further restore after reset
            mframe_command(MFRAME_STARTUP, NULL);
          }
          else
          {
            startup_info_t si;
            ecg_data_t ecg_data;
            memset(&si, 0, sizeof(startup_info_t));
            unit_get_data(ECG, &ecg_data);
            si.ecg_set = ecg_data.set;
            mframe_command(MFRAME_SET_STARTUP_CFG, (void*)&si);
            dio_module_cmd(PD_ID_ECG, SBDP_OFF_MOD);
          }
#endif
          break;
        case SPO2:
         // dio_module_cmd(PD_ID_SPO2, (r) ? SBDP_RST_MOD : SBDP_OFF_MOD); // <- ne to be reconfigured at startup in a reset case
          cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(SPO2, r));
          break;
        case RESP:
          cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(RESP, r));
          break;
        case NIBP:
          crbpio_send_msg(CRBP_SET_POWER, DEV_MASK_NIBP, 0x00, (r)?0x01:0x00, 0x00); // power on/off NiBP
          break;
        case CO2:
          dio_module_cmd(PD_ID_CO2, (r) ? SBDP_RST_MOD : SBDP_OFF_MOD);
          cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(CO2, r));
          break;
      }
    }
  }

  // clear all set alarms for disabled units
  alarm_cfg_t ac;
  alarm_cfg_get(&ac);
  for (i=0; i<NUM_VIEWS; i++)
  {
    if (!unit_isenabled(i))
    {
      int j;
      for (j=0; j<NUM_ALARMS; j++)
      {
        if (ac.alarm_msg[j].chno == i) alarm_clr(ac.alarm_msg[j].msgno);
      }
    }
  }
 // if (chmask == 0) mframe_command(MFRAME_HIDE, NULL);
}

static void mod_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
}

void menu_modules_openproc(void)
{
  int i, r;

  for (i=0; i<NUM_VIEWS; i++)
  {
    r = unit_isenabled(i);
    if (r < 0) continue;
    if (r)
    {
      chmask |= (1 << i);
      switch (i)
      {
        case ECG:
          inpfoc_wnd_check(inpfoc_find(INPFOC_MOD, MOD_ECG), IWC_CHECKED);
          break;
        case SPO2:
          inpfoc_wnd_check(inpfoc_find(INPFOC_MOD, MOD_SPO2), IWC_CHECKED);
          break;
        case NIBP:
          inpfoc_wnd_check(inpfoc_find(INPFOC_MOD, MOD_NAD), IWC_CHECKED);
          break;
        case RESP:
          inpfoc_wnd_check(inpfoc_find(INPFOC_MOD, MOD_RESP), IWC_CHECKED);
          break;
        case T1T2:
          inpfoc_wnd_check(inpfoc_find(INPFOC_MOD, MOD_T), IWC_CHECKED);
          break;
        case ECG_NEO:
          inpfoc_wnd_check(inpfoc_find(INPFOC_MOD, MOD_ID1), IWC_CHECKED);
          break;
        case CO2:
          inpfoc_wnd_check(inpfoc_find(INPFOC_MOD, MOD_CO2), IWC_CHECKED);
          break;
        default:
          chmask &= ~(1<<i);
          fprintf(stderr, "%s: unknown chno (%d)\n", __FUNCTION__, i);
      }
    }
  }

 // debug("%s: chmask = 0x%X\n", __FUNCTION__, chmask);

  inpfoc_disable(INPFOC_MOD, MOD_ID1);
  inpfoc_disable(INPFOC_MOD, MOD_ID2);

  inpfoc_set(INPFOC_MOD, MOD_EXIT);
}

