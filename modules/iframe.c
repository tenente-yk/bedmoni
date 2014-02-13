#ifdef UNIX
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "bedmoni.h"
#include "grph.h"
#include "alarm.h"
#include "dview.h"
#include "iframe.h"
#include "unit.h"
#include "patient.h"
#include "crbpio.h"

#include "bed.cc"
#include "nobell.cc"

static iframe_t iframe;

static void iframe_on_reload(void);

static void iframe_bitblt(iframe_t * piframe, int x, int y, int cx, int cy);

void iframe_init(void)
{
  iframe_t * piframe;

  piframe = &iframe;

  memset(piframe, 0, sizeof(iframe_t));

  piframe->x    = IFRAME_X0;
  piframe->y    = IFRAME_Y0;
  piframe->cx   = IFRAME_CX;
  piframe->cy   = IFRAME_CY;
  piframe->mode = IFRAME_MODE_MONITOR;
  piframe->visible = 1;

  piframe->num_cmds_in_queue = 0;

  piframe->bkdc = grph_createdc(rootwfd, 0, 0, piframe->cx, piframe->cy, 0);
  piframe->fgdc = grph_createdc(rootwfd, 0, 0, piframe->cx, piframe->cy, 0);
  piframe->updc = grph_createdc(rootwfd, 0, 0, piframe->cx, piframe->cy, 0);
  grph_fillrect(piframe->bkdc, 0, 0, piframe->cx, piframe->cy, 0x000000);
  grph_fillrect(piframe->fgdc, 0, 0, piframe->cx, piframe->cy, 0x000000);
  grph_fillrect(piframe->updc, 0, 0, piframe->cx, piframe->cy, 0x000000);

  iframe_on_reload();

  iframe_bitblt(piframe, 0, 0, piframe->cx, piframe->cy);
}

void iframe_deinit()
{
  iframe_t * piframe;

  piframe = &iframe;

  grph_releasedc(piframe->bkdc);
  grph_releasedc(piframe->fgdc);
  grph_releasedc(piframe->updc);
}

//void iframe_ioctl(int cmd, ...)
//{
//  iframe_command(IFRAME_MSG_UPDATE, NULL);
//}

