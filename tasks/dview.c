#ifdef UNIX
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#ifdef UNIX
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <pthread.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif
#include <errno.h>
#include <assert.h>

#include "fifo.h"
#include "cframe.h"
#include "lframe.h"
#include "tframe.h"
#include "iframe.h"
#include "sframe.h"
#include "uframe.h"
#include "eframe.h"
#include "unit.h"
#include "mframe.h"
#include "demo.h"
#include "bedmoni.h"
#include "dview.h"
#include "alarm.h"
#include "patient.h"
#include "fb.h"
#include "grph.h"
#include "utils.h"

int dview_chan_buf[NUM_VIEWS][DVIEW_MAX_FIFO_SIZE];
fifo_t fifo_dview_chan[NUM_VIEWS]; // chan data

#ifdef UNIX
static pthread_mutex_t cs_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static unsigned char * pgd_data;
static int             pgd_size = 0;
static fb_info_t       fb_info;
static upd_view_info_t upd_view_info[DVIEW_UPD_MAX_SIZE];
static int upd_view_cnt = 0;
static int current_active_frame = -1;
static int dview_state = 1; // enabled by default

frame_interface_t frame_interface[NUM_FRAMES] =
{
  // interface functions must be in enum order
  /*{CFRAME,*/ {cframe_init, cframe_deinit, cframe_on_command, NULL, NULL, NULL},
  /*{MFRAME,*/ {mframe_init, mframe_deinit, mframe_on_command, NULL, NULL, NULL},
  /*{TFRAME,*/ {tframe_init, tframe_deinit, tframe_on_command, tframe_show, tframe_hide, tframe_on_resize},
  /*{LFRAME,*/ {lframe_init, lframe_deinit, lframe_on_command, lframe_show, lframe_hide, lframe_on_resize},
  /*{UFRAME,*/ {uframe_init, uframe_deinit, uframe_on_command, NULL, NULL, NULL},
  /*{IFRAME,*/ {iframe_init, iframe_deinit, iframe_on_command, NULL, NULL, NULL},
  /*{SFRAME,*/ {sframe_init, sframe_deinit, sframe_on_command, NULL, NULL, NULL},
  /*{EFRAME,*/ {eframe_init, eframe_deinit, eframe_on_command, eframe_show, eframe_hide, eframe_on_resize},
  /*NUM_FRAMES*/
};

unsigned short chan_ids[NUM_VIEWS] =
{
  IDS_I,       // ECG_I
  IDS_II,      // ECG_II
  IDS_III,     // ECG_III
  IDS_AVR,     // ECG_aVR
  IDS_AVL,     // ECG_aVL
  IDS_AVF,     // ECG_aVF
  IDS_V,       // ECG_V
  IDS_ECG_NEO, // ECG_NEO
  IDS_FPG,     // SPO2
  IDS_NIBP,    // NIBP
  IDS_RESP,    // RESP
  IDS_T,       // T1T2
  IDS_CO2,     // CO2
};

unsigned int chancolor[NUM_VIEWS] = 
{
  RGB(31,198,50),      // ECG_I
  RGB(31,198,50),      // ECG_II
  RGB(31,198,50),      // ECG_III
  RGB(31,198,50),      // ECG_aVR
  RGB(31,198,50),      // ECG_aVL
  RGB(31,198,50),      // ECG_aVF
  RGB(31,198,50),      // ECG_V
  RGB(31,198,50),      // ECG_NEO
  RGB(16,184,214),     // SPO2
  RGB(222,88,88),      // NIBP
  RGB(0xaa,0xaa,0xaa), // RESP
  RGB(148,194,116),    // T1T2
  RGB(227,222,23),     // CO2
};

unsigned int paramcolor[NUM_PARAMS] = 
{
  RGB(0,0,0),          // PARAM_NONE
  RGB(0,0,0),          // PARAM_TS,
  RGB(31,198,50),      // PARAM_HR,
  RGB(16,184,214),     // PARAM_SPO2,
  RGB(30,30,255) ,     // PARAM_PULSE,
  RGB(222,88,88),      // PARAM_BP,
  RGB(148,194,116),    // PARAM_T1,
  RGB(148,194,116),    // PARAM_T2,
  RGB(148,194,116),    // PARAM_DT,
  RGB(0xaa,0xaa,0xaa), // PARAM_BR,
  RGB(0xff,0xff,0xff), // PARAM_STI,
  RGB(0xee,0xee,0xee), // PARAM_STII,
  RGB(0xdd,0xdd,0xdd), // PARAM_STIII,
  RGB(0xcc,0xcc,0xcc), // PARAM_STAVR,
  RGB(0xbb,0xbb,0xbb), // PARAM_STAVL,
  RGB(0xaa,0xaa,0xaa), // PARAM_STAVF,
  RGB(0x99,0x99,0x99), // PARAM_STV,
  RGB(249,249,123),    // PARAM_BRC,
  RGB(227,222,23),     // PARAM_ETCO2,
  RGB(241,210,131),    // PARAM_ICO2,
};

