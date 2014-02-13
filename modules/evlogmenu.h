#ifndef __EVLOGMENU_H
#define __EVLOGMENU_H

#include "inpfoc.h"

enum
{
  EVLOG_NONE = 0,   // 0 - have to be not used
  EVLOG_SEEK0,      //
  EVLOG_VSCROLL,    //
  EVLOG_GOTO_TABLE, //
  EVLOG_SET,        //
  EVLOG_EXIT,       //
  EVLOG_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_evlog_funclist[EVLOG_NUM_ITEMS+1];

#endif // __EVLOGMENU_H
