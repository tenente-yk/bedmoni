/*! \file inpfoc_wnd.c
    \brief Input focus graphics
*/

#ifdef WIN32
//#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "bedmoni.h"
#include "dview.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "grph.h"

#ifdef SMOOTHING_BUTTONS
#define SB_CX     4
#define SB_CY     4
unsigned char smoothing_mask[] = 
{
  0x0F,
  0x03,
  0x01,
  0x01,
};
#endif

void inpfoc_wnd_init(void)
{
}

void inpfoc_wnd_deinit(void)
{
}

static void inpfoc_wnd_draw(inpfoc_wnd_t * pifw, int fd, int state)
{
  assert(pifw);
  int dc;
  rect_t rect, rect2;
  slider_info_t * psli = NULL;
  COLORREF color;

  assert(pifw);

  dc = fd;
  rect.x0 = pifw->x;
  rect.y0 = pifw->y;
  rect.x1 = pifw->x+pifw->cx;
  rect.y1 = pifw->y+pifw->cy;

  grph_fillrect(pifw->upfd, pifw->x, pifw->y, pifw->cx, pifw->cy, RGB(0,0,0));
  if (dc == pifw->bkfd)
  {
    grph_fillrect(pifw->bkfd, pifw->x, pifw->y, pifw->cx, pifw->cy-1, pifw->bkcolor);
  }
  if (state == STATE_NONE)
    color = INPFOC_COLOR_DEF;
  else if (state == STATE_ACTIVE)
    color = INPFOC_COLOR_ACTIVE;
  else if (state == STATE_SELECT)
    color = INPFOC_COLOR_SELECT;
  else if (state == STATE_DISABLED)
    color = INPFOC_COLOR_DISABLED;
  else
    color = INPFOC_COLOR_DEF;

  switch (pifw->type)
  {
    case INPFOC_WND_LABEL:
    case INPFOC_WND_LIST:
      if (!pifw->transp || dc != pifw->bkfd)
      {
        grph_fillrect(dc, pifw->x, pifw->y, pifw->cx, pifw->cy-1, color);
        grph_setfgcolor(dc, (state == STATE_SELECT) ? RGB(0x11,0x11,0x11) : RGB(0xcc,0xcc,0xcc));
        grph_line(dc, pifw->x, pifw->y, pifw->x+pifw->cx-1, pifw->y);
        if (pifw->type != INPFOC_WND_LIST)
        {
          grph_line(dc, pifw->x, pifw->y, pifw->x, pifw->y+pifw->cy-2);
#ifdef SMOOTHING_BUTTONS
          grph_setfgcolor(dc, RGB(0,0,0));
          grph_line(dc, pifw->x, pifw->y+pifw->cy-2, pifw->x, pifw->y+pifw->cy-SB_CY);
#endif
        }
#ifndef SMOOTHING_BUTTONS
        grph_setfgcolor(dc, (state == STATE_SELECT) ? RGB(0xcc,0xcc,0xcc) : RGB(0x11,0x11,0x11));
        grph_line(dc, pifw->x, pifw->y+pifw->cy-2, pifw->x+pifw->cx-1, pifw->y+pifw->cy-2);
        if (pifw->type != INPFOC_WND_LIST)
          grph_line(dc, pifw->x+pifw->cx-1, pifw->y, pifw->x+pifw->cx-1, pifw->y+pifw->cy-2);
#endif
#ifdef SMOOTHING_BUTTONS
       // if (pifw->type == INPFOC_WND_PUSH)
        {
          int j,i,b,dx0,dx1,dy0,dy1;
          dx0 = dx1 = 0;
          dy0 = dy1 = 0;
          if (state == STATE_SELECT) dy0 = dy1 = 1;
          if (pifw->type == INPFOC_WND_LIST)
          {
            dx0 = pifw->cy / 4 + 2;
            dx1 = pifw->cy / 4 + 1;
            grph_setfgcolor(dc, pifw->bkcolor);
          }
          for (i=0; i<SB_CY; i++)
          {
            for (j=0; j<SB_CX; j++)
            {
              if (smoothing_mask[i*((SB_CX+7)/8) + j/8] & (1<<(j&0x7)))
              {
                grph_line(dc, pifw->x+dx0+j, pifw->y+dy0+i, pifw->x+dx0+j, pifw->y+dy0+i);
                grph_line(dc, pifw->x+dx0+j, pifw->y+pifw->cy-2+dy1-i, pifw->x+dx0+j, pifw->y+pifw->cy-2+dy1-i);
                grph_line(dc, pifw->x+pifw->cx-1-dx1-j, pifw->y+dy0+i, pifw->x+pifw->cx-1-dx1-j, pifw->y+dy0+i);
                grph_line(dc, pifw->x+pifw->cx-1-dx1-j, pifw->y+pifw->cy-2+dy1-i, pifw->x+pifw->cx-1-dx1-j, pifw->y+pifw->cy-2+dy1-i);
              }
            }
          }
          if (state == STATE_SELECT && pifw->type == INPFOC_WND_LIST)
          {
#ifdef SMOOTHING_BUTTONS
            int kk,n;
            for (kk=0, n=0; kk<8; kk++)
            {
              if (smoothing_mask[0] & (1<<kk)) n++;
            }
            grph_setfgcolor(dc, color);
            grph_line(dc, pifw->x+dx0+n, pifw->y+pifw->cy-1, pifw->x+pifw->cx-dx1-n,  pifw->y+pifw->cy-1);
#else
#endif
          }
          grph_setfgcolor(dc, (state == STATE_SELECT) ? RGB(0x11,0x11,0x11) : RGB(0xcc,0xcc,0xcc));
          for (i=0; i<SB_CY; i++)
          {
            b = 0;
            for (j=0; j<SB_CX; j++)
            {
              if (smoothing_mask[i*((SB_CX+7)/8) + j/8] & (1<<(j&0x7)))
                b = 1;
              else
              {
                if (b)
                {
                  grph_line(dc, pifw->x+dx0+j, pifw->y+i, pifw->x+dx0+j, pifw->y+i);
                }
                b = 0;
              }
            }
          }
        }
#endif
      }
      if (pifw->type == INPFOC_WND_LIST)
      {
        unsigned char b;
        color = (dc != pifw->bkfd) ? 0x000000 : pifw->bkcolor;
        b = pifw->cy / 4;
        grph_setfgcolor(dc, color);
#if 0
        grph_filltriangle(dc, pifw->x, pifw->y, pifw->x+b, pifw->y, pifw->x, pifw->y+pifw->cy/2);
        grph_filltriangle(dc, pifw->x, pifw->y+pifw->cy-1, pifw->x+b, pifw->y+pifw->cy-1, pifw->x, pifw->y+pifw->cy/2);
        grph_filltriangle(dc, pifw->x+pifw->cx-1-b, pifw->y, pifw->x+pifw->cx-1, pifw->y, pifw->x+pifw->cx-1, pifw->y+pifw->cy/2);
        grph_filltriangle(dc, pifw->x+pifw->cx-1, pifw->y+pifw->cy-1, pifw->x+pifw->cx-1-b, pifw->y+pifw->cy-1, pifw->x+pifw->cx-1, pifw->y+pifw->cy/2);
#else
        if (state == STATE_SELECT) pifw->y += 1;

        grph_fillrect(dc, pifw->x, pifw->y, b+1, b+1, color);
        grph_fillrect(dc, pifw->x, pifw->y+pifw->cy-b+1, b+1, b-1, color);
        grph_filltriangle(dc, pifw->x, pifw->y+b+1, pifw->x+b-1, pifw->y+b+1, pifw->x, pifw->y+2*b+1);
        grph_filltriangle(dc, pifw->x, pifw->y+3*b, pifw->x+b, pifw->y+3*b, pifw->x, pifw->y+2*b);

        grph_fillrect(dc, pifw->x+pifw->cx-b, pifw->y-1, b, b+1, color);
        grph_fillrect(dc, pifw->x+pifw->cx-b, pifw->y+pifw->cy-b, b, b, color);
        grph_filltriangle(dc, pifw->x+pifw->cx-b, pifw->y+b, pifw->x+pifw->cx, pifw->y+b, pifw->x+pifw->cx, pifw->y+2*b);
        grph_filltriangle(dc, pifw->x+pifw->cx-b, pifw->y+3*b, pifw->x+pifw->cx, pifw->y+2*b, pifw->x+pifw->cx, pifw->y+3*b);

        if (state == STATE_SELECT) pifw->y -= 1;

        grph_setfgcolor(dc, pifw->bkcolor);
        grph_line(dc, pifw->x+b+1, pifw->y, pifw->x+b+1, pifw->y+pifw->cy);
        grph_line(dc, pifw->x+pifw->cx-b-1, pifw->y, pifw->x+pifw->cx-b-1, pifw->y+pifw->cy);
#endif
      }
      grph_setbkcolor(dc, pifw->bkcolor);
      if (state == STATE_ACTIVE)
        grph_setfgcolor(dc, RGB(0x10,0x10,0x10));
      else if (state == STATE_SELECT)
        grph_setfgcolor(dc, RGB(0xee,0xee,0xee));
      else
        grph_setfgcolor(dc, pifw->fgcolor);
      grph_setfont(dc, LABEL_FONT);
      if (state == STATE_SELECT)
      {
        rect.x0 += 1;
        rect.y0 += 1; // zalipanie
      }
      if (pifw->ids)
      {
        char s[200], s2[200];
        if (pifw->caption[0])
        {
          ids2string(pifw->ids, s2);
          strcpy(s, pifw->caption);
          strcat(s, s2);
        }
        else
          ids2string(pifw->ids, s);
        grph_drawtext(dc, s, -1, &rect, DT_CENTER);
      }
      else
        grph_drawtext(dc, pifw->caption, -1, &rect, DT_CENTER);
      break;
    case INPFOC_WND_CHECKBOX:
      grph_fillrect(dc, pifw->x, pifw->y, pifw->cx, pifw->cy, color);
      grph_setfgcolor(dc, INPFOC_COLOR_TEXT);
      grph_line(dc, pifw->x+4, pifw->y+4, pifw->x+4+11, pifw->y+4);
      grph_line(dc, pifw->x+4+11, pifw->y+4, pifw->x+4+11, pifw->y+4+11);
      grph_line(dc, pifw->x+4+11, pifw->y+4+11, pifw->x+4, pifw->y+4+11);
      grph_line(dc, pifw->x+4, pifw->y+4+11, pifw->x+4, pifw->y+4);
      rect.x0 = pifw->x +3 + 15;
     // grph_drawtext(dc, pifw->caption, -1, &rect, DT_LEFT);
      if (pifw->ids)
      {
        char s[200], s2[200];
        if (pifw->caption[0])
        {
          ids2string(pifw->ids, s2);
          strcpy(s, pifw->caption);
          strcat(s, s2);
        }
        else
          ids2string(pifw->ids, s);
        grph_drawtext(dc, s, -1, &rect, DT_CENTER);
      }
      else
        grph_drawtext(dc, pifw->caption, -1, &rect, DT_CENTER);
      if (pifw->checked)
      {
        rect2.x0 = pifw->x + 4 + 3;
        rect2.y0 = pifw->y + 4 + 3;
        rect2.x1 = pifw->x + 4 + 3 + 11 - 5;
        rect2.y1 = pifw->y + 4 + 11 - 2;
        grph_fillrect(dc, rect2.x0, rect2.y0, rect2.x1 - rect2.x0, rect2.y1 - rect2.y0, INPFOC_COLOR_TEXT);
      }
      break;
    case INPFOC_WND_RADIOBUTTON:
      grph_fillrect(dc, pifw->x, pifw->y, pifw->cx, pifw->cy, color);
      grph_setfgcolor(dc, INPFOC_COLOR_TEXT);
      grph_circle(dc, pifw->x + 7, pifw->y+3 + 5, 6);
      rect.x0 = pifw->x + 17;
      if (pifw->ids)
      {
        char s[200], s2[200];
        if (pifw->caption[0])
        {
          ids2string(pifw->ids, s2);
          strcpy(s, pifw->caption);
          strcat(s, s2);
        }
        else
          ids2string(pifw->ids, s);
        grph_drawtext(dc, s, -1, &rect, DT_CENTER);
      }
      else
        grph_drawtext(dc, pifw->caption, -1, &rect, DT_CENTER);
     // grph_drawtext(dc, pifw->caption, -1, &rect, DT_LEFT);
      if (pifw->checked)
      {
        rect2.x0 = pifw->x + 5;
        rect2.y0 = pifw->y + 3 + 3;
        rect2.x1 = pifw->x + 5 + 10 - 5;
        rect2.y1 = pifw->y + 3 + 10 - 2;
        grph_fillrect(dc, rect2.x0, rect2.y0, rect2.x1 - rect2.x0, rect2.y1 - rect2.y0, INPFOC_COLOR_TEXT);
      }
      break;
    case INPFOC_WND_SPINBUTTON:
      grph_fillrect(dc, pifw->x, pifw->y, pifw->cx, pifw->cy, color);
      grph_setfgcolor(dc, (state == STATE_SELECT) ? RGB(0x11,0x11,0x11) : RGB(0xcc,0xcc,0xcc));
      grph_line(dc, pifw->x, pifw->y, pifw->x+pifw->cx-1, pifw->y);
      grph_line(dc, pifw->x, pifw->y, pifw->x, pifw->y+pifw->cy-1);
      grph_setfgcolor(dc, (state == STATE_SELECT) ? RGB(0xcc,0xcc,0xcc) : RGB(0x11,0x11,0x11));
      grph_line(dc, pifw->x, pifw->y+pifw->cy-1, pifw->x+pifw->cx-1, pifw->y+pifw->cy-1);
      grph_line(dc, pifw->x+pifw->cx-1, pifw->y, pifw->x+pifw->cx-1, pifw->y+pifw->cy-1);
      grph_setfgcolor(dc, pifw->fgcolor);
      grph_line(dc, pifw->x+1, pifw->y+4, pifw->x+pifw->cx/2, pifw->y+1);
      grph_line(dc, pifw->x+pifw->cx/2, pifw->y+1, pifw->x-1+pifw->cx, pifw->y+4);
      grph_line(dc, pifw->x+1, pifw->cy+pifw->y-4, pifw->x+pifw->cx/2, pifw->cy+pifw->y-1);
      grph_line(dc, pifw->x+pifw->cx/2, pifw->cy+pifw->y-1, pifw->x-1+pifw->cx, pifw->cy+pifw->y-4);
      grph_drawtext(dc, pifw->caption, -1, &rect, DT_CENTER);
      break;
    case INPFOC_WND_SLIDER:
      psli = (slider_info_t*)pifw->caption;
      assert(psli->range);
      if (psli->slider_type == SLIDER_TYPE_HORZ)
      {
        grph_fillrect(dc, pifw->x, pifw->y + pifw->cy / 2 - 1, pifw->cx, 2, INPFOC_COLOR_DEF);
        rect2.x0 = pifw->x + (int) ((double)psli->pos / psli->range * pifw->cx) - 2;
        rect2.y0 = rect.y0 + 1 + 1;
        rect2.x1 = pifw->x + (int) ((double)psli->pos / psli->range * pifw->cx) + 2;
        rect2.y1 = rect.y1 - 1 - 1;
        if (rect2.x0 < pifw->x)
        {
          rect2.x0 = pifw->x;
          rect2.x1 += 2;
        }
        if (rect2.x1 > rect.x1)
        {
          rect2.x1 = rect.x1;
          rect2.x0 = rect.x1 - 4;
        }
        grph_fillrect(dc, rect2.x0, rect2.y0, rect2.x1-rect2.x0, rect2.y1-rect2.y0, color);
      }
      break;
  }

  pifw->bitblt_func(pifw->x, pifw->y, pifw->cx, pifw->cy, pifw->pframe);
}

