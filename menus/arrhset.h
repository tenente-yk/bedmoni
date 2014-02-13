#ifndef __ARRHSET_H
#define __ARRHSET_H

#include "inpfoc.h"

enum
{
  ARRHSET_NONE = 0,   // 0 - have to be not used
  ARRHSET_ASYS,       // 1
  ARRHSET_ASYSALR,    // 2
  ARRHSET_NORESPDLY,  // 3
  ARRHSET_NORESPALR,  // 4
  ARRHSET_TACH,       // 5
  ARRHSET_TACHALR,    // 6
  ARRHSET_EXIT,       // 7
  ARRHSET_NUM_ITEMS,  // 8
};

extern inpfoc_funclist_t inpfoc_arrhset_funclist[ARRHSET_NUM_ITEMS+1];
extern char menu_arrhset[];

void menu_arrhset_openproc(void);

#endif // __ARRHSET_H
