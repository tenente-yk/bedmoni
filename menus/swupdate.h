#ifndef __SWUPDATE_H
#define __SWUPDATE_H

#include "inpfoc.h"

enum
{
  SWUPD_NONE = 0,   // 0 - have to be not used
  SWUPD_PROGRESS,   // 1
  SWUPD_IPS1,       // 2
  SWUPD_IPS2,       // 3
  SWUPD_IPS3,       // 4
  SWUPD_IPS4,       // 5
  SWUPD_IPS_OK,     // 6
  SWUPD_EXIT,       // 10
  SWUPD_NUM_ITEMS,  // 11
};

extern inpfoc_funclist_t inpfoc_swupdate_funclist[SWUPD_NUM_ITEMS+1];
extern char menu_swupdate[];

void menu_swupdate_openproc(void);

#endif // __SWUPDATE_H
