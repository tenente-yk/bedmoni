#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "uframe.h"
#include "iframe.h"
#include "lframe.h"
#include "tframe.h"
#include "inpfoc.h"
#include "tr.h"
#include "inpfoc_wnd.h"
#include "patient.h"
#include "patientnew.h"

static void patnew_none_func(void*);
static void patnew_yes_func(void*);
static void patnew_no_func(void*);

static int h0[NUM_PAT_CAT] =
{
  170,
  90,
#if defined (FEATURE_NEONATE)
  40,
#endif
};
static int w0[NUM_PAT_CAT] =
{
  70,
  200,
#if defined (FEATURE_NEONATE)
  2550,
#endif
};

inpfoc_funclist_t inpfoc_patnew_funclist[PATNEW_NUM_ITEMS+1] = 
{
  { PATNEW_NONE,   patnew_none_func         },
  { PATNEW_YES,    patnew_yes_func          },
  { PATNEW_NO,     patnew_no_func           },
  { -1      ,      patnew_none_func         }, // -1 must be last
};

static void patnew_none_func(void * parg)
{

}

static void patnew_yes_func(void * parg)
{
  pat_pars_t lpp;

  pat_get(&lpp);
  assert(lpp.type < NUM_PAT_CAT);

  lpp.h = h0[lpp.type];
  lpp.w = w0[lpp.type];

  pat_set(&lpp);

 // iframe_ioctl(IFRAME_SET_ID, IFRAME_ID_PAT, lpp.type);
 // iframe_ioctl(IFRAME_SET_ID, IFRAME_ID_BED, 0);
 // iframe_ioctl(IFRAME_SET_ID, IFRAME_ID_BEDNO, lpp.bedno);
 // iframe_ioctl(IFRAME_SET_ID, IFRAME_ID_CARDNO, lpp.cardno);

  // reset thresholds to current category

  lframe_command(LFRAME_NEWDATA, NULL);
  tframe_command(TFRAME_NEWDATA, NULL);

  uframe_command(UFRAME_CHANGE, (void*)MENU_PATIENT);
  // clean trends
 // debug("%s: to do clean trends\n", __FUNCTION__);
  tr_reset();
}

static void patnew_no_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_PATIENT);
}

void menu_patientnew_openproc(void)
{
  inpfoc_set(INPFOC_PATNEW, PATNEW_NO);
}

