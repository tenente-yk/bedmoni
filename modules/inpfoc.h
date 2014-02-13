/*! \file inpfoc.h
    \brief Input cursor interface.
*/

#ifndef __INPFOC_H
#define __INPFOC_H

/*! \enum
    \brief List of input cursors types.
*/
enum
{
  INPFOC_TYPE_PUSH,
  INPFOC_TYPE_LIST,
};

/*! \enum inpfoc_e
    \brief List of all employed input cursors.
*/
enum
{
  INPFOC_NONE,     // 0
  INPFOC_MAIN,     // 1
  INPFOC_DEFAULT = INPFOC_MAIN,
  INPFOC_GENERAL,  // 2
  INPFOC_DT,       // 3
  INPFOC_PAT,      // 4
  INPFOC_SET,      // 5
  INPFOC_ALARM,    // 6
  INPFOC_TRENDS,   // 7
  INPFOC_MOD,      // 8
  INPFOC_DEFSET,   // 9
  INPFOC_NETSET,   // 10
  INPFOC_ECGSET,   // 11
  INPFOC_SPO2SET,  // 12
  INPFOC_RESPSET,  // 13
  INPFOC_NIBPSET,  // 14
  INPFOC_T1T2SET,  // 15
  INPFOC_TABLE,    // 16
  INPFOC_TABLESET, // 17
  INPFOC_PRINTSET, // 18
  INPFOC_TRENDSET, // 19
  INPFOC_SHUTDOWN, // 20
  INPFOC_PASSWORD, // 21
  INPFOC_PATNEW,   // 22
  INPFOC_STSET,    // 23
  INPFOC_ARRHSET,  // 24
  INPFOC_NIBPCAL,  // 25
  INPFOC_SWUPD,    // 26
  INPFOC_LOWBAT,   // 27
  INPFOC_LANG,     // 28
  INPFOC_DISP,     // 29
  INPFOC_DEMOSET,  // 30
  INPFOC_DIALOG,   // 31
  INPFOC_LEAKTEST, // 32
  INPFOC_VERIF,    // 33
  INPFOC_CO2SET,   // 34
  INPFOC_CO2INISET,// 35
  INPFOC_EVLOG,    // 36
  INPFOC_EVLOGSET, // 37
  INPFOC_APPINFO,  // 38
  INPFOC_ADMSET,   // 39
  INPFOC_MAX_NUM,  //
};

#pragma pack(1)
typedef struct
{
  int delta;
} inpfoc_cmd_t;

typedef struct inpfoc_item inpfoc_item_t;
struct inpfoc_item
{
  void          * parg;      // inpfoc_wnd_t *
  inpfoc_item_t * next;
  inpfoc_item_t * prev;
  short           item;
  signed char     delta;
  char            first;
  char            last;
  char            active;
  char            changed;
  char            pressed;
  char            checked;
  char            selected;
  char            rolled;
  char            deleted;
  char            disabled;
  unsigned char   type;
};

typedef struct
{
  int fno;
  void (*func)(void*);
} inpfoc_funclist_t;
#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*! \fn inpfoc_item_t * inpfoc_find_active(int id)
 *  \brief Find currently active cursor item.
 *  \param id Cursor group id
 *  \return Pointer to \a inpfoc_item_t structure, containing found item, NULL - if no any item found.
 */
inpfoc_item_t * inpfoc_find_active(int id);

/*! \fn inpfoc_item_t * inpfoc_find(int id, int item)
 *  \brief Find currently active cursor item.
 *  \param id Cursor group id
 *  \param item Cursor item id in group mentioned by \a id
 *  \return Pointer to \a inpfoc_item_t structure, containing found item, NULL - if no any item found.
 */
inpfoc_item_t * inpfoc_find(int id, int item);

/*! \fn inpfoc_item_t * inpfoc_find_selected(int id)
 *  \brief Find currently pressed cursor item.
 *  \param id Cursor group id
 *  \return Pointer to \a inpfoc_item_t structure, containing found item, NULL - if no any item found.
 */
inpfoc_item_t * inpfoc_find_selected(int id);

/*! \fn inpfoc_item_t * inpfoc_find_first(int id)
 *  \brief Find first cursor item in linked list.
 *  \param id Cursor group id
 *  \return Pointer to \a inpfoc_item_t structure, containing found item, NULL - if no any item found.
 */
inpfoc_item_t * inpfoc_find_first(int id);

// add to the end
void inpfoc_add(int id, int item, void * parg);

// insert after the item0 (if item0 == NULL is inserted as first element)
void inpfoc_ins(int id, int item, void * parg, int item0);

void inpfoc_del(int id, int item);

void inpfoc_list(int id);

void inpfoc_set(int id, int item);

void inpfoc_press();

void inpfoc_shl(int delta);

void inpfoc_shr(int delta);

void inpfoc_update(void);

void inpfoc_invalidate(int id);

void inpfoc_disable(int id, int item);

void inpfoc_enable(int id, int item);

void inpfoc_focus(int id);

void inpfoc_proc(const int id, const int item, int delta);

//void inpfoc_set_caption(int id, int item, char * s);

void inpfoc_change(int id);

int  inpfoc_getcurr(void);

void inpfoc_rm(int id);

void inpfoc_reload(int id); // !!! make changes instantly

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __INPFOC_H
