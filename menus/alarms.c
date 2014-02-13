#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include "bedmoni.h"
#include "uframe.h"
#include "mframe.h"
#include "iframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "mainmenu.h"
#include "alarm.h"
#include "alarms.h"

static void alr_none_func(void*);
static void alr_phys_func(void*);
static void alr_tech_func(void*);
static void alr_thsh_func(void*);
static void alr_vol_func(void*);
static void alr_dly_func(void*);
static void alr_prall_func(void*);
static void alr_ha_ext_func(void*);
static void alr_ma_ext_func(void*);
static void alr_ta_ext_func(void*);
static void alr_clear_all_func(void*);
static void alr_exit_func(void*);

extern alarm_msg_t alarm_msg[NUM_ALARMS];

inpfoc_funclist_t inpfoc_alr_funclist[ALARM_NUM_ITEMS+1] = 
{
  { ALARM_NONE,       alr_none_func         },
  { ALARM_PHYS,       alr_phys_func         },
  { ALARM_TECH,       alr_tech_func         },
  { ALARM_THSH,       alr_thsh_func         },
  { ALARM_VOL,        alr_vol_func          },
  { ALARM_DLY,        alr_dly_func          },
  { ALARM_PRALL,      alr_prall_func        },
  { ALARM_HA_EXT,     alr_ha_ext_func       },
  { ALARM_MA_EXT,     alr_ma_ext_func       },
  { ALARM_TA_EXT,     alr_ta_ext_func       },
  { ALARM_CLEAR_ALL,  alr_clear_all_func    },
  { ALARM_EXIT,       alr_exit_func         },
  { -1        ,       alr_none_func         }, // -1 must be last
};

static void alr_none_func(void * parg)
{

}

static void alr_phys_func(void * parg)
{
  alarm_on_off_all_phys(inpfoc_wnd_getchecked(inpfoc_find(INPFOC_ALARM, ALARM_PHYS)));
  mframe_command(MFRAME_UPDATE, NULL);
}

static void alr_tech_func(void * parg)
{
  alarm_on_off_all_tech(inpfoc_wnd_getchecked(inpfoc_find(INPFOC_ALARM, ALARM_TECH)));
  mframe_command(MFRAME_UPDATE, NULL);
}

static void alr_thsh_func(void * parg)
{
  alarm_cfg_t lac;
  char s[200];
  int v;

  alarm_cfg_get(&lac);
  lac.thsh_visible = !lac.thsh_visible;

  v = lac.thsh_visible;
  mframe_command(MFRAME_UNIT_PAR_RANGES, (void*)v);

  ids2string((lac.thsh_visible) ? IDS_ON : IDS_OFF, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ALARM, ALARM_THSH), s);

  alarm_cfg_set(&lac);
}

static void alr_vol_func(void * parg)
{
  alarm_cfg_t ac;
  alarm_info_t ai;
  char s[40];
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;

  alarm_cfg_get(&ac);
  ac.volume += 1*pcmd->delta;
  while ((signed char)ac.volume < 0)                ac.volume += SOUND_MAX_VOLUME;
  while ((signed char)ac.volume > SOUND_MAX_VOLUME) ac.volume -= SOUND_MAX_VOLUME;

  // invalidate sound alarm
  alarm_info_get(&ai);
  ai.current_sound_alarm = AlarmSound_None;
  alarm_info_set(&ai);

  iframe_command(IFRAME_RELOAD, NULL);

  snprintf(s, sizeof(s), "%d", ac.volume);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ALARM, ALARM_VOL), s);

  alarm_cfg_set(&ac);
}

static void alr_dly_func(void * parg)
{
  alarm_cfg_t lac;
  char s[40];
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;

  alarm_cfg_get(&lac);
  lac.delay += 1*pcmd->delta;
  while (lac.delay < 30)  lac.delay += (180 - 30 + 1);
  while (lac.delay > 180) lac.delay -= (180 - 30 + 1);

  snprintf(s, sizeof(s), "%d", lac.delay);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ALARM, ALARM_DLY), s);

  alarm_cfg_set(&lac);
}

