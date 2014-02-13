#ifndef __DIALOG_H
#define __DIALOG_H

#include "inpfoc.h"

enum
{
  DIALOG_NONE = 0,   // 0 - have to be not used
  DIALOG_YES,        //
  DIALOG_NO,         //
  DIALOG_OK,         //
  DIALOG_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_dialog_funclist[DIALOG_NUM_ITEMS+1];

#endif // __DIALOG_H
