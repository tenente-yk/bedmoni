#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include "bedmoni.h"
#include "iframe.h"
#include "cframe.h"
#include "mframe.h"
#include "uframe.h"
#include "tframe.h"
#include "lframe.h"
#include "tframe.h"
#include "eframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "general.h"

static void general_none_func(void*);
static void general_pat_func(void*);
static void general_alr_func(void*);
static void general_scr_func(void*);
static void general_evnt_func(void*);
static void general_trn_func(void*);
static void general_tbl_func(void*);
static void general_prnset_func(void*);
static void general_set_func(void*);
static void general_admin_func(void*);
static void general_exit_func(void*);

inpfoc_funclist_t inpfoc_general_funclist[GENERAL_NUM_ITEMS+1] = 
{
  { GENERAL_PAT,  general_pat_func          },
  { GENERAL_ALR,  general_alr_func          },
  { GENERAL_CALC, general_none_func         },
  { GENERAL_SCR,  general_scr_func          },
  { GENERAL_EVNT, general_evnt_func         },
  { GENERAL_TRN,  general_trn_func          },
  { GENERAL_TBL,  general_tbl_func          },
  { GENERAL_PRN,  general_prnset_func       },
  { GENERAL_SET,  general_set_func          },
  { GENERAL_ADMIN,general_admin_func        },
  { GENERAL_EXIT, general_exit_func         },
  { -1          , general_none_func         }, // -1 must be last
};

static void general_none_func(void * parg)
{

}

static void general_pat_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_PATIENT);
}

static void general_alr_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_ALARMS);
}

static void general_scr_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_DISPSET);
}

static void general_evnt_func(void* parg)
{
  cframe_command(CFRAME_HIDE, NULL);
  dview_frame_active(EFRAME);
}

static void general_trn_func(void* parg)
{
  cframe_command(CFRAME_HIDE, NULL);
  dview_frame_active(TFRAME);
}

static void general_tbl_func(void* parg)
{
  cframe_command(CFRAME_HIDE, NULL);
  dview_frame_active(LFRAME);
}

static void general_prnset_func(void* parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_PRINTSET);
}

static void general_set_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
}

static void general_admin_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_ADMSET);
}

static void general_exit_func(void * parg)
{
  uframe_command(UFRAME_DESTROY, NULL);
}

void menu_general_openproc(void)
{
  inpfoc_set(INPFOC_GENERAL, GENERAL_EXIT);

  inpfoc_disable(INPFOC_GENERAL, GENERAL_CALC);
 // inpfoc_disable(INPFOC_GENERAL, GENERAL_EVNT);
}

