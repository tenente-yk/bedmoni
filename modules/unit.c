/*! \file unit.c
    \brief Channels units
 */

#ifdef UNIX
#include <unistd.h>
#include <pthread.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include "bedmoni.h"
#include "grph.h"
#include "mframe.h"
#include "sframe.h"
#include "stconf.h"
#include "unit.h"
#include "dview.h"
#include "mframe.h"
#include "t1t2.h"
#include "nibp.h"
#include "ep.h"
#include "ecgset.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"

#include "bell.cc"
#include "bell_inv.cc"
#include "nobell.cc"
#include "nobell_inv.cc"
#include "heart.cc"
#include "heart_inv.cc"
#include "blank.cc"
#include "pm.cc"

#define NORM(color) color
#define DARK(color) \
((unsigned char)((unsigned char)(color >> 16) * 0.8) << 16) | ((unsigned char)((unsigned char)(color >> 8) * 0.8) << 8) | ((unsigned char)((unsigned char)(color >> 0) * 0.8) << 0)

static unit_t m_unit[NUM_VIEWS];
mframe_t * pmframe = NULL;

static spo2_data_t spo2_data;
static nibp_data_t nibp_data;
static t1t2_data_t t1t2_data;
static ecg_data_t  ecg_data;
static resp_data_t resp_data;
static co2_data_t  co2_data;

static void unit_resize(int chno, int x, int y, int cx, int cy, UINT fuFlags);
static void unit_on_redraw(int chno);

static void unit_draw_int_data(int chno, item_t * pitem, int redraw);
static void unit_draw_float_data(int chno, item_t * pitem, int redraw);
static void unit_draw_text_data(int chno, item_t * pitem, int redraw);
static void unit_draw_progress_data(int chno, item_t * pitem, int redraw);
static void unit_draw_icon_data(int chno, item_t * pitem, int redraw);
static void unit_draw_label_data(int chno, item_t * pitem, int redraw);
static void unit_draw_no_data(int chno, item_t * pitem, int redraw);

static void frame_bitblt(int x, int y, int cx, int cy, void *parg);

static void unit_add_alarm(int chno, int alarm)
{
  int i;
  unit_t * punit;

  assert (chno < NUM_VIEWS);
  punit = pmframe->punit[chno];
  assert(punit);

  for (i=0; i<UNIT_MAX_NUM_ALARMS; i++)
  {
    if (punit->alarms[i] == alarm) return; // already in list
    if (punit->alarms[i] == NO_ALARMS) break;
  }
  if (i == UNIT_MAX_NUM_ALARMS)
  {
    error("%s: alr overflow\n", __FUNCTION__);
    return;
  }

  punit->alarms[i] = alarm;

  punit->alarmed = 1;

//  mframe_command(MFRAME_UNIT_ALARM, (void*)chno);
}

static void unit_del_alarm(int chno, int alarm)
{
  int i;
  unit_t * punit;

  assert (chno < NUM_VIEWS);
  punit = pmframe->punit[chno];
  assert(punit);

  //debug("%s %d\n", __FUNCTION__, alarm);

  for (i=0; i<UNIT_MAX_NUM_ALARMS; i++)
  {
    if (punit->alarms[i] == alarm) break;
  }
  if (i == UNIT_MAX_NUM_ALARMS)
  {
    // not found
    return;
  }

//printf("%s\n", __FUNCTION__);

  memmove(&punit->alarms[i], &punit->alarms[i+1], UNIT_MAX_NUM_ALARMS-i-1);

  if (i == 0)
  {
    punit->alarmed = 1; // no more unit's alarms, clear background
  }
//  if (punit->alarmed)
//  mframe_command(MFRAME_UNIT_ALARM, (void*)chno);
}

void unit_init(mframe_t * pmframe00)
{
 // WNDCLASS wndclass;
  int i;

  assert(pmframe00);

  pmframe = pmframe00;

  for (i=0; i<NUM_VIEWS; i++)
  {
    pmframe->punit[i] = &m_unit[i];
    pmframe->punit[i]->mx = 1.0f;    // filled after units' enumeration
    pmframe->punit[i]->my = 1.0f;    // filled after units' enumeration
  }

  memset(&nibp_data, 0, sizeof(nibp_data));

  pmframe->punit[ECG]->data  = &ecg_data;
  pmframe->punit[SPO2]->data = &spo2_data;
  pmframe->punit[NIBP]->data = &nibp_data;
  pmframe->punit[RESP]->data = &resp_data;
  pmframe->punit[T1T2]->data = &t1t2_data;
  pmframe->punit[CO2]->data  = &co2_data;

  unit_ini_def(TRUE);
}

void unit_deinit(void)
{
}

