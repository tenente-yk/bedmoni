#ifndef __DEMOSET_H
#define __DEMOSET_H

#include "inpfoc.h"

enum
{
  DEMOSET_NONE = 0,   // 0 - have to be not used
  DEMOSET_EXIT,       //
  DEMOSET_RB_DEMO,    //
  DEMOSET_RB_DEVICE,  //
  DEMOSET_DOREC,      //
  DEMOSET_FILE1_SEL,  //
  DEMOSET_FILE2_SEL,  //
  DEMOSET_FILE3_SEL,  //
  DEMOSET_FILE4_SEL,  //
  DEMOSET_FILE_SAVE,  //
  DEMOSET_FILES_DEL,  //
  DEMOSET_VIDEO,      //
  DEMOSET_NUM_ITEMS,  //
};

extern inpfoc_funclist_t inpfoc_demoset_funclist[DEMOSET_NUM_ITEMS+1];
extern char menu_demoset[];

void menu_demoset_openproc(void);
void menu_demoset_closeproc(void);

#endif // __ECGSET_H
