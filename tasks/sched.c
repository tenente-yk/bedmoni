#ifdef UNIX
#include <unistd.h>
#include <pthread.h>
#endif
#ifdef WIN32
//#include <windows.h>
#endif
#include <stdio.h>
#include <assert.h>
#include "bedmoni.h"
#include "dview.h"
#include "dproc.h"
#include "mframe.h"
#include "sched.h"
#include "ecgm.h"
#include "nibp.h"
#include "tprn.h"
#include "alarm.h"
#include "iframe.h"
#include "resp.h"
#if defined (RESPCALC_COLIBRI)
#include "respcalc.h"
#endif
#include "ep.h"
#include "csio.h"
#include "crbpio.h"

static sched_task_t sched_task[SCHED_NUM_TASKS];
static int sched_suspended = 0;

static void sched_stepit(int id, unsigned long t1);

int  sched_init(void)
{
  int i;
#ifndef NIBP_DISABLED
  nibp_data_t nibp_data;
#endif
  ecg_data_t ecg_data;

  for (i=0; i<SCHED_NUM_TASKS; i++)
    sched_task[i].active = 0;

#ifndef NIBP_DISABLED
  debug("nibp io port: %s\n", NIBP_COMMPORT);
  if ( nibp_init(NIBP_COMMPORT) != NIBP_ERR_OK)
  {
    error("unable to open nibp comm port %s\n", NIBP_COMMPORT);
  }

  nibp_command(NIBP_CMD_GET_RETURN_STRING);

  unit_get_data(NIBP, &nibp_data);
  if (nibp_data.meas_interval != NIBP_MEAS_INTERVAL_MANU)
  {
    sched_start(SCHED_NIBP, nibp_meas_interval[nibp_data.meas_interval]*60*1000, nibp_do_bp, SCHED_NORMAL);
  }
#else
  debug("nibp io disabled!!!\n");
#endif // NIBP_DISABLED

  sched_start(SCHED_ALARMS, 1000, alarm_update, SCHED_NORMAL);
 // sched_start(SCHED_ALARMS, 1000, mframe_process_alarms);
 // sched_start(SCHED_IFRAME_MSGS, 1000, iframe_process_msgs);
  sched_start(SCHED_CRBP_RESP_STAT, 100, crbp_resp_stat, SCHED_NORMAL);
#ifndef NIBP_DISABLED
  sched_start(SCHED_NIBP_INPUT, NIBP_PROCESS_INPUT_PERIOD, nibp_process_input, SCHED_NORMAL);
  sched_start(SCHED_NIBP_REQDATA, NIBP_UPDATE_CUFFDATA_PERIOD, nibp_reqdata, SCHED_NORMAL);
#endif

  unit_get_data(ECG, &ecg_data);
  sched_start(SCHED_QRS_WAIT, ecg_data.asys_sec*1000, ecg_on_no_qrs, SCHED_NORMAL);
  sched_start(SCHED_EP_WAIT, 5000, ecg_on_no_ep, SCHED_NORMAL);
#ifndef TPRN_DISABLED
  sched_start(SCHED_TPRN, TPRN_TS_PERIOD_MS, tprn_stepit, SCHED_NORMAL);
#endif
  sched_start(SCHED_CS_SLOW, 500, csio_send_slow_data, SCHED_NORMAL); // every 500 ms

  return 0;
}

void sched_deinit(void)
{
  int i;

  for (i=0; i<SCHED_NUM_TASKS; i++)
    sched_stop(i);

#ifndef NIBP_DISABLED
  nibp_deinit();
#endif
}

void sched_reinit(void)
{
  unsigned int t0, i;
  t0 = gettimemillis();
  for (i=0; i<SCHED_NUM_TASKS; i++)
  {
    if (sched_task[i].active)
    {
      sched_task[i].active = 0;
      sched_task[i].t0 = t0;
      sched_task[i].active = 1;
    }
  }
}

void sched_update(void)
{
  unsigned long t1;
  int i;
  if (sched_suspended) return;
  t1 = gettimemillis();
  for (i=0; i<SCHED_NUM_TASKS; i++)
  {
    sched_stepit(i, t1);
  }
}

void sched_start(int id, int reload_time_ms, SCHEDPROC proc, unsigned long flags)
{
  assert(id < SCHED_NUM_TASKS);
  assert(proc);

  if (id == SCHED_ANY)
  {
    int i;
    for (i=SCHED_MISC1; i<(SCHED_MISC_LAST+1); i++)
    {
      if (sched_task[i].active && sched_task[i].func == proc)
      {
        sched_task[i].t0 = gettimemillis();
        sched_task[i].dly_ms = reload_time_ms;
        sched_task[i].flags = flags;
        return;
      }
    }
    for (i=SCHED_MISC1; i<(SCHED_MISC_LAST+1); i++)
    {
      if (sched_task[i].active == 0) break;
    }
    if (i > SCHED_MISC_LAST)
    {
      error("sched_start ignored, cause no any free sched ids\n");
      return;
    }
    id = i;
  }

  sched_stop(id);

  sched_task[id].dly_ms = reload_time_ms;
  sched_task[id].t0 = gettimemillis();
  sched_task[id].func = proc;
  sched_task[id].flags = flags;
  sched_task[id].active = 1;
}

void sched_stop(int id)
{
  assert(id < SCHED_NUM_TASKS);
  sched_task[id].active = 0;
}

static void sched_stepit(int id, unsigned long t1)
{
  assert(id < SCHED_NUM_TASKS);
  if (sched_is_started(id))
  {
    if ((long)(t1 - sched_task[id].t0) > (long)sched_task[id].dly_ms)
    {
      sched_exec(id);
      if (sched_task[id].flags & SCHED_DO_ONCE) sched_stop(id);
    }
  }
}

int sched_is_started(int id)
{
  assert(id < SCHED_NUM_TASKS);
  return (sched_task[id].active);
}

void sched_exec(int id)
{
  assert(id < SCHED_NUM_TASKS);
//  sched_task[id].active = 0;
  sched_task[id].func();
  sched_task[id].t0 = gettimemillis();
//  sched_task[id].active = 1;
}

void sched_suspend(void)
{
  sched_suspended = 1;
}

void sched_resume(void)
{
#if defined (RESPCALC_COLIBRI)
  respcalc_reset();
#endif

  sched_reinit();
  sched_suspended = 0;
}
