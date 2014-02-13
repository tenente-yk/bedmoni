#ifdef WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#ifdef UNIX
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include <errno.h>
#include <math.h>
#include <assert.h>

#include "grph.h"

#include "bedmoni.h"
#include "fifo.h"
#include "sched.h"
#include "filter.h"
#include "mainmenu.h"
#include "shm.h"
#include "cframe.h"
#include "mframe.h"
#include "uframe.h"
#include "dio.h"
#include "spo2.h"
#include "resp.h"
#include "inpfoc_wnd.h"
#include "dview.h"
#include "ids.h"
#include "lang.h"
#include "stconf.h"

#include "ecgset.h"
#include "spo2set.h"
#include "respset.h"
#include "co2set.h"

static float mmps[MAXNUM_MMPS] = {6.25, 12.5, 25.0, 50.0}; // 6.25, 12.5, 25, 50 [mm/sec]
static float my1[NUM_VIEWS] = 
{
  2.5f*MM2PIXY/1000, // ECG1
  2.5f*MM2PIXY/1000, // ECG2
  2.5f*MM2PIXY/1000, // ECG3
  2.5f*MM2PIXY/1000, // ECG4
  2.5f*MM2PIXY/1000, // ECG5
  2.5f*MM2PIXY/1000, // ECG6
  2.5f*MM2PIXY/1000, // ECG7
  0.0f,
  0.001f ,           // SpO2    ///<-- this value could be changed in code
  0.0f,
  0.25f,             // RESP
  0.0f,              // T1T2
  10.0f*MM2PIXY/100, // CO2
};

static unsigned char cur_mode = MODE_DIAGNOSTICS;

//static char *shm_ptr;

static int freeze_screen = 0;
static cframe_t cframe, *pcframe;
static int filt_5060Hz_id[NUM_VIEWS] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static int filt_150Hz_id[NUM_ECG] = {0,0,0,0,0,0,0};
static int filt_37Hz_id[NUM_ECG] = {0,0,0,0,0,0,0};
static int filt_hp_id[NUM_VIEWS] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static int filt_resp_id;
static int after_reset_countdown = 4;
//static unsigned short chd[NUM_VIEWS][CFRAME_CX*CFRAME_CY];
//static int chfd[NUM_VIEWS];

static void cframe_chandata_reset(cframe_t * pcframe);
static void cframe_chandata_update(cframe_t * pcframe);
static void frame_bitblt(int x, int y, int cx, int cy, void * parg);

static void cframe_cfg_save(void)
{
  cframe_t * pcframe;
  int i;
  cframe_cfg_t cframe_cfg;

  pcframe = &cframe;

  cframe_cfg.frame_visibility = pcframe->visible;
  cframe_cfg.mode = cur_mode;
  for (i=0; i<NUM_VIEWS; i++)
  {
    memcpy(&cframe_cfg.chandata[i], &pcframe->chanview[i].chandata, sizeof(chandata_t));
    cframe_cfg.view_visibility[i] = pcframe->chanview[i].visible;
  }
  stconf_write(STCONF_CFRAME, &cframe_cfg, sizeof(cframe_cfg_t));
}

static void cframe_freeze_icon(cframe_t * pcframe)
{
  assert(pcframe);

  // use updc as bkdc isn't cleared during frame_reset

  grph_setfgcolor(pcframe->updc, freeze_screen ? 0xCCCCCC : 0x000000);

  grph_fillrect(pcframe->updc, pcframe->cx-40+2, 14, 3, 12, freeze_screen ? 0xCCCCCC : 0x000000);
  grph_fillrect(pcframe->updc, pcframe->cx-40+9, 14, 3, 12, freeze_screen ? 0xCCCCCC : 0x000000);
 // grph_circle(pcframe->updc, pcframe->cx-40+7, 20, 12); // ?? not worked

  frame_bitblt(pcframe->cx-40, 14, 12, 12, pcframe);

//  grph_circle(pcframe->updc, pcframe->cx-40, 14, 12);


 // frame_bitblt(0, pcframe->y-7, 16, 16, pcframe);
//  frame_bitblt(0, 0, pcframe->cx, pcframe->cy, pcframe);
}

static void cframe_bk_update(cframe_t * pcframe, int clear, int paint)
{
  int w, h, i, r;
  int x0, y0;
  static int my_old = MY_1;
  static int h_old, y0_old;
  static int nv_old = 0;
  char s[200], s2[200];

  assert(pcframe);

  x0 = 8;
  w = 10;
  ids2string(IDS_MM_HG, s2);
  sprintf(s, "50 %s", s2);
  grph_setfont(pcframe->bkdc, MONOS_10);

  y0 = y0_old;
  h = h_old;

  if (clear)
  {
    grph_setfgcolor(pcframe->bkdc, CFRAME_BKCOLOR);
    grph_line(pcframe->bkdc, x0+0, y0, x0+5, y0);
    grph_line(pcframe->bkdc, x0+5, y0, x0+5, y0-h);
    grph_line(pcframe->bkdc, x0+5, y0-h, x0+w+5, y0-h);
    grph_line(pcframe->bkdc, x0+w+5, y0-h, x0+w+5, y0);
    grph_line(pcframe->bkdc, x0+w+5, y0, x0+w+5+5, y0);
   // grph_textout(pcframe->bkdc, x0+20, y0-14, s, strlen(s));
    grph_fillrect(pcframe->bkdc, x0+20, y0-14, 80, 16, CFRAME_BKCOLOR);

    // clear old signal borders
    int n = nv_old;
    int i;
    if (n > 1)
    {
//      grph_select_pen(pcframe->bkdc, PS_SOLID);
      int dy = (pcframe->cy-CFRAME_LABEL_DY_OFFS) / (n);
      grph_setfgcolor(pcframe->bkdc, CFRAME_BKCOLOR);
      for (i=0; i<n; i++)
      {
        grph_line(pcframe->bkdc, 0, CFRAME_LABEL_DY_OFFS+i*dy, CFRAME_CX, CFRAME_LABEL_DY_OFFS+i*dy);
        frame_bitblt(0, CFRAME_LABEL_DY_OFFS+i*dy, CFRAME_CX, CFRAME_LABEL_DY_OFFS+i*dy, pcframe);
      }
    }
  }

  if (pcframe->chanview[CO2].visible && unit_isenabled(CO2) && paint)
  {
    y0 = pcframe->chanview[CO2].y0+0;// - 4;
    h = 10*DISPLAY_CY/DISPLAY_CY_MM;
    if (pcframe->chanview[CO2].chandata.my < MY_2) h /= 2;

    grph_setfgcolor(pcframe->bkdc, chancolor[CO2]);
    grph_line(pcframe->bkdc, x0+0, y0, x0+5, y0);
    grph_line(pcframe->bkdc, x0+5, y0, x0+5, y0-h);
    grph_line(pcframe->bkdc, x0+5, y0-h, x0+w+5, y0-h);
    grph_line(pcframe->bkdc, x0+w+5, y0-h, x0+w+5, y0);
    grph_line(pcframe->bkdc, x0+w+5, y0, x0+w+5+5, y0);
#if 1
    grph_textout(pcframe->bkdc, x0+20, y0-14, s, strlen(s));
#endif

    h_old = h;
    y0_old = y0;
  }

  if (clear && paint && unit_isenabled(CO2) && pcframe->chanview[CO2].visible)
  {
    frame_bitblt(x0+20, y0-14, 80, 16, pcframe);
  }
 
  if (paint) // signal borders
  {
    int i, n = pcframe->num_visible_views;
    nv_old = n;
    if (n > 1)
    {
      grph_setfgcolor(pcframe->bkdc, RGB(0x44,0x44,0x44));
      grph_select_pen(pcframe->bkdc, PS_DOT);
      int dy = (pcframe->cy-CFRAME_LABEL_DY_OFFS) / (n);
#if 1
      for (i=0; i<n; i++)
      {
//printf("%d/%d -> %d\n", i, n, CFRAME_LABEL_DY_OFFS+i*dy);
        grph_line(pcframe->bkdc, 0, CFRAME_LABEL_DY_OFFS+i*dy, pcframe->cx, CFRAME_LABEL_DY_OFFS+i*dy);
        frame_bitblt(0, CFRAME_LABEL_DY_OFFS+i*dy, pcframe->cx, CFRAME_LABEL_DY_OFFS+i*dy, pcframe);
      }
#else
      for (i=0; i<NUM_VIEWS; i++)
      {
        if (!pcframe->chanview[i].visible) continue;
//printf("--> %d\n", pcframe->chanview[i].y0);
        grph_line(pcframe->bkdc, 0, pcframe->chanview[i].y0, pcframe->cx, pcframe->chanview[i].y0);
        frame_bitblt(0, pcframe->chanview[i].y0, pcframe->cx, pcframe->chanview[i].y0, pcframe);
      }
#endif
      grph_select_pen(pcframe->bkdc, PS_SOLID);
    }
  }

  x0 = 8;
  y0 = 20;
  w = 10;

#ifdef WIN32
#pragma warning (disable: 4244)
#endif

  // ecg metrics
  if (clear)
  {
    grph_setfgcolor(pcframe->bkdc, CFRAME_BKCOLOR);
    grph_line(pcframe->bkdc, x0+0, y0+1000*my1[ECG]*pow(2,my_old), x0+5, y0+1000*my1[ECG]*pow(2,my_old));
    grph_line(pcframe->bkdc, x0+5, y0+1000*my1[ECG]*pow(2,my_old), x0+5, y0+0);
    grph_line(pcframe->bkdc, x0+5, y0+0, x0+w+5, y0+0);
    grph_line(pcframe->bkdc, x0+w+5, y0+0, x0+w+5, y0+1000*my1[ECG]*pow(2,my_old));
    grph_line(pcframe->bkdc, x0+w+5, y0+1000*my1[ECG]*pow(2,my_old), x0+w+5+5, y0+1000*my1[ECG]*pow(2,my_old));
  }

  r = 0;
  for (i=ECG_1; i<=ECG_LAST; i++)
  {
    if (pcframe->chanview[i].visible) { r = 1; break; }
  }

 // grph_setfgcolor(pcframe->bkdc, RGB(0xff,0xff,0xff));
  if (r && unit_isenabled(ECG) && paint)
  {
    grph_setfgcolor(pcframe->bkdc, chancolor[ECG_1]);
    grph_line(pcframe->bkdc, x0+0, y0+1000*my1[ECG]*pow(2,pcframe->chanview[ECG].chandata.my), x0+5, y0+1000*my1[ECG]*pow(2,pcframe->chanview[ECG].chandata.my));
    grph_line(pcframe->bkdc, x0+5, y0+1000*my1[ECG]*pow(2,pcframe->chanview[ECG].chandata.my), x0+5, y0+0);
    grph_line(pcframe->bkdc, x0+5, y0+0, x0+w+5, y0+0);
    grph_line(pcframe->bkdc, x0+w+5, y0+0, x0+w+5, y0+1000*my1[ECG]*pow(2,pcframe->chanview[ECG].chandata.my));
    grph_line(pcframe->bkdc, x0+w+5, y0+1000*my1[ECG]*pow(2,pcframe->chanview[ECG].chandata.my), x0+w+5+5, y0+1000*my1[ECG]*pow(2,pcframe->chanview[ECG].chandata.my));
    my_old = pcframe->chanview[ECG].chandata.my;
  }
#ifdef WIN32
#pragma warning (default: 4244)
#endif
}

