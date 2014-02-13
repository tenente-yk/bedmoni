#ifndef __MAINMENU_H
#define __MAINMENU_H

#include "inpfoc.h"

enum
{
  MAIN_NONE = 0,   // 0 - have to be not used
  MAIN_ECG_C,      // 1
  MAIN_ECG_1,      // 2
  MAIN_ECG_2,      // 3
  MAIN_ECG_3,      // 4
  MAIN_ECG_4,      // 5
  MAIN_ECG_5,      // 6
  MAIN_ECG_6,      // 7
  MAIN_ECG_7,      // 8
  MAIN_ECG_SPD,    // 9
  MAIN_ECG_AMP,    // 10
  MAIN_ECG_NEO,    // 11
  MAIN_SPO2_C,     // 12
  MAIN_SPO2_AMP,   // 13
  MAIN_SPO2_SPD,   // 14
  MAIN_RPG_C,      // 15
  MAIN_RPG_AMP,    // 16
  MAIN_RPG_SPD,    // 17
  MAIN_RESP_C,     // 18
  MAIN_RESP_AMP,   // 19
  MAIN_RESP_SPD,   // 20
  MAIN_MODE,       // 21
  MAIN_ECG,        // 22
  MAIN_SPO2,       // 23
  MAIN_RESP,       // 24
  MAIN_T1T2,       // 25
  MAIN_NIBP,       // 26
  MAIN_CO2,        // 27
  MAIN_CO2_C,      // 28
  MAIN_CO2_AMP,    // 29
  MAIN_CO2_SPD,    // 30
  MAIN_NUM_ITEMS,  // 31
};

extern inpfoc_funclist_t inpfoc_main_funclist[MAIN_NUM_ITEMS+1];

#endif // __MAINMENU_H
