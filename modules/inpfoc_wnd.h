/*! \file inpfoc_wnd.h
    \brief Input focus graphics
*/

#ifndef __INPFOC_WND_H_
#define __INPFOC_WND_H_

#include "defs.h"

#define IWC_CHECKED      0x1
#define IWC_UNCHECKED    0x0
#define IWC_TOGGLED      0x2

#define SMOOTHING_BUTTONS

enum
{
  STATE_NONE,
  STATE_ACTIVE,
  STATE_SELECT,
  STATE_DISABLED,
};

enum
{
  INPFOC_WND_LABEL,
  INPFOC_WND_PUSH = INPFOC_WND_LABEL,
  INPFOC_WND_LIST,
  INPFOC_WND_RADIOBUTTON,
  INPFOC_WND_CHECKBOX,
  INPFOC_WND_SPINBUTTON,
  INPFOC_WND_SCROLLBAR,
  INPFOC_WND_SLIDER,
  INPFOC_WND_PROGRESSBAR,
};

#define LABEL_FONT            MONOS_10
//#define LABEL_FONT            ARIAL_12

#define INPFOC_COLOR_TEXT     0x222222
#define INPFOC_COLOR_DEF      0x888888
#define INPFOC_COLOR_ACTIVE   0xDDDDDD
#define INPFOC_COLOR_SELECT   RGB(189,130,130)
#define INPFOC_COLOR_DISABLED 0x444444

#define SCROLLBAR_TYPE_HORZ    0
#define SCROLLBAR_TYPE_VERT    1

#define SLIDER_TYPE_HORZ       0
#define SLIDER_TYPE_VERT       1

typedef struct
{
  short         scroll_type;
  short         range;
  short         pos;
} scrollbar_info_t;

typedef struct
{
  short         slider_type;
  short         range;
  short         pos;
} slider_info_t;

typedef struct
{
  short         progress_type;
  short         range;
  short         pos;
} progressbar_info_t;

typedef void (* USER_FRAMEBITBLT)(int, int, int, int, void*);

typedef struct
{
  unsigned int fgcolor;
  unsigned int bkcolor;
  int          bkdc;
  int          updc;
} inpfoc_wnd_dc_t;

typedef struct
{
  short            x;
  short            y;
  short            cx;
  short            cy;
  int              bkfd;
  int              upfd;
  char             caption[60]; // addr has to be 0x4-aligned
  unsigned short   ids;
  unsigned int     fgcolor;
  unsigned int     bkcolor;
  short            checked;
  short            transp;
  void *           pframe;
  void *           parg;
  USER_FRAMEBITBLT bitblt_func;
  unsigned char    type;
  unsigned char    unused[1];
} inpfoc_wnd_t;

void inpfoc_wnd_init(void);

void inpfoc_wnd_deinit(void);

inpfoc_wnd_t * inpfoc_wnd_create(char * name, int x, int y, int cx, int cy, char *s, unsigned short ids, inpfoc_wnd_dc_t * pifwd, USER_FRAMEBITBLT bitblt_func, void *pframe, ...);

void inpfoc_wnd_destroy(inpfoc_item_t * pifi);

void inpfoc_wnd_select(inpfoc_item_t * pifi);

void inpfoc_wnd_deselect(inpfoc_item_t * pifi);

void inpfoc_wnd_press(inpfoc_item_t * pifi);

void inpfoc_wnd_check(inpfoc_item_t * pifi, unsigned int state);

void inpfoc_wnd_scroll(inpfoc_item_t * pifi, short delta);

//void inpfoc_wnd_progress(inpfoc_item_t * pifi, unsigned short pos);

void inpfoc_wnd_move(inpfoc_item_t * pifi, int x, int y, int clrOld);

void inpfoc_wnd_size(inpfoc_item_t * pifi, int cx, int cy, int clrOld);

//void inpfoc_wnd_resize(inpfoc_item_t * pifi, int cx, int cy);
void inpfoc_wnd_resize(inpfoc_item_t * pifi, double mx, double my);

void inpfoc_wnd_setbkcolor(inpfoc_item_t * pifi, COLORREF color);

void inpfoc_wnd_setcaption(inpfoc_item_t * pifi, char *s);

void inpfoc_wnd_getcaption(inpfoc_item_t * pifi, char *s, int len);

void inpfoc_wnd_setids(inpfoc_item_t * pifi, unsigned short ids);

int  inpfoc_wnd_getchecked(inpfoc_item_t * pifi);

void inpfoc_wnd_setrange(inpfoc_item_t * pifi, unsigned short range);

void inpfoc_wnd_setpos(inpfoc_item_t * pifi, unsigned short pos);

void inpfoc_wnd_disable(inpfoc_item_t * pifi);

#endif // __INPFOC_WND_H_
