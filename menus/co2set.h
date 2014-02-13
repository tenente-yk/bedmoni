#ifndef __CO2SET_H
#define __CO2SET_H

#include "inpfoc.h"

enum
{
  CO2SET_NONE = 0,   // 0 - have to be not used
  CO2SET_ETCO2H,
  CO2SET_ETCO2L,
  CO2SET_ETCO2ALR,
  CO2SET_ICO2,
  CO2SET_ICO2ALR,
  CO2SET_BRH,
  CO2SET_BRL,
  CO2SET_BRALR,
  CO2SET_APNOE,
  CO2SET_APNOEALR,
  CO2SET_SPEED,
  CO2SET_DRAWCO2,
  CO2SET_INISET,
  CO2SET_EXIT,       //
  CO2SET_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_co2set_funclist[CO2SET_NUM_ITEMS+1];
extern char menu_co2set[];

void menu_co2set_openproc(void);

#endif // __CO2SET_H
