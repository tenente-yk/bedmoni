#ifndef __PRINTSET_H
#define __PRINTSET_H

#include "inpfoc.h"

enum
{
  PRINTSET_NONE = 0,    // 0 - have to be not used
  PRINTSET_NUMCURVES,   // 1
  PRINTSET_CURVE1,      // 2
  PRINTSET_CURVE1_SCALE,// 3
  PRINTSET_CURVE2,      // 3
  PRINTSET_CURVE2_SCALE,// 4
  PRINTSET_MMPS,        // 5
  PRINTSET_PRN_GRID,    // 6
  PRINTSET_PRN_VAL,     // 7
  PRINTSET_WIDTH,       // 8
  PRINTSET_BRIGHTNESS,  // 9
  PRINTSET_TS,          // 10
  PRINTSET_DELAY1,      // 11
  PRINTSET_DELAY2,      // 12
  PRINTSET_EXIT,        // 13
  PRINTSET_NUM_ITEMS,   // 14
};

extern inpfoc_funclist_t inpfoc_prnset_funclist[PRINTSET_NUM_ITEMS+1];
extern char menu_printset[];

void menu_printset_openproc(void);

#endif // __PRNSET_H
