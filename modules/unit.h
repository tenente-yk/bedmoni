/*! \file unit.h
    \brief Channels units
*/

#ifndef __UNIT_H
#define __UNIT_H

#include "alarm.h"
#include "grph.h"

#define UNIT_DEF_HEIGHT           87 // (480-25-20) / 5

#define UNIT_TYPE_UNDEFINED       0x00
#define UNIT_TYPE_VAL_UNDEFINED   0x00
#define UNIT_TYPE_TEXT            0x01
#define UNIT_TYPE_ICON            0x02
#define UNIT_TYPE_PROGRESS        0x03
#define UNIT_TYPE_INT             0x04
#define UNIT_TYPE_FLOAT           0x05
#define UNIT_TYPE_LABEL           0x06
#define UNIT_TYPE_VAL_TEXT            0x01
#define UNIT_TYPE_VAL_ICON            0x02
#define UNIT_TYPE_VAL_PROGRESS        0x03
#define UNIT_TYPE_VAL_INT             0x04
#define UNIT_TYPE_VAL_FLOAT           0x05
#define UNIT_TYPE_VAL_LABEL           0x06

#define UNIT_TYPE_RANGE           0x4006

#define UNIT_TEMP_T1              1
#define UNIT_TEMP_T2              2

#define UNIT_BKCOLOR              RGB(0x00,0x00,0x00) // RGB(0x50,0x00,0x00)
#define UNIT_FGCOLOR              RGB(0x00,0xFF,0x00)

#define UNIT_MAX_NUM_ITEMS        30
#define UNIT_MAX_NUM_ALARMS       20
#define UNIT_MAX_NUM_RISKQUEUE    40

#define UNDEF_VALUE               (0x7AAA)

#define UNIT_FONTNAME             "ARIAL_N"

#define UNIT_CMD_UPDATE           44
#define UNIT_CMD_SHOW             45
#define UNIT_CMD_HIDE             46
#define ITEM_CMD_SHOW             47
#define ITEM_CMD_HIDE             48
#define ITEM_CMD_SET_COLOR        49

/**
 * \brief Ioctl commands for units.
 */
enum
{
  SET_ALARM = 1,
  SET_ALARM_ITEM,
  CLR_ALARM,
  CLR_ALARM_ITEM,
  SET_VALUE,
};

/**
 * \brief Images (icons) identificators.
 */
enum
{
  IMAGE_BELL = 144,
  IMAGE_NOBELL,
  IMAGE_BLANK,
  IMAGE_HEART,
  IMAGE_PM,
};

typedef    unsigned char  image_id_t;

//item_ntuni_t;

#pragma pack (1)
typedef struct _alarm_item alarm_item_t;
struct _alarm_item
{
  alarm_msg_t *  p_alarm_msg;
  alarm_item_t * next;
};

/*
typedef struct
{
  char           fontname[100]; // for text type
  int            fontsize;      // for text type
  unsigned int   fontcolor;     // for text type
  image_id_t     image_id;      // for icon type
  short          cx;            // for progress bar type
  short          cy;            // for progress bar type
  unsigned short ids;
  union
  {
    int        pbs;
    int        inpfoc_item;
  };
} item_info_t;
*/

/**
 * \brief Call-back function for unit component redraw.
 */
typedef void (*upd_func_t)(int chno, void * pitem, int redraw);

typedef struct
{
  unsigned char range : 1;
  unsigned char val   : 7;
} item_type_t;

/**
 * \brief Unit item data structure.
 */
typedef struct
{
  item_type_t    type;
  unsigned char  id;
  unsigned char  visible;
  short          x;
  short          y;
  int            v;
  double         df;
  unsigned short ids;            // ids for language switch
  char           s[200];         // text value (for text caption or text data)
  rect_t         rc0;            // old rect value
  unsigned int   fgcolor;
  unsigned int   bkcolor;
  short          cx;             // cx for progress bar
  short          cy;             // cy for progress bar
  union
  {
  unsigned char  pbs;            // style for progress bar
  };
  unsigned char  fontsize;       // for text labels
  int            font_id;
  image_id_t     image_id;
  unsigned char  alarmed;
  upd_func_t     upd_func;
} item_t;

