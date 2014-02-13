/*! \file respcalc.c
 *  \brief Resp rate and apnoe detection algorithm
 */
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "resp.h"

#ifdef UNIX
#include "bedmoni.h"
#include "dview.h"
#include "alarm.h"
#include "mframe.h"
#include "sched.h"
#include "dproc.h"
#include "tr.h"
#include "resp.h"
#include "ecgcalc.h"
#include "respcalc.h"
#else
#define  yfprintf  fprintf
#endif
#if defined (RESPCALC_CUBIC_SPLINE)
#include "cubic.h"
#endif

#if defined (RESPCALC_COLIBRI)

#pragma pack(1)
static struct
{
  unsigned char apnoe : 1;
  unsigned char br    : 7;
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
resp_pars;
#pragma pack()

int find_breath(short * buf, int len, int apnoe_time, unsigned char * pb);
static void respcalc_proc(void);

static int wp;
static int num_data_in_acc = 0;

static short resp_acc_buf[RESPCALC_PROC_INTERVAL];
static short resp_proc_buf[RESPCALC_PROC_INTERVAL];

#if defined (RESPCALC_CUBIC_SPLINE)
short pulse_beep_pos[CUBIC_SPLINE_MAX_NUM_ITEMS];
int pulse_beep_pos_ptr;
int cubic_fd;
#endif

void respcalc_init(void)
{
  respcalc_reset();
#if defined (RESPCALC_CUBIC_SPLINE)
  cubic_fd = cubic_spline_init();
#endif
}

void respcalc_deinit(void)
{
#if defined (RESPCALC_CUBIC_SPLINE)
  cubic_spline_deinit(cubic_fd);
#endif
}

void respcalc_reset(void)
{
  alarm_clr(RESP_RISKBR);
  wp = 0;
  num_data_in_acc = 0;
  memset(resp_acc_buf, 0, sizeof(resp_acc_buf));

  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_STATE, IDS_UNDEF7);
  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR, UNDEF_VALUE);

#if defined (RESPCALC_CUBIC_SPLINE)
  memset(pulse_beep_pos, 0xff, sizeof(pulse_beep_pos));
  pulse_beep_pos_ptr = 0;
#endif
}

void respcalc_pulse_beep(void)
{
#if defined (RESPCALC_CUBIC_SPLINE)
  pulse_beep_pos[pulse_beep_pos_ptr++] = wp;
  if (pulse_beep_pos_ptr >= CUBIC_SPLINE_MAX_NUM_ITEMS)
    pulse_beep_pos_ptr = 0;
#endif
}

void respcalc_add_value(short val , unsigned char break_byte, unsigned char lead)
{
#if 0
  static FILE * ff = 0;
  if (!ff) ff = fopen("respdata_raw.txt", "wt");
  if (ff)
  {
    fprintf(ff, "rpg=%d\n", (int) val);
    fflush(ff);
  }
#endif
#define RESP_DATA_ACC_SIZE       40

#if defined (RESPCALC_CUBIC_SPLINE)
#undef RESP_DATA_ACC_SIZE
#define RESP_DATA_ACC_SIZE       10

  static int beeps_valid_counter = 0;
  static int pulse_beep_pos_ptr_prev = 0;
  beeps_valid_counter ++;
  if (pulse_beep_pos_ptr != pulse_beep_pos_ptr_prev)
  {
    pulse_beep_pos_ptr_prev = pulse_beep_pos_ptr;
    beeps_valid_counter = 0;
  }
  if (beeps_valid_counter > 3*FD_ECG) // no any beep during 3 sec
  {
    memset(pulse_beep_pos, 0xff, sizeof(pulse_beep_pos));
    pulse_beep_pos_ptr = 0;
    beeps_valid_counter = 0;
  }
#endif

  long sum_rpg;
  static int ptr = 0;
  static short resp_data_acc[RESP_DATA_ACC_SIZE];

  resp_data_acc[ptr++] = val;
  if ((ptr % 10) == 0)
  {
    int i;
    for (i=0,sum_rpg=0;i<RESP_DATA_ACC_SIZE;i++)
      sum_rpg += resp_data_acc[i];
    val = sum_rpg / RESP_DATA_ACC_SIZE;
  }
  else
  {
    return;
  }
  if (ptr >= RESP_DATA_ACC_SIZE) ptr = 0;

#if !defined (RESPCALC_COLIBRI)
  // resp analysis is implemented in SBDP since v 1.2
  return;
#endif

  if (
      ((break_byte & 0x5) != 0 && lead == 1) || // RF
      ((break_byte & 0x3) != 0 && lead == 0)    // RL
     )
  {
    // if (num_data_in_acc) num_data_in_acc --;
    num_data_in_acc = 0;
    alarm_clr(RESP_APNOE);
  }

  resp_acc_buf[wp++] = val;
  if (wp >= RESPCALC_PROC_INTERVAL)
  {
    wp = 0;
  }

#if defined (RESPCALC_CUBIC_SPLINE)
  int i;
  for (i=0; i<CUBIC_SPLINE_MAX_NUM_ITEMS; i++)
  {
    if (pulse_beep_pos[i] == wp) pulse_beep_pos[i] = (short)0xffff;
  }
#endif

  num_data_in_acc ++;

  if (num_data_in_acc > RESPCALC_PROC_INTERVAL)
  {
    num_data_in_acc = RESPCALC_PROC_INTERVAL;
  }

  if (num_data_in_acc >= RESPCALC_MINCALC_INTERVAL && ((num_data_in_acc % FD_RPG) == 0))
  {
    // start process data in resp_proc_buf
    sched_start(SCHED_RESP, 0, respcalc_proc, SCHED_DO_ONCE);
  }
}

