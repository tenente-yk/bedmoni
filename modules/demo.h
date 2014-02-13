#ifndef __DEMO_H
#define __DEMO_H

#include <sys/types.h>
#include <sys/stat.h>

#define NUM_DEMO_FILES     4

typedef struct
{
  unsigned char fno;
  unsigned char demo;
  unsigned char unused[2];
} demo_cfg_t;

void demo_ini(void);

void demo_deini(void);

void demo_ini_def(void);

void demo_cfg_get(demo_cfg_t * pcfg);

void demo_cfg_set(demo_cfg_t * pcfg);

void demo_record_start(void);

void demo_record_stop(void);

int  demo_is_recording(void);

void demo_record_append(void * pdata, int size);

void demo_record_saveas(int fno);

void demo_record_remove_all(void);

int  demo_record_stat(int fno, struct stat *buf);

int  demo_record_tmp_exist(void);

void demo_cfg_save(void);

extern pid_t demo_video_pid;

#endif // __DEMO_H

