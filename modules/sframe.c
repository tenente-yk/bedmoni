/*! \file sframe.c
    \brief Frame, containing status information
 */

#ifdef UNIX
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#include "bedmoni.h"
#include "grph.h"
#include "utils.h"
#include "dview.h"
#include "sframe.h"

#include "bat.cc"
#include "mains.cc"

static sframe_t sframe;
#ifdef SFRAME_BMONI_TS
static int vermode = 0;
#endif

static void sframe_bitblt(sframe_t * psframe, int x, int y, int cx, int cy);

void sframe_init(void)
{
  sframe_t * psframe;
  struct tm ltm;

  psframe = &sframe;

  memset(psframe, 0, sizeof(sframe_t));

  psframe->x    = SFRAME_X0;
  psframe->y    = SFRAME_Y0;
  psframe->cx   = SFRAME_CX;
  psframe->cy   = SFRAME_CY;

  psframe->num_cmds_in_queue = 0;

  psframe->bkdc = grph_createdc(rootwfd, 0, 0, psframe->cx, psframe->cy, 0);
  psframe->fgdc = grph_createdc(rootwfd, 0, 0, psframe->cx, psframe->cy, 0);
  psframe->updc = grph_createdc(rootwfd, 0, 0, psframe->cx, psframe->cy, 0);
  grph_fillrect(psframe->bkdc, 0, 0, psframe->cx, psframe->cy, 0x000000);
  grph_fillrect(psframe->fgdc, 0, 0, psframe->cx, psframe->cy, 0x000000);
  grph_fillrect(psframe->updc, 0, 0, psframe->cx, psframe->cy, 0x000000);

  read_hwclock_data(&ltm);
  sframe_command(SFRAME_SET_BATSTAT, (void*) 0x7f000000);
  sframe_command(SFRAME_SET_DATE, (struct tm*) &ltm);
  sframe_command(SFRAME_SET_TIME, (struct tm*) &ltm);

  grph_setfgcolor(psframe->bkdc, RGB(0x44,0x44,0x44));
  grph_line(psframe->bkdc, 0, 0, 0, psframe->cy);
 // grph_line(psframe->bkdc, 0, 0, psframe->cx, 0);

  sframe_bitblt(psframe, 0, 0, psframe->cx-1, psframe->cy);
}

void sframe_deinit(void)
{
  sframe_t * psframe;

  psframe = &sframe;

  grph_releasedc(psframe->bkdc);
  grph_releasedc(psframe->fgdc);
  grph_releasedc(psframe->updc);
}

static void sframe_on_set_batstat(int stat)
{
  sframe_t * psframe = &sframe;
  rect_t rc, rc2;
  COLORREF color = 0;
  int charge;

  // clear all bat area
  rc.x0 = 44+0+142;
  rc.y0 = 0+2;
  rc.x1 = rc.x0 + bat_image.width;
  rc.y1 = rc.y0 + 20 - 2;
  grph_filldata16(psframe->fgdc, rc.x0, rc.y0, bat_image.width, bat_image.height, (void*) bat_image.pixel_data);

  // charge
  charge = (stat>>24) & 0x7F;
  if (charge < 0 || charge > 100)
  {
    // 0x7F - undef state
    char s[] = "?";
    grph_setfgcolor(psframe->fgdc, RGB(0xFF,0xFF,0x00)/*SFRAME_FGCOLOR*/);
    grph_setbkcolor(psframe->fgdc, SFRAME_BKCOLOR);
    grph_setfont(psframe->fgdc, SFRAME_FONT);
   // grph_drawtext(psframe->fgdc, s, -1, &rc, DT_CENTER);
    grph_textout(psframe->fgdc, rc.x0+40, rc.y0, s, strlen(s));
   // stat &= ~(0x80); // clear mains
  }
  else
  {
    // fill bat charge
    rc2.x0 = rc.x0 + 4 + (100-charge) * (bat_image.width-4-2)/ 100;
    rc2.y0 = rc.y0 + 2;
    rc2.x1 = rc.x1 - 2;
    rc2.y1 = rc.y1 - 2;
    if (stat & 0x80) // mains
    {
      if (charge >= 85) color = RGB(0x00, 0xff, 0x00);
      else if (charge >= 6) color = RGB(244, 198, 55);
      else color = RGB(0xff, 0x00, 0x00);
    }
    else
    {
      int minutes_to_shutdown = (stat&0xFF00) | ((stat>>16)&0xFF);
      if (minutes_to_shutdown >= 30) color = RGB(0x00, 0xff, 0x00);
      else color = RGB(244, 198, 55);
    }
    if (charge != 127 && (rc2.x1-rc2.x0<2)) rc2.x0 = rc2.x1-2;
    grph_fillrect(psframe->fgdc, rc2.x0, rc2.y0, rc2.x1-rc2.x0, rc2.y1-rc2.y0, color);
  }

  // mains
  rc2.x0 = 44+0+142+bat_image.width/2-mains_image.width/2;
  rc2.y0 = 0+2+1;
  rc2.x1 = rc.x0 + mains_image.width;
  rc2.y1 = rc.y0 + 20 - 2;

  if (stat & 0x80)
    grph_filldata16(psframe->updc, rc2.x0, rc2.y0, mains_image.width, mains_image.height, (void*)  mains_image.pixel_data);
  else
    grph_fillrect(psframe->updc, rc2.x0, rc2.y0, mains_image.width, mains_image.height, 0x000000);

  // bitblt
  sframe_bitblt(psframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);
}