static void frame_bitblt(int x, int y, int cx, int cy, void *parg)
{
  cframe_t * pcframe = &cframe;
  dc_list_t dclist;

  assert(pcframe);

  dclist.xres = ((PGDC)(pcframe->bkdc))->xres;
  dclist.yres = ((PGDC)(pcframe->bkdc))->yres;
  dclist.p_bk = (unsigned char*)((PGDC)(pcframe->bkdc))->addr;
  dclist.p_fg = (unsigned char*)((PGDC)(pcframe->fgdc))->addr;
  dclist.p_up = (unsigned char*)((PGDC)(pcframe->updc))->addr;

//printf("cframe: %d %d %d %d %d %d %d\n", pcframe->x+x, pcframe->y+y, cx, cy, &dclist, x, y);

  if (x + cx >= pcframe->cx) cx = pcframe->cx-x;
  if (y + cy >= pcframe->cy) cy = pcframe->cy-y;

  dview_bitblt(pcframe->x+x, pcframe->y+y, cx, cy, &dclist, x, y);
}

static void frame_bitblt_fast(int x, int y, int cx, int cy, void *parg)
{
  cframe_t * pcframe = &cframe;
  dc_list_t dclist;

  assert(pcframe);

  dclist.xres = ((PGDC)(pcframe->bkdc))->xres;
  dclist.yres = ((PGDC)(pcframe->bkdc))->yres;
  dclist.p_bk = (unsigned char*)((PGDC)(pcframe->bkdc))->addr;
  dclist.p_fg = (unsigned char*)((PGDC)(pcframe->fgdc))->addr;
  dclist.p_up = (unsigned char*)((PGDC)(pcframe->updc))->addr;

#if 0
  unsigned char *pd, *ps;
  unsigned short w;//[CFRAME_CX];
  unsigned int xres = ((PSD)(pcframe->fgdc->psd))->xres;
  unsigned int bypp = (((PSD)(pcframe->fgdc->psd))->bpp + 7) / 8;
  int i, k;
  pd = (unsigned char*) ((PSD)(pcframe->fgdc->psd))->addr;
  for (i=0; i<cy; i++)
  {
    int j;
    memset(pd + xres*(y+i)*bypp + x*bypp, 0x00, cx*bypp);
    for (j=0; j<cx; j++)
    {
      w = 0x0000;
      for (k=0; k<NUM_VIEWS; k++)
      {
        if (!chno_is_ecg(k) && (k!=SPO2) && (k!=RESP) && (k!=ECG_NEO) && (k!=RPG)) continue;
        ps = (unsigned char*)chd[k];
        w |= *((unsigned short*)(ps + xres*(y+i)*bypp + (x+j)*bypp));
      }
      *((unsigned short*)(pd + xres*(y+i)*bypp + (x+j)*bypp)) = w;
    }
  }
#endif
//printf("%s %d %d %d %d %d %d\n", __FUNCTION__, pcframe->x+x, pcframe->y+y, cx, cy, x, y);

  dview_bitblt_fast(pcframe->x+x, pcframe->y+y, cx, cy, &dclist, x, y);
}

