#ifndef __SFRAME_H
#define __SFRAME_H

#include "dview.h"

#define SFRAME_CX          244
#define SFRAME_CY          20
#define SFRAME_X0          (DISPLAY_CX-SFRAME_CX-1)
#ifdef UNIX
#define SFRAME_Y0          460
#endif
#ifdef WIN32
#define SFRAME_Y0          425 // podgon
#endif

#define SFRAME_BKCOLOR     RGB(0x00,0x00,0x00)
#define SFRAME_FGCOLOR     RGB(0xff,0xff,0xff)
#define SFRAME_FONT        ARIAL_12

#define SFRAME_MAX_NUM_CMDS_IN_QUEUE   10

// sframe commands range 0x90-0x9f
#define SFRAME_RELOAD          0x90
#define SFRAME_SET_DATE        0x91
#define SFRAME_SET_TIME        0x92
#define SFRAME_SET_BATSTAT     0x93
#define SFRAME_TOGGLE_VERMODE  0x95

#pragma pack (1)
typedef struct
{
  short      x;
  short      y;
  short      cx;
  short      cy;
  int        bkdc;
  int        fgdc;
  int        updc;
  int        num_cmds_in_queue;
  exec_t     exec[SFRAME_MAX_NUM_CMDS_IN_QUEUE];
} sframe_t;
#pragma pack ()

void sframe_init(void);

void sframe_deinit(void);

void sframe_command(int cmd, void * arg);

void sframe_on_command(int cmd, void * arg);

void sframe_bmoni_ts_disable(void);

#endif // __SFRAME_H
