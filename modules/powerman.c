/*! \file powerman.c
 *  \brief Power management
 */

#ifdef UNIX
#include <unistd.h>
#endif
#include <string.h>
#include "bedmoni.h"
#include "crbpio.h"
#include "cframe.h"
#include "mframe.h"
#include "resp.h"
#if defined (RESPCALC_COLIBRI)
#include "respcalc.h"
#endif
#include "demo.h"
#include "sched.h"
#include "dio.h"
#include "demo.h"
#include "port.h"
#include "powerman.h"

static unsigned long power_status;

void powerman_stat(unsigned long val)
{
  power_status = val;
}

void powerman_stat_get(unsigned long * pv)
{
  if (pv) *pv = power_status;
}

void sbdp_reset(void)
{
  static int already_here = 0;
#ifdef ARM
  demo_cfg_t dcfg;
  int fd;
  int timeout;
#endif

#ifdef ARM
  // fix: unrecognized issue -> when TPRN starts sometimes sbdp_reset call happens
  tprn_cfg_t tprn_cfg;
  tprn_cfg_get(&tprn_cfg);
  if (tprn_cfg.prnmask.started)
  {
    debug("reset sbdp skipped (%s, line %d)\n", __FILE__, __LINE__);
    return;
  }
#endif

#ifdef ARM
  demo_cfg_get(&dcfg);
  if (dcfg.demo)
  {
    debug("reset sbdp in demo skipped\n");
    return;
  }
#else
  debug("reset sbdp in PC version disabled\n");
#endif

  if (already_here) return;
  already_here = 1;

  sched_stop(SCHED_QRS_WAIT);

  debug("reset sbdp\n");
#ifdef ARM
  dio_deinit();
  crbpio_send_msg(CRBP_SET_POWER, DEV_MASK_SBDP, 0x00, 0x00, 0x00); // sbdp off

  timeout = 30; // 3 sec
  do
  {
    debug("goodbye %s\n", DEV);
    fd = com_open(DEV, BR115200, 0);
    if (fd > 0)
      com_close(fd);
    else
      break;
#ifdef UNIX
    usleep(100*1000);
#endif
    timeout --;
    crbp_resp_stat();
  } while (timeout > 0);
  crbpio_send_msg(CRBP_SET_POWER, DEV_MASK_SBDP, 0x00, 0x01, 0x00); // sbdp on
  timeout = 25; // 5 sec
  do
  {
    debug("wait for %s\n", DEV);
#ifdef UNIX
    fd = com_open(DEV, BR115200, 0);
    if (fd > 0)
    {
      com_close(fd);
      break;
    }
    usleep(200*1000);
#endif
    timeout --;
    crbp_resp_stat();
  } while (timeout > 0);
  dcfg.demo = 0;
  demo_cfg_set(&dcfg);
  dio_init();
  cframe_command(CFRAME_RESET, NULL); // clear DEMO title from cframe

  // set startup info for further restore after reset
  startup_info_t si;
  ecg_data_t ecg_data;
  memset(&si, 0, sizeof(startup_info_t));
  unit_get_data(ECG, &ecg_data);
  si.ecg_set = ecg_data.set;
  mframe_command(MFRAME_SET_STARTUP_CFG, (void*)&si);

 // resp_soft_hw_reset();
  mframe_command(MFRAME_STARTUP, NULL);

#if defined (RESPCALC_COLIBRI)
  respcalc_reset();
#endif

#else // !ARM
#endif
  already_here = 0;
}

void lowbat_shutdown(void)
{
  debug("accum is empty, shutting down...\n");
  safe_exit(EXIT_LOWBAT);
}

