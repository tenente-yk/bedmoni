#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bedmoni.h"
#include "dview.h"
#include "grph.h"
#include "cframe.h"
#include "iframe.h"
#include "mframe.h"
#include "tframe.h"
#include "lframe.h"
#include "menucs.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "dialog.h"
#include "lang.h"
#include "uframe.h"

#include "sched.h"
void restart_data(void);
extern volatile int stop_fast_data_flag;

static uframe_t uframe;

static void uframe_on_destroy_ex(int);
static void uframe_on_destroy(void);

static void frame_bitblt(int x, int y, int cx, int cy, void * parg)
{
  uframe_t * puframe;
  dc_list_t dclist;

  puframe = (uframe_t*) parg;
  assert(puframe);

  dclist.xres = ((PGDC)(puframe->bkdc))->xres;
  dclist.yres = ((PGDC)(puframe->bkdc))->yres;
  dclist.p_bk = ((PGDC)(puframe->bkdc))->addr;
  dclist.p_fg = ((PGDC)(puframe->fgdc))->addr;
  dclist.p_up = ((PGDC)(puframe->updc))->addr;

  if (x + cx >= puframe->cx) cx = puframe->cx - x;
  if (y + cy >= puframe->cy) cy = puframe->cy - y;

  dview_bitblt(puframe->x+x, puframe->y+y, cx, cy, &dclist, x, y);
}

void uframe_init(void)
{
  uframe_t * puframe = &uframe;

  puframe->x = UFRAME_X0;
  puframe->y = UFRAME_Y0;
  puframe->cx = UFRAME_CX;
  puframe->cy = UFRAME_CY;
  puframe->pmcs = (menucs_t*) calloc(sizeof(menucs_t), 1);
  if (!puframe->pmcs)
  {
    error("pmcs == NULL\n");
    while(1);
  }

  puframe->bkdc = grph_createdc(rootwfd, 0, 0, puframe->cx, puframe->cy, 0);
  puframe->fgdc = grph_createdc(rootwfd, 0, 0, puframe->cx, puframe->cy, 0);
  puframe->updc = grph_createdc(rootwfd, 0, 0, puframe->cx, puframe->cy, 0);

  puframe->num_cmds_in_queue = 0;

 // uframe_command(UFRAME_CREATE, (void*) MENU_ISNEWPATIENT);
/*  mb_yes_no_t mbyn;
  mbyn.on_yes_func = yes_func;
  mbyn.on_no_func = no_func;
  strcpy(mbyn.caption, "New patient");
  uframe_command(UFRAME_MB_YES_NO, (void*) &mbyn);
*/
 // uframe_command(UFRAME_MB_OK, "Hello from bedmoni monitor!!!");
}

void uframe_deinit(void)
{
  uframe_t * puframe = &uframe;

  uframe_on_destroy_ex(FALSE);

  free(puframe->pmcs);

  grph_releasedc(puframe->bkdc);
  grph_releasedc(puframe->fgdc);
  grph_releasedc(puframe->updc);
}

