#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "bedmoni.h"
#include "crbpio.h"
#include "cframe.h"
#include "mframe.h"
#include "eframe.h"
#include "lframe.h"
#include "tframe.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "stconf.h"
#include "disp.h"
#include "dispset.h"

static void disp_none_func(void*);
static void disp_enlarge_func(void*);
static void disp_default_func(void*);
static void disp_brgh_func(void*);
static void disp_none_func(void*);
static void disp_exit_func(void*);

inpfoc_funclist_t inpfoc_dispset_funclist[DISP_NUM_ITEMS+1] = 
{
  { DISP_NONE,        disp_none_func         },
  { DISP_ENLARGE,     disp_enlarge_func      },
  { DISP_DEFAULT,     disp_default_func      },
  { DISP_BRGH,        disp_brgh_func         },
  { DISP_EXIT,        disp_exit_func         },
  { -1       ,        disp_none_func         }, // -1 must be last
};

static void disp_none_func(void * parg)
{

}

static void disp_enlarge_func(void * parg)
{
  uframe_clearbox(20, 195, 2, 20);

 // if (cframe_getptr()->visible == 0)
  if ((lframe_getptr()->visible || tframe_getptr()->visible || eframe_getptr()->visible))
  {
    char s[200];
    ids2string(IDS_OPERATION_IMPOSSIBLE, s);
    uframe_printbox(20, 195, -1, -1, s, RGB(0xAA,0x00,0x00));
    inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, DISP_ENLARGE), (mframe_getptr()->maximized)?IWC_CHECKED:IWC_UNCHECKED);
    inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, DISP_DEFAULT), (mframe_getptr()->maximized)?IWC_UNCHECKED:IWC_CHECKED);
    return;
  }

  mframe_command(MFRAME_MAXIMIZE, NULL);
  inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, DISP_ENLARGE), IWC_CHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, DISP_DEFAULT), IWC_UNCHECKED);
}

static void disp_default_func(void * parg)
{
  mframe_t * pmframe;
  int i, n;

  pmframe = mframe_getptr();
  uframe_clearbox(20, 195, 2, 20);

  if (lframe_getptr()->visible || tframe_getptr()->visible || eframe_getptr()->visible)
  {
    char s[200];
    ids2string(IDS_OPERATION_IMPOSSIBLE, s);
    uframe_printbox(20, 195, -1, -1, s, RGB(0xAA,0x00,0x00));
    inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, DISP_ENLARGE), (pmframe->maximized)?IWC_CHECKED:IWC_UNCHECKED);
    inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, DISP_DEFAULT), (pmframe->maximized)?IWC_UNCHECKED:IWC_CHECKED);
    return;
  }

  n = 0;
  for (i=0; i<NUM_VIEWS; i++)
  {
    n += ((pmframe->unit_chmask >> i) & 0x1);
  }

  n -= 5;
  for (i=NUM_VIEWS-1; i>=0 && n>0; i--)
  {
    if (pmframe->punit[i]->visible)
    {
      unit_ioctl(i, UNIT_CMD_HIDE);
      cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(i, 0));
      n --;
    }
  }

  mframe_command(MFRAME_NORMALIZE, NULL);
  inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, DISP_DEFAULT), IWC_CHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, DISP_ENLARGE), IWC_UNCHECKED);
}

static void disp_brgh_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[10];
  dispset_t ds;
  int v;

  disp_get(&ds);

  v = ds.brightness;
  v += 20*pcmd->delta;

  while (v < 20)  v += 100;
  while (v > 100) v -= 100;

  ds.brightness = v;

  snprintf(s, sizeof(s), "%d", ds.brightness / 20);

 // debug("crbpio_send_msg(CRBP_SET_BRIGHT, 0x00, 0x00, %d, 0x00);\n", ds.brightness);
  crbpio_send_msg(CRBP_SET_BRIGHT, 0x00, 0x00, ds.brightness, 0x00);

  disp_set(&ds);
  disp_cfg_save();

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DISP, DISP_BRGH), s);
}

static void disp_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_GENERAL);
}

void menu_dispset_openproc(void)
{
  mframe_t * pmframe = mframe_getptr();
  dispset_t ds;
  char s[10];

  inpfoc_wnd_check(inpfoc_find(INPFOC_DISP, (pmframe->maximized) ? DISP_ENLARGE : DISP_DEFAULT), IWC_CHECKED);

  disp_get(&ds);

  sprintf(s, "%d", (int)ds.brightness/20);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DISP, DISP_BRGH), s);

  inpfoc_set(INPFOC_DISP, DISP_EXIT);
}

static dispset_t dispset;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void disp_get(dispset_t * pds)
{
  pthread_mutex_lock(&mutex);

  if (pds) *pds = dispset;

  pthread_mutex_unlock(&mutex);
}

void disp_set(dispset_t * pds)
{
  pthread_mutex_lock(&mutex);

  if (pds) memcpy(&dispset, pds, sizeof(dispset_t));

  pthread_mutex_unlock(&mutex);
}

void disp_ini(void)
{
  dispset_t ds;

  disp_ini_def();

  memset(&ds, 0, sizeof(dispset_t));
  if ( stconf_read(STCONF_DISP, &ds, sizeof(dispset_t)) > 0 )
  {
    disp_set(&ds);
  }

  disp_get(&ds);
 // debug("set brightness %d\n", ds.brightness);
  crbpio_send_msg(CRBP_SET_BRIGHT, 0x00, 0x00, ds.brightness, 0x00);
}

void disp_ini_def(void)
{
  dispset_t ds;

  ds.brightness = 100;

  disp_set(&ds);
}

void disp_deini(void)
{
  disp_cfg_save();
}

void disp_cfg_save(void)
{
  dispset_t ds;
  disp_get(&ds);
  if ( stconf_write(STCONF_DISP, &ds, sizeof(dispset_t)) > 0 ) { }
}

