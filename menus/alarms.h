#ifndef __ALARMS_H
#define __ALARMS_H

#include "inpfoc.h"

enum
{
  ALARM_NONE = 0,   // 0 - have to be not used
  ALARM_PHYS,
  ALARM_TECH,
  ALARM_THSH,
  ALARM_VOL,
  ALARM_DLY,
  ALARM_PRALL,      //
  ALARM_HA_EXT,
  ALARM_MA_EXT,
  ALARM_TA_EXT,
  ALARM_CLEAR_ALL,
  ALARM_EXIT,       //
  ALARM_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_alr_funclist[ALARM_NUM_ITEMS+1];
extern char menu_alarms[];

void menu_alarms_openproc(void);

#endif // __ALARMS_H
