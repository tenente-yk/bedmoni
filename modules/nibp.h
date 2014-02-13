/*! \file nibp.h
    \brief NiBP module interface
 */

#ifndef __NIBP_H
#define __NIBP_H

/*
host -> module
[host start byte] [command byte] [data bytes] [crc]

module -> host
[module start byte] [packet byte] [data bytes] [crc]

crc = 0x100 - modulo 256 (start + command + data bytes)
*/

//#ifndef ARM
//#define NIBP_DEBUG
//#define NIBP_LOGGING
//#endif

/**
 * \brief Setting this define disables NiBP calibration check
 */
#define NIBP_CALIB_NO_CHECK        // TODO: undef in RELEASE

/** Setting this define disables NiBP */
#ifndef ARM // for TPRN simulation via /tty/USB0 port
#define NIBP_DISABLED
#endif

#ifndef NIBP_DEBUG
#undef NIBP_LOGGING
#endif

/** NiBP communication port */
#if defined (WIN32)
 #define NIBP_COMMPORT "\\\\.\\COM1"
#elif defined (ARM)
 #define NIBP_COMMPORT "/dev/ttyS1"
#elif defined (UNIX)
 #define NIBP_COMMPORT "/dev/ttyUSB1"
#else
 #error Specify your target OS
#endif

#define NIBP_ST_HSTART       0x3A   // ':' host start byte
#define NIBP_ST_MSTART       0x3E   // '>' module start byte

#define NIBP_ST_ERROR        'E'    // data error (received from module)

// blood-pressure module commands
#define NIBP_CMD_NONE                       0
#define NIBP_CMD_SET_INITIAL_INFLATE        1
#define NIBP_CMD_START_BP                   2
#define NIBP_CMD_START_PEDS_BP              3
#define NIBP_CMD_START_NEONATE_BP           4
#define NIBP_CMD_ABORT_BP                   5
#define NIBP_CMD_GET_CUFF_PRESSURE          6
#define NIBP_CMD_GET_BP_DATA                7
#define NIBP_CMD_GET_MODULE_DATA            8
#define NIBP_CMD_GET_RETURN_STRING          9
#define NIBP_CMD_SET_SLEEP_MODE             10
#define NIBP_CMD_CONTROL_PNEUMATICS         11
#define NIBP_CMD_CALIBRATE_TRANSDUCER       12
#define NIBP_CMD_GET_RETURN_CODE            13
#define NIBP_CMD_VENOUS_STASIS              14

#define NIBP_ERR_OK                         0
#define NIBP_ERR_SOF                        1
#define NIBP_ERR_EOF                        2
#define NIBP_ERR_CRC                        3
#define NIBP_ERR_LEN                        4
#define NIBP_ERR_TOUT                       5
#define NIBP_ERR_DATA                       6
#define NIBP_ERR_BUSY                       7

#define NIBP_STAT_ERR                       -1
#define NIBP_STAT_READY                     0
#define NIBP_STAT_BUSY                      1
#define NIBP_STAT_NOINIT                    2

#define NIBP_RESP_O                         'O'
#define NIBP_RESP_K                         'K'
#define NIBP_RESP_A                         'A'
#define NIBP_RESP_S                         'S'
#define NIBP_RESP_B                         'B'

#define NIBP_MAXRESPLEN                      200

#define NIBP_ACTIVITY_TIMEOUT                2400 // ms
#define NIBP_INACTIVITY_COUNT_LIMIT          12
#define NIBP_PROCESS_INPUT_PERIOD            200  // ms
#define NIBP_UPDATE_CUFFDATA_PERIOD          500  // ms
#define NIBP_UPDATE_DATA_PERIOD              2000 // ms

/**
 * \brief NiBP measurement interval enumeration.
 */
enum
{
  NIBP_MEAS_INTERVAL_MANU,
  NIBP_MEAS_INTERVAL_1M,
  NIBP_MEAS_INTERVAL_2M,
  NIBP_MEAS_INTERVAL_5M,
  NIBP_MEAS_INTERVAL_10M,
  NIBP_MEAS_INTERVAL_15M,
  NIBP_MEAS_INTERVAL_30M,
  NIBP_MEAS_INTERVAL_60M,
  NIBP_NUM_MEAS_INTERVALS,
};

