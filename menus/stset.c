#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "bedmoni.h"
#include "dio.h"
#include "dview.h"
#include "ecgcalc.h"
#include "alarm.h"
#include "mframe.h"
#include "uframe.h"
#include "cframe.h"
#include "inpfoc.h"
/*! \file stset.c
 *  \brief Popup frame connected with determination of ST shift on ECG
 */

#include "inpfoc_wnd.h"
#include "stset.h"

static void stset_none_func(void*);
static void stalr_sti_d_func(void*);
static void stalr_sti_r_func(void*);
static void stalr_sti_a_func(void*);
static void stalr_stii_d_func(void*);
static void stalr_stii_r_func(void*);
static void stalr_stii_a_func(void*);
static void stalr_stiii_d_func(void*);
static void stalr_stiii_r_func(void*);
static void stalr_stiii_a_func(void*);
static void stalr_stavr_d_func(void*);
static void stalr_stavr_r_func(void*);
static void stalr_stavr_a_func(void*);
static void stalr_stavl_d_func(void*);
static void stalr_stavl_r_func(void*);
static void stalr_stavl_a_func(void*);
static void stalr_stavf_d_func(void*);
static void stalr_stavf_r_func(void*);
static void stalr_stavf_a_func(void*);
static void stalr_stv_d_func(void*);
static void stalr_stv_r_func(void*);
static void stalr_stv_a_func(void*);
static void stset_alrall_func(void*);
static void stset_st1_func(void*);
static void stset_st2_func(void*);
static void stset_jshift_func(void*);
static void stset_exit_func(void*);

inpfoc_funclist_t inpfoc_stset_funclist[STSET_NUM_ITEMS+1] = 
{
  { STSET_NONE,     stset_none_func         },
  { STALR_STI_D,    stalr_sti_d_func        },
  { STALR_STI_R,    stalr_sti_r_func        },
  { STALR_STI_A,    stalr_sti_a_func        },
  { STALR_STII_D,   stalr_stii_d_func       },
  { STALR_STII_R,   stalr_stii_r_func       },
  { STALR_STII_A,   stalr_stii_a_func       },
  { STALR_STIII_D,  stalr_stiii_d_func      },
  { STALR_STIII_R,  stalr_stiii_r_func      },
  { STALR_STIII_A,  stalr_stiii_a_func      },
  { STALR_STAVR_D,  stalr_stavr_d_func      },
  { STALR_STAVR_R,  stalr_stavr_r_func      },
  { STALR_STAVR_A,  stalr_stavr_a_func      },
  { STALR_STAVL_D,  stalr_stavl_d_func      },
  { STALR_STAVL_R,  stalr_stavl_r_func      },
  { STALR_STAVL_A,  stalr_stavl_a_func      },
  { STALR_STAVF_D,  stalr_stavf_d_func      },
  { STALR_STAVF_R,  stalr_stavf_r_func      },
  { STALR_STAVF_A,  stalr_stavf_a_func      },
  { STALR_STV_D,    stalr_stv_d_func        },
  { STALR_STV_R,    stalr_stv_r_func        },
  { STALR_STV_A,    stalr_stv_a_func        },
  { STSET_ALRALL,   stset_alrall_func       },
  { STSET_ST1,      stset_st1_func          },
  { STSET_ST2,      stset_st2_func          },
  { STSET_JSHIFT,   stset_jshift_func       },
  { STSET_EXIT,     stset_exit_func         },
  { -1        ,     stset_none_func         }, // -1 must be last
};

static void stset_none_func(void * parg)
{

}

static void stalr_sti_d_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[100];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STI_D);
  ddh = ecg_data.st_max[ECG_I-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_I-ECG_FIRST];

  ddl += pcmd->delta*5;
  while (ddl < -200) ddl += (200-(-200))+5;
  while (ddl >  200) ddl -= (200-(-200))+5;

  if (ddl <= ddh)
  {
    ecg_data.st_min[ECG_I-ECG_FIRST] = ddl;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_I)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_I)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_min[ECG_I-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_sti_r_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STI_R);
  ddh = ecg_data.st_max[0];
  ddl = ecg_data.st_min[0];

  ddh += pcmd->delta*5;
  while (ddh < -200) ddh += (200-(-200))+5;
  while (ddh >  200) ddh -= (200-(-200))+5;

  if (ddh >= ddl)
  {
    ecg_data.st_max[0] = ddh;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_I)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_I)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_max[0]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_sti_a_func(void * parg)
{
  int v;
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);
  v = alarm_isenabled(ECG_RISKSTI);
  v = !v;
  alarm_on_off(ECG_RISKSTI, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;

  if (ecg_data.st1 == ECG_I)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, v);
  if (ecg_data.st2 == ECG_I)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, v);
}

