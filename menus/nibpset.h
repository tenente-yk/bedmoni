#ifndef __NIBPSET_H
#define __NIBPSET_H

#include "inpfoc.h"

enum
{
  NIBPSET_NONE = 0,  // 0 - have to be not used
  NIBPSET_ALR,       // 1
  NIBPSET_SDH,       // 2
  NIBPSET_SDL,       // 3
  NIBPSET_DDH,       // 4
  NIBPSET_DDL,       // 5
  NIBPSET_MDH,       // 6
  NIBPSET_MDL,       // 7
  NIBPSET_MODE,      // 8
  NIBPSET_INFL,      // 9
  NIBPSET_CALIB,     // 10
  NIBPSET_VERIF,     // 11
  NIBPSET_LEAK,      // 12
  NIBPSET_EXIT,      // 13
  NIBPSET_NUM_ITEMS, // 14
};

extern inpfoc_funclist_t inpfoc_nibpset_funclist[NIBPSET_NUM_ITEMS+1];
extern char menu_nibpset[];

void menu_nibpset_openproc(void);

#endif // __NIBPSET_H