void unit_align_all(void)
{
  int i, nvis = 0, k, h, w;
  rect_t rc;

  assert(pmframe);

  rc.x0 = 0;
  rc.x1 = pmframe->cx;
  rc.y0 = 0;
  rc.y1 = pmframe->cy;

  grph_fillrect(pmframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, MFRAME_BKCOLOR);
  grph_fillrect(pmframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(pmframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  grph_setfgcolor(pmframe->bkdc, RGB(0x44,0x44,0x44));
  grph_line(pmframe->bkdc, 0, 0, 0, pmframe->cy);
  grph_line(pmframe->bkdc, 0, pmframe->cy-1, pmframe->cx, pmframe->cy-1);

  frame_bitblt(0, 0, pmframe->cx, pmframe->cy, NULL);

  for (i=0; i<NUM_VIEWS; i++)
  {
    if (pmframe->punit[i]->visible) nvis++;
  }

//debug("%s: nvis = %d\n", __FUNCTION__, nvis);

  k = 0;
  if (pmframe->cx > MFRAME_CX+2) //maximized)
  {
    dc_list_t dclist;

    // clear section near sframe section (cause may be filled by cframe before this)
    rc.x0 = 0;
    rc.x1 = SFRAME_X0;
    rc.y0 = pmframe->y+pmframe->cy;
    rc.y1 = rc.y0 + SFRAME_CY;

    grph_fillrect(pmframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, MFRAME_BKCOLOR);
    grph_fillrect(pmframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
    grph_fillrect(pmframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

    dclist.xres = ((PGDC)(pmframe->bkdc))->xres;
    dclist.yres = ((PGDC)(pmframe->bkdc))->yres;
    dclist.p_bk = (unsigned char*)((PGDC)(pmframe->bkdc))->addr;
    dclist.p_fg = (unsigned char*)((PGDC)(pmframe->fgdc))->addr;
    dclist.p_up = (unsigned char*)((PGDC)(pmframe->updc))->addr;

    dview_bitblt(rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, &dclist, 0, pmframe->cy);

    if (nvis <= 2)
    {
      for (i=0; i<NUM_VIEWS; i++)
      {
        if (pmframe->punit[i]->visible)
        {
          w = pmframe->cx/2;
          h = (pmframe->cy/2);
          unit_resize(i, 0, k*h, w-1, h, 0);
          k ++;
        }
      }
    }
    else if (nvis <= 4)
    {
      for (i=0; i<NUM_VIEWS; i++)
      {
        if (pmframe->punit[i]->visible)
        {
          w = pmframe->cx/2;
          h = pmframe->cy/2;
          unit_resize(i, (k%2)*w, (k/2)*h, w-1, h, 0);
          k ++;
        }
      }
    }
    else if (nvis <= 6)
    {
      for (i=0; i<NUM_VIEWS; i++)
      {
        if (pmframe->punit[i]->visible)
        {
          w = pmframe->cx/2;
          h = pmframe->cy/3;
          unit_resize(i, (k%2)*w, (k/2)*h, w-1, h, 0);
          k ++;
        }
      }
    }
    else
    {
      for (i=0; i<NUM_VIEWS; i++)
      {
        if (pmframe->punit[i]->visible)
        {
          h = pmframe->cy / nvis;
          unit_resize(i, 0, k*h, pmframe->cx, h, 0);
          k ++;
        }
      }
    }
  }
  else // normalized frame
  {
    for (i=0; i<NUM_VIEWS; i++)
    {
      if (pmframe->punit[i]->visible)
      {
        if(nvis <= 5)
          h = UNIT_DEF_HEIGHT;
        else
          h = pmframe->cy / nvis;
        unit_resize(i, 0, k*h, pmframe->cx, h, 0);
        k ++;
      }
    }
  }

  inpfoc_focus(INPFOC_MAIN);

  if (demo_mode && pmframe->maximized)
  {
    rect_t rect;
    char s[200];
    grph_setfont(pmframe->updc, ARIAL_60);
    grph_setfgcolor(pmframe->updc, RGB(0xEE,0xEE,0xEE));
    rect.x0 = 0;
    rect.x1 = rect.x0 + pmframe->cx;
    rect.y0 = pmframe->cy / 2 - 50;
    ids2string(IDS_DEMO_LARGE, s);
    grph_drawtext(pmframe->updc, s, -1, &rect, DT_CENTER);

    frame_bitblt(0, 0, pmframe->cx, pmframe->cy, NULL);
  }

}

int  unit_isenabled(int chno)
{
  assert (pmframe);

  if (chno >= NUM_VIEWS) return -1;

  return pmframe->punit[chno]->visible;
}

void unit_on_show(int chno, int nCmdShow)
{
  int i, iafter = -1;
  inpfoc_wnd_dc_t ifwd;
  unit_t * punit;
  rect_t rc;

  assert (chno < NUM_VIEWS);
  assert(pmframe);
  punit = pmframe->punit[chno];

 // debug("%s %d\n", __FUNCTION__, nCmdShow);

  if ((BOOL)punit->visible == (BOOL)(nCmdShow ? 1 : 0)) return;

  for (i=0; i<chno; i++)
  {
    if (pmframe->punit[i]->visible) iafter = i;
  }

  punit->visible = (nCmdShow) ? 1 : 0;

 // debug("%s ch %d %d\n", __FUNCTION__, i, punit->visible);

  ifwd.bkdc = pmframe->bkdc;
  ifwd.updc = pmframe->updc;
  ifwd.fgcolor = punit->item[0].fgcolor;
  ifwd.bkcolor = punit->item[0].bkcolor;

  if (punit->visible)
  {
    rc.x0 = punit->x + punit->item[0].x;
    rc.y0 = punit->y + punit->item[0].y;
    grph_setfont(pmframe->fgdc, LABEL_FONT);
    grph_drawtext(pmframe->fgdc, punit->item[0].s, -1, &rc, DT_CALCRECT);
    punit->item[0].cx = 34; // rc.x1-rc.x0 + 2;
    punit->item[0].cy = rc.y1-rc.y0 + 2;
    if (iafter < 0)
      inpfoc_add(INPFOC_MAIN, punit->inpfoc_item, (void*) inpfoc_wnd_create("PUSH", punit->x+punit->item[0].x, punit->y+punit->item[0].y, punit->item[0].cx, punit->item[0].cy, 0, punit->item[0].ids, &ifwd, frame_bitblt, pmframe));
    else
      inpfoc_ins(INPFOC_MAIN, punit->inpfoc_item, (void*) inpfoc_wnd_create("PUSH", punit->x+punit->item[0].x, punit->y+punit->item[0].y, punit->item[0].cx, punit->item[0].cy, 0, punit->item[0].ids, &ifwd, frame_bitblt, pmframe), pmframe->punit[iafter]->inpfoc_item);
  }
  else
  {
    inpfoc_del(INPFOC_MAIN, punit->inpfoc_item);
  }
 // mframe_bitblt(pmframe, punit->x, punit->y, punit->cx, punit->cy);
}

static void unit_set_bckg(int chno, COLORREF color)
{
  int i;
  unit_t * punit;

  assert(pmframe);
  assert(chno < NUM_VIEWS);

  punit = pmframe->punit[chno];

  punit->bkcolor = color;

  if (!pmframe->visible || !punit->visible) return;

  for (i=0; i<UNIT_MAX_NUM_ITEMS; i++)
  {
    if (punit->item[i].type.val == UNIT_TYPE_VAL_UNDEFINED) continue;
    punit->item[i].bkcolor = color;
    inpfoc_wnd_setbkcolor(inpfoc_find(INPFOC_MAIN, punit->inpfoc_item), color);
  }

#if 0
  grph_fillrect(pmframe->bkdc, punit->x+1, punit->y+1, punit->cx-1-1, punit->cy-1-1, color);
  frame_bitblt(punit->x, punit->y, punit->cx, punit->cy, NULL);
#else
  unit_on_redraw(chno);
#endif
}

void unit_alarm_stepit(void)
{
  unit_t * punit;
  COLORREF color;
  static int curr_color[NUM_VIEWS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // undef color
  static unsigned char toggler = 0;
  int chno;

  assert (pmframe);

  toggler ++;

  for (chno=0; chno<NUM_VIEWS; chno++)
  {

    punit = pmframe->punit[chno];
  //  if (!punit->visible) return;

    if (punit->alarmed)
    {
    // acquire the level of risk
      int i;
      int level = 0;
      for (i=0; i<UNIT_MAX_NUM_ALARMS; i++)
      {
        if (punit->alarms[i] == NO_ALARMS) break;
        if (alarm_msg[punit->alarms[i]].level > level) level = alarm_msg[punit->alarms[i]].level;
      }
      if (level > LOW_RISK)
      {
#if 1
        color = (toggler & 0x1) ? risk_color[level] : risk_color[NO_RISK];
        for (i=0; i<punit->nitems; i++)
        {
          if (punit->item[i].alarmed)
          {
            //printf("alarmed %d %d\n", chno, i);
            item_t * pitem = &punit->item[i];
            punit->item[i].bkcolor = color;
            pitem->upd_func(chno, pitem, TRUE);
          }
        }
#else
        color = (toggler & 0x1) ? risk_color[level] : risk_color[NO_RISK];
        unit_set_bckg(chno, color);
#endif
      }
      else
      {
        color = risk_color[level];
        if ((color & 0xFFFFFF) != (curr_color[chno] & 0xFFFFFF))
          unit_set_bckg(chno, color);
      }
    }
    else
    {
      color = risk_color[NO_RISK];
      if ((curr_color[chno] & 0xFFFFFF) != (color & 0xFFFFFF))
        unit_set_bckg(chno, color);
    }
    curr_color[chno] = color;
  }
}

void unit_on_change(int chno, int item_id)
{
  unit_t * punit;
  item_t * pitem;
  int i;

  assert (pmframe);
  if (!pmframe->visible) return;

  assert(chno < NUM_VIEWS);
  punit = pmframe->punit[chno];
  if (!punit->visible) return;

  for (i=0; i<UNIT_MAX_NUM_ITEMS; i++)
  {
    if (punit->item[i].id == item_id) break;
  }

  pitem = &punit->item[i];
  pitem->upd_func(chno, pitem, TRUE);
}

static void unit_on_redraw(int chno)
{
  unit_t * punit;
  int i;
  rect_t rc;

  assert (pmframe);

  if (chno >= NUM_VIEWS)
  {
    error("%s: invalid chno (%d)\n", __FUNCTION__, chno);
    return;
  }

 // printf(__FUNCTION__);

  punit = pmframe->punit[chno];

  rc.x0 = punit->x;
  rc.y0 = punit->y;
  rc.x1 = punit->x+punit->cx;
  rc.y1 = punit->y+punit->cy;

  grph_fillrect(pmframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, punit->bkcolor);
  grph_fillrect(pmframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  grph_setfgcolor(pmframe->bkdc, RGB(0x44,0x44,0x44));
  grph_line(pmframe->bkdc, punit->x, punit->y, punit->x+punit->cx-1, punit->y);
  grph_line(pmframe->bkdc, punit->x, punit->y, punit->x, punit->y+punit->cy-1);
  grph_line(pmframe->bkdc, punit->x, punit->y+punit->cy-1, punit->x+punit->cx-1, punit->y+punit->cy-1);
  grph_line(pmframe->bkdc, punit->x+punit->cx-1, punit->y, punit->x+punit->cx-1, punit->y+punit->cy-1);
//printf("%s %d: %d %d %d %d %d\n", __FUNCTION__, chno, punit->x, punit->y, punit->cx, punit->cy, punit->y+punit->cy);

//  grph_setfgcolor(pmframe->bkdc, RGB(0x77,0x77,0x77));
//  grph_line(pmframe->bkdc, punit->x, punit->y, punit->x+punit->cx-1, punit->y);

  for (i=0; i<UNIT_MAX_NUM_ITEMS; i++)
  {
    if (punit->item[i].type.val == UNIT_TYPE_VAL_UNDEFINED) continue;
    if (punit->item[i].visible == 0) continue;
    // prevent filling new values in draw_text_data func
    punit->item[i].rc0.x1 = punit->item[i].rc0.x0;
    punit->item[i].rc0.y1 = punit->item[i].rc0.y0;
    punit->item[i].upd_func(chno, &punit->item[i], FALSE);
  }

  frame_bitblt(punit->x, punit->y, punit->cx, punit->cy, NULL);
}

static void unit_resize(int chno, int x, int y, int cx, int cy, UINT fuFlags)
{
  int i;
  int fontsize;
  unit_t * punit;

//printf("%s+\n", __FUNCTION__);

  assert (chno < NUM_VIEWS);
  assert(pmframe);
  punit = pmframe->punit[chno];

  if ((UINT)(fuFlags & SWP_NOMOVE) == 0)
  {
    punit->x = x;
    punit->y = y;
  }
  if ((UINT)(fuFlags & SWP_NOSIZE) == 0)
  {
    punit->cx = cx;
    punit->cy = cy;
  }

  punit->mx = (float) punit->cx / MFRAME_CX;
  punit->my = (float) punit->cy / UNIT_DEF_HEIGHT;
 // punit->mx = 1.64;
 // punit->my = 2.0;

  for (i=0; i<punit->nitems; i++)
  {
//printf("%d %d %s %d %d\n", chno, punit->item[i].type, punit->item[i].s, punit->item[i].fontsize, punit->item[i].v);
    switch(punit->item[i].type.val)
    {
      case UNIT_TYPE_VAL_PROGRESS:
        break;
      case UNIT_TYPE_VAL_TEXT:
      case UNIT_TYPE_VAL_INT:
      case UNIT_TYPE_VAL_FLOAT:
//printf("%d %s %d\n", chno, punit->item[punit->item[i].id].s, punit->item[punit->item[i].id].fontsize);
        fontsize = punit->item[i].fontsize;
        fontsize *= min(punit->mx,punit->my);
//printf("fontsize=%d (%f %f)\n", fontsize, (float)punit->mx, (float)punit->my);
        punit->item[i].font_id = grph_setfont_sh(pmframe->fgdc, UNIT_FONTNAME, fontsize);
        break;
      default:
        break;
    }
  }

  inpfoc_wnd_move(inpfoc_find(INPFOC_MAIN, punit->inpfoc_item), punit->x+punit->item[0].x*punit->mx, punit->y+punit->item[0].y*punit->my, FALSE);

  unit_on_redraw(chno);

//printf("%s-\n", __FUNCTION__);
}

static void frame_bitblt(int x, int y, int cx, int cy, void *parg)
{
  assert(pmframe);
  pmframe->bitblt_func(x, y, cx, cy);
}

/*
void unit_on_show_ranges(int showhide)
{
  int i, j;
  unit_t * punit;

  for (i=0; i<NUM_VIEWS; i++)
  {
    punit = pmframe->punit[i];
    for (j=0; j<punit->nitems; j++)
    {
      if (punit->item[j].type.range)
        punit->item[j].visible = (showhide) ? 1 : 0;
    }
  }
  mframe_command(MFRAME_ENUMERATE, NULL);
}
*/

void unit_item_add(int chno, int type, int id, int x, int y, int arg, int flags, ...)
{
  va_list argptr;
  inpfoc_wnd_dc_t ifwd;
  unit_t * punit;
  char s[200];

  assert(pmframe);
  assert(chno < NUM_VIEWS);

  punit = pmframe->punit[chno];

  int ino = punit->nitems;

  if (ino > UNIT_MAX_NUM_ITEMS-1)
  {
    error("%s: invalid chno %d id %d", __FUNCTION__, chno, id);
    return;
  }

  punit->item[ino].type.val = type;
  punit->item[ino].type.range = (flags == UNIT_TYPE_RANGE/* && punit->item[ino].type.val == UNIT_TYPE_VAL_TEXT*/);
  punit->item[ino].id = id;
  punit->item[ino].ids = 0;
  punit->item[ino].visible = 1;
  punit->item[ino].x = x;
  punit->item[ino].y = y;
  punit->item[ino].fgcolor = UNIT_FGCOLOR;
  punit->item[ino].bkcolor = punit->bkcolor;
  punit->item[ino].image_id = (image_id_t)0;
  punit->item[ino].rc0.x0 = 0;
  punit->item[ino].rc0.x1 = 0;
  punit->item[ino].rc0.y0 = 0;
  punit->item[ino].rc0.y1 = 0;

  punit->nitems ++;

  punit->item[ino].fgcolor = chancolor[chno];
  if (flags == UNIT_TYPE_RANGE/* && punit->item[ino].type.val == UNIT_TYPE_VAL_TEXT*/)
  {
    punit->item[ino].fgcolor = DARK(chancolor[chno]);
  }

  va_start(argptr, flags);

  switch (punit->item[ino].type.val)
  {
    case UNIT_TYPE_VAL_INT:
      punit->item[ino].fontsize = arg;
      punit->item[ino].v = va_arg(argptr, int);
      punit->item[ino].font_id = grph_setfont_sh(pmframe->fgdc, UNIT_FONTNAME, punit->item[ino].fontsize);
      punit->item[ino].upd_func = (upd_func_t)unit_draw_int_data;
      break;
    case UNIT_TYPE_VAL_FLOAT:
      punit->item[ino].fontsize = arg;
      punit->item[ino].df = va_arg(argptr, double);
      punit->item[ino].font_id = grph_setfont_sh(pmframe->fgdc, UNIT_FONTNAME, punit->item[ino].fontsize);
      punit->item[ino].upd_func = (upd_func_t)unit_draw_float_data;
      break;
    case UNIT_TYPE_VAL_TEXT:
      punit->item[ino].fontsize = arg;
      if (flags && flags != UNIT_TYPE_RANGE)
      {
        punit->item[ino].ids = flags;
        ids2string(punit->item[ino].ids, s);
        strncpy(punit->item[ino].s, s, sizeof(punit->item[ino].s));
      }
      else
      {
        strncpy(punit->item[ino].s, va_arg(argptr, char*), sizeof(punit->item[ino].s));
      }
      punit->item[ino].font_id = grph_setfont_sh(pmframe->fgdc, UNIT_FONTNAME, punit->item[ino].fontsize);
      punit->item[ino].upd_func = (upd_func_t)unit_draw_text_data;
      break;
    case UNIT_TYPE_VAL_PROGRESS:
      punit->item[ino].cx = LOWORD(arg);
      punit->item[ino].cy = HIWORD(arg);
      punit->item[ino].pbs = flags;
      punit->item[ino].v = va_arg(argptr, int);
      punit->item[ino].upd_func = (upd_func_t)unit_draw_progress_data;
      break;
    case UNIT_TYPE_ICON:
      punit->item[ino].v = arg;
//      punit->item[ino].image_id = va_arg(argptr, int);
      punit->item[ino].upd_func = (upd_func_t)unit_draw_icon_data;
      break;
    case UNIT_TYPE_LABEL:
      if (flags)
      {
        punit->item[ino].ids = flags;
        ids2string(punit->item[ino].ids, s);
        strncpy(punit->item[ino].s, s, sizeof(punit->item[ino].s));
      }
      else
      {
        strncpy(punit->item[ino].s, va_arg(argptr, char*), sizeof(punit->item[ino].s));
      }
      punit->item[ino].upd_func = (upd_func_t)unit_draw_label_data;
     // if (punit->visible)
      {
        int x, y, cx, cy;
        rect_t rc;

        x = punit->x + punit->item[ino].x;
        y = punit->y + punit->item[ino].y;
        rc.x0 = x;
        rc.y0 = y;
        grph_setfont(pmframe->fgdc, LABEL_FONT);
        grph_drawtext(pmframe->fgdc, punit->item[ino].s, -1, &rc, DT_CALCRECT);
        cx = rc.x1-rc.x0 + 2;
        cy = rc.y1-rc.y0 + 2;
        ifwd.bkdc = pmframe->bkdc;
        ifwd.updc = pmframe->updc;
        ifwd.fgcolor = punit->item[0].fgcolor;

        punit->inpfoc_item = arg;
        inpfoc_add(INPFOC_MAIN, punit->inpfoc_item, (void*) inpfoc_wnd_create("PUSH", x, y, cx, cy, 0, punit->item[ino].ids, &ifwd, frame_bitblt, pmframe));
      }
      break;
    default:
      punit->item[ino].upd_func = (upd_func_t)unit_draw_no_data; // to off warning
      error("%s: Undefined type %d\n", __FUNCTION__, type);
      punit->nitems --;
      break;
  }

  mframe_command(MFRAME_UNIT_CHANGE, (void*)MAKELONG(chno, id));

  va_end(argptr);
}

void unit_ioctl(int chno, int cmd, ...)
{
  int v;
  int i;
  double df;
  int type;
  int id, idx = -1;
  char * ps;
  unit_t * punit;
  va_list argptr;

 // assert (pmframe);
  if (!pmframe) return;
 // if (!pmframe->visible) return;

  assert(chno < NUM_VIEWS);

  punit = pmframe->punit[chno];
 // if (!punit->visible) return;

//printf("ioctl: %d %d %d\n", chno, idx, type);

  va_start(argptr, cmd);

  if (cmd == UNIT_CMD_UPDATE)
  {
    int i;
    for (i=0; i<punit->nitems; i++)
      mframe_command(MFRAME_UNIT_CHANGE, (void*)MAKELONG(chno,i));
    return;
  }

  if (cmd == UNIT_CMD_SHOW)
  {
   // punit->visible = 1;
    pmframe->unit_chmask |= (1 << chno);
    mframe_command(MFRAME_ENUMERATE, NULL);
    return;
  }

  if (cmd == UNIT_CMD_HIDE)
  {
    pmframe->unit_chmask &= ~(1 << chno);
   // punit->visible = 0;
    mframe_command(MFRAME_ENUMERATE, NULL);
    return;
  }

  if (cmd == ITEM_CMD_SHOW || cmd == ITEM_CMD_HIDE)
  {
    int id;
    id = va_arg(argptr, int);
    for (i=0; i<UNIT_MAX_NUM_ITEMS; i++)
    {
      if (punit->item[i].type.val == UNIT_TYPE_VAL_UNDEFINED) continue;
      if ((punit->item[i].id) == id)
      {
        punit->item[i].visible = (cmd == ITEM_CMD_SHOW) ? 1 : 0;
        break;
      }
    }
   // mframe_command(MFRAME_ENUMERATE, NULL);
    return;
  }

  if (cmd == ITEM_CMD_SET_COLOR)
  {
    int id;
    unsigned int color;
    id = va_arg(argptr, int);
    color = va_arg(argptr, int);
    for (i=0; i<UNIT_MAX_NUM_ITEMS; i++)
    {
      if (punit->item[i].type.val == UNIT_TYPE_VAL_UNDEFINED) continue;
      if ((punit->item[i].id) == id && punit->item[i].fgcolor != color)
      {
        punit->item[i].fgcolor = color;
        punit->item[i].upd_func(chno, &punit->item[i], TRUE);
        break;
      }
    }
   // mframe_command(MFRAME_ENUMERATE, NULL);
    return;
  }

  if (cmd == SET_ALARM || cmd == CLR_ALARM || cmd == SET_ALARM_ITEM || cmd == CLR_ALARM_ITEM)
  {
    int alarm;
    alarm = va_arg(argptr, int);
    if (cmd == SET_ALARM_ITEM || cmd == CLR_ALARM_ITEM)
    {
      v = va_arg(argptr, int);
      //printf("set alarm %d %d %d\n", chno, v, alarm);
      if (v < punit->nitems && v >= 0)
      {
        int kk;
        for (kk=0; kk<punit->nitems; kk++)
        {
          if (punit->item[kk].id == v)
          {
            if (cmd == SET_ALARM_ITEM) punit->item[kk].alarmed = 1;
            if (cmd == CLR_ALARM_ITEM) punit->item[kk].alarmed = 0;
          }
        }
      }
    }

    if (cmd == SET_ALARM || cmd == SET_ALARM_ITEM)
    {
      unit_add_alarm(chno, alarm);
    }
    if (cmd == CLR_ALARM || cmd == CLR_ALARM_ITEM)
    {
      unit_del_alarm(chno, alarm);
    }
//    printf("In module %d alarm %d\n", chno, v);
  }

  if (cmd == SET_VALUE)
  {
    if (punit->nitems == 0) return;

    id = va_arg(argptr, int);

    for (i=0; i<UNIT_MAX_NUM_ITEMS; i++)
    {
      if (punit->item[i].type.val == UNIT_TYPE_VAL_UNDEFINED) continue;
      if ((punit->item[i].id) == id)
      {
        idx = i;
        break;
      }
    }

    if (idx == -1)
    {
      error("%s: chno %d invalid id (%d)\n", __FUNCTION__, chno, id);
      return;
    }

    type = punit->item[idx].type.val;

    switch (type)
    {
      case UNIT_TYPE_VAL_INT:
      case UNIT_TYPE_VAL_PROGRESS:
      case UNIT_TYPE_VAL_ICON:
        v = va_arg(argptr, int);
        if (v != punit->item[idx].v)
        {
          punit->item[idx].v = v;
          mframe_command(MFRAME_UNIT_CHANGE, (void*)(void*)MAKELONG(chno, id));
        }
        break;
      case UNIT_TYPE_VAL_FLOAT:
        df = va_arg(argptr, double);
        if (df != punit->item[idx].df)
        {
          punit->item[idx].df = df;
          mframe_command(MFRAME_UNIT_CHANGE, (void*)(void*)MAKELONG(chno, id));
        }
        break;
      case UNIT_TYPE_VAL_TEXT:
        if (punit->item[idx].ids)
        {
          int ids = va_arg(argptr, int);
          if (ids != punit->item[idx].ids)
          {
            punit->item[idx].ids = ids;
           mframe_command(MFRAME_UNIT_CHANGE, (void*)(void*)MAKELONG(chno, id));
          }
        }
        else
        {
          ps = va_arg(argptr, char*);
          if (strncmp(punit->item[idx].s, ps, sizeof(punit->item[idx].s)) != 0)
          {
            strncpy(punit->item[idx].s, ps, sizeof(punit->item[idx].s));
            mframe_command(MFRAME_UNIT_CHANGE, (void*)(void*)MAKELONG(chno, id));
          }
        }
        break;
      default:
        error("%s: undefined type %d\n", __FUNCTION__, type);
        break;
    }
  }

  va_end(argptr);
}

#ifdef UNIX
pthread_mutex_t mutex_unitdata = PTHREAD_MUTEX_INITIALIZER;
#endif

int unit_get_data(int chno, void * dataptr)
{
  int res = 1;

  if (!pmframe) return -1;

  if (chno >= NUM_VIEWS)
  {
    error("%s: Invalid chno (%d)\n", __FUNCTION__, chno);
    return -1;
  }

#ifdef UNIX
  pthread_mutex_lock(&mutex_unitdata);
#endif
  switch (chno)
  {
    case ECG:
       *((ecg_data_t*)dataptr) = *((ecg_data_t*)pmframe->punit[chno]->data);
      break;
    case SPO2:
       *((spo2_data_t*)dataptr) = *((spo2_data_t*)pmframe->punit[chno]->data);
      break;
    case NIBP:
       *((nibp_data_t*)dataptr) = *((nibp_data_t*)pmframe->punit[chno]->data);
      break;
    case T1T2:
       *((t1t2_data_t*)dataptr) = *((t1t2_data_t*)pmframe->punit[chno]->data);
      break;
    case RESP:
       *((resp_data_t*)dataptr) = *((resp_data_t*)pmframe->punit[chno]->data);
      break;
    case CO2:
       *((co2_data_t*)dataptr) = *((co2_data_t*)pmframe->punit[chno]->data);
      break;
    default:
      error("%s\n", __FUNCTION__);
      res = 0;
  }
#ifdef UNIX
  pthread_mutex_unlock(&mutex_unitdata);
#endif

  return res;
}

int unit_set_data(int chno, void * dataptr)
{
  int res = 1;

  if (!pmframe) return -1;
  if (!dataptr) return -1;

  if (chno >= NUM_VIEWS)
  {
    error("%s: invalid chno (%d)\n", __FUNCTION__, chno);
    return -1;
  }

#ifdef UNIX
  pthread_mutex_lock(&mutex_unitdata);
#endif

  switch (chno)
  {
    case ECG:
      *((ecg_data_t*)pmframe->punit[chno]->data) = *((ecg_data_t*)dataptr);
      break;
    case SPO2:
      *((spo2_data_t*)pmframe->punit[chno]->data) = *((spo2_data_t*)dataptr);
      break;
    case NIBP:
      *((nibp_data_t*)pmframe->punit[chno]->data) = *((nibp_data_t*)dataptr);
      break;
    case T1T2:
      *((t1t2_data_t*)pmframe->punit[chno]->data) = *((t1t2_data_t*)dataptr);
      break;
    case RESP:
      *((resp_data_t*)pmframe->punit[chno]->data) = *((resp_data_t*)dataptr);
      break;
    case CO2:
       *((co2_data_t*)pmframe->punit[chno]->data) = *((co2_data_t*)dataptr);
      break;
    default:
      error("%s\n", __FUNCTION__);
      res = 0;
  }

#ifdef UNIX
  pthread_mutex_unlock(&mutex_unitdata);
#endif

  return res;
}

static void unit_draw_no_data(int chno, item_t * pitem, int redraw)
{
  assert(chno < NUM_VIEWS);
  assert(pitem);
}

static void unit_draw_int_data(int chno, item_t * pitem, int redraw)
{
  assert(pitem);
  if (pitem->v != UNDEF_VALUE)
#ifdef UNIX
    snprintf(pitem->s, sizeof(pitem->s), "%d", pitem->v);
#endif
#ifdef WIN32
    _snprintf(pitem->s, sizeof(pitem->s), "%d", pitem->v);
#endif
  else
    strcpy(pitem->s, "--");

  unit_draw_text_data(chno, pitem, redraw);
}

static void unit_draw_float_data(int chno, item_t * pitem, int redraw)
{
  char fmt[10];

  assert(chno < NUM_VIEWS);
  assert(pitem);

  switch (chno)
  {
    case T1T2:
      strncpy(fmt, "%.1f", sizeof(fmt));
      break;
    case ECG:
      strncpy(fmt, "%.2f", sizeof(fmt));
      break;
    default:
      strncpy(fmt, "%f", sizeof(fmt));
  }

  if (pitem->df != UNDEF_VALUE)
#ifdef UNIX
    snprintf(pitem->s, sizeof(pitem->s), fmt, pitem->df);
#endif
#ifdef WIN32
    _snprintf(pitem->s, sizeof(pitem->s), fmt, pitem->df);
#endif
  else
    strcpy(pitem->s, "--");

  unit_draw_text_data(chno, pitem, redraw);
}

static void unit_draw_text_data(int chno, item_t * pitem, int redraw)
{
  int x, y;
  unit_t * punit;
  rect_t rc;
  unsigned long fgcolor;
  char s[200];

  assert(pitem);
  assert(pmframe);
  assert(chno < NUM_VIEWS);

  if (!pmframe->punit[chno]->visible) return;

  punit = pmframe->punit[chno];

  grph_fillrect(pmframe->fgdc, pitem->rc0.x0, pitem->rc0.y0, pitem->rc0.x1-pitem->rc0.x0, pitem->rc0.y1-pitem->rc0.y0, 0x0000000);

  x = punit->x + punit->mx*pitem->x;
  y = punit->y + punit->my*pitem->y;

  fgcolor = (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0) ? pitem->fgcolor : RGB(0x33,0x33,0x33);

  if ((pitem->bkcolor & 0x00ffffff) != 0)
  {
    fgcolor = RGB(0x33,0x33,0x33);
  }

  grph_setbkcolor(pmframe->fgdc, pitem->bkcolor);
  grph_setfgcolor(pmframe->fgdc, fgcolor);
  grph_setfont(pmframe->fgdc, pitem->font_id);
  rc.x0 = x;
  rc.y0 = y;
  if (pitem->ids)
  {
    ids2string(pitem->ids, s);
  }
  else
  {
    strncpy(s, pitem->s, min(sizeof(pitem->s), sizeof(s)));
  }
  grph_drawtext(pmframe->fgdc, s, -1, &rc, DT_CALCRECT);
 // grph_fillrect(pmframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, pitem->bkcolor);
  grph_fillroundedrect(pmframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, pitem->bkcolor);
  grph_drawtext(pmframe->fgdc, s, -1, &rc, DT_CENTER);

  if (redraw)
    frame_bitblt(x, y, max(rc.x1, pitem->rc0.x1)-rc.x0, rc.y1-rc.y0, NULL);

  memcpy(&pitem->rc0, &rc, sizeof(rect_t));
}

static void unit_draw_progress_data(int chno, item_t * pitem, int redraw)
{
  int h, v;
  int x, y;
  rect_t rc;
  unit_t * punit;
  unsigned long fgcolor;

  assert(pitem);
  assert(pmframe);
  assert(chno < NUM_VIEWS);

  punit = pmframe->punit[chno];
  if (!punit->visible) return;

  x = (int)(punit->x + pitem->x*punit->mx);
  y = (int)(punit->y + pitem->y*punit->my);

  rc.x0 = x;
  rc.y0 = y;
  rc.x1 = rc.x0+pitem->cx*punit->mx;
  rc.y1 = rc.y0+pitem->cy*punit->my;
  grph_fillrect(pmframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  fgcolor = (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0) ? 0xaaaaaa : RGB(0x10,0x10,0x10);

  grph_setfgcolor(pmframe->fgdc, fgcolor);
  grph_line(pmframe->fgdc, x, y, (int)(x+pitem->cx*punit->mx), y);
  grph_line(pmframe->fgdc, (int)(x+pitem->cx*punit->mx), y, (int)(x+pitem->cx*punit->mx), (int)(y+pitem->cy*punit->my));
  grph_line(pmframe->fgdc, x+pitem->cx*punit->mx, y+pitem->cy*punit->my, x, y+pitem->cy*punit->my);
  grph_line(pmframe->fgdc, x, y+pitem->cy*punit->my, x, y);

  v = pitem->v;
  if (v < 0)   v = 0;
  if (v > 100) v = 100;

  h = pitem->cy*punit->mx * v / 100;
  h = pitem->cy*punit->my - h;

  rc.x0 = x+1;
  rc.x1 = x + pitem->cx*punit->mx;
  rc.y0 = y + h;
  rc.y1 = y + pitem->cy*punit->my;
 // fgcolor = (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0) ? pitem->fgcolor : RGB(0x33,0x33,0x33);
 // fgcolor = pitem->fgcolor;
  fgcolor = (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0) ? pitem->fgcolor : RGB(0x10,0x10,0x10);
  grph_fillrect(pmframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, fgcolor);

  if (redraw)
    frame_bitblt(x, y, pitem->cx*punit->mx+1, pitem->cy*punit->my+1, NULL);
}

static void unit_draw_icon_data(int chno, item_t * pitem, int redraw)
{
  int x, y, cx, cy;
  float m;
  unit_t * punit;

  assert(pmframe);
  assert(pitem);
  assert(chno < NUM_VIEWS);

  punit = pmframe->punit[chno];

  if (!punit->visible) return;

  x = punit->x + punit->mx*pitem->x;
  y = punit->y + punit->my*pitem->y;
  m = min(punit->mx, punit->my);

  pitem->font_id = ARIAL_10;
  switch(pitem->v)
  {
    case IMAGE_BELL:
      if (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0)
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, bell_image.width, bell_image.height, (int)(1.0f/3.2f*bell_image.width*m), (int)(1.0f/3.2f*bell_image.height*m), (void*)bell_image.pixel_data);
        cx = (int)(1.0f/3.2f*bell_image.width*m);
        cy = (int)(1.0f/3.2f*bell_image.height*m);
      }
      else
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, bell_inv_image.width, bell_inv_image.height, (int)(1.0f/3.2f*bell_inv_image.width*m), (int)(1.0f/3.2f*bell_inv_image.height*m), (void*)bell_inv_image.pixel_data);
        cx = (int)(1.0f/3.2f*bell_inv_image.width*m);
        cy = (int)(1.0f/3.2f*bell_inv_image.height*m);
      }
      break;
    case IMAGE_NOBELL:
      if (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0)
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, nobell_image.width, nobell_image.height, (int)(1.0f/3.2f*nobell_image.width*m), (int)(1.0f/3.2f*nobell_image.height*m), (void*)nobell_image.pixel_data);
        cx = (int)(1.0f/3.2f*nobell_image.width*m);
        cy = (int)(1.0f/3.2f*nobell_image.height*m);
      }
      else
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, nobell_inv_image.width, nobell_inv_image.height, (int)(1.0f/3.2f*nobell_inv_image.width*m), (int)(1.0f/3.2f*nobell_inv_image.height*m), (void*)nobell_inv_image.pixel_data);
        cx = (int)(1.0f/3.2f*nobell_inv_image.width*m);
        cy = (int)(1.0f/3.2f*nobell_inv_image.height*m);
      }
      break;
    case IMAGE_BLANK:
     /* if (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0)*/
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, blank_image.width, blank_image.height, (int)(1.0f/3.2f*blank_image.width*m), (int)(1.0f/3.2f*blank_image.height*m), (void*)blank_image.pixel_data);
        cx = (int)(1.0f/3.2f*blank_image.width*m);
        cy = (int)(1.0f/3.2f*blank_image.height*m);
      }
      break;
    case IMAGE_HEART:
      if (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0)
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, heart_image.width, heart_image.height, (int)(1.0f/3.2f*heart_image.width*m), (int)(1.0f/3.2f*heart_image.height*m), (void*)heart_image.pixel_data);
        cx = (int)(1.0f/3.2f*heart_image.width*m);
        cy = (int)(1.0f/3.2f*heart_image.height*m);
      }
      else
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, heart_inv_image.width, heart_inv_image.height, (int)(1.0f/3.2f*heart_inv_image.width*m), (int)(1.0f/3.2f*heart_inv_image.height*m), (void*)heart_inv_image.pixel_data);
        cx = (int)(1.0f/3.2f*heart_inv_image.width*m);
        cy = (int)(1.0f/3.2f*heart_inv_image.height*m);
      }
      break;
    case IMAGE_PM:
     /* if (((punit->bkcolor ^ MFRAME_BKCOLOR) & 0x00ffffff) == 0)*/
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, pm_image.width, pm_image.height, (int)(1.0f/3.2f*pm_image.width*m), (int)(1.0f/3.2f*pm_image.height*m), (void*)pm_image.pixel_data);
        cx = (int)(1.0f/3.2f*pm_image.width*m);
        cy = (int)(1.0f/3.2f*pm_image.height*m);
      }
     /* else
      {
        grph_filldatastretch16(pmframe->fgdc, x, y, pm_inv_image.width, pm_inv_image.height, (int)(1.0f/3.2f*pm_inv_image.width*punit->my), (int)(1.0f/3.2f*pm_inv_image.height*punit->my), (void*)pm_inv_image.pixel_data);
        cx = (int)(1.0f/3.2f*pm_inv_image.width*punit->my);
        cy = (int)(1.0f/3.2f*pm_inv_image.height*punit->my);
      }*/
      break;
    default:
      error("%s: unknown image no (%d)\n", __FUNCTION__, pitem->v);
      cx = 0;
      cy = 0;
      break;
  }
  if (redraw)
    frame_bitblt(x, y, cx, cy, NULL);
}