static void stalr_stii_d_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STII_D);
  ddh = ecg_data.st_max[ECG_II-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_II-ECG_FIRST];

  ddl += pcmd->delta*5;
  while (ddl < -200) ddl += (200-(-200))+5;
  while (ddl >  200) ddl -= (200-(-200))+5;

  if (ddl <= ddh)
  {
    ecg_data.st_min[ECG_II-ECG_FIRST] = ddl;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_II)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_II)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_min[ECG_II-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stii_r_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STII_R);
  ddh = ecg_data.st_max[ECG_II-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_II-ECG_FIRST];

  ddh += pcmd->delta*5;
  while (ddh < -200) ddh += (200-(-200))+5;
  while (ddh >  200) ddh -= (200-(-200))+5;

  if (ddh >= ddl)
  {
    ecg_data.st_max[ECG_II-ECG_FIRST] = ddh;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_II)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_II)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_max[ECG_II-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stii_a_func(void * parg)
{
  int v;
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);
  v = alarm_isenabled(ECG_RISKSTII);
  v = !v;
  alarm_on_off(ECG_RISKSTII, v);
  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;

  if (ecg_data.st1 == ECG_II)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, v);
  if (ecg_data.st2 == ECG_II)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, v);
}

static void stalr_stiii_d_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STIII_D);
  ddh = ecg_data.st_max[ECG_III-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_III-ECG_FIRST];

  ddl += pcmd->delta*5;
  while (ddl < -200) ddl += (200-(-200))+5;
  while (ddl >  200) ddl -= (200-(-200))+5;

  if (ddl <= ddh)
  {
    ecg_data.st_min[ECG_III-ECG_FIRST] = ddl;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_III)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_III)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_min[ECG_III-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stiii_r_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STIII_R);
  ddh = ecg_data.st_max[ECG_III-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_III-ECG_FIRST];

  ddh += pcmd->delta*5;
  while (ddh < -200) ddh += (200-(-200))+5;
  while (ddh >  200) ddh -= (200-(-200))+5;

  if (ddh >= ddl)
  {
    ecg_data.st_max[ECG_III-ECG_FIRST] = ddh;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_III)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_III)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_max[ECG_III-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stiii_a_func(void * parg)
{
  int v;
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);
  v = alarm_isenabled(ECG_RISKSTIII);
  v = !v;
  alarm_on_off(ECG_RISKSTIII, v);

  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  if (ecg_data.st1 == ECG_III)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, v);
  if (ecg_data.st2 == ECG_III)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, v);
}

static void stalr_stavr_d_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STAVR_D);
  ddh = ecg_data.st_max[ECG_aVR-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_aVR-ECG_FIRST];

  ddl += pcmd->delta*5;
  while (ddl < -200) ddl += (200-(-200))+5;
  while (ddl >  200) ddl -= (200-(-200))+5;

  if (ddl <= ddh)
  {
    ecg_data.st_min[ECG_aVR-ECG_FIRST] = ddl;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_aVR)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_aVR)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_min[ECG_aVR-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stavr_r_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STAVR_R);
  ddh = ecg_data.st_max[ECG_aVR-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_aVR-ECG_FIRST];

  ddh += pcmd->delta*5;
  while (ddh < -200) ddh += (200-(-200))+5;
  while (ddh >  200) ddh -= (200-(-200))+5;

  if (ddh >= ddl)
  {
    ecg_data.st_max[ECG_aVR-ECG_FIRST] = ddh;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_aVR)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_aVR)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_max[ECG_aVR-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stavr_a_func(void * parg)
{
  int v;
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);
  v = alarm_isenabled(ECG_RISKSTAVR);
  v = !v;
  alarm_on_off(ECG_RISKSTAVR, v);

  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  if (ecg_data.st1 == ECG_aVR)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, v);
  if (ecg_data.st2 == ECG_aVR)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, v);
}

