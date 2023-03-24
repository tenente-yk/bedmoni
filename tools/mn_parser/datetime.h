#ifndef __DATETIME_H
#define __DATETIME_H

#include "inpfoc.h"

#define ANYDEF 78

enum
{
  DT_NONE = 0,   // 0 - have to be not used
  DT_DAY,        // 1
  DT_MON,        // 2
  DT_YEAR,       // 3
  DT_EXIT,       // 4
  DT_NUM_ITEMS,  // 
};

extern inpfoc_funclist_t inpfoc_dt_funclist[DT_NUM_ITEMS+1];

#endif // __DATETIME_H