static void unit_draw_label_data(int chno, item_t * pitem, int redraw)
{
  int x, y;
  unit_t * punit;
  rect_t rc;

  assert(pitem);
  assert(pmframe);
  assert(chno < NUM_VIEWS);

  punit = pmframe->punit[chno];

  if (!punit->visible) return;

  grph_setbkcolor(pmframe->fgdc, pitem->bkcolor);
  grph_setfgcolor(pmframe->fgdc, pitem->fgcolor);
  grph_setfont(pmframe->fgdc, LABEL_FONT);

  x = (int)(punit->x + punit->mx*pitem->x);
  y = (int)(punit->y + punit->my*pitem->y);

  rc.x0 = x;
  rc.y0 = y;
  rc.x1 = rc.x0 + 34;
  rc.y1 = rc.y0 + 20;

  if (pitem->ids)
  {
    char s[200];
    ids2string(pitem->ids, s);
   // grph_drawtext(pmframe->fgdc, s, -1, &rc, DT_CALCRECT);
   // grph_drawtext(pmframe->fgdc, s, -1, &rc, DT_LEFT);
    grph_drawtext(pmframe->fgdc, s, -1, &rc, DT_CENTER);
  }
  else
  {
   // grph_drawtext(pmframe->fgdc, pitem->s, -1, &rc, DT_CALCRECT);
   // grph_drawtext(pmframe->fgdc, pitem->s, -1, &rc, DT_LEFT);
  }

  if (redraw)
    frame_bitblt(x, y, rc.x1-rc.x0, rc.y1-rc.y0, NULL);
}

