#ifndef __DATETIME_H
#define __DATETIME_H

#include "inpfoc.h"

enum
{
  DT_NONE = 0,   // 0 - have to be not used
  DT_DAY,        // 1
  DT_MON,        // 2
  DT_YEAR,       // 3
  DT_HOUR,       // 4
  DT_MIN,        // 5
  DT_SEC,        // 6
  DT_SET,        // 7
  DT_EXIT,       // 8
  DT_NUM_ITEMS,  // 9
};

extern inpfoc_funclist_t inpfoc_dt_funclist[DT_NUM_ITEMS+1];
extern char menu_datetime[];

void menu_datetime_openproc(void);

#endif // __DATETIME_H