static void respcalc_proc(void)
{
  resp_data_t resp_data;
  unsigned char st;
  int rp;
  int resp_proc_buf_len;

 // rp = wp;
  rp = wp - num_data_in_acc;
  if (rp < 0) rp += RESPCALC_PROC_INTERVAL;

  memcpy(&resp_proc_buf[0], &resp_acc_buf[rp], sizeof(short)*(RESPCALC_PROC_INTERVAL-rp));
  memcpy(&resp_proc_buf[RESPCALC_PROC_INTERVAL-rp], &resp_acc_buf[0], sizeof(short)*rp);

  resp_proc_buf_len = num_data_in_acc;

#if defined (RESPCALC_CUBIC_SPLINE)
  int pulse_beep_rp, i, n, beeps_valid, i0, ns;
  beeps_valid = 1;
  pulse_beep_rp = -1;
  for (n=0, i=pulse_beep_pos_ptr; n<CUBIC_SPLINE_MAX_NUM_ITEMS; n++, i++)
  {
    if (i >= CUBIC_SPLINE_MAX_NUM_ITEMS) i -= CUBIC_SPLINE_MAX_NUM_ITEMS;
    if (pulse_beep_pos[i] == (short)0xffff) continue;
    pulse_beep_rp = i;
    break;
  }
  if (pulse_beep_rp == -1)
  {
    beeps_valid = 0;
  }
  i0 = pulse_beep_pos[pulse_beep_rp];
  ns = 0;
  cubic_spline_clear_all(cubic_fd);
  for (i=0, n=0; i<CUBIC_SPLINE_MAX_NUM_ITEMS; i++)
  {
    if (beeps_valid == 0) break;
    if (pulse_beep_pos[pulse_beep_rp] != (short)0xffff)
    {
      int ii;
      ii = pulse_beep_pos[pulse_beep_rp] - i0;
      if (ii < 0) ii += RESPCALC_PROC_INTERVAL;
//printf("add %f %f\n", (float)(ii), (float)(resp_proc_buf[pulse_beep_pos[pulse_beep_rp]]));
      cubic_spline_add(cubic_fd, (double)(ii), (double)(resp_proc_buf[pulse_beep_pos[pulse_beep_rp]]));
      ns = ii;
      n ++;
    }
    pulse_beep_rp ++;
    if (pulse_beep_rp >= CUBIC_SPLINE_MAX_NUM_ITEMS) pulse_beep_rp -= CUBIC_SPLINE_MAX_NUM_ITEMS;
  }
  cubic_spline_build(cubic_fd);

//printf("SPLINE %d   %d  (%d-%d)\n", (n > 8 && beeps_valid), resp_proc_buf_len, i0, (i0+ns>=RESPCALC_PROC_INTERVAL)?i0+ns-RESPCALC_PROC_INTERVAL:i0+ns);

  if (n > 8 && beeps_valid)
  {
   // debug("spline resp\n");
    for (n=0; n<ns; n++)
    {
      if (n >= RESPCALC_PROC_INTERVAL) break;
      resp_proc_buf[n] = (short) cubic_spline_build_f(cubic_fd, n);

//if (n<20) printf("%d ", resp_proc_buf[n]);
//if (n==20) printf("\n");

    }
    resp_proc_buf_len = n;
  }
  if (beeps_valid == 0)
  {
    memset(pulse_beep_pos, 0xff, sizeof(pulse_beep_pos));
    pulse_beep_pos_ptr = 0;
  }
//printf("DO %d\n", resp_proc_buf_len);
#endif

  if (unit_get_data(RESP, &resp_data) <= 0)
  {
    error("%s: reading resp data\n", __FUNCTION__);
  }

  resp_pars.br = 0x7F;
  resp_pars.apnoe = 0;

  if (alarm_isset(RESP_BADCONTACT_RL) || alarm_isset(RESP_BADCONTACT_RF))
  {
    // bad contact, do not process rpg for breathing
  }
  else
  {
    // process resp data
    find_breath(resp_proc_buf, resp_proc_buf_len, resp_data.ap_max, (unsigned char*)&resp_pars);
  }

  resp_data.br = (resp_pars.br != 0x7F) ? resp_pars.br : UNDEF_VALUE;
  resp_data.ap = resp_pars.apnoe;

  unit_ioctl(RESP, SET_VALUE, UNIT_RESP_BR, resp_data.br);
  st = 0;
  if (resp_data.br != UNDEF_VALUE)
  {
    if      (resp_data.br < resp_data.br_min) st |= TR_ST_L;
    else if (resp_data.br > resp_data.br_max) st |= TR_ST_U;
  }
  alarm_set_clr(RESP_RISKBR, st && !resp_data.ap);

  alarm_set_clr(RESP_APNOE, resp_data.ap);

  if (!(alarm_isset(RESP_BADCONTACT_RL) || alarm_isset(RESP_BADCONTACT_RF)))
  {
    unit_ioctl(RESP, SET_VALUE, UNIT_RESP_STATE, (resp_data.ap)?IDS_APNOE:IDS_NORM);
  }

  dproc_add_trv(RESP, PARAM_BR, resp_data.br, st);

  if (unit_set_data(RESP, &resp_data) <= 0)
  {
    error("%s: writing resp data\n", __FUNCTION__);
  }

  // to call respcalc_proc every 1 sec
  if (num_data_in_acc >= RESPCALC_PROC_INTERVAL)
    num_data_in_acc -= 1*FD_RPG;
}

