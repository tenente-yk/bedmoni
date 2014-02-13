#ifndef __RESPSET_H
#define __RESPSET_H

#include "inpfoc.h"

enum
{
  RESPSET_NONE = 0,   // 0 - have to be not used
  RESPSET_BRALR,      // 1
  RESPSET_BRH,        // 2
  RESPSET_BRL,        // 3
  RESPSET_AP,         // 4
  RESPSET_APALR,      // 5
  RESPSET_DRAWRPG,    // 6
  RESPSET_EXIT,       // 7
  RESPSET_SPEED,      // 8
  RESPSET_CHANSEL,    // 9
  RESPSET_NUM_ITEMS,  // 10
};

extern inpfoc_funclist_t inpfoc_respset_funclist[RESPSET_NUM_ITEMS+1];
extern char menu_respset[];

void menu_respset_openproc(void);

#endif // __RESPSET_H
