#ifndef __LFRAME_H
#define __LFRAME_H

#include "dview.h"
#include "mframe.h"

#define TBL_BUTTON_CY       20
#define TBL_SCROLL_W        15

#define LFRAME_X0           0
#define LFRAME_Y0           25
#define LFRAME_CX           (DISPLAY_CX-MFRAME_CX-1)
#define LFRAME_CY           (DISPLAY_CY-LFRAME_Y0)

//#define LFRAME_BKCOLOR     RGB(0x77,0x77,0x77)
#define LFRAME_BKCOLOR      RGB(0x00,0x00,0x00)

#define LFRAME_MODE_BRIEF   0
#define LFRAME_MODE_FULL    1

#define TABLE_FONT          ARIAL_10
#define TABLE_FONTCOLOR     RGB(0xbb,0xbb,0xbb)

#define TABLE_MAX_NUM_IDS   30
#define TABLE_MAX_NUM_LINES 30

#define LFRAME_MAX_NUM_CMDS_IN_QUEUE   10

// lframe commands range 0x60-0x6f
#define LFRAME_CREATE          0x60
#define LFRAME_HSCROLL         0x61
#define LFRAME_VSCROLL         0x62
#define LFRAME_RESIZE          0x63
#define LFRAME_DESTROY         0x64
#define LFRAME_RELOAD          0x65
#define LFRAME_NEWDATA         0x66
#define LFRAME_CHANGE_TS       0x67

enum
{
  TSM_1,
  TSM_2,
  TSM_5,
  TSM_10,
  TSM_15,
  TSM_30,
  TSM_60,
  NUM_TSM,
};

#pragma pack(1)
typedef struct
{
  int id[TABLE_MAX_NUM_IDS];
} tbl_cs_t;

typedef struct
{
  int            x;
  int            y;
  short          cx;
  short          cy;
  char           s[40];
  unsigned short sw[2];
  short          flags;
}
 table_data_t;

typedef struct
{
  short             nl;
  short             nc;
  short             len;
  table_data_t    * ptd;
} table_t;

typedef struct
{
  short      visible;
  short      x;
  short      y;
  short      cx;
  short      cy;
  short      mode;
  short      tsm;
  short      voffs;
  short      hoffs;
  int        bkdc;
  int        fgdc;
  int        updc;
  table_t    table;
  int        num_cmds_in_queue;
  exec_t     exec[LFRAME_MAX_NUM_CMDS_IN_QUEUE];
} lframe_t;

typedef struct
{
  int        visible;
  short      mode;
  short      tsm;
  tbl_cs_t   tbl_cs;
} lframe_cfg_t;
#pragma pack()

void lframe_ini_def(void);

void lframe_init(void);

void lframe_deinit(void);

void lframe_show(void);

void lframe_hide(void);

void lframe_command(int cmd, void * arg);

void lframe_on_command(int cmd, void * arg);

void table_load(int mode, tbl_cs_t * ptbl_cs, table_t * ptbl);

void table_unload(table_t * ptbl);

void tbl_cs_read(tbl_cs_t * ptcs);

void tbl_cs_write(tbl_cs_t * ptcs);

void id2tblpar(int id, tbl_par_t * ptp);

// exec in PRIOR_NORM task
void lframe_on_resize(const int x, const int y, const int cx, const int cy);

void lframe_resize(const int x, const int y, const int cx, const int cy);

void lframe_cfg_save(void);

lframe_t * lframe_getptr(void);

extern short tsm[NUM_TSM]; // uses in tblmenu.c

#endif // __LFRAME_H
