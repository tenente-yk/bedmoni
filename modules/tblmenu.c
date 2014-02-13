#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include "bedmoni.h"
#include "grph.h"
#include "uframe.h"
#include "cframe.h"
#include "mframe.h"
#include "lframe.h"
#include "tframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "mainmenu.h"
#include "tblmenu.h"

static void tbl_none_func(void*);
static void tbl_ts_func(void*);
static void tbl_mode_func(void*);
static void tbl_vscroll_func(void*);
static void tbl_hscroll_func(void*);
static void tbl_print_func(void*);
static void tbl_set_func(void*);
static void tbl_exit_func(void*);

inpfoc_funclist_t inpfoc_tbl_funclist[TABLE_NUM_ITEMS+1] = 
{
  { TABLE_NONE,    tbl_none_func         },
  { TABLE_TS,      tbl_ts_func           },
  { TABLE_MODE,    tbl_mode_func         },
  { TABLE_VSCROLL, tbl_vscroll_func      },
  { TABLE_HSCROLL, tbl_hscroll_func      },
  { TABLE_PRINT,   tbl_print_func        },
  { TABLE_SET,     tbl_set_func          },
  { TABLE_EXIT,    tbl_exit_func         },
  { -1         ,   tbl_none_func         }, // -1 must be last
};

static void tbl_none_func(void * parg)
{

}

static void tbl_ts_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  lframe_command(LFRAME_CHANGE_TS, (void*)pcmd->delta);
}

static void tbl_mode_func(void * parg)
{
  char s[200];
  lframe_t * plframe = lframe_getptr();
  if (plframe->mode == LFRAME_MODE_BRIEF)
  {
    plframe->mode = LFRAME_MODE_FULL;
    ids2string(IDS_BRIEF, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TABLE, TABLE_MODE), s);
  }
  else if (plframe->mode == LFRAME_MODE_FULL)
  {
    plframe->mode = LFRAME_MODE_BRIEF;
    ids2string(IDS_DETAILED, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TABLE, TABLE_MODE), s);
  }
  else
  {
    error("%s: unknown mode\n", __FUNCTION__);
  }

  lframe_command(LFRAME_RELOAD, NULL);
}

static void tbl_vscroll_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  lframe_command(LFRAME_VSCROLL, (void*)pcmd->delta);
}

static void tbl_hscroll_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  lframe_command(LFRAME_HSCROLL, (void*)pcmd->delta);
}

static void tbl_print_func(void * parg)
{
  debug("%s not implemented\n", __FUNCTION__);
}

static void tbl_set_func(void * parg)
{
  uframe_command(UFRAME_CREATE, (void*)MENU_TABLESET);
}

static void tbl_exit_func(void * parg)
{
 // lframe_command(LFRAME_DESTROY, NULL);
 // tframe_command(TFRAME_DESTROY, NULL);

  dview_frame_active(CFRAME);

  cframe_command(CFRAME_RELOAD, NULL);
  mframe_command(MFRAME_RELOAD, NULL);
}
