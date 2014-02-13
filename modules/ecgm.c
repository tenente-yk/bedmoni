/*! \file ecgm.c
    \brief ECG module fuctionality
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bedmoni.h"
#include "iframe.h"
#include "unit.h"
#include "mframe.h"
#include "alarm.h"
#include "filter.h"
#include "dproc.h"
#include "csio.h"
#include "resp.h"
#if defined (RESPCALC_COLIBRI)
#include "respcalc.h"
#endif
#include "cframe.h"
#include "ecgm.h"
#include "ecgset.h"
#include "ecgcalc.h"
#include "sched.h"
#include "respset.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "dio.h"
#include "powerman.h"

extern int filt_resp_id; // declared and inited in dproc

/**
 * \brief ECG module command structure to be passed in queue
 */
typedef struct
{
  unsigned char cmd;    /**< command */
  signed char   cbit;   /**< bit has to be cleared */
  signed char   sbit;   /**< bit has to be set */
} ecgm_cmd_list_t;

/*! \var
    \brief ECG commands list with bits description to be set or clear by ECG borad
*/
static ecgm_cmd_list_t ecgm_cmd_list[] =
{
  { ECGM_N_TO_LINE,       3, -1 },
  { ECGM_N_TO_GND,       -1,  3 },
  { ECGM_ECG_TAU_3200,    0, -1 },
  { ECGM_ECG_TAU_320,    -1,  0 },
  { ECGM_ECG_TAU_160,    -1,  1 },
  { ECGM_RESP_TAU_6800,   2, -1 },
  { ECGM_RESP_TAU_320,   -1,  2 },
  { ECGM_RESP_ON,        -1, -1 },
  { ECGM_RESP_OFF,       -1, -1 },
  { ECGM_DHF_ON,         -1,  4 },
  { ECGM_DHF_OFF,         4, -1 },
  { ECGM_DATA_OFF,       -1, -1 },
  { ECGM_DATA_RESET,     -1, -1 },
  { ECGM_RESP_CHAN_RL,    5, -1 },
  { ECGM_RESP_CHAN_RF,   -1,  5 },
};

/*! \struct
    \brief structure to be analyzed in queue
*/
typedef struct
{
  unsigned char cmd;
  unsigned char retry_count;
  signed char   cbit;
  signed char   sbit;
  unsigned char running;
} ecgm_cmd_ex_t;

static ecgm_cmd_ex_t ecgm_cmd_ex[ECGM_MAX_NUM_CMDS_IN_QUEUE];
static int ecgm_num_cmds_running = 0;
static int ecgm_demo_mode = 0;

unsigned short ecg_mode_ids[MAXNUM_ECG_MODES] = {IDS_DIAGNOSTICS, IDS_DIAGNOSTICS_F, IDS_MONITORING, IDS_SURGERY};

//static unsigned char cur_mode = 0xFF;

//void ecg_pm_no_answer(void);
void ecgm_analyze_input(void);

