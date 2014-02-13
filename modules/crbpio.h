/*! \file crbpio.h
    \brief CRBP board i/o functions.
*/

#ifndef __CRBPIO_H
#define __CRBPIO_H

#include "bedmoni.h"

// #ifndef ARM
//#define CRBPIO_DEBUG
// #define CRBPIO_LOGGING
// #endif

//#define CRBPIO_OUT_DIRECTLY

#ifndef CRBPIO_DEBUG
#undef CRBPIO_LOGGING
#endif

#ifdef UNIX
#ifdef ARM
#define   CRBP_COMMPORT       "/dev/ttyS2"
#else
#define   CRBP_COMMPORT       "/dev/ttyUSB0"
#endif
#endif // UNIX
#ifdef WIN32
#define   CRBP_COMMPORT       "\\\\.\\COM1"
#endif

#if !defined (ARM)
#define FEATURE_KEYBOARD
#endif

#define CRBPIN_BUF_SIZE       100
#define CRBP_MSG_SIZE         8

#if !defined (CRBPIO_OUT_DIRECTLY)
#define CRBPOUT_BUF_SIZE      200
#endif

#define CRBP_MSG_SOF          0x81
#define CRBPIN_MSG_ID         0x88

#define CRBPOUT_MSG_ID        0x08

// CRBP -> Colibri messages id
#define CRBP_SYS_STAT         0x81
#define CRBP_KBD_STAT         0x82
//#define CRBP_BAT_STAT         0x83
#define CRBP_VOL_STAT         0x84
#define CRBP_REQ_QUERY        0x85
#define CRBP_POWEROFF_QUERY   0x86
#define CRBP_LIGHT_STAT       0x87
#define CRBP_ERR_STAT         0x88
#define CRBP_TIME_STAT        0x89
#define CRBP_DATE_STAT        0x8B
#define CRBP_ALARM_RESP       0x8E

// Colibri->CRBP messages id
#define CRBP_SET_POWER        0x01
#define CRBP_SET_BRIGHT       0x03
#define CRBP_RESP_STAT        0x05
#define CRBP_POWEROFF_OK      0x06
#define CRBP_TIME_QUERY       0x09
#define CRBP_DATE_QUERY       0x0B
#define CRBP_SOUND_ALARM      0x0E

// keyboard buttons' masks
#define CRBP_KBD_PWR_BMASK    0x01
#define CRBP_KBD_MSCR_BMASK   0x02
#define CRBP_KBD_FRZ_BMASK    0x04
#define CRBP_KBD_SLS_BMASK    0x08
#define CRBP_KBD_PRN_BMASK    0x10
#define CRBP_KBD_BPM_BMASK    0x20
#define CRBP_KBD_MENU_BMASK   0x40
#define CRBP_KBD_SEL_BMASK    0x80

// devices' masks
#define DEV_MASK_BAT          0x01
#define DEV_MASK_AUDIO        0x02
#define DEV_MASK_USBHUB       0x04
#define DEV_MASK_FTDI         0x08
#define DEV_MASK_NIBP         0x10
#define DEV_MASK_SBDP         0x20
#define DEV_MASK_TPRN         0x40
#define DEV_MASK_MAINS        0x80

/**
 * \brief Byte structure, containing periphery power status bits.
 */
typedef struct
{
  unsigned char   bat : 1;
} power_bits_t;

/**
 * \brief Byte structure, containing periphery power status bits.
 */
typedef struct
{
  union
  {
    power_bits_t   power_bits;
    unsigned char  power;
  };
} power_dev_info_t;

/*! \fn int crbpio_init(void)
 *  \brief CRBP i/o interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
int  crbpio_init(void);

/*! \fn void crbpio_deinit(void)
 *  \brief CRBP i/o interface deinit function.
 */
void crbpio_deinit(void);

/*! \fn void crbpio_update(void)
 *  \brief CRBP i/o interface update function.
 */
#ifdef UNIX
void crbpio_update(void);
#endif
#ifdef WIN32
unsigned long crbpio_update(void *);
#endif

/*! \fn void crbpio_send_msg(unsigned char id, unsigned char b3, unsigned char b2, unsigned char b1, unsigned char b0)
 *  \brief Send message to CRBP board.
 *  \param id Message ID.
 *  \param b3 Message byte b3.
 *  \param b2 Message byte b2.
 *  \param b1 Message byte b1.
 *  \param b0 Message byte b0.
 */
void crbpio_send_msg(unsigned char id, unsigned char b3, unsigned char b2, unsigned char b1, unsigned char b0);

/*! \fn void crbp_resp_stat(void)
 *  \brief Send alive status massage to CRBP board to prevent CRBP watchdog reset.
 */
void crbp_resp_stat(void);

#endif // __CRBPIO_H
