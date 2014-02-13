#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef UNIX
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include "bedmoni.h"
#include "shm.h"

#ifdef SHM_SHM
static int shm_id;
#endif
static int shm_resource_len = 0;
static unsigned char * shm_resource_ptr = 0;

#if defined (WIN32) || defined (SHM_FILE)
#define SHM_FNAME        "./shmdata.dat"
static FILE *fshm = NULL;
#endif

#ifdef UNIX
static pthread_mutex_t shm_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void shm_write(const void *info)
{
#ifdef UNIX
  pthread_mutex_lock(&shm_mutex);
#endif
#if defined (SHM_SHM) || defined (SHM_NONE)
  memcpy(shm_resource_ptr, info, shm_resource_len);
#endif
#ifdef SHM_FILE
  fseek(fshm, 0, SEEK_SET);
  fwrite(info, 1, shm_resource_len, fshm);
#endif
#ifdef UNIX
  pthread_mutex_unlock(&shm_mutex);
#endif
}

void shm_read(void * pdata)
{
#ifdef UNIX
  pthread_mutex_lock(&shm_mutex);
#endif
#if defined (SHM_SHM) || defined (SHM_NONE)
  memcpy(pdata, shm_resource_ptr, shm_resource_len);
#endif
#ifdef SHM_FILE
  fseek(fshm, 0, SEEK_SET);
  fread(pdata, 1, shm_resource_len, fshm);
#endif
#ifdef UNIX
  pthread_mutex_unlock(&shm_mutex);
#endif
}

void shm_rm(int id)
{
#ifdef UNIX
#ifdef SHM_SHM
  if (id < 0) return;
  shmctl(id, IPC_RMID, 0);
  debug("shm deleted\n");
#endif // SHM_SHM
#endif
#if defined (WIN32) || defined (SHM_FILE)
  char s[200];
  if (fshm)
    fclose(fshm);
  sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, SHM_FNAME);
  unlink(s);
#endif
#ifdef SHM_NONE
  free(shm_resource_ptr);
  shm_resource_ptr = 0;
#endif
}

#if 0
void shm_change_mode(int id, char *mode)
{
  struct shmid_ds mds;
  shmctl(id, IPC_STAT, &mds);

  printf("old: %o\n", mds.shm_perm.mode);
  sscanf(mode, "%o", (unsigned int *)&mds.shm_perm.mode);

  shmctl(shmid, IPC_SET, &mds);
  printf("new: %o\n", mds.shm_perm.mode);
}
#endif

int shm_lock(int len)
{
  int exist = 0;
#ifdef UNIX
#ifdef SHM_SHM
  key_t key;

  key = ftok(".", 'B');

  if ((shm_id = shmget(key, len/*sizeof(shm_data)*/, IPC_CREAT|IPC_EXCL|0660)) == -1)
  {
    printf("Segment is exist, connecting...\n");

    if ((shm_id = shmget(key, len/*sizeof(shm_data)*/, 0)) == -1)
    {
      error("shmget error\n");
     // printf("shmget error plya\n");
      strerror(errno);
      fprintf(stderr, "err: %s\n", strerror(errno));
      exit(1);
    }
    exist = 1;
  }
  else
  {
    printf("New segment has been created\n");
    exist = 0;
  }

  if ((shm_resource_ptr = shmat((int)shm_id, (void*)0, 0)) == (void*)(-1))
  {
    perror("shmat");
    exit(1);
  }

  shm_resource_len = len;

  if (ptr0)
  {
    *ptr0 = shm_resource_ptr;
  }
#endif // SHM_SHM
#endif
#if defined (WIN32) || defined (SHM_FILE)
  char s[200];
  sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, SHM_FNAME);
  shm_resource_len = len;
  fshm = fopen(s, "rb");
  exist = (fshm) ? 1 : 0;
  if (exist)
  {
  }
  else
  {
    fshm = fopen(s, "w+b");
  }
  if (!fshm)
  {
    error("unable to create %s\n", s);
  }
#endif
#ifdef SHM_NONE
  if (shm_resource_ptr)
  {
    error("shm already locked\n");
  }
  else
  {
    shm_resource_ptr = (unsigned char *) calloc(1,len);
    shm_resource_len = len;
   // assert(shm_resource_ptr);
    exist = 0;
  }
#endif
  return (exist);
}

void shm_unlock(void)
{
#ifdef SHM_SHM
  shm_rm(shm_id);
#else
  shm_rm(0);
#endif
}