void ecgm_process_packet(unsigned char *data, int len)
{
  ecgm_packet_t ecgm_packet, *pecgm_packet;
  ecg_data_t ecg_data;
  resp_data_t resp_data;
  static unsigned char old_sync = 0;
  static unsigned char old_set = 0xFF;
  static unsigned char old_break = 0xFF;
  int ev[NUM_ECG];
  int i, v;
  char s[128];
  static int break_countdown = 100;
  static unsigned char pacemaker_valid[2] = {0, 0};
  static int startup_delay_counter = 2*500; // 2 sec
  static int resp_hw_reset_counter = 0;
  unsigned char b;

#if 0
static FILE * f = 0;
//if (!f) f = fopen("60601-2-51-CAL20160-50HZ.bin", "wb");
if (!f) f = fopen("nsft-ecg.bin", "wb");
if (f)
{
 fwrite(data,1,len,f);
 fflush(f);
}
#endif

  unit_get_data(ECG, &ecg_data);

  if ((unsigned long)data & 0x1)
  {
    pecgm_packet = &ecgm_packet;
    memcpy(pecgm_packet, data, sizeof(ecgm_packet_t));
  }
  else
    pecgm_packet = (ecgm_packet_t*)data;

  if ((unsigned char)(pecgm_packet->sync - old_sync) != 1)
  {
    error("ECG sync error: %02X %02X\n", old_sync, pecgm_packet->sync);
  }
  old_sync = pecgm_packet->sync;

  if (ecg_data.num_leads == 3) pecgm_packet->break_bits.c = 0;
  if (ecg_data.num_leads == 1) pecgm_packet->break_bits.l = pecgm_packet->break_bits.c = 0;

  alarm_set_clr(ECG_RBREAK, pecgm_packet->break_bits.r);
  alarm_set_clr(ECG_LBREAK, pecgm_packet->break_bits.l);
  alarm_set_clr(ECG_FBREAK, pecgm_packet->break_bits.f);
  alarm_set_clr(ECG_CBREAK, pecgm_packet->break_bits.c);

  ev[ECG_I-ECG_1]   = (pecgm_packet->break_bits.l || pecgm_packet->break_bits.r) ? 0 : *((short*)(pecgm_packet->data)+0);
  ev[ECG_II-ECG_1]  = (pecgm_packet->break_bits.f || pecgm_packet->break_bits.r) ? 0 : *((short*)(pecgm_packet->data)+1);
  ev[ECG_III-ECG_1] = (pecgm_packet->break_bits.f || pecgm_packet->break_bits.l) ? 0 : ev[ECG_II-ECG_1] - ev[ECG_I-ECG_1];
  ev[ECG_aVR-ECG_1] = (pecgm_packet->break_bits.f || pecgm_packet->break_bits.r || pecgm_packet->break_bits.l) ? 0 : (-ev[ECG_II-ECG_1]-ev[ECG_I-ECG_1]) / 2;
  ev[ECG_aVL-ECG_1] = (pecgm_packet->break_bits.f || pecgm_packet->break_bits.r || pecgm_packet->break_bits.l) ? 0 : (2*ev[ECG_I-ECG_1] - ev[ECG_II-ECG_1]) / 2;
  ev[ECG_aVF-ECG_1] = (pecgm_packet->break_bits.f || pecgm_packet->break_bits.r || pecgm_packet->break_bits.l) ? 0 : ev[ECG_I-ECG_1] / 2;
  ev[ECG_V-ECG_1]   = (pecgm_packet->break_bits.c || pecgm_packet->break_bits.l || pecgm_packet->break_bits.r || pecgm_packet->break_bits.f) ? 0 : *((short*)(pecgm_packet->data)+2) - (ev[ECG_I-ECG_1] + ev[ECG_II-ECG_1]) / 3;

  if (old_break != pecgm_packet->break_byte)
  {
    ecg_data.break_byte = pecgm_packet->break_byte;
    unit_set_data(ECG, &ecg_data);
    old_break = pecgm_packet->break_byte;
    // reload asystolia detector
    if (pecgm_packet->break_byte == 0)
      sched_start(SCHED_QRS_WAIT, ecg_data.asys_sec*1000, ecg_on_no_qrs, SCHED_NORMAL);
//    if (pecgm_packet->break_byte != 0)
//      sched_stop(SCHED_QRS_WAIT);

    b = ((ecg_data.break_byte & 0x3) != 0 && ecg_data.set_bits.breath_ch == 0) || // RL
        ((ecg_data.break_byte & 0x5) != 0 && ecg_data.set_bits.breath_ch == 1);   // RF
    alarm_set_clr(RESP_BREAK, b);
    if (b)
    {
#if defined (RESPCALC_COLIBRI)
      respcalc_reset();
#endif
      unsigned char b = 0xFE;
      resp_process_packet(&b, 1);
    }
  }

  if (pecgm_packet->break_byte != 0) break_countdown = 100;
  else if (break_countdown) break_countdown --;

  if (pecgm_packet->break_bits.l) pecgm_packet->pacemaker_bits.l = 0;
  if (pecgm_packet->break_bits.f) pecgm_packet->pacemaker_bits.f = 0;
  if (pecgm_packet->break_bits.c) pecgm_packet->pacemaker_bits.c = 0;
  if (ecg_data.num_leads == 3) pecgm_packet->pacemaker_bits.c = 0;
  if (ecg_data.num_leads == 1) pecgm_packet->pacemaker_bits.l = pecgm_packet->pacemaker_bits.c = 0;

  // !!! this code makes delay 2*10 ms for the pacemaker mark, see pm drawing in cframe update !!!
  if (pacemaker_valid[1]) pacemaker_valid[1] ++;
  if (pecgm_packet->pacemaker != 0 && pecgm_packet->break_byte == 0)
  {
    pacemaker_valid[0] = pecgm_packet->pacemaker;
    pacemaker_valid[1] = 1;
    pecgm_packet->pacemaker = 0;
  }
  if (pecgm_packet->break_byte) pacemaker_valid[1] = 0;
  if (pacemaker_valid[1] == 10)
  {
    pacemaker_valid[1] = 0;
    pecgm_packet->pacemaker = pacemaker_valid[0];
    pacemaker_valid[0] = 0;
  }

#if 0
  if (pecgm_packet->break_byte)
  {
    pecgm_packet->pacemaker = 0;
    if (sched_is_started(SCHED_PMBEAT))
    {
      sched_stop(SCHED_PMBEAT);
      unit_hide_pm();
    }
  }
#endif

  if (break_countdown || startup_delay_counter) pecgm_packet->pacemaker = 0;
  if (startup_delay_counter) startup_delay_counter --;

  if (pecgm_packet->pacemaker)
  {
    debug("pm %X br %X\n", pecgm_packet->pacemaker, pecgm_packet->break_byte);
  }

  if ( pecgm_packet->pacemaker != 0)
  {
#if 0 //defined (RESPCALC_COLIBRI)
    respcalc_reset(); // now we are not able to process breath with pacemaker
                      // TODO: maybe need to perform this feature
#endif
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_PM_IMG, IMAGE_PM);
    sched_start(SCHED_PMBEAT, 4000, unit_hide_pm, SCHED_NORMAL);

    // no answer for PM arrhythmia detection is implemented in SBDP
   /* if (!sched_is_started(SCHED_PM_NORESP))
    {
      unit_get_data(ECG, &ecg_data);
      sched_start(SCHED_PM_NORESP, ecg_data.pmnr_ms, ecg_pm_no_answer);
    }*/
  }

  for (i=0; i<NUM_ECG; i++)
  {
    ev[i] = MAKELONG((signed short)(ev[i]), pecgm_packet->pacemaker);
  }

  dview_chandata_add(ECG_I,   ev[ECG_I-ECG_1]);
  dview_chandata_add(ECG_II,  ev[ECG_II-ECG_1]);
  dview_chandata_add(ECG_III, ev[ECG_III-ECG_1]);
  dview_chandata_add(ECG_aVF, ev[ECG_aVF-ECG_1]);
  dview_chandata_add(ECG_aVR, ev[ECG_aVR-ECG_1]);
  dview_chandata_add(ECG_aVL, ev[ECG_aVL-ECG_1]);
  dview_chandata_add(ECG_V,   ev[ECG_V-ECG_1]);

