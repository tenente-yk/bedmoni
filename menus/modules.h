#ifndef __MODULES_H
#define __MODULES_H

#include "inpfoc.h"

enum
{
  MOD_NONE = 0,   // 0 - have to be not used
  MOD_ECG,        // 1
  MOD_NAD,        // 2
  MOD_ID1,        // 3
  MOD_CO2,        // 4
  MOD_ID2,        // 5
  MOD_RESP,       // 6
  MOD_SPO2,       // 7
  MOD_T,          // 8
  MOD_SAVE,       // 9
  MOD_EXIT,       // 10
  MOD_NUM_ITEMS,  // 11
};

extern inpfoc_funclist_t inpfoc_mod_funclist[MOD_NUM_ITEMS+1];
extern char menu_modules[];

void menu_modules_openproc(void);

#endif // __MODULES_H
