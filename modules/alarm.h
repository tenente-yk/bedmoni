/*! \file alarm.h
    \brief alarm definitions
*/

#ifndef __ALARM_H
#define __ALARM_H

#define AlarmSound_None             0

#define AlarmSound_Any              1

#define AlarmSound_General_MP       2
#define AlarmSound_General_HP       3

#define AlarmSound_Cardiac_MP       4
#define AlarmSound_Cardiac_HP       5
/*
#define AlarmSound_ArtPerf_MP       6
#define AlarmSound_ArtPerf_HP       7

#define AlarmSound_Ventilation_MP   8
#define AlarmSound_Ventilation_HP   9

#define AlarmSound_Oxygen_MP        10
#define AlarmSound_Oxygen_HP        11

#define AlarmSound_TempEnergyDel_MP 12
#define AlarmSound_TempEnergyDel_HP 13

#define AlarmSound_DrugFluidDel_MP  14
#define AlarmSound_DrugFluidDel_HP  15
*/
#define AlarmSound_EquipmentFail_MP 16
#define AlarmSound_EquipmentFail_HP 17

#define BeepHeartRateSound          18

#define SOUND_MAX_VOLUME            5

/*! \enum
 *  \brief Alarm levels.
 */
enum
{
  NO_RISK,
  LOW_RISK,
  MED_RISK,
  HIGH_RISK,
  NUM_RISKS,
};

enum
{
  MSG_CLR,
  MSG_SET,
  MSG_CLR_FIXED,
};

enum
{
  MSG_NOFIX,
  MSG_FIX,
};

enum
{
  ALR_NONE,
  ALR_TECH,
  ALR_PHYS,
};


#define SND_ENABLED        1
#define SND_DISABLED       0

/*! \enum
 *  \brief Alarm identificators.
 */
typedef enum
{
  NO_ALARMS = 0,       // 0
  T1T2_NOSENSOR,       // 1
  T1T2_NOCONNECT,      // 2
  SPO2_NOSENSOR,       // 3
  SPO2_NOPATIENT,      // 4
  SPO2_NOPULSE,        // 5
  SPO2_RISKPULSE,      // 6
  SPO2_RISKSAT,        // 7
  SPO2_TUNEPROC,       // 8
  SPO2_TUNEERR,        // 9
  SPO2_DARKOBJ,        // 10
  SPO2_TRANSPOBJ,      // 11
  SPO2_BADSIGNAL,      // 12
  NIBP_NORESP,         // 13
  NIBP_RISKSS,         // 14
  NIBP_RISKDD,         // 15
  NIBP_RISKCC,         // 16
  NIBP_RISKMEASTIME,   // 17
  ECG_NOCONNECT,       // 18
  ECG_LBREAK,          // 19
  ECG_RBREAK,          // 20
  ECG_FBREAK,          // 21
  ECG_CBREAK,          // 22
  ECG_RISKHR,          // 23
  ECG_RISKSTI,         // 24
  ECG_RISKSTII,        // 25
  ECG_RISKSTIII,       // 26
  ECG_RISKSTAVR,       // 27
  ECG_RISKSTAVL,       // 28
  ECG_RISKSTAVF,       // 29
  ECG_RISKSTV,         // 30
  T1T2_RISKT1,         // 31
  T1T2_RISKT2,         // 32
  T1T2_RISKDT,         // 33
  RESP_RISKBR,         // 34
  RESP_APNOE,          // 35
  SYS_LOWCHARGE,       // 36
  SYS_EXTREMLOWCHARGE, // 37
  ECG_ASYSTOLIA,       // 38
  ECG_PM_NO_ANSWER,    // 39
  ECG_TACHYCARDIA,     // 40
  SPO2_NOCONNECT,      // 41
  TPRN_OPENED,         // 42
  TPRN_PAPEROUT,       // 43
  TPRN_OVERHEAT,       // 44
  TPRN_NOCONNECT,      // 45
  RESP_BADCONTACT_RL,  // 46
  RESP_BADCONTACT_RF,  // 47
  CO2_RISKETCO2,       // 48
  CO2_RISKICO2,        // 49
  CO2_RISKBRCO2,       // 50
  NIBP_RISKINFL,       // 51
  CO2_APNOE,           // 52
  SYS_NO_USBDISK,      // 53
  NIBP_WEAKSIGNAL,     // 54
  NIBP_ERRSIGNAL,      // 55
  NIBP_EXCRETRYCOUNT,  // 56
  NIBP_PNEUMBLOCK,     // 57
  NIBP_INFLTIMEOUT,    // 58
  NIBP_CALIB_EXPIRED,  // 59
  CO2_INVDATA,         // 60
  CO2_NOCONNECT,       // 61
  CO2_OVERTEMP,        // 62
  ECG_VENTR_FIBR,      // 63
  ECG_VENTR_FLUT,      // 64
  CO2_ZEROING,         // 65
  NIBP_INVPOWER,       // 66
  CO2_CHECKSENSOR,     // 67
  CO2_WARMUP,          // 68
  CO2_STARTUP,         // 69
  RESP_BREAK,          // 70
  SYS_CS_NOCONNECT,    // 71
  NUM_ALARMS,          // 72
} msg_num_e;

#pragma pack(1)

/*! \struct alarm_msg_t
 *  \brief Alarm parameters staructure.
 */
typedef struct
{
  unsigned char  msgno;     // 0 .. NUM_ALARMS
  unsigned char  level;
  unsigned char  state;
  unsigned char  fixed;
  unsigned char  enabled;
  unsigned char  type;
  unsigned char  alsnd;
  signed char    chno;      // -1 .. NUM_VIEWS
  signed char    item;        // unit item id
  unsigned char  __unused;
  unsigned short ids;
} alarm_msg_t;

