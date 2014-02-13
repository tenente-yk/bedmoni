#ifndef __ELOG_H
#define __ELOG_H

#include "utils.h"

#define ELOG_FNAME    "elog.dat"

enum
{
  EL_UPDATE,
  EL_ALARM,
  EL_NUMTYPES,
} elog_type_e;

typedef struct
{
  trts_t        trts;
  int           type;
  unsigned char payload[8];
} elog_data_t;

int elog_open(void);

void elog_close(void);

int elog_read(int offset, int count, elog_data_t * p);

void elog_add(int type, int wp, int lp);

unsigned long elog_getsize(void);

#endif // __ELOG_H

