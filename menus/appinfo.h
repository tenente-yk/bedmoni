#ifndef __APPINFO_H
#define __APPINFO_H

#include "inpfoc.h"

enum
{
  APPINFO_NONE = 0,   // 0 - have to be not used
  APPINFO_UPD,        // 1
  APPINFO_EXIT,       // 2
  APPINFO_NUM_ITEMS,  // 3
};

extern inpfoc_funclist_t inpfoc_appinfo_funclist[APPINFO_NUM_ITEMS+1];
extern char menu_appinfo[];

void menu_appinfo_openproc(void);

#endif // __APPINFO_H
