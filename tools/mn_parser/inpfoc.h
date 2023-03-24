#ifndef __INPFOC_H
#define __INPFOC_H

enum
{
  INPFOC_TYPE_PUSH,
  INPFOC_TYPE_LIST,
};

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
  INPFOC_BRGH,     // 8
  INPFOC_MOD,      // 9
  INPFOC_DEFSET,   // 10
  INPFOC_NETSET,   // 11
  INPFOC_ECGSET,   // 12
  INPFOC_SPO2SET,  // 13
  INPFOC_RESPSET,  // 14
  INPFOC_NIBPSET,  // 15
  INPFOC_T1T2SET,  // 16
  INPFOC_TABLE,    // 17
  INPFOC_TABLESET, // 18
  INPFOC_PRINTSET, // 19
  INPFOC_TRENDSET, // 20
  INPFOC_SHUTDOWN, // 21
  INPFOC_PASSWORD, // 22
  INPFOC_PATNEW,   // 23
  INPFOC_STALR,    // 24
  INPFOC_ARRHSET,  // 25
  INPFOC_NIBPCAL,  // 26
  INPFOC_SWUPD,    // 27
  INPFOC_ACCDUR5,  // 28
  INPFOC_LANG,     // 29
  INPFOC_DISP,     // 30
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
  unsigned int    type;
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

inpfoc_item_t * inpfoc_find_active(int id);

inpfoc_item_t * inpfoc_find(int id, int item);

inpfoc_item_t * inpfoc_find_selected(int id);

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

void inpfoc_invalidate(void);

void inpfoc_proc(const int id, const int item, int delta);

//void inpfoc_set_caption(int id, int item, char * s);

void inpfoc_change(int id);

int  inpfoc_getcurr(void);

void inpfoc_rm(int id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __INPFOC_H