unsigned int risk_color[NUM_RISKS] = 
{
  RGB(0x00,0x00,0x00),  // NO_RISK
  RGB(30,156,240),      // LOW_RISK
  RGB(0xff,0xff,0x00),  // MID_RISK
  RGB(0xff,0x00,0x00),  // HIGH_RISK
};

tbl_par_t tbl_par[NUM_PARAMS] = 
{
  {PARAM_NONE,  IDS_UNDEF3,   IDS_UNDEF3     },
  {PARAM_TS,    IDS_DATE,     IDS_HOURMIN    },
  {PARAM_HR,    IDS_HR,       IDS_HBPM       },
  {PARAM_SPO2,  IDS_SPO2,     IDS_PERCENT    },
  {PARAM_PULSE, IDS_PULSE,    IDS_HBPM       },
  {PARAM_NIBP,  IDS_NIBP,     IDS_MM_HG      },
  {PARAM_T1,    IDS_T1,       IDS_DEG        },
  {PARAM_T2,    IDS_T2,       IDS_DEG        },
  {PARAM_DT,    IDS_DT,       IDS_DEG        },
  {PARAM_BR,    IDS_BR,       IDS_BRPM       },
  {PARAM_STI,   IDS_ST_I,     IDS_MV         },
  {PARAM_STII,  IDS_ST_II,    IDS_MV         },
  {PARAM_STIII, IDS_ST_III,   IDS_MV         },
  {PARAM_STAVR, IDS_ST_AVR,   IDS_MV         },
  {PARAM_STAVL, IDS_ST_AVL,   IDS_MV         },
  {PARAM_STAVF, IDS_ST_AVF,   IDS_MV         },
  {PARAM_STV,   IDS_ST_V,     IDS_MV         },
  {PARAM_BRC,   IDS_BRC,      IDS_BRPM       },
  {PARAM_ETCO2, IDS_ETCO2,    IDS_MM_HG      },
  {PARAM_ICO2,  IDS_ICO2,     IDS_MM_HG      },
};

char * ecgstr[NUM_ECG] = {"I", "II", "III", "aVR", "aVL", "aVF", "V"};

int rootwfd;

static exec_t exec[DVIEW_MAX_NUM_CMDS_IN_QUEUE];
static int num_cmds_in_queue = 0;

#ifdef WIN32
LRESULT CALLBACK root_wproc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

static HWND root_wnd = NULL;

static HWND dview_create_root_wnd(void)
{
  HWND hwnd;
  WNDCLASS wndclass;
  HBRUSH hBrBk;
  static char szRootWndClassName[] = "bedmoni";

  hBrBk = CreateSolidBrush(RGB(0,0,0));

  wndclass.style          = /*CS_DBLCLKS | */CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc    = (WNDPROC) root_wproc;
  wndclass.cbClsExtra     =0;
  wndclass.cbWndExtra     =0;
  wndclass.hInstance      =0;
  wndclass.hIcon          =0;
  wndclass.hCursor        =0;
  wndclass.hbrBackground  = hBrBk;
  wndclass.lpszMenuName   = NULL;
  wndclass.lpszClassName  = szRootWndClassName;

  RegisterClass(&wndclass);

  hwnd=CreateWindowEx(0L,
                      szRootWndClassName,
                      NULL,
                      WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                      0,
                      0,
                      DISPLAY_CX,
                      DISPLAY_CY,
                      NULL,
                      NULL,
                      NULL,
                      NULL);
  root_wnd = hwnd;
  return hwnd;
}

LRESULT CALLBACK root_wproc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
  HDC hdc;
  PAINTSTRUCT ps;
 // RECT rect;

  switch (iMsg)
  {
    case WM_CREATE:
        break;
    case WM_GETMINMAXINFO:
      {
        MINMAXINFO * lpMMI = (MINMAXINFO*)lParam;
        POINT p = {DISPLAY_CX, DISPLAY_CY};

        lpMMI->ptMinTrackSize = p; 
        lpMMI->ptMaxTrackSize = p;
      }
      break;
    case WM_PAINT:
      /*case WM_MOUSEFIRST:*/
      hdc=BeginPaint(hwnd,&ps);
     // TextOut(hdc, 0, 0, "OPA", 3);
      EndPaint(hwnd,&ps);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_TIMER:
      break;
    default:
      return DefWindowProc(hwnd,iMsg,wParam,lParam);
  }
  return (0);
}


