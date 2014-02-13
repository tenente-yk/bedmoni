#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

#include "bedmoni.h"
#include "dview.h"
#include "dio.h"
#include "grph.h"
#include "lframe.h"
#include "iframe.h"
#include "uframe.h"
#include "sframe.h"
#include "cframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "tblmenu.h"
#include "stconf.h"
#include "tr.h"

static lframe_t lframe;

static tbl_cs_t tbl_cs =
{
  {
  PARAM_TS,
  PARAM_HR,
  PARAM_SPO2,
  PARAM_NIBP,
  PARAM_BR,
  PARAM_T1,
  PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE,
  PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE,
  PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE,
  PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE,
  PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE, PARAM_NONE,
  }
};

static int par_cx[NUM_PARAMS][2] = 
{
 {0,0},       // PARAM_NONE
 {110,140},
 {130,140},
 {100,140},
 {100,140},
 {120,140},
 {100,140},
 {100,170},
 {100,170},
 {100,170},
 {100,140},
 {100,160},
 {100,160},
 {100,160},
 {100,160},
 {100,160},
 {100,160},
 {100,160},   // PARAM_BRC
 {100,160},   // PARAM_ETCO2
 {100,160},   // PARAM_ICO2
};

short tsm[NUM_TSM] = {1, 2, 5, 10, 15, 30, 60};
static int vscroll_range = 0;

static void lframe_on_create(void);
static void lframe_on_destroy(void);
//static void lframe_bitblt_fast(lframe_t * plframe, int x, int y, int cx, int cy);

