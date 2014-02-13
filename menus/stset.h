#ifndef __STSET_H
#define __STSET_H

#include "inpfoc.h"

enum
{
  STSET_NONE = 0,   // 0 - have to be not used
  STALR_STI_D,      // 1
  STALR_STI_R,      // 2
  STALR_STI_A,      // 3
  STALR_STII_D,     // 4
  STALR_STII_R,     // 5
  STALR_STII_A,     // 6
  STALR_STIII_D,    // 7
  STALR_STIII_R,    // 8
  STALR_STIII_A,    // 9
  STALR_STAVR_D,    // 10
  STALR_STAVR_R,    // 11
  STALR_STAVR_A,    // 12
  STALR_STAVL_D,    // 13
  STALR_STAVL_R,    // 14
  STALR_STAVL_A,    // 15
  STALR_STAVF_D,    // 16
  STALR_STAVF_R,    // 17
  STALR_STAVF_A,    // 18
  STALR_STV_D,      // 19
  STALR_STV_R,      // 20
  STALR_STV_A,      // 21
  STSET_ALRALL,     // 22
  STSET_EXIT,       // 23
  STSET_ST1,        // 24
  STSET_ST2,        // 25
  STSET_JSHIFT,     // 26
  STSET_NUM_ITEMS,  // 27
};

extern inpfoc_funclist_t inpfoc_stset_funclist[STSET_NUM_ITEMS+1];
extern char menu_stset[];

void menu_stset_openproc(void);

#endif // __STSET_H