void cframe_init(void)
{
  int i;
  char s[200];
  int nv = 1;
  int shm_lock_res;
  inpfoc_wnd_dc_t ifwd;
  unsigned int curTick;
  shm_data_t shm_data;

  pcframe = &cframe;

  for (i=0; i<NUM_VIEWS; i++)
  {
    filt_5060Hz_id[i] = n5060_filt_init();
    switch (i)
    {
      case ECG_1:
      case ECG_2:
      case ECG_3:
      case ECG_4:
      case ECG_5:
      case ECG_6:
      case ECG_7:
        filt_hp_id[i] = hp_filt_init(0.08f);
        break;
      case RESP:
        filt_hp_id[i] = hp_filt_init(0.2f);    // equal to f = 0.02 Hz (as adcfreq = 50 Hz)
        n5060_filt_deinit(filt_5060Hz_id[i]);
        filt_5060Hz_id[i] = 0;
        break;
      case SPO2:
        filt_hp_id[i] = hp_filt_init(0.2f);    // f = 0.2 Hz
        break;
      case CO2:
        filt_hp_id[i] = 0;                     // do not perform filtr. for CO2
        break;
      default:
        filt_hp_id[i] = hp_filt_init(0.02f);   // f = 0.02 Hz for default
    }
  }

  for (i=0; i<NUM_ECG; i++)
  {
    filt_150Hz_id[i] = s150_filt_init();
    filt_37Hz_id[i] = s37_filt_init();
  }

  filt_resp_id = resp_filt_init(12.5);

  memset(pcframe, 0, sizeof(cframe_t));

  pcframe->visible = 1;
  pcframe->x    = CFRAME_X0;
  pcframe->y    = CFRAME_Y0;
  pcframe->cx   = CFRAME_CX;
  pcframe->cy   = CFRAME_CY;

  pcframe->num_cmds_in_queue = 0;

  for (i=0; i<NUM_VIEWS; i++)
  {
    pcframe->chanview[i].color = chancolor[i];
    pcframe->chanview[i].chandata.mmps  = mmps[2];
    pcframe->chanview[i].chandata.data_ptr  = 0;
    pcframe->chanview[i].chandata.old_tick  = gettimemillis();
    pcframe->chanview[i].chandata.my  = MY_1;
    pcframe->chanview[i].chandata.inv = 1;
    pcframe->chanview[i].y0  = 0;
    pcframe->chanview[i].chandata.no_data = 0;
    pcframe->chanview[i].num_inpfoc_items = 0;
    sprintf(pcframe->chanview[i].caption, "ch %d", i+1);
    pcframe->chanview[i].ids = IDS_UNDEF3;
  }

  pcframe->chanview[RESP].chandata.mmps  = mmps[0];
  pcframe->chanview[CO2].chandata.mmps  = mmps[0];

  pcframe->chanview[ECG_1].visible = 1;
  pcframe->chanview[ECG_2].visible = 1;
  pcframe->chanview[ECG_3].visible = 1;
  pcframe->chanview[ECG_4].visible = 0;
  pcframe->chanview[ECG_5].visible = 0;
  pcframe->chanview[ECG_6].visible = 0;
  pcframe->chanview[ECG_7].visible = 0;
  pcframe->chanview[SPO2].visible  = 1;
  pcframe->chanview[NIBP].visible  = 0;
  pcframe->chanview[RESP].visible = 1;
  pcframe->chanview[CO2].visible  = 0;

  pcframe->chanview[ECG_1].chandata.inv = -1;
  pcframe->chanview[ECG_2].chandata.inv = -1;
  pcframe->chanview[ECG_3].chandata.inv = -1;
  pcframe->chanview[ECG_4].chandata.inv = -1;
  pcframe->chanview[ECG_5].chandata.inv = -1;
  pcframe->chanview[ECG_6].chandata.inv = -1;
  pcframe->chanview[ECG_7].chandata.inv = -1;
  pcframe->chanview[SPO2].chandata.inv  = -1;
  pcframe->chanview[CO2].chandata.inv   = -1;

  assert(ECG_7 < NUM_ECG);

  pcframe->chanview[ECG_1].chandata.ecgtype = ECG_1;
  pcframe->chanview[ECG_2].chandata.ecgtype = ECG_2;
  pcframe->chanview[ECG_3].chandata.ecgtype = ECG_3;
  pcframe->chanview[ECG_4].chandata.ecgtype = ECG_4;
  pcframe->chanview[ECG_5].chandata.ecgtype = ECG_5;
  pcframe->chanview[ECG_6].chandata.ecgtype = ECG_6;
  pcframe->chanview[ECG_7].chandata.ecgtype = ECG_7;

  pcframe->chanview[SPO2].ids = IDS_SPO2;
  pcframe->chanview[RESP].ids = IDS_RESP;
  pcframe->chanview[CO2].ids = IDS_CO2;

  pcframe->chanview[SPO2].chandata.my    = MY_1;
  pcframe->chanview[RESP].chandata.my    = MY_1;
  pcframe->chanview[CO2].chandata.my     = MY_2;

  for (i=0; i<NUM_ECG; i++)
  {
    pcframe->chanview[i].chandata.my = MY_1;
  }
 // cframe_t cframe_cfg;
  cframe_cfg_t cframe_cfg;
  if (stconf_read(STCONF_CFRAME, (char*)&cframe_cfg, sizeof(cframe_cfg_t)) > 0)
  {
   // debug("read cfg cframe ok\n");
    pcframe->visible = cframe_cfg.frame_visibility;
    for (i=0; i<NUM_VIEWS; i++)
    {
      memcpy(&pcframe->chanview[i].chandata, &cframe_cfg.chandata[i], sizeof(chandata_t));
      pcframe->chanview[i].visible = cframe_cfg.view_visibility[i];
      cur_mode = cframe_cfg.mode;
    }
  }
  else
  {
    debug("read cfg cframe failed\n");
  }

  for (i=ECG_1; i<NUM_ECG; i++)
  {
    strcpy(pcframe->chanview[i].caption, ecgstr[pcframe->chanview[i].chandata.ecgtype]);
  }

  pcframe->num_visible_views = 0;
  for (i=0; i<NUM_VIEWS; i++)
  {
    if (pcframe->chanview[i].visible) pcframe->num_visible_views ++;
  }
  
  int cframe_cy = CFRAME_CY;
  if (pcframe->chanview[CO2].visible)
  {
#if 0 // commented in 2014 release 
    pcframe->num_visible_views --;
    cframe_cy = CFRAME_CY-20;
#else
  // new in 2014 release
    cframe_cy = CFRAME_CY;
#endif
  }
  for (i=0, nv=1; i<NUM_VIEWS; i++)
  {
    int k;
    if (pcframe->chanview[i].visible)
    {
      int dy = 0;
      if (pcframe->num_visible_views)
        dy = (cframe_cy-CFRAME_LABEL_DY_OFFS) / (pcframe->num_visible_views);
     // pcframe->chanview[i].y0 = CFRAME_LABEL_DY_OFFS + nv * dy;
      pcframe->chanview[i].y0 = CFRAME_LABEL_DY_OFFS + dy/2 + (nv-1) * dy;
//      if (chno_is_ecg(i))
//        pcframe->chanview[i].y0 += dy / 4;
//printf("y0[%d]=%d (%d, %d)\n", i,  pcframe->chanview[i].y0, pcframe->num_visible_views, cframe_cy);
      nv += 1;
    }
    for (k=0; k<sizeof(pcframe->chanview[i].yu) / sizeof(int); k++)
    {
      pcframe->chanview[i].yu[k] = CFRAME_NOVAL; //-1;
    }
    for (k=0; k<sizeof(pcframe->chanview[i].yl) / sizeof(int); k++)
    {
      pcframe->chanview[i].yl[k] = CFRAME_NOVAL; //-1;
    }
  }
  if (pcframe->chanview[CO2].visible)
  {
    pcframe->chanview[CO2].y0 = pcframe->cy - 20;
   // pcframe->num_visible_views ++; // comment in 2014 release
  }

  if (pcframe->chanview[SPO2].chandata.my == MY_AUTO)
  {
    my1[SPO2] = (float)pcframe->cy/(pcframe->num_visible_views+1)/0xFFF; // fit autoscaled SPO2 into channel window
  }

  pcframe->bkdc = grph_createdc(rootwfd, 0, 0, pcframe->cx, pcframe->cy, 0);
  pcframe->fgdc = grph_createdc(rootwfd, 0, 0, pcframe->cx, pcframe->cy, 0);
  pcframe->updc = grph_createdc(rootwfd, 0, 0, pcframe->cx, pcframe->cy, 0);

  if (pcframe->visible)
  {
    grph_fillrect(pcframe->bkdc, 0, 0, pcframe->cx, pcframe->cy, CFRAME_BKCOLOR);
    grph_fillrect(pcframe->fgdc, 0, 0, pcframe->cx, pcframe->cy, 0x000000);
    grph_fillrect(pcframe->updc, 0, 0, pcframe->cx, pcframe->cy, 0x000000);

   // cframe_bk_update(pcframe);
  }
  curTick = gettimemillis();

  memset(&shm_data, 0, sizeof(shm_data_t));
  shm_lock_res = shm_lock(sizeof(shm_data_t));
  if (shm_lock_res > 0) // restart_mode
  {
    debug("data reloaded from shm\n");
    shm_read(&shm_data);//, sizeof(shm_data), shm_ptr);
    for (i=0; i<NUM_VIEWS; i++)
      shm_data.old_tick[i] = curTick;
#if 0 // TEMPORARY !!!!!!!!!!!!!!!!!!!!
    shm_write(shm_ptr, (char*)&shm_data, sizeof(shm_data));
#endif
  }
  else if (shm_lock_res == 0) // new connection
  {
    for (i=0; i<NUM_VIEWS; i++)
    {
      shm_data.dview_data_ptr[i] = 0;
      shm_data.old_tick[i] = curTick;
    }
   // shm_write(shm_ptr, (char*)&shm_data, sizeof(shm_data));
    shm_write(&shm_data);
   // printf("NEWWWWWWWWWWWWWW\n");
  }
  else // shm_lock error
  {
    error("shm_lock error\n");
  }

#if 0
  for (i=0; i<NUM_VIEWS; i++)
  {
    shm_data.dview_data_ptr[i] = 0;
    shm_data.old_tick[i] = curTick;
  }
#endif

  ifwd.bkdc = pcframe->bkdc;
  ifwd.updc = pcframe->updc;
  ifwd.fgcolor = chancolor[ECG];
  ifwd.bkcolor = CFRAME_BKCOLOR;

  for (i=0; i<NUM_ECG; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      ifwd.fgcolor = pcframe->chanview[ECG_1+i].color;
      inpfoc_ins(INPFOC_MAIN, MAIN_ECG_1+i, (void*) inpfoc_wnd_create("LIST", 25, pcframe->chanview[ECG_1+i].y0-CFRAME_LABEL_DY_OFFS, 38, 20, pcframe->chanview[ECG_1+i].caption, 0, &ifwd, frame_bitblt, pcframe), 0);
    }
    pcframe->chanview[ECG_1+i].inpfoc_item[0] = MAIN_ECG_1+i;

    pcframe->chanview[i].inpfoc_item[1] = MAIN_ECG_AMP;
    pcframe->chanview[i].num_inpfoc_items = 2;
   // pcframe->chanview[i].num_inpfoc_items = 1;
  }

  for (i=ECG_1; i<NUM_ECG; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      ifwd.fgcolor = pcframe->chanview[i].color;

      if (pcframe->chanview[i].chandata.my > MY_1)
        sprintf(s, "%d ", (int)(pow(2,pcframe->chanview[i].chandata.my) * 2.5f));
      else
        sprintf(s, "%.1f ", (float) 2.5f * (int)pow(2,pcframe->chanview[i].chandata.my));
      inpfoc_add(INPFOC_MAIN, MAIN_ECG_AMP, (void*) inpfoc_wnd_create("LIST", 65, 15, 80, 20, s, IDS_MMPMV, &ifwd, frame_bitblt, pcframe));

      inpfoc_add(INPFOC_MAIN, MAIN_MODE, (void*) inpfoc_wnd_create("LIST", 150, 15, 130, 20, 0, IDS_UNDEF7, &ifwd, frame_bitblt, pcframe));

      break;
    }
  }

  pcframe->chanview[SPO2].num_inpfoc_items = 2;
  pcframe->chanview[SPO2].inpfoc_item[0] = MAIN_SPO2_C;
  pcframe->chanview[SPO2].inpfoc_item[1] = MAIN_SPO2_AMP;
  if (pcframe->chanview[SPO2].visible)
  {
    ifwd.fgcolor = pcframe->chanview[SPO2].color;
    inpfoc_ins(INPFOC_MAIN, MAIN_SPO2_C, (void*) inpfoc_wnd_create("PUSH", 25, pcframe->chanview[SPO2].y0-CFRAME_LABEL_DY_OFFS, 40, 20, 0, pcframe->chanview[SPO2].ids, &ifwd, frame_bitblt, pcframe), 0);
    if (pcframe->chanview[SPO2].chandata.my != MY_AUTO)
    {
      sprintf(s, "x%d", (int)(pow(2,pcframe->chanview[SPO2].chandata.my)));
      inpfoc_ins(INPFOC_MAIN, MAIN_SPO2_AMP, (void*) inpfoc_wnd_create("LIST", 70, pcframe->chanview[SPO2].y0-CFRAME_LABEL_DY_OFFS, 80, 20, s, 0, &ifwd, frame_bitblt, pcframe), 0);
    }
    else
    {
      inpfoc_ins(INPFOC_MAIN, MAIN_SPO2_AMP, (void*) inpfoc_wnd_create("LIST", 70, pcframe->chanview[SPO2].y0-CFRAME_LABEL_DY_OFFS, 80, 20, 0, IDS_AUTO, &ifwd, frame_bitblt, pcframe), 0);
    }
  }
  if (pcframe->chanview[SPO2].chandata.my == MY_AUTO)
    dio_module_cmd(PD_ID_SPO2, SPO2_SET_SCALE, SPO2_AUTOSCALE);
  else
    dio_module_cmd(PD_ID_SPO2, SPO2_SET_SCALE, SPO2_NOAUTOSCALE);

  pcframe->chanview[RESP].num_inpfoc_items = 2;
  pcframe->chanview[RESP].inpfoc_item[0] = MAIN_RESP_C;
  pcframe->chanview[RESP].inpfoc_item[1] = MAIN_RESP_AMP;
  if (pcframe->chanview[RESP].visible)
  {
    ifwd.fgcolor = pcframe->chanview[RESP].color;
    inpfoc_ins(INPFOC_MAIN, MAIN_RESP_C, (void*) inpfoc_wnd_create("PUSH", 25, pcframe->chanview[RESP].y0-CFRAME_LABEL_DY_OFFS, 40, 20, 0, pcframe->chanview[RESP].ids, &ifwd, frame_bitblt, pcframe), 0);
    sprintf(s, "x%d", (int)(pow(2,pcframe->chanview[RESP].chandata.my)) );
    inpfoc_ins(INPFOC_MAIN, MAIN_RESP_AMP, (void*) inpfoc_wnd_create("LIST", 70, pcframe->chanview[RESP].y0-CFRAME_LABEL_DY_OFFS, 80, 20, s, 0, &ifwd, frame_bitblt, pcframe), 0);
  }

  pcframe->chanview[CO2].num_inpfoc_items = 2;
  pcframe->chanview[CO2].inpfoc_item[0] = MAIN_CO2_C;
  pcframe->chanview[CO2].inpfoc_item[1] = MAIN_CO2_AMP;
  if (pcframe->chanview[CO2].visible)
  {
    ifwd.fgcolor = pcframe->chanview[CO2].color;
    inpfoc_ins(INPFOC_MAIN, MAIN_CO2_C, (void*) inpfoc_wnd_create("PUSH", 25, pcframe->chanview[CO2].y0-CFRAME_LABEL_DY_OFFS, 40, 20, 0, pcframe->chanview[CO2].ids, &ifwd, frame_bitblt, pcframe), 0);
    sprintf(s, "x%d", (int)(pow(2,pcframe->chanview[CO2].chandata.my)) );
    inpfoc_ins(INPFOC_MAIN, MAIN_CO2_AMP, (void*) inpfoc_wnd_create("LIST", 70, pcframe->chanview[CO2].y0-CFRAME_LABEL_DY_OFFS, 80, 20, s, 0, &ifwd, frame_bitblt, pcframe), 0);
  }

  for (i=0; i<NUM_ECG; i++)
  {
    if (pcframe->chanview[i].visible) break;
  }
  if (i == NUM_ECG)
    inpfoc_del(INPFOC_MAIN, MAIN_MODE);

  if (demo_mode)
  {
    rect_t rect;
    grph_setfont(pcframe->updc, ARIAL_60);
    grph_setfgcolor(pcframe->updc, RGB(0xEE,0xEE,0xEE));
    rect.x0 = 50;
    rect.x1 = rect.x0 + CFRAME_CX;
    rect.y0 = CFRAME_CY / 2 - 50;
    ids2string(IDS_DEMO_LARGE, s);
    grph_drawtext(pcframe->updc, s, -1, &rect, DT_CENTER);
  }

  if (!pcframe->visible)
    cframe_on_hide();

 // inpfoc_set(INPFOC_MAIN, MAIN_MODE);
 // inpfoc_set(INPFOC_MAIN, -1); // set first in inpfoc list

  cframe_command(CFRAME_RESET, NULL);
 // frame_bitblt(0, 0, pcframe->cx, pcframe->cy, pcframe);
}

void cframe_deinit(void)
{
  int i;
  cframe_cfg_t cframe_cfg;

  assert(pcframe);

  for (i=0; i<NUM_VIEWS; i++)
  {
    n5060_filt_deinit(filt_5060Hz_id[i]);
    hp_filt_deinit(filt_hp_id[i]);
  }

  for (i=0; i<NUM_ECG; i++)
  {
    s150_filt_deinit(filt_150Hz_id[i]);
    s37_filt_deinit(filt_37Hz_id[i]);
  }

  resp_filt_deinit(filt_resp_id);

  cframe_cfg.frame_visibility = pcframe->visible;
  cframe_cfg.mode = cur_mode;
  for (i=0; i<NUM_VIEWS; i++)
  {
    memcpy(&cframe_cfg.chandata[i], &pcframe->chanview[i].chandata, sizeof(chandata_t));
    cframe_cfg.view_visibility[i] = pcframe->chanview[i].visible;
  }
  stconf_write(STCONF_CFRAME, &cframe_cfg, sizeof(cframe_cfg_t));

  inpfoc_rm(INPFOC_MAIN);

  shm_unlock();

  // deinit chfd[i]

  grph_releasedc(pcframe->bkdc);
  grph_releasedc(pcframe->fgdc);
  grph_releasedc(pcframe->updc);
}