inpfoc_wnd_t * inpfoc_wnd_create(char * name, int x, int y, int cx, int cy, char *s, unsigned short ids, inpfoc_wnd_dc_t * pifwd, USER_FRAMEBITBLT bitblt_func, void* pframe, ...)
{
  inpfoc_wnd_t * pifw;
  slider_info_t * psli = NULL;
  scrollbar_info_t * psci = NULL;
  progressbar_info_t * ppbi = NULL;

  pifw = (inpfoc_wnd_t*)calloc(1, sizeof(inpfoc_wnd_t));

  pifw->x = x;
  pifw->y = y;
  pifw->cx = cx;
  pifw->cy = cy;

  assert(x >= 0 && y >= 0);

  pifw->bkfd = pifwd->bkdc;
  pifw->upfd = pifwd->updc;

  grph_setfont(pifw->bkfd, LABEL_FONT);
  grph_setfont(pifw->upfd, LABEL_FONT);

  pifw->ids = ids;
  pifw->caption[0] = 0;

  if ( strcmp(name, "INPFOC") == 0 || strcmp(name, "PUSH") == 0)
  {
    pifw->type = INPFOC_WND_LABEL;
  }
  else if (strcmp(name, "LIST") == 0)
  {
    pifw->type = INPFOC_WND_LIST;
  }
  else if (strcmp(name, "RADIOBUTTON") == 0)
  {
    pifw->type = INPFOC_WND_RADIOBUTTON;
    pifw->checked = 0;
  }
  else if (strcmp(name, "CHECKBOX") == 0)
  {
    pifw->type = INPFOC_WND_CHECKBOX;
    pifw->checked = 0;
  }
  else if (strcmp(name, "SPINBUTTON") == 0)
  {
    pifw->type = INPFOC_WND_SPINBUTTON;
  }
  else if (strcmp(name, "SCROLLBAR") == 0)
  {
    va_list argptr;
    pifw->type = INPFOC_WND_SCROLLBAR;
    va_start(argptr, pframe);
    psci = va_arg(argptr, scrollbar_info_t *);
    va_end(argptr);
    if (sizeof(scrollbar_info_t) > sizeof(((inpfoc_wnd_t*)(0))->caption))
    {
      error("%s: memory overlap\n", __FUNCTION__);
      while(1);
    }
    *((scrollbar_info_t*)pifw->caption) = *psci;
    if (psci->range) psci->pos %= psci->range;
    else psci->pos = 0;
    if (psci->pos < 0) psci->pos = 0;
    s = 0;
  }
  else if (strcmp(name, "SLIDER") == 0)
  {
    va_list argptr;
    pifw->type = INPFOC_WND_SLIDER;
    va_start(argptr, pframe);
    psli = va_arg(argptr, slider_info_t *);
    va_end(argptr);
    if (sizeof(slider_info_t) > sizeof(((inpfoc_wnd_t*)(0))->caption))
    {
      error("%s: memory overlap\n", __FUNCTION__);
      while(1);
    }
    *((slider_info_t*)pifw->caption) = *psli;
    psli->pos %= psli->range;
    if (psli->pos < 0) psli->pos = 0;
    s = 0;
  }
  else if (strcmp(name, "PROGRESSBAR") == 0)
  {
    pifw->type = INPFOC_WND_PROGRESSBAR;
    ppbi = (progressbar_info_t*)pifw->caption;
    ppbi->progress_type = 0;
    ppbi->pos = 0;
    ppbi->range = 100;
   // *((progressbar_info_t*)pifw->caption) = *ppbi;
  }
  else
  {
    pifw->type = INPFOC_WND_LABEL;
  }

  if (s) strcpy(pifw->caption, s);
  else assert(pifw->ids);

  pifw->bitblt_func = bitblt_func;
  pifw->pframe = pframe;

  pifw->fgcolor = pifwd->fgcolor;
  pifw->bkcolor = pifwd->bkcolor;
  if ((pifw->bkcolor & 0x00ffffff) == 0) pifw->transp = 1;

  inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);

  return pifw;
}