/*! \struct ext_bits_t
 *  \brief External alarm bit structure.
 */
typedef struct
{
  unsigned char high   : 1;
  unsigned char medium : 1;
  unsigned char tech   : 1;
  unsigned char unused : 5;
} ext_bits_t;

/*! \struct alarm_cfg_t
 *  \brief Alarm status configuration.
 */
typedef struct
{
  unsigned char thsh_visible;
  unsigned char ge;            // global enable
  unsigned char volume;
  unsigned char qrs_volume;
  unsigned char delay;
  unsigned char delay_remd;
  union
  {
    unsigned char ext;
    ext_bits_t    ext_bits;
  };
  unsigned char unused;
  alarm_msg_t   alarm_msg[NUM_ALARMS];
} alarm_cfg_t;

/*! \struct alarm_cfg_t
 *  \brief Current alarm state.
 */
typedef struct
{
  unsigned char current_sound_alarm;
  unsigned char current_color_level;
  unsigned char current_extsound_alarm;
  unsigned char unused[1];
} alarm_info_t;

#pragma pack()

/*! \fn void alarm_ini(void)
 *  \brief Initialize alarm interface
 */
void alarm_ini(void);

/*! \fn void alarm_deini(void)
 *  \brief Deinitialize alarm interface
 */
void alarm_deini(void);

/*! \fn void alarm_ini_def(void)
 *  \brief Initialize alarm interface with defaults
 */
void alarm_ini_def(void);

/*! \fn void alarm_set_clr(int alarm, int set_clr)
 *  \brief Set/clear alarm
 *  \param alarm Alarm identificator
 *  \param set_clr Flag (1 - set alarm, 0 - clear alarm)
 */
void alarm_set_clr(int alarm, int set_clr);

/*! \fn void alarm_set(int alarm)
 *  \brief Set alarm
 *  \param alarm Alarm identificator
 */
void alarm_set(int alarm);

/*! \fn void alarm_clr(int alarm)
 *  \brief Clear alarm
 *  \param alarm Alarm identificator
 */
void alarm_clr(int alarm);

/*! \fn void alarm_clr_fixed(int alarm)
 *  \brief Clear fixable alarm
 *  \param alarm Alarm identificator
 */
void alarm_clr_fixed(int alarm);

/*! \fn int alarm_isset(int alarm)
 *  \brief Retrieve has alarm set or not
 *  \param alarm Alarm identificator
 *  \return 1 - alarm is set, 0 - alarm is not set
 */
int  alarm_isset(int alarm);

/*! \fn void alarm_on_off(int alarm, int on_off)
 *  \brief Change alarm state
 *  \param alarm Alarm identificator
 *  \param on_off On/off flag
 */
void alarm_on_off(int alarm, int on_off);

/*! \fn void alarm_enable(int alarm)
 *  \brief Enable alarm
 *  \param alarm Alarm identificator
 */
void alarm_enable(int alarm);

/*! \fn void alarm_disable(int alarm)
 *  \brief Disable alarm
 *  \param alarm Alarm identificator
 */
void alarm_disable(int alarm);

/*! \fn int alarm_isenabled(int alarm)
 *  \brief Retrieve has alarm enabled or not
 *  \param alarm Alarm identificator
 */
int  alarm_isenabled(int alarm);

/*! \fn void alarm_ge_on(void)
 *  \brief Enable all sound alarms globally
 */
void alarm_ge_on(void);

/*! \fn void alarm_ge_off(void)
 *  \brief Disable all sound alarms globally
 */
void alarm_ge_off(void);

/*! \fn void alarm_rst(void)
 *  \brief Reset all alarms
 */
void alarm_rst(void);

/*! \fn void alarm_update(void)
 *  \brief Update alarms state
 */
void alarm_update(void);

/*! \fn void alarm_crbp(int color_level, int alarm_sound)
 *  \brief Send alarm status to CRBP board
 *  \param color_level Set \a color_level color
 *  \param alarm_sound Play sound with \a alarm_sound id
 */
void alarm_crbp(int color_level, int alarm_sound);

/*! \fn void alarm_on_off_all_phys(int enabled)
 *  \brief Change state of all physiologic alarms
 *  \param enabled Alarm status (1 - on, 0 - off)
 */
void alarm_on_off_all_phys(int enabled);

/*! \fn void alarm_on_off_all_tech(int enabled)
 *  \brief Change state of all technical alarms
 *  \param enabled Alarm status (1 - on, 0 - off)
 */
void alarm_on_off_all_tech(int enabled);

/*! \fn void alarm_cfg_get(alarm_cfg_t * pcfg)
 *  \brief Read alarm configuration from config file
 *  \param pcfg Pointer to \a alarm_cfg_t structure
 */
void alarm_cfg_get(alarm_cfg_t * pcfg);

/*! \fn void alarm_cfg_set(alarm_cfg_t * pcfg)
 *  \brief Store alarm configuration to config file
 *  \param pcfg Pointer to \a alarm_cfg_t structure
 */
void alarm_cfg_set(alarm_cfg_t * pcfg);

/*! \fn void alarm_info_get(alarm_info_t * pai)
 *  \brief Read alarm state from config file
 *  \param pai Pointer to \a alarm_info_t structure
 */
void alarm_info_get(alarm_info_t * pai);

/*! \fn void alarm_info_set(alarm_info_t * pai)
 *  \brief Store alarm state to config file
 *  \param pai Pointer to \a alarm_info_t structure
 */
void alarm_info_set(alarm_info_t * pai);

/*! \fn void alarm_cfg_save(void)
 *  \brief Flush alarm config to config file
 */
void alarm_cfg_save(void);

extern alarm_msg_t alarm_msg[NUM_ALARMS];

#endif // __ALARM_H