#endif // defined (RESPCALC_COLIBRI)

#define MAX_NUM_EXTR            300

static unsigned char Breath_Rate;
#if 1 //def WIN32
#define CycBr_max  200
#else
static const int CycBr_max = 200;
#endif

typedef struct
{
  short          extremum;
  short          location;
  unsigned char  extr_type;
  signed char    result;
} F_Br_extr;

typedef struct
{
  short ampl;
  short dlit;
} Amp_Dlitelnost;

typedef struct
{
  unsigned char  valid_b;
 // float          Cyc_BV;
  int            mult_Br;
  int            Zone_Br[4+1];
  int            Amp_Zone_Br[4+1];
  unsigned char  ident; // 12 - vdoh-pauza, 22 - vydoh-pauza, 33 - vdoh-vydoh, 0 - pauza v cikle dyhaniya
} CycBreath;

//typedef struct
//{
//  unsigned short position_S;
//}  Rheo_cyc_Mark;

#ifdef WIN32
static short FBlk1[RESPCALC_PROC_INTERVAL];
static short FBlk1_Inv[RESPCALC_PROC_INTERVAL];
#endif
// CA
static int cmi_br, cmi_stek;
static int stek_ptr = 0;
static CycBreath CycMarkBreath[CycBr_max];
static Amp_Dlitelnost Cyc_Amp_Dlit[CycBr_max];
static short mediana_signal, disp_signal;
static short max_signal, min_signal;
static int porog;
static int Problem_calc;

static short mean_Amp_Br_Cyc, disp_Amp_Br_Cyc, max_Amp_Br_Cyc, min_Amp_Br_Cyc;
static short mean_Dlit_Br_Cyc, disp_Dlit_Br_Cyc, max_Dlit_Br_Cyc, min_Dlit_br_Cyc;

static short extr_count, cyc_sum;
static int stek_full;
//static float t_i, t_e, t_p;
static CycBreath Breath_array[200];
//static Rheo_cyc_Mark mass_Rheo_cyc[200];
static F_Br_extr Extr_array[MAX_NUM_EXTR];
static Amp_Dlitelnost Amp_Dlit[200];
static float kritery[5+1];

static int Fblk_br_full;
static int Apnoe = 0;

static void resp_debug(char *fmt, ...)
#ifdef RESP_DEBUG
{
  va_list  args;
  char s[200];

  va_start(args, fmt);
  yfprintf(stdout, "resp: ");
  vsprintf(s, fmt, args);
  yfprintf(stdout, s);
  fflush(stdout);
// #ifdef RESP_LOGGING
//   fprintf(flog, "resp: ");
//   vfprintf(flog, fmt, args);
//   fflush(flog);
// #endif
  va_end( args );
}
#else // !RESP_DEBUG
{}
#endif

static void Breath_stats(short * buf, short * pA, short * pD, short * pmin_Br, short * pmax_Br, int N)
{
  int i;
  long A;
  double D;
  short min_Br, max_Br;

  A = 0;
  D = min_Br = max_Br = 0;
  for (i=0; i<N; i++)
  {
    A += buf[i];
    D += buf[i]*buf[i];
    if (max_Br > buf[i]) max_Br = buf[i];
    if (min_Br < buf[i]) min_Br = buf[i];
  }
  A /= N;
  D = D/N-A*A;
  D = (D>1e-6) ? sqrt(D) : 1;

  *pA = (short)A;
  *pD = (short)D;
  *pmin_Br = min_Br;
  *pmax_Br = max_Br;
}

