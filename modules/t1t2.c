#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bedmoni.h"
#include "iframe.h"
#include "unit.h"
#include "mframe.h"
#include "alarm.h"
#include "t1t2.h"
#include "dproc.h"

void t1t2_process_packet(unsigned char *data, int len)
{
  short t1, t2, dt;
  t1t2_data_t t1t2_data;
  unsigned char t1_st, t2_st, dt_st;

  if (unit_get_data(T1T2, &t1t2_data) <= 0)
  {
    error("%s: reading t1t2 data\n", __FUNCTION__);
  }

  t1 = (data[6]<<8 | data[5]);
  t2 = (data[4]<<8 | data[3]);
  dt = t2 - t1;
  if (dt < 0) dt = -dt;

 // debug("t1t2: %d %d\n", t1, t2);

  t1_st = t2_st = dt_st = 0;

  if (t1 < 0 || t1 > 500)
  {
    t1 = UNDEF_VALUE;
    t1t2_data.t1 = UNDEF_VALUE;
    dt = UNDEF_VALUE;
    alarm_clr(T1T2_RISKT1);
    alarm_clr(T1T2_RISKDT);
  }
  if (t2 < 0 || t2 > 500)
  {
    t2 = UNDEF_VALUE;
    t1t2_data.t2 = UNDEF_VALUE;
    dt = UNDEF_VALUE;
    alarm_clr(T1T2_RISKT2);
    alarm_clr(T1T2_RISKDT);
  }

  if ( (t1 < 0 || t1 > 500) && (t2 < 0 || t2 > 500) )
  {
    alarm_set(T1T2_NOSENSOR);
  }
  else
  {
    alarm_clr(T1T2_NOSENSOR);
  }

  if ( !(t1 < 0 || t1 > 500) )
  {
    if (t1 > t1t2_data.t1_max || t1 < t1t2_data.t1_min)
    {
    if      (t1 > t1t2_data.t1_max) t1_st |= TR_ST_U;
      else if (t1 < t1t2_data.t1_min) t1_st |= TR_ST_L;
      alarm_set(T1T2_RISKT1);
    }
    else
    {
      alarm_clr(T1T2_RISKT1);
    }
    t1t2_data.t1 = t1;
  }

  if ( !(t2 < 0 || t2 > 500) )
  {
    if (t2 > t1t2_data.t2_max || t2 < t1t2_data.t2_min)
    {
      if      (t2 > t1t2_data.t2_max) t2_st |= TR_ST_U;
      else if (t2 < t1t2_data.t2_min) t2_st |= TR_ST_L;
      alarm_set(T1T2_RISKT2);
    }
    else
    {
      alarm_clr(T1T2_RISKT2);
    }
    t1t2_data.t2 = t2;
  }

#if 0
  if ( !(t1 < 0 || t1 > 500) || !(t2 < 0 || t2 > 500) )
  {
    alarm_clr(T1T2_NOSENSOR);
  }
#endif

  if ( !(t1 < 0 || t1 > 500 || t2 < 0 || t2 > 500) )
  {
    if (dt > t1t2_data.dt_max)
    {
      if (dt > t1t2_data.dt_max) dt_st |= TR_ST_U;
      alarm_set(T1T2_RISKDT);
    }
    else
    {
      alarm_clr(T1T2_RISKDT);
    }
  }

  if (unit_set_data(T1T2, &t1t2_data) <= 0)
  {
    error("%s: writing t1t2 data\n", __FUNCTION__);
  }

  if (t1 != UNDEF_VALUE)
    dproc_add_trv(T1T2, PARAM_T1, t1, t1_st);
  else
    dproc_add_trv(T1T2, PARAM_T1, UNDEF_VALUE, 0);
  if (t2 != UNDEF_VALUE)
    dproc_add_trv(T1T2, PARAM_T2, t2, t2_st);
  else
    dproc_add_trv(T1T2, PARAM_T2, UNDEF_VALUE, 0);
  if (dt != UNDEF_VALUE)
    dproc_add_trv(T1T2, PARAM_DT, dt, dt_st);
  else
    dproc_add_trv(T1T2, PARAM_DT, UNDEF_VALUE, 0);

  t1t2_data.t1 = t1;
  t1t2_data.t2 = t2;

  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1, (t1!=UNDEF_VALUE) ? (float)t1/10 : (float)UNDEF_VALUE);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2, (t2!=UNDEF_VALUE) ? (float)t2/10 : (float)UNDEF_VALUE);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT, (dt!=UNDEF_VALUE) ? (float)dt/10 : (float)UNDEF_VALUE);
}