void cframe_on_resize(const int x0, const int y0, const int cx, const int cy)
{
  cframe_t * pcframe;
  int v;

  pcframe = cframe_getptr();
  assert(pcframe);
  v = pcframe->visible;
  pcframe->visible = 0;

  pcframe->x    = x0;
  pcframe->y    = y0;
  pcframe->cx   = cx;
  pcframe->cy   = cy;

  pcframe->visible = v;
  if (!pcframe->visible) return;

  cframe_chandata_reset(pcframe);
}

static void cframe_on_reload(cframe_t * pcframe)
{
  if (mframe_ismaximized()) return;
 // cframe_bk_update(pcframe, TRUE, TRUE); // 15.12.2011
  cframe_on_show();
}

void cframe_update_fast(void)
{
  assert(pcframe);

 // if (!pcframe->visible) return;
  cframe_chandata_update(pcframe);
}

volatile int stop_fast_data_flag = 0;

static void cframe_chandata_update(cframe_t * pcframe)
{
  shm_data_t shm_data;
  int i, j, num[NUM_VIEWS];
  int buf[NUM_VIEWS][DVIEW_MAX_FIFO_SIZE];
  unsigned char pm[DVIEW_MAX_FIFO_SIZE];
  static int v_old[NUM_VIEWS] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
  unsigned int curTick;
  double fHPixPerMm;
  unsigned long dwTimeMS;
  unsigned int nPoints;
  double PixelPerSecond;
  double PixelPerPoint;
  double d_ptr;
  int x0u[NUM_VIEWS], x1u[NUM_VIEWS];
  unsigned char x_update[CFRAME_CX];
 // float ky[NUM_VIEWS];
  int chno;
  tprn_cfg_t tprn_cfg;

  if (stop_fast_data_flag)
  {
    after_reset_countdown = 5;
    return;
  }

  if (!pcframe) return;

  curTick = gettimemillis();
  shm_read(&shm_data);

  if (after_reset_countdown)
  {
    after_reset_countdown--;
    for (j=0; j<NUM_VIEWS; j++)
    {
      shm_data.dview_data_ptr[j] = 0;
      shm_data.old_tick[j] = curTick;
      fifo_pop(&fifo_dview_chan[j], DVIEW_MAX_FIFO_SIZE*sizeof(int));
    }
    shm_write(&shm_data);
    return;
  }

  fHPixPerMm = (double)(DISPLAY_CX) / DISPLAY_CX_MM;
  tprn_cfg_get(&tprn_cfg);

  for (j=0; j<NUM_VIEWS; j++)
  {
    x0u[j] = x1u[j] = 0;
//    ky[j] = 1;
//    if (chno_is_ecg(j)) ky[j] = 0.4f; // 1000*0.05 (1mV * MX1) -> 20 (PIXPERMV)
    num[j] = fifo_avail(&fifo_dview_chan[j]);
    num[j] /= sizeof(int);
    fifo_get_bytes(&fifo_dview_chan[j], (unsigned char*)buf[j], sizeof(int)*num[j]);

    // filtration
    for (i=0; i<num[j]; i++)
    {
      if (chno_is_ecg(j))
      {
        pm[i] = (unsigned char) HIWORD(buf[j][i]);
        buf[j][i] = (signed short) LOWORD(buf[j][i]);
      }
      buf[j][i] = hp_filt_stepit(filt_hp_id[j], buf[j][i]);
      if (cur_mode != MODE_DIAGNOSTICS && cur_mode != MODE_DIAGNOSTICS_F)
      {
        buf[j][i] = n5060_filt_stepit(filt_5060Hz_id[j], buf[j][i]);
      }
      if (chno_is_ecg(j) && (cur_mode == MODE_DIAGNOSTICS_F/* || cur_mode == MODE_MONITORING*/))
      {
        buf[j][i] = s37_filt_stepit(filt_37Hz_id[j], buf[j][i]);
      }
      if (chno_is_ecg(j) && cur_mode == MODE_DIAGNOSTICS)
      {
        buf[j][i] = s150_filt_stepit(filt_150Hz_id[j], buf[j][i]);
      }
      if (j == RESP)
      {
        buf[j][i] = resp_filt_stepit(filt_resp_id, buf[j][i]);
      }
    }
  }

  for (j=0; j<NUM_VIEWS; j++)
  {
    chanview_t * pcv = &pcframe->chanview[j];
    chandata_t * pcd = &pcframe->chanview[j].chandata;

    if (num[j] < 1)
      continue;

    pcd->data_ptr = shm_data.dview_data_ptr[j];
    if (pcd->data_ptr > pcframe->cx) pcd->data_ptr = 0;

    if (!pcv->visible) continue;

    chno = (chno_is_ecg(j)) ? pcd->ecgtype : j;

    dwTimeMS = curTick - shm_data.old_tick[j];
    nPoints = num[chno];
    PixelPerSecond = (double)pcd->mmps * fHPixPerMm;
    PixelPerPoint = PixelPerSecond * ((double)dwTimeMS / nPoints / 1000);

    if (PixelPerPoint > 50)
    {
      cframe_chandata_reset(pcframe);
      return;
    }

//    if (chno_is_ecg(j)) any_ecg_visible ++;
 // printf("%d %d %d %f %d\n", j, dwTimeMS, nPoints, (float)pcd->data_ptr, buf[j]);
 // printf("%f %f %f\n", (float)PixelPerSecond, (float)PixelPerPoint, (float)pcd->mmps);

//if (j==0) printf("%d %d %d %f %d %f %f\n", num[j], dwTimeMS, nPoints, (float)pcd->data_ptr, buf[j], (float)PixelPerPoint, (float)(nPoints*PixelPerPoint));

    grph_setfgcolor(pcframe->fgdc/*chfd[i]*/, 0x000000);
    // do not clear current point (it was cleared before (... + 4))
    i = 0;
   // while ((int)(pcd->data_ptr + i*PixelPerPoint) == (int)pcd->data_ptr) i++;
   // while ((int)pcd->data_ptr + i == (int)pcd->data_ptr) i++;
    i = 1;

    d_ptr = pcd->data_ptr;

    x0u[j] = x1u[j] = (int)d_ptr;
    x1u[j] += (int)((nPoints+0)*PixelPerPoint + 4);
    x1u[j] = x1u[j] - (int)(x1u[j]/pcframe->cx)*pcframe->cx;

//printf("(int)((nPoints+0)*PixelPerPoint + 4)=%d\n", (int)((nPoints+0)*PixelPerPoint + 4));

#if 1
    int yu, yl;
    for(i=4-1; i<(int)((nPoints+0)*PixelPerPoint + 4); i++)
    {
      d_ptr = d_ptr - ((int)(d_ptr+i) / pcframe->cx) * pcframe->cx;

      yu = min(pcv->yu[(int)(d_ptr+i)], pcv->yu[(int)(d_ptr+i)+1]);
     // yu = min(pcv->yu[(int)(d_ptr+i)], pcv->yu[(int)(d_ptr+i)-1]);
      yl = max(pcv->yl[(int)(d_ptr+i)], pcv->yl[(int)(d_ptr+i)+1]);
     // yl = max(pcv->yl[(int)(d_ptr+i)], pcv->yl[(int)(d_ptr+i)-1]);
     // printf("%d %d %d %d %d %d\n", i, (int)(d_ptr+i), pcv->yu[(int)(d_ptr+i)], pcv->yl[(int)(d_ptr+i)], yu, yl);
      if (yu == CFRAME_NOVAL || yl == CFRAME_NOVAL)
      {
        // !!!
        //printf("opa!!!\n");
        continue;
      }
      grph_line(pcframe->fgdc/*chfd[j]*/, (int)(d_ptr+i), yu-1, (int)(d_ptr+i), yl+1);
      grph_line(pcframe->fgdc/*chfd[j]*/, ((int)(d_ptr+i)+1)%pcframe->cx, yu-1, ((int)(d_ptr+i)+1)%pcframe->cx, yl+1);
      // clear traces of pace maker
      if (chno_is_ecg(j)) //(j >= ECG_1 && j <= ECG_LAST)
      {
        grph_line(pcframe->fgdc/*chfd[j]*/, 0+(int)(d_ptr+i), 0, 0+(int)(d_ptr+i), 8);
      }
      pcv->yu[(int)(d_ptr+i)] = CFRAME_NOVAL;
      pcv->yl[(int)(d_ptr+i)] = CFRAME_NOVAL;
    }
#endif
  } // for (j<NUM_VIEWS)

  for (j=0; j<NUM_VIEWS; j++)
  {
    chanview_t * pcv = &pcframe->chanview[j];
    chandata_t * pcd = &pcframe->chanview[j].chandata;

    if (num[j] < 1)
      continue;

    pcd->data_ptr = shm_data.dview_data_ptr[j];
    if (pcd->data_ptr > pcframe->cx) pcd->data_ptr = 0;

    chno = (chno_is_ecg(j)) ? pcd->ecgtype : j;

    dwTimeMS = curTick - shm_data.old_tick[j];
    nPoints = num[chno];
    PixelPerSecond = (double)pcd->mmps * fHPixPerMm;
    PixelPerPoint = PixelPerSecond * ((double)dwTimeMS / nPoints / 1000);

    if ( j == tprn_cfg.c[0] && ((chno_is_ecg(j))?unit_isenabled(ECG):unit_isenabled(j)) ) tprn_push_data(0, buf[j], nPoints, dwTimeMS);
    if ( j == tprn_cfg.c[1] && ((chno_is_ecg(j))?unit_isenabled(ECG):unit_isenabled(j)) ) tprn_push_data(1, buf[j], nPoints, dwTimeMS);

//if ( j == tprn_cfg.c[1] && ((chno_is_ecg(j))?unit_isenabled(ECG):unit_isenabled(j)) )
//{
// printf("ok %d\n", buf[j][0]);
//}
  /*  if ( j == tprn_cfg.c[1] && ((chno_is_ecg(j))?unit_isenabled(ECG):unit_isenabled(j)) )
    {
      static signed char pila = 0;
      int i;
      for (i=0; i<nPoints; i++)
        buf[j][i] = pila++;
      tprn_push_data(1, buf[j], nPoints, dwTimeMS);
    }*/

    shm_data.old_tick[j] = curTick; // TPRN fix (for invisible channels to be printed)

    if (!pcv->visible) continue;

    grph_setfgcolor(pcframe->fgdc/*chfd[j]*/, pcv->color);
    //while (0)
    for (i=0; (unsigned int)i<nPoints; i++)
    {
      if (pcd->no_data)
      {
        debug("chno %d no data\n", j);
        break;
      }

      if ((int)pcd->data_ptr+(int)((i+0)*PixelPerPoint) >= pcframe->cx)
      {
       // pcd->data_ptr -= pcframe->cx;
        pcd->data_ptr = pcd->data_ptr - (int)((pcd->data_ptr+(int)((i+0)*PixelPerPoint)) / pcframe->cx) * pcframe->cx;
      }

      //if ((int)(pcd->data_ptr + i*PixelPerPoint) != (int)(pcd->data_ptr + (i+1)*PixelPerPoint))
      {
#if 1
#if 0
        int ptr;
        double myy;

        ptr = 0+(int)(pcd->data_ptr+((i+0)*PixelPerPoint));
        myy = my1[j]*pow(2,pcd->my);
        if (pcv->yu[ptr] == CFRAME_NOVAL)
        {
          pcv->yu[ptr] = (int)(pcv->y0+v_old[chno] * myy * pcd->inv);
          pcv->yl[ptr] = (int)(pcv->y0+v_old[chno] * myy * pcd->inv);
        }
        else
        {
          pcv->yu[ptr] = min( (int)(pcv->y0+v_old[chno] * myy * pcd->inv), pcv->yu[ptr]);
          pcv->yl[ptr] = max( (int)(pcv->y0+v_old[chno] * myy * pcd->inv), pcv->yl[ptr]);
        }
#endif
        if (pcv->yu[0+(int)(pcd->data_ptr+((i+0)*PixelPerPoint))] == CFRAME_NOVAL)
        {
          pcv->yu[0+(int)(pcd->data_ptr+((i+0)*PixelPerPoint))] = (int)(pcv->y0+v_old[chno] * my1[j]*pow(2,pcd->my) * pcd->inv/* * ky[j]*/);
          pcv->yl[0+(int)(pcd->data_ptr+((i+0)*PixelPerPoint))] = (int)(pcv->y0+v_old[chno] * my1[j]*pow(2,pcd->my) * pcd->inv/* * ky[j]*/);
        }
        else
        {
          pcv->yu[0+(int)(pcd->data_ptr+((i+0)*PixelPerPoint))] = min( (int)(pcv->y0+v_old[chno] * my1[j]*pow(2,pcd->my) * pcd->inv/* * ky[j]*/), pcv->yu[0+(int)(pcd->data_ptr+((i+0)*PixelPerPoint))]);
          pcv->yl[0+(int)(pcd->data_ptr+((i+0)*PixelPerPoint))] = max( (int)(pcv->y0+v_old[chno] * my1[j]*pow(2,pcd->my) * pcd->inv/* * ky[j]*/), pcv->yl[0+(int)(pcd->data_ptr+((i+0)*PixelPerPoint))]);
        }
        if (pcv->yu[0+(int)(pcd->data_ptr+((i+1)*PixelPerPoint))] == CFRAME_NOVAL)
        {
          pcv->yu[0+(int)(pcd->data_ptr+((i+1)*PixelPerPoint))] = (int)(pcv->y0+buf[chno][i] * my1[j]*pow(2,pcd->my) * pcd->inv/* * ky[j]*/);
          pcv->yl[0+(int)(pcd->data_ptr+((i+1)*PixelPerPoint))] = (int)(pcv->y0+buf[chno][i] * my1[j]*pow(2,pcd->my) * pcd->inv/* * ky[j]*/);
        }
        else
        {
          pcv->yu[0+(int)(pcd->data_ptr+((i+1)*PixelPerPoint))] = min( (int)(pcv->y0+buf[chno][i] * my1[j]*pow(2,pcd->my) * pcd->inv/* * ky[j]*/), pcv->yu[0+(int)(pcd->data_ptr+((i+1)*PixelPerPoint))]);
          pcv->yl[0+(int)(pcd->data_ptr+((i+1)*PixelPerPoint))] = max( (int)(pcv->y0+buf[chno][i] * my1[j]*pow(2,pcd->my) * pcd->inv/* * ky[j]*/), pcv->yl[0+(int)(pcd->data_ptr+((i+1)*PixelPerPoint))]);
        }
#endif

        // pacemaker
        if (chno_is_ecg(j))
        {
          if (pm[i])
          {
            int xpm;
            xpm = 0+(int)(pcd->data_ptr+(i+0)*PixelPerPoint)-10*PixelPerPoint; // because of 10 points delay for pm mark
           // xpm = 0+(int)(pcd->data_ptr+(i+0)*PixelPerPoint);
            if (xpm < 0) xpm += pcframe->cx;
            x0u[j] = xpm;
            grph_setfgcolor(pcframe->fgdc/*chfd[j]*/, RGB(0xff,0xff,0xff));
           // grph_line(pcframe->fgdc/*chfd[j]*/, 0+(int)(pcd->data_ptr+(i+0)*PixelPerPoint), 0, 0+(int)(pcd->data_ptr+(i+0)*PixelPerPoint), 8);
            grph_line(pcframe->fgdc/*chfd[j]*/, xpm, 0,  xpm, 8);
            grph_setfgcolor(pcframe->fgdc/*chfd[j]*/, pcv->color);
          }
        }

#if defined (CFRAME_CHANNEL_BOUNDS)
        int dy = 0;
        if (pcframe->num_visible_views)
          dy = (pcframe->cy-CFRAME_LABEL_DY_OFFS) / (pcframe->num_visible_views);
        int u_border = pcframe->chanview[chno].y0 - dy / 2;
        int l_border = pcframe->chanview[chno].y0 + dy / 2;
        if (chno == CO2)
        {
          u_border = pcframe->chanview[chno].y0 - dy + 20;
          l_border = pcframe->cy - 20;
        }
        grph_line_bound(pcframe->fgdc,
#else
        grph_line(pcframe->fgdc/*chfd[j]*/,
#endif
                  0+(int)(pcd->data_ptr+(i+0)*PixelPerPoint),
                  (int)(pcv->y0+v_old[chno] * my1[j]*pow(2,pcd->my) * pcd->inv),
                  0+(int)(pcd->data_ptr+(i+1)*PixelPerPoint),
                  (int)(pcv->y0+buf[chno][i] * my1[j]*pow(2,pcd->my) * pcd->inv)
#if defined (CFRAME_CHANNEL_BOUNDS)
                  , u_border,
                  l_border
#endif
                 );
        v_old[chno] = buf[chno][i];
      }
#if 1
      if (- (int)(pcd->data_ptr + i*PixelPerPoint) + (int)(pcd->data_ptr + (i+1)*PixelPerPoint) > 1)
      {
        int x;
        for (x=(int)(pcd->data_ptr + i*PixelPerPoint)+1; x<(int)(pcd->data_ptr + (i+1)*PixelPerPoint); x++)
        {
          if (x >= pcframe->cx) break;
//printf("x %d\n", x);
/*if (x>DISPLAY_CX)
{
printf("x=%d\n", x);
}*/
          pcv->yu[x] = min( pcv->yu[(int)(pcd->data_ptr + i*PixelPerPoint)], pcv->yu[(int)(pcd->data_ptr + (i+1)*PixelPerPoint)] );
          pcv->yl[x] = max( pcv->yl[(int)(pcd->data_ptr + i*PixelPerPoint)], pcv->yl[(int)(pcd->data_ptr + (i+1)*PixelPerPoint)] );
          if (x>sizeof(pcframe->chanview[j].yu) / sizeof(int)) break;
        }
      }
#endif
    }

    pcd->data_ptr +=  ((double)nPoints+0)*PixelPerPoint;

//printf("%d %f %f %f\n", j, (float)pcd->data_ptr, (float)PixelPerPoint, (float) (((double)nPoints+0)*PixelPerPoint) );

    pcd->data_ptr = pcd->data_ptr - (int)(pcd->data_ptr / pcframe->cx) * pcframe->cx;

    shm_data.dview_data_ptr[j] = pcd->data_ptr;
    shm_data.old_tick[j] = curTick;
  } // j: dview_num_channels

  // do not output on screen if cframe is disabled or in freeze mode
  if (!freeze_screen && pcframe->visible)
  {
    memset(x_update, 0, sizeof(x_update)); // sizeof(x_update) = pcframe->cx <= CFRAME_CX !!!
    for (j=0; j<NUM_VIEWS; j++)
    {
      int cx;

      if (x1u[j] >= x0u[j])
      {
        cx = min( x1u[j]-x0u[j], pcframe->cx-1 - x0u[j]);
        memset(x_update+x0u[j], 1, cx);
      }
      else
      {
        memset(x_update+x0u[j], 1, pcframe->cx-1-x0u[j]);
        cx = min( x1u[j], pcframe->cx-1 );
        memset(x_update, 1, x1u[j]);
      }
    }

    i = 0;
    while (i<pcframe->cx)
    {
      int x0, cx;
      if (x_update[i])
      {
        x0 = i;
        cx = 0;
        while (x_update[i] && i<pcframe->cx) { cx ++ ; i++; }
        frame_bitblt_fast(x0, 0, cx, pcframe->cy, pcframe);
      }
      i++;
    }
  } // if (!freeze_screen && pcframe->visible)

  shm_write(&shm_data);
}

static void cframe_chandata_reset(cframe_t * pcframe)
{
 // int i;
  rect_t rect = {0, 0, pcframe->cx, pcframe->cy};
 // unsigned int curTick;
 // shm_data_t shm_data;
  char s[128];

  if (!pcframe) return;

/*  curTick = gettimemillis();
  memset(&shm_data, 0, sizeof(shm_data_t));
  for (i=0; i<NUM_VIEWS; i++)
  {
    shm_data.dview_data_ptr[i] = 0;
    shm_data.old_tick[i] = curTick;

    fifo_pop(&fifo_dview_chan[i], fifo_avail(&fifo_dview_chan[i]));
  }

  shm_write(&shm_data);
*/
  if (!pcframe->visible) return;

 // grph_fillrect(pcframe->bkdc, 0, 0, pcframe->cx, pcframe->cy, 0x000000);
  cframe_bk_update(pcframe, FALSE, TRUE);
//  cframe_bk_update(pcframe, TRUE, TRUE);

  grph_fillrect(pcframe->fgdc, 0, 0, pcframe->cx, pcframe->cy, 0x000000);
  grph_fillrect(pcframe->updc, rect.x0, rect.y0, rect.x1-rect.x0, rect.y1-rect.y0, 0x000000);

  if (1) //(demo_mode)
  {
    rect_t rect;
    grph_setfont(pcframe->updc, ARIAL_60);
    grph_setfgcolor(pcframe->updc, demo_mode ? RGB(0xEE,0xEE,0xEE) : 0x000000);
    rect.x0 = 50;
    rect.x1 = pcframe->cx - 50;
    rect.y0 = pcframe->cy / 2 - 60 / 2;
    rect.y1 = rect.y0 + 100;
    ids2string(IDS_DEMO_LARGE, s);
    grph_drawtext(pcframe->updc, s, -1, &rect, DT_CENTER);
  }

#if defined (TEST_IDSTEXT_MODE)
  pat_pars_t pat_pars;
  pat_get(&pat_pars);
  grph_setfgcolor(pcframe->updc, demo_mode ? RGB(0xEE,0xEE,0xEE) : 0x000000);
  ids2string(pat_pars.bedno, s);
  grph_setfont(pcframe->updc, MONOS_10); // inpfoc
  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 100;
  rect.y1 = rect.y0 + 30;
  grph_drawtext(pcframe->updc, s, -1, &rect, DT_CENTER);

  grph_setfont(pcframe->updc, ARIAL_14); // iframe, eframe
  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 130;
  rect.y1 = rect.y0 + 40;
  grph_drawtext(pcframe->updc, s, -1, &rect, DT_CENTER);

  grph_setfont(pcframe->updc, ARIAL_12); // eframe
  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 170;
  rect.y1 = rect.y0 + 30;
  grph_drawtext(pcframe->updc, s, -1, &rect, DT_CENTER);

  grph_setfont(pcframe->updc, ARIAL_10); // tframe, eframe
  rect.x0 = 0;
  rect.x1 = pcframe->cx-1;
  rect.y0 = 20 + 200;
  rect.y1 = rect.y0 + 20;
  grph_fillrect(pcframe->updc, rect.x0, rect.y0, rect.x1-rect.x0, rect.y1-rect.y0, 0x000000);
  grph_drawtext(pcframe->updc, s, -1, &rect, DT_CENTER);
#endif

#if 0
  rect.x0= 100;
  rect.y0= 400;
  rect.y1= 50;
  grph_drawtext(pcframe->updc, "YK", -1, &rect, DT_VERTICAL | DT_CENTER);
#endif

  cframe_freeze_icon(pcframe);

  inpfoc_focus(INPFOC_MAIN);

 // frame_bitblt_fast(0, 0, pcframe->cx, pcframe->cy, pcframe);
 // pcframe->visible = 1;

  after_reset_countdown = 4;

  frame_bitblt(0, 0, pcframe->cx, pcframe->cy, pcframe);
}

static void cframe_on_ecglead_default(cframe_t * pcframe)
{
  int i;

  for (i=0; i<NUM_ECG; i++)
  {
    pcframe->chanview[i].inpfoc_item[0] = MAIN_ECG_1 + i;
    sprintf(pcframe->chanview[i].caption, ecgstr[i]);
    if (pcframe->chanview[i].visible)
      inpfoc_wnd_setcaption(inpfoc_find(INPFOC_MAIN, MAIN_ECG_1 + i), pcframe->chanview[i].caption);
    pcframe->chanview[i].chandata.ecgtype = ECG_1 + i;
  }

  ecg_data_t ecg_data;
  unit_get_data(ECG, &ecg_data);
  if (ecg_data.num_leads == 1)
  {
    sprintf(pcframe->chanview[0].caption, ecgstr[ECG_II]);
    if (pcframe->chanview[0].visible)
      inpfoc_wnd_setcaption(inpfoc_find(INPFOC_MAIN, MAIN_ECG_1), pcframe->chanview[0].caption);
    pcframe->chanview[0].chandata.ecgtype = ECG_II;

    sprintf(pcframe->chanview[1].caption, ecgstr[ECG_I]);
    if (pcframe->chanview[1].visible)
      inpfoc_wnd_setcaption(inpfoc_find(INPFOC_MAIN, MAIN_ECG_2), pcframe->chanview[1].caption);
    pcframe->chanview[1].chandata.ecgtype = ECG_I;
  }

  cframe_cfg_save();
}

static void cframe_on_ecglead(cframe_t * pcframe, int ecgpos, int delta)
{
  int i, j, type_old, type_new;
  chanview_t * pcv = NULL;
  int found;
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);

  if (ecg_data.num_leads == 1) return;

  const int num_ecg = (ecg_data.num_leads == 3) ? 6 : NUM_ECG;

  if (!pcframe) return;

  cframe_chandata_reset(pcframe);

  for (i=0; i<num_ecg; i++)
  {
    int k;
    for (k=0; k<pcframe->chanview[i].num_inpfoc_items; k++)
    {
      if (pcframe->chanview[i].inpfoc_item[k] == MAIN_ECG_1 + ecgpos)
      {
        pcv = &pcframe->chanview[i];
        break;
      }
    }
  }

  if (!pcv) return;

  type_old = pcv->chandata.ecgtype;
  type_new = type_old;
  found = 0;

  if (delta > 0)
  {
    for (j=1; j<num_ecg && !found; j++)
    {
      for (i=0; i<num_ecg && !found; i++)
      {
        if ( (pcframe->chanview[i].chandata.ecgtype == (type_old+j) % num_ecg) &&
             (pcframe->chanview[i].visible == 0) )
        {
          type_new = (type_old+j) % num_ecg;
          pcv->chandata.ecgtype = type_new;
          found = 1;
        }
      }
    }
  }
  if (delta < 0)
  {
    for (j=num_ecg-1; j>0 && !found; j--)
    {
      for (i=num_ecg-1; i>0 && !found; i--)
      {
        if ( (pcframe->chanview[i].chandata.ecgtype == (type_old+j) % num_ecg) &&
             (pcframe->chanview[i].visible == 0) )
        {
          type_new = (type_old+j) % num_ecg;
          pcv->chandata.ecgtype = type_new;
          found = 1;
        }
      }
    }
  }

  for (i=0; i<num_ecg; i++)
  {
    if ( (pcframe->chanview[i].chandata.ecgtype == type_new) && (pcv != &pcframe->chanview[i]) )
    {
      pcframe->chanview[i].chandata.ecgtype = type_old;
      break;
    }
  }

  sprintf(pcv->caption, ecgstr[type_new]);

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_MAIN, MAIN_ECG_1 + ecgpos), pcv->caption);

  cframe_cfg_save();
 // cframe_chandata_reset(pcframe);
}