static void Breath_Cyc_stats(short * pA1, short * pD1, short * pA2, short * pD2, short * pmax_Amp, short * pmin_Amp, short * pmax_Dlit, short * pmin_Dlit, int n)
{
  int i;
  short max_Amp, min_Amp, max_Dlit, min_Dlit;
  long A1, D1, A2, D2;

  A1 = D1 = A2 = D2 = max_Amp = max_Dlit = 0;
  min_Amp = 200;
  min_Dlit = 100;

  if (n == 0) return;

  for (i=0; i<n; i++)
  {
    A1 = A1 + Amp_Dlit[i].ampl;
    D1 = D1 + (Amp_Dlit[i].ampl)*(Amp_Dlit[i].ampl);
    A2 = A2 + Amp_Dlit[i].dlit;
    D2 = D2 + (Amp_Dlit[i].dlit)*(Amp_Dlit[i].dlit);
    if (Amp_Dlit[i].ampl > max_Amp)  max_Amp  = Amp_Dlit[i].ampl;
    if (Amp_Dlit[i].dlit > max_Dlit) max_Dlit = Amp_Dlit[i].dlit;
    if (Amp_Dlit[i].ampl < min_Amp)  min_Amp  = Amp_Dlit[i].ampl;
    if (Amp_Dlit[i].dlit < min_Dlit) min_Dlit = Amp_Dlit[i].dlit;
  }
  A1 = A1 / n;
  D1 = D1/n - A1*A1;
  A2 = A2 / n;
  D2 = D2/n - A2*A2;

  D1 = (D1>0) ? (short)sqrt(D1) : 1;
  D2 = (D2>0) ? (short)sqrt(D2) : 1;

  *pA1 = (short)A1;
  *pD1 = (short)D1;
  *pA2 = (short)A2;
  *pD2 = (short)D2;
  *pmax_Amp = (short)max_Amp;
  *pmin_Amp = (short)min_Amp;
  *pmax_Dlit = (short)max_Dlit;
  *pmin_Dlit = (short)min_Dlit;
}

static void Breath_mean_cut(short * buf, short mean, int n)
{
  int i;

  for (i=0; i<n; i++)
  {
    buf[i] -= mean;
  }
}

static int NextUp(short * buf, int i0, int i1, int * pj4, short* pUp)
{
  short Up;
  int j3, ret;
  ret = 0;
  Up = buf[i0];
  for (j3=i0; j3<i1; j3++)
  {
    if (buf[j3] > Up)
    {
      Up = buf[j3];
      if (pj4) *pj4 = j3;
      if (pUp) *pUp = Up;
    }
  }
  if (Up>buf[i1-1] && Up>buf[i0]) ret = 1;

  return ret;
}

static int NextDown(short * buf, int i0, int i1, int * pj5, short * pDown)
{
  short Down;
  int j3, ret;
  ret = 0;
  Down = buf[i0];
  for (j3=i0; j3<i1; j3++)
  {
    if (buf[j3] < Down)
    {
      Down = buf[j3];
      if (pj5) *pj5 = j3;
      if (pDown) *pDown = Down;
    }
  }
  if (Down<buf[i1-1] && Down<buf[i0]) ret = 1;

  return ret;
}

static void Find_Extremum(short * buf, int n)
{
  int i, j, j4, j5;
  short Up, Down;
  Up = Down = 0;
  j4 = j5 = 0;

  for (i=0; i<200; i++)
    Extr_array[i].extr_type = 0;
  extr_count = 1;
  for (i=0; i<n; i++)
  {
    if (extr_count >= MAX_NUM_EXTR) break;
    j = i+FD_RPG/2;
    if (j>n) j = n;
    if (NextUp(buf, i, j, &j4, &Up))
    {
      if (extr_count == 1 || (extr_count > 1 && j4 > Extr_array[extr_count-1].location))
      {
        Extr_array[extr_count].extremum = Up;
        Extr_array[extr_count].location = j4;
        Extr_array[extr_count].extr_type = 1;
        extr_count ++;
      }
    }
    if (NextDown(buf, i,j,&j5,&Down))
    {
      if (extr_count == 1 || (extr_count > 1 && j5 > Extr_array[extr_count-1].location))
      {
        Extr_array[extr_count].extremum = Down;
        Extr_array[extr_count].location = j5;
        Extr_array[extr_count].extr_type = 2;
        extr_count ++;
      }
    }
  }
  extr_count --;
  if (extr_count < 1)
  {
    cyc_sum = 1;
    Problem_calc = 1;
    Fblk_br_full = 0;
  }
  else
  {
    cyc_sum = extr_count;
  }

  if (!Problem_calc)
  {
    cyc_sum = 0;
    i = 0;
   // for (i=0; i<extr_count-1; i++)
    do
    {
      if (Extr_array[i].extr_type == Extr_array[i+1].extr_type && Extr_array[i].extr_type == 0)
      {
        i++;
        continue;
      }
      else
      if (Extr_array[i].extr_type == Extr_array[i+1].extr_type && Extr_array[i].extr_type == 1)
      {
        if (Extr_array[i].extremum > Extr_array[i+1].extremum)
          memmove(&Extr_array[i+1], &Extr_array[i+2], (extr_count-i-2)*sizeof(F_Br_extr));
        else
          memmove(&Extr_array[i], &Extr_array[i+1], (extr_count-i-1)*sizeof(F_Br_extr));
        extr_count --;
      }
      else
      if (Extr_array[i].extr_type == Extr_array[i+1].extr_type && Extr_array[i].extr_type == 2)
      {
        if (Extr_array[i].extremum > Extr_array[i+1].extremum)
          memmove(&Extr_array[i], &Extr_array[i+1], (extr_count-i-1)*sizeof(F_Br_extr));
        else
          memmove(&Extr_array[i+1], &Extr_array[i+2], (extr_count-i-2)*sizeof(F_Br_extr));
        extr_count --;
      }
      else
      {
        i ++;
        cyc_sum ++;
      }
    }
    while (i<extr_count-1);
  }
  extr_count = cyc_sum;
  do {} while (0);
}

