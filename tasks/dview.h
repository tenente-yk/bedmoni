/*! \file dview.h
    \brief Framebuffer and graphics.
*/

#ifndef __DVIEW_H
#define __DVIEW_H

#ifdef WIN32
//#include <windows.h>
#endif

#include "defs.h"
#include "pat.h"
#include "alarm.h"
#include "fifo.h"

#define DISPLAY_CX            800
#define DISPLAY_CY            480
#ifdef ARM
#define DISPLAY_CX_MM         155
#define DISPLAY_CY_MM         94
#else
#define DISPLAY_CX_MM         235
#define DISPLAY_CY_MM         145
#endif

#define MM2PIXY               (DISPLAY_CY / DISPLAY_CY_MM)

#define DVIEW_DELAY_US        (50*1000) // 50 ms
#define DVIEW_DELAY_MS        (50)

#define DVIEW_MAX_FIFO_SIZE   (10*DISPLAY_CX) //1000

#define DVIEW_MAX_NUM_CMDS_IN_QUEUE   300
#define DVIEW_ARG_DATA_SIZE           400
#define DVIEW_UPD_MAX_SIZE            20000

#ifdef ARM
#define  FB_DEV   "/dev/fb"
#else
#define  FB_DEV   "/dev/fb0"
#endif

/*! \enum
 *  \brief Screen frames.
 */
enum
{
  CFRAME = 0, ///< - channel data view
  MFRAME,     ///< - measurements data view
  TFRAME,     ///< - trends frame
  LFRAME,     ///< - table frame
  UFRAME,     ///< - menu frame
  IFRAME,     ///< - information panel
  SFRAME,     ///< - status frame
  EFRAME,     ///< - event logs frame
  NUM_FRAMES, // 8
};

/*! \enum
 *  \brief Data channels list.
 */
enum
{
  ECG_1,                 // 0
  ECG = ECG_1,           // 0
  ECG_FIRST = ECG_1,     // 0
  ECG_I = ECG_1,         // 0
  ECG_2,                 // 1
  ECG_II = ECG_2,        // 1
  ECG_3,                 // 2
  ECG_III = ECG_3,       // 2
  ECG_4,                 // 3
  ECG_aVR = ECG_4,       // 3
  ECG_5,                 // 4
  ECG_aVL = ECG_5,       // 4
  ECG_6,                 // 5
  ECG_aVF = ECG_6,       // 5
  ECG_7,                 // 6
  ECG_V = ECG_7,         // 6
  ECG_LAST = ECG_7,      // 6
  NUM_ECG,               // 7
  ECG_NEO = NUM_ECG,     // 7
  SPO2,                  // 8
  NIBP,                  // 9
  RESP,                  // 10
  T1T2,                  // 11
  CO2,                   // 12
  NUM_VIEWS,             // 13
};

/*! \enum
 *  \brief Measured parameters list.
 */
enum
{
  PARAM_NONE, // 0
  PARAM_TS,   // 1
  PARAM_FIRST = PARAM_TS,
  PARAM_HR,   // 2
  PARAM_SPO2, // 3
  PARAM_PULSE,// 4
  PARAM_NIBP, // 5
  PARAM_T1,   // 6
  PARAM_T2,   // 7
  PARAM_DT,   // 8
  PARAM_BR,   // 9
  PARAM_STI,  // 10
  PARAM_ST_FIRST = PARAM_STI,
  PARAM_STII, // 11
  PARAM_STIII,// 12
  PARAM_STAVR,// 13
  PARAM_STAVL,// 14
  PARAM_STAVF,// 15
  PARAM_STV,  // 16
  PARAM_ST_LAST = PARAM_STV,
  PARAM_BRC,  // 17
  PARAM_ETCO2,// 18
  PARAM_ICO2, // 19
  NUM_PARAMS, // 20
};

#pragma pack(1)

typedef struct
{
  short valid;
  short x0;
  short y0;
  short cx;
} upd_view_info_t;

typedef struct
{
  int            param;
  unsigned short ids_name;
  unsigned short ids_dim;
} tbl_par_t;

typedef struct
{
  int             frameid;
  int             cmd;
  unsigned char   data[DVIEW_ARG_DATA_SIZE];
} exec_t;

typedef struct
{
  short           xres;
  short           yres;
  unsigned char * p_bk;
  unsigned char * p_fg;
  unsigned char * p_up;
} dc_list_t;

typedef struct
{
  void (* init)(void);
  void (* deinit)(void);
  void (* command)(int cmd, void * arg);
  void (* show)(void);
  void (* hide)(void);
  void (* resize)(int x, int y, int cx, int cy);
}
frame_interface_t;

#pragma pack()

/*! \fn int dview_init(void)
 *  \brief Graphics interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
int  dview_init(void);

/*! \fn void dview_deinit(void)
 *  \brief Graphics interface deinit function.
 */
void dview_deinit(void);

/*! \fn void dview_update(void)
 *  \brief Graphics interface update function.
 */
#ifdef UNIX
void dview_update(void);
#endif
#ifdef WIN32
unsigned long dview_update(void *);
#endif

/*! \fn void dview_chandata_add(int chno, int v)
 *  \brief Add channel value to be printed.
 *  \param chno channel.
 *  \param v value.
 */
void dview_chandata_add(int chno, int v);

void dview_bitblt(int x, int y, int cx, int cy, dc_list_t * pdclist, int x0, int y0);

void dview_bitblt_fast(int x, int y, int cx, int cy, dc_list_t * pdclist, int x0, int y0);

void dview_draw(void);

#ifdef UNIX
void dview_update_fast(void);
#endif
#ifdef WIN32
unsigned long dview_update_fast(void *);
#endif

/*unsigned int get_fbmap_addr(void);*/

void dview_set_state(int t);

int dview_get_state(void);

/*! \fn void dview_frame_active(int frameid)
 *  \brief Set active frame.
 *  \param frameid Frame ID.
 */
void dview_frame_active(int frameid);

/*! \fn void dview_command(int frameid, int cmd, int cv, void * arg)
 *  \brief Send command to frame.
 *  \param frameid Frame ID.
 *  \param cmd Command.
 *  \param cv Flags.
 *  \param arg Argument.
 */
void dview_command(int frameid, int cmd, int cv, void * arg);

#ifdef WIN32
//HWND dview_get_root_wnd(void);
#endif

extern fifo_t fifo_dview_chan[NUM_VIEWS];

extern char chantitle[NUM_VIEWS][10];
extern unsigned short chan_ids[NUM_VIEWS];
extern unsigned int paramcolor[NUM_PARAMS];
extern unsigned int chancolor[NUM_VIEWS];
extern unsigned int risk_color[NUM_RISKS];
extern tbl_par_t tbl_par[NUM_PARAMS];
extern char * ecgstr[NUM_ECG];

extern frame_interface_t frame_interface[NUM_FRAMES];

extern int rootwfd;
#endif // __DVIEW_H