static void alr_prall_func(void * parg)
{
  if (inpfoc_wnd_getchecked(inpfoc_find(INPFOC_ALARM, ALARM_PRALL)))
  {
    inpfoc_wnd_check(inpfoc_find(INPFOC_ALARM, ALARM_PHYS), IWC_CHECKED);
    inpfoc_wnd_check(inpfoc_find(INPFOC_ALARM, ALARM_TECH), IWC_CHECKED);
  }
}

static void alr_ha_ext_func(void * parg)
{
  alarm_cfg_t lac;
  alarm_cfg_get(&lac);

  lac.ext_bits.high = !lac.ext_bits.high;

  alarm_cfg_set(&lac);
}

static void alr_ma_ext_func(void * parg)
{
  alarm_cfg_t lac;
  alarm_cfg_get(&lac);

  lac.ext_bits.medium = !lac.ext_bits.medium;

  alarm_cfg_set(&lac);
}

static void alr_ta_ext_func(void * parg)
{
  alarm_cfg_t lac;
  alarm_cfg_get(&lac);

  lac.ext_bits.tech = !lac.ext_bits.tech;

  alarm_cfg_set(&lac);
}

static void alr_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_GENERAL);
  alarm_cfg_save();
}

static void alr_clear_all_func(void * parg)
{
 // alarm_rst();
  int i;
  for (i=0; i<NUM_ALARMS; i++)
  {
    if (alarm_msg[i].type == ALR_PHYS)
    {
      if (alarm_msg[i].state != MSG_SET)
        alarm_clr_fixed(i);
    }
    else // TECH and -1
    {
      alarm_clr(i);
    }
  }
  alarm_cfg_save();
}

void menu_alarms_openproc(void)
{
  int i;
  int set;
  alarm_cfg_t lac;
  char s[200];

  set = 0;
  for (i=0; i<NUM_ALARMS; i++)
  {
 // printf("%d %d %d\n", i, alarm_msg[i].type, alarm_isenabled(i));
    if (alarm_msg[i].type == ALR_TECH && alarm_isenabled(i))
    {
      set = 1;
      break;
    }
  }
 // printf("set=%d\n", set);
  inpfoc_wnd_check(inpfoc_find(INPFOC_ALARM, ALARM_TECH), (set) ? IWC_CHECKED : IWC_UNCHECKED);

  set = 0;
  for (i=0; i<NUM_ALARMS; i++)
  {
//    printf("%d %d %d\n", i, alarm_msg[i].type, alarm_isenabled(i));
    if (alarm_msg[i].type == ALR_PHYS && alarm_isenabled(i))
    {
      set = 1;
      break;
    }
  }
//    printf("set=%d\n", set);
  inpfoc_wnd_check(inpfoc_find(INPFOC_ALARM, ALARM_PHYS), (set) ? IWC_CHECKED : IWC_UNCHECKED);

  alarm_cfg_get(&lac);

  ids2string((lac.thsh_visible) ? IDS_ON : IDS_OFF, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ALARM, ALARM_THSH), s);

  snprintf(s, sizeof(s), "%d", lac.volume);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ALARM, ALARM_VOL), s);

  snprintf(s, sizeof(s), "%d", lac.delay);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_ALARM, ALARM_DLY), s);

  inpfoc_set(INPFOC_ALARM, ALARM_EXIT);
  inpfoc_disable(INPFOC_ALARM, ALARM_PRALL);

  inpfoc_wnd_check(inpfoc_find(INPFOC_ALARM, ALARM_HA_EXT), (lac.ext_bits.high) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_ALARM, ALARM_MA_EXT), (lac.ext_bits.medium) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_ALARM, ALARM_TA_EXT), (lac.ext_bits.tech) ? IWC_CHECKED : IWC_UNCHECKED);
}

