/*! \file mframe.h
    \brief Frame, containing channels data/settings/measurements
 */

#ifndef __MFRAME_H
#define __MFRAME_H

#include "dview.h"
#include "unit.h"
#include "ecgm.h"
#include "utils.h"

#define MFRAME_CX          244
#define MFRAME_CY          435
#define MFRAME_X0          (DISPLAY_CX-MFRAME_CX-1)
#define MFRAME_Y0          25

//#define MFRAME_BKCOLOR     RGB(0x44,0x44,0x44)
#define MFRAME_BKCOLOR     RGB(0x00,0x00,0x00)

#define MFRAME_MAX_NUM_CMDS_IN_QUEUE     8

// mframe commands range 0x40-04f
#define MFRAME_MAXIMIZE    0x40
#define MFRAME_NORMALIZE   0x41
#define MFRAME_HIDE        0x42
#define MFRAME_RESIZE      0x43
#define MFRAME_ENUMERATE   0x44
#define MFRAME_RELOAD      0x45
#define MFRAME_UNITS_ALARM 0x46
#define MFRAME_UNIT_CHANGE 0x47
#define MFRAME_UNIT_PAR_RANGES   0x48
#define MFRAME_UPDATE      0x49
#define MFRAME_STARTUP     0x4A
#define MFRAME_SET_STARTUP_CFG   0x4B

/**
 * \brief Units components id's (ECG)
 */
enum
{
  UNIT_ECG_LABEL = 0,   // ECG - label must be first
 // UNIT_ECG_HRSRC_CAPTION,
 // UNIT_ECG_HRSRC,
  UNIT_ECG_HR_BELL,
  UNIT_ECG_HR_RANGE,
  UNIT_ECG_HR,
  UNIT_ECG_ST1_CAPTION,
  UNIT_ECG_ST1_LEAD_CAPTION,
  UNIT_ECG_ST1_VALUE,
  UNIT_ECG_ST1_RANGE,
  UNIT_ECG_ST1_BELL,
  UNIT_ECG_ST2_CAPTION,
  UNIT_ECG_ST2_LEAD_CAPTION,
  UNIT_ECG_ST2_VALUE,
  UNIT_ECG_ST2_RANGE,
  UNIT_ECG_ST2_BELL,
  UNIT_ECG_HEART_IMG,
  UNIT_ECG_PM_IMG,
};

/**
 * \brief Units components id's (SpO2)
 */
enum
{
  UNIT_SPO2_LABEL = 0,  // SpO2 - label must be first
  UNIT_SPO2_PULSE_CAPTION,
  UNIT_SPO2_PULSE,
  UNIT_SPO2_PULSE_RANGE,
  UNIT_SPO2_SAT,
  UNIT_SPO2_SAT_RANGE,
  UNIT_SPO2_SAT_BELL,
  UNIT_SPO2_HR_BELL,
  UNIT_SPO2_STOLBIK,
  UNIT_SPO2_SCALE_CAPTION,
  UNIT_SPO2_SCALE,
};

/**
 * \brief Units components id's (T1T2)
 */
enum
{
  UNIT_T1T2_LABEL = 0,  // T - label must be first
  UNIT_T1T2_T1,         // T1 36.6
  UNIT_T1T2_T2,         // T2 36.4
  UNIT_T1T2_DT,         // dT 0.2
  UNIT_T1T2_T1_BELL,
  UNIT_T1T2_T2_BELL,
  UNIT_T1T2_DT_BELL,
  UNIT_T1T2_T1_RANGE,
  UNIT_T1T2_T2_RANGE,
  UNIT_T1T2_DT_RANGE,
};

/**
 * \brief Units components id's (NiBP)
 */
