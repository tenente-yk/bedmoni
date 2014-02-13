#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bedmoni.h"
#include "dview.h"
#include "dio.h"
#include "grph.h"
#include "tframe.h"
#include "iframe.h"
#include "uframe.h"
#include "sframe.h"
#include "cframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "trnmenu.h"
#include "stconf.h"
#include "tr.h"

static tframe_t tframe;

static int tsc[NUM_TSC] = {2*60, 4*60, 8*60, 12*60, 24*60};
static int cursor_at_the_end = 0;

static void tframe_on_draw(void);
static void tframe_on_cursor_lseek(int delta);
static void tframe_on_read(void);
static void tframe_on_create(void);
static void tframe_on_destroy(void);

static void tframe_bitblt(tframe_t * ptframe, int x, int y, int cx, int cy)
{
  dc_list_t dclist;

  assert(ptframe);

  dclist.xres = ((PGDC)(ptframe->bkdc))->xres;
  dclist.yres = ((PGDC)(ptframe->bkdc))->yres;
  dclist.p_bk = ((PGDC)(ptframe->bkdc))->addr;
  dclist.p_fg = ((PGDC)(ptframe->fgdc))->addr;
  dclist.p_up = ((PGDC)(ptframe->updc))->addr;

  if (ptframe->x+x >= TFRAME_CX) return;

  if (x + cx >= ptframe->cx) cx = ptframe->cx-x;
  if (y + cy >= ptframe->cy) cy = ptframe->cy-y;

  dview_bitblt(ptframe->x+x, ptframe->y+y, cx, cy, &dclist, x, y);
}

static void frame_bitblt(int x, int y, int cx, int cy, void *parg)
{
  tframe_t * ptframe = &tframe;
  assert(ptframe);
  tframe_bitblt(ptframe, x, y, cx, cy);
}

void tframe_ini_def(void)
{
  tframe_t * ptframe = &tframe;
  int i;

  ptframe->visible = 0;

  for (i=0; i<TFRAME_MAX_NUM_VIEWS; i++)
  {
    ptframe->view[i].visible = 0;
    memset(ptframe->view[i].data, 0, sizeof( ((tview_t*)(0))->data ));
  }

  ptframe->vlen = TFRAME_VIEW_DATALEN;

  ptframe->view[0].visible = 1;
  ptframe->view[0].mask = 0x000f;
  ptframe->view[0].num_curves = 4;
  ptframe->view[0].type[0] = PARAM_HR;
  ptframe->view[0].type[1] = PARAM_NIBP;
  ptframe->view[0].type[2] = PARAM_SPO2;
  ptframe->view[0].type[3] = PARAM_PULSE;
  ptframe->view[0].vl = 30;
  ptframe->view[0].vu = 200;
  ptframe->view[0].kv = 1;

  ptframe->view[1].visible = 1;
  ptframe->view[1].mask = 0x0003;
  ptframe->view[1].num_curves = 7;
  ptframe->view[1].type[0] = PARAM_STI;
  ptframe->view[1].type[1] = PARAM_STII;
  ptframe->view[1].type[2] = PARAM_STIII;
  ptframe->view[1].type[3] = PARAM_STAVR;
  ptframe->view[1].type[4] = PARAM_STAVL;
  ptframe->view[1].type[5] = PARAM_STAVF;
  ptframe->view[1].type[6] = PARAM_STV;
  ptframe->view[1].vl = -100;
  ptframe->view[1].vu = 100;
  ptframe->view[1].kv = 1;

  ptframe->view[2].visible = 1;
  ptframe->view[2].mask = 0x0007;
  ptframe->view[2].num_curves = 3;
  ptframe->view[2].type[0] = PARAM_T1;
  ptframe->view[2].type[1] = PARAM_T2;
  ptframe->view[2].type[2] = PARAM_DT;
  ptframe->view[2].vl = 300;
  ptframe->view[2].vu = 400;
  ptframe->view[2].kv = 10;

  ptframe->view[3].visible = 1;
  ptframe->view[3].mask = 0x0001;
  ptframe->view[3].num_curves = 4;
  ptframe->view[3].type[0] = PARAM_BR;
  ptframe->view[3].type[1] = PARAM_BRC;
  ptframe->view[3].type[2] = PARAM_ETCO2;
  ptframe->view[3].type[3] = PARAM_ICO2;
  ptframe->view[3].vl = 0;
  ptframe->view[3].vu = 50;
  ptframe->view[3].kv = 1;

  ptframe->view[4].visible = 0;
  ptframe->view[4].num_curves = 1;
  ptframe->view[4].mask = 0x0001;
  ptframe->view[4].type[0] = PARAM_BR;
  ptframe->view[4].vl = 0;
  ptframe->view[4].vu = 50;
  ptframe->view[4].kv = 1;

 // ptframe->xcur = 0;
  ptframe->vpos = 0;
  ptframe->tsc = TSC_2;
}