#if 0
  static int cnt = 0;
  static double f = 0.2;
  *((short*)(pecgm_packet->data)+3) += 10000*sin((double)2*3.1415926*f*cnt*0.002);
  cnt ++;
#endif

  // filter RPG
 // *((short*)(pecgm_packet->data)+3) = resp_filt_stepit(filt_resp_id, (int) *((short*)(pecgm_packet->data)+3));

 // dview_chandata_add(RPG, *((short*)(pecgm_packet->data)+3) * 0.05);

  static int timer_after_soft_reset = 5*FD_ECG; // 5 sec
  v = *((short*)(pecgm_packet->data)+3) * 2;
  if (timer_after_soft_reset > 0) timer_after_soft_reset --;
  if (v < -32000 || v > 32000)
  {
    if (timer_after_soft_reset == 0)
    {
      timer_after_soft_reset = 5*FD_ECG; // 5 sec
      resp_soft_hw_reset();
    }
  }

#if defined (RESPCALC_COLIBRI)
  // add new RPG value for processing
  respcalc_add_value(v, pecgm_packet->break_byte, (unsigned char)ecg_data.set_bits.breath_ch);
#endif

  if (v < -32000) v = -32000;
  if (v > +32000) v = +32000;

  dview_chandata_add(RESP, (int)((float)v * 0.05));

  if (pecgm_packet->set_bits.taubr300)
  {
    resp_hw_reset_counter ++;
    if (resp_hw_reset_counter >= 2*FD_ECG) // 1 sec
    {
      resp_hw_reset_counter -= 100;
      ecgm_command(ECGM_RESP_TAU_6800);
#if defined (RESPCALC_COLIBRI)
      respcalc_reset();
#endif
    }
  }
  else
  {
    resp_hw_reset_counter = 0;
  }

