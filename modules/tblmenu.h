#ifndef __TBLMENU_H
#define __TBLMENU_H

#include "inpfoc.h"

enum
{
  TABLE_NONE = 0,   // 0 - have to be not used
  TABLE_TS,         //
  TABLE_MODE,       //
  TABLE_VSCROLL,    //
  TABLE_HSCROLL,    //
  TABLE_PRINT,      //
  TABLE_SET,        //
  TABLE_EXIT,       //
  TABLE_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_tbl_funclist[TABLE_NUM_ITEMS+1];

#endif // __TBLMENU_H
