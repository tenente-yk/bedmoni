#ifndef __EVLOGSET_H
#define __EVLOGSET_H

#include "inpfoc.h"

enum
{
  EVLOGSET_NONE = 0,   // 0 - have to be not used
  EVLOGSET_FONT,       // 1
  EVLOGSET_CLRALL,     // 2
  EVLOGSET_EXIT,       // 3
  EVLOGSET_NUM_ITEMS,  // 4
};

extern inpfoc_funclist_t inpfoc_evlogset_funclist[EVLOGSET_NUM_ITEMS+1];
extern char menu_evlogset[];

void menu_evlogset_openproc(void);

#endif // __EVLOGSET_H
