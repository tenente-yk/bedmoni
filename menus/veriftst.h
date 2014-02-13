#ifndef __NIBPVERIF_H
#define __NIBPVERIF_H

#include "inpfoc.h"

enum
{
  VERIF_NONE = 0,   // 0 - have to be not used
  VERIF_START,      // 1
  VERIF_EXIT,       // 2
  VERIF_NUM_ITEMS,  // 3
};

extern inpfoc_funclist_t inpfoc_veriftest_funclist[VERIF_NUM_ITEMS+1];
extern char menu_veriftst[];

void menu_veriftst_openproc(void);
void menu_veriftst_closeproc(void);


#endif // __NIBPVERIF_H