HWND dview_get_root_wnd(void)
{
  return (root_wnd);
}
#endif  // WIN32

int dview_init(void)
{
  int i;
  grph_screeninfo_t gsi;

  for (i=0; i<NUM_VIEWS; i++)
  {
    fifo_init(&fifo_dview_chan[i], (unsigned char*)dview_chan_buf[i], sizeof(int)*DVIEW_MAX_FIFO_SIZE);
  }

#ifdef WIN32
  dview_create_root_wnd();
#endif
#ifdef UNIX
  if (fb_open("/dev/fb") < 0)
  {
    error("fb_open failed\n");
    assert(0);
  }
  fb_get_info(&fb_info);
#endif

  grph_init();

  memset(&gsi, 0, sizeof(grph_screeninfo_t));
  gsi.xres = fb_info.xres;
  gsi.yres = fb_info.yres;
  gsi.bpp = fb_info.bpp;
  rootwfd = grph_create(&gsi, 0);
  gsi.addr = fb_info.addr;

  grph_changedc(rootwfd, &gsi);

  pgd_data = (unsigned char *)calloc(fb_info.xres * fb_info.yres, (fb_info.bpp + 8 - 1) / 8);
  pgd_size = fb_info.xres * fb_info.yres * ((fb_info.bpp + 8 - 1) / 8);
  memset(&upd_view_info, 0, sizeof(upd_view_info));
  upd_view_cnt = 0;

#ifdef ARM
  // clear the most left line
  grph_fillrect(rootwfd, DISPLAY_CX-1, 0, 1, DISPLAY_CY, 0x000000);
#endif

  assert(pgd_data);

  for (i=0; i<NUM_FRAMES; i++)
  {
    if (frame_interface[i].init) frame_interface[i].init();
  }

  return 0;
}

void dview_deinit(void)
{
  int i;
  debug("%s\n", __FUNCTION__);

  for (i=0; i<NUM_FRAMES; i++)
  {
    if (frame_interface[i].deinit) frame_interface[i].deinit();
  }

  if (exit_code == EXIT_OK)
    draw_poweroff_message(rootwfd);

  if (exit_code == EXIT_UPDATED || exit_code == EXIT_RESTART)
    draw_restart_message(rootwfd);

  if (exit_code == EXIT_LOWBAT)
  {
    draw_poweroff_message_ex(IDS_CHARGE_BAT, rootwfd);
    exit_code = EXIT_OK;
  }

  ((PGDC)(rootwfd))->addr = NULL; // in order not to free fb addr
  grph_destroy(rootwfd);

  grph_deinit();

  fb_close();

  free(pgd_data);

  for (i=0; i<NUM_VIEWS; i++)
  {
    fifo_deinit(&fifo_dview_chan[i]);
  }
 // debug("%s <-\n", __FUNCTION__);
}

void dview_chandata_add(int chno, int v)
{
  assert(chno < NUM_VIEWS);

  fifo_put_bytes(&fifo_dview_chan[chno], (unsigned char*)(&v), sizeof(int));
}

//volatile unsigned char busy = 0;
static unsigned char line_fast[DISPLAY_CX*sizeof(int)];
static unsigned char line[DISPLAY_CX*sizeof(int)];

