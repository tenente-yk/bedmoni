
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "bedmoni.h"
#include "utils.h"
#include "eframe.h"
#include "elog.h"

#define ELOG_ALLOC_SIZE        1024

static FILE *flog = NULL;

static struct
{
  unsigned char * data;
  unsigned long ptr;
  unsigned long size;
} ram_elog;

int elog_open(void)
{
  struct stat st;
  char s[200], stmp[200];
  sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, ELOG_FNAME);

  stat(s, &st);
  if (st.st_size > 200*sizeof(elog_data_t))
  {
    FILE *f, *ftmp;
    // resize file
    strcpy(stmp, s);
    strcat(stmp, ".tmp");
    ftmp = fopen(stmp, "wb");
    f = fopen(s, "rb");
    if (f)
    {
      int nr;
      char buf[200];
      fseek(f, st.st_size-100*sizeof(elog_data_t), SEEK_SET);
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

  flog = fopen(s, "r+b");
  if (!flog)
  {
    flog = fopen(s, "w+b");
  }

  ram_elog.data = NULL;
  ram_elog.size = 0;
  if (!flog)
  {
    ram_elog.data = calloc(1, ELOG_ALLOC_SIZE);
    assert(ram_elog.data);
    ram_elog.size = ELOG_ALLOC_SIZE;
    ram_elog.ptr = 0;
  }

  return 1;
}

void elog_close(void)
{
  if (flog>0) fclose(flog);
  flog = NULL;
  if (ram_elog.data) free(ram_elog.data);
  ram_elog.data = NULL;
  ram_elog.size = 0;
  ram_elog.ptr = 0;
}

int elog_read(int offset, int count, elog_data_t * p)
{
  int i, packet_size, nn;
  if (!p) return -1;

  if (flog)
  {
    fseek(flog, 0, SEEK_END);
    nn = ftell(flog);
    if (!nn || nn <= offset*sizeof(elog_data_t)) return 0;
    packet_size = sizeof(elog_data_t);
    fseek(flog, -(offset+1)*packet_size, SEEK_CUR);
    for (i=0; i<count; i++)
    {
      if (packet_size != fread(p, 1, packet_size, flog)) break;
      p++;
      fseek(flog, -1*packet_size, SEEK_CUR);
    }
    return i;
  }
  else
  {
    int ptr=ram_elog.ptr-(offset+1)*sizeof(elog_data_t);
    if (ptr < 0) return 0;
    for (i=0; i<count && ptr>=0; i++, ptr-=sizeof(elog_data_t), p++)
    {
      memcpy(p, ram_elog.data+ptr, sizeof(elog_data_t));
    }
    return i;
  }
}

void elog_add(int type, int wp, int lp)
{
  elog_data_t eld;
  struct tm ltm;

  read_hwclock_data(&ltm);
  eld.trts.hour = ltm.tm_hour;
  eld.trts.min = ltm.tm_min;
  eld.trts.day = ltm.tm_mday;
  eld.trts.mon = ltm.tm_mon+1;
  eld.trts.year = ltm.tm_year;
  eld.type = type;
  eld.payload[0] = wp;
  eld.payload[1] = lp;

  if (flog)
  {
    fseek(flog, 0, SEEK_END);
    fwrite(&eld, 1, sizeof(elog_data_t), flog);
    fflush(flog);
  }
  else
  {
    memcpy(ram_elog.data+ram_elog.ptr, &eld, sizeof(elog_data_t));
    ram_elog.ptr += sizeof(elog_data_t);
    if (ram_elog.ptr + sizeof(elog_data_t) >= ram_elog.size)
    {
      int shift = 10*sizeof(elog_data_t);
      assert(shift < ram_elog.size);
      memmove(ram_elog.data, ram_elog.data+shift, ram_elog.size-shift);
      ram_elog.ptr -= shift;
    }
  }

  eframe_command(EFRAME_NEWDATA, NULL);
}

unsigned long elog_getsize(void)
{
  if (flog)
  {
    fseek(flog, 0, SEEK_END);
    return (ftell(flog) / sizeof(elog_data_t));
  }
  else
  {
    return (ram_elog.ptr / sizeof(elog_data_t));
  }
}
