#ifndef __EFRAME_H
#define __EFRAME_H

#include "dview.h"
#include "mframe.h"

#define EFRAME_X0           0
#define EFRAME_Y0           25
#define EFRAME_CX           (DISPLAY_CX-MFRAME_CX-1)
#define EFRAME_CY           (DISPLAY_CY-25)

#define EFRAME_BKCOLOR      RGB(0x00,0x00,0x00)

#define EVLOG_BUTTON_CY     20

// eframe commands range 0x60-0x6f (same as lframe cmds)
#define EFRAME_CREATE          0x60
//#define EFRAME_HSCROLL         0x61
#define EFRAME_VSCROLL         0x62
#define EFRAME_RESIZE          0x63
#define EFRAME_DESTROY         0x64
#define EFRAME_RELOAD          0x65
#define EFRAME_NEWDATA         0x66
//#define EFRAME_CHANGE_TS       0x67

#define EFRAME_MAX_NUM_CMDS_IN_QUEUE   10

#define EFRAME_FONT_SMALL      ARIAL_10
#define EFRAME_FONT_NORMAL     ARIAL_12
#define EFRAME_FONT_LARGE      ARIAL_14

#pragma pack(1)

typedef struct
{
  short      visible;
  short      x;
  short      y;
  short      cx;
  short      cy;
  int        bkdc;
  int        fgdc;
  int        updc;
  int        offset;
  int        font;
  int        num_cmds_in_queue;
  exec_t     exec[EFRAME_MAX_NUM_CMDS_IN_QUEUE];
} eframe_t;

typedef struct
{
  int        visible;
  int        font;
} eframe_cfg_t;
#pragma pack()

void eframe_ini_def(void);

void eframe_init(void);

void eframe_deinit(void);

void eframe_command(int cmd, void * arg);

void eframe_on_command(int cmd, void * arg);

void eframe_on_resize(const int x, const int y, const int cx, const int cy);

void eframe_show(void);

void eframe_hide(void);


void eframe_cfg_save(void);

eframe_t * eframe_getptr(void);

#endif // __EFRAME_H
