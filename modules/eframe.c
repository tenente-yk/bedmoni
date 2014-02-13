#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include "bedmoni.h"
#include "dview.h"
#include "stconf.h"
#include "sframe.h"
#include "mframe.h"
#include "cframe.h"
#include "iframe.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "ids.h"
#include "evlogmenu.h"
#include "elog.h"
#include "dio.h"
#include "alarm.h"
#include "grph.h"
#include "eframe.h"

static eframe_t eframe;

static void eframe_on_create(void);
static void vposition_marker(eframe_t * peframe, float vpos); // vpos = 0-:-1

typedef struct
{
  unsigned int   color;
  unsigned short desc_ids;
} eldata_t;

static eldata_t eldata[EL_NUMTYPES] =
{
  {RGB(0x99,0x99,0x99), IDS_UPDATE}, // update
  {RGB(0xFF,0xFF,0xFF),  IDS_UNDEF7}, // alarm
};

static void eframe_bitblt(eframe_t * peframe, int x, int y, int cx, int cy)
{
  dc_list_t dclist;

  assert(peframe);

  dclist.xres = ((PGDC)(peframe->bkdc))->xres;
  dclist.yres = ((PGDC)(peframe->bkdc))->yres;
  dclist.p_bk = ((PGDC)(peframe->bkdc))->addr;
  dclist.p_fg = ((PGDC)(peframe->fgdc))->addr;
  dclist.p_up = ((PGDC)(peframe->updc))->addr;

  if (peframe->x+x >= EFRAME_CX) return;

  if (x + cx >= peframe->cx) cx = peframe->cx-x;
  if (y + cy >= peframe->cy) cy = peframe->cy-y;

  dview_bitblt(peframe->x+x, peframe->y+y, cx, cy, &dclist, x, y);
}

static void frame_bitblt(int x, int y, int cx, int cy, void *parg)
{
  eframe_t * peframe = &eframe;
  assert(peframe);
  eframe_bitblt(peframe, x, y, cx, cy);
}

void eframe_ini_def(void)
{
  // zeroing
}

void eframe_init(void)
{
  eframe_t * peframe = &eframe;

  memset(peframe, 0, sizeof(eframe_t));

  peframe->visible = 0;
  peframe->x  = EFRAME_X0;
  peframe->y  = EFRAME_Y0;
  peframe->cx = EFRAME_CX;
  peframe->cy = EFRAME_CY/* + SFRAME_CY*/;

  peframe->bkdc = grph_createdc(rootwfd, 0, 0, peframe->cx, peframe->cy, 0);
  peframe->fgdc = grph_createdc(rootwfd, 0, 0, peframe->cx, peframe->cy, 0);
  peframe->updc = grph_createdc(rootwfd, 0, 0, peframe->cx, peframe->cy, 0);

  peframe->num_cmds_in_queue = 0;

  peframe->offset = 0;
  peframe->font = EFRAME_FONT_NORMAL;

  eframe_ini_def();

  eframe_cfg_t eframe_cfg;
  if ( stconf_read(STCONF_EFRAME, (char*)&eframe_cfg, sizeof(eframe_cfg_t)) > 0 )
  {
    peframe->visible = eframe_cfg.visible;
    peframe->font = eframe_cfg.font;
  }

  if (peframe->visible)
  {
   // eframe_on_create();
    dview_frame_active(EFRAME);
  }
}

void eframe_deinit(void)
{
  eframe_t * peframe = &eframe;

  eframe_cfg_t eframe_cfg;

  eframe_cfg.visible = peframe->visible;
  eframe_cfg.font = peframe->font;
  stconf_write(STCONF_EFRAME, (char*)&eframe_cfg, sizeof(eframe_cfg_t));

  inpfoc_rm(INPFOC_EVLOG);

  grph_releasedc(peframe->bkdc);
  grph_releasedc(peframe->fgdc);
  grph_releasedc(peframe->updc);
}