//printf("%d\n", *((short*)(pecgm_packet->data)+3));

  if (old_set != (pecgm_packet->set & 0x7f))
  {
    inpfoc_item_t * pifi;

    if (unit_get_data(ECG, &ecg_data) <= 0)
    {
      error("%s: error reading ecg data\n", __FUNCTION__);
    }

    if (unit_get_data(RESP, &resp_data) <= 0)
    {
      error("%s: error reading resp data\n", __FUNCTION__);
    }

    resp_data.lead = pecgm_packet->set_bits.breath_ch;
    resp_data.tau = pecgm_packet->set_bits.taubr300;

    switch (pecgm_packet->set_bits.tau)
    {
      case TAU_3200:
       // diagnostics mode
      // if (cframe_get_ecgfilt_mode() != MODE_DIAGNOSTICS)
       cframe_set_ecgfilt_mode((ecg_data.set_bits.diag_f) ? MODE_DIAGNOSTICS_F : MODE_DIAGNOSTICS);
      // cframe_command(CFRAME_FILT_MODE_SET, (void*)((unsigned int)MODE_DIAGNOSTICS));
      // cur_mode = MODE_DIAGNOSTICS;
       break;
      case TAU_320:
       // monitoring mode
      // if (cframe_get_ecgfilt_mode() != MODE_MONITORING)
       cframe_set_ecgfilt_mode(MODE_MONITORING);
     // cframe_command(CFRAME_FILT_MODE_SET, (void*)((unsigned int)MODE_MONITORING));
      // cur_mode = MODE_MONITORING;
       break;
      case TAU_160:
       // surgery mode
      // if (cframe_get_ecgfilt_mode() != MODE_SURGERY)
       cframe_set_ecgfilt_mode(MODE_SURGERY);
      // cframe_command(CFRAME_FILT_MODE_SET, (void*)((unsigned int)MODE_SURGERY));
      // cur_mode = MODE_SURGERY;
       break;
      default:
       error("%s: unknown ecg mode (tau=%d)\n", __FILE__, pecgm_packet->set_bits.tau);
       break;
    }

    ecg_data.set &= ~0x7f;
    ecg_data.set |= pecgm_packet->set;

    ids2string((ecg_data.set_bits.dhf) ? IDS_ON : IDS_OFF, s);
    pifi = inpfoc_find(INPFOC_ECGSET, ECGSET_DHF);
    if (pifi) inpfoc_wnd_setcaption(pifi, s);

    pifi = inpfoc_find(INPFOC_RESPSET, RESPSET_CHANSEL);
    if (pifi) inpfoc_wnd_setcaption(pifi, (ecg_data.set_bits.breath_ch) ? "RF" : "RL");

    if (unit_set_data(ECG, &ecg_data) <= 0)
    {
      error("%s: error writing ecg data\n", __FUNCTION__);
    }

    if (unit_set_data(RESP, &resp_data) <= 0)
    {
      error("%s: error writing resp data\n", __FUNCTION__);
    }
//printf("ecg set %02X\n", ecg_data.set);
  } // new set

  // analyze Z base
 // if ( data[13] & 0xC0/*((data[13]<< 8) | data[12]) > 16000*/ ) // 16384 or higher
  if ( data[13] & 0x80 ) // 32768 or higher
  {
   // alarm_set((ecg_data.set_bits.breath_ch) ? RESP_BADCONTACT_RL : RESP_BADCONTACT_RF);
    alarm_set((data[14]&0x20) ? RESP_BADCONTACT_RF : RESP_BADCONTACT_RL);
    unsigned char b = 0xFE;
    resp_process_packet(&b, 1);
  }
  else
  {
    alarm_clr(RESP_BADCONTACT_RL);
    alarm_clr(RESP_BADCONTACT_RF);
  }

