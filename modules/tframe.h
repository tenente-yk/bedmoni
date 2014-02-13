/*! \file tframe.h
    \brief Frame with trends curves
*/

#ifndef __TFRAME_H
#define __TFRAME_H

#include "dview.h"
#include "mframe.h"
#include "tr.h"

#define TFRAME_COLOR_CURSOR    RGB(0x99,0x99,0x99)
#define TFRAME_FONT_SMALL      ARIAL_8
#define TFRAME_FONT_NORMAL     ARIAL_10

#define TRN_BUTTON_CY      20

#define TFRAME_X0          0
#define TFRAME_Y0          25
#define TFRAME_CX          (DISPLAY_CX-MFRAME_CX-1)
#define TFRAME_CY          (DISPLAY_CY-25)
#define TFRAME_LB          130
#define TFRAME_RB          10
//#define TFRAME_BKCOLOR     RGB(0x77,0x77,0x77)
#define TFRAME_BKCOLOR     RGB(0x00,0x00,0x00)

#define TFRAME_MAX_NUM_CMDS_IN_QUEUE   10

// tframe commands range 0x70-0x7f
#define TFRAME_CREATE          0x70
#define TFRAME_DESTROY         0x71
#define TFRAME_CURSOR_SEEK     0x72
#define TFRAME_CURSOR_SEEKEND  0x73
#define TFRAME_RESIZE          0x74
#define TFRAME_MAXIMIZE        0x75
#define TFRAME_CHANGE_STEP     0x76
#define TFRAME_CURSOR_LSEEK    0x77
#define TFRAME_NEWDATA         0x78
#define TFRAME_RELOAD          0x79

#define TFRAME_MAX_NUM_VIEWS   5
#define TFRAME_MAX_NUM_CURVES  8
#define TFRAME_VIEW_DATALEN    (24*60)

#define TFRAME_BORDER_W        10

enum
{
  TSC_2,
  TSC_4,
  TSC_8,
  TSC_12,
  TSC_24,
  NUM_TSC,
};

#pragma pack(1)

/**
 * \brief Frame view configuration data structure.
 */
typedef struct
{
  int visible;
  unsigned short mask;
  unsigned short num_curves;
  int vl; // lower level
  int vu; // upper level
  int kv;
  int type[TFRAME_MAX_NUM_CURVES];
} view_cfg_t;

/**
 * \brief Graphic view data structure.
 */
typedef struct
{
  int visible;
  unsigned short mask;
  unsigned short num_curves;
  int vl; // lower level
  int vu; // upper level
  int kv;
  int type[TFRAME_MAX_NUM_CURVES];
  int data[TFRAME_MAX_NUM_CURVES][TFRAME_VIEW_DATALEN];
} tview_t;

/**
 * \brief Trends data frame data structure.
 */
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
  tview_t    view[TFRAME_MAX_NUM_VIEWS];
  unsigned char beg[TFRAME_VIEW_DATALEN];
 // float      xcur;
  int        vpos;
  int        vlen;
  int        tsc;
  int        num_cmds_in_queue;
  exec_t     exec[TFRAME_MAX_NUM_CMDS_IN_QUEUE];
} tframe_t;

/**
 * \brief Trends data frame configuration data structure.
 */
typedef struct
{
  int         visible;
  int         tsc;
  view_cfg_t  view_cfg[TFRAME_MAX_NUM_VIEWS];
} tframe_cfg_t;

#pragma pack()

/*! \fn void tframe_init(void)
 *  \brief Init trends frame interface.
 */
void tframe_init(void);

/*! \fn void tframe_ini_def(void)
 *  \brief Init trends frame interface with default settings.
 */
void tframe_ini_def(void);

/*! \fn void tframe_deinit(void)
 *  \brief Deinit trends frame interface.
 */
void tframe_deinit(void);

/*! \fn void tframe_show(void)
 *  \brief Make trends frame visible.
 */
void tframe_show(void);

/*! \fn void tframe_hide(void)
 *  \brief Make trends frame invisible.
 */
void tframe_hide(void);

/*! \fn void tframe_command(int cmd, void * arg)
 *  \brief Throw command to trends frame interface.
 */
void tframe_command(int cmd, void * arg);

/*! \fn void tframe_on_command(int cmd, void * arg)
 *  \brief Peek and call-back trends frame commands.
 */
void tframe_on_command(int cmd, void * arg);

/*! \fn void tframe_on_resize(const int x, const int y, const int cx, const int cy)
 *  \brief Call-back function, resizes trends frame dimensions.
 *  \param x X-coordinate of trends frame.
 *  \param y Y-coordinate of trends frame.
 *  \param cx Width of trends frame.
 *  \param cy Height of trends frame.
 */
void tframe_on_resize(const int x, const int y, const int cx, const int cy);

/*! \fn void tframe_cfg_save(void)
 *  \brief Save trends frame configuration.
 */
void tframe_cfg_save(void);

/*! \fn tframe_t * tframe_getptr(void)
 *  \brief Get pointer to trends frame data structure.
 *  \return Returns pointer to trends frame data structure.
 */
tframe_t * tframe_getptr(void);

#endif // __TFRAME_H