static void eframe_on_destroy(void)
{
  eframe_t * peframe = &eframe;
  uframe_t * puframe = uframe_getptr();

  assert(puframe);

  iframe_command(IFRAME_SET_MODE, (void*)IFRAME_MODE_MONITOR);

  if (!peframe->visible) return;

  inpfoc_rm(INPFOC_EVLOG);
  inpfoc_change(INPFOC_MAIN);
  puframe->inpfoc_prev = INPFOC_MAIN;

  peframe->visible = 0;

  eframe_cfg_save();
}

static void eframe_on_draw(void)
{
  eframe_t * peframe = &eframe;
  rect_t rc, rc2;
  int y, offs, h;
  elog_data_t eld;
  char s[200], s2[200];
  int ids, color;

  rc.x0 = 0;
  rc.y0 = 0;
  rc.x1 = rc.x0 + peframe->cx;
  rc.y1 = rc.y0 + peframe->cy-EVLOG_BUTTON_CY-1;

  grph_fillrect(peframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(peframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(peframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  // no need return focus, refreshed display and inpfoc are not overlapped
 // inpfoc_focus(INPFOC_EFRAME);

  grph_setfont(peframe->bkdc, peframe->font);

  memcpy(&rc2, &rc, sizeof(rect_t));
  grph_drawtext(peframe->bkdc, "h", -1, &rc2, DT_CALCRECT);
  h = rc2.y1-rc2.y0;

  grph_setfgcolor(peframe->bkdc, RGB(0xDD,0xDD,0xDD));
  grph_line(peframe->bkdc, 10, 15-1, DISPLAY_CX-MFRAME_CX-50, 15-1);
  grph_line(peframe->bkdc, DISPLAY_CX-MFRAME_CX-50, 15-1, DISPLAY_CX-MFRAME_CX-50, 15+h);
  grph_line(peframe->bkdc, 10, 15+h, DISPLAY_CX-MFRAME_CX-50, 15+h);
  grph_line(peframe->bkdc, 10, 15-1, 10, 15+h);

  grph_setfgcolor(peframe->bkdc, 0xffffff);
  offs = peframe->offset;
  for (y = 15; y < peframe->cy-15-EVLOG_BUTTON_CY-h; y+=h, offs++)
  {
    if (elog_read(offs, 1, &eld) <= 0) break;

    switch (eld.type)
    {
      case EL_ALARM:
        if (eld.payload[1] == 1)
        {
          color = risk_color[alarm_msg[eld.payload[0]].level];
          ids = alarm_msg[eld.payload[0]].ids;
        }
        else
        {
          y -= 20;
          continue;
        }
        break;
      default:
        color = eldata[eld.type].color;
        ids = eldata[eld.type].desc_ids;
        break;
    }

    ids2string(ids, s2);
    grph_setfgcolor(peframe->bkdc, color);

    sprintf(s, "%02d.%02d.%4d %02d:%02d    %s\n", eld.trts.day, eld.trts.mon, eld.trts.year+1900, eld.trts.hour, eld.trts.min, s2);
    grph_textout(peframe->bkdc, 20, y, s, strlen(s));
  }

  unsigned long vscroll_range = elog_getsize();
  vposition_marker(peframe, (float)peframe->offset / ((vscroll_range) ? vscroll_range : 1));

  if (demo_mode && peframe->visible)
  {
    rect_t rect;
    char s[200];
    grph_setfont(peframe->updc, ARIAL_60);
    grph_setfgcolor(peframe->updc, RGB(0xEE,0xEE,0xEE));
    rect.x0 = 0;
    rect.x1 = rect.x0 + peframe->cx;
    rect.y0 = peframe->cy / 2 - 50;
    ids2string(IDS_DEMO_LARGE, s);
    grph_drawtext(peframe->updc, s, -1, &rect, DT_CENTER);
  }

  eframe_bitblt(peframe, 0, 0, peframe->cx, peframe->cy);
}

static void eframe_on_reload(void)
{
  eframe_t * peframe = &eframe;
  rect_t rc;

  if (!peframe->visible) return;

  rc.x0 = 0;
  rc.y0 = 0;
  rc.x1 = rc.x0+peframe->cx;
  rc.y1 = rc.y0+peframe->cy-EVLOG_BUTTON_CY-1;

  grph_fillrect(peframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(peframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(peframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  eframe_on_draw();
}

void eframe_on_resize(const int x, const int y, const int cx, const int cy)
{
  eframe_t * peframe = &eframe;
 /* inpfoc_item_t * pifi;*/

  peframe->x = x;
  peframe->y = y;
  peframe->cx = cx;
  peframe->cy = cy;

  if (peframe->cx <= 0)
    peframe->cx += EFRAME_CX;
  if (peframe->cy <= 0)
    peframe->cy += EFRAME_CY;

 /* pifi = inpfoc_find(INPFOC_EVLOG, EVLOG_SET);
  if (pifi)
    inpfoc_wnd_move(pifi, peframe->cx-80-5-55-5, EFRAME_CY-EVLOG_BUTTON_CY-1, TRUE);
  pifi = inpfoc_find(INPFOC_EVLOG, EVLOG_EXIT);
  if (pifi)
    inpfoc_wnd_move(pifi, peframe->cx-55-5, EFRAME_CY-EVLOG_BUTTON_CY-1, TRUE);
*/
  if (!peframe->visible) return;
  eframe_on_reload();
}

static void eframe_on_vscroll(short delta)
{
  eframe_t * peframe = &eframe;
  unsigned long vscroll_range = elog_getsize();

  peframe->offset += delta;
  if (peframe->offset < 0) peframe->offset = 0;
  if (peframe->offset > vscroll_range - 1) peframe->offset = vscroll_range - 1;

  eframe_on_reload();
}

static void eframe_on_create(void)
{
  eframe_t * peframe;
  uframe_t * puframe;
  mframe_t * pmframe = mframe_getptr();
  inpfoc_wnd_dc_t ifwd;
  char s[200];

  peframe = &eframe;
  assert(peframe);

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

  peframe->x  = EFRAME_X0;
  peframe->y  = EFRAME_Y0;
  peframe->cx = EFRAME_CX;
  peframe->cy = EFRAME_CY;
  peframe->visible = 1;

  peframe->offset = 0;

  grph_fillrect(peframe->bkdc, 0, peframe->cy, peframe->cx-SFRAME_CX, SFRAME_CY, 0x000000);
  eframe_bitblt(peframe, 0, peframe->cy, peframe->cx-SFRAME_CX, SFRAME_CY);

  memset(&ifwd, 0, sizeof(inpfoc_wnd_dc_t));

  ifwd.bkdc = peframe->bkdc;
  ifwd.updc = peframe->updc;
  ifwd.fgcolor = 0x444444;
  ifwd.bkcolor = 0x020202;

  strcpy(s, ">>|");
  inpfoc_add(INPFOC_EVLOG, EVLOG_SEEK0, (void*) inpfoc_wnd_create("PUSH", peframe->cx-60-5-60-5-65-5-102-5-60-5,
  EFRAME_CY-EVLOG_BUTTON_CY-1, 60, EVLOG_BUTTON_CY, s, 0, &ifwd, frame_bitblt, peframe));

  memset(s, 0, sizeof(s));
 // *((unsigned short*)&s[0]) = MAKEWORD(0xd0, 0x83);
 // s[2] = s[3] = ' ';
 // *((unsigned short*)&s[4]) = MAKEWORD(0xd0, 0x84);
  s[0] = 0x86;
  s[1] = s[2] = ' ';
  s[3] = 0x87;
  inpfoc_add(INPFOC_EVLOG, EVLOG_VSCROLL, (void*) inpfoc_wnd_create("LIST", peframe->cx-60-5-65-5-102-5-60-5,
  EFRAME_CY-EVLOG_BUTTON_CY-1, 60, EVLOG_BUTTON_CY, s, 0, &ifwd, frame_bitblt, peframe));

  inpfoc_add(INPFOC_EVLOG, EVLOG_GOTO_TABLE, (void*) inpfoc_wnd_create("PUSH", peframe->cx-65-5-102-5-60-5,
  EFRAME_CY-EVLOG_BUTTON_CY-1, 65-1, EVLOG_BUTTON_CY, 0, IDS_TABLE, &ifwd, frame_bitblt, peframe));

  inpfoc_add(INPFOC_EVLOG, EVLOG_SET, (void*) inpfoc_wnd_create("PUSH", peframe->cx-102-5-60-5, EFRAME_CY-EVLOG_BUTTON_CY-1, 102-1, EVLOG_BUTTON_CY, 0, IDS_SETTINGS, &ifwd, frame_bitblt, peframe));

  inpfoc_add(INPFOC_EVLOG, EVLOG_EXIT, (void*) inpfoc_wnd_create("PUSH", peframe->cx-60-5, EFRAME_CY-EVLOG_BUTTON_CY-1, 60-1, EVLOG_BUTTON_CY, 0, IDS_QUIT, &ifwd, frame_bitblt, peframe));

  inpfoc_change(INPFOC_EVLOG);
  puframe->inpfoc_prev = INPFOC_EVLOG;

  iframe_command(IFRAME_SET_MODE, (void*)IFRAME_MODE_EVLOGS);

  eframe_on_reload();

  inpfoc_set(INPFOC_EVLOG, EVLOG_EXIT); // has to be after xxx_draw routine

  eframe_cfg_save();
}

static void eframe_on_newdata(void)
{
  eframe_t * peframe = eframe_getptr();
  if (peframe->offset != 0) return;
  eframe_on_reload();
}

static void vposition_marker(eframe_t * peframe, float vpos) // vpos = 0-:-1
{
  rect_t rc, rc2;

  rc.x0 = DISPLAY_CX-MFRAME_CX-20-20;
  rc.x1 = rc.x0 + 20;
  rc.y0 = 20;
  rc.y1 = EFRAME_CY-EVLOG_BUTTON_CY-20;

  grph_fillrect(peframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  // borders
  grph_setfgcolor(peframe->bkdc, INPFOC_COLOR_DEF);
  grph_line(peframe->bkdc, rc.x0, rc.y0, rc.x1, rc.y0);
  grph_line(peframe->bkdc, rc.x1, rc.y0, rc.x1, rc.y1);
  grph_line(peframe->bkdc, rc.x1, rc.y1, rc.x0, rc.y1);
  grph_line(peframe->bkdc, rc.x0, rc.y1, rc.x0, rc.y0);

  // position marker
  rc2.x0 = rc.x0+2;
  rc2.x1 = rc.x1-1;
  rc2.y0 = rc.y0+2+vpos*(rc.y1-rc.y0+1-18);
  rc2.y1 = rc2.y0+18;
  grph_fillrect(peframe->bkdc, rc2.x0, rc2.y0, rc2.x1-rc2.x0, rc2.y1-rc2.y0, INPFOC_COLOR_DEF);
}

void eframe_show(void)
{
  uframe_command(UFRAME_DESTROY, NULL);
  eframe_command(EFRAME_CREATE, NULL);
}

void eframe_hide(void)
{
  eframe_command(EFRAME_DESTROY, NULL);
}

void eframe_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case EFRAME_RESIZE:
      dview_command(EFRAME, cmd, 1, arg);
      break;
    default:
      dview_command(EFRAME, cmd, 0, arg);
      break;
  }
}

void eframe_on_command(int cmd, void * arg)
{
    switch (cmd)
    {
      case EFRAME_CREATE:
        eframe_on_create();
        break;
      case EFRAME_DESTROY:
        eframe_on_destroy();
        break;
      case EFRAME_RESIZE:
        error("%s: line %d - add implementation\n", __FILE__, __LINE__);
        //eframe_on_resize(...);
        break;
      case EFRAME_RELOAD:
        eframe_on_reload();
        break;
      case EFRAME_NEWDATA:
        eframe_on_newdata();
        break;
      case EFRAME_VSCROLL:
        eframe_on_vscroll(*((int*)arg));
        break;
      default:
        error("%s: unknown cmd %d\n", __FUNCTION__, cmd);
        break;
    }
}

void eframe_cfg_save(void)
{
  eframe_t * peframe = &eframe;
  eframe_cfg_t eframe_cfg;

  eframe_cfg.visible = peframe->visible;
  stconf_write(STCONF_EFRAME, (char*)&eframe_cfg, sizeof(eframe_cfg_t));
}

eframe_t * eframe_getptr(void)
{
  return &eframe;
}
