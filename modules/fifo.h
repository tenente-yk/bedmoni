#ifndef __FIFO_H
#define __FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
typedef struct 
{
  unsigned char * beg;
  unsigned char * end;
  int           limit;
  int           size;
  unsigned char * ptr;

  unsigned int  wa;
  unsigned int ra;
} fifo_t;
#pragma pack()
//__attribute__ ((__packed__)

void fifo_init(fifo_t *fifo, unsigned char *buf, int limit);
void fifo_deinit(fifo_t *fifo);

int  fifo_put_byte(fifo_t *fifo, unsigned char c);
int  fifo_put_bytes(fifo_t *fifo, unsigned char * pData, size_t len);
int  fifo_get_byte(fifo_t *fifo, unsigned char *pc);
int  fifo_get_bytes(fifo_t *fifo, unsigned char * pBuf, size_t len);

/* returns size of available data in fifo buffer */
int  fifo_avail(fifo_t *fifo);
/* returns size of free space in fifo buffer */
int  fifo_free(fifo_t *fifo);

/* frees fifo buffer */
int fifo_pop(fifo_t *fifo, size_t len);
/* retrieve data from fifo buffer without ejecting it from the buffer's queue */
int fifo_watch(fifo_t *fifo, unsigned char* pBuf, size_t len);

#ifdef __cplusplus
}
#endif


#endif // __FIFO_H