static int H_Breath_Cyc(int x1, int y1, int x2, int y2, int x3, int y3)
{
  float k;
  long xa, ya;
  if (y3 == y1)
  {
    xa = x2;
    ya = y1;
  }
  else
  {
    k = ((float)y3-y1) / (x3-x1);
    xa = (int) ( ((float)y2-y3+(1/k)*x2+k*x3)/(k+(1/k)) );
    ya = (int) ((k*xa) + (y1-k*x1) );
  }
  return (int)(sqrt((x2-xa)*(x2-xa)+(y2-ya)*(y2-ya)));
}

static void Analize_Extr_Breath(int count, float disp_, float max_, float min_)
{
  unsigned short i,j;
  int Dlit1;
  unsigned short max_Dlit1;
  int max_H;
  float Sum_kritery;

   max_Dlit1 = 0;
   max_H = 0;
   for (i=1; i<count; i++)
   {
     if (abs(Extr_array[i].extremum-Extr_array[i-1].extremum)>max_H)
      max_H = abs(Extr_array[i].extremum-Extr_array[i-1].extremum);
     if (Extr_array[i].location-Extr_array[i-1].location>max_Dlit1)
      max_Dlit1 = Extr_array[i].location-Extr_array[i-1].location;
   }
  cyc_sum = count;

  i = 0;
  cyc_sum = 0;
  do
  {
    Dlit1 = Extr_array[i+1].location-Extr_array[i].location;
    if (Extr_array[i+1].extr_type == 0)
    {
      i ++;
      continue;
    }
    if (Dlit1 < 5*(FD_RPG/25))
    {
      int k;
      for (k=i; k<extr_count; k++)
      {
        Extr_array[k] = Extr_array[k+2];
      }
      extr_count -= 2;
    }
    else
    {
      cyc_sum ++;
      i ++;
    }
  } while (i<extr_count-1);
  extr_count = cyc_sum;
  count = cyc_sum;

  Sum_kritery = 0;
  for (i=0; i<count; i++)
  {
    if (Extr_array[i].extr_type == 1)
    {
      if (i == count)
        kritery[0] = 0.5;
      else
        kritery[0] = (float) fabs(((float)Extr_array[i+1].extremum-Extr_array[i].extremum)/max_H);
      if (i == 1)
        kritery[1] = 0.5;
      else
        kritery[1] = (float) fabs(((float)Extr_array[i].extremum-Extr_array[i-1].extremum)/max_H);
      if (i == count)
        kritery[2] = 0.5;
      else
        kritery[2] = ((float)Extr_array[i+1].location-Extr_array[i].location-5*(FD_RPG/25))/max_Dlit1;
      if (i == 1)
        kritery[3] = 0.5;
      else
        kritery[3] = ((float)Extr_array[i].location-Extr_array[i-1].location-5*(FD_RPG/25))/max_Dlit1;
      if (i == 1)
        kritery[4] = 0.5;
      else
        kritery[4] = (float) (fabs((float)Extr_array[i].extremum-Extr_array[i-1].extremum)/fabs(max_-min_));

      for (j=0; j<4; j++)
      {
        Sum_kritery = Sum_kritery+kritery[j];
      }
      kritery[5] = (Sum_kritery/5)*(Sum_kritery/5);
      Extr_array[i].result = (int)(kritery[5]*100);
      if (Extr_array[i].result<4.9/*5.1*/)
        Extr_array[i].result = 0;
      Sum_kritery = 0;
    }
    else
    {
      kritery[5] = 0;
      Extr_array[i].result = 0;
    }
  } // {for i+1 to count}

#if 1 // new fix -> May 2012
  for (i=0; i<count-1; i++)
  {
    if (Extr_array[i].result == 0 && Extr_array[i].extr_type == 1)
      Extr_array[i].extr_type = Extr_array[i+1].extr_type;
  }

  cyc_sum = 0;
  i = 0;
  do
  {
    if (Extr_array[i].extr_type == Extr_array[i+1].extr_type && Extr_array[i].extr_type == 0)
    {
      i++;
      continue;
    }
    else
    if (Extr_array[i].extr_type == Extr_array[i+1].extr_type && Extr_array[i].extr_type == 1)
    {
      if (Extr_array[i].extremum > Extr_array[i+1].extremum)
        memmove(&Extr_array[i+1], &Extr_array[i+2], (extr_count-i-2)*sizeof(F_Br_extr));
      else
        memmove(&Extr_array[i], &Extr_array[i+1], (extr_count-i-1)*sizeof(F_Br_extr));
      extr_count --;
    }
    else
    if (Extr_array[i].extr_type == Extr_array[i+1].extr_type && Extr_array[i].extr_type == 2)
    {
      if (Extr_array[i].extremum > Extr_array[i+1].extremum)
        memmove(&Extr_array[i], &Extr_array[i+1], (extr_count-i-1)*sizeof(F_Br_extr));
      else
        memmove(&Extr_array[i+1], &Extr_array[i+2], (extr_count-i-2)*sizeof(F_Br_extr));
      extr_count --;
    }
    else
    {
      i ++;
      cyc_sum ++;
    }
  }
  while (i<extr_count-1);
  extr_count = cyc_sum;
#endif

  cmi_br = 0;
  for (i=0; i<count-1; i++)
  {
    if (Extr_array[i].extr_type == 1 && Extr_array[i+1].extr_type == 2 && Extr_array[i+2].extr_type == 1)
    {
      Breath_array[cmi_br].valid_b = 1;
      Breath_array[cmi_br].Zone_Br[1] = Extr_array[i].location;
      Breath_array[cmi_br].Amp_Zone_Br[1] = Extr_array[i].extremum;
      Breath_array[cmi_br].Zone_Br[2] = Extr_array[i+1].location;
      Breath_array[cmi_br].Amp_Zone_Br[2] = Extr_array[i+1].extremum;
      Breath_array[cmi_br].Zone_Br[3] = Extr_array[i+1].location;
      Breath_array[cmi_br].Amp_Zone_Br[3] = Extr_array[i+1].extremum;
      Breath_array[cmi_br].Zone_Br[4] = Extr_array[i+2].location;
      Breath_array[cmi_br].Amp_Zone_Br[4] = Extr_array[i+2].extremum;
      if (Extr_array[i].result>0 && Extr_array[i+2].result == 0)
        Breath_array[cmi_br].ident = 12;
      if (Extr_array[i].result == 0 && Extr_array[i+2].result>0)
        Breath_array[cmi_br].ident = 22;
      if (Extr_array[i].result>0 && Extr_array[i+2].result == 0)
        Breath_array[cmi_br].ident = 0;
      if (Extr_array[i].result>0 && Extr_array[i+2].result>0)
        Breath_array[cmi_br].ident = 33;
      cmi_br ++;
    }
  }
  cmi_br--;
  for (i=0; i<cmi_br; i++)
  {
    Amp_Dlit[i].ampl = H_Breath_Cyc(Breath_array[i].Zone_Br[1],Breath_array[i].Amp_Zone_Br[1],Breath_array[i].Zone_Br[2],Breath_array[i].Amp_Zone_Br[2],Breath_array[i].Zone_Br[4],Breath_array[i].Amp_Zone_Br[4]);
    Breath_array[i].mult_Br = H_Breath_Cyc(Breath_array[i].Zone_Br[1],Breath_array[i].Amp_Zone_Br[1],Breath_array[i].Zone_Br[2],Breath_array[i].Amp_Zone_Br[2],Breath_array[i].Zone_Br[4],Breath_array[i].Amp_Zone_Br[4]);
    Amp_Dlit[i].dlit = Breath_array[i].Zone_Br[4] - Breath_array[i].Zone_Br[1];
  }
  do {} while (0);
} // procedure Analize_Extr_Breath

