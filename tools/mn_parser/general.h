#ifndef __GENERAL_H
#define __GENERAL_H

#include "inpfoc.h"

enum
{
  GENERAL_NONE = 0,   // 0 - have to be not used
  GENERAL_PAT,        // 1
  GENERAL_ALR,        // 2
  GENERAL_CALC,       // 3
  GENERAL_SCR,        // 4
  GENERAL_EVNT,       // 5
  GENERAL_TRN,        // 6
  GENERAL_TBL,        // 7
  GENERAL_PRN,        // 8
  GENERAL_SET,        // 9
  GENERAL_EXIT,       // 10
  GENERAL_NUM_ITEMS,  // 11
};

extern inpfoc_funclist_t inpfoc_general_funclist[GENERAL_NUM_ITEMS+1];
extern char menu_general[];

void menu_general_proc(void);

#endif // __GENERAL_H