static void stalr_stavl_d_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STAVL_D);
  ddh = ecg_data.st_max[ECG_aVL-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_aVL-ECG_FIRST];

  ddl += pcmd->delta*5;
  while (ddl < -200) ddl += (200-(-200))+5;
  while (ddl >  200) ddl -= (200-(-200))+5;

  if (ddl <= ddh)
  {
    ecg_data.st_min[ECG_aVL-ECG_FIRST] = ddl;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_aVL)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_aVL)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_min[ECG_aVL-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stavl_r_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STAVL_R);
  ddh = ecg_data.st_max[ECG_aVL-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_aVL-ECG_FIRST];

  ddh += pcmd->delta*5;
  while (ddh < -200) ddh += (200-(-200))+5;
  while (ddh >  200) ddh -= (200-(-200))+5;

  if (ddh >= ddl)
  {
    ecg_data.st_max[ECG_aVL-ECG_FIRST] = ddh;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_aVL)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_aVL)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_max[ECG_aVL-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stavl_a_func(void * parg)
{
  int v;
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);
  v = alarm_isenabled(ECG_RISKSTAVL);
  v = !v;
  alarm_on_off(ECG_RISKSTAVL, v);

  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  if (ecg_data.st1 == ECG_aVL)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, v);
  if (ecg_data.st2 == ECG_aVL)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, v);
}

static void stalr_stavf_d_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STAVF_D);
  ddh = ecg_data.st_max[ECG_aVF-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_aVF-ECG_FIRST];

  ddl += pcmd->delta*5;
  while (ddl < -200) ddl += (200-(-200))+5;
  while (ddl >  200) ddl -= (200-(-200))+5;

  if (ddl <= ddh)
  {
    ecg_data.st_min[ECG_aVF-ECG_FIRST] = ddl;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_aVF)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_aVF)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_min[ECG_aVF-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stavf_r_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STAVF_R);
  ddh = ecg_data.st_max[ECG_aVF-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_aVF-ECG_FIRST];

  ddh += pcmd->delta*5;
  while (ddh < -200) ddh += (200-(-200))+5;
  while (ddh >  200) ddh -= (200-(-200))+5;

  if (ddh >= ddl)
  {
    ecg_data.st_max[ECG_aVF-ECG_FIRST] = ddh;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_aVF)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_aVF)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_max[ECG_aVF-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stavf_a_func(void * parg)
{
  int v;
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);
  v = alarm_isenabled(ECG_RISKSTAVF);
  v = !v;
  alarm_on_off(ECG_RISKSTAVF, v);

  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  if (ecg_data.st1 == ECG_aVF)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, v);
  if (ecg_data.st2 == ECG_aVF)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, v);
}

static void stalr_stv_d_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STV_D);
  ddh = ecg_data.st_max[ECG_V-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_V-ECG_FIRST];

  ddl += pcmd->delta*5;
  while (ddl < -200) ddl += (200-(-200))+5;
  while (ddl >  200) ddl -= (200-(-200))+5;

  if (ddl <= ddh)
  {
    ecg_data.st_min[ECG_V-ECG_FIRST] = ddl;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_V)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_V)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_min[ECG_V-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stv_r_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int ddl, ddh;
  char s[20];
  ecg_data_t ecg_data;

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  pit = inpfoc_find(INPFOC_STSET, STALR_STV_R);
  ddh = ecg_data.st_max[ECG_V-ECG_FIRST];
  ddl = ecg_data.st_min[ECG_V-ECG_FIRST];

  ddh += pcmd->delta*5;
  while (ddh < -200) ddh += (200-(-200))+5;
  while (ddh >  200) ddh -= (200-(-200))+5;

  if (ddh >= ddl)
  {
    ecg_data.st_max[ECG_V-ECG_FIRST] = ddh;
    unit_set_data(ECG, &ecg_data);

    if (ecg_data.st1 == ECG_V)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);
    }
    if (ecg_data.st2 == ECG_V)
    {
      sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
      unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);
    }

    snprintf(s, sizeof(s), "%d", ecg_data.st_max[ECG_V-ECG_FIRST]);
    inpfoc_wnd_setcaption(pit, s);
  }
}

static void stalr_stv_a_func(void * parg)
{
  int v;
  ecg_data_t ecg_data;

  unit_get_data(ECG, &ecg_data);
  v = alarm_isenabled(ECG_RISKSTV);
  v = !v;
  alarm_on_off(ECG_RISKSTV, v);

  v = (v) ? IMAGE_BELL : IMAGE_NOBELL;
  if (ecg_data.st1 == ECG_V)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, v);
  if (ecg_data.st2 == ECG_V)
    unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, v);
}

