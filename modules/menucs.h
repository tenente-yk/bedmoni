/*! \file menucs.h
    \brief Menu resources interface.
*/

#ifndef __MENUCS_H
#define __MENUCS_H

/*
MEN file structure:
Consists of merged packets, ends by 4 MI_EOFs
Converts to *.men from *.mn resource files by using a "mn" utility
packet structure:
offset   length   description
0        1        MI_BEG
1        2        menu item packet len
3        1        menu item type (enum MI_)
4        2        menu item string id
6        1        menu item inpfoc id
7        1        menu item infoc item
8        2        x
10       2        y
12       2        cx
14       2        cy
16       1        reserved
17       1        reserved
18       1        MI_END
(I.e.: 0x05 ... 0x00 0x00 0x06 0x07 0x07 0x07 0x07)
*/

#define MI_BEG        0x05
#define MI_END        0x06
#define MI_EOF        0x07

/**
 * \brief List of GUI components.
 */
enum
{
  MI_NONE,        // 0
  MI_STATIC,      // 1
  MI_INPFOC,      // 2
  MI_SPINBUTTON,  // 3
  MI_CHECKBOX,    // 4
  MI_SLIDER,      // 5
  MI_PROGRESSBAR, // 6
  MI_LISTBUTTON,  // 7
  MI_RADIOBUTTON  // 8
};

/**
 * \brief Menu list.
 */
enum
{
  MENU_NONE,
  MENU_GENERAL,
  MENU_DATETIME,
  MENU_PATIENT,
  MENU_SETTINGS,
  MENU_ALARMS,
  MENU_MODULES,
  MENU_DEFAULTSET,
  MENU_NETWORKSET,
  MENU_ECGSET,
  MENU_SPO2SET,
  MENU_RESPSET,
  MENU_NIBPSET,
  MENU_T1T2SET,
  MENU_TABLESET,
  MENU_PRINTSET,
  MENU_TRENDSET,
  MENU_SHUTDOWN,
  MENU_PASSWORD,
  MENU_PATIENTNEW,
  MENU_STSET,
  MENU_ARRHSET,
  MENU_NIBPCAL,
  MENU_SWUPDATE,
  MENU_LOWBATSHUTDOWN,
  MENU_LOWBATCHARGE,
  MENU_LANGSEL,
  MENU_DISPSET,
  MENU_DEMOSET,
  MENU_LEAKTEST,
  MENU_VERIF,
  MENU_CO2SET,
  MENU_CO2INISET,
  MENU_EVLOGSET,
  MENU_APPINFO,
  MENU_ADMSET,
  NUM_MENUS,
};

#pragma pack(1)

typedef char* menucs_st;

/**
 * \brief Menu item structure.
 */
typedef struct
{
  unsigned char  type;
  unsigned short ids;
  unsigned short x;
  unsigned short y;
  unsigned short cx;
  unsigned short cy;
  int            ifid;
  int            ifit;
  char           sid[20];
  char           sidi[20];
  char           fontname[100];
  unsigned char  fontsize;
} menucs_item_t;

typedef struct menucs_item_lst menucs_item_lst_t;
struct menucs_item_lst
{
  menucs_item_t     * item;
  menucs_item_lst_t * next;
};

typedef void (*MENUONCS_PROC)(void);

/**
 * \brief Menu record structure.
 */
typedef struct
{
  menucs_item_t       menucs_item; // for a while only 1 elem
  menucs_item_lst_t * pmil;
  MENUONCS_PROC       openproc;    /**< this call-back function calls when menu appears */
  MENUONCS_PROC       closeproc;   /**< this call-back function calls when menu dissappears */
} menucs_t;

/**
    \brief Menu structure.
*/
typedef struct
{
  int            id;               /**< menu id */
  unsigned long  addr;             /**< begin adress of the menu data in common resource file */
  MENUONCS_PROC  openproc;         /**< this call-back function calls when menu appears */
  MENUONCS_PROC  closeproc;        /**< this call-back function calls when menu dissappears */
} menu_t;

#pragma pack()

/*! \fn int menucs_load(int id, menucs_t * pmcs)
 *  \brief Load menu from file to memory.
 *  \param id Menu id
 *  \param pmcs Pointer to menu data structure
 *  \return Returns 0 on success, -1 otherwise.
 */
int menucs_load(int id, menucs_t * pmcs);

/*! \fn int menucs_unload(int id, menucs_t * pmcs)
 *  \brief Unload menu from memory.
 *  \param id Menu id
 *  \param pmcs Pointer to menu data structure
 *  \return Returns 0.
 */
int menucs_unload(int id, menucs_t * pmcs);

#include "datetime.h"

/*! \param
    \brief List of menu data structures.
*/
extern menu_t menus[NUM_MENUS];

#endif // __MENUCS_H
