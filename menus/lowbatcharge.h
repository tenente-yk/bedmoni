#ifndef __LOWBATCHARGE_H
#define __LOWBATCHARGE_H

#include "inpfoc.h"

enum
{
  LOWBAT_NONE = 0,   // 0 - have to be not used
  LOWBAT_OK,         // 1
  LOWBAT_NUM_ITEMS,  // 2
};

extern inpfoc_funclist_t inpfoc_lowbat_funclist[LOWBAT_NUM_ITEMS+1];
extern char menu_lowbat[];

void menu_lowbat_openproc(void);

int lowbat_confirmed(void);

#endif // __ACCDUR5_H