void inpfoc_wnd_destroy(inpfoc_item_t * pifi)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);

  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);
/*
  grph_fillrect(pifw->bkfd, pifw->x, pifw->y, pifw->cx, pifw->cy, RGB(0,0,0));
  grph_fillrect(pifw->upfd, pifw->x, pifw->y, pifw->cx, pifw->cy, RGB(0,0,0));

  pifw->bitblt_func(pifw->x, pifw->y, pifw->cx, pifw->cy, pifw->pframe);
*/
  free(pifw);
}

void inpfoc_wnd_select(inpfoc_item_t * pifi)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);

  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  inpfoc_wnd_draw(pifw, pifw->upfd, STATE_ACTIVE);
}

void inpfoc_wnd_press(inpfoc_item_t * pifi)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);

  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  inpfoc_wnd_draw(pifw, pifw->upfd, STATE_SELECT);
}

void inpfoc_wnd_deselect(inpfoc_item_t * pifi)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);

  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  grph_fillrect(pifw->upfd, pifw->x, pifw->y, pifw->cx, pifw->cy, RGB(0,0,0));
 // inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);

  pifw->bitblt_func(pifw->x, pifw->y, pifw->cx, pifw->cy, pifw->pframe);
}

void inpfoc_wnd_check(inpfoc_item_t * pifi, unsigned int state)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;

  assert(pifw);

  if (!(pifw->type == INPFOC_WND_CHECKBOX || pifw->type == INPFOC_WND_RADIOBUTTON))
    return;

  if (state == IWC_TOGGLED)
    pifw->checked = !pifw->checked;
  else
    pifw->checked = (state == IWC_CHECKED) ? 1 : 0;

  inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);
  if (pifi->active)
  {
    inpfoc_wnd_draw(pifw, pifw->upfd, STATE_ACTIVE);
  }
}

