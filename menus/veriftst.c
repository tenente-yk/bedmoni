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
#include "veriftst.h"

static void veriftst_none_func(void*);
static void veriftst_start_func(void*);
static void veriftst_exit_func(void*);

inpfoc_funclist_t inpfoc_veriftest_funclist[VERIF_NUM_ITEMS+1] = 
{
  { VERIF_NONE,       veriftst_none_func         },
  { VERIF_START,      veriftst_start_func        },
  { VERIF_EXIT,       veriftst_exit_func         },
  { -1        ,       veriftst_none_func         }, // -1 must be last
};

static void veriftst_none_func(void * parg)
{

}

static void veriftst_start_func(void * parg)
{
  if (nibp_get_status() == 1)
  {
    sched_start(SCHED_NIBP_REQDATA, NIBP_UPDATE_CUFFDATA_PERIOD, nibp_reqdata, SCHED_NORMAL);
    nibp_command(NIBP_CMD_GET_MODULE_DATA); // perform nibp module to send 'OK' packet, status -> 2
    nibp_service(0);
    inpfoc_disable(INPFOC_VERIF, VERIF_START);
    inpfoc_set(INPFOC_VERIF, VERIF_EXIT);
  }
  else
  {
    error("%s: control pneumatics failed\n", __FUNCTION__);
  }
}

static void veriftst_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_NIBPSET);

  // closeproc will be called auyomatically by UFRAME
}

void menu_veriftst_openproc(void)
{
  char s[200];

#if 0
  ids2string(IDS_PRESS_START_1, s);
  uframe_printbox(20, 40, -1, -1, s, UFRAME_STATICTEXT_COLOR);
  ids2string(IDS_PRESS_START_2, s);
  uframe_printbox(20, 60, -1, -1, s, UFRAME_STATICTEXT_COLOR);
#endif

  nibp_service(1);
 // nibp_close_valves();

  ids2string(IDS_CURRENT_CUFF, s);
  uframe_printbox(20, 150, -1, -1, s, UFRAME_STATICTEXT_COLOR);

  inpfoc_set(INPFOC_VERIF, VERIF_EXIT);
}

void menu_veriftst_closeproc(void)
{
  nibp_data_t nibp_data;
  unsigned short ids;

  nibp_command(NIBP_CMD_GET_MODULE_DATA); // perform nibp module to send 'OK' packet, status -> 2
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
  v = (OFF<<0) | (OPEN<<8) | (OPEN<<16);
  nibp_command(NIBP_CMD_CONTROL_PNEUMATICS, v);
}
