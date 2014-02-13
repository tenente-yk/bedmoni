#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "iframe.h"
#include "unit.h"
#include "patient.h"
#include "iframe.h"
#include "stconf.h"
#include "defaultset.h"
#if defined (TEST_IDSTEXT_MODE)
#include "cframe.h"
#endif

static void pat_none_func(void*);
static void pat_new_func(void*);
static void pat_cat_func(void*);
static void pat_bed_func(void*);
static void pat_n_func(void*);
static void pat_h_func(void*);
static void pat_w_func(void*);
static void pat_exit_func(void*);

static int h0[NUM_PAT_CAT] = 
{
  170,
  90,
#if defined (NEONATE_FEATURE)
  40,
#endif
};
static int w0[NUM_PAT_CAT] = 
{
  70,
  200,
#if defined (NEONATE_FEATURE)
  2550
#endif
};

inpfoc_funclist_t inpfoc_pat_funclist[PAT_NUM_ITEMS+1] = 
{
  { PAT_NONE,   pat_none_func         },
  { PAT_NEW,    pat_new_func          },
  { PAT_CAT,    pat_cat_func          },
  { PAT_BED,    pat_bed_func          },
  { PAT_N,      pat_n_func            },
  { PAT_H,      pat_h_func            },
  { PAT_W,      pat_w_func            },
  { PAT_EXIT,   pat_exit_func         },
  { -1      ,   pat_none_func         }, // -1 must be last
};

static void pat_none_func(void * parg)
{

}

static void pat_new_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_PATIENTNEW);
}

static void pat_cat_func(void * parg)
{
  inpfoc_item_t * pifi;
  pat_pars_t lpp;
  inpfoc_cmd_t cmd;
  unsigned short ids;
  char s[128];

  pifi = inpfoc_find(INPFOC_PAT, PAT_CAT);
  assert(pifi);

  pat_get(&lpp);
  lpp.type += 1;
  lpp.type %= NUM_PAT_CAT;

  lpp.h = h0[lpp.type];
  lpp.w = w0[lpp.type];

  pat_set(&lpp);

  ids = IDS_WEIGHT_KG;
#if defined (NEONATE_FEATURE)
  if (lpp.type == NEONATE)
    ids = IDS_WEIGHT_G;
#endif
  ids2string(ids, s);
  uframe_clearbox(10, 172, 110, 20);
  uframe_printbox(10, 172, -1, -1, s, UFRAME_STATICTEXT_COLOR);

  memset(&cmd, 0, sizeof(inpfoc_cmd_t));
  cmd.delta = 0;

  pat_h_func(&cmd);
  pat_w_func(&cmd);

  ids2string(pat_cat_ids[lpp.type], s);
  inpfoc_wnd_setcaption(pifi, s);

  iframe_command(IFRAME_SET_PAT, (void*) lpp.type);

  unit_ini_def(TRUE);
 // unit_update_data_all();
}

static void pat_bed_func(void * parg)
{
  inpfoc_item_t *pit;
  pat_pars_t lpp;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[20];

  pit = inpfoc_find(INPFOC_PAT, PAT_BED);

  pat_get(&lpp);
  lpp.bedno += pcmd->delta;
  while (lpp.bedno < 0) lpp.bedno += 1000;
  lpp.bedno %= 1000;

  pat_set(&lpp);

  snprintf(s, sizeof(s), "%d", lpp.bedno);

  inpfoc_wnd_setcaption(pit, s);

  iframe_command(IFRAME_SET_BEDNO, (void*)lpp.bedno);

#if defined (TEST_IDSTEXT_MODE)
  rect_t rect;
  cframe_t * pcframe = cframe_getptr();
  assert(pcframe);
  pat_pars_t pat_pars;
  dc_list_t dclist;
  char s2[200];

  pat_get(&pat_pars);
  grph_setfgcolor(pcframe->updc, demo_mode ? RGB(0xEE,0xEE,0xEE) : 0x000000);
  ids2string(pat_pars.bedno, s2);
  grph_setfont(pcframe->updc, MONOS_10); // inpfoc
  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 100;
  rect.y1 = rect.y0 + 30;
  grph_fillrect(pcframe->updc, rect.x0, rect.y0, rect.x1-rect.x0, rect.y1-rect.y0, 0x000000);
  grph_drawtext(pcframe->updc, s2, -1, &rect, DT_CENTER);

  grph_setfont(pcframe->updc, ARIAL_14); // iframe, eframe
  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 130;
  rect.y1 = rect.y0 + 40;
  grph_fillrect(pcframe->updc, rect.x0, rect.y0, rect.x1-rect.x0, rect.y1-rect.y0, 0x000000);
  grph_drawtext(pcframe->updc, s2, -1, &rect, DT_CENTER);

  grph_setfont(pcframe->updc, ARIAL_12); // eframe
  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 170;
  rect.y1 = rect.y0 + 30;
  grph_fillrect(pcframe->updc, rect.x0, rect.y0, rect.x1-rect.x0, rect.y1-rect.y0, 0x000000);
  grph_drawtext(pcframe->updc, s2, -1, &rect, DT_CENTER);

  grph_setfont(pcframe->updc, ARIAL_10); // tframe, eframe
  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 200;
  rect.y1 = rect.y0 + 20;
  grph_fillrect(pcframe->updc, rect.x0, rect.y0, rect.x1-rect.x0, rect.y1-rect.y0, 0x000000);
  grph_drawtext(pcframe->updc, s2, -1, &rect, DT_CENTER);

  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 100;
  rect.y1 = rect.y0 + 130;

  dclist.xres = ((PGDC)(pcframe->bkdc))->xres;
  dclist.yres = ((PGDC)(pcframe->bkdc))->yres;
  dclist.p_bk = (unsigned char*)((PGDC)(pcframe->bkdc))->addr;
  dclist.p_fg = (unsigned char*)((PGDC)(pcframe->fgdc))->addr;
  dclist.p_up = (unsigned char*)((PGDC)(pcframe->updc))->addr;

  dview_bitblt(pcframe->x+rect.x0, pcframe->y+rect.y0, rect.x1-rect.x0, rect.y1-rect.y0, &dclist, rect.x0, rect.y0);
#endif
}