#if defined (ECGM_CR_ZBASE_MONITORING)
  static int cnt_z = 0;
  static long sum_zpulse = 0;
  static unsigned long sum_zbase = 0;
  sum_zbase += (unsigned char)data[13] * 256 + (unsigned char)data[12];
  sum_zpulse += *((short*)(pecgm_packet->data)+3);
  cnt_z ++;
  if (cnt_z == 500)
  {
    sum_zbase /= cnt_z;
    iframe_command(IFRAME_SET_CARDNO, (void*)((int)(sum_zbase)));
    sum_zpulse /= (cnt_z*10);
    if (sum_zpulse < 0) sum_zpulse = -sum_zpulse;
    iframe_command(IFRAME_SET_BEDNO, (void*)((int)(sum_zpulse)));
    cnt_z = 0;
    sum_zbase = 0;
  }
#endif

 // printf("Z pulse = %d\n", *((short*)(pecgm_packet->data)+3));
 // printf("Z base = %d\n", ((data[13]<< 8) | data[12]));

  old_set = pecgm_packet->set & 0x7f;

  csio_ecgdata_c_t csio_ecgdata_c;
  memset(&csio_ecgdata_c, 0, sizeof(csio_ecgdata_c_t));
  csio_ecgdata_c.sync = pecgm_packet->sync;
  csio_ecgdata_c.mode = ecg_data.set;
  csio_ecgdata_c.break_byte = pecgm_packet->break_byte;
  memcpy(&csio_ecgdata_c.data[0], &pecgm_packet->data[0], 3*sizeof(short));
  csio_ecgdata_c.pm = pecgm_packet->pacemaker;
  csio_send_msg(CSIO_ID_ECG_C, &csio_ecgdata_c, sizeof(csio_ecgdata_c_t));

  csio_respdata_c_t csio_respdata_c;
  memset(&csio_respdata_c, 0, sizeof(csio_respdata_c_t));
  csio_respdata_c.sync = pecgm_packet->sync;
  csio_respdata_c.lead = pecgm_packet->set;
  csio_respdata_c.rpg = *((short*)(pecgm_packet->data)+3);
  csio_send_msg(CSIO_ID_RESP_C, &csio_respdata_c, sizeof(csio_respdata_c_t));
}

void ecgm_command_queue_init(void)
{
  memset(&ecgm_cmd_ex, 0, sizeof(ecgm_cmd_ex));
}

void ecgm_command(unsigned char cmd)
{
  int i, l;

  if (cmd == ECGM_DEMO_OFF)
  {
    ecgm_demo_mode = 0;
    return;
  }
  else
  if (cmd == ECGM_DEMO_ON)
  {
    ecgm_demo_mode = 1;
    return;
  }

  if (ecgm_demo_mode)
  {
    return;
  }

#if defined (RESPCALC_COLIBRI)
  if (cmd == ECGM_RESP_CHAN_RL || cmd == ECGM_RESP_CHAN_RF || cmd == ECGM_RESP_TAU_320 || cmd == ECGM_RESP_TAU_6800 || cmd == ECGM_RESP_ON)
  {
    sched_start(SCHED_ANY, 5*1000, respcalc_reset, SCHED_DO_ONCE);
  }
#endif

  if (ecgm_num_cmds_running >= ECGM_MAX_NUM_CMDS_IN_QUEUE)
  {
    error("ecgm cmds overflow\n");
    for (i=0; i<ECGM_MAX_NUM_CMDS_IN_QUEUE; i++)
    {
      ecgm_cmd_ex[i].running = 0;
    }
    ecgm_num_cmds_running = 0;
    return;
  }
  for (i=0; i<ECGM_MAX_NUM_CMDS_IN_QUEUE; i++)
  {
    if (ecgm_cmd_ex[i].cmd == cmd)
    {
      ecgm_cmd_ex[i].running = 0;
      break;
    }
  }
  if (i == ECGM_MAX_NUM_CMDS_IN_QUEUE)
  {
    for (i=0; i<ECGM_MAX_NUM_CMDS_IN_QUEUE; i++)
    {
      if (ecgm_cmd_ex[i].running == 0) break;
    }
  }
  if (i >= ECGM_MAX_NUM_CMDS_IN_QUEUE)
  {
    return;
  }
  for (l=0; l<sizeof(ecgm_cmd_list)/sizeof(ecgm_cmd_list_t); l++)
  {
    if (ecgm_cmd_list[l].cmd == cmd) break;
  }
  if (l == sizeof(ecgm_cmd_list)/sizeof(ecgm_cmd_list_t))
  {
    return;
  }

  ecgm_cmd_ex[i].cmd  = ecgm_cmd_list[l].cmd;
  ecgm_cmd_ex[i].cbit = ecgm_cmd_list[l].cbit;
  ecgm_cmd_ex[i].sbit = ecgm_cmd_list[l].sbit;
  ecgm_cmd_ex[i].retry_count = 0;
  ecgm_cmd_ex[i].running = 1;
  dio_module_cmd(PD_ID_ECG, ecgm_cmd_ex[i].cmd);
  if (ecgm_num_cmds_running == 0)
  {
    sched_start(SCHED_ECGM, 100, ecgm_analyze_input, SCHED_NORMAL);
  }
  ecgm_num_cmds_running ++;
}

