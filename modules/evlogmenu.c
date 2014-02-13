#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "uframe.h"
#include "grph.h"
#include "iframe.h"
#include "cframe.h"
#include "mframe.h"
#include "tframe.h"
#include "lframe.h"
#include "eframe.h"
#include "elog.h"
#include "tr.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "mainmenu.h"
#include "evlogmenu.h"

static void evlog_none_func(void*);
static void evlog_seek0_func(void*);
static void evlog_vscroll_func(void*);
static void evlog_goto_table_func(void*);
static void evlog_set_func(void*);
static void evlog_exit_func(void*);

inpfoc_funclist_t inpfoc_evlog_funclist[EVLOG_NUM_ITEMS+1] = 
{
  { EVLOG_NONE,    evlog_none_func          },
  { EVLOG_SEEK0,   evlog_seek0_func         },
  { EVLOG_VSCROLL, evlog_vscroll_func       },
  { EVLOG_GOTO_TABLE, evlog_goto_table_func },
  { EVLOG_SET,     evlog_set_func           },
  { EVLOG_EXIT,    evlog_exit_func          },
  { -1         ,   evlog_none_func          }, // -1 must be last
};

static void evlog_none_func(void * parg)
{

}

static void evlog_seek0_func(void * parg)
{
  eframe_t * peframe = eframe_getptr();
  peframe->offset = 0;
  eframe_command(EFRAME_RELOAD, NULL);
}

static void evlog_vscroll_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  eframe_command(EFRAME_VSCROLL, (void*)pcmd->delta);
}

static void evlog_goto_table_func(void * parg)
{
  dview_frame_active(LFRAME);

#if 0
  eframe_t * peframe = eframe_getptr();
  lframe_t * plframe = lframe_getptr();
  int i;
  trts_t trts1;
  elog_data_t eld;

  trts1.year = 2099 - 1900;
  trts1.mon  = 12;
  trts1.day  = 31;
  trts1.hour = 23;
  trts1.min  = 59;

  i = elog_read(peframe->offset, 1, &eld);
  if (i>0 && eld.type == EL_ALARM)
  {
    int ntrh;
    trh_t ltrh;
    int offs, offs0;
    short tsm[1] = {1};

    plframe->tsm = TSM_1;
    ntrh = trh_size();
    assert(plframe->tsm == 0);

    offs = 0;
    for (i=ntrh-1; i>=0; i--)
    {
      trh_read(i, &ltrh);
//printf("%02d.%02d - %02d.%02d (%d - %d)\n", ltrh.trts0.hour, ltrh.trts0.min, ltrh.trts1.hour, ltrh.trts1.min, ltrh.offs0, ltrh.offs1);
      if (ltrh.offs1 == ltrh.offs0) continue;
      offs0 = ( (ltrh.offs1 - ltrh.offs0)/sizeof(tr_t) ) / tsm[plframe->tsm];
//debug("<> t0: %02d:%02d       t1: %02d:%02d     e: %02d:%02d\n", ltrh.trts0.hour, ltrh.trts0.min, ltrh.trts1.hour, ltrh.trts1.min, eld.trts.hour, eld.trts.min);
      if ( trtscmp(&ltrh.trts0, &eld.trts) < 0)
      {
      //  plframe->voffs = offs + trtscmp(&eld.trts, &ltrh.trts0) - 1;

        dview_frame_active(LFRAME);
       // cframe_command(CFRAME_RELOAD, NULL);
       // mframe_command(MFRAME_RELOAD, NULL);
        lframe_command(LFRAME_VSCROLL, (void*)( offs + ((trtscmp(&ltrh.trts1, &eld.trts) < 0) ? -1 : trtscmp(&ltrh.trts1, &eld.trts)) ));

//printf("offs0 %d, cmp %d  %d            %d\n", offs, trtscmp(&eld.trts, &ltrh.trts0), (trtscmp(&ltrh.trts1, &eld.trts) < 0) ? -1 : trtscmp(&ltrh.trts1, &eld.trts),     offs + ((trtscmp(&ltrh.trts1, &eld.trts) < 0) ? -1 : trtscmp(&ltrh.trts1, &eld.trts)));
//debug("t0: %02d:%02d       t1: %02d:%02d\n", ltrh.trts0.hour, ltrh.trts0.min, ltrh.trts1.hour, ltrh.trts1.min);
        break;
      }

      offs += offs0;
    }
   // debug("goto to time: %02d:%02d (%d)\n", eld.trts.hour, eld.trts.min, offs0);
  }
  else
  {
    error("cannot open this line in table\n");
  }
#endif
}

static void evlog_set_func(void * parg)
{
  uframe_command(UFRAME_CREATE, (void*)MENU_EVLOGSET);
}

static void evlog_exit_func(void * parg)
{
 // lframe_command(LFRAME_DESTROY, NULL);
 // tframe_command(TFRAME_DESTROY, NULL);
 // eframe_command(EFRAME_DESTROY, NULL);
  dview_frame_active(CFRAME);
  cframe_command(CFRAME_RELOAD, NULL);
  mframe_command(MFRAME_RELOAD, NULL);
}
