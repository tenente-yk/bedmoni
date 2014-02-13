#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "uframe.h"
#include "eframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "elog.h"
#include "evlogset.h"

static void evlogset_none_func(void*);
static void evlogset_font_func(void*);
static void evlogset_clrall_func(void*);
static void evlogset_exit_func(void*);

inpfoc_funclist_t inpfoc_evlogset_funclist[EVLOGSET_NUM_ITEMS+1] = 
{
  { EVLOGSET_NONE,    evlogset_none_func     },
  { EVLOGSET_FONT,    evlogset_font_func     },
  { EVLOGSET_CLRALL,  evlogset_clrall_func   },
  { EVLOGSET_EXIT,    evlogset_exit_func     },
  { -1       ,        evlogset_none_func     }, // -1 must be last
};

static void evlogset_none_func(void * parg)
{
}

static void evlogset_font_func(void * parg)
{
  eframe_t * peframe = eframe_getptr();
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  unsigned short ids;
  int v;

  switch(peframe->font)
  {
    case EFRAME_FONT_SMALL:
      v = 0;
      break;
    case EFRAME_FONT_NORMAL:
      v = 1;
      break;
    case EFRAME_FONT_LARGE:
      v = 2;
      break;
    default:
      v = 1;
      break;
  }

  v += pcmd->delta;
  while (v<0) v += 2+1;
  while (v>2) v -= 2+1;

  switch(v)
  {
    case 0:
      peframe->font = EFRAME_FONT_SMALL;
      ids = IDS_SMALL;
      break;
    case 1:
      peframe->font = EFRAME_FONT_NORMAL;
      ids = IDS_NORMAL;
      break;
    case 2:
      peframe->font = EFRAME_FONT_LARGE;
      ids = IDS_LARGE;
      break;
    default:
      peframe->font = EFRAME_FONT_NORMAL;
      ids = IDS_NORMAL;
      break;
  }
  inpfoc_wnd_setids(inpfoc_find(INPFOC_EVLOGSET, EVLOGSET_FONT), ids);

  eframe_command(EFRAME_RELOAD, NULL);
}

static void evlogset_clrall_func(void * parg)
{
  char s[200];
  sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, ELOG_FNAME);
  elog_close();
  unlink(s);
  elog_open();

  eframe_t * peframe = eframe_getptr();
  peframe->offset = 0;
  eframe_command(EFRAME_NEWDATA, NULL);
}

static void evlogset_exit_func(void * parg)
{
  uframe_command(UFRAME_DESTROY, NULL);
}

void menu_evlogset_openproc(void)
{
  eframe_t * peframe = eframe_getptr();
  unsigned short ids;

  switch(peframe->font)
  {
    case EFRAME_FONT_SMALL:
      ids = IDS_SMALL;
      break;
    case EFRAME_FONT_NORMAL:
      ids = IDS_NORMAL;
      break;
    case EFRAME_FONT_LARGE:
      ids = IDS_LARGE;
      break;
    default:
      ids = IDS_NORMAL;
      break;
  }

  inpfoc_wnd_setids(inpfoc_find(INPFOC_EVLOGSET, EVLOGSET_FONT), ids);

  inpfoc_set(INPFOC_EVLOGSET, EVLOGSET_EXIT);
}

