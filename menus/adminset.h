#ifndef __ADMINSET_H
#define __ADMINSET_H

#include "inpfoc.h"

enum
{
  ADMSET_NONE = 0,   // 0 - have to be not used
  ADMSET_MENU,       //
  ADMSET_APPLY,      //
  ADMSET_EXIT,       //
  ADMSET_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_adminset_funclist[ADMSET_NUM_ITEMS+1];
extern char menu_adminset[];

void menu_adminset_openproc(void);

#endif // __ADMINSET_H
