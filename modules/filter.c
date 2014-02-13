/*! \file filter.c
 *  \brief Digital filters
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "filter.h"

int qqrs_filt_init(void)
{
  qqrs_filt_data_t * pfd;
  pfd = (qqrs_filt_data_t*) calloc(1, sizeof(qqrs_filt_data_t));
  pfd->coef[0]  = pfd->coef[21-1-0]  = -446;
  pfd->coef[1]  = pfd->coef[21-1-1]  = -107;
  pfd->coef[2]  = pfd->coef[21-1-2]  = +295;
  pfd->coef[3]  = pfd->coef[21-1-3]  = +1893;
  pfd->coef[4]  = pfd->coef[21-1-4]  = +2732;
  pfd->coef[5]  = pfd->coef[21-1-5]  = +1520;
  pfd->coef[6]  = pfd->coef[21-1-6]  = +7284;
  pfd->coef[7]  = pfd->coef[21-1-7]  = -807;
  pfd->coef[8]  = pfd->coef[21-1-8]  = +12884;
  pfd->coef[9]  = pfd->coef[21-1-9]  = -1971;
  pfd->coef[10]                      = +16026;
  return (int)(pfd);
}

void qqrs_filt_deinit(int fd)
{
  free((void*)fd);
}

void qqrs_filt_reset(int fd)
{
  memset( ((qqrs_filt_data_t*)fd)->tbuf, 0, sizeof(((qqrs_filt_data_t*)fd)->tbuf) );
}

short qqrs_filt_stepit(const int fd, short w)
{
  qqrs_filt_data_t * pfd;
  long v;
  int i;

  pfd = (qqrs_filt_data_t*)fd;
  memmove(&pfd->tbuf[0], &pfd->tbuf[1], sizeof(pfd->tbuf)-sizeof(pfd->tbuf[0]));
  pfd->tbuf[21-1] = w;

  v = 0;
  for (i=0; i<21; i++)
  {
    v += pfd->coef[i]*pfd->tbuf[i];
  }

#ifdef ARM
  return (v >> 16);
#else
  return *((short*)&v + 1);
#endif
}

int hp_filt_init(double freq)
{
  const int adc_freq = 500; // quantity frequency
  hp_filt_data_t * pfd;
  pfd = (hp_filt_data_t*) calloc(1, sizeof(hp_filt_data_t));
  float k;

  assert(pfd);

  k = (float)tan(3.1415926f * freq / adc_freq);
  pfd->C11 = 1;
  pfd->D11 = k - pfd->C11;
  pfd->D01 = pfd->C11 + k;
  pfd->A11 = - pfd->C11;
  pfd->A01 = pfd->C11;

  return (int)pfd;
}

void hp_filt_deinit(int fd)
{
  hp_filt_data_t * pfd;
  pfd = (hp_filt_data_t*)fd;
 // assert(pfd);
  if (!pfd) return;
  free(pfd);
}

int hp_filt_stepit(int fd, int v)
{
  hp_filt_data_t * pfd;
  pfd = (hp_filt_data_t *) fd;

  if (fd == 0) return v;

  pfd->x[0] = pfd->x[1];
  pfd->x[1] = (float)v;
  pfd->y[0] = pfd->y[1];
  pfd->y[1] = (1 / pfd->D01) * (-pfd->D11 * pfd->y[0] + pfd->A01 * pfd->x[1] + pfd->A11 * pfd->x[0]);

  return (int) pfd->y[1];
}

int n5060_filt_init(void)
{
  n5060_filt_data_t * pfd;
  pfd = (n5060_filt_data_t*) calloc(1, sizeof(n5060_filt_data_t));

  pfd->coef1[0]  = pfd->coef1[4-1-0] = 11504;
  pfd->coef1[1]  = pfd->coef1[4-1-1] = 35683;

  pfd->coef2[0]  = pfd->coef2[8-1-0] = -2317;
  pfd->coef2[1]  = pfd->coef2[8-1-1] = -3505;
  pfd->coef2[2]  = pfd->coef2[8-1-2] = 12784;
  pfd->coef2[3]  = pfd->coef2[8-1-3] = 39111;

  pfd->coef3[0]  = pfd->coef3[14-1-0] = -518;
  pfd->coef3[1]  = pfd->coef3[14-1-1] = -498;
  pfd->coef3[2]  = pfd->coef3[14-1-2] = 2074;
  pfd->coef3[3]  = pfd->coef3[14-1-3] = 231;
  pfd->coef3[4]  = pfd->coef3[14-1-4] = -6476;
  pfd->coef3[5]  = pfd->coef3[14-1-5] = 5268;
  pfd->coef3[6]  = pfd->coef3[14-1-6] = 32420;

  return (int)(pfd);
}

void n5060_filt_deinit(int fd)
{
  if (!fd) return;
  free((void*)fd);
}

static int long_filter1(int fd, int len, data_t * pwi, data_t * pwo)
{
  n5060_filt_data_t * pfd;
  int i,j;
  long v;
  data_t * po = pwo;

  pfd = (n5060_filt_data_t*)fd;

  for (j=0; j<len; j++)
  {
    memmove(pfd->tbuf1d, pfd->tbuf1d+1, (4-1)*sizeof(data_t));
    pfd->tbuf1d[4-1] = pwi[j];
    v = 0;
    for (i=0; i<4; i++)
    {
      v += pfd->coef1[i]*pfd->tbuf1d[i];
    }

    if (j & 0x1)
#ifdef ARM
      *po++ = (v >> 16);
#else
      *po++ = *((short*)&v + 1);
#endif
  }

  return 1;
}

static int long_filter2(int fd, int len, data_t * pwi, data_t * pwo)
{
  n5060_filt_data_t * pfd;
  int i,j;
  long v;
  data_t * po = pwo;

  pfd = (n5060_filt_data_t*)fd;

  for (j=0; j<len; j++)
  {
    memmove(pfd->tbuf2d, pfd->tbuf2d+1, (8-1)*sizeof(data_t));
    pfd->tbuf2d[8-1] = pwi[j];
    v = 0;
    for (i=0; i<8; i++)
    {
      v += pfd->coef2[i]*pfd->tbuf2d[i];
    }

    if (j & 0x1)
#ifdef ARM
      *po++ = (v >> 16);
#else
      *po++ = *((short*)&v + 1);
#endif
  }

  return 1;
}

static int long_filter3(int fd, int len, data_t * pwi, data_t * pwo)
{
  n5060_filt_data_t * pfd;
  int i,j;
  long v;
  data_t * po = pwo;

  pfd = (n5060_filt_data_t*)fd;

  for (j=0; j<len; j++)
  {
    memmove(pfd->tbuf3, pfd->tbuf3+1, (14-1)*sizeof(data_t));
    pfd->tbuf3[14-1] = pwi[j];
    v = 0;
    for (i=0; i<14; i++)
    {
      v += pfd->coef3[i]*pfd->tbuf3[i];
    }

#ifdef ARM
    *po++ = (v >> 16);
#else
    *po++ = *((short*)&v + 1);
#endif
  }

  return 1;
}

static int long_filter4(int fd, int len, data_t * pwi, data_t * pwo)
{
  n5060_filt_data_t * pfd;
  int i,j;
  long v;
  data_t * po = pwo;

  pfd = (n5060_filt_data_t*)fd;

  for (j=0; j<2*len; j++)
  {
    memmove(pfd->tbuf2i, pfd->tbuf2i+1, (8-1)*sizeof(data_t));
    if (j&0x1)
      pfd->tbuf2i[8-1] = pwi[j/2];
    else
      pfd->tbuf2i[8-1] = 0;
    v = 0;
    for (i=0; i<8; i++)
    {
      v += pfd->coef2[i]*pfd->tbuf2i[i];
    }

#ifdef ARM
    *po++ = (v >> 16);
#else
    *po++ = *((short*)&v + 1);
#endif
  }

  return 1;
}

static int long_filter5(int fd, int len, data_t * pwi, data_t * pwo)
{
  n5060_filt_data_t * pfd;
  int i,j;
  long v;
  data_t * po = pwo;

  pfd = (n5060_filt_data_t*)fd;

  for (j=0; j<2*len; j++)
  {
    memmove(pfd->tbuf1i, pfd->tbuf1i+1, (4-1)*sizeof(data_t));
    if (j&0x1)
      pfd->tbuf1i[4-1] = pwi[j/2];
    else
      pfd->tbuf1i[4-1] = 0;
    v = 0;
    for (i=0; i<4; i++)
    {
      v += pfd->coef1[i]*pfd->tbuf1i[i];
    }

#ifdef ARM
    *po++ = (v >> 16);
#else
    *po++ = *((short*)&v + 1);
#endif
  }

  return 1;
}

data_t n5060_filt_stepit(int fd, data_t v)
{
  n5060_filt_data_t * pfd;
  data_t po1[2];
  data_t po2[1];
  data_t po3[1];

  pfd = (n5060_filt_data_t*)fd;
  if (!fd) return v;

  pfd->x[pfd->pos] = v;
  pfd->pos ++;
  if (pfd->pos == 4)
  {
    pfd->pos = 0;
    long_filter1(fd, 4, pfd->x, po1);
    long_filter2(fd, 2, po1, po2);
    long_filter3(fd, 1, po2, po3);
    long_filter4(fd, 1, po3, po1);
    long_filter5(fd, 2, po1, pfd->y);
  }

  return pfd->y[pfd->pos];
}

static long kfsum150HZ;

int s150_filt_init(void)
{
  s150_filt_data_t * pfd;
  int i;

  pfd = (s150_filt_data_t*) calloc(1, sizeof(s150_filt_data_t));

  if (!pfd) return 0;

  pfd->coef[0]  = pfd->coef[40-1-0]  = -0.041311706147252*32768,
  pfd->coef[1]  = pfd->coef[40-1-1]  = -0.006767890235791*32768,
  pfd->coef[2]  = pfd->coef[40-1-2]  =  0.002127402124426*32768,
  pfd->coef[3]  = pfd->coef[40-1-3]  =  0.016068337032538*32768,
  pfd->coef[4]  = pfd->coef[40-1-4]  =  0.049566300172619*32768,
  pfd->coef[5]  = pfd->coef[40-1-5]  =  0.048592299764481*32768,
  pfd->coef[6]  = pfd->coef[40-1-6]  =  0.009399018174259*32768,
  pfd->coef[7]  = pfd->coef[40-1-7]  = -0.005019738983644*32768,
  pfd->coef[8]  = pfd->coef[40-1-8]  = -0.010712721492315*32768,
  pfd->coef[9]  = pfd->coef[40-1-9]  = -0.056810514946780*32768,
  pfd->coef[10] = pfd->coef[40-1-10] = -0.073050494439215*32768,
  pfd->coef[11] = pfd->coef[40-1-11] = -0.008709577536872*32768,
  pfd->coef[12] = pfd->coef[40-1-12] =  0.024576431510918*32768,
  pfd->coef[13] = pfd->coef[40-1-13] = -0.005740986047014*32768,
  pfd->coef[14] = pfd->coef[40-1-14] =  0.041038010769597*32768,
  pfd->coef[15] = pfd->coef[40-1-15] =  0.122335854395558*32768,
  pfd->coef[16] = pfd->coef[40-1-16] =  0.022720220980213*32768,
  pfd->coef[17] = pfd->coef[40-1-17] = -0.120151173339820*32768,
  pfd->coef[18] = pfd->coef[40-1-18] =  0.066470559178806*32768,
  pfd->coef[19] = pfd->coef[40-1-19] =  0.433467282491911*32768,

  memset(pfd->tbuf, 0, sizeof(((s150_filt_data_t*)(0))->tbuf));

  kfsum150HZ = 0;
  for(i=0; i<40; i++)
  {
    kfsum150HZ += pfd->coef[i];
  }

  return (int)(pfd);
}

short s150_filt_stepit(int fd, short w)
{
  s150_filt_data_t * pfd;
  long v;
  int i;

  pfd = (s150_filt_data_t*)fd;
  memmove(&pfd->tbuf[0], &pfd->tbuf[1], sizeof(pfd->tbuf)-sizeof(pfd->tbuf[0]));
  pfd->tbuf[40-1] = w;

  v = 0;
  for (i=0; i<40; i++)
  {
    v += pfd->coef[i]*pfd->tbuf[i];
  }

  return v/kfsum150HZ;
}

void s150_filt_deinit(int fd)
{
  if (!fd) return;
  free((void*)fd);
}

int resp_filt_init(double freq)
{
  resp_filt_data_t * pfd;
  pfd = (resp_filt_data_t*) calloc(1, sizeof(resp_filt_data_t));

  assert(pfd);

  pfd->ptr = 0;

  return (int)pfd;
}

void resp_filt_deinit(int fd)
{
  resp_filt_data_t * pfd;
  pfd = (resp_filt_data_t*)fd;
  if (!pfd) return;
  free(pfd);
}

int resp_filt_stepit(int fd, int v)
{
  resp_filt_data_t * pfd;
  int i;
  long sum;

  if (!fd) return v;

  pfd = (resp_filt_data_t *) fd;
  pfd->tbuf[pfd->ptr] = v;
  pfd->ptr ++;
  if (pfd->ptr >= 40) pfd->ptr = 0;

  for (i=0,sum=0; i<40; i++)
  {
    sum += pfd->tbuf[i];
  }

  return (int)(sum/40);
}

static long kfsum37HZ;

int s37_filt_init(void)
{
  s37_filt_data_t * pfd;
  int i;

  pfd = (s37_filt_data_t*) calloc(1, sizeof(s37_filt_data_t));

  if (!pfd) return 0;

  pfd->coef[0]  = pfd->coef[20-1-0]  = -198,
  pfd->coef[1]  = pfd->coef[20-1-1]  = -130,
  pfd->coef[2]  = pfd->coef[20-1-2]  =  0,
  pfd->coef[3]  = pfd->coef[20-1-3]  =  179,
  pfd->coef[4]  = pfd->coef[20-1-4]  =  389,
  pfd->coef[5]  = pfd->coef[20-1-5]  =  611,
  pfd->coef[6]  = pfd->coef[20-1-6]  =  821,
  pfd->coef[7]  = pfd->coef[20-1-7]  =  1000,
  pfd->coef[8]  = pfd->coef[20-1-8]  =  1130,
  pfd->coef[9]  = pfd->coef[20-1-9]  =  1198,

  memset(pfd->tbuf, 0, sizeof(((s37_filt_data_t*)(0))->tbuf));

  kfsum37HZ = 0;
  for(i=0; i<20; i++)
  {
    kfsum37HZ += pfd->coef[i];
  }

  return (int)(pfd);
}

short s37_filt_stepit(int fd, short w)
{
  s37_filt_data_t * pfd;
  long v;
  int i;

  pfd = (s37_filt_data_t*)fd;
  memmove(&pfd->tbuf[0], &pfd->tbuf[1], sizeof(pfd->tbuf)-sizeof(pfd->tbuf[0]));
  pfd->tbuf[20-1] = w;

  v = 0;
  for (i=0; i<20; i++)
  {
    v += pfd->coef[i]*pfd->tbuf[i];
  }

  return v/kfsum37HZ;
}

void s37_filt_deinit(int fd)
{
  if (!fd) return;
  free((void*)fd);
}