static void stset_alrall_func(void * parg)
{
  int v;
  v = inpfoc_wnd_getchecked(inpfoc_find(INPFOC_STSET, STSET_ALRALL));

  alarm_on_off(ECG_RISKSTI, v);
  alarm_on_off(ECG_RISKSTII, v);
  alarm_on_off(ECG_RISKSTIII, v);
  alarm_on_off(ECG_RISKSTAVR, v);
  alarm_on_off(ECG_RISKSTAVL, v);
  alarm_on_off(ECG_RISKSTAVF, v);
  alarm_on_off(ECG_RISKSTV, v);

  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STI_A),   v ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STII_A),  v ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STIII_A), v ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STAVR_A), v ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STAVL_A), v ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STAVF_A), v ? IWC_CHECKED : IWC_UNCHECKED);
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STV_A),   v ? IWC_CHECKED : IWC_UNCHECKED);

  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, v ? IMAGE_BELL : IMAGE_NOBELL);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, v ? IMAGE_BELL : IMAGE_NOBELL);
}

static void stset_st1_func(void * parg)
{
  ecg_data_t ecg_data;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[200];
  unsigned char ecg_first, ecg_last, num_ecg;

  unit_get_data(ECG, &ecg_data);

  switch (ecg_data.num_leads)
  {
    case 1:
      ecg_first = ECG_I;
      ecg_last = ECG_II;
      num_ecg = 2;
      break;
    case 3:
    case 5:
    default:
      ecg_first = ECG_FIRST;
      ecg_last = (ecg_data.num_leads == 3) ? ECG_aVF : ECG_LAST;
      num_ecg = (ecg_data.num_leads == 3) ? 6 : NUM_ECG;
      break;
  }

  ecg_data.st1 += pcmd->delta;

  while ((signed char)ecg_data.st1 < ecg_first) ecg_data.st1 += num_ecg;
  while ((signed char)ecg_data.st1 > ecg_last)  ecg_data.st1 -= num_ecg;

  strcpy(s, "ST ");
  ids2string(chan_ids[ecg_data.st1], s+strlen("ST "));
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STSET_ST1), s);

  ids2string(chan_ids[ecg_data.st1], s);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_LEAD_CAPTION, s);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_VALUE, ecg_data.st[ecg_data.st1-ECG_FIRST]);

  sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st1-ECG_FIRST], ecg_data.st_max[ecg_data.st1-ECG_FIRST]);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_RANGE, s);

  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST1_BELL, alarm_isenabled(ECG_RISKSTI+(ecg_data.st1-ECG_FIRST)) ? IMAGE_BELL : IMAGE_NOBELL);

  unit_set_data(ECG, &ecg_data);
}

static void stset_st2_func(void * parg)
{
  ecg_data_t ecg_data;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  char s[200];
  unsigned char ecg_first, ecg_last, num_ecg;

  unit_get_data(ECG, &ecg_data);

  switch (ecg_data.num_leads)
  {
    case 1:
      ecg_first = ECG_I;
      ecg_last = ECG_II;
      num_ecg = 2;
      break;
    case 3:
    case 5:
    default:
      ecg_first = ECG_FIRST;
      ecg_last = (ecg_data.num_leads == 3) ? ECG_aVF : ECG_LAST;
      num_ecg = (ecg_data.num_leads == 3) ? 6 : NUM_ECG;
      break;
  }

  ecg_data.st2 += pcmd->delta;

  while ((signed char)ecg_data.st2 < ecg_first) ecg_data.st2 += num_ecg;
  while ((signed char)ecg_data.st2 > ecg_last)  ecg_data.st2 -= num_ecg;

  strcpy(s, "ST ");
  ids2string(chan_ids[ecg_data.st2], s+strlen("ST "));
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STSET_ST2), s);

  ids2string(chan_ids[ecg_data.st2], s);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_LEAD_CAPTION, s);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_VALUE, ecg_data.st[ecg_data.st2-ECG_FIRST]);

  sprintf(s, "%d..%d", ecg_data.st_min[ecg_data.st2-ECG_FIRST], ecg_data.st_max[ecg_data.st2-ECG_FIRST]);
  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_RANGE, s);

  unit_ioctl(ECG, SET_VALUE, UNIT_ECG_ST2_BELL, alarm_isenabled(ECG_RISKSTI+(ecg_data.st2-ECG_FIRST)) ? IMAGE_BELL : IMAGE_NOBELL);

  unit_set_data(ECG, &ecg_data);
}