/**
 * \brief Unit data structure.
 */
typedef struct
{
  int           visible;
  unsigned int  bkcolor;
  int           x;
  int           y;
  int           cx;
  int           cy;
  float         mx;
  float         my;
  int           nitems;
  item_t        item[UNIT_MAX_NUM_ITEMS];
  int           inpfoc_item;
  unsigned char alarmed;
  unsigned char alarms[UNIT_MAX_NUM_ALARMS];
  void *        data;
} unit_t;
#pragma pack ()

//void unit_create(unit_t ** ppunit, HWND parent, int x, int y, int cx, int cy);

/*! \fn void unit_ini_def(int reset_meas)
 *  \brief Init units with default settings.
 *  \param reset_meas Reset all units fields
 */
void unit_ini_def(int reset_meas);

/*! \fn void unit_align_all(void)
 *  \brief Align all units.
 */
void unit_align_all(void);

/*! \fn void unit_on_show(int chno, int nCmdShow)
 *  \brief Call-back function that is used to show or hide unit.
 *  \param chno Channel ID
 *  \param nCmdShow SW_HIDE or SW_SHOW (hide or show unit respectively)
 */
void unit_on_show(int chno, int nCmdShow);

/*! \fn void unit_item_add(int chno, int type, int id, int x, int y, int arg, int flags, ...)
 *  \brief Add unit component.
 *  \param chno Channel ID
 *  \param type Component type
 *  \param id Component ID
 *  \param x X coordinate where component to be placed
 *  \param y Y coordinate where component to be placed
 *  \param arg Any argument
 *  \param falgs Any argument/flags
 */
void unit_item_add(int chno, int type, int id, int x, int y, int arg, int flags, ...);

/*! \fn void void unit_ioctl(int chno, int cmd, ...)
 *  \brief Ioctl fucntion to handle unit or component.
 *  \param chno Channel ID
 *  \param cmd Command to be passed in Ioctl
 */
void unit_ioctl(int chno, int cmd, ...);

/*! \fn int  unit_isenabled(int chno)
 *  \brief Check is mentioned unit enabled.
 *  \param chno Channel ID
 *  \return Returns true if \a chno is enabled, false - otherwise
 */
int  unit_isenabled(int chno);

/*! \fn void unit_alarm_stepit(void)
 *  \brief Step it unit alarm.
 */
void unit_alarm_stepit(void);

/*! \fn void unit_on_change(int chno, int item_id)
 *  \brief Call-back redraw routine connected with mentioned unit item.
 *  \param chno Channel ID
 *  \param item_id Unit component ID
 */
void unit_on_change(int chno, int item_id);

/*! \fn int  unit_set_data(int chno, void * dataptr)
 *  \brief Set up unit data and settings.
 *  \param chno Channel ID
 *  \param dataptr Data pointer to be stored
 *  \return Returns 1 if all data has been stored, -1 - in case of failure
 */
int  unit_set_data(int chno, void * dataptr);

/*! \fn int  unit_get_data(int chno, void * dataptr)
 *  \brief Read unit data and settings.
 *  \param chno Channel ID
 *  \param dataptr Data pointer to array that will be fill with data
 *  \return Returns 1 if all data has been stored, -1 - in case of failure
 */
int  unit_get_data(int chno, void * dataptr);

/*! \fn void unit_update_data_all(void)
 *  \brief Update all units.
 */
void unit_update_data_all(void);

/*! \fn void unit_cfg_save(void)
 *  \brief Save unit configuration.
 */
void unit_cfg_save(void);

/*! \fn void unit_hide_heart(void)
 *  \brief Call-back function, called when need to hide heart icon on ECG unit.
 */
void unit_hide_heart(void);

/*! \fn void unit_hide_pm(void)
 *  \brief Call-back function, called when need to hide pacemaker icon on ECG unit.
 */
void unit_hide_pm(void);

#endif // __UNIT_H
