/*! \file leaktst.c
    \brief Leak test popup frame
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "crbpio.h"
#include "uframe.h"
#include "mframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "sched.h"
#include "nibp.h"
#include "leaktst.h"

static void leaktst_none_func(void*);
static void leaktst_start_func(void*);
static void leaktst_exit_func(void*);

inpfoc_funclist_t inpfoc_leaktest_funclist[LEAKTEST_NUM_ITEMS+1] = 
{
  { LEAKTEST_NONE,       leaktst_none_func         },
  { LEAKTEST_START,      leaktst_start_func        },
  { LEAKTEST_EXIT,       leaktst_exit_func         },
  { -1           ,       leaktst_none_func         }, // -1 must be last
};

static void leaktst_none_func(void * parg)
{

}

static void leaktst_start_func(void * parg)
{
  if (nibp_get_status() == 1)
  {
    sched_start(SCHED_NIBP_REQDATA, NIBP_UPDATE_CUFFDATA_PERIOD, nibp_reqdata, SCHED_NORMAL);
    nibp_command(NIBP_CMD_GET_MODULE_DATA); // perform nibp module to send 'OK' packet, status -> 2
    nibp_service(0);
    inpfoc_disable(INPFOC_LEAKTEST, LEAKTEST_START);
    inpfoc_set(INPFOC_LEAKTEST, LEAKTEST_EXIT);
  }
  else
  {
    error("%s: control pneumatics failed\n", __FUNCTION__);
  }
}

static void leaktst_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_NIBPSET);

  // menu_leaktst_closeproc will be called by UFRAME
}

void menu_leaktst_openproc(void)
{
#if 0
  char s[200];

  ids2string(IDS_PRESS_START_1, s);
  uframe_printbox(20, 40, -1, -1, s, UFRAME_STATICTEXT_COLOR);
  ids2string(IDS_PRESS_START_2, s);
  uframe_printbox(20, 60, -1, -1, s, UFRAME_STATICTEXT_COLOR);
#endif
  nibp_service(1);
 // nibp_close_valves();

 // ids2string(UNDEF7, s);
 // uframe_printbox(20, 150, -1, -1, s, UFRAME_STATICTEXT_COLOR);

  inpfoc_set(INPFOC_LEAKTEST, LEAKTEST_EXIT);
}

void menu_leaktst_closeproc(void)
{
  nibp_data_t nibp_data;
  unsigned short ids;

  sched_stop(SCHED_NIBP);

  if (unit_get_data(NIBP, &nibp_data) <= 0)
  {
    debug("%s: error reading nibp data\n", __FUNCTION__);
  }

  if (nibp_data.meas_interval == NIBP_MEAS_INTERVAL_MANU)
  {
    ids = IDS_MANUALLY;
    sched_stop(SCHED_NIBP);
  }
  else
  {
    switch (nibp_meas_interval[nibp_data.meas_interval])
    {
      case 1:
        ids = IDS_1MIN;
        break;
      case 2:
        ids = IDS_2MIN;
        break;
      case 5:
        ids = IDS_5MIN;
        break;
      case 10:
        ids = IDS_10MIN;
        break;
      case 15:
        ids = IDS_15MIN;
        break;
      case 30:
        ids = IDS_30MIN;
        break;
      case 60:
        ids = IDS_60MIN;
        break;
      default:
       ids = IDS_UNDEF7;
    }
    sched_start(SCHED_NIBP, nibp_meas_interval[nibp_data.meas_interval]*60*1000, nibp_do_bp, SCHED_NORMAL);
  }
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_MEAS_INTERVAL, ids);

  sched_start(SCHED_NIBP_REQDATA, NIBP_UPDATE_CUFFDATA_PERIOD, nibp_reqdata, SCHED_NORMAL);

  nibp_service(0);

  unsigned int v;
  v = (ON<<0) | (OPEN<<8) | (OPEN<<16);
  nibp_command(NIBP_CMD_CONTROL_PNEUMATICS, v);
}
