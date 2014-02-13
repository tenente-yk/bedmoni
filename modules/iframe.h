#ifndef __IFRAME_H
#define __IFRAME_H

#include "dview.h"

#define IFRAME_X0          0
#define IFRAME_Y0          0
#define IFRAME_CX          800
#define IFRAME_CY          25

#define IFRAME_BKCOLOR     RGB(0x00,0x00,0x00)
#define IFRAME_FGCOLOR     RGB(0xff,0xff,0xff)

#define IFRAME_UPDATE_PERIOD   100
#define IFRAME_FONT        ARIAL_14

#define IFRAME_MAX_NUM_CMDS_IN_QUEUE   10

enum
{
  IFRAME_MODE_MONITOR = 0,
  IFRAME_MODE_TRENDS,
  IFRAME_MODE_TABLE,
  IFRAME_MODE_BLANK,
  IFRAME_MODE_EVLOGS,
  IFRAME_NUM_MODES,
};

// iframe commands range 0x80-0x8f
#define IFRAME_RELOAD          0x80
#define IFRAME_SET_PAT         0x81
#define IFRAME_SET_BEDNO       0x82
#define IFRAME_SET_CARDNO      0x83
#define IFRAME_MSG_UPDATE      0x84
#define IFRAME_SET_MODE        0x85
#define IFRAME_SET_ID          0x86
#define IFRAME_SET_STRING      0x87
#define IFRAME_SET_TIMERVAL    0x88

typedef struct
{
  int        x;
  int        y;
  int        cx;
  int        cy;
  int        visible;
  int        mode;
  int        v;
  int        bkdc;
  int        fgdc;
  int        updc;
  int        num_cmds_in_queue;
  exec_t     exec[IFRAME_MAX_NUM_CMDS_IN_QUEUE];
} iframe_t;
#pragma pack ()

void iframe_init(void);

void iframe_deinit(void);

void iframe_command(int cmd, void * arg);

void iframe_on_command(int cmd, void * arg);

void iframe_process_msgs(void);

iframe_t * iframe_getptr(void);

#endif // __IFRAME_H
