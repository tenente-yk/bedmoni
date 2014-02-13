#include <string.h>
/*
#ifdef UNIX
#include <stdlib.h>
#include <pthread.h>
static pthread_mutex_t fifo_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#ifdef WIN32
#include <windows.h>
static CRITICAL_SECTION cs;
#endif
*/
#include "fifo.h"

#ifndef MIN
#define MIN(a,b) (a<b)?a:b
#endif

/*****************************************************************************/
void fifo_init(fifo_t *fifo, unsigned char *buf, int limit)
{
  fifo->beg   = buf;
  fifo->end   = buf + limit;
  fifo->limit = limit; //fifo->end - fifo->beg + 1;
  fifo->size  = 0;
  fifo->ptr   = fifo->beg;

  fifo->wa = fifo->ra = 0;

  memset(buf, 0, limit);
/*
#ifdef WIN32
  InitializeCriticalSection(&cs);
#endif
*/
}
/*****************************************************************************/
void fifo_deinit(fifo_t *fifo)
{
  // TODO: fifo_deinit {DeleteCriticalSection..}
}
/*****************************************************************************/
int fifo_put_bytes(fifo_t *fifo, unsigned char* pData, size_t len)
{
/*
#ifdef UNIX
  pthread_mutex_lock(&fifo_mutex);
#endif
#ifdef WIN32
  EnterCriticalSection(&cs);
#endif
*/
  if (fifo->wa) return -1;
  fifo->wa = 1;

  if ((int)len < (int)(fifo->end-fifo->ptr))
  {
    memcpy(fifo->ptr, pData, len);
    fifo->ptr += len;
  }
  else
  {
    memcpy(fifo->ptr, pData, fifo->end-fifo->ptr);
    memcpy(fifo->beg, pData+(fifo->end-fifo->ptr), len-(fifo->end-fifo->ptr));
    fifo->ptr = fifo->beg + len-(fifo->end-fifo->ptr);
  }

  fifo->size += len;
  fifo->size = (fifo->size > fifo->limit) ? fifo->limit : fifo->size;

  fifo->wa = 0;
/*
#ifdef UNIX
  pthread_mutex_unlock(&fifo_mutex);
#endif
#ifdef WIN32
  LeaveCriticalSection(&cs);
#endif
*/
  return( len );
}
/*****************************************************************************/
int fifo_get_bytes(fifo_t *fifo, unsigned char* pBuf, size_t len)
{
  if (fifo->size == 0)
  {
    return 0;
  }
/*
#ifdef UNIX
  pthread_mutex_lock(&fifo_mutex);
#endif
#ifdef WIN32
  EnterCriticalSection(&cs);
#endif
*/
  if (fifo->ra) return -1;
  fifo->ra = 1;

  if ((int)len > fifo->size)
    len = fifo->size;

  if (fifo->beg + fifo->size < fifo->ptr)
  {
    memcpy(pBuf, fifo->ptr-fifo->size, len);
  }
  else
  {
    memcpy(pBuf, fifo->ptr+fifo->limit-fifo->size, MIN((int)len, fifo->end-(fifo->ptr+fifo->limit-fifo->size)));
    if ((int)len > fifo->end-(fifo->ptr+fifo->limit-fifo->size))
    {
      pBuf += fifo->end-(fifo->ptr+fifo->limit-fifo->size);
      memcpy(pBuf, fifo->beg, len-(fifo->end-(fifo->ptr+fifo->limit-fifo->size)) );
    }
  }

  fifo->size -= len;

  fifo->ra = 0;
/*
#ifdef UNIX
  pthread_mutex_unlock(&fifo_mutex);
#endif
#ifdef WIN32
  LeaveCriticalSection(&cs);
#endif
*/
  return len;
}
/*****************************************************************************/
int fifo_get_byte(fifo_t *fifo, unsigned char *pc)
{
  return fifo_get_bytes(fifo, pc, 1);
}
/*****************************************************************************/
int fifo_put_byte(fifo_t *fifo, unsigned char c)
{
  return fifo_put_bytes(fifo, &c, 1);
}
/*****************************************************************************/
int fifo_avail(fifo_t *fifo)
{
  return (fifo->size);
}
/*****************************************************************************/
int fifo_free(fifo_t *fifo)
{
  return (fifo->limit - fifo->size);
}
/*****************************************************************************/
int fifo_pop(fifo_t *fifo, size_t len)
{
/*
#ifdef UNIX
  pthread_mutex_lock(&fifo_mutex);
#endif
#ifdef WIN32
  EnterCriticalSection(&cs);
#endif
*/
  if (fifo->ra) return -1;
  fifo->ra = 1;

  if ((int)len > fifo->size)
    len = fifo->size;

  fifo->size -= len;

/*
#ifdef UNIX
  pthread_mutex_unlock(&fifo_mutex);
#endif
#ifdef WIN32
  LeaveCriticalSection(&cs);
#endif
*/
  fifo->ra = 0;

  return len;
}
/*****************************************************************************/
int fifo_watch(fifo_t *fifo, unsigned char* pBuf, size_t len)
{
/*
#ifdef UNIX
  pthread_mutex_lock(&fifo_mutex);
#endif
#ifdef WIN32
  EnterCriticalSection(&cs);
#endif
*/
  if (fifo->ra) return -1;
  fifo->ra = 1;

  if ((int)len > fifo->size)
    len = fifo->size;

  if (fifo->beg + fifo->size < fifo->ptr)
  {
    memcpy(pBuf, fifo->ptr-fifo->size, len);
  }
  else
  {
    memcpy(pBuf, fifo->ptr+fifo->limit-fifo->size, fifo->end-(fifo->ptr+fifo->limit-fifo->size));
    if ((int)len > fifo->end-(fifo->ptr+fifo->limit-fifo->size))
    {
      pBuf += fifo->end-(fifo->ptr+fifo->limit-fifo->size);
      memcpy(pBuf, fifo->beg, len-(fifo->end-(fifo->ptr+fifo->limit-fifo->size)) );
    }
  }

  fifo->ra = 0;

/*
#ifdef UNIX
  pthread_mutex_unlock(&fifo_mutex);
#endif
#ifdef WIN32
  LeaveCriticalSection(&cs);
#endif
*/
  return len;
}
/*****************************************************************************/
