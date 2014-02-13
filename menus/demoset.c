#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include "bedmoni.h"
#include "grph.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "dio.h"
#include "utils.h"
#include "demo.h"
#include "demoset.h"

static void demoset_none_func(void*);
static void demoset_rb_demo_func(void*);
static void demoset_rb_device_func(void*);
static void demoset_exit_func(void*);
static void demoset_dorec_func(void*);
static void demoset_file1_sel_func(void*);
static void demoset_file2_sel_func(void*);
static void demoset_file3_sel_func(void*);
static void demoset_file4_sel_func(void*);
static void demoset_file_save_func(void*);
static void demoset_files_del_func(void*);
static void demoset_video_func(void*);

inpfoc_funclist_t inpfoc_demoset_funclist[DEMOSET_NUM_ITEMS+1] = 
{
  { DEMOSET_NONE,       demoset_none_func        },
  { DEMOSET_EXIT,       demoset_exit_func        },
  { DEMOSET_RB_DEMO,    demoset_rb_demo_func     },
  { DEMOSET_RB_DEVICE,  demoset_rb_device_func   },
  { DEMOSET_DOREC,      demoset_dorec_func       },
  { DEMOSET_FILE1_SEL,  demoset_file1_sel_func   },
  { DEMOSET_FILE2_SEL,  demoset_file2_sel_func   },
  { DEMOSET_FILE3_SEL,  demoset_file3_sel_func   },
  { DEMOSET_FILE4_SEL,  demoset_file4_sel_func   },
  { DEMOSET_FILE_SAVE,  demoset_file_save_func   },
  { DEMOSET_FILES_DEL,  demoset_files_del_func   },
  { DEMOSET_VIDEO,      demoset_video_func       },
  { -1       ,          demoset_none_func        }, // -1 must be last
};

static void demoset_none_func(void * parg)
{

}

static void demoset_rb_demo_func(void * parg)
{
  demo_cfg_t dcfg;

  demo_cfg_get(&dcfg);

  dio_suspend();
  dio_deinit();

  demo_mode = 1;
  dcfg.demo = 1;
  demo_cfg_set(&dcfg);

  dio_init();
  dio_resume();

  inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, demo_mode ? DEMOSET_RB_DEMO : DEMOSET_RB_DEVICE), IWC_CHECKED);

  inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, !demo_mode ? DEMOSET_RB_DEMO : DEMOSET_RB_DEVICE), IWC_UNCHECKED);
}

static void demoset_rb_device_func(void * parg)
{
  demo_cfg_t dcfg;

  demo_cfg_get(&dcfg);

  dio_suspend();
  dio_deinit();

  demo_mode = 0;
  dcfg.demo = 0;
  demo_cfg_set(&dcfg);

  dio_init();
  dio_resume();

  inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, demo_mode ? DEMOSET_RB_DEMO : DEMOSET_RB_DEVICE), IWC_CHECKED);

  inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, !demo_mode ? DEMOSET_RB_DEMO : DEMOSET_RB_DEVICE), IWC_UNCHECKED);

  ecgm_command(demo_mode ? ECGM_DEMO_ON : ECGM_DEMO_OFF);
}

static void demoset_dorec_func(void * parg)
{
  char s[128];
  if (demo_is_recording())
  {
    demo_record_stop();
    ids2string(IDS_RECORD, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_DOREC), s);
   // ids2string(IDS_SAVE, s);
   // inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE_SAVE), s);
    inpfoc_wnd_setids(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE_SAVE), IDS_SAVE);
    inpfoc_enable(INPFOC_DEMOSET, DEMOSET_FILE_SAVE);
  }
  else
  {
    demo_record_start();
    ids2string(IDS_STOP, s);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_DOREC), s);
   // inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE_SAVE), ".......");
  }
}

static void demoset_file1_sel_func(void * parg)
{
  int i;
  demo_cfg_t dcfg;

  demo_cfg_get(&dcfg);
  for (i=0; i<NUM_DEMO_FILES; i++)
  {
    inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL + i), (DEMOSET_FILE1_SEL + i != DEMOSET_FILE1_SEL) ? IWC_UNCHECKED : IWC_CHECKED);
  }
  dcfg.fno = 1;
  demo_cfg_set(&dcfg);

  if (demo_mode)
  {
    dio_suspend();
    dio_deinit();
    dio_init();
    dio_resume();
  }
}

static void demoset_file2_sel_func(void * parg)
{
  int i;
  demo_cfg_t dcfg;

  demo_cfg_get(&dcfg);
  for (i=0; i<NUM_DEMO_FILES; i++)
  {
    inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL + i), (DEMOSET_FILE1_SEL + i != DEMOSET_FILE2_SEL) ? IWC_UNCHECKED : IWC_CHECKED);
  }
  dcfg.fno = 2;
  demo_cfg_set(&dcfg);

  if (demo_mode)
  {
    dio_suspend();
    dio_deinit();
    dio_init();
    dio_resume();
  }
}

