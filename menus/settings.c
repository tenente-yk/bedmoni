#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include "bedmoni.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "settings.h"

static void set_none_func(void*);
static void set_dt_func(void*);
static void set_lang_func(void*);
static void set_mod_func(void*);
static void set_def_func(void*);
static void set_demo_func(void*);
static void set_info_func(void*);
static void set_net_func(void*);
static void set_exit_func(void*);

inpfoc_funclist_t inpfoc_set_funclist[SET_NUM_ITEMS+1] = 
{
  { SET_NONE,  set_none_func         },
  { SET_DT,    set_dt_func           },
  { SET_LANG,  set_lang_func         },
  { SET_MOD,   set_mod_func          },
  { SET_DEF,   set_def_func          },
  { SET_SUPP,  set_none_func         },
  { SET_DEMO,  set_demo_func         },
  { SET_INFO,  set_info_func         },
  { SET_NET,   set_net_func          },
  { SET_EXIT,  set_exit_func         },
  { -1      ,  set_none_func         }, // -1 must be last
};

static void set_none_func(void * parg)
{

}

static void set_dt_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_DATETIME);
}

static void set_lang_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_LANGSEL);
}

static void set_mod_func(void * parg)
{
 // menu_password_onokmenu(MENU_MODULES);
  uframe_command(UFRAME_CHANGE, (void*)MENU_MODULES);
}

static void set_def_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_DEFAULTSET);
}

static void set_demo_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_DEMOSET);
}

static void set_info_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_APPINFO);
}

static void set_net_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_NETWORKSET);
}

static void set_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_GENERAL);
}

void menu_settings_openproc(void)
{
 // inpfoc_disable(INPFOC_SET, SET_SUPP);

  inpfoc_set(INPFOC_SET, SET_EXIT);
}

