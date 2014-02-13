#ifndef __LEAKTEST_H
#define __LEAKTEST_H

#include "inpfoc.h"

enum
{
  LEAKTEST_NONE = 0,   // 0 - have to be not used
  LEAKTEST_START,      // 1
  LEAKTEST_EXIT,       // 2
  LEAKTEST_NUM_ITEMS,  // 3
};

extern inpfoc_funclist_t inpfoc_leaktest_funclist[LEAKTEST_NUM_ITEMS+1];
extern char menu_leaktst[];

void menu_leaktst_openproc(void);
void menu_leaktst_closeproc(void);

#endif // __LEAKTEST_H