enum
{
  UNIT_NIBP_LABEL = 0,   // NIBP - label must be first
  UNIT_NIBP_BP_SS,       // 129/70 (104) - 129
  UNIT_NIBP_BP_SEP,      // 129/70 (104) - /
  UNIT_NIBP_BP_DD,       // 129/70 (104) - 70
  UNIT_NIBP_BP_BRK_O,    // 129/70 (104) - (
  UNIT_NIBP_BP_BRK_C,    // 129/70 (104) - )
  UNIT_NIBP_BP_CC,       // 129/70 (104) - 104
  UNIT_NIBP_STIME,       // Thu Feb 18 11:06:06 2010
  UNIT_NIBP_BP_SS_RANGE, //
  UNIT_NIBP_BP_DD_RANGE, //
  UNIT_NIBP_BP_CC_RANGE, //
  UNIT_NIBP_BELL,
  UNIT_NIBP_CUFF_PRESSURE,
  UNIT_NIBP_MEAS_INTERVAL,
 // UNIT_NIBP_PULSE_CAPTION,
 // UNIT_NIBP_PULSE,
};

/**
 * \brief Units components id's (Resp)
 */
enum
{
  UNIT_RESP_LABEL = 0,  // RESP - label must be first
  UNIT_RESP_BR,         // 15
  UNIT_RESP_BR_RANGE,
  UNIT_RESP_STATE,
  UNIT_RESP_APNOE_TIME,
  UNIT_RESP_BR_BELL,
  UNIT_RESP_APNOE_BELL,
};

/**
 * \brief Units components id's (CO2)
 */
enum
{
  UNIT_CO2_LABEL = 0,  // CO2 - label must be first
  UNIT_CO2_MMHG_CAPTION,
  UNIT_CO2_ETCO2_CAPTION,
  UNIT_CO2_BR_CAPTION,
  UNIT_CO2_ICO2_CAPTION,
  UNIT_CO2_ETCO2,
  UNIT_CO2_ETCO2_RANGE,
  UNIT_CO2_ETCO2_BELL,
  UNIT_CO2_BR,
  UNIT_CO2_BR_RANGE,
  UNIT_CO2_BR_BELL,
  UNIT_CO2_ICO2,
  UNIT_CO2_ICO2_RANGE,
  UNIT_CO2_ICO2_BELL,
  UNIT_CO2_BRSTATE,
  UNIT_CO2_APNOE_TIME,
  UNIT_CO2_APNOE_BELL,
};

/**
 * \brief Units components id's (Neonatal ECG)
 */
enum
{
  UNIT_ECG_NEO_LABEL = 0,  // ECG Neo - label must be first
};

#pragma pack(1)
/**
 * \brief Startup configuration to be sent to HW modules
 */
typedef struct
{
  unsigned char ecg_set;
  char          unused[3];
} startup_info_t;

/**
 * \brief ECG data and settings structure
 */