static void Record_in_stek(void)
{
  int i;
  for (i=0; i<cmi_br; i++)
  {
    CycMarkBreath[stek_ptr] = Breath_array[i];
    Cyc_Amp_Dlit[stek_ptr] = Amp_Dlit[i];
    stek_ptr ++;
    if (stek_ptr == CycBr_max)
      stek_ptr = 0;
  }
  cmi_stek += cmi_br;
  if (cmi_stek > CycBr_max) cmi_stek = CycBr_max;
} // procedure Record_in_stek

static void Analize_Breath_Cyc(int count, int apnoe_time, short mean_Amp, short mean_Dlit, int * pporog, int stek)
{
  int i;
  float H_porog;
  int Cyc_val_2;
  int Time_stop;
 // float Qmax,Qcarry;
 // float porog_new;
  int porog_ = 15;

  Cyc_val_2 = 0;
 // porog_new = 0;

  if (pporog)
   porog_ = *pporog;

  for (i=0; i<count; i++)
  {
    if (!Breath_array[i].valid_b) continue;
    if (Breath_array[i].mult_Br<(mean_Amp*porog_/100))
      Breath_array[i].valid_b = 2;

    if (Breath_array[i].mult_Br<250)
      Breath_array[i].valid_b = 2;

    if (Breath_array[i].Zone_Br[4] - Breath_array[i].Zone_Br[1] > (FD_RPG*apnoe_time))
    {
      Apnoe = 1;
      return;
    }
  }

  if (!stek)
  {
    Time_stop = 0;
    Apnoe = 0;
    H_porog = 0;
    Cyc_val_2 = 0;

    int valid_b3_cnt = 0;

    for (i=0; i<count; i++)
    {
      if (Breath_array[i].valid_b == 2 || Breath_array[i].valid_b == 3)
      {
        if (Breath_array[i].valid_b == 3) valid_b3_cnt ++;
        Time_stop = Time_stop+(Breath_array[i].Zone_Br[4] - Breath_array[i].Zone_Br[1]);
        H_porog = H_porog+Breath_array[i].mult_Br;
        Cyc_val_2 ++;
        if (Time_stop>(apnoe_time*FD_RPG))
          Apnoe = 1;
      }
     /* if (Breath_array[i].valid_b != 3)
      {
        if (valid_b3_cnt < 4)
        {
          Time_stop = 0;
          Apnoe = 0;
        }
        valid_b3_cnt = 0;
      }*/
      if (Breath_array[i].valid_b == 1/* || Breath_array[i].valid_b == 0*/)
      {
        Time_stop = 0;
        Apnoe = 0;
      }
    } // {for 2}

#if 0
    if (H_porog > 0 && Cyc_val_2 > 0)
      porog_ = (int) ((double)2*H_porog/Cyc_val_2/mean_Amp*100);
#else
   // debug("%s: %d\n", __FUNCTION__, __LINE__);
#endif
  }
  if (stek)
  {
    int ptr;
/*    H_porog = 0;
    ptr = stek_ptr;
    Qmax = (float) ( (double)mean_Dlit/(sqrt((mean_Dlit*mean_Dlit)+(mean_Amp*porog_/100)*(mean_Amp*porog_/100))) );
    for (i=0; i<cmi_stek; i++)
    {
      ptr --;
      if (ptr < 0) ptr += CycBr_max;
      Qcarry = (float) ( Cyc_Amp_Dlit[ptr].ampl/(sqrt((Cyc_Amp_Dlit[ptr].dlit*Cyc_Amp_Dlit[ptr].dlit)+(Cyc_Amp_Dlit[ptr].ampl*Cyc_Amp_Dlit[ptr].ampl))) );
      if (Qcarry<Qmax)
        CycMarkBreath[ptr].valid_b = 2;
      if (CycMarkBreath[ptr].valid_b == 2)
      {
        H_porog = H_porog+CycMarkBreath[ptr].mult_Br;
        Cyc_val_2 ++;
      }
    }

//    porog_ = (int)((double)2*H_porog/Cyc_val_2/mean_Amp*100);
printf("porog = %d\n", porog_);
*/
    Time_stop = 0;
    Apnoe = 0;
    H_porog = 0;
    Cyc_val_2 = 0;
    ptr = stek_ptr;
    for (i=0; i<cmi_stek; i++)
    {
      ptr --;
      if (ptr < 0) ptr += CycBr_max;
      if (CycMarkBreath[ptr].mult_Br<(porog_*mean_Amp)/100)
        CycMarkBreath[ptr].valid_b = 2;
      else
        CycMarkBreath[ptr].valid_b = 1;
      if (CycMarkBreath[ptr].valid_b == 2)
      {
        Time_stop = Time_stop+(CycMarkBreath[ptr].Zone_Br[4]-CycMarkBreath[ptr].Zone_Br[1]);
        H_porog = H_porog+CycMarkBreath[ptr].mult_Br;
        Cyc_val_2 ++;
        if (Time_stop>apnoe_time*FD_RPG)
          Apnoe = 1;
      }
      if (CycMarkBreath[ptr].valid_b == 1)
      {
        Time_stop = 0;
        Apnoe = 0;
      }
    }
//    porog_ = (int)((double)2*H_porog/Cyc_val_2/mean_Amp*100);
  }
  if (pporog)
    *pporog = porog_;
}

