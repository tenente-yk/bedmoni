#ifndef __SHUTDOWN_H
#define __SHUTDOWN_H

#include "inpfoc.h"

enum
{
  SHUTDOWN_NONE = 0,   // 0 - have to be not used
  SHUTDOWN_YES,        // 1
  SHUTDOWN_NO,         // 2
  SHUTDOWN_NUM_ITEMS,  // 3
};

extern inpfoc_funclist_t inpfoc_shutdown_funclist[SHUTDOWN_NUM_ITEMS+1];
extern char menu_shutdown[];

void menu_shutdown_openproc(void);

#endif // __DEFAULTSET_H
