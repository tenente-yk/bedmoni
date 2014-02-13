#ifndef __TRNMENU_H
#define __TRNMENU_H

#include "inpfoc.h"

enum
{
  TRENDS_NONE = 0,   // 0 - have to be not used
  TRENDS_LSEEK,      //
  TRENDS_SEEK,       //
  TRENDS_END,        //
  TRENDS_STEP,       //
  TRENDS_SET,        //
  TRENDS_EXIT,       //
  TRENDS_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_trn_funclist[TRENDS_NUM_ITEMS+1];

#endif // __TRNMENU_H