void ecgm_analyze_input(void)
{
  int i, ok;
  ecg_data_t ecg_data;
  unit_get_data(ECG, &ecg_data);

 // debug("ecgm set byte: %02X\n", ecg_data.set);

  for (i=0; i<ECGM_MAX_NUM_CMDS_IN_QUEUE; i++)
  {
    if (ecgm_cmd_ex[i].running)
    {
      ok = 0;
      if (ecgm_cmd_ex[i].sbit >= 0)
      {
        if ((ecg_data.set & (1<<ecgm_cmd_ex[i].sbit)) != 0)
        {
         // debug("ecgm cmd %02X okay.\n", ecgm_cmd_ex[i].cmd);
          ok = 1;
          ecgm_cmd_ex[i].running = 0;
          ecgm_num_cmds_running --;
          if (ecgm_num_cmds_running == 0)
          {
           // debug("all ecgm cmds complete\n");
            sched_stop(SCHED_ECGM);
          }
        }
      }
      if (ecgm_cmd_ex[i].cbit >= 0)
      {
        if ((ecg_data.set & (1<<ecgm_cmd_ex[i].cbit)) == 0)
        {
         // debug("ecgm cmd %02X okay.\n", ecgm_cmd_ex[i].cmd);
          ok = 1;
          ecgm_cmd_ex[i].running = 0;
          ecgm_num_cmds_running --;
          if (ecgm_num_cmds_running == 0)
          {
           // debug("all ecgm cmds complete\n");
            sched_stop(SCHED_ECGM);
          }
        }
      }
      if (!ok)
      {
        ecgm_cmd_ex[i].retry_count ++;
        if (ecgm_cmd_ex[i].retry_count >= 15) // 1.5 sec
        {
         // debug("ecgm cmd set timeout, reset ecgm\n");
         // dio_module_cmd(PD_ID_ECG, SBDP_CMD_RST_MOD);
          debug("ecgm cmd set timeout, reset sbdp\n");
          sbdp_reset();
          ecgm_cmd_ex[i].retry_count = 0;
        }
        else
          dio_module_cmd(PD_ID_ECG, ecgm_cmd_ex[i].cmd);
      }
    }
  }
}

void ecg_on_no_qrs(void)
{
  ecg_data_t ecg_data;
  int i;

  unit_get_data(ECG, &ecg_data);

  if ((ecg_data.break_byte & 0xF) == 0 && !alarm_isset(ECG_NOCONNECT)) // no breaks
  {
    alarm_set(ECG_ASYSTOLIA);
   // sched_stop(SCHED_QRS_WAIT);
  }
  else
  {
    alarm_clr(ECG_ASYSTOLIA);
   // reloads automatically
   // sched_start(SCHED_QRS_WAIT, ecg_data.asys_sec*1000, ecg_on_no_qrs, SCHED_NORMAL);
  }
  ecg_data.hr = UNDEF_VALUE;
  for (i=0; i<NUM_ECG; i++)
    ecg_data.st[i] = UNDEF_VALUE;
  if (ecg_data.hr_src == HRSRC_ECG || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == ECG))
  {
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, UNDEF_VALUE);
  }
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_VALUE, UNDEF_VALUE);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_VALUE, UNDEF_VALUE);

  unit_set_data(ECG, &ecg_data);
  unit_cfg_save();
}

int chno_is_ecg(int chno)
{
  return (chno >= ECG_1 && chno <= ECG_LAST);
}
