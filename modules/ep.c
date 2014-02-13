/*! \file ep.c
 *  \brief ECG data processing
 */

// #include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bedmoni.h"
#include "dview.h"
#include "alarm.h"
#include "mframe.h"
#include "unit.h"
#include "ecgcalc.h"
#include "ecgset.h"
#include "tr.h"
#include "dproc.h"
#include "dio.h"
#include "sched.h"
#include "ep.h"

static int ep_good = 0;  // precise ecg calc algorith tuned if ep_good is true

void ep_process_packet(unsigned char *data, int len)
{
  ecg_pars_t ecg_pars;
  ecg_data_t ecg_data;
  unsigned char st;

  assert(len == sizeof(ecg_pars_t));

  // ECG
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    error("%s: reading ecg data\n", __FUNCTION__);
  }

  memcpy(&ecg_pars, data, len);

  if (ecg_pars.hr == ECGCALC_UNDEF_VALUE || ecg_pars.hr > 303 || ecg_pars.hr == 0)
  {
    ep_good = 0;
    alarm_clr(ECG_RISKHR);
   // unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, UNDEF_VALUE);
    if (ecg_data.hr_src == HRSRC_ECG || ecg_data.hr_src == HRSRC_AUTO)
    {
      ecg_data.hr = UNDEF_VALUE;
    }
  }
  else
  {
    ep_good = 1;
    st = 0;
    if (ecg_pars.hr > ecg_data.hr_max || ecg_pars.hr < ecg_data.hr_min)
    {
      if      (ecg_pars.hr > ecg_data.hr_max) st |= TR_ST_U;
      else if (ecg_pars.hr < ecg_data.hr_min) st |= TR_ST_L;
      alarm_set(ECG_RISKHR);
    }
    else
      alarm_clr(ECG_RISKHR);
    if (ecg_data.hr_src == HRSRC_ECG || ecg_data.hr_src == HRSRC_AUTO)
    {
      if (ecg_data.hr_src_curr != ECG && (!alarm_isset(ECG_ASYSTOLIA)))
      {
        unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HR, chancolor[ECG]);
        ecg_data.hr_src_curr = ECG;
      }
      if (!alarm_isset(ECG_ASYSTOLIA))
      {
        unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, ecg_pars.hr);
        ecg_data.hr = ecg_pars.hr;
      }
      else
        ecg_data.hr = UNDEF_VALUE;
    }
