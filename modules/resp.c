/*! \file resp.c
    \brief Resp data interface
 */

#include <string.h>
#include "dview.h"
#include "mframe.h"
#include "bedmoni.h"
#include "sched.h"
#include "dproc.h"
#include "resp.h"

#pragma pack(1)
static struct
{
  unsigned char apnoe : 1;
  unsigned char br    : 7;
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
resp_pars;
#pragma pack()

void resp_process_packet(unsigned char * data, int len)
{
  resp_data_t resp_data;
  ecg_data_t ecg_data;
  unsigned char st;
  unsigned char undef_state;

  if (len != 1) return;

  if (unit_get_data(RESP, &resp_data) <= 0)
  {
    error("%s: reading resp data\n", __FUNCTION__);
  }

  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    error("%s: reading ecg data\n", __FUNCTION__);
  }

  resp_pars.br = 0x7F;
  resp_pars.apnoe = 0;
  undef_state = 0;

  if ( alarm_isset(RESP_BADCONTACT_RL) || alarm_isset(RESP_BADCONTACT_RF) ||
       ((ecg_data.break_byte & 0x3) != 0 && ecg_data.set_bits.breath_ch == 0) || // ignore C_BREAK & F_BREAK
       ((ecg_data.break_byte & 0x5) != 0 && ecg_data.set_bits.breath_ch == 1)    // ignore C_BREAK & L_BREAK
     )
  {
    resp_pars.br = 0x7F;
    resp_pars.apnoe = 0;
    undef_state = 1;
  }
  else
  {
    // process resp data
    resp_pars.br = (data[0]>>1)&0x7F;
    resp_pars.apnoe = data[0]&0x1;
  }

  resp_data.br = (resp_pars.br != 0x7F) ? resp_pars.br : UNDEF_VALUE;
  resp_data.ap = resp_pars.apnoe;

  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR, resp_data.br);

  st = 0;
  if (resp_data.br != UNDEF_VALUE)
  {
    if      (resp_data.br < resp_data.br_min) st |= TR_ST_L;
    else if (resp_data.br > resp_data.br_max) st |= TR_ST_U;
  }

  alarm_set_clr(RESP_RISKBR, st);

  alarm_set_clr(RESP_APNOE, resp_data.ap);
  if (undef_state)
    unit_ioctl(RESP, SET_VALUE, UNIT_RESP_STATE, IDS_UNDEF7);
  else
    unit_ioctl(RESP, SET_VALUE, UNIT_RESP_STATE, (resp_data.ap)?IDS_APNOE:IDS_NORM);

  dproc_add_trv(RESP, PARAM_BR, resp_data.br, st);

  if (unit_set_data(RESP, &resp_data) <= 0)
  {
    error("%s: writing resp data\n", __FUNCTION__);
  }
}

void resp_soft_hw_reset(void)
{
  ecgm_command(ECGM_RESP_TAU_320);
  // 6.8s mode will be set automatically (see ecgm.c)
}

