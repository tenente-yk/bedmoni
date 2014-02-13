
#ifdef UNIX
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <assert.h>
#include "bedmoni.h"
#include "menucs.h"
#include "uframe.h"
#include "utils.h"
#include "tr.h"

#define TR_ALLOC_SIZE        32768

static FILE * fh = 0;
static FILE * f = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned int rp = 0, wp = 0;
static int flen = 0;
static int beg = 0;
static int trh_start_offset = 0;

static struct
{
  unsigned char * data;
  unsigned long size;
} ram_tr;

int tr_open(void)
{
  char fname[150];
  char fpath[150];
  char s[200];
  char stmp[200];
  int res;
  struct stat st;

  res = 1;

  snprintf(fpath, sizeof(fpath), "%s/%s", USBDISK_PATH, DATA_DIR);
  snprintf(fname, sizeof(fname), "%s", TR_DATA_FNAME);
  snprintf(s, sizeof(s), "%s/%s", fpath, fname);
  stat(s, &st);
  if (st.st_size > 10*24*60*sizeof(tr_t))
  {
    FILE * ftmp;
    // resize file
    strcpy(stmp, s);
    strcat(stmp, ".tmp");
    ftmp = fopen(stmp, "wb");
    f = fopen(s, "rb");
    if (f)
    {
      int nr;
      char buf[200];
      fseek(f, st.st_size-7*24*60*sizeof(tr_t), SEEK_SET);
      while (!feof(f))
      {
        nr = fread(buf, 1, sizeof(buf), f);
        if (nr > 0)
          fwrite(buf, 1, nr, ftmp);
      }
      fclose(f);
      fclose(ftmp);
     // unlink(s);
      rename(stmp, s);
    }
    else
    {
      error("unable to resize %s\n", s);
    }
  }
  f = fopen(s, "r+");
  if (!f)
  {
    f = fopen(s, "w+");
  }
  if (!f)
  {
    fh = NULL;
  }

  snprintf(fname, sizeof(fname), "%s", TR_HEAD_FNAME);
  snprintf(s, sizeof(s), "%s/%s", fpath, fname);
  stat(s, &st);
  if (st.st_size > 10*24*60*sizeof(trh_t))
  {
    FILE * ftmp;
    // resize file
    strcpy(stmp, s);
    strcat(stmp, ".tmp");
    ftmp = fopen(stmp, "wb");
    f = fopen(s, "rb");
    if (f)
    {
      int nr;
      char buf[200];
      fseek(f, st.st_size-7*24*60*sizeof(trh_t), SEEK_SET);
      while (!feof(f))
      {
        nr = fread(buf, 1, sizeof(buf), f);
        if (nr > 0)
          fwrite(buf, 1, nr, ftmp);
      }
      fclose(f);
      fclose(ftmp);
     // unlink(s);
      rename(stmp, s);
    }
    else
    {
      error("unable to resize %s\n", s);
    }
  }
  fh = fopen(s, "r+");
  if (!fh)
  {
    fh = fopen(s, "w+");
  }
  if (!fh)
  {
//    return -1;
  }

  ram_tr.data = NULL;
  ram_tr.size = 0;

  if (!f || !fh)
  {
    ram_tr.data = calloc(1, TR_ALLOC_SIZE);
    assert(ram_tr.data);
    ram_tr.size += TR_ALLOC_SIZE;
    flen = 0;
    res = 0;
  }

  if (f)
  {
    fseek(f, 0, SEEK_END);
    flen = ftell(f);
  }
  wp = flen;
  rp = 0;
  beg = 1;

  trh_start_offset = 0;
  if (fh)
  {
    trh_t ltrh;
    int ntrh = trh_size();
    int i;
    trts_t trts_curr;
    time_t tt;
    struct tm * ptm;

    time(&tt);
    ptm = localtime(&tt);
    memset(&trts_curr, 0, sizeof(trts_t));
    trts_curr.hour = ptm->tm_hour;
    trts_curr.min = ptm->tm_min;
    trts_curr.day = ptm->tm_mday;
    trts_curr.mon = ptm->tm_mon+1;
    trts_curr.year = ptm->tm_year;

    trh_start_offset = 0;
    for (i=ntrh-1; i>=0; i--)
    {
      trh_read(i, &ltrh);
      if (ltrh.offs1 == ltrh.offs0) continue;
//printf("%d %d %d.%d %02d:%02d   %d\n", i, trtscmp(&trts_curr, &ltrh.trts1), ltrh.trts1.day,ltrh.trts1.mon, ltrh.trts1.hour,ltrh.trts1.min, ltrh.offs1);
      if (trtscmp(&trts_curr, &ltrh.trts1) > 7*24*60)
      {
        break;
      }
    }
    if (i>=0) trh_start_offset = i;
//printf("%d start offs = %d , %d\n", i, ltrh.offs1, trh_start_offset);
  }

#ifdef TR_DO_NOT_APPEND
  debug("TR_DO_NOT_APPEND enabled\n");
#endif

  return res;
}