static void pat_n_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  pat_pars_t lpp;
  char s[10];

  pit = inpfoc_find(INPFOC_PAT, PAT_N);
  assert(pit);

  pat_get(&lpp);
  lpp.cardno += pcmd->delta;
  while (lpp.cardno < 0) lpp.cardno += 100000;
  lpp.cardno %= 100000;

  pat_set(&lpp);

  snprintf(s, sizeof(s), "%d", lpp.cardno);

  inpfoc_wnd_setcaption(pit, s);

  iframe_command(IFRAME_SET_CARDNO, (void*)lpp.cardno);
}

static void pat_h_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[10];
  pat_pars_t lpp;
  int hmax = 0, hmin = 0;

  pit = inpfoc_find(INPFOC_PAT, PAT_H);
  assert(pit);

  pat_get(&lpp);

  if (lpp.type == ADULT)
  {
    hmin = 80;
    hmax = 220;
  }
  else if (lpp.type == CHILD)
  {
    hmin = 50;
    hmax = 160;
  }
#if defined (NEONATE_FEATURE)
  else if (lpp.type == NEONATE)
  {
    hmin = 18;
    hmax = 100;
  }
#endif

  lpp.h += pcmd->delta;
  while (lpp.h < hmin) lpp.h += (hmax-hmin);
  while (lpp.h > hmax) lpp.h -= (hmax-hmin);

  pat_set(&lpp);

  snprintf(s, sizeof(s), "%d", lpp.h);

  inpfoc_wnd_setcaption(pit, s);
}

static void pat_w_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[10];
  pat_pars_t lpp;
  float wmax = 0, wmin = 0;
  float wstp = 0;

  pat_get(&lpp);
  pit = inpfoc_find(INPFOC_PAT, PAT_W);
  assert(pit);

  if (lpp.type == ADULT)
  {
    wstp = 1;
    wmin = 30;
    wmax = 180;
  }
  else if (lpp.type == CHILD)
  {
    wstp = (lpp.w >= 100) ? 5 : 1; // mux 10
    wmin = 40;
    wmax = 600;
  }
#if defined (NEONATE_FEATURE)
  else if (lpp.type == NEONATE)
  {
    wstp = 10;
    wmin = 100;
    wmax = 8000;
  }
#endif

  lpp.w += pcmd->delta * wstp;
  while (lpp.w > wmax) lpp.w -= (wmax - wmin);
  while (lpp.w < wmin) lpp.w += (wmax - wmin);

  pat_set(&lpp);

  if ( lpp.type == ADULT
#if defined (NEONATE_FEATURE)
        || lpp.type == NEONATE
#endif
     )
    snprintf(s, sizeof(s), "%d", lpp.w);
  else // CHILD
    snprintf(s, sizeof(s), "%.1f", (float) lpp.w / 10);

  inpfoc_wnd_setcaption(pit, s);
}

static void pat_exit_func(void * parg)
{
  pat_pars_t lpp;
  pat_cfg_t pat_cfg;

  memset(&lpp, 0, sizeof(pat_pars_t));
  pat_get(&lpp);

  // pat_cfg_save
  assert( sizeof(pat_pars_t) == sizeof(pat_cfg_t) );
  memcpy(&pat_cfg, &lpp, sizeof(pat_cfg_t));
  if ( stconf_write(STCONF_PATIENT, &pat_cfg, sizeof(pat_cfg_t)) > 0 );

  uframe_command(UFRAME_CHANGE, (void*)MENU_GENERAL);
}

void menu_patient_openproc(void)
{
  pat_pars_t lpp;
//  pat_cfg_t pat_cfg;
  char s[128];
  unsigned short ids;

#if 0
  memset(&lpp, 0, sizeof(pat_pars_t));
  if ( stconf_read(STCONF_PATIENT, &pat_cfg, sizeof(pat_cfg_t)) > 0 )
  {
    assert( sizeof(pat_cfg_t) == sizeof(pat_pars_t) );
    memcpy(&lpp, &pat_cfg, sizeof(pat_pars_t));
    pat_set(&lpp);
  }
  else
    pat_get(&lpp);
#endif

  pat_get(&lpp);

  ids2string(pat_cat_ids[lpp.type], s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PAT, PAT_CAT), s);
  snprintf(s, sizeof(s), "%d", lpp.bedno);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PAT, PAT_BED), s);
  snprintf(s, sizeof(s), "%d", lpp.cardno);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PAT, PAT_N), s);
  snprintf(s, sizeof(s), "%d", lpp.w);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PAT, PAT_W), s);
  snprintf(s, sizeof(s), "%d", lpp.h);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PAT, PAT_H), s);

  ids = IDS_WEIGHT_KG;
#if defined (NEONATE_FEATURE)
  if (lpp.type == NEONATE)
    ids = IDS_WEIGHT_G;
#endif
  ids2string(ids, s);
  uframe_clearbox(10, 172, 110, 20);
  uframe_printbox(10, 172, -1, -1, s, UFRAME_STATICTEXT_COLOR);

  inpfoc_set(INPFOC_PAT, PAT_EXIT);
}