void inpfoc_wnd_scroll(inpfoc_item_t * pifi, short delta)
{
 // rect_t rc;
  inpfoc_wnd_t * pifw;
//  scrollbar_info_t * psci;
  slider_info_t * psli;
//  rect_t rect2;
//  int v;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;

  if (!(pifw->type == INPFOC_WND_SCROLLBAR || pifw->type == INPFOC_WND_SLIDER))
    return;

/*  rc.x0 = pifw->x;
  rc.y0 = pifw->y;
  rc.x1 = rc.x0 + pifw->cx;
  rc.y1 = rc.y0 + pifw->cy;

  grph_fillrect(pifw->upfd, pifw->x, pifw->y, pifw->cx, pifw->cy, RGB(0,0,0));
*/
  switch (pifw->type)
  {
    case INPFOC_WND_SCROLLBAR:
      break;
    case INPFOC_WND_SLIDER:
      psli = (slider_info_t*)pifw->caption;
    /*  grph_fillrect(pifw->bkfd, pifw->x, pifw->y, pifw->cx, pifw->cy, RGB(0,0,0));
      if (psli->slider_type == SLIDER_TYPE_HORZ && psli->range)
      {
        grph_fillrect(pifw->bkfd, pifw->x, pifw->y + pifw->cy / 2 - 1, pifw->cx, 2, INPFOC_COLOR_DEF);
      }*/
      psli->pos += delta;
      if (psli->pos < 0) psli->pos = 0;
      if (psli->pos > psli->range) psli->pos = psli->range;
      inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);
      if (pifi->selected)
        inpfoc_wnd_draw(pifw, pifw->upfd, STATE_SELECT);
/*
      if (psli->slider_type == SLIDER_TYPE_HORZ && psli->range)
      {
        rect2.x0 = pifw->x + (int) ((double)psli->pos / psli->range * pifw->cx) - 2;
        rect2.y0 = pifw->y + 1 + 1;
        rect2.x1 = pifw->x + (int) ((double)psli->pos / psli->range * pifw->cx) + 2;
        rect2.y1 = pifw->y + pifw->cy - 1 - 1;
        if (rect2.x0 < pifw->x)
        {
          rect2.x0 = pifw->x;
          rect2.x1 += 2;
        }
        if (rect2.x1 > pifw->x + pifw->cx)
        {
          rect2.x1 = pifw->x + pifw->cx;
          rect2.x0 = pifw->x + pifw->cx - 4;
        }
        grph_fillrect(pifw->bkfd, rect2.x0, rect2.y0, rect2.x1-rect2.x0, rect2.y1-rect2.y0, INPFOC_COLOR_DEF);
      }
      if (psli->slider_type == SLIDER_TYPE_VERT && psli->range)
      {
        debug("%s: line %s - not implemented\n", __FILE__, __LINE__);
        //grph_fillrect(pifw->bkfd, rc2.x0, rc2.y0, rc2.x1-rc2.x0, rc2.y1-rc2.y0, INPFOC_COLOR_DEF);
      }

      if (pifi->selected)
        inpfoc_wnd_press(pifi);
      else if (pifi->active)
        inpfoc_wnd_select(pifi);
      break;*/
  }
