#ifndef __CFRAME_H
#define __CFRAME_H

#include "mframe.h"

#define CFRAME_X0          0
#define CFRAME_Y0          25
#define CFRAME_CX          (800-MFRAME_CX-1)
#define CFRAME_CY          455

//#define CFRAME_X1          (CFRAME_X0 + CFRAME_CX)
//#define CFRAME_Y1          (CFRAME_X0 + CFRAME_CY)

#define CFRAME_LBCOLOR     RGB(0xff,0xff,0xff)
#define CFRAME_BKCOLOR     RGB(0x00,0x00,0x00)

#define MAXNUM_MMPS             4
#define PIXPERMV                20

#define MAXNUM_INPFOC_ITEMS     4 // per view

#define CFRAME_MAX_NUM_CMDS_IN_QUEUE     8

#define CFRAME_LABEL_DY_OFFS    (35) // in order not to overlap ecg mode caption

#define CFRAME_CHANNEL_BOUNDS

// cframe commands ranges 0x00-0x3f
#define CFRAME_HIDE             0x20       // arg - none
#define CFRAME_SHOW             0x21       // arg - none
#define CFRAME_RESIZE           0x22       // arg - pointer to RECT structure
#define CFRAME_RELOAD           0x23
#define CFRAME_CHAN_VISIBILITY  0x24
#define CFRAME_FREEZE_SCREEN    0x25
#define CFRAME_CHANSPD          0x26
#define CFRAME_CHANAMP          0x27
#define CFRAME_FILT_MODE_ROLL   0x28
#define CFRAME_ECGLEAD          0x29
#define CFRAME_RESET            0x2a
#define CFRAME_FILT_MODE_SET    0x2b
#define CFRAME_ECGLEAD_DEFAULT  0x2c
//#define CFRAME_START

//#define CFRAME_CHAN_CONST_MY       (0.05f)

#define CFRAME_NOVAL          -1000

#include "dview.h"
#include "tprn.h"

enum
{
  MY_AUTO,
  MY_FIRST = MY_AUTO,
  MY_1 = MY_AUTO,
  MY_2,
  MY_3,
  MY_4,
  MY_5,
  MAXNUM_MY,
};

#pragma pack (1)

typedef struct
{
  int          ecgtype;
  double       data_ptr;
  unsigned int old_tick;
  int          no_data;
  float        mmps;
  int          my;
  int          inv;
} chandata_t;

typedef struct
{
  int           visible;
  int           yu[CFRAME_CX + 30]; // 30 - reserve
  int           yl[CFRAME_CX + 30]; // 30 - reserve
  unsigned int  color;
  chandata_t    chandata;
  int           inpfoc_item[MAXNUM_INPFOC_ITEMS];
  int           num_inpfoc_items;
  int           y0;
  char          caption[48];
  int           ids;
} chanview_t;

typedef struct
{
  int        visible;
  int        x;
  int        y;
  int        cx;
  int        cy;
  int        num_visible_views;
  int        bkdc;
  int        fgdc;
  int        updc;
  chanview_t chanview[NUM_VIEWS];
 // int        tprn_c[TPRN_NUM_CURVES];
  int        num_cmds_in_queue;
  exec_t     exec[CFRAME_MAX_NUM_CMDS_IN_QUEUE];
} cframe_t;

typedef struct
{
  int           frame_visibility;
  chandata_t    chandata[NUM_VIEWS];
  int           view_visibility[NUM_VIEWS];
  unsigned char mode;
  unsigned char unused[3];
} cframe_cfg_t;

#pragma pack ()

void cframe_init(void);

void cframe_deinit(void);

void cframe_update_fast(void);

void cframe_chandata_nodata(int chno, int is_no_data);

int  cframe_chandata_isnodata(int chno);

//void cframe_chandata_update(cframe_t * pcframe);

void cframe_set_ecgfilt_mode(unsigned char mode);

//int  cframe_get_ecgfilt_mode(void);

void cframe_change_ecgchan(cframe_t * pcframe, int ecgpos, int delta);

void cframe_change_chanspd(cframe_t * pcframe, int chno, int delta);

void cframe_change_chanamp(cframe_t * pcframe, int chno, int delta);

void cframe_change_mode(cframe_t * pcframe, int delta);

//void cframe_chan_visible(cframe_t * pcframe, int chno, int visible);

void cframe_command(int cmd, void * arg);

void cframe_on_command(int cmd, void * arg);

// exec in PRIOR_NORM task
void cframe_on_resize(const int x0, const int y0, const int cx, const int cy);

void cframe_on_hide(void);

void cframe_on_show(void);

cframe_t * cframe_getptr(void);
#endif // __CFRAME_H