static void iframe_on_set_pat(int pat)
{
  pat_pars_t lpp;
  char s[128];
  rect_t rc;
  iframe_t * piframe = &iframe;

  if (piframe->mode != IFRAME_MODE_MONITOR) return;

  ids2string(pat_cat_ids[pat], s);

  rc.x0 = 0+5;
  rc.y0 = 0+0;
  rc.x1 = rc.x0 + 140;
  rc.y1 = 0+IFRAME_CY;

  grph_fillrect(piframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_setfont(piframe->fgdc, IFRAME_FONT);
  grph_setfgcolor(piframe->fgdc, IFRAME_FGCOLOR);
  grph_setbkcolor(piframe->fgdc, IFRAME_BKCOLOR);
  grph_drawtext(piframe->fgdc, s, -1, &rc, DT_LEFT);

  iframe_bitblt(piframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);

  pat_get(&lpp);
  lpp.type = pat;
  pat_set(&lpp);
}

static void iframe_on_set_bedno(int bedno)
{
  iframe_t * piframe = &iframe;
  pat_pars_t lpp;
  rect_t rc;
  char s[10];

  if (piframe->mode != IFRAME_MODE_MONITOR) return;

  rc.x0 = 0+150;
  rc.y0 = 0+3;
  rc.x1 = rc.x0 + bed_image.width;
  rc.y1 = rc.y0 + 20 - 1;
  grph_filldata16(piframe->fgdc, rc.x0, rc.y0, bed_image.width, bed_image.height, (void*) bed_image.pixel_data);
  iframe_bitblt(piframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);

  snprintf(s, sizeof(s), "%d", bedno);

  rc.x0 = 205;
  rc.y0 = 0+0;
  rc.x1 = rc.x0 + 36;
  rc.y1 = IFRAME_CY;

  grph_fillrect(piframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_setfgcolor(piframe->fgdc, IFRAME_FGCOLOR);
  grph_setbkcolor(piframe->fgdc, IFRAME_BKCOLOR);
  grph_textout(piframe->fgdc, rc.x0, rc.y0, s, strlen(s));
  iframe_bitblt(piframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);

  pat_get(&lpp);
  lpp.bedno = bedno;
  pat_set(&lpp);
}

static void iframe_on_set_cardno(int cardno)
{
  iframe_t * piframe = &iframe;
  pat_pars_t lpp;
  char s[20];
  rect_t rc;

  if (piframe->mode != IFRAME_MODE_MONITOR) return;

  sprintf(s, "N %d", cardno);

  rc.x0 = 265;
  rc.y0 = 0+0;
  rc.x1 = rc.x0 + 78;
  rc.y1 = IFRAME_CY;

  grph_fillrect(piframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_setfont(piframe->fgdc, IFRAME_FONT);
  grph_setfgcolor(piframe->fgdc, IFRAME_FGCOLOR);
  grph_setbkcolor(piframe->fgdc, IFRAME_BKCOLOR);
  grph_drawtext(piframe->fgdc, s, -1, &rc, DT_LEFT);

  iframe_bitblt(piframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);

  pat_get(&lpp);
  lpp.cardno = cardno;
  pat_set(&lpp);
}

static void iframe_on_msg_update(void)
{
//#define ALR_DEB
  iframe_t * piframe = &iframe;
  int i;
  int color_level = 0;
  int sound_level = 0;
  static signed char msgno_old = NO_ALARMS;
  signed char msgno_d[30], nummsgno = 0;
  rect_t rc;
  alarm_cfg_t acfg;
  int alarm_sound;
  char s[200];
#ifdef ALR_DEB
  int sound_msgno, color_msgno[30];
  sound_msgno = 0;
#endif

 // if (piframe->mode != IFRAME_MODE_MONITOR) return;

  alarm_cfg_get(&acfg);

  for (i=0; i<NUM_ALARMS; i++)
  {
    if (alarm_msg[i].chno != -1)
    {
      if (!unit_isenabled(alarm_msg[i].chno)) continue;
    }
    if ((alarm_msg[i].state == MSG_SET || (alarm_msg[i].state == MSG_CLR && alarm_msg[i].fixed == MSG_FIX)) && alarm_msg[i].level > color_level)
    {
      color_level = alarm_msg[i].level;
    }
    if ((alarm_msg[i].state == MSG_SET || (alarm_msg[i].state == MSG_CLR && alarm_msg[i].fixed == MSG_FIX)) && alarm_msg[i].level > sound_level && alarm_msg[i].enabled)
    {
      sound_level = alarm_msg[i].level;
    }
  }

  msgno_d[0] = NO_ALARMS;
  alarm_sound = AlarmSound_None;
  for (i=0; i<NUM_ALARMS; i++)
  {
    if (alarm_msg[i].chno != -1)
    {
      if (!unit_isenabled(alarm_msg[i].chno)) continue;
    }
    if ((alarm_msg[i].state == MSG_SET || (alarm_msg[i].state == MSG_CLR && alarm_msg[i].fixed == MSG_FIX)) && alarm_msg[i].level == color_level)
    {
#ifdef ALR_DEB
      color_msgno[nummsgno] = alarm_msg[i].msgno;
#endif
      msgno_d[nummsgno++] = alarm_msg[i].msgno;
      if (nummsgno >= 30)
      {
        assert(0);
      }
      if (alarm_msg[i].chno != -1)
      {
#if 0
        unit_ioctl(alarm_msg[i].chno, SET_ALARM, i);
#else
        int it = alarm_msg[i].item;
        unit_ioctl(alarm_msg[i].chno, SET_ALARM_ITEM, i, it);
#endif
      }
    }
    if ((alarm_msg[i].state == MSG_SET || (alarm_msg[i].state == MSG_CLR && alarm_msg[i].fixed == MSG_FIX)) && alarm_msg[i].level == sound_level && alarm_msg[i].enabled)
    {
      if (alarm_sound < alarm_msg[i].alsnd)
      {
        alarm_sound = alarm_msg[i].alsnd;
#ifdef ALR_DEB
        sound_msgno = alarm_msg[i].msgno;
#endif
      }
    }
  }

#ifdef ALR_DEB
  int l;
  printf("COLOR ALR: ");
  for (l=0; l<nummsgno; l++)
  {
    printf("%d ", color_msgno[l]);
  }
  printf("\n");
  printf("SOUND ALR: %d\n", sound_msgno);
#endif
//printf("nummsgno = %d, old %d\n", nummsgno, msgno_old);
//printf("level %d sound %d\n", level, alarm_sound);

  alarm_crbp(color_level, alarm_sound);

  for (i=0; i<nummsgno; i++)
  {
    if (msgno_d[i] == msgno_old)
    {
      msgno_old = msgno_d[(i+1)%nummsgno];
      break;
    }
  }
  if (i == nummsgno) msgno_old = msgno_d[0];

//printf("2nummsgno = %d, old %d\n", nummsgno, msgno_old);

//printf(": %d %d\n", alarm_msg[2].state, isel);

  rc.x0 = 360;
  rc.y0 = 0;
  rc.x1 = IFRAME_X0+IFRAME_CX - 25 - 40;
  rc.y1 = IFRAME_Y0+IFRAME_CY;

  grph_fillrect(piframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  if (msgno_old == NO_ALARMS)
  {
    crbpio_send_msg(CRBP_SOUND_ALARM, 0x00, 0x00, 0x00, 0xff); // stop sound alarms
    iframe_bitblt(piframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);
    return;
  }

  grph_setfgcolor(piframe->fgdc, risk_color[color_level]);
  grph_setbkcolor(piframe->fgdc, IFRAME_BKCOLOR);
  grph_setfont(piframe->fgdc, IFRAME_FONT);
  ids2string(alarm_msg[msgno_old].ids, s);
  grph_textout(piframe->fgdc, rc.x0, rc.y0, s, strlen(s));

#if defined (TEST_IDSTEXT_MODE)
  pat_pars_t pat_pars;
  pat_get(&pat_pars);
  if (pat_pars.cardno < NUM_ALARMS)
  {
    rc.x0 = 360;
    rc.y0 = 0;
    rc.x1 = IFRAME_X0+IFRAME_CX - 25 - 40;
    rc.y1 = IFRAME_Y0+IFRAME_CY;
    grph_fillrect(piframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

    ids2string(alarm_msg[pat_pars.cardno].ids, s);
    grph_setfgcolor(piframe->fgdc, RGB(0x00,0xFF,0x00));
    grph_textout(piframe->fgdc, rc.x0, rc.y0, s, strlen(s));
  }
#endif

  iframe_bitblt(piframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);
}

static void iframe_on_reload(void)
{
  pat_pars_t lpp;
  char s[128];
  alarm_cfg_t ac;
  iframe_t * piframe = &iframe;

  grph_fillrect(piframe->fgdc, 0, 0, 360, IFRAME_CY, 0x000000);
  grph_fillrect(piframe->fgdc, IFRAME_CX - 25 - 40, 0, 25+40, IFRAME_CY, 0x000000);

  alarm_cfg_get(&ac);
  if (piframe->v == 0)
  {
    if (ac.volume == 0)
    {
      grph_filldatastretch16(piframe->fgdc, IFRAME_X0+IFRAME_CX-25-40, 0, nobell_image.width, nobell_image.height, 20, 20, (void*)nobell_image.pixel_data);
    }
    iframe_bitblt(piframe, IFRAME_X0+IFRAME_CX-25-40, 0, 25, IFRAME_CY);
  }

  switch (piframe->mode)
  {
    case IFRAME_MODE_TRENDS:
      ids2string(IDS_TRENDS, s);
      break;
    case IFRAME_MODE_TABLE:
      ids2string(IDS_TABLE, s);
      break;
    case IFRAME_MODE_EVLOGS:
      ids2string(IDS_EVENT_LOGS, s);
      break;
    case IFRAME_MODE_MONITOR:
      pat_get(&lpp);

      iframe_on_set_pat(lpp.type);       // might realize not to bitblt items
      iframe_on_set_bedno(lpp.bedno);    // cause later a whole frame will be "bitblted"
      iframe_on_set_cardno(lpp.cardno);  //
      iframe_bitblt(piframe, 0, 0, 360, IFRAME_CY);
      return;
    default:
      iframe_bitblt(piframe, 0, 0, 360, IFRAME_CY);
      return;
  }

  grph_setfont(piframe->fgdc, IFRAME_FONT);
  grph_setfgcolor(piframe->fgdc, IFRAME_FGCOLOR);
  grph_setbkcolor(piframe->fgdc, IFRAME_BKCOLOR);

  grph_textout(piframe->fgdc, 30, 0, s, strlen(s));

  iframe_bitblt(piframe, 0, 0, IFRAME_CX, IFRAME_CY);
}

static void iframe_on_set_mode(int mode)
{
  iframe_t * piframe = &iframe;

  assert(mode < IFRAME_NUM_MODES);

  piframe->mode = mode;

  iframe_on_reload();
}

static void iframe_on_set_string(char * s)
{
  iframe_t * piframe = &iframe;
  rect_t rc;

  if (piframe->mode != IFRAME_MODE_TABLE) return;

  rc.x0 = rc.y0 = 0;
  rc.x1 = rc.x0 + IFRAME_CX;
  rc.y1 = rc.y0 + IFRAME_CY;
  grph_fillrect(piframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  grph_setfont(piframe->fgdc, IFRAME_FONT);
  grph_setfgcolor(piframe->fgdc, IFRAME_FGCOLOR);
  grph_setbkcolor(piframe->fgdc, IFRAME_BKCOLOR);
  rc.x0 = 130;
  rc.y0 = 0;
  rc.x1 = 360;
  rc.y1 = rc.y0 + IFRAME_CY;

  grph_textout(piframe->fgdc, rc.x0, rc.y0, s, strlen(s));
  iframe_bitblt(piframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);
}

static void iframe_on_set_timerval(int sec)
{
  iframe_t * piframe = &iframe;
  char s[20];
  rect_t rc;
  alarm_cfg_t ac;

 // if (piframe->mode != IFRAME_MODE_MONITOR) return;

  rc.x0 = IFRAME_X0 + IFRAME_CX - 25 - 40;
  rc.y0 = 1;
  rc.x1 = IFRAME_X0 + IFRAME_CX;
  rc.y1 = rc.y0 + IFRAME_CY;

  grph_fillrect(piframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  grph_setfont(piframe->fgdc, ARIAL_14);
  grph_setfgcolor(piframe->fgdc, RGB(0xDD,0x00,0x00)/*IFRAME_FGCOLOR*/);
  grph_setbkcolor(piframe->fgdc, IFRAME_BKCOLOR);

  alarm_cfg_get(&ac);

  piframe->v = sec;
  if (piframe->v)
  {
    grph_filldatastretch16(piframe->fgdc, rc.x0, 0, nobell_image.width, nobell_image.height, 20, 20, (void*)nobell_image.pixel_data);
   // alarm_ge_off();
    snprintf(s, sizeof(s), "%d:%02d", sec/60, sec%60);
    rc.x0 += 25;
    grph_drawtext(piframe->fgdc, s, -1, &rc, DT_LEFT);
    rc.x0 -= 25;

    ac.delay_remd = sec;
    alarm_cfg_set(&ac);
    alarm_cfg_save();
  }
  else
  {
    if (ac.volume == 0)
    {
      grph_filldatastretch16(piframe->fgdc, IFRAME_X0+IFRAME_CX-25-40, 0, nobell_image.width, nobell_image.height, 20, 20, (void*)nobell_image.pixel_data);
      iframe_bitblt(piframe, IFRAME_X0+IFRAME_CX-25-40, 0, 25, IFRAME_CY);
    }
  }
  iframe_bitblt(piframe, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0);
}

void iframe_process_msgs(void)
{
  iframe_t * piframe = &iframe;
  iframe_command(IFRAME_MSG_UPDATE, NULL);
  if (piframe->v)
    iframe_command(IFRAME_SET_TIMERVAL, (void*)(--piframe->v));
  else
    alarm_ge_on();
}

void iframe_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case IFRAME_SET_STRING:
      *((char*)arg+DVIEW_ARG_DATA_SIZE-1) = 0; // null-terminate string, if no null-terminated
      dview_command(IFRAME, cmd, DVIEW_ARG_DATA_SIZE, arg);
      break;
    default:
      dview_command(IFRAME, cmd, 0, arg);
      break;
  }
}

void iframe_on_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case IFRAME_RELOAD:
      iframe_on_reload();
      break;
    case IFRAME_SET_PAT:
      iframe_on_set_pat(*((int*)arg));
      break;
    case IFRAME_SET_BEDNO:
      iframe_on_set_bedno(*((int*)arg));
      break;
    case IFRAME_SET_CARDNO:
      iframe_on_set_cardno(*((int*)arg));
      break;
    case IFRAME_MSG_UPDATE:
      iframe_on_msg_update();
      break;
    case IFRAME_SET_MODE:
      iframe_on_set_mode(*((int*)arg));
      break;
    case IFRAME_SET_STRING:
      iframe_on_set_string((char*)arg);
      break;
    case IFRAME_SET_TIMERVAL:
      iframe_on_set_timerval(*((int*)arg));
      break;
    default:
      error("%s: unknown cmd %d\n", __FUNCTION__, cmd);
      break;
  }
}

static void iframe_bitblt(iframe_t * piframe, int x, int y, int cx, int cy)
{
  dc_list_t dclist;

  assert(piframe);

  dclist.xres = ((PGDC)(piframe->bkdc))->xres;
  dclist.yres = ((PGDC)(piframe->bkdc))->yres;
  dclist.p_bk = (unsigned char*)((PGDC)(piframe->bkdc))->addr;
  dclist.p_fg = (unsigned char*)((PGDC)(piframe->fgdc))->addr;
  dclist.p_up = (unsigned char*)((PGDC)(piframe->updc))->addr;

  if (x + cx >= IFRAME_CX) cx = IFRAME_CX - x;
  if (y + cy >= IFRAME_CY) cy = IFRAME_CY - y;

 // debug("%s: %d %d %d %d %d %d\n", __FUNCTION__, piframe->x+x, piframe->y+y, cx, cy, x, y);

  dview_bitblt(piframe->x+x, piframe->y+y, cx, cy, &dclist, x, y);
}

iframe_t * iframe_getptr(void)
{
  return &iframe;
}

