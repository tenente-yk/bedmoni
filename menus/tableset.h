#ifndef __TABLESET_H
#define __TABLESET_H

#include "inpfoc.h"

enum
{
  TABLESET_NONE = 0,   // 0 - have to be not used
  TABLESET_PAR1,       // 1
  TABLESET_PAR_FIRST = TABLESET_PAR1,
  TABLESET_PAR2,       // 2
  TABLESET_PAR3,       // 3
  TABLESET_PAR4,       // 4
  TABLESET_PAR5,       // 5
  TABLESET_PAR6,       // 6
  TABLESET_PAR7,       // 7
  TABLESET_PAR8,       // 8
  TABLESET_PAR9,       // 9
  TABLESET_PAR10,      // 10
  TABLESET_PAR11,      // 11
  TABLESET_PAR12,      // 12
  TABLESET_PAR13,      // 13
  TABLESET_PAR14,      // 14
  TABLESET_PAR15,      // 15
  TABLESET_PAR_LAST = TABLESET_PAR15,
  TABLESET_DEFAULT,    // 16
  TABLESET_MODE,       // 17
  TABLESET_EXIT,       // 18
  TABLESET_NUM_ITEMS,  // 19
};

extern inpfoc_funclist_t inpfoc_tblset_funclist[TABLESET_NUM_ITEMS+1];
extern char menu_tableset[];

void menu_tableset_openproc(void);

#endif // __TABLESET_H