static void cframe_on_chanspd(cframe_t * pcframe, int chno, int delta)
{
  char s[200];
  int immps = 0;
  int i, j;
//  chanview_t * pcv;

  if (!pcframe) return;

  cframe_chandata_reset(pcframe);

  for (i=0; i<MAXNUM_MMPS; i++)
    if (mmps[i] == (float)pcframe->chanview[chno].chandata.mmps)
      break;
  immps = i;
  assert (immps < MAXNUM_MMPS);
  if (delta > 0)
  {
    immps += 1;
    immps %= MAXNUM_MMPS;
  }
  if (delta < 0)
  {
    if (--immps < 0) immps = MAXNUM_MMPS-1;
  }

  if (chno >= ECG && chno < NUM_ECG)
  {
    for (j=0; j<NUM_ECG; j++)
    {
      pcframe->chanview[j].chandata.mmps = mmps[immps];
      if (pcframe->chanview[j].visible) chno = j;
    }
  }

  pcframe->chanview[chno].chandata.mmps = mmps[immps];

  if (pcframe->chanview[chno].chandata.mmps == 6.25f)
    sprintf(s, "%.2f ", (float)pcframe->chanview[chno].chandata.mmps);
  else
  if (pcframe->chanview[chno].chandata.mmps == 12.5f)
    sprintf(s, "%.1f ", (float)pcframe->chanview[chno].chandata.mmps);
  else
    sprintf(s, "%d ", (int)pcframe->chanview[chno].chandata.mmps);

  inpfoc_item_t * pifi;
  char s2[200];
  ids2string(IDS_MMPS, s2);
  strcat(s, " ");
  strcat(s, s2);
  pifi = inpfoc_find(INPFOC_RESPSET, RESPSET_SPEED);
  if (pifi) inpfoc_wnd_setcaption(pifi, s);
  pifi = inpfoc_find(INPFOC_SPO2SET, SPO2SET_SPEED);
  if (pifi) inpfoc_wnd_setcaption(pifi, s);
  pifi = inpfoc_find(INPFOC_ECGSET, ECGSET_SPEED);
  if (pifi) inpfoc_wnd_setcaption(pifi, s);
  pifi = inpfoc_find(INPFOC_CO2SET, CO2SET_SPEED);
  if (pifi) inpfoc_wnd_setcaption(pifi, s);

 // pcv = &pcframe->chanview[chno];

  cframe_cfg_save();
}