static void sframe_on_set_date(struct tm * ptm)
{
  sframe_t * psframe = &sframe;
  rect_t rc;
  char s[100];

  if (vermode)
  {
    rc.x0 = 44+0+0;
    rc.y0 = 0+2;
    rc.x1 = rc.x0 + 140;
    rc.y1 = rc.y0 + SFRAME_CY - 3;

    grph_fillrect(psframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
    grph_setfgcolor(psframe->fgdc, RGB(0xCC,0x00,0x00));
    grph_setbkcolor(psframe->fgdc, SFRAME_BKCOLOR);
    grph_setfont(psframe->fgdc, SFRAME_FONT);

    sprintf(s, "%s %s", __DATE__, __TIME__);
    s[strlen(s)-3] = 0; // remove seconds from ts string
    grph_textout(psframe->fgdc, rc.x0, rc.y0, s, strlen(s));

    sframe_bitblt(psframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);

    return;
  }

  if (ptm == NULL) return;

  rc.x0 = 44+0+0;
  rc.y0 = 0+2;
  rc.x1 = rc.x0 + 90;
  rc.y1 = rc.y0 + SFRAME_CY - 3;

  sprintf(s, "%02d.%02d.%04d", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900);

  grph_fillrect(psframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_setfgcolor(psframe->fgdc, SFRAME_FGCOLOR);
  grph_setbkcolor(psframe->fgdc, SFRAME_BKCOLOR);
  grph_setfont(psframe->fgdc, SFRAME_FONT);

  grph_textout(psframe->fgdc, rc.x0, rc.y0, s, strlen(s));
 // grph_drawtext(psframe->fgdc, s, -1, &rc, DT_CALCRECT);

  sframe_bitblt(psframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);
}

static void sframe_on_set_time(struct tm * ptm)
{
  sframe_t * psframe = &sframe;
  rect_t rc;
  char s[20];

  if (vermode || ptm == NULL)
  {
    return;
  }

  rc.x0 = 44+90+0;
  rc.y0 = 0+2;
  rc.x1 = rc.x0 + 52;
  rc.y1 = rc.y0 + SFRAME_CY - 3;

  snprintf(s, sizeof(s), "%02d:%02d", ptm->tm_hour, ptm->tm_min);

  grph_fillrect(psframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_setfgcolor(psframe->fgdc, SFRAME_FGCOLOR);
  grph_setbkcolor(psframe->fgdc, SFRAME_BKCOLOR);
  grph_setfont(psframe->fgdc, SFRAME_FONT);

  grph_textout(psframe->fgdc, rc.x0, rc.y0, s, strlen(s));
 // grph_drawtext(psframe->fgdc, s, -1, &rc, DT_CALCRECT);

  sframe_bitblt(psframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);
}

static void sframe_on_reload(void)
{
 // debug("%s not implemented\n", __FUNCTION__);
}

static void sframe_on_toggle_vermode(void)
{
#ifdef SFRAME_BMONI_TS
  struct tm ltm;
  vermode = !vermode;
  debug("toggle to %s mode\n", vermode ? "ver ts" : "normal");
 // sighandler(SIGALRM);
  read_hwclock_data(&ltm);
  sframe_command(SFRAME_SET_DATE, (void*) &ltm);
  sframe_command(SFRAME_SET_TIME, (void*) &ltm);
#else
  // no action
#endif
}

void sframe_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case SFRAME_SET_DATE:
    case SFRAME_SET_TIME:
      dview_command(SFRAME, cmd, sizeof(struct tm), arg);
      break;
    default:
      dview_command(SFRAME, cmd, 0, arg);
      break;
  }
}

void sframe_on_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case SFRAME_RELOAD:
      sframe_on_reload();
      break;
    case SFRAME_SET_BATSTAT:
      sframe_on_set_batstat(*((int*)arg));
      break;
    case SFRAME_SET_DATE:
      sframe_on_set_date((struct tm*)arg);
      break;
    case SFRAME_SET_TIME:
      sframe_on_set_time((struct tm*)arg);
      break;
    case SFRAME_TOGGLE_VERMODE:
      sframe_on_toggle_vermode();
      break;
    default:
      error("%s: unknown cmd %d\n", __FUNCTION__, cmd);
      break;
  }
}

static void sframe_bitblt(sframe_t * psframe, int x, int y, int cx, int cy)
{
  dc_list_t dclist;

  assert(psframe);

  dclist.xres = ((PGDC)(psframe->bkdc))->xres;
  dclist.yres = ((PGDC)(psframe->bkdc))->yres;
  dclist.p_bk = (unsigned char*)((PGDC)(psframe->bkdc))->addr;
  dclist.p_fg = (unsigned char*)((PGDC)(psframe->fgdc))->addr;
  dclist.p_up = (unsigned char*)((PGDC)(psframe->updc))->addr;

  if (x + cx >= SFRAME_CX) cx = SFRAME_CX - x;
  if (y + cy >= SFRAME_CY) cy = SFRAME_CY - y;

 // debug("%s: %d %d %d %d %d %d\n", __FUNCTION__, piframe->x+x, piframe->y+y, cx, cy, x, y);

  dview_bitblt(psframe->x+x, psframe->y+y, cx, cy, &dclist, x, y);
}

void sframe_bmoni_ts_disable(void)
{
  struct tm ltm;
  vermode = 0;
  read_hwclock_data(&ltm);
  sframe_command(SFRAME_SET_DATE, (void*) &ltm);
  sframe_command(SFRAME_SET_TIME, (void*) &ltm);
}