/**
 * \brief Output NiBP debug data.
 */
void bpm_debug(char *fmt, ...);

#pragma pack(1)
typedef struct
{
  unsigned short sss;    // systolic value in mmHg
  unsigned short ddd;    // diastolic value in mmHg
  unsigned char  btc;    // bit test code
  unsigned char  bps;    // bp status
  unsigned char  tc[8];  // test codes (8 unsigned bytes)
  unsigned short rate;   // heart rate
  unsigned short map;    // mean arterial pressure
  unsigned char  ec;     // error code
  unsigned char  sp1;    // spare byte (not used)
  unsigned char  sp2;    // spare byte (not used)
} nibp_suntech_data_t;

typedef struct
{
  char bp_fw[8];             // blood pressure fw rev
  char s_fw[8];              // safety fw rev
  unsigned char pr_code[16]; // processor lot code
  unsigned char mod_id[16];  // module serial number
  char cyc[14];              // cycle counter string
  unsigned short unused;     // unused
} nibp_fwinfo_t;

typedef struct
{
  short cuff;
  nibp_suntech_data_t data;
  nibp_fwinfo_t fw_info;
} nibp_info_t;

#pragma pack()

enum
{
  OPEN = 0,
  OFF = 0,
  CLOSE = 1,
  ON = 1,
};

enum
{
  IMMEDIATE,
  AUTOSLEEP,
  NO_AUTOSLEEP,
};

typedef void (*nibp_cmd_func_t)(void * pArg);

/*! \fn int  nibp_init(char* commport)
 *  \brief Init NiBP module interface.
 *  \param commport Communication port name, e.g. '/dev/ttyS0'.
 *  \return Returns 0 on success, not zero - otherwise.
 */
int  nibp_init(char* commport);

/*! \fn void nibp_deinit(void)
 *  \brief Deinit NiBP module interface.
 */
void nibp_deinit(void);

/*! \fn void nibp_command(int ncmd, ...)
 *  \brief Sends any command from supported command list to NiBP module.
 *  \param ncmd Command.
 */
void nibp_command(int ncmd, ...);

/*! \fn void nibp_process_input(void)
 *  \brief Process incoming data from NiBP module.
 */
void nibp_process_input(void);

/*! \fn void nibp_reqdata(void)
 *  \brief Request data/response from NiBP module.
 */
void nibp_reqdata(void);

/*! \fn void nibp_get_last_data(nibp_info_t *pni)
 *  \brief Obtain last measurement data from NiBP module.
 *  \param pni Pointer to \a nibp_info_t structure to fill it in routine.
 */
void nibp_get_last_data(nibp_info_t *pni);

/*! \fn int  nibp_is_running(void)
 *  \brief Informs is any measurement in progress or not.
 *  \return Returns true if measurement is in progress, false - otherwise.
 */
int  nibp_is_running(void);

/*! \fn void nibp_do_bp(void)
 *  \brief Start pressure measurement.
 */
void nibp_do_bp(void);

/*! \fn void nibp_calib(void)
 *  \brief Start NiBP module calibration procedure.
 */
void nibp_calib(void);

/*! \fn void nibp_leak_test(void)
 *  \brief Start NiBP module leak test.
 */
void nibp_leak_test(void);

/*! \fn void nibp_verif(void)
 *  \brief Start NiBP module verification procedure.
 */
void nibp_verif(void);

/*! \fn int  nibp_get_status(void)
 *  \brief Returns status step of current procedure.
 *  \return Status.
 */
int  nibp_get_status(void);

/*! \fn void nibp_reset(void)
 *  \brief Reset NiBP module.
 */
void nibp_reset(void);

/*! \fn int nibp_calibration_stat(void)
 *  \brief Checks validation of NiBP module calibration.
 *  \return Returns code signifying validation of NiBP module calibration.
 */
int nibp_calibration_stat(void);

/*! \fn void nibp_service(int on_off)
 *  \brief Set flag, informing that NiBP module is in /out from service mode.
 *  \param on_off 0 - normal mode (measurements), non 0 - service mode (calibration etc).
 */
void nibp_service(int on_off);

/**
 *  \brief NiBP data measurements request interval.
 */
extern int nibp_meas_interval[NIBP_NUM_MEAS_INTERVALS];

#endif // __NIBP_H