static void cframe_on_chanamp(cframe_t * pcframe, int chno, int delta)
{
  char s[200];
  int imy = 0;
  int i, j, maxnum_my;

  if (!pcframe) return;

  cframe_chandata_reset(pcframe);

  for (i=MY_FIRST; i<MAXNUM_MY; i++)
  {
    if (i == (float)pcframe->chanview[chno].chandata.my)
      break;
  }
  imy = i;
  assert (imy < MAXNUM_MY);
  imy += delta;
  maxnum_my = (chno == CO2) ? 2 : MAXNUM_MY;
  while (imy < 0) imy += maxnum_my;
  imy %= maxnum_my;

  pcframe->chanview[chno].chandata.my = imy;
  sprintf(s, "x%d", (int)(pow(2, pcframe->chanview[chno].chandata.my)) );

  if (chno_is_ecg(chno))
  {
    for (j=0; j<NUM_ECG; j++)
    {
      pcframe->chanview[j].chandata.my = imy;
    }
    chno = ECG;
    if (pcframe->chanview[i].chandata.my > MY_1)
      sprintf(s, "%d ", (int)(pow(2,pcframe->chanview[i].chandata.my) * 2.5f));
    else
      sprintf(s, "%.1f ", (float) 2.5f * (int)pow(2,pcframe->chanview[i].chandata.my));
    cframe_bk_update(pcframe, TRUE, TRUE);
  }

  assert(inpfoc_find(INPFOC_MAIN, pcframe->chanview[chno].inpfoc_item[1]));
  if (chno == SPO2)
  {
    if (imy == MY_AUTO) // autoscale
    {
      ids2string(IDS_AUTO, s);
      dio_module_cmd(PD_ID_SPO2, SPO2_SET_SCALE, SPO2_AUTOSCALE);
      *s=0;
      inpfoc_wnd_setids(inpfoc_find(INPFOC_MAIN, pcframe->chanview[chno].inpfoc_item[1]), IDS_AUTO);

      my1[SPO2] = (float)pcframe->cy/(pcframe->num_visible_views+1)/0xFFF; // fit autoscaled SPO2 into channel window
    }
#if 0  // for spo2 debug
    else if (imy == MY_2)
    {
      debug("try spo2 pila\n");
      sprintf(s, "pila");
      dio_module_cmd(PD_ID_SPO2, SPO2_SET_SCALE, SPO2_PILASCALE);
    }
#endif
    else
    {
      dio_module_cmd(PD_ID_SPO2, SPO2_SET_SCALE, SPO2_NOAUTOSCALE);
      inpfoc_wnd_setids(inpfoc_find(INPFOC_MAIN, pcframe->chanview[chno].inpfoc_item[1]), 0);

      my1[SPO2] = 0.001f;
    }
   // trpn_reinit();
  }

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_MAIN, pcframe->chanview[chno].inpfoc_item[1]), s);

  if (chno == CO2)
  {
    cframe_bk_update(pcframe, TRUE, TRUE);
  }

  cframe_cfg_save();
}


