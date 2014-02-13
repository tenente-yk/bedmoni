#ifndef __NIBPCAL_H
#define __NIBPCAL_H

#include "inpfoc.h"

enum
{
  NIBPCAL_NONE = 0,   // 0 - have to be not used
  NIBPCAL_SET0,       // 1
  NIBPCAL_SET250,     // 2
  NIBPCAL_EXIT,       // 3
  NIBPCAL_NUM_ITEMS,  // 4
};

extern inpfoc_funclist_t inpfoc_nibpcal_funclist[NIBPCAL_NUM_ITEMS+1];
extern char menu_nibpcal[];

void menu_nibpcal_openproc(void);
void menu_nibpcal_closeproc(void);


#endif // __NIBPCAL_H