void tframe_init(void)
{
  int i;
  tframe_t * ptframe = &tframe;

  memset(ptframe, 0, sizeof(tframe_t));

  ptframe->visible = 0;
  ptframe->x  = TFRAME_X0;
  ptframe->y  = TFRAME_Y0;
  ptframe->cx = TFRAME_CX;
  ptframe->cy = TFRAME_CY + SFRAME_CY;

  ptframe->bkdc = grph_createdc(rootwfd, 0, 0, ptframe->cx, ptframe->cy, 0);
  ptframe->fgdc = grph_createdc(rootwfd, 0, 0, ptframe->cx, ptframe->cy, 0);
  ptframe->updc = grph_createdc(rootwfd, 0, 0, ptframe->cx, ptframe->cy, 0);

  ptframe->num_cmds_in_queue = 0;

  tframe_ini_def();

  tframe_cfg_t tframe_cfg;
  if ( stconf_read(STCONF_TFRAME, (char*)&tframe_cfg, sizeof(tframe_cfg_t)) > 0 )
  {
    for (i=0; i<TFRAME_MAX_NUM_VIEWS; i++)
    {
      ptframe->view[i].visible = tframe_cfg.view_cfg[i].visible;
      ptframe->view[i].mask = tframe_cfg.view_cfg[i].mask;
      ptframe->view[i].num_curves = tframe_cfg.view_cfg[i].num_curves;
      ptframe->view[i].vl = tframe_cfg.view_cfg[i].vl;
      ptframe->view[i].vu = tframe_cfg.view_cfg[i].vu;
      ptframe->view[i].kv = tframe_cfg.view_cfg[i].kv;
      memcpy(&ptframe->view[i].type, &tframe_cfg.view_cfg[i].type, TFRAME_MAX_NUM_CURVES*sizeof(int));
    }
    ptframe->visible = tframe_cfg.visible;
    ptframe->tsc = tframe_cfg.tsc;
  }

  if (ptframe->visible)
  {
//      grph_fillrect(ptframe->bkdc, 0, 0, ptframe->cx, ptframe->cy, 0x000000);
//      grph_fillrect(ptframe->fgdc, 0, 0, ptframe->cx, ptframe->cy, 0x000000);
//      grph_fillrect(ptframe->updc, 0, 0, ptframe->cx, ptframe->cy, 0x000000);

    dview_frame_active(TFRAME);
  }
}

void tframe_deinit(void)
{
  tframe_t * ptframe = &tframe;
  int i;

  tframe_cfg_t tframe_cfg;

  tframe_cfg.visible = ptframe->visible;
  tframe_cfg.tsc = ptframe->tsc;
  for (i=0; i<TFRAME_MAX_NUM_VIEWS; i++)
  {
    tframe_cfg.view_cfg[i].visible = ptframe->view[i].visible;
    tframe_cfg.view_cfg[i].mask = ptframe->view[i].mask;
    tframe_cfg.view_cfg[i].num_curves = ptframe->view[i].num_curves;
    tframe_cfg.view_cfg[i].vl = ptframe->view[i].vl;
    tframe_cfg.view_cfg[i].vu = ptframe->view[i].vu;
    tframe_cfg.view_cfg[i].kv = ptframe->view[i].kv;
    memcpy(&tframe_cfg.view_cfg[i].type, &ptframe->view[i].type, TFRAME_MAX_NUM_CURVES*sizeof(int));
  }
  stconf_write(STCONF_TFRAME, (char*)&tframe_cfg, sizeof(tframe_cfg_t));

  inpfoc_rm(INPFOC_TRENDS);

  grph_releasedc(ptframe->bkdc);
  grph_releasedc(ptframe->fgdc);
  grph_releasedc(ptframe->updc);
}