void tr_close(void)
{
  time_t tt;
  struct tm * ptm;
  trts_t trts;
  unsigned long v;

  if (f && fh)
  {
    int nr;
    fseek(fh, 0, SEEK_END);
    fseek(fh, -3*sizeof(int), SEEK_CUR);
    v = 0;
    nr = fread(&v, 1, sizeof(int), fh);
    if (nr != sizeof(int))
    {
      error("fread error %s\n", __FUNCTION__);
    }
    if (v == 0x12345678)
    {
      fseek(fh, 0, SEEK_END);
      time(&tt);
      ptm = localtime(&tt);
      memset(&trts, 0, sizeof(trts_t));
      trts.hour = ptm->tm_hour;
      trts.min = ptm->tm_min;
      trts.day = ptm->tm_mday;
      trts.mon = ptm->tm_mon+1;
      trts.year = ptm->tm_year;
      fwrite(&trts, 1, sizeof(trts_t), fh);
      fseek(f, 0, SEEK_END);
      v = ftell(f);
      fwrite(&v, 1, sizeof(v), fh);
    }

    fclose(f);
    fclose(fh);
  }
  else
  {
    if (ram_tr.data) free(ram_tr.data);
    ram_tr.data = NULL;
    ram_tr.size = 0;
  }

  f = 0;
  fh = 0;
  beg = 0;
}

void tr_reset(void)
{
  char fname[150];
  char fpath[150];
  char s[200];

  pthread_mutex_lock(&mutex);

  tr_close();

  snprintf(fpath, sizeof(fpath), "%s/%s", USBDISK_PATH, DATA_DIR);
  snprintf(fname, sizeof(fname), "%s", TR_DATA_FNAME);
  snprintf(s, sizeof(s), "%s/%s", fpath, fname);
  unlink(s);
  snprintf(fpath, sizeof(fpath), "%s/%s", USBDISK_PATH, DATA_DIR);
  snprintf(fname, sizeof(fname), "%s", TR_HEAD_FNAME);
  snprintf(s, sizeof(s), "%s/%s", fpath, fname);
  unlink(s);

  tr_open();

  pthread_mutex_unlock(&mutex);
}

int tr_append(tr_t * ptr)
{
#if defined (TR_DO_NOT_APPEND)
  return 1;
#endif

  if (!f || !fh)
  {
    pthread_mutex_lock(&mutex);

    memcpy(ram_tr.data+wp, ptr, sizeof(tr_t));
    wp += sizeof(tr_t);
    flen = wp;
    if (wp + sizeof(tr_t) >= ram_tr.size)
    {
      int shift = 10*sizeof(tr_t);
      assert(shift < ram_tr.size);
      memmove(ram_tr.data, ram_tr.data+shift, ram_tr.size-shift);
      wp -= shift;
    }

    pthread_mutex_unlock(&mutex);

    return 1;
  }

  pthread_mutex_lock(&mutex);

  fseek(f, wp, SEEK_SET);

  if (beg)
  {
    time_t tt;
    struct tm * ptm;
    trts_t trts;
    unsigned long v;

    fseek(fh, 0, SEEK_END);
    time(&tt);
    ptm = localtime(&tt);
    memset(&trts, 0, sizeof(trts_t));
    trts.hour = ptm->tm_hour;
    trts.min = ptm->tm_min;
    trts.day = ptm->tm_mday;
    trts.mon = ptm->tm_mon+1;
    trts.year = ptm->tm_year;
    fseek(f, 0, SEEK_END);
    v = ftell(fh);
    // finish earlier sessions' fields
    if (v % (5*sizeof(int)) != 0)
    {
      tr_t tr;
      fwrite(&trts, 1, sizeof(trts_t), fh);
      fseek(f, 0, SEEK_END);
      fseek(f, -1*sizeof(tr_t), SEEK_CUR);
      if (fread(&tr, 1, sizeof(tr_t), f) == sizeof(tr_t))
      {
        memcpy(&trts, ((trts_t*)(&tr.v[PARAM_TS])), sizeof(trts_t));
      }
      else
      {
        error("%s: unable to finalize previous session\n", __FUNCTION__);
      }
      fwrite(&trts, 1, sizeof(trts_t), fh);
      fseek(f, 0, SEEK_END);
      v = ftell(f);
      fwrite(&v, 1, sizeof(v), fh);  // trts1 and offs1 for prev session
    }
    v = 0x12345678;
    fwrite(&v, 1, sizeof(v), fh);           // magic
    fwrite(&trts, 1, sizeof(trts_t), fh);   // trts0
    fseek(f, 0, SEEK_END);
    v = ftell(f);
    fwrite(&v, 1, sizeof(v), fh);           // offs0

    ((trts_t*)(&ptr->v[PARAM_TS]))->beg = 1;
    beg = 0;
  }

#ifndef TR_DO_NOT_APPEND
  fwrite(ptr, 1, sizeof(tr_t), f);
  fflush(f);
  wp = ftell(f);
  flen += sizeof(tr_t);
#endif

  pthread_mutex_unlock(&mutex);

  beg = 0;
  return 1;
}

