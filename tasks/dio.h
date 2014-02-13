/*! \file dio.h
    \brief SBDP board and other fast periphery i/o functions.
*/

#ifndef __DIO_H
#define __DIO_H

#include "spo2.h"
#include "ecgm.h"
#include "co2.h"

#define DIO_DELAY_MS           10

#define DATAOUT_MAXSIZE        300

#define DOUT_START            'V'
#define DOUT_END              'W'

#define DIN_START             'V'
#define DIN_END               'W'

#define SBDP_RST_MOD          0xA0
#define SBDP_OFF_MOD          0xAE
#define SBDP_QUERY_VER        0xBB

/*! \enum
 *  \brief SBDP modules identificators list.
 */
enum
{
  PD_ID_NONE = 0x0000,    /*!< 0 */
  PD_ID_ECG = 0x0001,     /*!< 1 */
  PD_ID_CO2,              /*!< 2 */
  PD_ID_T1T2,             /*!< 3 */
  PD_ID_SPO2,             /*!< 4 */
  PD_ID_ED,               /*!< 5 */
  PD_ID_RPEAK,            /*!< 6 */
  PD_ID_ECS,              /*!< 7 */
  PD_ID_RESP,             /*!< 8 */
  NUM_PD_ID,              /*!< 9 */
};

#define PD_RESERVED            0x00

#ifdef UNIX
#define DEV                    "/dev/ttyACM0"
#endif
#ifdef WIN32
#define DEV                    "\\\\.\\COM4"
#endif

#define DIO_FIFO_SIZE          10000
#define DIO_TXFIFO_SIZE        500

#define THERME_PACKET_SIZE     (8)

/*! \fn int dio_init(void)
 *  \brief SBDP i/o interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
int  dio_init(void);

/*! \fn void dio_deinit(void)
 *  \brief SBDP i/o interface deinit function.
 */
void dio_deinit(void);

/*! \fn void dio_module_cmd(unsigned short id, unsigned char cmd, ...)
 *  \brief Send message to SBDP board.
 *  \param id Module identificator.
 *  \param id Command.
 *  \param ... Arguments.
 */
void dio_module_cmd(unsigned short id, unsigned char cmd, ...);

/*! \fn void dio_suspend(void)
 *  \brief Suspend SBDP i/o.
 */
void dio_suspend(void);

/*! \fn void dio_resume(void)
 *  \brief Resume SBDP i/o.
 */
void dio_resume(void);

/*! \fn void dio_update(void)
 *  \brief SBDP i/o interface update function.
 */
#ifdef UNIX
void dio_update(void);
#endif
#ifdef WIN32
unsigned long dio_update(void *);
#endif

extern char sbdp_version[10];

#endif // DIO_H
