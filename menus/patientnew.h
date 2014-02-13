#ifndef __PATIENTNEW_H
#define __PATIENTNEW_H

#include "inpfoc.h"

enum
{
  PATNEW_NONE = 0,   // 0 - have to be not used
  PATNEW_YES,
  PATNEW_NO,
  PATNEW_NUM_ITEMS,  // 
};

extern inpfoc_funclist_t inpfoc_patnew_funclist[PATNEW_NUM_ITEMS+1];
extern char menu_patientnew[];

void menu_patientnew_openproc(void);

#endif // __PATIENTNEW_H
