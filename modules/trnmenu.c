#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include "bedmoni.h"
#include "uframe.h"
#include "grph.h"
#include "cframe.h"
#include "mframe.h"
#include "tframe.h"
#include "lframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "mainmenu.h"
#include "trnmenu.h"

static void trn_none_func(void*);
static void trn_lseek_func(void*);
static void trn_seek_func(void*);
static void trn_end_func(void*);
static void trn_step_func(void*);
static void trn_set_func(void*);
static void trn_exit_func(void*);

inpfoc_funclist_t inpfoc_trn_funclist[TRENDS_NUM_ITEMS+1] = 
{
  { TRENDS_NONE,    trn_none_func         },
  { TRENDS_LSEEK,   trn_lseek_func        },
  { TRENDS_SEEK,    trn_seek_func         },
  { TRENDS_END,     trn_end_func          },
  { TRENDS_STEP,    trn_step_func         },
  { TRENDS_SET,     trn_set_func          },
  { TRENDS_EXIT,    trn_exit_func         },
  { -1         ,    trn_none_func         }, // -1 must be last
};

static void trn_none_func(void * parg)
{

}

static void trn_lseek_func(void* parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_command(TFRAME_CURSOR_LSEEK, (void*)pcmd->delta);
}

static void trn_seek_func(void* parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_command(TFRAME_CURSOR_SEEK, (void*)pcmd->delta);
}

static void trn_end_func(void* parg)
{
  tframe_command(TFRAME_CURSOR_SEEKEND, NULL);
}

static void trn_step_func(void* parg)
{
  tframe_command(TFRAME_CHANGE_STEP, NULL);
}

static void trn_set_func(void * parg)
{
  rect_t rc;

  rc.x0 = TFRAME_X0+UFRAME_CX;
  rc.y0 = TFRAME_Y0;
  rc.x1 = rc.x0 + TFRAME_CX-UFRAME_CX;
  rc.y1 = rc.y0 + TFRAME_CY;
  lframe_command(TFRAME_RESIZE, &rc);

  uframe_command(UFRAME_CREATE, (void*)MENU_TRENDSET);
}

static void trn_exit_func(void * parg)
{
 // lframe_command(LFRAME_DESTROY, NULL);
 // tframe_command(TFRAME_DESTROY, NULL);

  dview_frame_active(CFRAME);

  cframe_command(CFRAME_RELOAD, NULL);
  mframe_command(MFRAME_RELOAD, NULL);
}
