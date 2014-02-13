#ifndef __SETTINGS_H
#define __SETTINGS_H

#include "inpfoc.h"

enum
{
  SET_NONE = 0,   // 0 - have to be not used
  SET_DT,         // 1
  SET_LANG,       // 2
  SET_MOD,        // 3
  SET_DEF,        // 4
  SET_SUPP,       // 5
  SET_DEMO,       // 6
  SET_INFO,       // 7
  SET_NET,        // 8
  SET_EXIT,       // 9
  SET_NUM_ITEMS,  // 10
};

extern inpfoc_funclist_t inpfoc_set_funclist[SET_NUM_ITEMS+1];
extern char menu_settings[];

void menu_settings_openproc(void);

#endif // __SETTINGS_H