void unit_ini_def(int reset_meas)
{
  int i;
  ecg_data_t   ecg_data;
  spo2_data_t  spo2_data;
  nibp_data_t  nibp_data;
  resp_data_t  resp_data;
  t1t2_data_t  t1t2_data;
  co2_data_t   co2_data;
  pat_pars_t lpp;
  long v;

  unit_get_data(ECG,  &ecg_data);
  unit_get_data(SPO2, &spo2_data);
  unit_get_data(NIBP, &nibp_data);
  unit_get_data(RESP, &resp_data);
  unit_get_data(T1T2, &t1t2_data);
  unit_get_data(CO2,  &co2_data);

  if (reset_meas) ecg_data.hr = UNDEF_VALUE;
  ecg_data.hr_min = 50;
  ecg_data.hr_max = 120;
  for (i=0; i<NUM_ECG; i++)
  {
    if (reset_meas) ecg_data.st[i] = UNDEF_VALUE;
    ecg_data.st_min[i] = -200;
    ecg_data.st_max[i] = 200;
  }
  ecg_data.asys_sec = 5;
  ecg_data.tach_bpm = 100;
  ecg_data.pmnr_ms = 120;
  ecg_data.hr_src = 1; // 0 - auto, 1 - ecg // see ecgset.h
  ecg_data.hr_src_curr = 1; // 1 - ecg // see ecgset.h
  ecg_data.num_leads = 5; // 5-lead ecg by deafult
  ecg_data.st1 = ECG_I;
  ecg_data.st2 = ECG_II;
  ecg_data.j_shift = 50;
  ecg_data.set_bits.diag_f = 1; // DIAG+F as default

  if (reset_meas) spo2_data.hr = UNDEF_VALUE;
//   spo2_data.hr_min = 50;
//   spo2_data.hr_max = 120;
  if (reset_meas) spo2_data.spo2 = UNDEF_VALUE;
//   spo2_data.spo2_min = 91;
//   spo2_data.spo2_max = 100;
  if (reset_meas) spo2_data.stolb = 0;
  if (reset_meas) spo2_data.scale = UNDEF_VALUE;

//   nibp_data.sd_min = 160;
//   nibp_data.sd_max = 90;
//   nibp_data.dd_max = 90;
//   nibp_data.dd_min = 50;
//   nibp_data.md_max = 110;
//   nibp_data.md_min = 60;
  if (reset_meas) nibp_data.sd  = UNDEF_VALUE;
  if (reset_meas) nibp_data.dd  = UNDEF_VALUE;
  if (reset_meas) nibp_data.md  = UNDEF_VALUE;
  if (reset_meas) nibp_data.hr  = UNDEF_VALUE;
  v = UNDEF_VALUE;
  assert(sizeof(v) == sizeof(trts_t));
  if (reset_meas) memcpy(&nibp_data.trts_meas, &v, sizeof(v));
  if (reset_meas) nibp_data.hr  = UNDEF_VALUE;
  nibp_data.meas_interval   = NIBP_MEAS_INTERVAL_MANU; // manually
  nibp_data.ec = 0;

  if (reset_meas) resp_data.br = UNDEF_VALUE;
//   resp_data.br_min = 8;
//   resp_data.br_max = 30;
  if (reset_meas) resp_data.ap = UNDEF_VALUE;
//   resp_data.ap_min = 5;
//   resp_data.ap_max = 20;

  if (reset_meas) t1t2_data.t1 = UNDEF_VALUE; // 0.0f
  if (reset_meas) t1t2_data.t2 = UNDEF_VALUE;
  t1t2_data.t1_min = 360;
  t1t2_data.t1_max = 390;
  t1t2_data.t2_min = 360;
  t1t2_data.t2_max = 390; // 39.0f
  t1t2_data.dt_max = 20;  // 2.0f

  if (reset_meas) co2_data.etco2 = UNDEF_VALUE;
  if (reset_meas) co2_data.ico2 = UNDEF_VALUE;
  if (reset_meas) co2_data.br = UNDEF_VALUE;
  co2_data.etco2_min = 20;
  co2_data.etco2_max = 60;
  co2_data.ico2_max = 20;
  co2_data.ap_max = 20;
  co2_data.br_min = 5;
  co2_data.br_max = 30;
  co2_data.pressure = 760;
  co2_data.o2_compens = 16;
  co2_data.gas_balance = 0;
  co2_data.anest_agent = 0;

  // patients' type dependent thresholds
  pat_get(&lpp);
  switch (lpp.type)
  {
    case ADULT:
      spo2_data.spo2_min = 90;
      spo2_data.spo2_max = 100;
      spo2_data.hr_min = 50;
      spo2_data.hr_max = 120;
      nibp_data.sd_min = 90;
      nibp_data.sd_max = 160;
      nibp_data.dd_min = 50;
      nibp_data.dd_max = 90;
      nibp_data.md_min = 60;
      nibp_data.md_max = 110;
      nibp_data.infl   = 160;
      resp_data.br_max = 30;
      resp_data.br_min = 8;
      resp_data.ap_max = 20;
      break;
    case CHILD:
      spo2_data.spo2_min = 90;
      spo2_data.spo2_max = 100;
      spo2_data.hr_min = 75;
      spo2_data.hr_max = 160;
      nibp_data.sd_min = 70;
      nibp_data.sd_max = 120;
      nibp_data.dd_min = 40;
      nibp_data.dd_max = 70;
      nibp_data.md_min = 50;
      nibp_data.md_max = 90;
      nibp_data.infl   = 120;
      resp_data.br_max = 30;
      resp_data.br_min = 8;
      resp_data.ap_max = 20;
      break;
#if defined (FEATURE_NEONATE)
    case NEONATE:
      spo2_data.spo2_min = 85;
      spo2_data.spo2_max = 95;
      spo2_data.hr_min = 100;
      spo2_data.hr_max = 200;
      nibp_data.sd_min = 40;
      nibp_data.sd_max = 90;
      nibp_data.dd_min = 20;
      nibp_data.dd_max = 60;
      nibp_data.md_min = 25;
      nibp_data.md_max = 70;
      nibp_data.infl   = 90;
      resp_data.br_max = 100;
      resp_data.br_min = 30;
      resp_data.ap_max = 10;
      break;
#endif
    default:
      error("%s: unexpected case %d in switch\n", __FUNCTION__, lpp.type);
      break;
  }

  unit_set_data(ECG,  &ecg_data);
  unit_set_data(SPO2, &spo2_data);
  unit_set_data(NIBP, &nibp_data);
  unit_set_data(RESP, &resp_data);
  unit_set_data(T1T2, &t1t2_data);
  unit_set_data(CO2,  &co2_data);

  // clear all phys alarms
  if (reset_meas)
  {
    for (i=0; i<NUM_ALARMS; i++)
    {
      if (alarm_msg[i].type == ALR_PHYS)
      {
        alarm_clr(i);
      }
    }
  }

  unit_update_data_all();
}