typedef struct
{
  union
  {
    ecgmset_bits_t  set_bits;
    unsigned char   set;
  };
  union
  {
    break_bits_t    break_bits;
    unsigned char   break_byte;
  };
  short             hr;
  short             hr_min;
  short             hr_max;
  short             st[NUM_ECG];
  short             st_min[NUM_ECG];
  short             st_max[NUM_ECG];
  unsigned char     asys_sec;
  unsigned short    tach_bpm;
  unsigned short    pmnr_ms;
  unsigned char     hr_src;      // 0-auto, 1-ecg, 2-spo2, 3-nibp // see ecgset.h
  unsigned char     hr_src_curr; // 1-ecg, 2-spo2, 3-nibp // see ecgset.h
  unsigned char     num_leads;
  unsigned char     st1;
  unsigned char     st2;
  short             j_shift;
  unsigned char     unused[2];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
ecg_data_t;

/**
 * \brief SpO2 data and settings structure
 */
typedef struct
{
  short  spo2;
  short  spo2_min;
  short  spo2_max;
  short  hr;
  short  hr_min;
  short  hr_max;
  short  stolb;
  short  scale;
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
spo2_data_t;

/**
 * \brief NiBP data and settings structure
 */
typedef struct
{
  short  sd_max;
  short  sd_min;
  short  dd_max;
  short  dd_min;
  short  md_max;
  short  md_min;
  short  sd;
  short  dd;
  short  md;
  short  infl;
  short  meas_interval;
  unsigned char ec; // error code
  short  hr;
  trts_t trts_meas;
  trts_t trts_cal;
  unsigned char hwerr_ack;
  unsigned char unused[2];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
nibp_data_t;

/**
 * \brief CO2 data and settings structure
 */
typedef struct
{
  short         etco2; // at least 2 bytes
  short         ico2;  // at least 2 bytes
  short         br;    // at least 2 bytes
  short         etco2_min;
  short         etco2_max;
  short         ico2_max;
  unsigned char br_min;
  unsigned char br_max;
  unsigned char ap_max;
  short         pressure;
  unsigned char o2_compens;
  unsigned char gas_balance;
  short         anest_agent;
  unsigned char unused[3];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
co2_data_t;

/**
 * \brief T1T2 data and settings structure
 */
typedef struct
{
  short t1;
  short t2;
  short t1_max;
  short t1_min;
  short t2_max;
  short t2_min;
  short dt_max;
  short unused;
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
t1t2_data_t;

/**
 * \brief Resp data and settings structure
 */
typedef struct
{
  short          br;
  short          ap;
  short          br_max;
  short          br_min;
  short          ap_max;
  unsigned char  lead; // 1 - RF, 0 - RL
  unsigned char  tau;  // 0 - 6.8 s, 1 - 0.32 s
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
resp_data_t;

/**
 * \brief Units configuration data structure
 */
typedef struct
{
  ecg_data_t     ecg_data;
  spo2_data_t    spo2_data;
  nibp_data_t    nibp_data;
  t1t2_data_t    t1t2_data;
  resp_data_t    resp_data;
  co2_data_t     co2_data;
} units_cfg_t;

typedef void (*frame_bitblt_func)(int x, int y, int cx, int cy);

/**
 * \brief Measurements frame data structure
 */
typedef struct
{
  int        visible;
  int        x;
  int        y;
  int        cx;
  int        cy;
  unit_t *   punit[NUM_VIEWS];
  int        bkdc;
  int        fgdc;
  int        updc;
  int        maximized;
  int        num_cmds_in_queue;
  exec_t     exec[MFRAME_MAX_NUM_CMDS_IN_QUEUE];
  unsigned long      unit_chmask;
  frame_bitblt_func  bitblt_func;
} mframe_t;

/**
 * \brief Measurements frame configuration
 */
typedef struct
{
  int           frame_visibility;
  unsigned long unit_chmask;
  int           maximized;
} mframe_cfg_t;

#pragma pack()

extern void unit_init(mframe_t * pmframe00);

extern void unit_deinit(void);

/*! \fn void mframe_init(void)
 *  \brief Init measurements frame.
 */
void mframe_init(void);

/*! \fn void mframe_deinit(void)
 *  \brief Deinit measurements frame.
 */
void mframe_deinit(void);

/*! \fn void mframe_command(int cmd, void * arg)
 *  \brief Push command to measurements frame.
 *  \param cmd Command
 *  \param arg Pointer to argument or argument
 */
void mframe_command(int cmd, void * arg);

/*! \fn void mframe_on_command(int cmd, void * arg)
 *  \brief Process measurements frame command.
 *  \param cmd Command
 *  \param arg Pointer to argument or argument
 */
void mframe_on_command(int cmd, void * arg);

/*! \fn int  mframe_ismaximized(void)
 *  \brief Tests measurements frame to be in fullscreen mode.
 *  \return Returns true if measurements is in fullscreen mode, false - otherwise
 */
int  mframe_ismaximized(void);

/*! \fn void mframe_process_alarms(void)
 *  \brief Process alarms and visualize it on units.
 */
void mframe_process_alarms(void);

// exec in PRIOR_NORM task

/*! \fn void mframe_on_resize(const int x0, const int y0, const int cx, const int cy)
 *  \brief Call-back function to resize measurements frame.
 *  \param x0 X coordinate of the left upper point of the rect to be mframe placed
 *  \param y0 Y coordinate of the left upper point of the rect to be mframe placed
 *  \param cx Width of the measuremets frame
 *  \param cy Height of the measuremets frame
 */
void mframe_on_resize(const int x0, const int y0, const int cx, const int cy);

/*! \fn mframe_t * mframe_getptr(void)
 *  \brief Obtain pointer to measurements frame data structure.
 *  \return Pointer to measurements frame data structure.
 */
mframe_t * mframe_getptr(void);

#endif // __MFRAME_H
