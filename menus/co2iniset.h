#ifndef __CO2INISET_H
#define __CO2INISET_H

#include "inpfoc.h"

enum
{
  CO2INISET_NONE = 0,   // 0 - have to be not used
  CO2INISET_PRESSURE,
  CO2INISET_O2_COMPENS,
  CO2INISET_GAS_BALANCE,
  CO2INISET_ANEST_AGENT,
  CO2INISET_ZEROING,
  CO2INISET_EXIT,       //
  CO2INISET_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_co2iniset_funclist[CO2INISET_NUM_ITEMS+1];
extern char menu_co2iniset[];

void menu_co2iniset_openproc(void);

#endif // __CO2INISET_H