void cframe_set_ecgfilt_mode(unsigned char mode)
{
  assert(mode < MAXNUM_ECG_MODES);
  cur_mode = mode;

  cframe_command(CFRAME_FILT_MODE_SET, (void*)((unsigned int)cur_mode));

  cframe_cfg_save();
}

static void cframe_on_filt_mode_set(cframe_t * pcframe, int mode)
{
  inpfoc_item_t * pifi;
  if (!pcframe->visible) return;
  pifi = inpfoc_find(INPFOC_MAIN, MAIN_MODE);
  if (!pifi) return;
  inpfoc_wnd_setids(pifi, ecg_mode_ids[mode]);
}

static void cframe_on_filt_mode_roll(cframe_t * pcframe, int delta)
{
  ecg_data_t ecg_data;
  if (!pcframe) return;

  unit_get_data(ECG, &ecg_data);
  ecg_data.set_bits.diag_f = 0;

  cframe_chandata_reset(pcframe);

  if (cur_mode == MODE_DIAGNOSTICS && delta > 0)
  {
    ecg_data.set_bits.diag_f = 1;
    cframe_on_filt_mode_set(pcframe, MODE_DIAGNOSTICS_F);
  }

  if (cur_mode == MODE_DIAGNOSTICS_F && delta < 0)
  {
    ecg_data.set_bits.diag_f = 0;
    cframe_on_filt_mode_set(pcframe, MODE_DIAGNOSTICS);
  }

  if (delta > 0) cur_mode += 1;
  if (delta < 0) cur_mode -= 1;

  while ((signed char)cur_mode >= MAXNUM_ECG_MODES) cur_mode -= MAXNUM_ECG_MODES;
  while ((signed char)cur_mode < 0)                 cur_mode += MAXNUM_ECG_MODES;

  switch(cur_mode)
  {
    case MODE_DIAGNOSTICS:
      if (demo_mode == 1)
        cframe_on_filt_mode_set(pcframe, MODE_DIAGNOSTICS);
      else
      {
        ecgm_command(ECGM_ECG_TAU_3200);
        ecgm_command(ECGM_DHF_OFF);
      }
      break;
    case MODE_DIAGNOSTICS_F:
      ecg_data.set_bits.diag_f = 1;
      if (demo_mode == 1)
        cframe_on_filt_mode_set(pcframe, MODE_DIAGNOSTICS_F);
      else
      {
        ecgm_command(ECGM_ECG_TAU_3200);
        ecgm_command(ECGM_DHF_OFF);
      }
      break;
    case MODE_MONITORING:
      if (demo_mode == 1)
        cframe_on_filt_mode_set(pcframe, MODE_MONITORING);
      else
      {
        ecgm_command(ECGM_ECG_TAU_320);
        ecgm_command(ECGM_DHF_OFF);
      }
      break;
    case MODE_SURGERY:
      if (demo_mode == 1)
        cframe_on_filt_mode_set(pcframe, MODE_SURGERY);
      else
      {
        ecgm_command(ECGM_ECG_TAU_160);
        ecgm_command(ECGM_DHF_ON);
      }
      break;
  }

  unit_set_data(ECG, &ecg_data);
}

static void cframe_on_chan_visibility(cframe_t * pcframe, int v)
{
  int i;
  int k;
  int nv = 1;
  chanview_t * pcv;
  char s[200], *ps = s;
  int num_visible_ecgs;
  inpfoc_item_t * pifi;
  inpfoc_wnd_dc_t ifwd;
  inpfoc_wnd_t * pifw;
  rect_t rc;
  int chno, visible;
  unsigned short ids = 0;
  ecg_data_t ecg_data;

  chno = LOWORD(v);
  visible = HIWORD(v);

  if (!pcframe) return;

  assert(chno < NUM_VIEWS);

  unit_get_data(ECG, &ecg_data);
  int num_ecg = (ecg_data.num_leads == 3) ? 6 : NUM_ECG;
  if (ecg_data.num_leads == 1) num_ecg = 1;

  pcv = &pcframe->chanview[chno];

  if (pcv->visible == visible) return;

  pcv->visible = visible;

  // update .y0 fields
  pcframe->num_visible_views = 0;
  num_visible_ecgs = 0;
  for (i=0; i<NUM_VIEWS; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      pcframe->num_visible_views ++;
      if (i>= ECG_1 && i<num_ecg) num_visible_ecgs ++;
    }
  }
 // if (pcframe->chanview[CO2].visible)
 // {
 //   pcframe->num_visible_views --;
 // }
  int dy = 0;
  if (pcframe->num_visible_views)
    dy = (pcframe->cy-CFRAME_LABEL_DY_OFFS) / (pcframe->num_visible_views);
  for (i=0, nv=1; i<NUM_VIEWS; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      pcframe->chanview[i].y0 = CFRAME_LABEL_DY_OFFS + dy/2 + (nv-1)*dy;
      nv += 1;
    }
  }
  if (pcframe->chanview[CO2].visible)
  {
    pcframe->chanview[CO2].y0 = pcframe->cy - 20;
 //   pcframe->num_visible_views ++;
  }

  if (!pcframe->visible) return;

  cframe_bk_update(pcframe, TRUE, FALSE);

  if (pcframe->chanview[SPO2].chandata.my == MY_AUTO)
    my1[SPO2] = (float)(pcframe->cy)/(pcframe->num_visible_views+1)/0xFFF;
  else
    my1[SPO2] = 0.001f;

 // cframe_chandata_reset(pcframe);
  if (pcv->visible) // show channel
  {
    int inpfoc_ins_after = 0;
    for (i=NUM_VIEWS; i>chno; i--)
    {
      if (pcframe->chanview[i].visible)
      {
        assert(pcframe->chanview[i].num_inpfoc_items);
        if (chno_is_ecg(i))
        {
          if (num_visible_ecgs == 1 && chno_is_ecg(chno))
            inpfoc_ins_after = 0;
       //   else
          if ( (num_visible_ecgs == 2 && chno_is_ecg(chno)) || (num_visible_ecgs == 1 && !chno_is_ecg(chno)) )
            inpfoc_ins_after = MAIN_MODE;
          else
            inpfoc_ins_after = pcframe->chanview[i].inpfoc_item[0];
        }
        else
          inpfoc_ins_after = pcframe->chanview[i].inpfoc_item[0];
      }
    }

//printf("chno %d after inpfoc_ins_after %d\n", chno, inpfoc_ins_after);

    assert( pcframe->chanview[chno].num_inpfoc_items == 2 );

    ifwd.bkdc = pcframe->bkdc;
    ifwd.updc = pcframe->updc;
    ifwd.fgcolor = pcframe->chanview[chno].color;
    ifwd.bkcolor = CFRAME_BKCOLOR;

    if (chno_is_ecg(chno))
    {
      if (pcframe->chanview[ECG_1].chandata.my > MY_1)
        sprintf(s, "%d ", (int)(pow(2,pcframe->chanview[ECG_1].chandata.my) * 2.5f));
      else
       sprintf(s, "%.1f ", (float) 2.5f * (int)pow(2,pcframe->chanview[ECG_1].chandata.my));
    }
    else
    if (chno == SPO2)
    {
      if (pcframe->chanview[SPO2].chandata.my != MY_AUTO)
      {
        sprintf(s, "x%d", (int)(pow(2, pcframe->chanview[SPO2].chandata.my)) );
        ids = 0; // already set with init value
      }
      else
      {
        ps = 0;
        ids = IDS_AUTO;
      }
    }
    else
      sprintf(s, "x%d", (int)(pow(2, pcframe->chanview[chno].chandata.my)));

    if ( (chno_is_ecg(chno) && num_visible_ecgs == 1) || (!chno_is_ecg(chno)) )
    {
      if (chno_is_ecg(chno) && num_visible_ecgs == 1)
      {
        ecg_data_t ecg_data;
        unsigned short ids;
        unit_get_data(ECG, &ecg_data);
        switch(ecg_data.set & 0x3)
        {
          case 0:
            ids = IDS_DIAGNOSTICS;
            break;
          case 1:
            ids = IDS_MONITORING;
            break;
          case 2:
            ids = IDS_SURGERY;
            break;
          default:
            ids = IDS_UNDEF7;
            break;
        }
        ifwd.bkdc = pcframe->bkdc;
        ifwd.updc = pcframe->updc;
        ifwd.fgcolor = pcframe->chanview[chno].color;
        ifwd.bkcolor = CFRAME_BKCOLOR;
        inpfoc_ins(INPFOC_MAIN, MAIN_MODE, (void*) inpfoc_wnd_create("LIST", 150, 15, 130, 20, 0, ids, &ifwd, frame_bitblt, pcframe), inpfoc_ins_after);
      }
      if (chno_is_ecg(chno))
      {
        inpfoc_ins(INPFOC_MAIN, pcv->inpfoc_item[1], (void*) inpfoc_wnd_create("LIST", 65, 15, 80, 20, s, IDS_MMPMV, &ifwd, frame_bitblt, pcframe), inpfoc_ins_after);
      }
      else
      {
        inpfoc_ins(INPFOC_MAIN, pcv->inpfoc_item[1], (void*) inpfoc_wnd_create("LIST", 70, pcframe->chanview[chno].y0-CFRAME_LABEL_DY_OFFS, 80, 20, ps, ids, &ifwd, frame_bitblt, pcframe), inpfoc_ins_after);
      }
    }

    if (chno_is_ecg(chno))
    {
//printf("ADD %d %d %s\n", chno, pcframe->chanview[chno].chandata.ecgtype, ecgstr[chno]);
      sprintf(s, "%s", "LIST");
      strcpy(pcframe->chanview[chno].caption, ecgstr[pcframe->chanview[chno].chandata.ecgtype]);
      inpfoc_ins(INPFOC_MAIN, pcv->inpfoc_item[0], (void*) inpfoc_wnd_create(s, 25, pcframe->chanview[chno].y0-CFRAME_LABEL_DY_OFFS, 38, 20, pcframe->chanview[chno].caption, 0, &ifwd, frame_bitblt, pcframe), inpfoc_ins_after);
    }
    else
    {
      sprintf(s, "%s", "PUSH");
      inpfoc_ins(INPFOC_MAIN, pcv->inpfoc_item[0], (void*) inpfoc_wnd_create(s, 25, pcframe->chanview[chno].y0-CFRAME_LABEL_DY_OFFS, 38, 20, 0, pcframe->chanview[chno].ids, &ifwd, frame_bitblt, pcframe), pcv->inpfoc_item[1]);
    }
  }
  else // hide channel
  {
    for (k=0; k<pcv->num_inpfoc_items; k++)
    {
      // do not delete ecg inpfoc (amp and spd) if visible ecg chans are remaining
      if (chno_is_ecg(chno) && k > 0 && num_visible_ecgs > 0) continue;

      if (chno_is_ecg(chno) && k > 0 && (pifi = inpfoc_find(INPFOC_MAIN, MAIN_MODE)))
      {
        if (!pifi) continue; // have been already deleted
        pifw = (inpfoc_wnd_t *) pifi->parg;
        rc.x0 = pifw->x;
        rc.y0 = pifw->y;
        rc.x1 = rc.x0 + pifw->cx;
        rc.y1 = rc.y0 + pifw->cy;
        inpfoc_del(INPFOC_MAIN, MAIN_MODE);

        // permanently delete inpfoc item from screen
        grph_fillrect(pcframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, CFRAME_BKCOLOR);
        if (rc.x1 >= pcframe->cx) rc.x1 = pcframe->cx-1;
        frame_bitblt_fast(rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, pcframe);
      }

      pifi = inpfoc_find(INPFOC_MAIN, pcv->inpfoc_item[k]);
      assert(pifi);
      pifw = (inpfoc_wnd_t *) pifi->parg;
      rc.x0 = pifw->x;
      rc.y0 = pifw->y;
      rc.x1 = rc.x0 + pifw->cx;
      rc.y1 = rc.y0 + pifw->cy;
      inpfoc_del(INPFOC_MAIN, pcv->inpfoc_item[k]);

      // permanently delete inpfoc item from screen
      grph_fillrect(pcframe->bkdc, rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, CFRAME_BKCOLOR);
      if (rc.x1 >= pcframe->cx) rc.x1 = pcframe->cx-1;
      frame_bitblt_fast(rc.x0, rc.y0, rc.x1-rc.x0, rc.y1-rc.y0, pcframe);
    }
    if (chno == CO2)
    {
      cframe_bk_update(pcframe, TRUE, FALSE);
      pcframe->chanview[CO2].y0 = 0;//pcframe->chanview[RESP].y0 + 20;
    }
  }

  for (i=0; i<NUM_VIEWS; i++)
  {
    if (!pcframe->chanview[i].visible) continue;
    for (k=0; k<pcframe->chanview[i].num_inpfoc_items; k++)
    {
      inpfoc_wnd_move(inpfoc_find(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[k]), -1, pcframe->chanview[i].y0-CFRAME_LABEL_DY_OFFS, TRUE);
      if (chno_is_ecg(i)) break; // leave internal loop
    }
  }

 /* for (i=ECG_1; i<NUM_ECG; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      inpfoc_wnd_move(inpfoc_find(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[1]), -1, pcframe->chanview[i].y0-CFRAME_LABEL_DY_OFFS, TRUE);
      inpfoc_wnd_move(inpfoc_find(INPFOC_MAIN, MAIN_MODE), -1, pcframe->chanview[i].y0-CFRAME_LABEL_DY_OFFS, TRUE);
      break;
    }
  }*/

  for (i=0; i<NUM_VIEWS; i++)
  {
    for (k=0; k<pcframe->chanview[i].num_inpfoc_items; k++)
    {
      pifi = inpfoc_find(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[k]);
      if (!pifi) continue;
      inpfoc_wnd_getcaption(pifi, s, sizeof(s));
      inpfoc_wnd_setcaption(pifi, s[0] ? s : NULL);
    }
  }

  cframe_bk_update(pcframe, FALSE, TRUE);
  cframe_chandata_reset(pcframe);

  cframe_cfg_save();
}