static void stset_jshift_func(void * parg)
{
  ecg_data_t ecg_data;
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int v;
  char s[200];

  unit_get_data(ECG, &ecg_data);

  pit = inpfoc_find(INPFOC_STSET, STSET_JSHIFT);
  v = ecg_data.j_shift;

  v += pcmd->delta*1;

  while (v > 300) v -= (300+1);
  while (v < 0)   v += (300+1);

  ecg_data.j_shift = v;

  unit_set_data(ECG, &ecg_data);

  sprintf(s, "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  dio_module_cmd(PD_ID_ECS, ECS_JPOINT_SHIFT, ecg_data.j_shift);
}

static void stset_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_ECGSET);
}

void menu_stset_openproc(void)
{
  ecg_data_t ecg_data;
  char s[200];

  memset(&ecg_data, 0, sizeof(ecg_data_t));
  if (unit_get_data(ECG, &ecg_data) <= 0)
  {
    debug("%s: error reading ecg data\n", __FUNCTION__);
  }

  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STI_A),   alarm_isenabled(ECG_RISKSTI));
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STII_A),  alarm_isenabled(ECG_RISKSTII));
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STIII_A), alarm_isenabled(ECG_RISKSTIII));
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STAVR_A), alarm_isenabled(ECG_RISKSTAVR));
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STAVL_A), alarm_isenabled(ECG_RISKSTAVL));
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STAVF_A), alarm_isenabled(ECG_RISKSTAVF));
  inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STALR_STV_A),   alarm_isenabled(ECG_RISKSTV));

  if
  (
    alarm_isenabled(ECG_RISKSTI) || alarm_isenabled(ECG_RISKSTII) || alarm_isenabled(ECG_RISKSTIII) ||
    alarm_isenabled(ECG_RISKSTAVR) || alarm_isenabled(ECG_RISKSTAVL) || alarm_isenabled(ECG_RISKSTAVF) ||
    alarm_isenabled(ECG_RISKSTV)
  )
  {
    inpfoc_wnd_check(inpfoc_find(INPFOC_STSET, STSET_ALRALL), IWC_CHECKED);
  }

  snprintf(s, sizeof(s), "%d", ecg_data.st_min[0]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STI_D), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_max[0]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STI_R), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_min[1]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STII_D), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_max[1]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STII_R), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_min[2]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STIII_D), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_max[2]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STIII_R), s);

  snprintf(s, sizeof(s), "%d", ecg_data.st_min[3]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STAVR_D), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_max[3]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STAVR_R), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_min[4]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STAVL_D), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_max[4]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STAVL_R), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_min[5]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STAVF_D), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_max[5]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STAVF_R), s);

  snprintf(s, sizeof(s), "%d", ecg_data.st_min[6]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STV_D), s);
  snprintf(s, sizeof(s), "%d", ecg_data.st_max[6]);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STALR_STV_R), s);

  strcpy(s, "ST ");
  ids2string(chan_ids[ecg_data.st1], s+strlen("ST "));
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STSET_ST1), s);
  ids2string(chan_ids[ecg_data.st2], s+strlen("ST "));
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STSET_ST2), s);

  if (ecg_data.num_leads == 3)
  {
    inpfoc_disable(INPFOC_STSET, STALR_STAVR_D);
    inpfoc_disable(INPFOC_STSET, STALR_STAVR_R);
    inpfoc_disable(INPFOC_STSET, STALR_STAVR_A);

    inpfoc_disable(INPFOC_STSET, STALR_STAVL_D);
    inpfoc_disable(INPFOC_STSET, STALR_STAVL_R);
    inpfoc_disable(INPFOC_STSET, STALR_STAVL_A);

    inpfoc_disable(INPFOC_STSET, STALR_STAVF_D);
    inpfoc_disable(INPFOC_STSET, STALR_STAVF_R);
    inpfoc_disable(INPFOC_STSET, STALR_STAVF_A);

    inpfoc_disable(INPFOC_STSET, STALR_STV_D);
    inpfoc_disable(INPFOC_STSET, STALR_STV_R);
    inpfoc_disable(INPFOC_STSET, STALR_STV_A);
  }

  sprintf(s, "%d", ecg_data.j_shift);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_STSET, STSET_JSHIFT), s);

  inpfoc_set(INPFOC_STSET, STSET_EXIT);
}