//  pifw->bitblt_func(pifw->x, pifw->y, pifw->cx, pifw->cy, pifw->pframe);
}

void inpfoc_wnd_move(inpfoc_item_t * pifi, int x, int y, int clrOld)
{
  int old_x, old_y;
  rect_t rc;
  inpfoc_wnd_t * pifw;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  old_x = pifw->x;
  old_y = pifw->y;
  if (x >= 0) pifw->x = x;
  if (y >= 0) pifw->y = y;
 /* if (old_x == pifw->x && old_y == pifw->y)
    return;
*/
  if (clrOld)
  {
    rc.x0 = old_x;
    rc.y0 = old_y;
    rc.x1 = old_x + pifw->cx;
    rc.y1 = old_y + pifw->cy;
    grph_fillrect(pifw->bkfd, rc.x0, rc.y0, pifw->cx, pifw->cy, pifw->bkcolor);
    grph_fillrect(pifw->upfd, rc.x0, rc.y0, pifw->cx, pifw->cy, 0x000000);
    pifw->bitblt_func(rc.x0, rc.y0, pifw->cx, pifw->cy, pifw->pframe);
  }

  inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);
}

void inpfoc_wnd_size(inpfoc_item_t * pifi, int cx, int cy, int clrOld)
{
  int old_cx, old_cy;
  rect_t rc;
 // HBRUSH hbr;
  inpfoc_wnd_t * pifw;
 // scrollbar_info_t * psi;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  old_cx = pifw->cx;
  old_cy = pifw->cy;
  if (cx >= 0) pifw->cx = cx;
  if (cy >= 0) pifw->cy = cy;
 /* if (old_x == pifw->x && old_y == pifw->y)
    return;
*/
  if (clrOld)
  {
    rc.x0 = pifw->x;
    rc.y0 = pifw->y;
    rc.x1 = pifw->x + old_cx;
    rc.y1 = pifw->y + old_cy;
    grph_fillrect(pifw->bkfd, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
    grph_fillrect(pifw->upfd, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
    pifw->bitblt_func(rc.x0, rc.y0, pifw->cx, pifw->cy, pifw->pframe);
  }

  inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);
#if 0
  rc.x0 = pifw->x;
  rc.y0 = pifw->y;
  rc.x1 = rc.x0 + pifw->cx;
  rc.y1 = rc.y0 + pifw->cy;
  grph_setfgcolor(pifw->bkfd, pifw->fgcolor);
  grph_setbkcolor(pifw->bkfd, pifw->bkcolor);
  grph_setfont(pifw->bkfd, LABEL_FONT);
//printf("inpfoc_wnd_move: %s %d %d   %d %d\n", pifw->caption, pifw->x, pifw->y, x, y);
 // ExtTextOutU(pifw->bkdc, pifw->x, pifw->y, 0, NULL, (LPCWSTR)pifw->caption, -1, NULL, NULL);
  switch (pifw->type)
  {
    case INPFOC_WND_LABEL:
    case INPFOC_WND_LIST:
    case INPFOC_WND_SPINBUTTON:
      if (!pifw->transp)
        grph_fillrect(pifw->bkfd, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, INPFOC_COLOR_DEF);
      grph_setfgcolor(pifw->bkfd, pifw->fgcolor);
      grph_setbkcolor(pifw->bkfd, pifw->bkcolor);
      grph_setfont(pifw->bkfd, LABEL_FONT);
      grph_drawtext(pifw->bkfd, pifw->caption, -1, &rc, DT_CENTER);
      break;
    case INPFOC_WND_RADIOBUTTON:
    case INPFOC_WND_CHECKBOX:
      break;
    case INPFOC_WND_SCROLLBAR:
#if 0
      SelectObject(pifw->bkdc, pifw->hbkbmp);
     // if (!pifw->transp)
     //   FillRect(pifw->bkdc, &rc, hBrDef);
      psi = (scrollbar_info_t*)pifw->caption;
      SelectObject(pifw->bkdc, hPenDef);
      MoveToEx(pifw->bkdc, rc.left, rc.top, NULL);
      LineTo(pifw->bkdc, rc.right-1, rc.top);
      LineTo(pifw->bkdc, rc.right-1, rc.bottom-1);
      LineTo(pifw->bkdc, rc.left, rc.bottom-1);
      LineTo(pifw->bkdc, rc.left, rc.top);
//printf("scrollbar %d %d %d\n", psi->scroll_type, psi->range, psi->pos);
      if (psi->scroll_type == SCROLLBAR_TYPE_HORZ)
      {
        POINT pt[3];
        SetTextColor(pifw->bkdc, INPFOC_COLOR_DEF);
        SetBkColor(pifw->bkdc, pifw->bkcolor);
        MoveToEx(pifw->bkdc, rc.left+pifw->cy, rc.top, NULL);
        LineTo(pifw->bkdc, rc.left+pifw->cy, rc.bottom);
        MoveToEx(pifw->bkdc, rc.right-1-pifw->cy, rc.top, NULL);
        LineTo(pifw->bkdc, rc.right-1-pifw->cy, rc.bottom);
        SelectObject(pifw->bkdc, hBrDef);
        pt[0].x = rc.left+pifw->cy-3; pt[0].y = rc.top+1;
        pt[1].x = rc.left+pifw->cy-3; pt[1].y = rc.bottom-1-1;
        pt[2].x = rc.left+3;          pt[2].y = rc.top+pifw->cy/2;
        Polygon(pifw->bkdc, pt, 3);
        SelectObject(pifw->bkdc, hBrDef);
        pt[0].x = rc.right-pifw->cy+2; pt[0].y = rc.top;
        pt[1].x = rc.right-pifw->cy+2; pt[1].y = rc.bottom-1-1;
        pt[2].x = rc.right-3;          pt[2].y = rc.top+pifw->cy/2;
        Polygon(pifw->bkdc, pt, 3);

        if (psi->range)
        {
          v = (pifw->cx - 2*pifw->cy - 4) / psi->range;
          if (v < 10) v = 10;
          rc2.left = pifw->cy + rc.left+2 + ((double)pifw->cx - 2*pifw->cy - 4) * psi->pos / (psi->range);
          rc2.top = rc.top + 2;
          rc2.right = rc2.left+v;
          rc2.bottom = rc.bottom - 2;
        }
        else
        {
          rc2.left = pifw->cy + pifw->x + 2;
          rc2.top = rc.top + 2;
          rc2.right = rc.right - pifw->cy - 2;
          rc2.bottom = rc.bottom - 2;
        }
      }
      if (psi->scroll_type == SCROLLBAR_TYPE_VERT)
      {
        POINT pt[3];
        SetTextColor(pifw->bkdc, INPFOC_COLOR_DEF);
        SetBkColor(pifw->bkdc, pifw->bkcolor);
        MoveToEx(pifw->bkdc, rc.left, rc.top+pifw->cx, NULL);
        LineTo(pifw->bkdc, rc.left+pifw->cx,rc.top+pifw->cx);
        MoveToEx(pifw->bkdc, rc.left, rc.bottom-1-pifw->cx, NULL);
        LineTo(pifw->bkdc, rc.left+pifw->cx, rc.bottom-1-pifw->cx);
        SelectObject(pifw->bkdc, hBrDef);
        pt[0].x = rc.left+pifw->cx/2; pt[0].y = rc.top+3;
        pt[1].x = rc.left+2;          pt[1].y = rc.top+pifw->cx-3;
        pt[2].x = rc.left+pifw->cx-3; pt[2].y = rc.top+pifw->cx-3;
        Polygon(pifw->bkdc, pt, 3);
        SelectObject(pifw->bkdc, hBrDef);
        pt[0].x = rc.left+pifw->cx/2; pt[0].y = rc.bottom-2;
        pt[1].x = rc.left+2; pt[1].y = rc.bottom+2-pifw->cx;
        pt[2].x = rc.right-3;          pt[2].y = rc.bottom+2-pifw->cx;
        Polygon(pifw->bkdc, pt, 3);

        if (psi->range)
        {
          v = (pifw->cy - 2*pifw->cx - 4) / psi->range;
          if (v < 10) v = 10;
          rc2.left = rc.left+2;
          rc2.top = pifw->cx + rc.top+2 + ((double)pifw->cy - 2*pifw->cx - 4) * psi->pos / (psi->range);
          rc2.right = rc.right-2;
          rc2.bottom = rc2.top + v;
        }
        else
        {
          rc2.left = rc.left+2;
          rc2.top = pifw->cx + rc.top+2;
          rc2.right = rc.right-2;
          rc2.bottom = rc.bottom - pifw->cx - 2;
        }
      }
      FillRect(pifw->bkdc, &rc2, hBrDef);
#endif
      break;
  }

  if (pifi->selected)
  {
    inpfoc_wnd_press(pifi);
    return;
  }
  else if (pifi->active)
  {
    inpfoc_wnd_select(pifi);
    return;
  }
  else
    pifw->bitblt_func(pifw->x, pifw->y, pifw->cx, pifw->cy, pifw->pframe);
#endif
}

void inpfoc_wnd_resize(inpfoc_item_t * pifi, double mx, double my)
{
}

void inpfoc_wnd_setbkcolor(inpfoc_item_t * pifi, COLORREF color)
{
  inpfoc_wnd_t * pifw;
  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  pifw->bkcolor = color;
}

void inpfoc_wnd_setcaption(inpfoc_item_t * pifi, char *s)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;

  assert(pifw);
  if (s)
  {
    int caption_max_length = sizeof(((inpfoc_wnd_t*)(0))->caption);
    memcpy(pifw->caption, s, caption_max_length-1);
    pifw->caption[caption_max_length-1] = '\0';
  }

  inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);
  if (pifi->selected)
  {
    inpfoc_wnd_draw(pifw, pifw->upfd, STATE_SELECT);
  }
  else
  if (pifi->active)
  {
    inpfoc_wnd_draw(pifw, pifw->upfd, STATE_ACTIVE);
  }
  else
  if (pifi->disabled)
  {
    inpfoc_wnd_draw(pifw, pifw->upfd, STATE_DISABLED);
  }
}

