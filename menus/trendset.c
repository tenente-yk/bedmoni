#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "grph.h"
#include "tframe.h"
#include "uframe.h"
#include "cframe.h"
#include "lframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "trendset.h"

static void trnset_none_func(void*);
static void trnset_suite1_show_func(void*);
static void trnset_suite1_min_func(void*);
static void trnset_suite1_max_func(void*);
static void trnset_suite1_hr_func(void*);
static void trnset_suite1_spo2_func(void*);
static void trnset_suite1_nibp_func(void*);
static void trnset_suite1_pulse_func(void*);
static void trnset_suite2_show_func(void*);
static void trnset_suite2_min_func(void*);
static void trnset_suite2_max_func(void*);
static void trnset_suite2_sti_func(void*);
static void trnset_suite2_stii_func(void*);
static void trnset_suite2_stiii_func(void*);
static void trnset_suite2_stavr_func(void*);
static void trnset_suite2_stavl_func(void*);
static void trnset_suite2_stavf_func(void*);
static void trnset_suite2_stv_func(void*);
static void trnset_suite3_show_func(void*);
static void trnset_suite3_min_func(void*);
static void trnset_suite3_max_func(void*);
static void trnset_suite3_t1_func(void*);
static void trnset_suite3_t2_func(void*);
static void trnset_suite3_dt_func(void*);
static void trnset_suite4_show_func(void*);
static void trnset_suite4_min_func(void*);
static void trnset_suite4_max_func(void*);
static void trnset_suite4_br_func(void*);
static void trnset_suite4_brc_func(void*);
static void trnset_suite4_etco2_func(void*);
static void trnset_suite4_ico2_func(void*);
static void trnset_exit_func(void*);

inpfoc_funclist_t inpfoc_trnset_funclist[TRENDSET_NUM_ITEMS+1] = 
{
  { TRENDSET_NONE,         trnset_none_func           },
  { TRENDSET_SUITE1_SHOW,  trnset_suite1_show_func    },
  { TRENDSET_SUITE1_MIN,   trnset_suite1_min_func     },
  { TRENDSET_SUITE1_MAX,   trnset_suite1_max_func     },
  { TRENDSET_SUITE1_HR,    trnset_suite1_hr_func      },
  { TRENDSET_SUITE1_SPO2,  trnset_suite1_spo2_func    },
  { TRENDSET_SUITE1_NIBP,  trnset_suite1_nibp_func    },
  { TRENDSET_SUITE1_PULSE, trnset_suite1_pulse_func   },
  { TRENDSET_SUITE2_SHOW,  trnset_suite2_show_func    },
  { TRENDSET_SUITE2_MIN,   trnset_suite2_min_func     },
  { TRENDSET_SUITE2_MAX,   trnset_suite2_max_func     },
  { TRENDSET_SUITE2_STI,   trnset_suite2_sti_func     },
  { TRENDSET_SUITE2_STII,  trnset_suite2_stii_func    },
  { TRENDSET_SUITE2_STIII, trnset_suite2_stiii_func   },
  { TRENDSET_SUITE2_STAVR, trnset_suite2_stavr_func   },
  { TRENDSET_SUITE2_STAVL, trnset_suite2_stavl_func   },
  { TRENDSET_SUITE2_STAVF, trnset_suite2_stavf_func   },
  { TRENDSET_SUITE2_STV,   trnset_suite2_stv_func     },
  { TRENDSET_SUITE3_SHOW,  trnset_suite3_show_func    },
  { TRENDSET_SUITE3_MIN,   trnset_suite3_min_func     },
  { TRENDSET_SUITE3_MAX,   trnset_suite3_max_func     },
  { TRENDSET_SUITE3_T1,    trnset_suite3_t1_func      },
  { TRENDSET_SUITE3_T2,    trnset_suite3_t2_func      },
  { TRENDSET_SUITE3_DT,    trnset_suite3_dt_func      },
  { TRENDSET_SUITE4_SHOW,  trnset_suite4_show_func    },
  { TRENDSET_SUITE4_MIN,   trnset_suite4_min_func     },
  { TRENDSET_SUITE4_MAX,   trnset_suite4_max_func     },
  { TRENDSET_SUITE4_BR,    trnset_suite4_br_func      },
  { TRENDSET_SUITE4_BRC,   trnset_suite4_brc_func     },
  { TRENDSET_SUITE4_ETCO2, trnset_suite4_etco2_func   },
  { TRENDSET_SUITE4_ICO2,  trnset_suite4_ico2_func    },
  { TRENDSET_EXIT,         trnset_exit_func           },
  { -1       ,             trnset_none_func           }, // -1 must be last
};