void dview_bitblt_fast(int x, int y, int cx, int cy, dc_list_t * pdclist, int x0, int y0)
{
  int xx, yy;
  int x1, y1;
  int bypp, xres, xres0;
  unsigned char * p_dev;
  unsigned char *pwfg, *pwup, *pwo;
  unsigned long zero;

  if (!pdclist) return;
  if (!dview_state) return;

#ifdef UNIX
//  pthread_mutex_lock(&cs_mutex);
#endif

  x1 = x;
  y1 = y;

  p_dev = fb_info.addr;
  bypp = (fb_info.bpp + 8 - 1) / 8;
  xres = fb_info.xres;
  xres0 = pdclist->xres;
  zero = 0x00000000;

  if (bypp == 2)
  {
    // optimized for 16 bpp
    unsigned short *pwfg, *pwup, *pwo;

    for (yy=0; yy<cy; yy++)
    {
      memcpy(line_fast, (unsigned short*)pdclist->p_bk + (y0+yy)*xres0 + x0, cx*bypp);
      pwfg = (unsigned short*)pdclist->p_fg + (y0+yy)*xres0 + x0;
      pwup = (unsigned short*)pdclist->p_up + (y0+yy)*xres0 + x0;
      pwo  = (unsigned short*)line_fast;
      for (xx=0; xx<cx; xx++)
      {
        if (*pwup != 0x0000)
          *pwo = *pwup;
        else
        if (*pwfg != 0x0000)
          *pwo = *pwfg;
        pwfg ++;
        pwup ++;
        pwo ++;
      }

      memcpy(p_dev + (y1+yy)*bypp*xres + x1*bypp,
             line_fast,
             cx*bypp);
    }
  }
  else
  {
    for (yy=0; yy<cy; yy++)
    {
      memcpy(line_fast, (unsigned char*)pdclist->p_bk + (y0+yy)*xres0*bypp + x0*bypp, cx*bypp);
      pwfg = (unsigned char*)pdclist->p_fg + (y0+yy)*xres0*bypp + x0*bypp;
      pwup = (unsigned char*)pdclist->p_up + (y0+yy)*xres0*bypp + x0*bypp;
      pwo  = (unsigned char*)line_fast;
      for (xx=0; xx<cx; xx++)
      {
        if (memcmp(pwup, &zero, bypp) != 0)
        {
          memcpy(pwo, pwup, bypp);
        }
        else
        if (memcmp(pwfg, &zero, bypp) != 0)
        {
          memcpy(pwo, pwfg, bypp);
        }
        pwfg += bypp;
        pwup += bypp;
        pwo  += bypp;
      }

      memcpy(p_dev + (y1+yy)*bypp*xres + x1*bypp,
             line_fast,
             cx*bypp);
    }
  }

#ifdef UNIX
//  pthread_mutex_unlock(&cs_mutex);
#endif
}

void dview_bitblt(int x, int y, int cx, int cy, dc_list_t * pdclist, int x0, int y0)
{
  int xx, yy;
  int x1, y1;
  int bypp, xres, xres0;
  unsigned char *pwfg, *pwup, *pwo;
  unsigned long zero;

  if (!pdclist) return;
  if (!dview_state) return;

#ifdef UNIX
  pthread_mutex_lock(&cs_mutex);
#endif

  x1 = x;
  y1 = y;

  bypp = (fb_info.bpp + 8 - 1) / 8;
  xres = fb_info.xres;
  xres0 = pdclist->xres;
  zero = 0x00000000;

  if (bypp == 2)
  {
    // optimized for 16 bpp
    unsigned short *pwfg, *pwup, *pwo;
    for (yy=0; yy<cy; yy++)
    {
#if 1
      memcpy(line, (unsigned short*)pdclist->p_bk + (y0+yy)*xres0 + x0, cx*bypp);
#endif
      pwfg = (unsigned short*)pdclist->p_fg + (y0+yy)*xres0 + x0;
      pwup = (unsigned short*)pdclist->p_up + (y0+yy)*xres0 + x0;
      pwo  = (unsigned short*)line;
      for (xx=0; xx<cx; xx++)
      {
        if (*pwup != 0x0000)
          *pwo = *pwup;
        else
        if (*pwfg != 0x0000)
          *pwo = *pwfg;
        pwfg ++;
        pwup ++;
        pwo ++;
      }

#if 1
      memcpy(pgd_data + (y1+yy)*bypp*xres + x1*bypp, line, cx*bypp);
      if (upd_view_cnt >= DVIEW_UPD_MAX_SIZE)
      {
        assert(0);
      }
      upd_view_info[upd_view_cnt].x0 = x1;
      upd_view_info[upd_view_cnt].y0 = y1+yy;
      upd_view_info[upd_view_cnt].cx = cx;
      upd_view_info[upd_view_cnt].valid = 1;
      upd_view_cnt ++;
#endif
    }
  }
  else
  {
    for (yy=0; yy<cy; yy++)
    {
      memcpy(line, (unsigned char*)pdclist->p_bk + (y0+yy)*xres0*bypp + x0*bypp, cx*bypp);
      pwfg = (unsigned char*)pdclist->p_fg + (y0+yy)*xres0*bypp + x0*bypp;
      pwup = (unsigned char*)pdclist->p_up + (y0+yy)*xres0*bypp + x0*bypp;
      pwo  = (unsigned char*)line;
      for (xx=0; xx<cx; xx++)
      {
        if (memcmp(pwup, &zero, bypp) != 0)
        {
          memcpy(pwo, pwup, bypp);
        }
        else
        if (memcmp(pwfg, &zero, bypp) != 0)
        {
          memcpy(pwo, pwfg, bypp);
        }
        pwfg += bypp;
        pwup += bypp;
        pwo  += bypp;
      }

#if 1
      memcpy(pgd_data + (y1+yy)*bypp*xres + x1*bypp, line, cx*bypp);
      if (upd_view_cnt >= DVIEW_UPD_MAX_SIZE)
      {
        assert(0);
      }
      upd_view_info[upd_view_cnt].x0 = x1;
      upd_view_info[upd_view_cnt].y0 = y1+yy;
      upd_view_info[upd_view_cnt].cx = cx;
      upd_view_info[upd_view_cnt].valid = 1;
      upd_view_cnt ++;
#endif
    }
  }

//printf("dview: %d %d %d %d\n", x1, y1, cx, cy);

#ifdef UNIX
   pthread_mutex_unlock(&cs_mutex);
#endif
}

