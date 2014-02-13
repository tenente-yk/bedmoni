#ifndef __PATIENT_H
#define __PATIENT_H

#include "inpfoc.h"

enum
{
  PAT_NONE = 0,   // 0 - have to be not used
  PAT_NEW,        // 1
  PAT_CAT,
  PAT_BED,
  PAT_N,
  PAT_H,
  PAT_W,
  PAT_EXIT,       // 7
  PAT_NUM_ITEMS,  // 8
};

extern inpfoc_funclist_t inpfoc_pat_funclist[PAT_NUM_ITEMS+1];
extern char menu_patient[];

void menu_patient_openproc(void);

#endif // __PATIENT_H
