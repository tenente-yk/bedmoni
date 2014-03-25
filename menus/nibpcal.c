#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "mframe.h"
#include "stconf.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "nibp.h"
#include "sched.h"
#include "nibpcal.h"

static void nibpcal_none_func(void*);
static void nibpcal_set0_func(void*);
static void nibpcal_set250_func(void*);
static void nibpcal_exit_func(void*);

inpfoc_funclist_t inpfoc_nibpcal_funclist[NIBPCAL_NUM_ITEMS+1] = 
{
  { NIBPCAL_NONE,      nibpcal_none_func         },
  { NIBPCAL_SET0,      nibpcal_set0_func         },
  { NIBPCAL_SET250,    nibpcal_set250_func       },
  { NIBPCAL_EXIT,      nibpcal_exit_func         },
  { -1       ,         nibpcal_none_func         }, // -1 must be last
};

static void nibpcal_none_func(void * parg)
{

}

static void nibpcal_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_NIBPSET);

  // closeproc will be called automatically by UFRAME
}

static void nibpcal_set0_func(void * parg)
{
 // nibp_calib(0);
  if (nibp_get_status() == 1)
  {
   // sched_start(SCHED_NIBP_REQDATA, NIBP_UPDATE_CUFFDATA_PERIOD, nibp_reqdata, SCHED_NORMAL);
    nibp_command(NIBP_CMD_GET_MODULE_DATA); // perform nibp module to send 'OK' packet, status -> 2
    nibp_service(1);
    inpfoc_enable(INPFOC_NIBPCAL, NIBPCAL_SET250);
   // inpfoc_set(INPFOC_NIBPCAL, NIBPCAL_SET250);
  }
  else
  {
    error("%s: control pneumatics failed\n", __FUNCTION__);
  }
}

static void nibpcal_set250_func(void * parg)
{
 // nibp_calib(250);
  if (nibp_get_status() == 5)
  {
    nibp_command(NIBP_CMD_GET_MODULE_DATA); // perform nibp module to send 'OK' packet, status -> 2
    nibp_service(1);
   // inpfoc_set(INPFOC_NIBPCAL, NIBPCAL_EXIT);
  }
  else
  {
    error("%s: control pneumatics failed\n", __FUNCTION__);
  }
}

void menu_nibpcal_openproc(void)
{
  char s[200];
  nibp_data_t nibp_data;

 /* nibp_service(1);
  nibp_close_valves();*/

  unit_get_data(NIBP, &nibp_data);
  if (*((int*)&nibp_data.trts_cal) == 0)
    ids2string(IDS_UNDEF7, s);
  else
    sprintf(s, "%02d.%02d.%04d", nibp_data.trts_cal.day, nibp_data.trts_cal.mon, nibp_data.trts_cal.year+1900);
  uframe_printbox(20, 437, -1, -1, s, UFRAME_STATICTEXT_COLOR);

  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_CUFF_PRESSURE, UNDEF_VALUE);

  inpfoc_disable(INPFOC_NIBPCAL, NIBPCAL_SET0);
  inpfoc_disable(INPFOC_NIBPCAL, NIBPCAL_SET250);

  inpfoc_set(INPFOC_NIBPCAL, NIBPCAL_EXIT);
}

void menu_nibpcal_closeproc(void)
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