void inpfoc_wnd_getcaption(inpfoc_item_t * pifi, char *s, int len)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  if (!s) return;

  memcpy(s, pifw->caption, min(len, sizeof(((inpfoc_wnd_t*)(0))->caption)) );
}

void inpfoc_wnd_setids(inpfoc_item_t * pifi, unsigned short ids)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  pifw->ids = ids;

  inpfoc_wnd_setcaption(pifi, NULL);
/*
  inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);
  if (pifi->selected)
  {
    inpfoc_wnd_draw(pifw, pifw->upfd, STATE_SELECT);
  }
  else
  if (pifi->active)
  {
    inpfoc_wnd_draw(pifw, pifw->upfd, STATE_ACTIVE);
  }
  else
  if (pifi->disabled)
  {
    inpfoc_wnd_draw(pifw, pifw->upfd, STATE_DISABLED);
  }
*/
}

int  inpfoc_wnd_getchecked(inpfoc_item_t * pifi)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  return pifw->checked;
}

void inpfoc_wnd_setrange(inpfoc_item_t * pifi, unsigned short range)
{
  inpfoc_wnd_t * pifw;
  slider_info_t * psli;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  if (!(pifw->type == INPFOC_WND_SCROLLBAR || pifw->type == INPFOC_WND_SLIDER))
    return;

  psli = (slider_info_t*)pifw->caption;
  psli->range = range;

  inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);
}

void inpfoc_wnd_setpos(inpfoc_item_t * pifi, unsigned short pos)
{
  inpfoc_wnd_t * pifw;
  slider_info_t * psli;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  if (!(pifw->type == INPFOC_WND_SCROLLBAR || pifw->type == INPFOC_WND_SLIDER))
    return;

  psli = (slider_info_t*)pifw->caption;
  psli->pos = pos;

  inpfoc_wnd_draw(pifw, pifw->bkfd, STATE_NONE);
}

void inpfoc_wnd_enable(inpfoc_item_t * pifi)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  inpfoc_wnd_draw(pifw, pifw->upfd, STATE_NONE);
}

void inpfoc_wnd_disable(inpfoc_item_t * pifi)
{
  inpfoc_wnd_t * pifw;

  assert(pifi);
  pifw = (inpfoc_wnd_t*)pifi->parg;
  assert(pifw);

  inpfoc_wnd_draw(pifw, pifw->upfd, STATE_DISABLED);
}

