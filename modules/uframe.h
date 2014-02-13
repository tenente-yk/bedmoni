#ifndef __UFRAME_H
#define __UFRAME_H

#include "dview.h"
#include "menucs.h"

#define UFRAME_X0          0
#define UFRAME_Y0          25
#define UFRAME_CX          (200-1)
#define UFRAME_CY          455

#define UFRAME_BKCOLOR     RGB(0x00,0x11,0x00)
//#define UFRAME_BKCOLOR     RGB(0x00,0x00,0x00)
#define UFRAME_STATICTEXT_COLOR     0xcccccc

#define UFRAME_MAX_NUM_CMDS_IN_QUEUE   10

// uframe commands range 0x50-0x5f
#define UFRAME_CREATE        0x50
#define UFRAME_DESTROY       0x51
#define UFRAME_SETSTEXT      0x52
#define UFRAME_CHANGE        0x53
#define UFRAME_UPDATEMENU    0x54
#define UFRAME_MB_YES_NO     0x55
#define UFRAME_MB_OK         0x56
#define UFRAME_CLEARRECT     0x57

#define UFRAME_NUM_STEXT     4
#define UFRAME_FONT_TEXT     MONOS_10

#pragma pack (1)
typedef struct
{
  short         x;
  short         y;
  unsigned int  color;
  char          s[100];
} stext_t;

typedef void (*DialogProc)(void*);

typedef struct
{
  char caption[300];
  DialogProc on_yes_func;
  DialogProc on_no_func;
} mb_yes_no_t;

typedef struct
{
  int        visible;
  int        id;
  int        x;
  int        y;
  int        cx;
  int        cy;
  int        fgfd;
  int        bkdc;
  int        fgdc;
  int        updc;
  int        inpfoc;
  int        inpfoc_prev;
  menucs_t * pmcs;
  int        num_cmds_in_queue;
  exec_t  exec[UFRAME_MAX_NUM_CMDS_IN_QUEUE];
} uframe_t;
#pragma pack ()

void uframe_init(void);

void uframe_deinit(void);

void uframe_command(int cmd, void * arg);

void uframe_on_command(int cmd, void * arg);

void uframe_exec_command_queue(void);

uframe_t * uframe_getptr(void);

void uframe_printbox(short x, short y, short cx, short cy, char *s, COLORREF color);

void uframe_clearbox(short x, short y, short cx, short cy);

#endif // __UFRAME_H
