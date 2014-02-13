/*! \file printset.c
 *  \brief Popup frame with printer settings
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "bedmoni.h"
#include "uframe.h"
#include "cframe.h"
#include "mframe.h"
#include "ecgm.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "tprn.h"
#include "printset.h"

extern float roundf(float x); // to disable warning

static void prnset_none_func(void*);
static void prnset_numcurves_func(void*);
static void prnset_curve1_func(void*);
static void prnset_curve1_scale_func(void*);
static void prnset_curve2_func(void*);
static void prnset_curve2_scale_func(void*);
static void prnset_mmps_func(void*);
static void prnset_prn_grid_func(void*);
static void prnset_prn_val_func(void*);
static void prnset_width_func(void*);
static void prnset_brightness_func(void*);
static void prnset_ts_func(void*);
static void prnset_delay1_func(void*);
static void prnset_delay2_func(void*);
static void prnset_exit_func(void*);

inpfoc_funclist_t inpfoc_prnset_funclist[PRINTSET_NUM_ITEMS+1] = 
{
  { PRINTSET_NONE,         prnset_none_func         },
  { PRINTSET_NUMCURVES,    prnset_numcurves_func    },
  { PRINTSET_CURVE1,       prnset_curve1_func       },
  { PRINTSET_CURVE1_SCALE, prnset_curve1_scale_func },
  { PRINTSET_CURVE2,       prnset_curve2_func       },
  { PRINTSET_CURVE2_SCALE, prnset_curve2_scale_func },
  { PRINTSET_MMPS,         prnset_mmps_func         },
  { PRINTSET_PRN_GRID,     prnset_prn_grid_func     },
  { PRINTSET_PRN_VAL,      prnset_prn_val_func      },
  { PRINTSET_WIDTH,        prnset_width_func        },
  { PRINTSET_BRIGHTNESS,   prnset_brightness_func   },
  { PRINTSET_DELAY1,       prnset_delay1_func       },
  { PRINTSET_DELAY2,       prnset_delay2_func       },
  { PRINTSET_TS,           prnset_ts_func           },
  { PRINTSET_EXIT,         prnset_exit_func         },
  { -1       ,             prnset_none_func         }, // -1 must be last
};

static void prnset_none_func(void * parg)
{

}

static void prnset_numcurves_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tprn_cfg_t tprn_cfg;
  char s[200], s2[200];

  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.n += pcmd->delta;
  while ((signed char)tprn_cfg.n > 2) tprn_cfg.n -= 2+1;
  while ((signed char)tprn_cfg.n < 0) tprn_cfg.n += 2+1;

  snprintf(s, sizeof(s), "%d", tprn_cfg.n);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_NUMCURVES), s);

  if (tprn_cfg.n > 1 && ((chno_is_ecg(tprn_cfg.c[0]) && tprn_cfg.amp[0] == TPRN_AMP_40) || (chno_is_ecg(tprn_cfg.c[1]) && tprn_cfg.amp[1] == TPRN_AMP_40) || ((tprn_cfg.amp[0] == TPRN_AMP_20 && chno_is_ecg(tprn_cfg.c[0])) && (chno_is_ecg(tprn_cfg.c[1]) && tprn_cfg.amp[1] == TPRN_AMP_20))))
  {
    if (tprn_cfg.amp[0] >= TPRN_AMP_20) tprn_cfg.amp[0] = TPRN_AMP_10;
    if (tprn_cfg.amp[1] >= TPRN_AMP_20) tprn_cfg.amp[1] = TPRN_AMP_10;

    if (chno_is_ecg(tprn_cfg.c[0]))
    {
      ids2string(IDS_MMPMV, s2);
      if ( tprn_cfg.amp[0] == TPRN_AMP_X1 )
        snprintf(s, sizeof(s), "%.1f %s", (float) tprn_amp_mul[tprn_cfg.c[0]]*1.0f, s2);
      else
        snprintf(s, sizeof(s), "%d %s", (int) (tprn_amp_mul[tprn_cfg.c[0]] * tprn_amp[tprn_cfg.amp[0]]), s2);
    }
    else
    {
      sprintf(s, "-");
    }
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE1_SCALE), s);

    if (chno_is_ecg(tprn_cfg.c[1]))
    {
      ids2string(IDS_MMPMV, s2);
      if ( tprn_cfg.amp[1] == TPRN_AMP_X1 )
        snprintf(s, sizeof(s), "%.1f %s", (float) tprn_amp_mul[tprn_cfg.c[1]]*1.0f, s2);
      else
        snprintf(s, sizeof(s), "%d %s", (int) (tprn_amp_mul[tprn_cfg.c[1]] * tprn_amp[tprn_cfg.amp[1]]), s2);
    }
    else
    {
      sprintf(s, "-");
    }
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE2_SCALE), s);
  }

  tprn_cfg_set(&tprn_cfg);

  tprn_reset_grph();
}

static void prnset_curve1_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tprn_cfg_t tprn_cfg;
  ecg_data_t ecg_data;
  char s[200], s2[200];

  unit_get_data(ECG, &ecg_data);
  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.c[0] += pcmd->delta;
  while ((signed char)tprn_cfg.c[0] >= NUM_VIEWS)  tprn_cfg.c[0] -= NUM_VIEWS;
  while ((signed char)tprn_cfg.c[0] < 0)           tprn_cfg.c[0] += NUM_VIEWS;
  while ( (tprn_cfg.c[0] == tprn_cfg.c[1]) || tprn_cfg.c[0] == NIBP || tprn_cfg.c[0] == T1T2 || tprn_cfg.c[0] == ECG_NEO ||
  ( (tprn_cfg.c[0] == ECG_I || tprn_cfg.c[0] == ECG_III || tprn_cfg.c[0] == ECG_aVR || tprn_cfg.c[0] == ECG_aVL || tprn_cfg.c[0] == ECG_aVF || tprn_cfg.c[0] == ECG_V) && ecg_data.num_leads == 1 ) ||
  ( tprn_cfg.c[0] == ECG_V && ecg_data.num_leads == 3 )
        )

  {
    tprn_cfg.c[0] += (pcmd->delta > 0) ? 1 : -1;
    while ((signed char)tprn_cfg.c[0] >= NUM_VIEWS)  tprn_cfg.c[0] -= NUM_VIEWS;
    while ((signed char)tprn_cfg.c[0] < 0)           tprn_cfg.c[0] += NUM_VIEWS;
  }

  ids2string(chan_ids[tprn_cfg.c[0]], s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE1), s);

  if (tprn_cfg.c[0] == CO2 && tprn_cfg.amp[0] > TPRN_AMP_X2)
  {
    tprn_cfg.amp[0] = TPRN_AMP_X2;
  }

  if (chno_is_ecg(tprn_cfg.c[0]))
  {
    ids2string(IDS_MMPMV, s2);
    if ( tprn_cfg.amp[0] == TPRN_AMP_X1 )
      snprintf(s, sizeof(s), "%.1f %s", (float) tprn_amp_mul[tprn_cfg.c[0]]*1.0f, s2);
    else
      snprintf(s, sizeof(s), "%d %s", (int) (tprn_amp_mul[tprn_cfg.c[0]] * tprn_amp[tprn_cfg.amp[0]]), s2);
  }
  else
  {
    snprintf(s, sizeof(s), "x%d", (int) tprn_amp[tprn_cfg.amp[0]]);
  }
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE1_SCALE), s);

  if (((chno_is_ecg(tprn_cfg.c[0]) && tprn_cfg.amp[0] == TPRN_AMP_40) || (( chno_is_ecg(tprn_cfg.c[0]) && tprn_cfg.amp[0] == TPRN_AMP_20) && (chno_is_ecg(tprn_cfg.c[1]) && tprn_cfg.amp[1] == TPRN_AMP_20))) && tprn_cfg.n > 1)
  {
    tprn_cfg.n = 1;

    snprintf(s, sizeof(s), "%d", tprn_cfg.n);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_NUMCURVES), s);

   // tprn_reset_grph();
  }
  tprn_cfg_set(&tprn_cfg);
}

static void prnset_curve1_scale_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tprn_cfg_t tprn_cfg;
  char s[200], s2[128];

  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.amp[0] += pcmd->delta;

  if (tprn_cfg.c[0] == CO2)
  {
    while ((signed char)tprn_cfg.amp[0] > TPRN_AMP_X2)  tprn_cfg.amp[0] -= TPRN_AMP_X2+1;
    while ((signed char)tprn_cfg.amp[0] < 0)            tprn_cfg.amp[0] += TPRN_AMP_X2+1;
  }

  while ((signed char)tprn_cfg.amp[0] >= MAXNUM_TPRN_AMP)  tprn_cfg.amp[0] -= MAXNUM_TPRN_AMP;
  while ((signed char)tprn_cfg.amp[0] < 0)                 tprn_cfg.amp[0] += MAXNUM_TPRN_AMP;

  if (chno_is_ecg(tprn_cfg.c[0]))
  {
    ids2string(IDS_MMPMV, s2);
    if ( tprn_cfg.amp[0] == TPRN_AMP_X1 )
      snprintf(s, sizeof(s), "%.1f %s", (float) tprn_amp_mul[tprn_cfg.c[0]]*1.0f, s2);
    else
      snprintf(s, sizeof(s), "%d %s", (int) (tprn_amp_mul[tprn_cfg.c[0]] * tprn_amp[tprn_cfg.amp[0]]), s2);
  }
  else
  {
    snprintf(s, sizeof(s), "x%d", (int) tprn_amp[tprn_cfg.amp[0]]);
  }

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE1_SCALE), s);

  if (((chno_is_ecg(tprn_cfg.c[0]) && tprn_cfg.amp[0] == TPRN_AMP_40) || (( chno_is_ecg(tprn_cfg.c[0]) && tprn_cfg.amp[0] == TPRN_AMP_20) && (chno_is_ecg(tprn_cfg.c[1]) && tprn_cfg.amp[1] == TPRN_AMP_20))) && tprn_cfg.n > 1)
  {
    tprn_cfg.n = 1;

    snprintf(s, sizeof(s), "%d", tprn_cfg.n);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_NUMCURVES), s);

   // tprn_reset_grph();
  }

  tprn_cfg_set(&tprn_cfg);
}

static void prnset_curve2_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tprn_cfg_t tprn_cfg;
  ecg_data_t ecg_data;
  char s[200], s2[200];

  unit_get_data(ECG, &ecg_data);
  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.c[1] += pcmd->delta;
  while ((signed char)tprn_cfg.c[1] >= NUM_VIEWS)  tprn_cfg.c[1] -= NUM_VIEWS;
  while ((signed char)tprn_cfg.c[1] < 0)           tprn_cfg.c[1] += NUM_VIEWS;
  while ( (tprn_cfg.c[1] == tprn_cfg.c[0]) || tprn_cfg.c[1] == NIBP || tprn_cfg.c[1] == T1T2 || tprn_cfg.c[1] == ECG_NEO ||
  ( (tprn_cfg.c[1] == ECG_I || tprn_cfg.c[1] == ECG_III || tprn_cfg.c[1] == ECG_aVR || tprn_cfg.c[1] == ECG_aVL || tprn_cfg.c[1] == ECG_aVF || tprn_cfg.c[1] == ECG_V) && ecg_data.num_leads == 1 ) ||
  ( tprn_cfg.c[1] == ECG_V && ecg_data.num_leads == 3 )
        )
  {
    tprn_cfg.c[1] += (pcmd->delta > 0) ? 1 : -1;
    while ((signed char)tprn_cfg.c[1] >= NUM_VIEWS)  tprn_cfg.c[1] -= NUM_VIEWS;
    while ((signed char)tprn_cfg.c[1] < 0)           tprn_cfg.c[1] += NUM_VIEWS;
  }

  ids2string(chan_ids[tprn_cfg.c[1]], s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE2), s);

  if (tprn_cfg.c[1] == CO2 && tprn_cfg.amp[1] > TPRN_AMP_X2)
  {
    tprn_cfg.amp[1] = TPRN_AMP_X2;
  }

  if (chno_is_ecg(tprn_cfg.c[1]))
  {
    ids2string(IDS_MMPMV, s2);
    if ( tprn_cfg.amp[1] == TPRN_AMP_X1 )
      snprintf(s, sizeof(s), "%.1f %s", (float) tprn_amp_mul[tprn_cfg.c[1]]*1.0f, s2);
    else
      snprintf(s, sizeof(s), "%d %s", (int) (tprn_amp_mul[tprn_cfg.c[1]] * tprn_amp[tprn_cfg.amp[1]]), s2);
  }
  else
  {
    snprintf(s, sizeof(s), "x%d", (int) tprn_amp[tprn_cfg.amp[1]]);
  }
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE2_SCALE), s);

  if (((chno_is_ecg(tprn_cfg.c[1]) && tprn_cfg.amp[1] == TPRN_AMP_40) || ((chno_is_ecg(tprn_cfg.c[0]) && tprn_cfg.amp[0] == TPRN_AMP_20) && (chno_is_ecg(tprn_cfg.c[1]) && tprn_cfg.amp[1] == TPRN_AMP_20))) && tprn_cfg.n > 1)
  {
    tprn_cfg.n = 1;

    snprintf(s, sizeof(s), "%d", tprn_cfg.n);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_NUMCURVES), s);

   // tprn_reset_grph();
  }
  tprn_cfg_set(&tprn_cfg);
}

static void prnset_curve2_scale_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tprn_cfg_t tprn_cfg;
  char s[200], s2[128];

  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.amp[1] += pcmd->delta;

  if (tprn_cfg.c[1] == CO2)
  {
    while ((signed char)tprn_cfg.amp[1] > TPRN_AMP_X2)  tprn_cfg.amp[1] -= TPRN_AMP_X2+1;
    while ((signed char)tprn_cfg.amp[1] < 0)            tprn_cfg.amp[1] += TPRN_AMP_X2+1;
  }

  while ((signed char)tprn_cfg.amp[1] >= MAXNUM_TPRN_AMP)  tprn_cfg.amp[1] -= MAXNUM_TPRN_AMP;
  while ((signed char)tprn_cfg.amp[1] < 0)                 tprn_cfg.amp[1] += MAXNUM_TPRN_AMP;

  if (chno_is_ecg(tprn_cfg.c[1]))
  {
    ids2string(IDS_MMPMV, s2);
    if ( tprn_cfg.amp[1] == TPRN_AMP_X1 )
      snprintf(s, sizeof(s), "%.1f %s", (float) tprn_amp_mul[tprn_cfg.c[1]]*1.0f, s2);
    else
      snprintf(s, sizeof(s), "%d %s", (int) (tprn_amp_mul[tprn_cfg.c[1]] * tprn_amp[tprn_cfg.amp[1]]), s2);
  }
  else
  {
    snprintf(s, sizeof(s), "x%d", (int) tprn_amp[tprn_cfg.amp[1]]);
  }

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE2_SCALE), s);

  if (((chno_is_ecg(tprn_cfg.c[1]) && tprn_cfg.amp[1] == TPRN_AMP_40) || ((chno_is_ecg(tprn_cfg.c[0]) && tprn_cfg.amp[0] == TPRN_AMP_20) && (chno_is_ecg(tprn_cfg.c[1]) && tprn_cfg.amp[1] == TPRN_AMP_20))) && tprn_cfg.n > 1)
  {
    tprn_cfg.n = 1;

    snprintf(s, sizeof(s), "%d", tprn_cfg.n);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_NUMCURVES), s);

   // tprn_reset_grph();
  }

  tprn_cfg_set(&tprn_cfg);
}

static void prnset_mmps_func(void * parg)
{
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  tprn_cfg_t tprn_cfg;
  char s[200], s2[128];

  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.spd += pcmd->delta;

  while ((signed char)tprn_cfg.spd >= MAXNUM_TPRN_SPD)  tprn_cfg.spd -= MAXNUM_TPRN_SPD;
  while ((signed char)tprn_cfg.spd < 0)                 tprn_cfg.spd += MAXNUM_TPRN_SPD;
  tprn_cfg_set(&tprn_cfg);

  ids2string(IDS_MMPS, s2);
  if ( (roundf(tprn_spd[tprn_cfg.spd]) - tprn_spd[tprn_cfg.spd]) == 0 )
    snprintf(s, sizeof(s), "%d %s", (int) tprn_spd[tprn_cfg.spd], s2);
  else
    snprintf(s, sizeof(s), "%.1f %s", (float) tprn_spd[tprn_cfg.spd], s2);

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_MMPS), s);
}

static void prnset_prn_grid_func(void * parg)
{
  tprn_cfg_t tprn_cfg;

  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.prnmask.grid = !tprn_cfg.prnmask.grid;
  tprn_cfg_set(&tprn_cfg);

  tprn_reset_grph();
}

static void prnset_prn_val_func(void * parg)
{
  tprn_cfg_t tprn_cfg;
  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.prnmask.val = !tprn_cfg.prnmask.val;
  tprn_cfg_set(&tprn_cfg);
}

static void prnset_width_func(void * parg)
{
  tprn_cfg_t tprn_cfg;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[10];
  int w;

  tprn_cfg_get(&tprn_cfg);
  w = tprn_cfg.prnmask.width;
  w += pcmd->delta;

  while (w > 3) w -= 3;
  while (w < 1) w += 3;

  tprn_cfg.prnmask.width = w;

  snprintf(s, sizeof(s), "%d", w);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_WIDTH), s);

  tprn_cfg_set(&tprn_cfg);
}

static void prnset_brightness_func(void * parg)
{
  tprn_cfg_t tprn_cfg;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int v;
  char s[10];

  tprn_cfg_get(&tprn_cfg);
  v = tprn_cfg.prnmask.brightness;

  v += pcmd->delta;

  while (v >= 10) v -= 11;
  while (v < 0)   v += 11;

  tprn_cfg.prnmask.brightness = v;

  snprintf(s, sizeof(s), "%d", v*10);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_BRIGHTNESS), s);

  tprn_cfg_set(&tprn_cfg);
}

static void prnset_delay1_func(void * parg)
{
  tprn_cfg_t tprn_cfg;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[200], s2[128];

  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.delay1 += pcmd->delta;

  while ((signed char)tprn_cfg.delay1 > 5)  tprn_cfg.delay1 -= (5-0)+1;
  while ((signed char)tprn_cfg.delay1 < 0)  tprn_cfg.delay1 += (5-0)+1;

  ids2string(IDS_sEC, s2);
  snprintf(s, sizeof(s), "%d %s", tprn_cfg.delay1, s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_DELAY1), s);

  inpfoc_wnd_setrange(inpfoc_find(INPFOC_PRINTSET, PRINTSET_TS), tprn_cfg.delay1+tprn_cfg.delay2);
  inpfoc_wnd_setpos(inpfoc_find(INPFOC_PRINTSET, PRINTSET_TS), tprn_cfg.delay1);

  tprn_cfg_set(&tprn_cfg);
}

static void prnset_delay2_func(void * parg)
{
  tprn_cfg_t tprn_cfg;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[200], s2[128];

  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.delay2 += pcmd->delta;

  while ((signed char)tprn_cfg.delay2 > 12)  tprn_cfg.delay2 -= (12-3)+1;
  while ((signed char)tprn_cfg.delay2 < 3)   tprn_cfg.delay2 += (12-3)+1;

  ids2string(IDS_sEC, s2);
  snprintf(s, sizeof(s), "%d %s", tprn_cfg.delay2, s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_DELAY2), s);

  inpfoc_wnd_setrange(inpfoc_find(INPFOC_PRINTSET, PRINTSET_TS), tprn_cfg.delay1+tprn_cfg.delay2);
  inpfoc_wnd_setpos(inpfoc_find(INPFOC_PRINTSET, PRINTSET_TS), tprn_cfg.delay1);

  tprn_cfg_set(&tprn_cfg);
}

static void prnset_ts_func(void * parg)
{
  // slider shifts automatically, inpfoc_wnd_scroll call is not needed
  inpfoc_cmd_t * pcmd = (inpfoc_cmd_t *) parg;
  inpfoc_wnd_scroll(inpfoc_find(INPFOC_PRINTSET, PRINTSET_TS), -pcmd->delta); // compensate scroll
}

static void prnset_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_GENERAL);

  tprn_cfg_save();
}

void menu_printset_openproc(void)
{
  tprn_cfg_t tprn_cfg;
  char s[200], s2[128];

  tprn_cfg_get(&tprn_cfg);

  snprintf(s, sizeof(s), "%d", tprn_cfg.n);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_NUMCURVES), s);
  ids2string(chan_ids[tprn_cfg.c[0]], s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE1), s);
  ids2string(chan_ids[tprn_cfg.c[1]], s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE2), s);

  if (chno_is_ecg(tprn_cfg.c[0]))
  {
    ids2string(IDS_MMPMV, s2);
    if ( tprn_cfg.amp[0] == TPRN_AMP_X1 )
      snprintf(s, sizeof(s), "%.1f %s", (float) tprn_amp_mul[tprn_cfg.c[0]]*1.0f, s2);
    else
      snprintf(s, sizeof(s), "%d %s", (int) (tprn_amp_mul[tprn_cfg.c[0]] * tprn_amp[tprn_cfg.amp[0]]), s2);
  }
  else
  {
    snprintf(s, sizeof(s), "x%d", (int) tprn_amp[tprn_cfg.amp[0]]);
  }
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE1_SCALE), s);

  if (chno_is_ecg(tprn_cfg.c[1]))
  {
    ids2string(IDS_MMPMV, s2);
    if ( tprn_cfg.amp[1] == TPRN_AMP_X1 )
      snprintf(s, sizeof(s), "%.1f %s", (float) tprn_amp_mul[tprn_cfg.c[1]]*1.0f, s2);
    else
      snprintf(s, sizeof(s), "%d %s", (int) (tprn_amp_mul[tprn_cfg.c[1]] * tprn_amp[tprn_cfg.amp[1]]), s2);
  }
  else
  {
    snprintf(s, sizeof(s), "x%d", (int) tprn_amp[tprn_cfg.amp[1]]);
  }
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_CURVE2_SCALE), s);

  ids2string(IDS_MMPS, s2);
  if ( (roundf(tprn_spd[tprn_cfg.spd]) - tprn_spd[tprn_cfg.spd]) == 0 )
    snprintf(s, sizeof(s), "%d %s", (int) tprn_spd[tprn_cfg.spd], s2);
  else
    snprintf(s, sizeof(s), "%.1f %s", (float) tprn_spd[tprn_cfg.spd], s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_MMPS), s);

  inpfoc_wnd_check(inpfoc_find(INPFOC_PRINTSET, PRINTSET_PRN_GRID), tprn_cfg.prnmask.grid);
  inpfoc_wnd_check(inpfoc_find(INPFOC_PRINTSET, PRINTSET_PRN_VAL), tprn_cfg.prnmask.val);

  snprintf(s, sizeof(s), "%d", tprn_cfg.prnmask.width);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_WIDTH), s);

  snprintf(s, sizeof(s), "%d", tprn_cfg.prnmask.brightness*10);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_BRIGHTNESS), s);

  ids2string(IDS_sEC, s2);
  snprintf(s, sizeof(s), "%d %s", tprn_cfg.delay1, s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_DELAY1), s);
  snprintf(s, sizeof(s), "%d %s", tprn_cfg.delay2, s2);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PRINTSET, PRINTSET_DELAY2), s);

  inpfoc_wnd_setrange(inpfoc_find(INPFOC_PRINTSET, PRINTSET_TS), tprn_cfg.delay1+tprn_cfg.delay2);
  inpfoc_wnd_setpos(inpfoc_find(INPFOC_PRINTSET, PRINTSET_TS), tprn_cfg.delay1);

  inpfoc_disable(INPFOC_PRINTSET, PRINTSET_BRIGHTNESS);
  inpfoc_disable(INPFOC_PRINTSET, PRINTSET_TS);

  inpfoc_set(INPFOC_PRINTSET, PRINTSET_EXIT);
}

