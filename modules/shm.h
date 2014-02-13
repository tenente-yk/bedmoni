#ifndef __SHM_H
#define __SHM_H

#include "dview.h"
#include "tprn.h"

#define SHM_NONE
//#define SHM_FILE
//#define SHM_SHM

#if defined (WIN32) && defined (SHM_SHM)
#undef  SHM_SHM
#define SHM_FILE
#endif

#if defined (SHM_NONE) && defined (SHM_FILE)
#error check SHM_ defines
#endif

#if defined (SHM_NONE) && defined (SHM_SHM)
#error check SHM_ defines
#endif

#if defined (SHM_FILE) && defined (SHM_SHM)
#error check SHM_ defines
#endif

typedef struct
{
  unsigned int old_tick[NUM_VIEWS];
//  unsigned int old_tick_prn_c[TPRN_NUM_CURVES];
  double dview_data_ptr[NUM_VIEWS];
} shm_data_t;

int  shm_lock(int len);

void shm_unlock(void);

void shm_write(const void *ptr); //, char *info, int len);

void shm_read(void * pdata); //, int len, char *ptr);

void shm_rm(int id);

#if 0
void shm_change_mode(int id, char *mode);
#endif

#endif // __SHM_H