static void uframe_on_create_ex(int id, int create_frame)
{
  uframe_t * puframe;
  rect_t rc;
  inpfoc_wnd_dc_t ifwd;
  int ifid = -1;
  void * pv;
  slider_info_t slider_info;
  char s[200];
  int i;

  if (create_frame)
  {
    // destroy opened menu
    uframe_on_destroy();

    cframe_on_resize(CFRAME_X0+UFRAME_CX, CFRAME_Y0, CFRAME_CX-UFRAME_CX, CFRAME_CY);
    if (mframe_ismaximized())
    {
      mframe_on_resize(0+UFRAME_CX, MFRAME_Y0, DISPLAY_CX-UFRAME_CX, MFRAME_CY);
    }

    for (i=0; i<NUM_FRAMES; i++)
    {
      if (frame_interface[i].resize)
        frame_interface[i].resize(UFRAME_CX, IFRAME_CY, -UFRAME_CX, 0);
    }
  }

  puframe = &uframe;

  puframe->visible = 1;

  puframe->id = id;

  puframe->x = UFRAME_X0;
  puframe->y = UFRAME_Y0;
  puframe->cx = UFRAME_CX;
  puframe->cy = UFRAME_CY;

  rc.x0 = 0;
  rc.y0 = 0;
  rc.x1 = puframe->cx;
  rc.y1 = puframe->cy;

  grph_fillrect(puframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, UFRAME_BKCOLOR);
  grph_fillrect(puframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(puframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  // draw border
  grph_setfgcolor(puframe->bkdc, RGB(0x44,0x44,0x44));
  grph_fillrect(puframe->bkdc, 0, 1, 1, puframe->cy, RGB(0x44,0x44,0x44));
  grph_fillrect(puframe->bkdc, puframe->cx-1, 0, 1, puframe->cy, RGB(0x44,0x44,0x44));
  grph_fillrect(puframe->bkdc, 0, 0, puframe->cx, 1, RGB(0x44,0x44,0x44));
 // grph_fillrect(puframe->bkdc, 0, puframe->cy-1-1, puframe->cx, 1, RGB(0x44,0x44,0x44));
  grph_line(puframe->bkdc, 0, puframe->cy-1, puframe->cx, puframe->cy-1);

  if ( menucs_load(id, puframe->pmcs) == 0 )
  {
    menucs_item_lst_t * ptr = puframe->pmcs->pmil;
//printf("0, pmcs = %d\n", (int)puframe->pmcs);
    for (;ptr; ptr=ptr->next)
    {
//printf("=%d=", ptr->item->type);
      switch(ptr->item->type)
      {
        case MI_STATIC:
          grph_setbkcolor(puframe->bkdc, UFRAME_BKCOLOR);
          grph_setfgcolor(puframe->bkdc, UFRAME_STATICTEXT_COLOR);
          grph_setfont(puframe->bkdc, UFRAME_FONT_TEXT);
          rc.x0 = ptr->item->x;
          rc.y0 = ptr->item->y;
          rc.x1 = ptr->item->x+ptr->item->cx;
          rc.y1 = ptr->item->y+ptr->item->cy;
          ids2string(ptr->item->ids, s);
          grph_drawtext(puframe->bkdc, s, -1, &rc, 0);
          break;
        case MI_INPFOC:
        //  printf("%s %s %s %d %d %d %d\n", ptr->item->s, ptr->item->sid, ptr->item->sidi, ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy);
          ifwd.bkdc = puframe->bkdc;
          ifwd.updc = puframe->updc;
          ifwd.fgcolor = INPFOC_COLOR_TEXT;
          ifwd.bkcolor = INPFOC_COLOR_DEF;

//          inpfoc_add(ptr->item->ifid, ptr->item->ifit, (void*) new_inpfoc_wnd(ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy, ptr->item->s, &ifwd, frame_bitblt, puframe));
         // ids2string(ptr->item->ids, s);
          pv = inpfoc_wnd_create("INPFOC", ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy, 0, ptr->item->ids, &ifwd, frame_bitblt, puframe);

          inpfoc_add(ptr->item->ifid, ptr->item->ifit, pv);

          ifid = ptr->item->ifid;
          break;

        case MI_SPINBUTTON:
          ifwd.bkdc = puframe->bkdc;
          ifwd.updc = puframe->updc;
          ifwd.fgcolor = INPFOC_COLOR_TEXT;
          ifwd.bkcolor = INPFOC_COLOR_DEF;

         // ids2string(ptr->item->ids, s);
          pv = inpfoc_wnd_create("SPINBUTTON", ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy, 0, ptr->item->ids, &ifwd, frame_bitblt, puframe);

          inpfoc_add(ptr->item->ifid, ptr->item->ifit, pv);

          ifid = ptr->item->ifid;
          break;

        case MI_CHECKBOX:
          ifwd.bkdc = puframe->bkdc;
          ifwd.updc = puframe->updc;
          ifwd.fgcolor = UFRAME_STATICTEXT_COLOR;
          ifwd.bkcolor = UFRAME_BKCOLOR;

         // ids2string(ptr->item->ids, s);
          pv = inpfoc_wnd_create("CHECKBOX", ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy, 0, ptr->item->ids, &ifwd, frame_bitblt, puframe);

          inpfoc_add(ptr->item->ifid, ptr->item->ifit, pv);

          ifid = ptr->item->ifid;
          break;
        case MI_SLIDER:
          ifwd.bkdc = puframe->bkdc;
          ifwd.updc = puframe->updc;
          ifwd.fgcolor = UFRAME_STATICTEXT_COLOR;
          ifwd.bkcolor = UFRAME_BKCOLOR;

          memset(&slider_info, 0, sizeof(slider_info_t));

          slider_info.slider_type = SLIDER_TYPE_HORZ;
          slider_info.pos = 0;
          slider_info.range = 10;

         // ids2string(ptr->item->ids, s);
          pv = inpfoc_wnd_create("SLIDER", ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy, 0, ptr->item->ids, &ifwd, frame_bitblt, puframe, &slider_info);

          inpfoc_add(ptr->item->ifid, ptr->item->ifit, pv);

          ifid = ptr->item->ifid;
          break;

        case MI_LISTBUTTON:
          ifwd.bkdc = puframe->bkdc;
          ifwd.updc = puframe->updc;
          ifwd.fgcolor = INPFOC_COLOR_TEXT;
          ifwd.bkcolor = UFRAME_BKCOLOR;

         // ids2string(ptr->item->ids, s);
          pv = inpfoc_wnd_create("LIST", ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy, 0, ptr->item->ids, &ifwd, frame_bitblt, puframe);

          inpfoc_add(ptr->item->ifid, ptr->item->ifit, pv);

          ifid = ptr->item->ifid;
          break;

        case MI_RADIOBUTTON:
          ifwd.bkdc = puframe->bkdc;
          ifwd.updc = puframe->updc;
          ifwd.fgcolor = UFRAME_STATICTEXT_COLOR;
          ifwd.bkcolor = UFRAME_BKCOLOR;

         // ids2string(ptr->item->ids, s);
          pv = inpfoc_wnd_create("RADIOBUTTON", ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy, 0, ptr->item->ids, &ifwd, frame_bitblt, puframe);

          inpfoc_add(ptr->item->ifid, ptr->item->ifit, pv);

          ifid = ptr->item->ifid;
          break;

#if 0
        case MENUCS_TYPE_PROGRESSBAR:
          ifwd.lbfont = puframe->lbfont;
          ifwd.hwnd = NULL;
          ifwd.bkdc = puframe->bkdc;
          ifwd.updc = puframe->updc;
          ifwd.hbkbmp = puframe->hbkbmp;
          ifwd.hupbmp = puframe->hupbmp;
          ifwd.fgcolor = UFRAME_STATICTEXT_COLOR;
          ifwd.bkcolor = UFRAME_BKCOLOR;
          pv = inpfoc_wnd_create_("PROGRESSBAR", ptr->item->x, ptr->item->y, ptr->item->cx, ptr->item->cy, ptr->item->s, &ifwd, frame_bitblt, puframe, NULL);
          inpfoc_add(ptr->item->ifid, ptr->item->ifit, pv);
          break;
#endif
        default:
          error("%s: unrecognized item type %d\n", ptr->item->type);
          break;
      }
    }
  }
  else
  {
    error("failed to load menu\n");
  }

  if (puframe->pmcs->openproc) puframe->pmcs->openproc();

  assert(ifid < INPFOC_MAX_NUM);

  if (ifid != -1)
  {
    puframe->inpfoc_prev = inpfoc_getcurr();
    puframe->inpfoc = ifid;

    inpfoc_change(puframe->inpfoc);
  }

  frame_bitblt(0, 0, puframe->cx, puframe->cy, puframe);

  sched_start(SCHED_ANY, 120, restart_data, SCHED_DO_ONCE); // this is YK fix, sometimes app fails if when UFRAME_CREATE cmd is called
}

void restart_data(void)
{
  stop_fast_data_flag = 0;
}

static void uframe_on_destroy_ex(int destroy_frame)
{
  uframe_t * puframe;
  int i;

  puframe = &uframe;
  assert(puframe);

  if (puframe->visible == 0) return;

  if (puframe->pmcs->closeproc) puframe->pmcs->closeproc();

//debug("%s+\n", __FUNCTION__);

//debug("0\n", __FUNCTION__);
  if (puframe->inpfoc > 0)
  {
//debug("1 =%d\n", puframe->inpfoc_prev);
    inpfoc_change(puframe->inpfoc_prev);
//debug("2 =%d\n", puframe->inpfoc);
    inpfoc_rm(puframe->inpfoc);
  }
//debug("3\n", __FUNCTION__);
//debug("%d puframe->pmcs = %d\n", puframe->id, puframe->pmcs);
  menucs_unload(puframe->id, puframe->pmcs);
//debug("4\n", __FUNCTION__);

  puframe->visible = 0;

  if (destroy_frame)
  {
    rect_t rc;

    rc.x0 = 0;
    rc.y0 = 0;
    rc.x1 = puframe->cx;
    rc.y1 = puframe->cy;

    // clear background remained after uframe in a part of sframe
    grph_fillrect(puframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
    grph_fillrect(puframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
    grph_fillrect(puframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
    frame_bitblt(0, 0, puframe->cx, puframe->cy, puframe);

    cframe_on_resize(CFRAME_X0, CFRAME_Y0, CFRAME_CX, CFRAME_CY);
    if (mframe_ismaximized())
    {
      mframe_on_resize(0, MFRAME_Y0, DISPLAY_CX, MFRAME_CY);
    }

    for (i=0; i<NUM_FRAMES; i++)
    {
      if (frame_interface[i].resize)
        frame_interface[i].resize(0, IFRAME_CY, 0, 0);
    }

  }

//debug("%s-\n", __FUNCTION__);
}

static void uframe_on_create(int id)
{
  uframe_on_create_ex(id, TRUE);
}

static void uframe_on_destroy(void)
{
  uframe_on_destroy_ex(TRUE);
}

static void uframe_on_settext(stext_t * pst)
{
  uframe_t * puframe;
  rect_t rc;

  assert(pst);

  puframe = &uframe;
  assert(puframe);

  grph_setbkcolor(puframe->fgdc, UFRAME_BKCOLOR);
  grph_setfgcolor(puframe->fgdc, pst->color ? pst->color : UFRAME_STATICTEXT_COLOR);
  grph_setfont(puframe->fgdc, UFRAME_FONT_TEXT);
  rc.x0 = pst->x;
  rc.y0 = pst->y;
  rc.x1 = puframe->cx;
  rc.y1 = rc.y0+20;
  grph_drawtext(puframe->fgdc, pst->s, -1, &rc, DT_CALCRECT);
  grph_fillrect(puframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_drawtext(puframe->fgdc, pst->s, -1, &rc, 0);

  frame_bitblt(rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, puframe);
}

static void uframe_on_updatemenu(int menuid)
{
  int i;
  uframe_t * puframe;

  puframe = &uframe;
  assert(puframe);

  for (i=0; i<NUM_MENUS; i++)
  {
    if (menuid == menus[i].id)
    {
      menus[i].openproc();
      break;
    }
  }
}

static void uframe_on_change(int menuid)
{
  uframe_on_destroy_ex(FALSE);
  uframe_on_create_ex(menuid, FALSE);
}

static void uframe_on_mb_yes_no(void * arg)
{
  uframe_t * puframe;
  rect_t rc;
  inpfoc_wnd_dc_t ifwd;
  void * pv;
  mb_yes_no_t * ptr = (mb_yes_no_t*)arg;
  int i;

  assert(ptr);

 // debug("%s +\n", __FUNCTION__);

  if (1)
  {
    // destroy opened menu
    uframe_on_destroy();

    cframe_on_resize(CFRAME_X0+UFRAME_CX, CFRAME_Y0, CFRAME_CX-UFRAME_CX, CFRAME_CY);
    if (mframe_ismaximized())
    {
      mframe_on_resize(0+UFRAME_CX, MFRAME_Y0, DISPLAY_CX-UFRAME_CX, MFRAME_CY);
    }

    for (i=0; i<NUM_FRAMES; i++)
    {
      if (frame_interface[i].resize)
        frame_interface[i].resize(UFRAME_CX, IFRAME_CY, -UFRAME_CX, 0);
    }

  }

  puframe = &uframe;

  puframe->visible = 1;

  puframe->id = 0;

  rc.x0 = 0;
  rc.y0 = 0;
  rc.x1 = puframe->cx;
  rc.y1 = puframe->cy;

  grph_fillrect(puframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, UFRAME_BKCOLOR);
  grph_fillrect(puframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(puframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  // caption
  grph_setbkcolor(puframe->bkdc, UFRAME_BKCOLOR);
  grph_setfgcolor(puframe->bkdc, UFRAME_STATICTEXT_COLOR);
  grph_setfont(puframe->bkdc, UFRAME_FONT_TEXT);
  rc.x0 = 10;
  rc.y0 = 40;
  rc.x1 = 190;
  rc.y1 = 40+20;
  grph_drawtext(puframe->bkdc, ptr->caption, -1, &rc, 0);

  // 'yes' inpfoc
  ifwd.bkdc = puframe->bkdc;
  ifwd.updc = puframe->updc;
  ifwd.fgcolor = INPFOC_COLOR_TEXT;
  ifwd.bkcolor = INPFOC_COLOR_DEF;
 // ids2string(IDS_YES, s);
  pv = inpfoc_wnd_create("INPFOC", 20, 80, 60, 20, NULL, IDS_YES, &ifwd, frame_bitblt, puframe);
  inpfoc_add(INPFOC_DIALOG, DIALOG_YES, pv);

  for (i=0; i<1000; i++) // 1000 - dummy
  {
    if (inpfoc_dialog_funclist[i].fno == -1) break;
    if (inpfoc_dialog_funclist[i].fno == DIALOG_YES && ptr->on_yes_func) inpfoc_dialog_funclist[i].func = ptr->on_yes_func;
  }

  // 'no' inpfoc
  ifwd.bkdc = puframe->bkdc;
  ifwd.updc = puframe->updc;
  ifwd.fgcolor = INPFOC_COLOR_TEXT;
  ifwd.bkcolor = INPFOC_COLOR_DEF;
 // ids2string(IDS_NO, s);
  pv = inpfoc_wnd_create("INPFOC", 120, 80, 60, 20, NULL, IDS_NO, &ifwd, frame_bitblt, puframe);
  inpfoc_add(INPFOC_DIALOG, DIALOG_NO, pv);

  for (i=0; i<1000; i++) // 1000 - dummy
  {
    if (inpfoc_dialog_funclist[i].fno == -1) break;
    if (inpfoc_dialog_funclist[i].fno == DIALOG_NO && ptr->on_no_func) inpfoc_dialog_funclist[i].func = ptr->on_no_func;
  }

  puframe->inpfoc_prev = inpfoc_getcurr();
  puframe->inpfoc = INPFOC_DIALOG;

  inpfoc_change(puframe->inpfoc);

  frame_bitblt(0, 0, puframe->cx, puframe->cy, puframe);

 // debug("%s -\n", __FUNCTION__);
}

static void uframe_on_mb_ok(void * arg)
{
  uframe_t * puframe;
  rect_t rc;
  inpfoc_wnd_dc_t ifwd;
  void * pv;
  char * caption = (char*)arg;
  char s[100];
  int n, i;
  int xx;
  int yy;
  char * ps0;

  assert(caption);

  if (1)
  {
    // destroy opened menu
    uframe_on_destroy();

    cframe_on_resize(CFRAME_X0+UFRAME_CX, CFRAME_Y0, CFRAME_CX-UFRAME_CX, CFRAME_CY);
    if (mframe_ismaximized())
    {
      mframe_on_resize(0+UFRAME_CX, MFRAME_Y0, DISPLAY_CX-UFRAME_CX, MFRAME_CY);
    }

   // tframe_on_resize(TFRAME_X0+UFRAME_CX, TFRAME_Y0, TFRAME_CX-UFRAME_CX, TFRAME_CY);
   // lframe_on_resize(LFRAME_X0+UFRAME_CX, LFRAME_Y0, LFRAME_CX-UFRAME_CX, LFRAME_CY);

    for (i=0; i<NUM_FRAMES; i++)
    {
      if (frame_interface[i].resize)
        frame_interface[i].resize(UFRAME_CX, IFRAME_CY, -UFRAME_CX, 0);
    }

  }

  puframe = &uframe;

  puframe->visible = 1;

  puframe->id = 0;

  rc.x0 = 0;
  rc.y0 = 0;
  rc.x1 = puframe->cx;
  rc.y1 = puframe->cy;

  grph_fillrect(puframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, UFRAME_BKCOLOR);
  grph_fillrect(puframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(puframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  xx = 0;
  yy = 0;
  ps0 = caption;
  s[0] = 0;

  do
  {
    caption ++;
    xx += 12;

    if (*caption == ' ' || *caption == '\t' || *caption == 0)
    {
//printf("copy %s %d\n", ps0, caption-ps0+1);
      n = caption-ps0+1;
      strncat(s, ps0, n);
      ps0 = ps0 + n;
//printf("res %s, ps0 = %s, caption=%s\n", s, ps0, caption);
    }

    if (xx > 200 || *caption == 0)
    {
      // caption
      grph_setbkcolor(puframe->bkdc, UFRAME_BKCOLOR);
      grph_setfgcolor(puframe->bkdc, UFRAME_STATICTEXT_COLOR);
      grph_setfont(puframe->bkdc, UFRAME_FONT_TEXT);
      rc.x0 = 10;
      rc.y0 = 40 + yy;
      rc.x1 = 190;
      rc.y1 = 40+20;
      grph_drawtext(puframe->bkdc, s, -1, &rc, DT_CENTER);

//printf("print s = %s, xx = %d\n", s, xx);
      s[0] = 0;
      xx = 0;
      yy += 20;
    }
  } while (*caption);

  // 'ok' inpfoc
  ifwd.bkdc = puframe->bkdc;
  ifwd.updc = puframe->updc;
  ifwd.fgcolor = INPFOC_COLOR_TEXT;
  ifwd.bkcolor = INPFOC_COLOR_DEF;
  pv = inpfoc_wnd_create("INPFOC", 60, 40+yy+20, 80, 20, "OK", 0, &ifwd, frame_bitblt, puframe);
  inpfoc_add(INPFOC_DIALOG, DIALOG_OK, pv);
  inpfoc_set(INPFOC_DIALOG, DIALOG_OK);

  puframe->inpfoc_prev = inpfoc_getcurr();
  puframe->inpfoc = INPFOC_DIALOG;

  inpfoc_change(puframe->inpfoc);

  frame_bitblt(0, 0, puframe->cx, puframe->cy, puframe);
}

void uframe_on_clearrect(rect_t * prc)
{
  uframe_t * puframe;
  rect_t rc;

  rc = *prc;

  puframe = &uframe;
  assert(puframe);

#if 1
  grph_fillrect(puframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, UFRAME_BKCOLOR);
#endif

  grph_fillrect(puframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  frame_bitblt(rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, puframe);
}

void uframe_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case UFRAME_SETSTEXT:
      dview_command(UFRAME, cmd, sizeof(stext_t), arg);
      break;
    case UFRAME_MB_YES_NO:
      dview_command(UFRAME, cmd, sizeof(mb_yes_no_t), arg);
      break;
    case UFRAME_MB_OK:
      dview_command(UFRAME, cmd, DVIEW_ARG_DATA_SIZE, arg);
      break;
    case UFRAME_CLEARRECT:
      dview_command(UFRAME, cmd, sizeof(rect_t), arg);
      break;
    default:
      dview_command(UFRAME, cmd, 0, arg);
      break;
  }
  if (cmd == UFRAME_CREATE) stop_fast_data_flag = 1;
}

void uframe_on_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case UFRAME_CREATE:
      uframe_on_create(*((int*)arg));
      break;
    case UFRAME_DESTROY:
      uframe_on_destroy();
      break;
    case UFRAME_SETSTEXT:
      uframe_on_settext(arg);
      break;
    case UFRAME_CHANGE:
      uframe_on_change(*((int*)arg));
      break;
    case UFRAME_UPDATEMENU:
      uframe_on_updatemenu(*((int*)arg));
      break;
    case UFRAME_MB_YES_NO:
      uframe_on_mb_yes_no(arg);
      break;
    case UFRAME_MB_OK:
      uframe_on_mb_ok(arg);
      break;
    case UFRAME_CLEARRECT:
      uframe_on_clearrect(arg);
      break;
    default:
      error("%s: unknown cmd %d\n", __FUNCTION__, cmd);
      break;
  }
}

void uframe_printbox(short x, short y, short cx, short cy, char *s, COLORREF color)
{
  stext_t stext;
  memset(&stext, 0, sizeof(stext_t));
  stext.x = x;
  stext.y = y;
  stext.color = color;
  strcpy(stext.s, s);
  uframe_command(UFRAME_SETSTEXT, (void*)&stext);
}

void uframe_clearbox(short x, short y, short cx, short cy)
{
  rect_t rc;
  rc.x0 = x;
  rc.y0 = y;
  rc.x1 = x + cx;
  rc.y1 = y + cy;
  uframe_command(UFRAME_CLEARRECT, (void*)&rc);
}

uframe_t * uframe_getptr(void)
{
 return &uframe;
}

