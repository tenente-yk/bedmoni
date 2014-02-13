#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "bedmoni.h"
#include "stconf.h"
#include "demo.h"

static demo_cfg_t demo_cfg;
static pthread_mutex_t demo_cfg_mutex = PTHREAD_MUTEX_INITIALIZER;
static int demo_record_in_progress = 0;
static FILE * ftmp = NULL;

pid_t demo_video_pid = 0;

void demo_ini(void)
{
  demo_cfg_t dcfg;

  demo_ini_def();
  demo_cfg_get(&dcfg);

  if ( stconf_read(STCONF_DEMO, &dcfg, sizeof(demo_cfg_t)) > 0 )
  {
    dcfg.demo = 0; // always try to start in device mode
    demo_cfg_set(&dcfg);
  }
}

void demo_deini(void)
{
  char s[200];

  demo_cfg_save();

  sprintf(s, "%s/%s/demodata_tmp.bin", USBDISK_PATH, DEMOFILES_DIR);
  unlink(s);
}

void demo_ini_def(void)
{
  demo_cfg_t dcfg;

  dcfg.fno = 1;
  dcfg.demo = 0;

  demo_cfg_set(&dcfg);
}

void demo_cfg_get(demo_cfg_t * pcfg)
{
  pthread_mutex_lock(&demo_cfg_mutex);
  if (pcfg) *pcfg = demo_cfg;
  pthread_mutex_unlock(&demo_cfg_mutex);
}

void demo_cfg_set(demo_cfg_t * pcfg)
{
  pthread_mutex_lock(&demo_cfg_mutex);
  if (pcfg) memcpy(&demo_cfg, pcfg, sizeof(demo_cfg_t));
  pthread_mutex_unlock(&demo_cfg_mutex);
}

void demo_record_start(void)
{
  char s[200];
  sprintf(s, "%s/%s/demodata_tmp.bin", USBDISK_PATH, DEMOFILES_DIR);
  ftmp = fopen(s, "wb");
  demo_record_in_progress = 1;
}

void demo_record_stop(void)
{
  if (ftmp) fclose(ftmp);
  ftmp = NULL;
  demo_record_in_progress = 0;
}

void demo_record_append(void * pdata, int size)
{
  if (ftmp)
  {
    fwrite(pdata, 1, size, ftmp);
  }
}

int demo_is_recording(void)
{
  return demo_record_in_progress;
}

void demo_record_saveas(int fno)
{
  char s1[200], s2[200];
  if (!demo_record_tmp_exist()) return;
  if (fno < 0)
  {
    sprintf(s1, "%s/%s/demodata_tmp.bin", USBDISK_PATH, DEMOFILES_DIR);
    unlink(s1);
    return;
  }
  sprintf(s1, "%s/%s/demodata_tmp.bin", USBDISK_PATH, DEMOFILES_DIR);
  sprintf(s2, "%s/%s/demodata%d.bin", USBDISK_PATH, DEMOFILES_DIR, fno);
  rename(s1, s2);
}

int  demo_record_stat(int fno, struct stat *buf)
{
  FILE * f;
  char s[200];

  sprintf(s, "%s/%s/demodata%d.bin", USBDISK_PATH, DEMOFILES_DIR, fno);
  f = fopen(s, "rb");
  if (f)
  {
    tzset();
    if (buf) fstat(fileno(f), buf);
    fclose(f);
  }
  return (f > 0);
}

void demo_record_remove_all(void)
{
  char s[200];
  int i;
  for (i=0; i<NUM_DEMO_FILES; i++)
  {
    sprintf(s, "%s/%s/demodata%d.bin", USBDISK_PATH, DEMOFILES_DIR, i+1);
    unlink(s);
  }
}

int  demo_record_tmp_exist(void)
{
  char s[200];
  FILE * f;
  sprintf(s, "%s/%s/demodata_tmp.bin", USBDISK_PATH, DEMOFILES_DIR);
  f = fopen(s, "rb");
  if (f) fclose(f);
  return (f > 0);
}

void demo_cfg_save(void)
{
  demo_cfg_t dcfg;

  demo_cfg_get(&dcfg);
  if ( stconf_write(STCONF_DEMO, &dcfg, sizeof(demo_cfg_t)) > 0 );
}