static void trnset_none_func(void * parg)
{

}

static void trnset_suite1_show_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();
  inpfoc_item_t *pit;
  char s[200];

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_SHOW);

  assert(pit);
  assert(ptframe);

  ptframe->view[0].visible = !ptframe->view[0].visible;

  ids2string(ptframe->view[0].visible ? IDS_YES : IDS_NO, s);

  inpfoc_wnd_setcaption(pit, s);

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite1_min_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_t * ptframe = tframe_getptr();
  int vl, vu;
  char s[10];

  assert(ptframe);

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_MIN);
  assert(pit);

  vl = ptframe->view[0].vl;
  vu = ptframe->view[0].vu;

  vl += 1*pcmd->delta;
  while (vl < 30)  vl += (199 - 30 + 1);
  while (vl > 199) vl -= (199 - 30 + 1);

  if (vl < vu)
  {
    snprintf(s, sizeof(s), "%d", vl);
    ptframe->view[0].vl = vl;
    inpfoc_wnd_setcaption(pit, s);
  }

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite1_max_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_t * ptframe = tframe_getptr();
  int vl, vu;
  char s[10];

  assert(ptframe);

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_MAX);
  assert(pit);

  vl = ptframe->view[0].vl;
  vu = ptframe->view[0].vu;

  vu += 1*pcmd->delta;
  while (vu < 31)  vu += (300 - 31 + 1);
  while (vu > 300) vu -= (300 - 31 + 1);

  if (vu > vl)
  {
    snprintf(s, sizeof(s), "%d", vu);
    ptframe->view[0].vu = vu;
    inpfoc_wnd_setcaption(pit, s);
  }

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite1_hr_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[0].mask &= ~(0x01);
  ptframe->view[0].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_HR))) ? 1 : 0) << 0;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite1_nibp_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[0].mask &= ~(0x02);
  ptframe->view[0].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_NIBP))) ? 1 : 0) << 1;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite1_spo2_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[0].mask &= ~(0x04);
  ptframe->view[0].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_SPO2))) ? 1 : 0) << 2;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite1_pulse_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[0].mask &= ~(0x08);
  ptframe->view[0].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_PULSE))) ? 1 : 0) << 3;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_show_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();
  inpfoc_item_t *pit;
  char s[200];

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_SHOW);

  assert(pit);
  assert(ptframe);

  ptframe->view[1].visible = !ptframe->view[1].visible;

  ids2string(ptframe->view[1].visible ? IDS_YES : IDS_NO, s);

  inpfoc_wnd_setcaption(pit, s);

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_min_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_t * ptframe = tframe_getptr();
  int vl, vu;
  char s[10];

  assert(ptframe);

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_MIN);
  assert(pit);

  vl = ptframe->view[1].vl;
  vu = ptframe->view[1].vu;

  vl += 1*pcmd->delta;
  while (vl < -200)  vl += (95 - -200 + 1);
  while (vl > 95)    vl -= (95 - -200 + 1);

  if (vl < vu)
  {
   // snprintf(s, sizeof(s), "%.02f", (float)vl / ptframe->view[1].kv);
    snprintf(s, sizeof(s), "%d", vl);
    ptframe->view[1].vl = vl;
    inpfoc_wnd_setcaption(pit, s);
  }

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_max_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_t * ptframe = tframe_getptr();
  int vl, vu;
  char s[10];

  assert(ptframe);

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_MAX);
  assert(pit);

  vl = ptframe->view[1].vl;
  vu = ptframe->view[1].vu;

  vu += 1*pcmd->delta;
  while (vu < -95)  vu += (200 - -95 + 1);
  while (vu > 200)  vu -= (200 - -95 + 1);

  if (vu > vl)
  {
   // snprintf(s, sizeof(s), "%.02f", (float)vu / ptframe->view[1].kv);
    snprintf(s, sizeof(s), "%d", vu);
    ptframe->view[1].vu = vu;
    inpfoc_wnd_setcaption(pit, s);
  }

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_sti_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[1].mask &= ~(0x01);
  ptframe->view[1].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STI))) ? 1 : 0) << 0;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_stii_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[1].mask &= ~(0x02);
  ptframe->view[1].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STII))) ? 1 : 0) << 1;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_stiii_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[1].mask &= ~(0x04);
  ptframe->view[1].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STIII))) ? 1 : 0) << 2;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_stavr_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[1].mask &= ~(0x08);
  ptframe->view[1].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STAVR))) ? 1 : 0) << 3;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_stavl_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[1].mask &= ~(0x10);
  ptframe->view[1].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STAVL))) ? 1 : 0) << 4;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_stavf_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[1].mask &= ~(0x20);
  ptframe->view[1].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STAVF))) ? 1 : 0) << 5;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite2_stv_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[1].mask &= ~(0x40);
  ptframe->view[1].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STV))) ? 1 : 0) << 6;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite3_show_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();
  inpfoc_item_t *pit;
  char s[200];

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_SHOW);

  assert(pit);
  assert(ptframe);

  ptframe->view[2].visible = !ptframe->view[2].visible;

  ids2string(ptframe->view[2].visible ? IDS_YES : IDS_NO, s);

  inpfoc_wnd_setcaption(pit, s);

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite3_min_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_t * ptframe = tframe_getptr();
  int vl, vu;
  char s[10];

  assert(ptframe);

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_MIN);
  assert(pit);

  vl = ptframe->view[2].vl;
  vu = ptframe->view[2].vu;

  vl += 1*pcmd->delta;
  while (vl < 0)    vl += (499 - 0 + 1);
  while (vl > 499)  vl -= (499 - 0 + 1);

  if (vl < vu)
  {
    snprintf(s, sizeof(s), "%.01f", (float)vl / ptframe->view[2].kv);
    ptframe->view[2].vl = vl;
    inpfoc_wnd_setcaption(pit, s);
  }

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite3_max_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_t * ptframe = tframe_getptr();
  int vl, vu;
  char s[10];

  assert(ptframe);

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_MAX);
  assert(pit);

  vl = ptframe->view[2].vl;
  vu = ptframe->view[2].vu;

  vu += 1*pcmd->delta;
  while (vu < 1)    vu += (500 - 1 + 1);
  while (vu > 500)  vu -= (500 - 1 + 1);

  if (vu > vl)
  {
    snprintf(s, sizeof(s), "%.01f", (float)vu / ptframe->view[2].kv);
    ptframe->view[2].vu = vu;
    inpfoc_wnd_setcaption(pit, s);
  }

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite3_t1_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[2].mask &= ~(0x01);
  ptframe->view[2].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_T1))) ? 1 : 0) << 0;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite3_t2_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[2].mask &= ~(0x02);
  ptframe->view[2].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_T2))) ? 1 : 0) << 1;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite3_dt_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[2].mask &= ~(0x04);
  ptframe->view[2].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_DT))) ? 1 : 0) << 2;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite4_show_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();
  inpfoc_item_t *pit;
  char s[200];

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_SHOW);

  assert(pit);
  assert(ptframe);

  ptframe->view[3].visible = !ptframe->view[3].visible;

  ids2string(ptframe->view[3].visible ? IDS_YES : IDS_NO, s);

  inpfoc_wnd_setcaption(pit, s);

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite4_min_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_t * ptframe = tframe_getptr();
  int vl, vu;
  char s[10];

  assert(ptframe);

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_MIN);
  assert(pit);

  vl = ptframe->view[3].vl;
  vu = ptframe->view[3].vu;

  vl += 1*pcmd->delta;
  while (vl < 0)   vl += (49 - 0 + 1);
  while (vl > 49)  vl -= (49 - 0 + 1);

  if (vl < vu)
  {
    snprintf(s, sizeof(s), "%d", vl);
    ptframe->view[3].vl = vl;
    inpfoc_wnd_setcaption(pit, s);
  }

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite4_max_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tframe_t * ptframe = tframe_getptr();
  int vl, vu;
  char s[10];

  assert(ptframe);

  pit = inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_MAX);
  assert(pit);

  vl = ptframe->view[3].vl;
  vu = ptframe->view[3].vu;

  vu += 1*pcmd->delta;
  while (vu < 1)   vu += (120 - 1 + 1);
  while (vu > 120) vu -= (120 - 1 + 1);

  if (vu > vl)
  {
    snprintf(s, sizeof(s), "%d", vu);
    ptframe->view[3].vu = vu;
    inpfoc_wnd_setcaption(pit, s);
  }

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite4_br_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[3].mask &= ~(0x01);
  ptframe->view[3].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_BR))) ? 1 : 0) << 0;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite4_brc_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[3].mask &= ~(0x02);
  ptframe->view[3].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_BRC))) ? 1 : 0) << 1;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite4_etco2_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[3].mask &= ~(0x04);
  ptframe->view[3].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_ETCO2))) ? 1 : 0) << 2;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_suite4_ico2_func(void * parg)
{
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  ptframe->view[3].mask &= ~(0x08);
  ptframe->view[3].mask |= ((inpfoc_wnd_getchecked(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_ICO2))) ? 1 : 0) << 3;

  tframe_command(TFRAME_RELOAD, NULL);
}