void cframe_on_hide(void)
{
  cframe_t * pcframe;
  int i;

  pcframe = cframe_getptr();
  assert(pcframe);

  pcframe->visible = 0;

  for (i=ECG_1; i<=ECG_LAST; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      int k;
      for (k=0; k<pcframe->chanview[i].num_inpfoc_items; k++)
      {
        if (inpfoc_find(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[k]))
          inpfoc_del(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[k]);
      }
      break;
    }
  }

  for (; i<=ECG_LAST; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      inpfoc_del(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[0]);
    }
  }

  for (i=ECG_LAST+1; i<NUM_VIEWS; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      int k;
      assert( pcframe->chanview[i].num_inpfoc_items == 2 );
      for (k=0; k<pcframe->chanview[i].num_inpfoc_items; k++)
      {
        inpfoc_del(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[k]);
      }
    }
  }

  inpfoc_del(INPFOC_MAIN, MAIN_MODE);

  cframe_cfg_save();
}

void cframe_on_show(void)
{
  cframe_t * pcframe;
  uframe_t * puframe;
  char s[200], *ps = s;
  int i;
  inpfoc_wnd_dc_t ifwd;
  unsigned short ids = 0;

  pcframe = cframe_getptr();
  assert(pcframe);

  puframe = uframe_getptr();
  assert(puframe);

  if (mframe_ismaximized())
  {
    debug("leaving %s with no changes!!!\n", __FUNCTION__);
    return;
  }

  if (pcframe->visible) return;

  grph_fillrect(pcframe->bkdc, 0, 0, CFRAME_CX, CFRAME_CY, CFRAME_BKCOLOR);
  cframe_bk_update(pcframe, TRUE, TRUE);

  if (puframe->visible)
    cframe_on_resize(CFRAME_X0+UFRAME_CX, CFRAME_Y0, CFRAME_CX-UFRAME_CX, CFRAME_CY);
  else
    cframe_on_resize(CFRAME_X0, CFRAME_Y0, CFRAME_CX, CFRAME_CY);

  ifwd.bkdc = pcframe->bkdc;
  ifwd.updc = pcframe->updc;
  ifwd.fgcolor = 0x000000;
  ifwd.bkcolor = CFRAME_BKCOLOR;

  ifwd.fgcolor = pcframe->chanview[ECG].color;
  for (i=ECG_1; i<=ECG_LAST; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      inpfoc_ins(INPFOC_MAIN, MAIN_MODE, (void*) inpfoc_wnd_create("LIST", 150, 15, 130, 20, 0, ecg_mode_ids[cur_mode], &ifwd, frame_bitblt, pcframe), 0);

      if (pcframe->chanview[i].chandata.my > MY_1)
        sprintf(s, "%d ", (int)(pow(2,pcframe->chanview[i].chandata.my) * 2.5f));
      else
        sprintf(s, "%.1f ", (float) 2.5f * (int)pow(2,pcframe->chanview[i].chandata.my));
      inpfoc_ins(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[1], (void*) inpfoc_wnd_create("LIST", 65, 15, 80, 20, s, IDS_MMPMV, &ifwd, frame_bitblt, pcframe), 0);

      break;
    }
  }

  ifwd.fgcolor = pcframe->chanview[ECG].color;
  for (i=ECG_1; i<=ECG_LAST; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      inpfoc_ins(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[0], (void*) inpfoc_wnd_create("LIST", 25, pcframe->chanview[i].y0-CFRAME_LABEL_DY_OFFS, 38, 20, pcframe->chanview[i].caption, 0, &ifwd, frame_bitblt, pcframe), 0);
    }
  }

  for (i=ECG_LAST+1; i<NUM_VIEWS; i++)
  {
    if (pcframe->chanview[i].visible)
    {
      ifwd.fgcolor = pcframe->chanview[i].color;
      inpfoc_ins(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[0], (void*) inpfoc_wnd_create("PUSH", 25, pcframe->chanview[i].y0-CFRAME_LABEL_DY_OFFS, 38, 20, 0, pcframe->chanview[i].ids, &ifwd, frame_bitblt, pcframe), 0);
      sprintf(s, "x%d", (int)(pow(2,pcframe->chanview[i].chandata.my)) );
      if (i == SPO2 && pcframe->chanview[i].chandata.my == MY_AUTO)
      {
        ids = IDS_AUTO;
        ps = 0;
      }
      else
      {
        ids = 0;
        ps = s;
      }
      inpfoc_ins(INPFOC_MAIN, pcframe->chanview[i].inpfoc_item[1], (void*) inpfoc_wnd_create("LIST", 70, pcframe->chanview[i].y0-CFRAME_LABEL_DY_OFFS, 80, 20, ps, ids, &ifwd, frame_bitblt, pcframe), 0);
    }
  }

  if (!puframe->visible) // may be not needed this if
    inpfoc_change(INPFOC_MAIN);
 // frame_bitblt(0, 0, pcframe->cx, pcframe->cy, pcframe);

  pcframe->visible = 1;
  cframe_chandata_reset(pcframe);

  cframe_cfg_save();
}

static void cframe_on_freeze_screen(cframe_t * pcframe)
{
  if (mframe_ismaximized()) return;
  if (!pcframe->visible) return;
  freeze_screen = !freeze_screen;
  cframe_freeze_icon(pcframe);
  if (!freeze_screen)
    cframe_chandata_reset(pcframe);
}

void cframe_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case CFRAME_RESIZE:
      dview_command(CFRAME, cmd, sizeof(rect_t), arg);
      break;
    default:
      dview_command(CFRAME, cmd, 0, arg);
      break;
  }
}

void cframe_on_command(int cmd, void * arg)
{
  switch (cmd)
  {
    case CFRAME_HIDE:
      cframe_on_hide();
      break;
    case CFRAME_SHOW:
      cframe_on_show();
      break;
    case CFRAME_RESIZE:
//printf("%s RESIZE %d %d %d %d \n", __FUNCTION__, prc->left, prc->top, prc->right, prc->bottom );
      cframe_on_resize(((rect_t*)arg)->x0, ((rect_t*)arg)->y0, ((rect_t*)arg)->x1-((rect_t*)arg)->x0, ((rect_t*)arg)->y1-((rect_t*)arg)->y0);
      break;
    case CFRAME_RELOAD:
      cframe_on_reload(pcframe);
      break;
    case CFRAME_CHAN_VISIBILITY:
      cframe_on_chan_visibility(pcframe, *((int*)arg));
      break;
    case CFRAME_FREEZE_SCREEN:
      cframe_on_freeze_screen(pcframe);
      break;
    case CFRAME_CHANSPD:
      cframe_on_chanspd(pcframe, (short) LOWORD(*((int*)arg)), (short) HIWORD(*((int*)arg)));
      break;
    case CFRAME_CHANAMP:
      cframe_on_chanamp(pcframe, (short) LOWORD(*((int*)arg)), (short) HIWORD(*((int*)arg)));
      break;
    case CFRAME_FILT_MODE_ROLL:
      cframe_on_filt_mode_roll(pcframe, (int) (*((int*)arg)));
      break;
    case CFRAME_FILT_MODE_SET:
      cframe_on_filt_mode_set(pcframe, (int) (*((int*)arg)));
      break;
    case CFRAME_ECGLEAD:
      cframe_on_ecglead(pcframe, (short) LOWORD(*((int*)arg)), (short) HIWORD(*((int*)arg)));
      break;
    case CFRAME_RESET:
      cframe_chandata_reset(pcframe);
      break;
    case CFRAME_ECGLEAD_DEFAULT:
      cframe_on_ecglead_default(pcframe);
      break;
//     case CFRAME_START:
//       cframe_on_start(pcframe);
//       break;
    default:
      error("%s: unknown cmd %d\n", __FUNCTION__, cmd);
      break;
  }
}

cframe_t * cframe_getptr(void)
{
  return &cframe;
}

