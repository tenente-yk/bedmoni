#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "grph.h"
#include "uframe.h"
#include "cframe.h"
#include "lframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "tableset.h"

extern tbl_par_t tbl_par[NUM_PARAMS];

static void tableset_none_func(void*);
static void tableset_par_func(void*);
static void tableset_mode_func(void*);
static void tableset_default_func(void*);
static void tableset_exit_func(void*);

inpfoc_funclist_t inpfoc_tblset_funclist[TABLESET_NUM_ITEMS+1] = 
{
  { TABLESET_NONE,     tableset_none_func         },
  { TABLESET_PAR1,     tableset_par_func          },
  { TABLESET_PAR2,     tableset_par_func          },
  { TABLESET_PAR3,     tableset_par_func          },
  { TABLESET_PAR4,     tableset_par_func          },
  { TABLESET_PAR5,     tableset_par_func          },
  { TABLESET_PAR6,     tableset_par_func          },
  { TABLESET_PAR7,     tableset_par_func          },
  { TABLESET_PAR8,     tableset_par_func          },
  { TABLESET_PAR9,     tableset_par_func          },
  { TABLESET_PAR10,    tableset_par_func          },
  { TABLESET_PAR11,    tableset_par_func          },
  { TABLESET_PAR12,    tableset_par_func          },
  { TABLESET_PAR13,    tableset_par_func          },
  { TABLESET_PAR14,    tableset_par_func          },
  { TABLESET_PAR15,    tableset_par_func          },
  { TABLESET_DEFAULT,  tableset_default_func      },
  { TABLESET_MODE,     tableset_mode_func         },
  { TABLESET_EXIT,     tableset_exit_func         },
  { -1       ,         tableset_none_func         }, // -1 must be last
};

static void tableset_none_func(void * parg)
{

}

static void tableset_par_func(void * parg)
{
  char s[200], s1[200];
  int i;
  inpfoc_item_t * pifi;
  tbl_cs_t tbl_cs;
  inpfoc_cmd_t * pcmd = (inpfoc_cmd_t *) parg;

  pifi = inpfoc_find_active(INPFOC_TABLESET);
  assert(pifi);

  inpfoc_wnd_getcaption(pifi, s, sizeof(s));
 // printf("%s %d\n", s, pifi->item);

  for (i=NUM_PARAMS-1; i>PARAM_NONE; i--)
  {
    ids2string(tbl_par[i].ids_name, s1);
    if (strcmp(s, s1) == 0)
      break;
  }

  i += pcmd->delta;
  while (i<0)           i += NUM_PARAMS;
  while (i>=NUM_PARAMS) i -= NUM_PARAMS;

  ids2string(tbl_par[i].ids_name, s1);
  inpfoc_wnd_setcaption(pifi, s1);

  tbl_cs_read(&tbl_cs);
  tbl_cs.id[pifi->item - TABLESET_PAR_FIRST + 0] = i;
  tbl_cs_write(&tbl_cs);
}

static void tableset_mode_func(void * parg)
{
  lframe_t * plframe = lframe_getptr();
  char s[200];

  if (plframe->mode == LFRAME_MODE_BRIEF)
  {
    plframe->mode = LFRAME_MODE_FULL;
    ids2string(IDS_BRIEF, s);
   // ids2string(IDS_DETAILED, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TABLESET, TABLESET_MODE), s);
  }
  else if (plframe->mode == LFRAME_MODE_FULL)
  {
    plframe->mode = LFRAME_MODE_BRIEF;
   // ids2string(IDS_BRIEF, s);
    ids2string(IDS_DETAILED, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TABLESET, TABLESET_MODE), s);
  }
  else
  {
    debug("%s: unknown mode\n", __FUNCTION__);
  }

  lframe_command(LFRAME_RELOAD, NULL);
}

static void tableset_default_func(void * parg)
{
  lframe_ini_def();
  lframe_command(LFRAME_RELOAD, NULL);
}

static void tableset_exit_func(void * parg)
{
  uframe_command(UFRAME_DESTROY, NULL);

#if 0
  rect_t rc;
  rc.x0 = LFRAME_X0;
  rc.y0 = LFRAME_Y0;
  rc.x1 = rc.x0 + LFRAME_CX;
  rc.y1 = rc.y0 + LFRAME_CY;
  lframe_command(LFRAME_RESIZE, &rc);
#endif
  lframe_command(LFRAME_RELOAD, NULL);

  lframe_cfg_save();
}

void menu_tableset_openproc(void)
{
  tbl_cs_t tbl_cs;
  tbl_par_t tp;
  int num_table_pars;
  int i;
  lframe_t * plframe = lframe_getptr();
  char s[200];

  num_table_pars = min(TABLE_MAX_NUM_IDS, TABLESET_PAR_LAST-TABLESET_PAR_FIRST+1);

  tbl_cs_read(&tbl_cs);
  for (i=0; i<num_table_pars; i++)
  {
    id2tblpar(tbl_cs.id[i], &tp);
    ids2string(tp.ids_name, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TABLESET, TABLESET_PAR1+i), s);
  }

  if (plframe->mode == LFRAME_MODE_BRIEF)
  {
    ids2string(IDS_DETAILED, s);
   // ids2string(IDS_BRIEF, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TABLESET, TABLESET_MODE), s);
  }
  else if (plframe->mode == LFRAME_MODE_FULL)
  {
   // ids2string(IDS_DETAILED, s);
    ids2string(IDS_BRIEF, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TABLESET, TABLESET_MODE), s);
  }
  else
    debug("%s: unknown mode\n", __FUNCTION__);

  inpfoc_set(INPFOC_TABLESET, TABLESET_EXIT);
}

