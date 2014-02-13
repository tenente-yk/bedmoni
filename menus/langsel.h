#ifndef __LANGSEL_H
#define __LANGSEL_H

#include "inpfoc.h"

enum
{
  LANG_NONE = 0,   // 0 - have to be not used
  LANG_RUS,        // 1
  LANG_ENG,        // 2
  LANG_GER,        // 3
  LANG_FRA,        // 4
  LANG_ITA,        // 5
  LANG_ESP,        // 6
  LANG_UKR,        // 7
  LANG_EXIT,       // 8
  LANG_NUM_ITEMS,  // 9
};

extern inpfoc_funclist_t inpfoc_langsel_funclist[LANG_NUM_ITEMS+1];
extern char menu_langsel[];

void menu_langsel_openproc(void);

#endif // __LANGSEL_H