//     else
//       ep_good = 0;
    dproc_add_trv(ECG, PARAM_HR, ecg_pars.hr, st);
  }

  st = 0;
  if (ecg_pars.st[0] == ECGCALC_UNDEF_VALUE || ecg_data.num_leads == 1)
  {
    ecg_data.st[ECG_I-ECG_FIRST] = UNDEF_VALUE;
    alarm_clr(ECG_RISKSTI);
    dproc_add_trv(ECG, PARAM_STI, UNDEF_VALUE, st);
  }
  else
  {
    ecg_data.st[ECG_I-ECG_FIRST] = ecg_pars.st[0];
    if (ecg_data.st[ECG_I-ECG_FIRST] < ecg_data.st_min[ECG_I-ECG_FIRST] || ecg_data.st[ECG_I-ECG_FIRST] > ecg_data.st_max[ECG_I-ECG_FIRST])
    {
      if      (ecg_data.st[ECG_I-ECG_FIRST] < ecg_data.st_min[ECG_I-ECG_FIRST]) st |= TR_ST_L;
      else if (ecg_data.st[ECG_I-ECG_FIRST] > ecg_data.st_max[ECG_I-ECG_FIRST]) st |= TR_ST_U;
      alarm_set(ECG_RISKSTI);
    }
    else
      alarm_clr(ECG_RISKSTI);
    dproc_add_trv(ECG, PARAM_STI, ecg_data.st[ECG_I-ECG_FIRST], st);
  }

  st = 0;
  if (ecg_pars.st[1] == ECGCALC_UNDEF_VALUE)
  {
    ecg_data.st[ECG_II-ECG_FIRST] = UNDEF_VALUE;
    alarm_clr(ECG_RISKSTII);
    dproc_add_trv(ECG, PARAM_STII, UNDEF_VALUE, st);
  }
  else
  {
    ecg_data.st[ECG_II-ECG_FIRST] = ecg_pars.st[1];
    if (ecg_data.st[ECG_II-ECG_FIRST] < ecg_data.st_min[ECG_II-ECG_FIRST] || ecg_data.st[ECG_II-ECG_FIRST] > ecg_data.st_max[ECG_II-ECG_FIRST])
    {
      if      (ecg_data.st[ECG_II-ECG_FIRST] < ecg_data.st_min[ECG_II-ECG_FIRST]) st |= TR_ST_L;
      else if (ecg_data.st[ECG_II-ECG_FIRST] > ecg_data.st_max[ECG_II-ECG_FIRST]) st |= TR_ST_U;
      alarm_set(ECG_RISKSTII);
    }
    else
      alarm_clr(ECG_RISKSTII);
    dproc_add_trv(ECG, PARAM_STII, ecg_data.st[ECG_II-ECG_FIRST], st);
  }

  st = 0;
  if (ecg_pars.st[2] == ECGCALC_UNDEF_VALUE || ecg_data.num_leads == 1)
  {
    ecg_data.st[ECG_III-ECG_FIRST] = UNDEF_VALUE;
    alarm_clr(ECG_RISKSTIII);
    dproc_add_trv(ECG, PARAM_STIII, UNDEF_VALUE, st);
  }
  else
  {
    ecg_data.st[ECG_III-ECG_FIRST] = ecg_pars.st[2];
    if (ecg_data.st[ECG_III-ECG_FIRST] < ecg_data.st_min[ECG_III-ECG_FIRST] || ecg_data.st[ECG_III-ECG_FIRST] > ecg_data.st_max[ECG_III-ECG_FIRST])
    {
      if      (ecg_data.st[ECG_III-ECG_FIRST] < ecg_data.st_min[ECG_III-ECG_FIRST]) st |= TR_ST_L;
      else if (ecg_data.st[ECG_III-ECG_FIRST] > ecg_data.st_max[ECG_III-ECG_FIRST]) st |= TR_ST_U;
      alarm_set(ECG_RISKSTIII);
    }
    else
      alarm_clr(ECG_RISKSTIII);
    dproc_add_trv(ECG, PARAM_STIII, ecg_data.st[ECG_III-ECG_FIRST], st);
  }

  st = 0;
  if (ecg_pars.st[3] == ECGCALC_UNDEF_VALUE || ecg_data.num_leads == 1)
  {
    ecg_data.st[ECG_aVR-ECG_FIRST] = UNDEF_VALUE;
    alarm_clr(ECG_RISKSTAVR);
    dproc_add_trv(ECG, PARAM_STAVR, UNDEF_VALUE, st);
  }
  else
  {
    ecg_data.st[ECG_aVR-ECG_FIRST] = ecg_pars.st[3];
    if (ecg_data.st[ECG_aVR-ECG_FIRST] < ecg_data.st_min[ECG_aVR-ECG_FIRST] || ecg_data.st[ECG_aVR-ECG_FIRST] > ecg_data.st_max[ECG_aVR-ECG_FIRST])
    {
      if      (ecg_data.st[ECG_aVR-ECG_FIRST] < ecg_data.st_min[ECG_aVR-ECG_FIRST]) st |= TR_ST_L;
      else if (ecg_data.st[ECG_aVR-ECG_FIRST] > ecg_data.st_max[ECG_aVR-ECG_FIRST]) st |= TR_ST_U;
      alarm_set(ECG_RISKSTAVR);
    }
    else
      alarm_clr(ECG_RISKSTAVR);
    dproc_add_trv(ECG, PARAM_STAVR, ecg_data.st[ECG_aVR-ECG_FIRST], st);
  }

  st = 0;
  if (ecg_pars.st[4] == ECGCALC_UNDEF_VALUE || ecg_data.num_leads == 1)
  {
    ecg_data.st[ECG_aVL-ECG_FIRST] = UNDEF_VALUE;
    alarm_clr(ECG_RISKSTAVL);
    dproc_add_trv(ECG, PARAM_STAVL, UNDEF_VALUE, st);
  }
  else
  {
    ecg_data.st[ECG_aVL-ECG_FIRST] = ecg_pars.st[4];
    if (ecg_data.st[ECG_aVL-ECG_FIRST] < ecg_data.st_min[ECG_aVL-ECG_FIRST] || ecg_data.st[ECG_aVL-ECG_FIRST] > ecg_data.st_max[ECG_aVL-ECG_FIRST])
    {
      if      (ecg_data.st[ECG_aVL-ECG_FIRST] < ecg_data.st_min[ECG_aVL-ECG_FIRST]) st |= TR_ST_L;
      else if (ecg_data.st[ECG_aVL-ECG_FIRST] > ecg_data.st_max[ECG_aVL-ECG_FIRST]) st |= TR_ST_U;
      alarm_set(ECG_RISKSTAVL);
    }
    else
      alarm_clr(ECG_RISKSTAVL);
    dproc_add_trv(ECG, PARAM_STAVL, ecg_data.st[ECG_aVL-ECG_FIRST], st);
  }

  st = 0;
  if (ecg_pars.st[5] == ECGCALC_UNDEF_VALUE || ecg_data.num_leads == 1)
  {
    ecg_data.st[ECG_aVF-ECG_FIRST] = UNDEF_VALUE;
    alarm_clr(ECG_RISKSTAVF);
    dproc_add_trv(ECG, PARAM_STAVF, UNDEF_VALUE, st);
  }
  else
  {
    ecg_data.st[ECG_aVF-ECG_FIRST] = ecg_pars.st[5];
    if (ecg_data.st[ECG_aVF-ECG_FIRST] < ecg_data.st_min[ECG_aVF-ECG_FIRST] || ecg_data.st[ECG_aVF-ECG_FIRST] > ecg_data.st_max[ECG_aVF-ECG_FIRST])
    {
      if      (ecg_data.st[ECG_aVF-ECG_FIRST] < ecg_data.st_min[ECG_aVF-ECG_FIRST]) st |= TR_ST_L;
      else if (ecg_data.st[ECG_aVF-ECG_FIRST] > ecg_data.st_max[ECG_aVF-ECG_FIRST]) st |= TR_ST_U;
      alarm_set(ECG_RISKSTAVF);
    }
    else
      alarm_clr(ECG_RISKSTAVF);
    dproc_add_trv(ECG, PARAM_STAVF, ecg_data.st[ECG_aVF-ECG_FIRST], st);
  }

  st = 0;
  if (ecg_pars.st[6] == ECGCALC_UNDEF_VALUE || ecg_data.num_leads == 1 || ecg_data.num_leads == 3)
  {
    ecg_data.st[ECG_V-ECG_FIRST] = UNDEF_VALUE;
    alarm_clr(ECG_RISKSTV);
    dproc_add_trv(ECG, PARAM_STV, UNDEF_VALUE, st);
  }
  else
  {
    ecg_data.st[ECG_V-ECG_FIRST] = ecg_pars.st[6];
    if (ecg_data.st[ECG_V-ECG_FIRST] < ecg_data.st_min[ECG_V-ECG_FIRST] || ecg_data.st[ECG_V-ECG_FIRST] > ecg_data.st_max[ECG_V-ECG_FIRST])
    {
      if      (ecg_data.st[ECG_V-ECG_FIRST] < ecg_data.st_min[ECG_V-ECG_FIRST]) st |= TR_ST_L;
      else if (ecg_data.st[ECG_V-ECG_FIRST] > ecg_data.st_max[ECG_V-ECG_FIRST]) st |= TR_ST_U;
      alarm_set(ECG_RISKSTV);
    }
    else
      alarm_clr(ECG_RISKSTV);
    dproc_add_trv(ECG, PARAM_STV, ecg_data.st[ECG_V-ECG_FIRST], st);
  }

  if (!alarm_isset(ECG_ASYSTOLIA))
  {
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_VALUE, (int)ecg_data.st[ecg_data.st1-ECG_FIRST]);
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_VALUE, (int)ecg_data.st[ecg_data.st2-ECG_FIRST]);
  }

  if (unit_set_data(ECG, &ecg_data) <= 0)
  {
    error("%s: writing ecg data\n", __FUNCTION__);
  }
}

int ep_is_ok(void)
{
  return ep_good;
}

void ecg_on_no_ep(void)
{
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);

  if ((ecg_data.break_byte & 0xF) == 0 && unit_isenabled(ECG)) // no breaks
  {
    // restart 3s task (EP)
    error("3s ecgcalc restarted\n");

    dio_module_cmd(PD_ID_ECS, ECS_RESTART_EP);
  }
  else
  {
  }
  sched_start(SCHED_EP_WAIT, 10000, ecg_on_no_ep, SCHED_NORMAL);
}