static void demoset_file3_sel_func(void * parg)
{
  int i;
  demo_cfg_t dcfg;

  demo_cfg_get(&dcfg);
  for (i=0; i<NUM_DEMO_FILES; i++)
  {
    inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL + i), (DEMOSET_FILE1_SEL + i != DEMOSET_FILE3_SEL) ? IWC_UNCHECKED : IWC_CHECKED);
  }
  dcfg.fno = 3;
  demo_cfg_set(&dcfg);

  if (demo_mode)
  {
    dio_suspend();
    dio_deinit();
    dio_init();
    dio_resume();
  }
}

static void demoset_file4_sel_func(void * parg)
{
  int i;
  demo_cfg_t dcfg;

  demo_cfg_get(&dcfg);
  for (i=0; i<NUM_DEMO_FILES; i++)
  {
    inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL + i), (DEMOSET_FILE1_SEL + i != DEMOSET_FILE4_SEL) ? IWC_UNCHECKED : IWC_CHECKED);
  }
  dcfg.fno = 4;
  demo_cfg_set(&dcfg);

  if (demo_mode)
  {
    dio_suspend();
    dio_deinit();
    dio_init();
    dio_resume();
  }
}

static void demoset_file_save_func(void * parg)
{
  demo_cfg_t dcfg;
  char s[40];
  struct tm ltm, *ptm;
  struct stat st;

  demo_cfg_get(&dcfg);

  demo_record_stop();
  demo_record_saveas(dcfg.fno);
  read_hwclock_data(&ltm);
  demo_record_stat(dcfg.fno, &st);
  ptm = gmtime(&st.st_mtime);

  sprintf(s, "%02d.%02d.%04d %02d:%02d", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, (int)(ptm->tm_hour+ltm.tm_gmtoff/3600)%24, ptm->tm_min);

  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL+(dcfg.fno-1)), s);

 // inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE_SAVE), "-------");
  inpfoc_set(INPFOC_DEMOSET, DEMOSET_EXIT);

  inpfoc_disable(INPFOC_DEMOSET, DEMOSET_FILE_SAVE);
}

static void demoset_files_del_func(void * parg)
{
  int i;

  if (demo_mode)
  {
    dio_suspend();
    dio_deinit();
  }

  demo_record_remove_all();

  for (i=0; i<NUM_DEMO_FILES; i++)
  {
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL+i), " ");
  }

  if (demo_mode)
  {
    dio_init();
    dio_resume();
  }
}

static void demoset_video_func(void * parg)
{
#ifdef ARM
  if ( access("/mnt/usbdisk/extra/mplayer", X_OK) != 0)
  {
    error("%s not found\n", "/mnt/usbdisk/extra/mplayer");
    return;
  }
  else
  if ( access("/mnt/usbdisk/extra/video.avi", R_OK) != 0)
  {
    error("%s not found\n", "/mnt/usbdisk/extra/video.avi");
    return;
  }

  // disable all framebuffer output
  dview_set_state(0);

  // launch demo video
  char s[200];
  sprintf(s, "/mnt/usbdisk/extra/mplayer /mnt/usbdisk/extra/video.avi -fs -loop 0");
#if 0
  system(s);
#else
  int pid1;
  pid1 = fork();
  if (pid1 < 0)
  {
    error("fork for mplayer launch: unable to fork\n");
    system(s);
    return;
  }
  if (pid1 == 0)
  {
    execl("/mnt/usbdisk/extra/mplayer", "/mnt/usbdisk/extra/mplayer", "/mnt/usbdisk/extra/video.avi", "-fs", /*"-nosound",*/ "-loop", "0",  NULL);
    exit(0);
  }

  demo_video_pid = pid1;
#endif

#endif
}

static void demoset_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
  demo_cfg_save();
}

void menu_demoset_openproc(void)
{
  char s[200];
  demo_cfg_t dcfg;
  int i;
  struct stat st;
  struct tm * ptm, ltm;

  inpfoc_set(INPFOC_DEMOSET, DEMOSET_EXIT);

  inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, demo_mode ? DEMOSET_RB_DEMO : DEMOSET_RB_DEVICE), IWC_CHECKED);

  read_hwclock_data(&ltm);

  demo_cfg_get(&dcfg);
  for (i=0; i<NUM_DEMO_FILES; i++)
  {
    if (demo_record_stat(i+1, &st))
    {
      ptm = gmtime(&st.st_mtime);
      sprintf(s, "%02d.%02d.%04d %02d:%02d", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, (int)(ptm->tm_hour+ltm.tm_gmtoff/3600)%24, ptm->tm_min);
      inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL + i), s);
    }
    else
    {
      inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL + i), "");
    }
  }
  if (dcfg.fno > NUM_DEMO_FILES || dcfg.fno < 1)
  {
    error("dcfg.fno = %d (invalid), setting to 1\n", dcfg.fno);
    dcfg.fno = 1;
    demo_cfg_set(&dcfg);
  }
  inpfoc_wnd_check(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE1_SEL + (dcfg.fno - 1)), IWC_CHECKED);

 // inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_FILE_SAVE), "-------");
  ids2string(IDS_RECORD, s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DEMOSET, DEMOSET_DOREC), s);

  inpfoc_disable(INPFOC_DEMOSET, DEMOSET_FILE_SAVE);
}

void menu_demoset_closeproc(void)
{
  if (demo_is_recording())
  {
    demo_record_stop();
    demo_record_saveas(-1); // remove temporary file
  }
}