static void tframe_on_create(void)
{
  tframe_t * ptframe = &tframe;
  uframe_t * puframe = uframe_getptr();
  mframe_t * pmframe = mframe_getptr();
  inpfoc_wnd_dc_t ifwd;

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

  ptframe->visible = 1;
  ptframe->x  = TFRAME_X0;
  ptframe->y  = TFRAME_Y0;
  ptframe->cx = TFRAME_CX;
  ptframe->cy = TFRAME_CY;

  grph_fillrect(ptframe->bkdc, 0, ptframe->cy-TRN_BUTTON_CY-1, ptframe->cx, TRN_BUTTON_CY, 0x000000);
  tframe_bitblt(ptframe, 0, ptframe->cy-TRN_BUTTON_CY-1, ptframe->cx, TRN_BUTTON_CY);

  grph_fillrect(ptframe->bkdc, 0, ptframe->cy, ptframe->cx-SFRAME_CX, SFRAME_CY, 0x000000);
  tframe_bitblt(ptframe, 0, ptframe->cy, ptframe->cx-SFRAME_CX, SFRAME_CY);

  ifwd.bkdc = ptframe->bkdc;
  ifwd.updc = ptframe->updc;
  ifwd.fgcolor = 0x444444;
  ifwd.bkcolor = 0x020202;
 // ifwd.fgcolor = RGB(0xbb,0xbb,0xbb);
 // ifwd.bkcolor = TFRAME_BKCOLOR;

  inpfoc_add(INPFOC_TRENDS, TRENDS_LSEEK, (void*) inpfoc_wnd_create("LIST", 0+1, TFRAME_CY-TRN_BUTTON_CY-1, 60-1, TRN_BUTTON_CY, "<< >>", 0, &ifwd, frame_bitblt, ptframe));
  inpfoc_add(INPFOC_TRENDS, TRENDS_SEEK, (void*) inpfoc_wnd_create("LIST", 65, TFRAME_CY-TRN_BUTTON_CY-1, 60, TRN_BUTTON_CY, " < > ", 0, &ifwd, frame_bitblt, ptframe));
  inpfoc_add(INPFOC_TRENDS, TRENDS_END, (void*) inpfoc_wnd_create("PUSH", 130, TFRAME_CY-TRN_BUTTON_CY-1, 60, TRN_BUTTON_CY, " >>| ", 0, &ifwd, frame_bitblt, ptframe));
#if 0
  ids2string(IDS_STEP, s);
  inpfoc_add(INPFOC_TRENDS, TRENDS_STEP, (void*) inpfoc_wnd_create_("PUSH", 195, TFRAME_CY-TRN_BUTTON_CY-1, 60, TRN_BUTTON_CY, s, &ifwd, frame_bitblt, ptframe));
#else
  inpfoc_add(INPFOC_TRENDS, TRENDS_STEP, (void*) inpfoc_wnd_create("PUSH", 195, TFRAME_CY-TRN_BUTTON_CY-1, 60, TRN_BUTTON_CY, ">| |<", 0, &ifwd, frame_bitblt, ptframe));
#endif

  inpfoc_add(INPFOC_TRENDS, TRENDS_SET, (void*) inpfoc_wnd_create("PUSH", ptframe->cx - 102 - 5 - 60 - 5, TFRAME_CY-TRN_BUTTON_CY-1, 102, TRN_BUTTON_CY, 0, IDS_SETTINGS, &ifwd, frame_bitblt, ptframe));
  inpfoc_add(INPFOC_TRENDS, TRENDS_EXIT, (void*) inpfoc_wnd_create("PUSH", ptframe->cx - 60 - 5, TFRAME_CY-TRN_BUTTON_CY-1, 60-1, TRN_BUTTON_CY, 0, IDS_QUIT, &ifwd, frame_bitblt, ptframe));

  inpfoc_change(INPFOC_TRENDS);
  puframe->inpfoc_prev = INPFOC_TRENDS;

  iframe_command(IFRAME_SET_MODE, (void*)IFRAME_MODE_TRENDS);

  tr_seek(0, SEEK_SET);
  tframe_on_read();

  tframe_on_draw();

  inpfoc_set(INPFOC_TRENDS, TRENDS_EXIT);

  tframe_cfg_save();
}

static void tframe_on_read(void)
{
  tframe_t * ptframe = &tframe;
  tr_t tr;
  int i;

  for (i=0; i<TFRAME_VIEW_DATALEN; i++)
  {
    if (tr_read(&tr, 1, i, 1) == 0) break;
    ptframe->view[0].data[0][i] = tr.v[PARAM_HR];
    ptframe->view[0].data[1][i] = tr.v[PARAM_NIBP];
    ptframe->view[0].data[2][i] = tr.v[PARAM_SPO2];
    ptframe->view[0].data[3][i] = tr.v[PARAM_PULSE];

    ptframe->view[1].data[0][i] = tr.v[PARAM_STI];
    ptframe->view[1].data[1][i] = tr.v[PARAM_STII];
    ptframe->view[1].data[2][i] = tr.v[PARAM_STIII];
    ptframe->view[1].data[3][i] = tr.v[PARAM_STAVR];
    ptframe->view[1].data[4][i] = tr.v[PARAM_STAVL];
    ptframe->view[1].data[5][i] = tr.v[PARAM_STAVF];
    ptframe->view[1].data[6][i] = tr.v[PARAM_STV];

    ptframe->view[2].data[0][i] = tr.v[PARAM_T1];
    ptframe->view[2].data[1][i] = tr.v[PARAM_T2];
    ptframe->view[2].data[2][i] = tr.v[PARAM_DT];

    ptframe->view[3].data[0][i] = tr.v[PARAM_BR];
    ptframe->view[3].data[1][i] = tr.v[PARAM_BRC];
    ptframe->view[3].data[2][i] = tr.v[PARAM_ETCO2];
    ptframe->view[3].data[3][i] = tr.v[PARAM_ICO2];

    ptframe->view[4].data[0][i] = tr.v[PARAM_TS];
  }
  ptframe->vlen = i-1;

 // tframe_on_draw();
}

