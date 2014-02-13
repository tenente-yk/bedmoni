#ifndef __SPO2SET_H
#define __SPO2SET_H

#include "inpfoc.h"

enum
{
  SPO2SET_NONE = 0,   // 0 - have to be not used
  SPO2SET_SPO2ALR,    // 1
  SPO2SET_TRSHSPO2H,  // 2
  SPO2SET_TRSHSPO2L,  // 3
  SPO2SET_HRALR,      // 4
  SPO2SET_TRSHHRH,    // 5
  SPO2SET_TRSHHRL,    // 6
  SPO2SET_SPEED,      // 7
  SPO2SET_DRAWFPG,    // 8
  SPO2SET_EXIT,       // 9
  SPO2SET_NUM_ITEMS,  // 10
};

extern inpfoc_funclist_t inpfoc_spo2set_funclist[SPO2SET_NUM_ITEMS+1];
extern char menu_spo2set[];

void menu_spo2set_openproc(void);

#endif // __SPO2SET_H