static void trnset_exit_func(void * parg)
{
  rect_t rc;

  uframe_command(UFRAME_DESTROY, NULL);

  rc.x0 = TFRAME_X0;
  rc.y0 = TFRAME_Y0;
  rc.x1 = rc.x0 + TFRAME_CX;
  rc.y1 = rc.y0 + TFRAME_CY;
  tframe_command(TFRAME_RESIZE, &rc);

  tframe_cfg_save();
}

void menu_trendset_openproc(void)
{
  char s[200];
  tframe_t * ptframe = tframe_getptr();

  assert(ptframe);

  inpfoc_set(INPFOC_TRENDSET, TRENDSET_EXIT);

  ids2string(ptframe->view[0].visible ? IDS_YES : IDS_NO, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_SHOW), s);
  snprintf(s, sizeof(s), "%d", ptframe->view[0].vl);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_MIN), s);
  snprintf(s, sizeof(s), "%d", ptframe->view[0].vu);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_MAX), s);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_HR), (ptframe->view[0].mask & 0x1) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_NIBP), (ptframe->view[0].mask & 0x2) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_SPO2), (ptframe->view[0].mask & 0x4) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE1_PULSE), (ptframe->view[0].mask & 0x8) ? IWC_CHECKED : IWC_UNCHECKED);

  ids2string(ptframe->view[1].visible ? IDS_YES : IDS_NO, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_SHOW), s);
 // snprintf(s, sizeof(s), "%.02f", (float)ptframe->view[1].vl / ptframe->view[1].kv );
  snprintf(s, sizeof(s), "%d", ptframe->view[1].vl);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_MIN), s);
  snprintf(s, sizeof(s), "%d", ptframe->view[1].vu);
 // snprintf(s, sizeof(s), "%.02f", (float)ptframe->view[1].vu / ptframe->view[1].kv);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_MAX), s);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STI), (ptframe->view[1].mask & 0x1) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STII), (ptframe->view[1].mask & 0x2) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STIII), (ptframe->view[1].mask & 0x4) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STAVR), (ptframe->view[1].mask & 0x8) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STAVL), (ptframe->view[1].mask & 0x10) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STAVF), (ptframe->view[1].mask & 0x20) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE2_STV), (ptframe->view[1].mask & 0x40) ? IWC_CHECKED : IWC_UNCHECKED);

  ids2string(ptframe->view[2].visible ? IDS_YES : IDS_NO, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_SHOW), s);
  snprintf(s, sizeof(s), "%.01f", (float)ptframe->view[2].vl / ptframe->view[2].kv );
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_MIN), s);
  snprintf(s, sizeof(s), "%.01f", (float)ptframe->view[2].vu / ptframe->view[2].kv );
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_MAX), s);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_T1), (ptframe->view[2].mask & 0x1) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_T2), (ptframe->view[2].mask & 0x2) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE3_DT), (ptframe->view[2].mask & 0x4) ? IWC_CHECKED : IWC_UNCHECKED);

  ids2string(ptframe->view[3].visible ? IDS_YES : IDS_NO, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_SHOW), s);
  snprintf(s, sizeof(s), "%d", ptframe->view[3].vl);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_MIN), s);
  snprintf(s, sizeof(s), "%d", ptframe->view[3].vu);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_MAX), s);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_BR), (ptframe->view[3].mask & 0x1) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_BRC), (ptframe->view[3].mask & 0x2) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_ETCO2), (ptframe->view[3].mask & 0x4) ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_TRENDSET, TRENDSET_SUITE4_ICO2), (ptframe->view[3].mask & 0x8) ? IWC_CHECKED : IWC_UNCHECKED);

}