static int Calc_Breath_Rate(int count)
{
#if 1
  int i;
  int cyc_val1;
  float Calc_Breath;

  if (count < 2) return 0xFF;

  Calc_Breath = 0;
  cyc_val1 = 0;
  for (i=count-2; i>=0; i--)
  {
    if (Breath_array[i].valid_b == 1)
    {
       Calc_Breath = Calc_Breath+(Breath_array[i].Zone_Br[4]-Breath_array[i].Zone_Br[1]);
       cyc_val1 ++;
    }
    if (cyc_val1 >= 10) break;
  }
  return (int) (60*FD_RPG/(Calc_Breath/cyc_val1));
#else
/*
  int i;
  int cyc_val1;
  float Calc_Breath;
  int dlit, prev_dlit;

  Calc_Breath = 0;
  cyc_val1 = 0;
  prev_dlit = 0;
  for (i=count-1; i>=0; i--)
  {
    if (Breath_array[i].valid_b == 1)
    {
      dlit = (Breath_array[i].Zone_Br[4]-Breath_array[i].Zone_Br[1]);
      Calc_Breath = Calc_Breath+dlit;
      cyc_val1 ++;
      if (dlit != 0)
      {
        if ( ((double)abs(dlit - prev_dlit) / dlit) > 0.3 && cyc_val1 > 1)
        {
          printf("PERESTROIKA\n");
          break;
        }
      }
      prev_dlit = dlit;
    }
  }
  if (cyc_val1 > 1)
    return (int) (60*FD_RPG/(Calc_Breath/cyc_val1));
  else
    return 0xFF;
*/
  int i;
  int cyc_val1;
  float Calc_Breath;
  int dlit, prev_dlit;
  int ptr;

  Calc_Breath = 0;
  cyc_val1 = 0;
  prev_dlit = 0;
  ptr = stek_ptr;
  for (i=0; i<cmi_stek; i++)
  {
    ptr --;
    if (ptr < 0) ptr += CycBr_max;
    if (CycMarkBreath[ptr].valid_b == 1)
    {
      dlit = (CycMarkBreath[ptr].Zone_Br[4]-CycMarkBreath[ptr].Zone_Br[1]);
      Calc_Breath += dlit;
      cyc_val1 ++;
      if (dlit != 0 && prev_dlit != 0 && cyc_val1 > 1)
      {
        if ( (((double)abs(dlit - prev_dlit) / dlit) > 0.6) || (((double)abs(dlit - prev_dlit) / prev_dlit) > 0.6) )
        {
//          printf("PERESTROIKA %d, %d %d | %d %d (%d)\n", cyc_val1, dlit, prev_dlit, 60*FD_RPG/dlit, 60*FD_RPG/prev_dlit, cmi_stek);
          Calc_Breath -= dlit;
          cyc_val1 -= 1;
          break;
        }
      }
      prev_dlit = dlit;
    }
  }
  if (cyc_val1 >= 1)
    return (int) (60*FD_RPG/(Calc_Breath/cyc_val1));
  else
    return 0xFF;
#endif
}