int tr_read(tr_t * ptr, int count, int offset, int read_all_tr)
{
  int nr, n, r, na;

  if (!f)
  {
    pthread_mutex_lock(&mutex);

    count = min(count, flen/sizeof(tr_t)-offset);
    if (count < 0) count = 0;
    if (rp+offset*sizeof(tr_t) < 0) count = 0;
    if (offset*sizeof(tr_t)+rp > flen) count = 0;
    memcpy(ptr, ram_tr.data+rp+offset*sizeof(tr_t), count*sizeof(tr_t));

    pthread_mutex_unlock(&mutex);

    return count;
  }

  pthread_mutex_lock(&mutex);

  trh_t ltrh;
  trh_read(0, &ltrh);
//printf("%d   ltrh.offs1 = %d %d.%d\n", rp+offset*sizeof(tr_t), ltrh.offs1, ltrh.trts0.day, ltrh.trts0.mon);
  if ( rp+offset*sizeof(tr_t) < ltrh.offs1 && (trh_size() > 1) && !read_all_tr )
  {
    count = 0;
  }

  n = sizeof(tr_t);
  fseek(f, rp+offset*n, SEEK_SET);

  na = 0;
  nr = 0;
  r = 0;
  while (nr != n && count > 0)
  {
    r = fread((unsigned char*)ptr+nr, 1, n-nr, f);
    if (r <= 0) break;
    nr += r;
    if (nr == n)
    {
      na += nr;
      count --;
      ptr ++;
      nr = r = 0;
    }
  }
  pthread_mutex_unlock(&mutex);

  return na;
}

int tr_seek(long offset, int mode)
{
  int ok = 1;
  int ret = 0;

  pthread_mutex_lock(&mutex);

  if (mode == SEEK_CUR)
  {
    if ((signed int)(rp+offset*sizeof(tr_t)) > flen) ok = 0;
    if ((signed int)(rp+offset*sizeof(tr_t)) < 0)
    {
      offset = -( (signed int)rp/sizeof(tr_t));
      ret = -1;
    }
  }
  else
  if (mode == SEEK_SET)
  {
    if ((signed int)(offset*sizeof(tr_t)) > flen) ok = 0;
  }

  if (ok)
  {
    if (f)
    {
      if (mode == SEEK_CUR)
        fseek(f, rp, SEEK_SET);
      fseek(f, offset*sizeof(tr_t), mode);
      rp = ftell(f);
    }
    else
    {
      if (mode == SEEK_CUR)
        rp += offset*sizeof(tr_t);
      if (mode == SEEK_SET)
        rp = offset*sizeof(tr_t);
      if (mode == SEEK_END)
        rp = flen;
    }
  }
  else
    ret = -1;

  pthread_mutex_unlock(&mutex);

  return ret;
}

void tr_seek_end(unsigned int tsc)
{
  pthread_mutex_lock(&mutex);

  if (f)
  {
    fseek(f, 0, SEEK_END);
    if (ftell(f) > tsc * sizeof(tr_t))
      fseek(f, (-tsc+0)*sizeof(tr_t), SEEK_CUR);
    else
      fseek(f, 0, SEEK_SET);
    rp = ftell(f);
  }
  else
  {
    if (flen > tsc * sizeof(tr_t))
      rp = flen - tsc * sizeof(tr_t);
    else
      rp = 0;
  }

  pthread_mutex_unlock(&mutex);
}

long tr_get_size(void)
{
  long size;

  pthread_mutex_lock(&mutex);

//  fseek(f, 0, SEEK_END);
//  size = ftell(f);
  size = (long)flen;

  pthread_mutex_unlock(&mutex);

  return size / sizeof(tr_t);
}

int  trh_read(int offset, trh_t * ptrh)
{
  if (!ptrh) return -1;
  if (!fh) return -1;

//printf("trh_start_offset+offset = %d (%d)\n", trh_start_offset+offset, trh_start_offset);
  fseek(fh, (trh_start_offset+offset)*sizeof(trh_t), SEEK_SET);

  return fread(ptrh, 1, sizeof(trh_t), fh);
}

int  trh_size(void)
{
  if (!fh) return 0;
  fseek(fh, 0, SEEK_END);
  return (ftell(fh) / sizeof(trh_t) - trh_start_offset);
}
