#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "stconf.h"
#include <sys/types.h>
#include <unistd.h>

#include "elog.h"
#include "pat.h"
#include "networkset.h"
#include "tprn.h"
#include "alarm.h"
#include "demo.h"
#include "lang.h"

#include "bedmoni.h"

static FILE *fc = NULL;

int stconf_init(void)
{
  char s[200];
  int i;
  stconf_hdr_t hdr;

  sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, CONFIG_FNAME);
  if ( access(s, R_OK) != 0)
  {
    FILE *f;
    memset(&hdr, 0, sizeof(hdr));
    hdr.num_id = STCONF_NUM_ID;
    f = fopen(s, "wb");
    if (f)
    {
      // store standard configuration
      for (i=0; i<STCONF_NUM_ID; i++)
      {
        fwrite(&hdr, 1, sizeof(stconf_hdr_t), f);
      }
      fclose(f);
    }
  }
  sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, CONFIG_FNAME);
  fc = fopen(s, "r+b");
  if (fc)
  {
    fchown(fileno(fc), 1000, 1000);

    fseek(fc, 0, SEEK_SET);
    fread(&hdr, 1, sizeof(stconf_hdr_t), fc);

   // debug("hdr.num_id = %d, STCONF_NUM_ID=%d\n", hdr.num_id, STCONF_NUM_ID);
    assert (hdr.num_id == STCONF_NUM_ID);
    fseek(fc, 0, SEEK_SET);
  }
  else
  {
    error("openning cfg file failed\n");
  }

  elog_open();

  pat_ini(NULL);
  network_ini(NULL);
 // alarm_ini(); // have to be performed after unit_init
  tprn_init();
  demo_ini();
  lang_ini();
 // disp_ini(); // have to be performed after crbpio_init

#if 0
  printf("stdonf id: ");
  for (i=0; i<STCONF_NUM_ID; i++)
  {
    fread(&hdr, 1, sizeof(stconf_hdr_t), fc);
    printf("%d ", hdr.id);
  }
  printf("\n");
#endif

  return (int)fc;
}

void stconf_deinit(void)
{
  pat_deini();
  network_deini();
  tprn_deinit();
  demo_deini();
  lang_deini();

  elog_close();

  if (!fc) return;
  fclose(fc);
  fc = 0;
}

int stconf_read(int id, void * buffer, int len)
{
 // int nr = 0;
  int i;
  stconf_hdr_t hdr;
 // unsigned int num_id;

//debug("%s: %d %d\n", __FUNCTION__, id, len);
  if (!fc) return -1;

  assert(buffer);

  fseek(fc, 0, SEEK_SET);
 // fread(&hdr, 1, sizeof(stconf_hdr_t), fc);
 // num_id = hdr.num_id;
  for (i=0; i<STCONF_NUM_ID; i++)
  {
    fread(&hdr, 1, sizeof(stconf_hdr_t), fc);
    if (hdr.id == id) break;
  }

  if (i == STCONF_NUM_ID)
  {
    // not found
    error("%s [line %d] (id %d)\n", __FUNCTION__, __LINE__, id);
    return -1;
  }

  if (len)
  {
    if (hdr.len != len)
    {
      char s[200];
      debug("rewrite stconf\n");
      sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, CONFIG_FNAME);
      unlink(s);
    }
    assert(hdr.len == len);
#if 0
    if (hdr.len != len)
    {
      char s[200];
      debug("rewrite stconf\n");
      sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, CONFIG_FNAME);
      unlink(s);
      stconf_init();
      return -1;
    }
#endif
  }
  else
  {
    len = hdr.len; 
  }
  fseek(fc, hdr.offs, SEEK_SET);

  return fread((unsigned char*)buffer, 1, len, fc);
}

int stconf_write(int id, const void * buffer, int len)
{
  int nw = 0;
  int i;
  stconf_hdr_t hdr;
  unsigned int offs;

  if (!fc) return -1;

  assert(buffer);

//debug("%s: %d %d\n", __FUNCTION__, id, len);

  offs = STCONF_NUM_ID*sizeof(stconf_hdr_t);
  fseek(fc, 0, SEEK_SET);
  for (i=0; i<STCONF_NUM_ID; i++)
  {
    fread(&hdr, 1, sizeof(stconf_hdr_t), fc);
    if (hdr.id == id || hdr.id == STCONF_NONE)
      break;
    offs += hdr.len;
  }

  if (i == STCONF_NUM_ID)
  {
    // not found any empty cell
    error("%s [line %d]\n", __FUNCTION__, __LINE__);
   // debug("id=%d\n", id);
    return -1;
  }

  hdr.id = id;
  hdr.len = len;
  hdr.offs = offs;

  fseek(fc, i*sizeof(stconf_hdr_t), SEEK_SET);
  fwrite(&hdr, 1, sizeof(stconf_hdr_t), fc);

  fseek(fc, offs, SEEK_SET);

  nw = fwrite((unsigned char*)buffer, 1, len, fc);
  fflush(fc);

  if (nw != len)
  {
    error("write config for id %d failed\n", id);
  }

  return nw;
}
