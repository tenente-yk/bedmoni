/*! \file ecgm.h
    \brief ECG module fuctionality
*/

#ifndef __ECGM_H
#define __ECGM_H

//#define ECGM_CR_ZBASE_MONITORING

#if defined (LPC2478)
#define ECGM_USART USART2
#elif defined (LPC2468)
#define ECGM_USART USART3
#else
#if !defined (UNIX) && !defined (WIN32)
#error ECGM_USART not defined
#endif
#endif

#define ECGM_PACKET_SIZE  (16)
#define ECGM_DATABUF_MAXSIZE (40*ECGM_PACKET_SIZE)

#define TAU_3200   0x0
#define TAU_320    0x1
#define TAU_160    0x2

// ecgm commands range 0x00 - 0x1f
#define ECGM_N_TO_LINE       0x01
#define ECGM_N_TO_GND        0x02
#define ECGM_ECG_TAU_3200    0x03
#define ECGM_ECG_TAU_320     0x04
#define ECGM_ECG_TAU_160     0x05
#define ECGM_RESP_TAU_6800   0x06
#define ECGM_RESP_TAU_320    0x07
//#define ECGM_PM_RL           0x08
//#define ECGM_PM_RF           0x09
//#define ECGM_PM_RC           0x0a
#define ECGM_RESP_ON         0x0b
#define ECGM_RESP_OFF        0x0c
#define ECGM_DHF_ON          0x0d
#define ECGM_DHF_OFF         0x0e
#define ECGM_DATA_OFF        0x0f
#define ECGM_DATA_RESET      0x10
#define ECGM_RESP_CHAN_RL    0x11
#define ECGM_RESP_CHAN_RF    0x12
#define ECGM_DEMO_OFF        0xF0
#define ECGM_DEMO_ON         0xF1

#define ECGM_MAX_NUM_CMDS_IN_QUEUE       20

/**
 *  \brief List of ECG filtration mode.
 */
enum
{
  MODE_DIAGNOSTICS,
  MODE_DIAGNOSTICS_F,
  MODE_MONITORING,
  MODE_SURGERY,
  MAXNUM_ECG_MODES,
};

/**
 * \brief Break bits structure.
 */
typedef struct
{ // MLB
  unsigned char r:         1;    /**< R-electrode break */
  unsigned char l:         1;    /**< L-electrode break */
  unsigned char f:         1;    /**< F-electrode break */
  unsigned char c:         1;    /**< C-electrode break */
  unsigned char unused:    4;
} // MSB
break_bits_t;

/**
 *  \brief ECG module settings bits structure.
 */
typedef struct
{ // MLB
  unsigned char tau:       2;    /**< filtration mode (HW LP filter) */
  unsigned char taubr300:  1;    /**< Resp HW HP filter */
  unsigned char setzn:     1;
  unsigned char dhf:       1;
  unsigned char breath_ch: 1;    /**< Resp channel (RL or RF) */
  unsigned char scan_freq: 1;
  unsigned char diag_f:    1;    /**< 1 signifies DIAG+F mode enabled */
} // MSB
ecgmset_bits_t;

/**
 * \brief ECG pacemaker bits structure.
 */
typedef struct
{ // MLB
  unsigned char l:         2;    /**< L-channel pacemaker with sign */
  unsigned char f:         2;    /**< F-channel pacemaker with sign */
  unsigned char c:         2;    /**< C-channel pacemaker with sign */
  unsigned char unused:    2;
} // MSB
pacemaker_bits_t;

#pragma pack(1)
/**
 *   \brief ECG data frame structure.
 */
typedef struct
{
  unsigned char       sync;
  union
  {
    unsigned char     break_byte;
    break_bits_t      break_bits;
  };
  unsigned char       data[12];
  union
  {
    ecgmset_bits_t    set_bits;
    unsigned char     set;
  };
  union
  {
    pacemaker_bits_t  pacemaker_bits;
    unsigned char     pacemaker;
  };
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
ecgm_packet_t;
#pragma pack()

#if defined (UNIX) || defined (WIN32)
/*! \fn void ecgm_command_queue_init(void)
 *  \brief Init ECG module commands queue.
 */
void ecgm_command_queue_init(void);

/*! \fn void ecgm_command(unsigned char cmd)
 *  \brief Send command to ECG module.
 *  \param cmd Command
 */
void ecgm_command(unsigned char cmd);

/*! \fn void ecgm_process_packet(unsigned char *data, int len)
 *  \brief Process ECG data frame.
 *  \param data Array containing data frame
 *  \param len Number of bytes in \a data array
 */
void ecgm_process_packet(unsigned char *data, int len);

/*! \fn void ecg_on_no_qrs(void)
 *  \brief Call-back function, calls when no QRS for a determinated interval - asystolia.
 */
void ecg_on_no_qrs(void);
#endif

#if defined (LPC2468) || defined (LPC2478)
int  ecgm_test_message(unsigned char * buf);

void ecgm_proc_data(unsigned char * buf, int len);

void ecgm_task(void * pvParameters);
#endif

/*! \fn int chno_is_ecg(int chno)
 *  \brief Tests is the mentioned channel id is an id of ECG channel/lead.
 *  \param chno channel identificator
 *  \return Returns true if \a chno is ECG channel id, false - otherwise
 */
int chno_is_ecg(int chno);

/*! \param
    \brief String identificators of ECG mode names.
*/
extern unsigned short ecg_mode_ids[MAXNUM_ECG_MODES];

#endif // __ECGM_H