static void Mark_Rheo_Cyc(int count)
{
} // procedure Mark_rheo_cyc

int find_breath(short * buf, int len, int apnoe_time, unsigned char * pb)
{
  resp_debug("find_breath\n");

#if 0
#if defined (UNIX) && !defined (ARM)
  static FILE * fw = 0;
  if (!fw)
  {
    debug("start record debug resp data\n");
    fw = fopen("respdata.dat", "wb");
  }
  if (fw)
  {
    static unsigned long v = 0x00;
    fwrite(&v, 1, 4, fw);
    v ++;
    assert(len == RESPCALC_PROC_INTERVAL);
    fwrite(buf, 1, len*2, fw);
    fflush(fw);
   // return -1;
  }
#endif
#endif

  cmi_br = 0;
  Problem_calc = 0;

  // ???
  Apnoe = 0;

  Fblk_br_full = 1;

  if (Fblk_br_full)
  {
    if (cmi_stek == 0) { cmi_stek = 0; stek_full = 0; porog = 15; }
    if (cmi_stek < 0)  { cmi_stek = 0; stek_full = 0; porog = 15; }
    if (cmi_stek > CycBr_max) { cmi_stek = 0; stek_full = 0; porog = 15; }

    Breath_stats(buf, &mediana_signal, &disp_signal, &max_signal, &min_signal, len);

    Breath_mean_cut(buf, mediana_signal, len);

    Find_Extremum(buf, len);

    if (!Problem_calc)
    {
      Analize_Extr_Breath(extr_count, disp_signal, max_signal, min_signal);

      if (cmi_br > 0)
      {
        Breath_Cyc_stats(&mean_Amp_Br_Cyc,&disp_Amp_Br_Cyc,
                         &mean_Dlit_Br_Cyc,&disp_Dlit_Br_Cyc,
                         &max_Amp_Br_Cyc,&min_Amp_Br_Cyc,
                         &max_Dlit_Br_Cyc,&min_Dlit_br_Cyc,cmi_br);

        Record_in_stek();
      }
      else
      {
        Apnoe = 1;
      }

      if (cmi_stek == CycBr_max)
        stek_full = 1;
    }

    if (cmi_br>1)
    {
      // calc by stek is disabled as stek is no needed as we analyze 80 sec record
      Analize_Breath_Cyc(cmi_br, apnoe_time, mean_Amp_Br_Cyc, mean_Dlit_Br_Cyc, &porog, 0*stek_full);

      Mark_Rheo_Cyc(cmi_br);
    }
    else
    {
      Apnoe = 1;
    }

    Breath_Rate = 0xFF;
    if (!Apnoe)
    {
      if (Calc_Breath_Rate(cmi_br)>0)
        Breath_Rate = Calc_Breath_Rate(cmi_br); //(60*FD_RPG) / mean_Dlit_Br_Cyc; //Calc_Breath_Rate(cmi_br);
    }
    else
      Breath_Rate = 0;
  } // !Problem_calc ???

  if (Apnoe && len<RESPCALC_PROC_INTERVAL/2)
  {
    Breath_Rate = 0xFF;
    Apnoe = 0;
  }

  if (pb) *pb = (Breath_Rate << 1) | ((Apnoe) ? 0x01 : 0x00);

  resp_debug("BR = %d\n", Breath_Rate);

  return 0;
}

#ifdef WIN32
int main(int argc, char ** argv)
{
  int i;
  unsigned char b;
  short w;
  FILE * f;
  unsigned long v;
  int pos;

  f = fopen("respdata.dat", "rb");
  assert(f);

  pos = 0;
  fseek(f, 0*(2*RESPCALC_PROC_INTERVAL+4), SEEK_SET);
  while (!feof(f))
  {
    memset(FBlk1, 0, sizeof(FBlk1));
    fread(&v, 1, 4, f);
    for (i=0; i<RESPCALC_PROC_INTERVAL; i++)
    {
      fread(&w, 1, 2, f);
      if (feof(f)) break;
      FBlk1[i] = w;
   // FBlk1[i] = (short) (1000*sin(2*3.14926*0.2*i*0.02));
      FBlk1_Inv[i] = -w;
    }

#if 0
    {
      char s[200];
      static FILE * fw = 0;
      if (!fw)
      {
        sprintf(s, "FBlk1_%d.xls", pos);
        fw = fopen(s, "wt");
      }
      if (fw)
      {
        for (i=0; i<RESPCALC_PROC_INTERVAL; i++)
          fprintf(fw, "%d\n", -FBlk1[i]);
      }
      fflush(fw);
      fclose(fw);
      fw = 0;
    }
#endif
    if (pos == 25)
      do {} while (0);

    pos ++;

    find_breath(FBlk1, RESPCALC_PROC_INTERVAL, 10, &b);
  }

  fclose(f);

  return 0;
}
#endif

