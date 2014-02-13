#ifndef __DEFAULTSET_H
#define __DEFAULTSET_H

#include "inpfoc.h"

enum
{
  DEFSET_NONE = 0,   // 0 - have to be not used
  DEFSET_YES,        // 1
  DEFSET_NO,         // 2
  DEFSET_NUM_ITEMS,  // 3
};

extern inpfoc_funclist_t inpfoc_defset_funclist[DEFSET_NUM_ITEMS+1];
extern char menu_defaultset[];

void menu_defaultset_openproc(void);

//void defaultset_units(void);

#endif // __DEFAULTSET_H
