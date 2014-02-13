#ifndef __T1T2SET_H
#define __T1T2SET_H

#include "inpfoc.h"

enum
{
  T1T2SET_NONE = 0,   // 0 - have to be not used
  T1T2SET_T1ALR,      // 1
  T1T2SET_T1H,        // 2
  T1T2SET_T1L,        // 3
  T1T2SET_T2ALR,      // 4
  T1T2SET_T2H,        // 5
  T1T2SET_T2L,        // 6
  T1T2SET_DTALR,      // 7
  T1T2SET_DT,         // 8
  T1T2SET_EXIT,       // 9
  T1T2SET_NUM_ITEMS,  // 10
};

extern inpfoc_funclist_t inpfoc_t1t2set_funclist[T1T2SET_NUM_ITEMS+1];
extern char menu_t1t2set[];

void menu_t1t2set_openproc(void);

#endif // __T1T2SET_H
