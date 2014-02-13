#ifndef __TR_H
#define __TR_H

#include "dview.h"
#include "unit.h"
#include "utils.h"

//#define TR_DO_NOT_APPEND

#define TR_ST_U         0x01
#define TR_ST_L         0x02
#define TR_UNDEF_VALUE  0x0BBBBBBB

#define TR_DATA_FNAME   "trd.dat"
#define TR_HEAD_FNAME   "trh.dat"

#pragma pack(1)

typedef struct
{
  unsigned int ss  : 10;
  unsigned int dd  : 10;
  unsigned int mm  : 10;
  unsigned int unused  : 2;
} nibpv_t;

typedef struct
{
  int            v[NUM_PARAMS];
  int            vl[NUM_PARAMS];
  int            vu[NUM_PARAMS];
  unsigned char  st[NUM_PARAMS];
}tr_t;

typedef struct
{
  int          magic;
  trts_t       trts0;
  int          offs0;
  trts_t       trts1;
  int          offs1;
} trh_t;

#pragma pack()

int  tr_open(void);

void tr_close(void);

void tr_reset(void);

int  tr_append(tr_t * ptr);

int  tr_read(tr_t * ptr, int count, int offset, int read_all_tr);

int  tr_seek(long offset, int mode);

void tr_seek_end(unsigned int tsc);

long tr_get_size(void);

int  trh_read(int offset, trh_t * ptrh);

int  trh_size(void);

#endif // __TR_H