static void tframe_on_destroy(void)
{
  tframe_t * ptframe = &tframe;
  uframe_t * puframe = uframe_getptr();

  assert(puframe);

  iframe_command(IFRAME_SET_MODE, (void*)IFRAME_MODE_MONITOR);

  if (!ptframe->visible) return;

  inpfoc_rm(INPFOC_TRENDS);
  inpfoc_change(INPFOC_MAIN);
  puframe->inpfoc_prev = INPFOC_MAIN;

  ptframe->visible = 0;

  tframe_cfg_save();
}

static void tframe_on_draw(void)
{
  tframe_t * ptframe;
  rect_t rc;
  int i, j, k, nv;
  int wh, wh1, ww;
  int yu, yl;
  float kk, bb;
  char s[200], s1[200];
  unsigned int t;
  trts_t trts;

  ptframe = &tframe;

  rc.x0 = 0;
  rc.y0 = 0;
  rc.x1 = rc.x0 + ptframe->cx;
  rc.y1 = rc.y0 + TFRAME_CY-TRN_BUTTON_CY-5;

  grph_fillrect(ptframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(ptframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(ptframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  // no need return focus, refreshed display and inpfoc are not overlapped
 // inpfoc_focus(INPFOC_MAIN);

#if 0
  dclist_t dclist;
  // clear section near sframe section (cause may be filled by cframe before this)
  rc.x0 = 0;
  rc.x1 = SFRAME_X0;
  rc.y0 = ptframe->y+ptframe->cy;
  rc.y1 = rc.y0 + SFRAME_CY;

  grph_fillrect(ptframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(ptframe->fgdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
  grph_fillrect(ptframe->updc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);

  dclist.xres = ((PGDC)(ptframe->bkdc))->xres;
  dclist.yres = ((PGDC)(ptframe->bkdc))->yres;
  dclist.p_bk = (unsigned char*)((PGDC)(ptframe->bkdc))->addr;
  dclist.p_fg = (unsigned char*)((PGDC)(ptframe->fgdc))->addr;
  dclist.p_up = (unsigned char*)((PGDC)(ptframe->updc))->addr;
  dview_bitblt(rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, &dclist, 0, ptframe->cy);
#endif

  wh = 0;
  for (i=0; i<TFRAME_MAX_NUM_VIEWS; i++)
  {
    if (!ptframe->view[i].visible) continue;
    wh ++;
  }
  if (wh == 0)
  {
    tframe_bitblt(ptframe, 0, 0, ptframe->cx, ptframe->cy);
    return;
  }

  wh = (int)( ((float)(TFRAME_CY-TRN_BUTTON_CY-10-4)) / (wh) );
  ww = ptframe->cx - TFRAME_LB - TFRAME_RB;

#if 1
  t = 0;
  nv = 0;
  for (i=0; i<TFRAME_MAX_NUM_VIEWS; i++)
  {
    tview_t * ptv = &ptframe->view[i];
    if (!ptv->visible) continue;
    wh1 = wh - 2*TFRAME_BORDER_W;
    yu = nv*wh+TFRAME_BORDER_W;
    yl = (nv+1)*wh-TFRAME_BORDER_W;

    assert(ptv->vl - ptv->vu != 0);
    kk = ((float)yl-yu) / (ptv->vl - ptv->vu);
    bb = ((float)ptv->vl*yu-ptv->vu*yl) / (ptv->vl - ptv->vu);

    grph_setfgcolor(ptframe->bkdc, RGB(0xff,0xff,0xff));
    grph_setbkcolor(ptframe->bkdc, RGB(0x00,0x00,0x00));
    grph_setfont(ptframe->bkdc, TFRAME_FONT_SMALL);

    int div = 1;
    switch (ptframe->tsc)
    {
      case TSC_2:
        div = 15;
        break;
      case TSC_4:
        div = 30;
        break;
      case TSC_8:
      case TSC_12:
        div = 60;
        break;
      case TSC_24:
        div = 120;
        break;
      default:
        error("%s: line %s\n", __FILE__, __LINE__);
        assert(0);
    }

    // draw grid
    grph_setfgcolor(ptframe->fgdc, RGB(0xff,0xff,0xff));
    grph_setbkcolor(ptframe->fgdc, RGB(0x00,0x00,0x00));
    grph_setfont(ptframe->fgdc, TFRAME_FONT_SMALL);
    grph_line(ptframe->fgdc, TFRAME_LB, yu, TFRAME_LB, yl);
    grph_line(ptframe->fgdc, TFRAME_LB, yl, TFRAME_LB+ww, yl);
    for (j=0; j<5+1; j++)
    {
      grph_line(ptframe->fgdc, TFRAME_LB-5, (int)(yu+(float)wh1*j/5), TFRAME_LB+5, (int)(yu+(float)wh1*j/5));
      if (ptv->kv == 1)
        snprintf(s, sizeof(s), "%d", (int)((float)ptv->vu - j*((ptv->vu - ptv->vl) / 5)) );
      else if (ptv->kv == 10)
        snprintf(s, sizeof(s), "%.01f", ((float)ptv->vu - j*((ptv->vu - ptv->vl) / 5)) / ptv->kv );
      else if (ptv->kv == 100)
        snprintf(s, sizeof(s), "%.02f", ((float)ptv->vu - j*((ptv->vu - ptv->vl) / 5)) / ptv->kv );
      else
        snprintf(s, sizeof(s), "--" );
      rc.y0 = (int)(yu+(float)wh1*j/5)-5;
      rc.x1 = TFRAME_LB-2;
      rc.x0 = rc.x1 - 30;
      rc.y1 = rc.y0 + 10;
      grph_drawtext(ptframe->fgdc, s, strlen(s), &rc, DT_CENTER);
    }

   // if (i == 4) // TODO
    {
      trts = *((trts_t *) &ptframe->view[4].data[0][0]);
      t = trts.hour*60 + trts.min;
      for (k=0; k<tsc[ptframe->tsc]; k++, t++)
      {
        trts = *((trts_t *) &ptframe->view[4].data[0][k]);
        if (trts.beg && k < ptframe->vlen)
        {
          grph_setfgcolor(ptframe->bkdc, RGB(0x88,0x88,0x88));
          //grph_line(ptframe->bkdc, TFRAME_LB+(int)((float)k*ww/tsc[ptframe->tsc]), 0, TFRAME_LB+(int)((float)k*ww/tsc[ptframe->tsc]), ptframe->cy-TRN_BUTTON_CY-6);
          int ll;
          for (ll=0;ll<(ptframe->cy-TRN_BUTTON_CY-6)/3-1; ll++)
          {
            grph_line(ptframe->bkdc, TFRAME_LB+(int)((float)k*ww/tsc[ptframe->tsc]), 3*ll, TFRAME_LB+(int)((float)k*ww/tsc[ptframe->tsc]), 3*ll+1);
          }
          t = trts.hour*60 + trts.min;
        }
        grph_setfgcolor(ptframe->bkdc, RGB(0xff,0xff,0xff));
        if (t % div == 0)
        {
          grph_line(ptframe->bkdc, TFRAME_LB+(int)((float)k*ww/tsc[ptframe->tsc]), yl-3, TFRAME_LB+(int)((float)k*ww/tsc[ptframe->tsc]), yl+3);
          if (TFRAME_CY-TRN_BUTTON_CY-10-TFRAME_BORDER_W - yl < 20)
          {
            rc.x0 = TFRAME_LB+k*ww/tsc[ptframe->tsc] - 8;
            rc.y0 = yl + 8;
            rc.x1 = rc.x0 + 16;
            rc.y1 = rc.y0 + 10;
            switch (ptframe->tsc)
            {
              case TSC_2:
                snprintf(s, sizeof(s), "%02d:%02d", (t/60)%24, t%60);
                break;
              case TSC_4:
                snprintf(s, sizeof(s), "%02d:%02d", (t/60)%24, t%60);
                break;
              case TSC_8:
              case TSC_12:
                snprintf(s, sizeof(s), "%02d:00", (t/60)%24);
                break;
              case TSC_24:
                snprintf(s, sizeof(s), "%02d:00", (t/60)%24);
                break;
              default:
                error("%s: line %s\n", __FILE__, __LINE__);
               while(1);
            }
            grph_setfgcolor(ptframe->bkdc, 0xffffff);
            grph_drawtext(ptframe->bkdc, s, strlen(s), &rc, DT_CENTER);
          }
        } // t % div
      }
    } // if (i == 4)

    // plot curves
    for (j=0; j<ptv->num_curves; j++)
    {
      if ( (ptv->mask & (1<<j)) == 0 ) continue;
      grph_setfgcolor(ptframe->bkdc, paramcolor[ptv->type[j]]);
      if (ptv->type[j] == PARAM_NIBP)
      {
        nibpv_t nv;
        for (k=0; k<tsc[ptframe->tsc]-1; k++)
        {
          if (k > ptframe->vlen) continue;
          nv = *((nibpv_t*)&ptv->data[j][k]);
          if (nv.ss != 0 && nv.dd != 0 && ptv->data[j][k] != TR_UNDEF_VALUE)
          {
            grph_line(ptframe->bkdc, TFRAME_LB+k*ww/tsc[ptframe->tsc], (int)(kk*nv.ss+bb), TFRAME_LB+k*ww/tsc[ptframe->tsc], (int)(kk*nv.dd+bb));

            grph_line(ptframe->bkdc, TFRAME_LB+k*ww/tsc[ptframe->tsc]-1, (int)(kk*nv.ss+bb), TFRAME_LB+k*ww/tsc[ptframe->tsc]+2, (int)(kk*nv.ss+bb));
            grph_line(ptframe->bkdc, TFRAME_LB+k*ww/tsc[ptframe->tsc]-1, (int)(kk*nv.dd+bb), TFRAME_LB+k*ww/tsc[ptframe->tsc]+2, (int)(kk*nv.dd+bb));
          }
        }
       // printf("read: ss=%d\n", nv.ss, ptv->data[j][tsc[ptframe->tsc]-1]);
       // continue;
      }
      else
      {
        for (k=0; k<tsc[ptframe->tsc]-1; k++)
        {
          if (k >= ptframe->vlen) break;
          if (ptv->data[j][k] != TR_UNDEF_VALUE && ptv->data[j][k+1] != TR_UNDEF_VALUE)
          {
            if (ptv->data[j][k] < ptv->vl || ptv->data[j][k] > ptv->vu) continue;
            if (ptv->data[j][k+1] < ptv->vl) ptv->data[j][k+1] = ptv->vl;
            if (ptv->data[j][k+1] > ptv->vu) ptv->data[j][k+1] = ptv->vu;
            grph_line(ptframe->bkdc, TFRAME_LB+k*ww/tsc[ptframe->tsc], (int)(kk*ptv->data[j][k]+bb), TFRAME_LB+(k+1)*ww/tsc[ptframe->tsc], (int)(kk*ptv->data[j][k+1]+bb) );
          }
        }
      }

      // draw curves' values in cursor pos
      grph_setfgcolor(ptframe->bkdc, paramcolor[ptv->type[j]]);
      rc.x0 = 10; //TFRAME_LB+ww+20;
      rc.y0 = yu+12*j;
      rc.x1 = rc.x0 + TFRAME_LB - 2 - 30; //rc.x0 + TFRAME_RB - 20 - 20;
      rc.y1 = rc.y0 + 10;
      if (ptframe->vpos > ptframe->vlen || ptv->data[j][ptframe->vpos] == TR_UNDEF_VALUE)
      {
        ids2string(tbl_par[ptv->type[j]].ids_name, s1);
        snprintf(s, sizeof(s), "%s   --", s1);
      }
      else if (ptv->kv == 1)
      {
        if (ptv->type[j] == PARAM_NIBP)
        {
          nibpv_t nibpv;
          nibpv = *((nibpv_t*) &(ptv->data[j][ptframe->vpos]));
          ids2string(tbl_par[ptv->type[j]].ids_name, s1);
          snprintf(s, sizeof(s), "%s   %d/%d (%d)", s1, nibpv.ss, nibpv.dd, nibpv.mm);
        }
        else
        {
          ids2string(tbl_par[ptv->type[j]].ids_name, s1);
          snprintf(s, sizeof(s), "%s   %d", s1, ptv->data[j][ptframe->vpos]);
         // grph_circle(ptframe->bkdc, TFRAME_LB+(int)((float)ptframe->vpos*ww/tsc[ptframe->tsc]), (int)(kk*ptv->data[j][ptframe->vpos]+bb), 2);
          grph_fillrect(ptframe->fgdc, TFRAME_LB+(int)((float)ptframe->vpos*ww/tsc[ptframe->tsc])-1, (int)(kk*ptv->data[j][ptframe->vpos]+bb)-1, 3, 3, paramcolor[ptv->type[j]]);
        }
      }
      else if (ptv->kv == 10)
      {
        ids2string(tbl_par[ptv->type[j]].ids_name, s1);
        snprintf(s, sizeof(s), "%s   %.01f", s1,  1.0f/ptv->kv*ptv->data[j][ptframe->vpos]);
      }
      else if (ptv->kv == 100)
      {
        ids2string(tbl_par[ptv->type[j]].ids_name, s1);
        snprintf(s, sizeof(s), "%s   %.02f", s1,  1.0f/ptv->kv*ptv->data[j][ptframe->vpos]);
      }
      else
      {
        ids2string(tbl_par[ptv->type[j]].ids_name, s1);
        snprintf(s, sizeof(s), "%s   --", s1);
      }
      grph_drawtext(ptframe->bkdc, s, -1, &rc, DT_LEFT);
    }
    nv ++;
  } // for (i..

  // draw cursor
  grph_setfgcolor(ptframe->bkdc, RGB(0xff,0x80,0x00));
  grph_line(ptframe->bkdc, TFRAME_LB+(int)((float)ptframe->vpos*ww/tsc[ptframe->tsc]), 0, TFRAME_LB+(int)((float)ptframe->vpos*ww/tsc[ptframe->tsc]), (TFRAME_CY-TRN_BUTTON_CY-6));

  // draw cursor date/time position
  trts = *((trts_t*) (&ptframe->view[4].data[0][ptframe->vpos]));
  grph_setfont(ptframe->bkdc, TFRAME_FONT_NORMAL);
  grph_setfgcolor(ptframe->bkdc, RGB(0xff,0xff,0xff));
  rc.x0 = TFRAME_LB+(int)((float)ptframe->vpos*ww/tsc[ptframe->tsc]);
  rc.x1 = rc.x0+36;
  rc.y0 = (TFRAME_CY-TRN_BUTTON_CY-10-TFRAME_BORDER_W)+3; //0+20;
  rc.y1 = rc.y0+14;
  if (trts.year != 0)
  {
    snprintf(s, sizeof(s), "%02d:%02d", trts.hour, trts.min);
    snprintf(s1, sizeof(s1), "%02d.%02d.%04d", trts.day, trts.mon, trts.year+1900);
  }
  else
  {
    snprintf(s, sizeof(s), "--:--");
    snprintf(s1, sizeof(s1), "--.--.----");
  }

  grph_textout(ptframe->bkdc, 40, TFRAME_CY-TRN_BUTTON_CY-10-40, s, strlen(s));
  grph_textout(ptframe->bkdc, 20, TFRAME_CY-TRN_BUTTON_CY-10-20, s1, strlen(s1));

  grph_setfgcolor(ptframe->bkdc, RGB(0xff,0x80,0x00));
  grph_setfont(ptframe->bkdc, ARIAL_N_12);
  if ( (TFRAME_LB+(int)((float)ptframe->vpos*ww/tsc[ptframe->tsc])+2) > (ptframe->cx - TFRAME_RB - (rc.x1-rc.x0)) )
  {
    grph_fillrect(ptframe->bkdc, rc.x0-(rc.x1-rc.x0)-1, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
   // snprintf(s, sizeof(s), "%02d:%02d", trts.hour, trts.min);
    rc.x1 = rc.x0;
    grph_drawtext(ptframe->bkdc, s, strlen(s), &rc, DT_RIGHT);
  }
  else
  {
    rc.x0 += 1;
    grph_fillrect(ptframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, 0x000000);
   // snprintf(s, sizeof(s), "%02d:%02d", trts.hour, trts.min);
    rc.x0 += 2;
    grph_drawtext(ptframe->bkdc, s, strlen(s), &rc, DT_LEFT);
  }

  if (demo_mode && ptframe->visible)
  {
    rect_t rect;
    char s[200];
    grph_setfont(ptframe->updc, ARIAL_60);
    grph_setfgcolor(ptframe->updc, RGB(0xEE,0xEE,0xEE));
    rect.x0 = 0;
    rect.x1 = rect.x0 + ptframe->cx;
    rect.y0 = ptframe->cy / 2 - 50;
    ids2string(IDS_DEMO_LARGE, s);
    grph_drawtext(ptframe->updc, s, -1, &rect, DT_CENTER);
  }

  tframe_bitblt(ptframe, 0, 0, ptframe->cx, ptframe->cy);
#endif
}

static void tframe_on_cursor_seek(int delta)
{
  tframe_t * ptframe;
  ptframe = &tframe;

  cursor_at_the_end = 0;
  ptframe->vpos += delta;

  if (ptframe->vpos < 0)
  {
    tframe_on_cursor_lseek(-1);
    ptframe->vpos = tsc[ptframe->tsc]/2;
  }

  if (ptframe->vpos > tsc[ptframe->tsc])
  {
    tframe_on_cursor_lseek(+1);
    ptframe->vpos = tsc[ptframe->tsc]/2;
  }

  tframe_on_draw();
}

static void tframe_on_cursor_seekend(void)
{
  tframe_t * ptframe;
  ptframe = &tframe;

  tr_seek_end(tsc[ptframe->tsc]);

  tframe_on_read();

  ptframe->vpos = min(ptframe->vlen, tsc[ptframe->tsc]-1);
  cursor_at_the_end = 1;

  tframe_on_draw();
}

static void tframe_on_cursor_lseek(int delta)
{
  tframe_t * ptframe;
  ptframe = &tframe;

  cursor_at_the_end = 0;
  tr_seek(delta*tsc[ptframe->tsc] / 2, SEEK_CUR);

  tframe_on_read();

  tframe_on_draw();
}

static void tframe_on_change_step(void)
{
  tframe_t * ptframe;
  ptframe = &tframe;

  ptframe->tsc ++;
  ptframe->tsc %= NUM_TSC;

  if (cursor_at_the_end)
  {
    tr_seek_end(tsc[ptframe->tsc]);
    tframe_on_read();
  }

  tframe_on_draw();

  tframe_cfg_save();
}

void tframe_on_resize(const int x, const int y, const int cx, const int cy)
{
  tframe_t * ptframe = &tframe;
 /* inpfoc_item_t * pifi;*/

  ptframe->x = x;
  ptframe->y = y;
  ptframe->cx = cx;
  ptframe->cy = cy;

  if (ptframe->cx <= 0)
    ptframe->cx += TFRAME_CX;
  if (ptframe->cy <= 0)
    ptframe->cy += TFRAME_CY;

 /* pifi = inpfoc_find(INPFOC_TRENDS, TRENDS_SET);
  if (pifi)
    inpfoc_wnd_move(pifi, ptframe->cx-102-5-60-5, TFRAME_CY-TRN_BUTTON_CY-1, TRUE);
  pifi = inpfoc_find(INPFOC_TRENDS, TRENDS_EXIT);
  if (pifi)
    inpfoc_wnd_move(pifi, ptframe->cx-60-5, TFRAME_CY-TRN_BUTTON_CY-1, TRUE);
*/
  if (!ptframe->visible) return;
  tframe_on_draw();
}

static void tframe_on_maximize(void)
{
  tframe_on_resize(TFRAME_X0, TFRAME_Y0, TFRAME_CX, TFRAME_CY);
}

static void tframe_on_reload(void)
{
  tframe_t * ptframe;

  ptframe = &tframe;
  if (!ptframe->visible) return;
 // tframe_on_read();
  tframe_on_draw();
}

static void tframe_on_newdata(void)
{
  tframe_t * ptframe;

  ptframe = &tframe;

  if (!ptframe->visible) return;

 // if (ptframe->vpos == tsc[ptframe->tsc]-1)
  if (cursor_at_the_end && ptframe->vlen >= tsc[ptframe->tsc]-1)
  {
    tr_seek(+1, SEEK_CUR);
  }

  tframe_on_read();

  tframe_on_draw();
}

void tframe_show(void)
{
  uframe_command(UFRAME_DESTROY, NULL);
  tframe_command(TFRAME_CREATE, NULL);
}

void tframe_hide(void)
{
  tframe_command(TFRAME_DESTROY, NULL);
}

void tframe_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case TFRAME_RESIZE:
      dview_command(TFRAME, cmd, 1, arg);
      break;
    default:
      dview_command(TFRAME, cmd, 0, arg);
      break;
  }
}

void tframe_on_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case TFRAME_CREATE:
      tframe_on_create();
      break;
    case TFRAME_DESTROY:
      tframe_on_destroy();
      break;
    case TFRAME_CURSOR_SEEK:
      tframe_on_cursor_seek(*((int*)arg));
      break;
    case TFRAME_CURSOR_SEEKEND:
      tframe_on_cursor_seekend();
      break;
    case TFRAME_CURSOR_LSEEK:
      tframe_on_cursor_lseek(*((int*)arg));
      break;
    case TFRAME_RESIZE:
      error("%s: line %d - add implementation\n", __FILE__, __LINE__);
      break;
    case TFRAME_MAXIMIZE:
      tframe_on_maximize();
      break;
    case TFRAME_CHANGE_STEP:
      tframe_on_change_step();
      break;
    case TFRAME_NEWDATA:
      tframe_on_newdata();
      break;
    case TFRAME_RELOAD:
      tframe_on_reload();
      break;
    default:
      error("%s: unknown cmd %d\n", __FUNCTION__, cmd);
      break;
  }
}

void tframe_cfg_save(void)
{
  tframe_t * ptframe = &tframe;
  int i;

  tframe_cfg_t tframe_cfg;

  tframe_cfg.visible = ptframe->visible;
  tframe_cfg.tsc = ptframe->tsc;
  for (i=0; i<TFRAME_MAX_NUM_VIEWS; i++)
  {
    tframe_cfg.view_cfg[i].visible = ptframe->view[i].visible;
    tframe_cfg.view_cfg[i].mask = ptframe->view[i].mask;
    tframe_cfg.view_cfg[i].num_curves = ptframe->view[i].num_curves;
    tframe_cfg.view_cfg[i].vl = ptframe->view[i].vl;
    tframe_cfg.view_cfg[i].vu = ptframe->view[i].vu;
    tframe_cfg.view_cfg[i].kv = ptframe->view[i].kv;
    memcpy(&tframe_cfg.view_cfg[i].type, &ptframe->view[i].type, TFRAME_MAX_NUM_CURVES*sizeof(int));
  }
  stconf_write(STCONF_TFRAME, (char*)&tframe_cfg, sizeof(tframe_cfg_t));
}

tframe_t * tframe_getptr(void)
{
  return &tframe;
}