void dview_draw(void)
{
  int i;
  int bypp, xres;

#ifdef UNIX
   pthread_mutex_lock(&cs_mutex);
#endif

// printf("%s\n", __FUNCTION__);
// return;

#ifdef ARM
  bypp = 2;
#else
  bypp = (fb_info.bpp + 8 - 1) / 8;
#endif
  xres = fb_info.xres;

  i = 0;
 // upd_view_cnt = 0;
  for (i=0; i<upd_view_cnt/*DVIEW_UPD_MAX_SIZE*/; i++)
  {
    if (!upd_view_info[i].valid) break;
    memcpy((unsigned char*)fb_info.addr + upd_view_info[i].y0*bypp*xres +  upd_view_info[i].x0*bypp, pgd_data + upd_view_info[i].y0*bypp*xres +  upd_view_info[i].x0*bypp, upd_view_info[i].cx*bypp);
   // upd_view_info[i].valid = 0;
  }
  upd_view_cnt = 0;
  memset(&upd_view_info, 0, sizeof(upd_view_info));

#ifdef UNIX
   pthread_mutex_unlock(&cs_mutex);
#endif
}

void dview_set_state(int t)
{
  dview_state = t;

  if (t && demo_video_pid)
  {
    kill(demo_video_pid, SIGKILL);
    demo_video_pid = 0;

    int yy;
    for (yy=0; yy<DISPLAY_CY; yy++)
    {
      upd_view_info[upd_view_cnt].x0 = 0;
      upd_view_info[upd_view_cnt].y0 = 0+yy;
      upd_view_info[upd_view_cnt].cx = DISPLAY_CX;
      upd_view_info[upd_view_cnt].valid = 1;
      upd_view_cnt ++;
    }
  }
}

int dview_get_state(void)
{
  return dview_state;
}

void dview_frame_active(int frameid)
{
  if (frameid < 0 || frameid >= NUM_FRAMES) return;
  if (current_active_frame >= 0 && frame_interface[current_active_frame].hide)
  {
    frame_interface[current_active_frame].hide();
  }
  current_active_frame = frameid;
  if (frame_interface[current_active_frame].show)
  {
    frame_interface[current_active_frame].show();
  }
}

void dview_command(int frameid, int cmd, int len, void * arg)
{
  if (num_cmds_in_queue >= DVIEW_MAX_NUM_CMDS_IN_QUEUE-1)
  {
    error("%s: cmds overflow\n", __FUNCTION__);
#if 0
    num_cmds_in_queue = 0;
#endif
    return;
  }

//printf("%s: %x %d %d %d\n", __FUNCTION__, frameid, cmd, isptr, (int)arg);

  assert(len <= DVIEW_ARG_DATA_SIZE);

  exec[num_cmds_in_queue].frameid = frameid;
  exec[num_cmds_in_queue].cmd = cmd;
  if (len && arg)
    memcpy(exec[num_cmds_in_queue].data, arg, len);
  else
    *((int*)exec[num_cmds_in_queue].data) = (int)arg;
  num_cmds_in_queue += 1;

//printf("%s-\n", __FUNCTION__);
}

static void dview_exec_command_queue(void)
{
  while(num_cmds_in_queue > 0 && !exit_flag)
  {
    if (exec[0].frameid >= 0 && exec[0].frameid < NUM_FRAMES)
    {
      if (frame_interface[exec[0].frameid].command)
      {
        frame_interface[exec[0].frameid].command(exec[0].cmd, exec[0].data);
      }
    }
    else
    {
      error("%s: unknown frameid %d\n", __FUNCTION__, exec[0].frameid);
    }
    num_cmds_in_queue--;
    memcpy(&exec[0], &exec[1], num_cmds_in_queue * sizeof(exec_t));
  }
}

void dview_update(void)
{
  dview_exec_command_queue();
}

void dview_update_fast(void)
{
  cframe_update_fast();
}

