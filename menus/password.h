#ifndef __PASSWORD_H
#define __PASSWORD_H

#include "inpfoc.h"

enum
{
  PASSWORD_NONE = 0,   // 0 - have to be not used
  PASSWORD_U1,         // 1
  PASSWORD_U2,         // 2
  PASSWORD_U3,         // 3
  PASSWORD_U4,         // 4
  PASSWORD_OK,         // 5
  PASSWORD_EXIT,       // 6
  PASSWORD_NUM_ITEMS,  // 8
};

extern inpfoc_funclist_t inpfoc_password_funclist[PASSWORD_NUM_ITEMS+1];
extern char menu_password[];

void menu_password_openproc(void);

void menu_password_onokmenu(int id);

unsigned int menu_password_getpass(void);

void menu_password_setpass(unsigned int pw);

#endif // __PASSWORD_H
