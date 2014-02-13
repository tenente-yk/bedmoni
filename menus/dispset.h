#ifndef __DISPSET_H
#define __DISPSET_H

#include "inpfoc.h"

enum
{
  DISP_NONE = 0,   // 0 - have to be not used
  DISP_DEFAULT,    // 1
  DISP_ENLARGE,    // 2
  DISP_BRGH,       // 3
  DISP_EXIT,       // 4
  DISP_NUM_ITEMS,  // 5
};

extern inpfoc_funclist_t inpfoc_dispset_funclist[DISP_NUM_ITEMS+1];
extern char menu_dispset[];

void menu_dispset_openproc(void);

#endif // __DISPSET_H
