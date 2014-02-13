#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include "bedmoni.h"
#include "uframe.h"
#include "mframe.h"
#include "iframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "mainmenu.h"
#include "adminset.h"

static void admset_none_func(void*);
static void admset_menu_func(void*);
static void admset_apply_func(void*);
static void admset_exit_func(void*);

inpfoc_funclist_t inpfoc_adminset_funclist[ADMSET_NUM_ITEMS+1] = 
{
  { ADMSET_NONE,      admset_none_func      },
  { ADMSET_MENU,      admset_menu_func      },
  { ADMSET_APPLY,     admset_apply_func     },
  { ADMSET_EXIT,      admset_exit_func      },
  { -1        ,       admset_none_func      }, // -1 must be last
};

static void admset_none_func(void * parg)
{

}

static void admset_menu_func(void * parg)
{
  inpfoc_enable(INPFOC_ADMSET, ADMSET_APPLY);
}

static void admset_apply_func(void * parg)
{
  admin_mode = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_ADMSET, ADMSET_MENU)) ? 1 : 0;
  inpfoc_disable(INPFOC_ADMSET, ADMSET_APPLY);
  inpfoc_set(INPFOC_ADMSET, ADMSET_EXIT);
}

static void admset_exit_func(void * parg)
{
  inpfoc_disable(INPFOC_ADMSET, ADMSET_APPLY);

  uframe_command(UFRAME_CHANGE, (void*)MENU_GENERAL);
}

void menu_adminset_openproc(void)
{
  inpfoc_set(INPFOC_ADMSET, ADMSET_EXIT);
}

