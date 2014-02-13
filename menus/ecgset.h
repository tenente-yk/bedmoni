#ifndef __ECGSET_H
#define __ECGSET_H

#include "inpfoc.h"

enum
{
  ECGSET_NONE = 0,   // 0 - have to be not used
  ECGSET_EXIT,       // 1
  ECGSET_HRL,
  ECGSET_HRH,
  ECGSET_HRALR,
  ECGSET_PRNALR,
  ECGSET_QRSSIGVOL,
  ECGSET_HRCALCSRC,
  ECGSET_NUMCHANS,
  ECGSET_SPEED,
  ECGSET_STCALCPROC,
  ECGSET_STSETPROC,
  ECGSET_ARRHPROC,
  ECGSET_HRSRC,
  ECGSET_DHF,
  ECGSET_CABLETYPE,
  ECGSET_NUM_ITEMS,  //
};

enum
{
  HRSRC_AUTO = 0,
  HRSRC_ECG,
  HRSRC_SPO2,
  HRSRC_NIBP,
  HRSRC_NUM,
};

extern inpfoc_funclist_t inpfoc_ecgset_funclist[ECGSET_NUM_ITEMS+1];
extern char menu_ecgset[];

void menu_ecgset_openproc(void);

void menu_ecgset_closeproc(void);

#endif // __ECGSET_H