static void scrollbar_load(lframe_t * plframe, int x, int y, int cx, int cy, scrollbar_info_t * psbi)
{
  rect_t rc;
  int v;

 // assert(plframe);

  rc.x0 = x;
  rc.x1 = x + cx;
  rc.y0 = y;
  rc.y1 = y + cy;

  grph_fillrect(plframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  grph_setfgcolor(plframe->bkdc, INPFOC_COLOR_DEF);
  grph_line(plframe->bkdc, x, y, x+cx, y);
  grph_line(plframe->bkdc, x+cx, y, x+cx, y+cy);
  grph_line(plframe->bkdc, x+cx, y+cy, x, y+cy);
  grph_line(plframe->bkdc, x, y+cy, x, y);

  if (psbi->range == 0)
  {
    rc.x0 = x+2;
    rc.y0 = y+2;
    rc.x1 = rc.x0+cx-2-2;
    rc.y1 = rc.y0+cy-2-2;
  }
  else
  {
    if (psbi->scroll_type == SCROLLBAR_TYPE_HORZ)
    {
      v = (cx / psbi->range);
      rc.x0 = x + psbi->pos*cx/psbi->range + 2;
      rc.y0 = y+2;
      rc.x1 = rc.x0 + v - 2 - 2;
      rc.y1 = y+cy-1;
    }
    if (psbi->scroll_type == SCROLLBAR_TYPE_VERT)
    {
      v = (cy / psbi->range);
      rc.x0 = x+2;
      rc.y0 = y + psbi->pos*cy/psbi->range + 2;
      rc.x1 = x+cx-1;
      if (v < 10) v = 10;
      if (rc.y0 + v - 2 > y + cy)
      {
        rc.y1 = y + cy - 1;
        rc.y0 = rc.y1 - v;
      }
      else
        rc.y1 = rc.y0 + v - 2 - 1;
    }
  }

  grph_fillrect(plframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, INPFOC_COLOR_DEF);
}

#if 0 // not used
static void lframe_progress_stepit(lframe_t * plframe)
{
  static unsigned short cnt;
  unsigned char b, bb;
  rect_t rc;

  assert(plframe);

  debug("%s now not implemented\n", __FUNCTION__);

  cnt ++;
  if (cnt & (8-1)) return;

  b = (cnt / 8) & 0xf;

  rc.x0 = 300;
  rc.x1 = rc.x0+100;
  rc.y0 = plframe->y+plframe->cy;
  rc.y1 = rc.y0+8;
  bb = (rc.x1 - rc.x0) / 16;

  grph_fillrect(plframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  grph_setfgcolor(plframe->updc, 0x006600);
  grph_line(plframe->updc, rc.x0-1, rc.y0-1, rc.x1+1, rc.y0-1);
  grph_line(plframe->updc, rc.x0-1, rc.y1+1, rc.x1+1, rc.y1+1);
  grph_line(plframe->updc, rc.x0-1, rc.y0-1, rc.x0-1, rc.y1+1);
  grph_line(plframe->updc, rc.x1+1, rc.y0-1, rc.x1+1, rc.y1+1);

  grph_fillrect(plframe->updc, rc.x0+b*bb, rc.y0, bb, rc.y1-rc.y0, 0x009900);

  lframe_bitblt_fast(plframe, rc.x0, rc.y0-1, rc.x1-rc.x0, rc.y1-rc.y0+1);
}
#endif

static void lframe_bitblt(lframe_t * plframe, int x, int y, int cx, int cy)
{
  dc_list_t dclist;

  assert(plframe);

  dclist.xres = ((PGDC)(plframe->bkdc))->xres;
  dclist.yres = ((PGDC)(plframe->bkdc))->yres;
  dclist.p_bk = ((PGDC)(plframe->bkdc))->addr;
  dclist.p_fg = ((PGDC)(plframe->fgdc))->addr;
  dclist.p_up = ((PGDC)(plframe->updc))->addr;

  if (plframe->x+x >= LFRAME_CX) return;

  if (x + cx >= plframe->cx) cx = plframe->cx-x;
  if (y + cy >= plframe->cy) cy = plframe->cy-y;

  dview_bitblt(plframe->x+x, plframe->y+y, cx, cy, &dclist, x, y);
}

static void frame_bitblt(int x, int y, int cx, int cy, void *parg)
{
  lframe_t * plframe = &lframe;
  assert(plframe);
  lframe_bitblt(plframe, x, y, cx, cy);
}

void lframe_ini_def(void)
{
  lframe_t * plframe = &lframe;
  int i;

  plframe->mode = LFRAME_MODE_BRIEF;
  plframe->voffs = plframe->hoffs = 0;
  plframe->tsm = TSM_1;

  // fill TABLE_MAX_NUM_IDS items
  for (i=0; i<TABLE_MAX_NUM_IDS; i++)
    tbl_cs.id[i] = PARAM_NONE;

  tbl_cs.id[0] = PARAM_TS;
  tbl_cs.id[1] = PARAM_HR;
  tbl_cs.id[2] = PARAM_SPO2;
  tbl_cs.id[3] = PARAM_NIBP;
  tbl_cs.id[4] = PARAM_BR;
  tbl_cs.id[5] = PARAM_T1;
}

void lframe_init(void)
{
  lframe_t * plframe = &lframe;

  memset(plframe, 0, sizeof(lframe_t));

  plframe->visible = 0;
  plframe->x  = LFRAME_X0;
  plframe->y  = LFRAME_Y0;
  plframe->cx = LFRAME_CX;
  plframe->cy = LFRAME_CY;//+ SFRAME_CY;

  memset(&plframe->table, 0, sizeof(table_t));

  plframe->bkdc = grph_createdc(rootwfd, 0, 0, plframe->cx, plframe->cy, 0);
  plframe->fgdc = grph_createdc(rootwfd, 0, 0, plframe->cx, plframe->cy, 0);
  plframe->updc = grph_createdc(rootwfd, 0, 0, plframe->cx, plframe->cy, 0);

  plframe->num_cmds_in_queue = 0;

#if 0
  for (i=0; i<TABLE_MAX_NUM_IDS; i++)
  {
    tbl_cs.id[i] = PARAM_NONE;
  }

  for (i=PARAM_FIRST; i<PARAM_MAX_NUM; i++)
  {
    tbl_cs.id[i] = i;
  }
#endif

  plframe->table.ptd = NULL;

  plframe->table.ptd = calloc(1, TABLE_MAX_NUM_IDS * TABLE_MAX_NUM_LINES * sizeof(table_data_t));
  assert(plframe->table.ptd);

  lframe_ini_def();

  lframe_cfg_t lframe_cfg;
  if ( stconf_read(STCONF_LFRAME, (char*)&lframe_cfg, sizeof(lframe_cfg_t)) > 0 )
  {
    plframe->visible = lframe_cfg.visible;
    plframe->mode = lframe_cfg.mode;
    plframe->tsm = lframe_cfg.tsm;
    tbl_cs_write(&lframe_cfg.tbl_cs);
  }

  if (plframe->visible)
  {
//     grph_fillrect(plframe->bkdc, 0, 0, plframe->cx, plframe->cy, 0x000000);
//     grph_fillrect(plframe->fgdc, 0, 0, plframe->cx, plframe->cy, 0x000000);
//     grph_fillrect(plframe->updc, 0, 0, plframe->cx, plframe->cy, 0x000000);

   // lframe_on_create();
    dview_frame_active(LFRAME);
  }
}

void lframe_deinit(void)
{
  lframe_t * plframe = &lframe;

  lframe_cfg_t lframe_cfg;

  lframe_cfg.visible = plframe->visible;
  lframe_cfg.mode = plframe->mode;
  lframe_cfg.tsm = plframe->tsm;
  tbl_cs_read(&lframe_cfg.tbl_cs);
  stconf_write(STCONF_LFRAME, (char*)&lframe_cfg, sizeof(lframe_cfg_t));

  if (plframe->table.ptd)
    free(plframe->table.ptd);

  inpfoc_rm(INPFOC_TABLE);

  grph_releasedc(plframe->bkdc);
  grph_releasedc(plframe->fgdc);
  grph_releasedc(plframe->updc);
}

static void lframe_on_destroy(void)
{
  lframe_t * plframe = &lframe;
  uframe_t * puframe = uframe_getptr();

  assert(puframe);

  iframe_command(IFRAME_SET_MODE, (void*)IFRAME_MODE_MONITOR);

  if (!plframe->visible) return;

  table_unload(&plframe->table);

  inpfoc_rm(INPFOC_TABLE);
  inpfoc_change(INPFOC_MAIN);
  puframe->inpfoc_prev = INPFOC_MAIN;

  plframe->visible = 0;

  lframe_cfg_save();
}

static void lframe_on_draw(void)
{
  lframe_t * plframe = &lframe;
  rect_t rc;
  int i;
  table_t * ptbl = &plframe->table;
  char s[4] = {158, 159}; // strelki vverh i vniz

  rc.x0 = 4+TBL_SCROLL_W+1;
  rc.y0 = 0;
  rc.x1 = rc.x0 + plframe->cx-TBL_SCROLL_W-5;
  rc.y1 = rc.y0 + LFRAME_CY-TBL_BUTTON_CY-TBL_SCROLL_W-1;

  grph_fillrect(plframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(plframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(plframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  // no need return focus, refreshed display and inpfoc are not overlapped
 // inpfoc_focus(INPFOC_TABLE);

  grph_setfont(plframe->fgdc, TABLE_FONT);
  grph_setfgcolor(plframe->fgdc, TABLE_FONTCOLOR);
  grph_setbkcolor(plframe->fgdc, LFRAME_BKCOLOR);
  table_data_t * ptr = ptbl->ptd;
  for (i=0; i<ptbl->len; i++)
  {
    rc.x0 = ptr->x + 4+TBL_SCROLL_W+1;
    rc.y0 = ptr->y;
    rc.x1 = rc.x0 + ptr->cx;
    rc.y1 = rc.y0 + ptr->cy;

    if (ptr->flags & TR_ST_U) strncat(ptr->s, s+0, 1);
    if (ptr->flags & TR_ST_L) strncat(ptr->s, s+1, 1);
    grph_drawtext(plframe->fgdc, ptr->s, -1, &rc, DT_CENTER);
    ptr ++;
  }

  if (demo_mode && plframe->visible)
  {
    rect_t rect;
    char s[200];
    grph_setfont(plframe->updc, ARIAL_60);
    grph_setfgcolor(plframe->updc, RGB(0xEE,0xEE,0xEE));
    rect.x0 = 0;
    rect.x1 = rect.x0 + plframe->cx;
    rect.y0 = plframe->cy / 2 - 50;
    ids2string(IDS_DEMO_LARGE, s);
    grph_drawtext(plframe->updc, s, -1, &rect, DT_CENTER);
  }

  lframe_bitblt(plframe, 0, 0, plframe->cx, plframe->cy);
}

static void lframe_on_reload(void)
{
  lframe_t * plframe = &lframe;
  tbl_cs_t l_tbl_cs;
  scrollbar_info_t scrollbar_info;
  int i;//, j, nr;
  trts_t trts1;//, trts1;
 // tr_t tr;
  rect_t rc;
 // dc_list_t dclist;
 // char s[200];

  if (!plframe->visible) return;

//printf("%s %d\n", __FUNCTION__, __LINE__);

  rc.x0 = 0;
  rc.x1 = rc.x0+plframe->cx;
  rc.y0 = 0;
  rc.y1 = rc.y0+plframe->cy-TBL_BUTTON_CY-1;

  grph_fillrect(plframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(plframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(plframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

#if 0
  dc_list_t dclist;
  rc.x0 = 0;
  rc.x1 = SFRAME_X0;
  rc.y0 = plframe->y+plframe->cy;
  rc.y1 = rc.y0 + SFRAME_CY;

  grph_fillrect(plframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(plframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(plframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  dclist.xres = ((PGDC)(plframe->bkdc))->xres;
  dclist.yres = ((PGDC)(plframe->bkdc))->yres;
  dclist.p_bk = (unsigned char*)((PGDC)(plframe->bkdc))->addr;
  dclist.p_fg = (unsigned char*)((PGDC)(plframe->fgdc))->addr;
  dclist.p_up = (unsigned char*)((PGDC)(plframe->updc))->addr;
  dview_bitblt(rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, &dclist, 0, plframe->cy);
#endif

/*  snprintf(s, sizeof(s), STR_SYNCHRONIZING);
  grph_setfgcolor(plframe->bkdc, RGB(0x00,0x99,0x00));
  grph_setfont(plframe->bkdc, ARIAL_14);
  rc.x0 = 0;
  rc.x1 = rc.x0+plframe->cx;
  rc.y0 = 100;
  rc.y1 = 100+20;
  grph_drawtext(plframe->bkdc, s, -1, &rc, DT_CENTER);
  lframe_bitblt_fast(plframe, 0, 0, plframe->cx, plframe->cy);
*/

  tbl_cs_read(&l_tbl_cs);
  table_load(plframe->mode, &l_tbl_cs, &plframe->table);
  tbl_cs_write(&l_tbl_cs);

  // clear progress bar
  // TODO: uncomment and test
/*
  rc.x0 = 300;
  rc.x1 = rc.x0+100;
  rc.y0 = plframe->y+plframe->cy;
  rc.y1 = rc.y0+10+3;
  grph_fillrect(plframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  lframe_bitblt_fast(plframe, rc.x0, rc.y0-1, rc.x1-rc.x0, rc.y1-rc.y0+1);
*/
  scrollbar_info.scroll_type = SCROLLBAR_TYPE_HORZ;
  scrollbar_info.range = plframe->table.nc;
  scrollbar_info.pos = plframe->hoffs;
  scrollbar_load(plframe, TBL_SCROLL_W+3, LFRAME_CY-TBL_BUTTON_CY-TBL_SCROLL_W-1, plframe->cx-TBL_SCROLL_W-5-2, TBL_SCROLL_W-2, &scrollbar_info);

  if (vscroll_range == 0)
  {
    trh_t ltrh;
    int rem;

    trts1.year = 2030 - 1900;
    trts1.mon  = 12;
    trts1.day  = 31;
    trts1.hour = 23;
    trts1.min  = 59;

    rem = 0;
    i = 0;
    i = trh_size();
    for (i=i-1; i>0; i--)
    {
      if (trh_read(i, &ltrh) <= 0)
        break;
      vscroll_range += ((ltrh.offs1 - ltrh.offs0) / sizeof(tr_t) + rem) / tsm[plframe->tsm];

      if ( ( (trts1.day*24*60+trts1.hour*60+trts1.min) - (ltrh.trts1.day*24*60+ltrh.trts1.hour*60+ltrh.trts1.min) ) > tsm[plframe->tsm] )
      {
        rem = 0;
      }

      rem = ((ltrh.offs1 - ltrh.offs0) / sizeof(tr_t) + rem) % tsm[plframe->tsm];

      memcpy(&trts1, &ltrh.trts0, sizeof(trts_t));
    }
  }

//printf("vscroll_range = %d\n", vscroll_range);

  scrollbar_info.range = vscroll_range;
  scrollbar_info.scroll_type = SCROLLBAR_TYPE_VERT;
  scrollbar_info.pos = plframe->voffs;
  scrollbar_load(plframe, 3, 2, TBL_SCROLL_W, LFRAME_CY-TBL_BUTTON_CY-TBL_SCROLL_W-1-2, &scrollbar_info);

  lframe_on_draw();
}

void lframe_on_resize(const int x, const int y, const int cx, const int cy)
{
  lframe_t * plframe = &lframe;
 /* inpfoc_item_t * pifi;*/

  plframe->x = x;
  plframe->y = y;
  plframe->cx = cx;
  plframe->cy = cy;

  if (plframe->cx <= 0)
    plframe->cx += LFRAME_CX;
  if (plframe->cy <= 0)
    plframe->cy += LFRAME_CY;
/*
  pifi = inpfoc_find(INPFOC_TABLE, TABLE_SET);
  if (pifi)
    inpfoc_wnd_move(pifi, plframe->cx-80-5-55-5, LFRAME_CY-TBL_BUTTON_CY-1, TRUE);
  pifi = inpfoc_find(INPFOC_TABLE, TABLE_EXIT);
  if (pifi)
    inpfoc_wnd_move(pifi, plframe->cx-55-5, LFRAME_CY-TBL_BUTTON_CY-1, TRUE);
*/
  if (!plframe->visible) return;
  lframe_on_reload();
}

static void lframe_on_hscroll(short delta)
{
  lframe_t * plframe = &lframe;

  plframe->hoffs += delta;
  if (plframe->hoffs < 0) plframe->hoffs = 0;
  if (plframe->hoffs > plframe->table.nc-1) plframe->hoffs = plframe->table.nc-1;

  lframe_on_reload();
}

static void lframe_on_vscroll(short delta)
{
  lframe_t * plframe = &lframe;

  plframe->voffs += delta;
  if (plframe->voffs > vscroll_range - 1) plframe->voffs = vscroll_range - 1;
  if (plframe->voffs < 0) plframe->voffs = 0;

  lframe_on_reload();
}

static void lframe_on_create(void)
{
  lframe_t * plframe;
  uframe_t * puframe;
  mframe_t * pmframe = mframe_getptr();
  inpfoc_wnd_dc_t ifwd;
  char s[200], s2[128];

  plframe = &lframe;
  assert(plframe);

  puframe = uframe_getptr();
  assert(puframe);

  if (pmframe->maximized)
  {
    int n, i;
    n = 0;
    for (i=0; i<NUM_VIEWS; i++)
    {
      if (unit_isenabled(i)) n++;
    }
    for (i=NUM_VIEWS-1; i>=0 && n>5; i--)
    {
      if (unit_isenabled(i))
      {
        int k;
        unit_ioctl(i, UNIT_CMD_HIDE);
        switch (i)
        {
          case ECG:
            for (k=0; k<NUM_ECG; k++)
              cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(ECG_1+k, 0));
          break;
          case SPO2:
         // dio_module_cmd(PD_ID_SPO2, (r) ? SBDP_RST_MOD : SBDP_OFF_MOD); // <- need to be reconfigured at startup in a reset case
            cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(SPO2, 0));
            break;
          case RESP:
            cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(RESP, 0));
            break;
          case CO2:
            dio_module_cmd(PD_ID_CO2, SBDP_OFF_MOD);
            cframe_command(CFRAME_CHAN_VISIBILITY, (void*)MAKELONG(CO2, 0));
            break;
        }
        n --;
      }
    }
    pmframe->maximized = 0;
  }

  mframe_on_resize(MFRAME_X0, MFRAME_Y0, MFRAME_CX, MFRAME_CY);

  plframe->x  = LFRAME_X0;
  plframe->y  = LFRAME_Y0;
  plframe->cx = LFRAME_CX;
  plframe->cy = LFRAME_CY;
  plframe->visible = 1;

  grph_fillrect(plframe->bkdc, 0, plframe->cy-TBL_BUTTON_CY-1, plframe->cx, TBL_BUTTON_CY, 0x000000);
  lframe_bitblt(plframe, 0, plframe->cy-TBL_BUTTON_CY-1, plframe->cx, TBL_BUTTON_CY);

 // grph_fillrect(plframe->bkdc, 0, plframe->cy, plframe->cx-SFRAME_CX, SFRAME_CY, 0x000000);
 // lframe_bitblt(plframe, 0, plframe->cy, plframe->cx-SFRAME_CX, SFRAME_CY);

  memset(&ifwd, 0, sizeof(inpfoc_wnd_dc_t));

  ifwd.bkdc = plframe->bkdc;
  ifwd.updc = plframe->updc;
  ifwd.fgcolor = 0x444444;
  ifwd.bkcolor = 0x020202;

  ids2string(IDS_mIN, s2);
  snprintf(s, sizeof(s), "%d %s", tsm[plframe->tsm], s2);
  inpfoc_add(INPFOC_TABLE, TABLE_TS, (void*) inpfoc_wnd_create("LIST", 0+1, LFRAME_CY-TBL_BUTTON_CY-1, 60-1, TBL_BUTTON_CY, s, 0, &ifwd, frame_bitblt, plframe));
#if 0
  snprintf(s, sizeof(s), "%s", "---");
  if (plframe->mode == LFRAME_MODE_BRIEF) ids2string(IDS_DETAILED, s);
  if (plframe->mode == LFRAME_MODE_FULL) ids2string(IDS_BRIEF, s);
  inpfoc_add(INPFOC_TABLE, TABLE_MODE, (void*) inpfoc_wnd_create_("PUSH", 65, LFRAME_CY-TBL_BUTTON_CY-1, 70, TBL_BUTTON_CY, s, &ifwd, frame_bitblt, plframe));
#endif
  memset(s, 0, sizeof(s));
 // *((unsigned short*)&s[0]) = MAKEWORD(0xd0, 0x83);
 // s[2] = s[3] = ' ';
 // *((unsigned short*)&s[4]) = MAKEWORD(0xd0, 0x84);
  s[0] = 0x86;
  s[1] = s[2] = ' ';
  s[3] = 0x87;
  inpfoc_add(INPFOC_TABLE, TABLE_VSCROLL, (void*) inpfoc_wnd_create("LIST", 65,
  LFRAME_CY-TBL_BUTTON_CY-1, 60, TBL_BUTTON_CY, s, 0, &ifwd, frame_bitblt, plframe));
  inpfoc_add(INPFOC_TABLE, TABLE_HSCROLL, (void*) inpfoc_wnd_create("LIST", 130, LFRAME_CY-TBL_BUTTON_CY-1, 60, TBL_BUTTON_CY, "<< >>", 0, &ifwd, frame_bitblt, plframe));
#if 0
  ids2string(IDS_PRINT, s);
  inpfoc_add(INPFOC_TABLE, TABLE_PRINT, (void*) inpfoc_wnd_create_("PUSH", 270, LFRAME_CY-TBL_BUTTON_CY-1, 80, TBL_BUTTON_CY, s, &ifwd, frame_bitblt, plframe));
  inpfoc_disable(INPFOC_TABLE, TABLE_PRINT);
#endif

  inpfoc_add(INPFOC_TABLE, TABLE_SET, (void*) inpfoc_wnd_create("PUSH", plframe->cx-60-5-102-5, LFRAME_CY-TBL_BUTTON_CY-1, 102, TBL_BUTTON_CY, 0, IDS_SETTINGS, &ifwd, frame_bitblt, plframe));
  inpfoc_add(INPFOC_TABLE, TABLE_EXIT, (void*) inpfoc_wnd_create("PUSH", plframe->cx-60-5, LFRAME_CY-TBL_BUTTON_CY-1, 60-1, TBL_BUTTON_CY, 0, IDS_QUIT, &ifwd, frame_bitblt, plframe));

  inpfoc_change(INPFOC_TABLE);
  puframe->inpfoc_prev = INPFOC_TABLE;

  plframe->voffs = plframe->hoffs = 0;

  iframe_command(IFRAME_SET_MODE, (void*)IFRAME_MODE_TABLE);

  tr_t tr0, tr1;
  int r;
  tr_seek(0, SEEK_SET);
  r = 1;
  r &= (tr_read(&tr0, 1, 0, 0) > 0);
  tr_seek(tr_get_size()-1, SEEK_SET);
  r &= (tr_read(&tr1, 1, 0, 0) > 0);
  if (r)
  {
    trts_t trts0, trts1;
    trts0 = *((trts_t*)&tr0.v[PARAM_TS]);
    trts1 = *((trts_t*)&tr1.v[PARAM_TS]);
    snprintf(s, sizeof(s), "%02d.%02d.%04d - %02d.%02d.%04d", trts0.day, trts0.mon, trts0.year+1900, trts1.day, trts1.mon, trts1.year+1900);
    iframe_command(IFRAME_SET_STRING, (void*)s);
  }

  lframe_on_reload();

  inpfoc_set(INPFOC_TABLE, TABLE_EXIT);

  lframe_cfg_save();
}

static void lframe_on_newdata(void)
{
  lframe_on_reload();

  // TODO: vscroll_range has to be updated
}

static void lframe_on_change_ts(int delta)
{
  lframe_t * plframe = lframe_getptr();
  char s[200], s2[200];

  plframe->tsm += delta;

  while (plframe->tsm >= NUM_TSM) plframe->tsm -= NUM_TSM;
  while (plframe->tsm < 0)        plframe->tsm += NUM_TSM;

  ids2string(IDS_mIN, s2);
  snprintf(s, sizeof(s), "%d %s", tsm[plframe->tsm], s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TABLE, TABLE_TS), s);

  vscroll_range = 0;
  lframe_on_reload();

  lframe_cfg_save();
}

void lframe_show(void)
{
  uframe_command(UFRAME_DESTROY, NULL);
  lframe_command(LFRAME_CREATE, NULL);
}

void lframe_hide(void)
{
  lframe_command(LFRAME_DESTROY, NULL);
}

void lframe_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case LFRAME_RESIZE:
      dview_command(LFRAME, cmd, 1, arg);
      break;
    default:
      dview_command(LFRAME, cmd, 0, arg);
      break;
  }
}

void lframe_on_command(int cmd, void * arg)
{
    switch (cmd)
    {
      case LFRAME_CREATE:
        lframe_on_create();
        break;
      case LFRAME_DESTROY:
        lframe_on_destroy();
        break;
      case LFRAME_HSCROLL:
        lframe_on_hscroll(*((int*)arg));
        break;
      case LFRAME_VSCROLL:
        lframe_on_vscroll(*((int*)arg));
        break;
      case LFRAME_RESIZE:
        error("%s: line %d - add implementation\n", __FILE__, __LINE__);
        //lframe_on_resize(...);
        break;
      case LFRAME_RELOAD:
        lframe_on_reload();
        break;
      case LFRAME_NEWDATA:
        lframe_on_newdata();
        break;
      case LFRAME_CHANGE_TS:
        lframe_on_change_ts(*((int*)arg));
        break;
      default:
        error("%s: unknown cmd %d\n", __FUNCTION__, cmd);
        break;
    }
}

void table_load(int mode, tbl_cs_t * ptbl_cs, table_t * ptbl)
{
  int i, j, j0, k, len;
  tbl_par_t tp;
  lframe_t * plframe = &lframe;
  table_data_t * ptr = NULL;
  tr_t tr;
  int w = 0;
  int h = 0, hm = 0;
  int nr;
  static trts_t trts0, trts1;
  char s1[200], s2[200];
 // int v;

//printf("%s+\n", __FUNCTION__);

  assert(plframe);

  if (!ptbl) return;
  if (!ptbl_cs) return;

  table_unload(ptbl);

  ptbl->nl = 0;
  ptbl->nc = 0;
  ptbl->len = 0;

  for (i=0; i<TABLE_MAX_NUM_IDS; i++)
  {
    if (ptbl_cs->id[i] != PARAM_NONE)
    {
      ptbl->nc ++;
    }
  }

  // calc ptbl->nl
  ptbl->nl = tr_get_size();
  debug("time quants in a table: %d\n", ptbl->nl);

  // calc hor offset (j0)
  j0 = 0;
  for (i=0; i<TABLE_MAX_NUM_IDS; i++)
  {
    if (ptbl_cs->id[i] != PARAM_NONE)
    {
      if (j0 == plframe->hoffs) break;
      j0++;
    }
  }

  w = 0;
  h = 0;
  hm = 0;
  len = 0;
  k = 0;

  // store in ptbl->ptd header of table
  ptr = (table_data_t*) ((unsigned char*) (ptbl->ptd) + len * sizeof(table_data_t));
  for (j=j0; j<TABLE_MAX_NUM_IDS; j++)
  {
    if (j > ptbl->nc) break;
    if (ptbl_cs->id[j] != PARAM_NONE)
    {
      id2tblpar(ptbl_cs->id[j], &tp);
      if (tp.param != PARAM_NONE)
      {
        ptr->flags = 0;
        memset(ptr->sw, 0, sizeof(((table_data_t*)(0))->sw));
        ids2string(tp.ids_name, s1);
        ids2string(tp.ids_dim, s2);
        snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%s, %s", s1, s2);
        ptr->x = TBL_SCROLL_W+w;
        ptr->y = h;
        if (mode == LFRAME_MODE_FULL)
        {
          ptr->cx = par_cx[tp.param][1];
        }
        else if (mode == LFRAME_MODE_BRIEF)
        {
          ptr->cx = par_cx[tp.param][0];
        }
         // ptr->cx = 140;
        ptr->cy = 20;
        w += ptr->cx;
        hm = max(ptr->cy, hm);
        if (4+TBL_SCROLL_W+1+w >= plframe->x+plframe->cx) break;
        if (h+hm >= (LFRAME_CY-TBL_BUTTON_CY-TBL_SCROLL_W-1)) break;
        k ++;
        ptr ++;
      }
    }
  }
  len += k;
  h += hm;

  w = 0;
  trts1.year = 2099 - 1900;
  trts1.mon  = 12;
  trts1.day  = 31;
  trts1.hour = 23;
  trts1.min  = 59;
#if 0
  // calc initial i
  for (i=0, j=0; i<plframe->voffs; i++)
  {
    tr_seek(0, SEEK_END);
    tr_seek(-1-j, SEEK_CUR);
    nr = tr_read(&tr, 1, 0);
    trts1 = *((trts_t*)&tr.v[PARAM_TS]);

    lframe_progress_stepit(plframe);

    while ( (((trts0.day*24*60+trts0.hour*60+trts0.min) - (trts1.day*24*60+trts1.hour*60+trts1.min)) < tsm[plframe->tsm]) && nr )
    {
      j ++;
      tr_seek(0, SEEK_END);
      tr_seek(-1-j, SEEK_CUR);
      nr = tr_read(&tr, 1, 0);
      trts1 = *((trts_t*)&tr.v[PARAM_TS]);

      lframe_progress_stepit(plframe);

    }
    memcpy(&trts0, &trts1, sizeof(trts_t));
  }
#endif

  int ntrh;
  trh_t ltrh;
  int offs, offs0;
  int rem;
  int i0;

  ntrh = trh_size();
 // printf("ntrh= %d\n", ntrh);

  offs = 0;
  rem = 0;
  i0 = 0;
  for (i=ntrh-1; i>0; i--)
  {
    trh_read(i, &ltrh);
//printf("%02d.%02d - %02d.%02d (%d - %d)\n", ltrh.trts0.hour, ltrh.trts0.min, ltrh.trts1.hour, ltrh.trts1.min, ltrh.offs0, ltrh.offs1);
    if (ltrh.offs1 == ltrh.offs0) continue;
    if ( (trts1.day*24*60+trts1.hour*60+trts1.min) - (ltrh.trts1.day*24*60+ltrh.trts1.hour*60+ltrh.trts1.min) > tsm[plframe->tsm] )
    {
      rem = 0;
    }
   // offs0 = ( (ltrh.trts1.day*24*60+ltrh.trts1.hour*60+ltrh.trts1.min) - (ltrh.trts0.day*24*60+ltrh.trts0.hour*60+ltrh.trts0.min) + rem ) / tsm[plframe->tsm];
    offs0 = ( (ltrh.offs1 - ltrh.offs0)/sizeof(tr_t) + rem ) / tsm[plframe->tsm];
    if (offs + offs0 >= plframe->voffs)
    {
//printf("%d %d %d\n", plframe->voffs,offs,offs0);
      i0 += (plframe->voffs-offs);
//printf("i00=%d\n", i0);
      break;
    }
    offs += offs0;
    i0 += (ltrh.offs1 - ltrh.offs0) / sizeof(tr_t);
   // rem = ( (ltrh.trts1.day*24*60+ltrh.trts1.hour*60+ltrh.trts1.min) - (ltrh.trts0.day*24*60+ltrh.trts0.hour*60+ltrh.trts0.min) + rem ) % tsm[plframe->tsm];
    rem = ( (ltrh.offs1 - ltrh.offs0)/sizeof(tr_t) + rem ) % tsm[plframe->tsm];
    memcpy(&trts1, &ltrh.trts0, sizeof(trts_t));
  }

//printf("i0=%d\n", i0);

  for (i=i0; ;/*i++*/)
  {
    if (h >= (plframe->cy-TBL_BUTTON_CY-TBL_SCROLL_W-1)) break;
    if (i == ptbl->nl) break;

    ptr = (table_data_t*) ((unsigned char*) (ptbl->ptd) + len * sizeof(table_data_t));

    k = 0;
    w = 0;
    hm = 0;

#if 0
     tr_seek(0, SEEK_END);
     tr_seek(-1-i*tsm[plframe->tsm], SEEK_CUR);
     nr = tr_read(&tr, 1, 0);
#else
    tr_seek(0, SEEK_END);
    tr_seek(-(i+1), SEEK_CUR);
    nr = tr_read(&tr, 1, 0, 0);
    if (nr == 0) break;
#endif
//printf("i=%d, nr=%d\n", i, nr);
    for (j=j0; TABLE_MAX_NUM_IDS; j++)
    {
      if (j > ptbl->nc) break;
      if (ptbl_cs->id[j] != PARAM_NONE)
      {
//sprintf(ptr->s, "");
        id2tblpar(ptbl_cs->id[j], &tp);
        if (tp.param != PARAM_NONE)
        {
          ptr->flags = 0;
          memset(ptr->sw, 0, sizeof(((table_data_t*)(0))->sw));
          if (tr.v[tp.param] == TR_UNDEF_VALUE)
          {
            if (mode == LFRAME_MODE_FULL)
            {
              snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "- - - (-)");
            }
            else if (mode == LFRAME_MODE_BRIEF)
            {
              snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "-");
            }
          }
          else
          {
            switch (tp.param)
            {
              case PARAM_TS:
                snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%02d.%02d %02d:%02d", ((trts_t*)(&tr.v[tp.param]))->day, ((trts_t*)(&tr.v[tp.param]))->mon, ((trts_t*)(&tr.v[tp.param]))->hour, ((trts_t*)(&tr.v[tp.param]))->min);
                break;
              case PARAM_NIBP:
                snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%d/%d(%d)", ((nibpv_t*)(&tr.v[tp.param]))->ss, ((nibpv_t*)(&tr.v[tp.param]))->dd, ((nibpv_t*)(&tr.v[tp.param]))->mm);
                break;
              case PARAM_T1:
              case PARAM_T2:
              case PARAM_DT:
                if (mode == LFRAME_MODE_FULL)
                {
                  snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%.1f - %.1f (%.1f)", (float)tr.vl[tp.param]/10, (float)tr.vu[tp.param]/10, (float)tr.v[tp.param]/10);
                }
                else if (mode == LFRAME_MODE_BRIEF)
                {
                  snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%.1f", (float)tr.v[tp.param]/10);
                }
                break;
              case PARAM_STI:
              case PARAM_STII:
              case PARAM_STIII:
              case PARAM_STAVR:
              case PARAM_STAVL:
              case PARAM_STAVF:
              case PARAM_STV:
                if (mode == LFRAME_MODE_FULL)
                {
                  snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%.2f - %.2f (%.2f)", (float)tr.vl[tp.param]/100, (float)tr.vu[tp.param]/100, (float)tr.v[tp.param]/100);
                }
                else if (mode == LFRAME_MODE_BRIEF)
                {
                  snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%.2f", (float)tr.v[tp.param]/100);
                }
                break;
              default:
                if (mode == LFRAME_MODE_FULL)
                {
               // snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "F");
                  snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%d - %d (%d)", tr.vl[tp.param], tr.vu[tp.param], tr.v[tp.param]);
                }
                else if (mode == LFRAME_MODE_BRIEF)
                {
                  snprintf(ptr->s, sizeof(((table_data_t*)(0))->s), "%d", tr.v[tp.param]);
                }
                break;
            } // switch
            if (tr.st[tp.param] & TR_ST_U)
            {
              ptr->sw[0] = 0x2191;
              ptr->flags |= TR_ST_U;
            }
            if (tr.st[tp.param] & TR_ST_L)
            {
              if (ptr->sw[0] == 0)
                ptr->sw[0] = 0x2193;
              else
                ptr->sw[1] = 0x2193;
              ptr->flags |= TR_ST_L;
            }
          }
          ptr->x = TBL_SCROLL_W+w;
          ptr->y = h;
          if (mode == LFRAME_MODE_FULL)
          {
            ptr->cx = par_cx[tp.param][1];
          }
          else if (mode == LFRAME_MODE_BRIEF)
          {
            ptr->cx = par_cx[tp.param][0];
          }
         // ptr->cx = 140;
          ptr->cy = 20;
          w += ptr->cx;
          hm = max(ptr->cy, hm);
          if (4+TBL_SCROLL_W+1+w >= plframe->x+plframe->cx-5) break;
          if (h+hm >= (LFRAME_CY-TBL_BUTTON_CY-TBL_SCROLL_W-1)) break;
          k ++;
          ptr ++;
        }
      }
    }
    len += k;
    h += hm;

    trts1 = *((trts_t*)&tr.v[PARAM_TS]);
    trts0 = trts1;

   // lframe_progress_stepit(plframe);
    // skip (tsm-1) lines
    while ( (((trts1.day*24*60+trts1.hour*60+trts1.min) - (trts0.day*24*60+trts0.hour*60+trts0.min)) < tsm[plframe->tsm]) && nr>0 )
    {
      i ++;
      tr_seek(0, SEEK_END);
      if ( tr_seek(-(i+1), SEEK_CUR) < 0)
      {
       // printf("n==0 -> %d\n", -1-i);
        nr = 0;
        break;
      }
      nr = tr_read(&tr, 1, 0, 0);
      trts0 = *((trts_t*)&tr.v[PARAM_TS]);

      if ( ((trts1.day*24*60+trts1.hour*60+trts1.min) - (trts0.day*24*60+trts0.hour*60+trts0.min)) < 0)
        break;
     // lframe_progress_stepit(plframe);
    }
    if (nr == 0) break;
   // memcpy(&trts1, &trts0, sizeof(trts_t));
  }

//printf("%s-\n", __FUNCTION__);

  ptbl->len = len;
}

void table_unload(table_t * ptbl)
{
  if (!ptbl) return;
  if (!ptbl->ptd) return;

  ptbl->nc = ptbl->nl = ptbl->len = 0;
}

void tbl_cs_read(tbl_cs_t * ptcs)
{
  assert(ptcs);
  *ptcs = tbl_cs;
}

void tbl_cs_write(tbl_cs_t * ptcs)
{
  assert(ptcs);
  tbl_cs = *ptcs;
}

void id2tblpar(int id, tbl_par_t * ptp)
{
  int j;

  assert(ptp);

  for (j=NUM_PARAMS-1; j>PARAM_NONE; j--)
  {
    if (tbl_par[j].param == id) break;
  }
  *ptp = *(&tbl_par[j]);
}

void lframe_cfg_save(void)
{
  lframe_t * plframe = &lframe;
  lframe_cfg_t lframe_cfg;

  lframe_cfg.visible = plframe->visible;
  lframe_cfg.mode = plframe->mode;
  lframe_cfg.tsm = plframe->tsm;
  tbl_cs_read(&lframe_cfg.tbl_cs);
  stconf_write(STCONF_LFRAME, (char*)&lframe_cfg, sizeof(lframe_cfg_t));
}

lframe_t * lframe_getptr(void)
{
  return &lframe;
}