void unit_update_data_all(void)
{
  char s[200];
 // unsigned short ids;
  ecg_data_t  ecg_data;
  spo2_data_t spo2_data;
  nibp_data_t nibp_data;
  resp_data_t resp_data;
  t1t2_data_t t1t2_data;
  co2_data_t  co2_data;

  unit_get_data(ECG,  &ecg_data);
  unit_get_data(SPO2, &spo2_data);
  unit_get_data(NIBP, &nibp_data);
  unit_get_data(RESP, &resp_data);
  unit_get_data(T1T2, &t1t2_data);
  unit_get_data(CO2,  &co2_data);

  // ECG
#if 0
  switch (ecg_data.hr_src)
  {
    case HRSRC_AUTO:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, RGB(0xff,0xff,0xff));
      ids = IDS_AUTO;
      break;
    case HRSRC_ECG:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[ECG]);
      ids = IDS_ECG;
      break;
    case HRSRC_SPO2:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[SPO2]);
      ids = IDS_SPO2;
      break;
    case HRSRC_NIBP:
      unit_ioctl(ECG, ITEM_CMD_SET_COLOR, UNIT_ECG_HRSRC, chancolor[NIBP]);
      ids = IDS_NIBP;
      break;
    default:
      ids = IDS_UNDEF3;
  }
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HRSRC, ids);
#endif
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR_BELL, (int) (alarm_isenabled(ECG_RISKHR) ? IMAGE_BELL : IMAGE_NOBELL));
  snprintf(s, sizeof(s), "%d..%d", ecg_data.hr_min, ecg_data.hr_max);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR_RANGE, s);

  if (ecg_data.hr_src == HRSRC_ECG || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == ECG))
  {
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int) ecg_data.hr);
  }
  if (ecg_data.hr_src == HRSRC_SPO2 || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == SPO2))
   unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int) spo2_data.hr);
  if (ecg_data.hr_src == HRSRC_NIBP || (ecg_data.hr_src == HRSRC_AUTO && ecg_data.hr_src_curr == NIBP))
   unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HR, (int) nibp_data.hr);

  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_VALUE, ecg_data.st[ecg_data.st1-ECG_FIRST]);
  snprintf(s, sizeof(s), "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, (int) (alarm_isenabled(ECG_RISKSTI+(ecg_data.st1-ECG_FIRST)) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_VALUE, ecg_data.st[ecg_data.st2-ECG_FIRST]);
  snprintf(s, sizeof(s), "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, (int) (alarm_isenabled(ECG_RISKSTI+(ecg_data.st2-ECG_1)) ? IMAGE_BELL : IMAGE_NOBELL));

  // ECG Neo

  // SpO2
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_PULSE, (int)(spo2_data.hr));
  snprintf(s, sizeof(s), "%d..%d", spo2_data.hr_min, spo2_data.hr_max);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_PULSE_RANGE, s);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT, spo2_data.spo2);
  snprintf(s, sizeof(s), "%d..%d", spo2_data.spo2_min, spo2_data.spo2_max);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT_RANGE, s);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SAT_BELL, (int) (alarm_isenabled(SPO2_RISKSAT) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_HR_BELL, (int) (alarm_isenabled(SPO2_RISKPULSE) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_STOLBIK, spo2_data.stolb*100/64);
  unit_ioctl(SPO2, SET_VALUE, UNIT_SPO2_SCALE, spo2_data.scale);

  // NIBP
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_SS, nibp_data.sd);
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_DD, nibp_data.dd);
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_CC, nibp_data.md);
 // unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_PULSE, nibp_data.hr);
  snprintf(s, sizeof(s), "- - - - - - -");
  if (nibp_data.trts_meas.year < (2012-1900))
  {
    *((long*)&nibp_data.trts_meas) = UNDEF_VALUE;
  }
  if (*((long*)&nibp_data.trts_meas) != UNDEF_VALUE && nibp_data.trts_meas.year > (2011-1900))
  {
    sprintf(s, "%02d:%02d %02d.%02d.%04d", nibp_data.trts_meas.hour, nibp_data.trts_meas.min, nibp_data.trts_meas.day, nibp_data.trts_meas.mon, nibp_data.trts_meas.year+1900);
  }
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_STIME, s);
  snprintf(s, sizeof(s), "%d..%d", nibp_data.sd_min, nibp_data.sd_max);
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_SS_RANGE, s);
  snprintf(s, sizeof(s), "%d..%d", nibp_data.dd_min, nibp_data.dd_max);
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_DD_RANGE, s);
  snprintf(s, sizeof(s), "%d..%d", nibp_data.md_min, nibp_data.md_max);
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BP_CC_RANGE, s);
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_BELL, (int) (alarm_isenabled(NIBP_RISKSS) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(NIBP, SET_VALUE, UNIT_NIBP_CUFF_PRESSURE, (int)UNDEF_VALUE);

  // Spiro
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR, resp_data.br);
  snprintf(s, sizeof(s), "%d..%d", resp_data.br_min, resp_data.br_max);
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR_RANGE, s);
  ids2string(IDS_UNDEF3, s);
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_STATE, s);
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_APNOE_TIME, resp_data.ap_max);
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR_BELL, (int) (alarm_isenabled(RESP_RISKBR) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_APNOE_BELL, (int) (alarm_isenabled(RESP_APNOE) ? IMAGE_BELL : IMAGE_NOBELL));

  // T1 T2
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1, (t1t2_data.t1 != UNDEF_VALUE) ? (float)t1t2_data.t1 / 10.0f : (float) UNDEF_VALUE);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2, (t1t2_data.t2 != UNDEF_VALUE) ? (float)t1t2_data.t2 / 10.0f : (float) UNDEF_VALUE);
  if (t1t2_data.t1 == UNDEF_VALUE || t1t2_data.t2 == UNDEF_VALUE)
    unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT, (float)UNDEF_VALUE);
  else
    unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT, (float)(abs(t1t2_data.t1 - t1t2_data.t2)) / 10.0f);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1_BELL, (int) (alarm_isenabled(T1T2_RISKT1) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2_BELL, (int) (alarm_isenabled(T1T2_RISKT2) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT_BELL, (int) (alarm_isenabled(T1T2_RISKDT) ? IMAGE_BELL : IMAGE_NOBELL));
  snprintf(s, sizeof(s), "%.1f..%.1f", (float) (t1t2_data.t1_min) / 10.0f, (float) (t1t2_data.t1_max) / 10.0f);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T1_RANGE, s);
  snprintf(s, sizeof(s), "%.1f..%.1f", (float) (t1t2_data.t2_min) / 10.0f,  (float) (t1t2_data.t2_max) / 10.0f);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_T2_RANGE, s);
  snprintf(s, sizeof(s), "%.1f", (float) (t1t2_data.dt_max) / 10.0f);
  unit_ioctl(T1T2, SET_VALUE, UNIT_T1T2_DT_RANGE, s);

  // CO2
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2, co2_data.etco2);
  snprintf(s, sizeof(s), "%d..%d", co2_data.etco2_min, co2_data.etco2_max);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2_RANGE, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ETCO2_BELL, (int) (alarm_isenabled(CO2_RISKETCO2) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR, co2_data.br);
  snprintf(s, sizeof(s), "%d..%d", co2_data.br_min, co2_data.br_max);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR_RANGE, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BR_BELL, (int) (alarm_isenabled(CO2_RISKBRCO2) ? IMAGE_BELL : IMAGE_NOBELL));
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2, co2_data.ico2);
  snprintf(s, sizeof(s), "%d", co2_data.ico2_max);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2_RANGE, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_ICO2_BELL, (int) (alarm_isenabled(CO2_RISKICO2) ? IMAGE_BELL : IMAGE_NOBELL));
  ids2string(IDS_UNDEF3, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_BRSTATE, s);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_APNOE_TIME, co2_data.ap_max);
  unit_ioctl(CO2, SET_VALUE, UNIT_CO2_APNOE_BELL, (int) (alarm_isenabled(CO2_APNOE) ? IMAGE_BELL : IMAGE_NOBELL));
}

void unit_cfg_save(void)
{
  units_cfg_t units_cfg;
  memset(&units_cfg, 0, sizeof(units_cfg_t));
  unit_get_data(ECG,  &units_cfg.ecg_data);
  unit_get_data(SPO2, &units_cfg.spo2_data);
  unit_get_data(NIBP, &units_cfg.nibp_data);
  unit_get_data(T1T2, &units_cfg.t1t2_data);
  unit_get_data(RESP, &units_cfg.resp_data);
  unit_get_data(CO2,  &units_cfg.co2_data);
  if ( stconf_write(STCONF_UNITS, &units_cfg, sizeof(units_cfg_t)) > 0 );
}

void unit_hide_heart(void)
{
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_HEART_IMG, IMAGE_BLANK);
}

void unit_hide_pm(void)
{
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_PM_IMG, IMAGE_BLANK);
}

