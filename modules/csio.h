/*! \file csio.h
    \brief Central station i/o.
*/

#ifndef __CSIO_H
#define __CSIO_H

#include "udp.h"

#define   CS_OUT_START      '{'
#define   CS_OUT_END        '}'

#define   CS_IN_START       '{'
#define   CS_IN_END         '}'

#define CSIO_OUTBUF_SIZE    50000

#include "csio_typedefs.h"

/*! \fn void csio_init(void)
 *  \brief Interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
void csio_init(void);

/*! \fn void csio_deinit(void)
 *  \brief Interface deinit function.
 */
void csio_deinit(void);

/*! \fn int csio_read(void * pbuf, int len)
 *  \brief Read data from central station.
 *  \param pbuf Read buffer
 *  \param len Number of bytes to be read
 *  \return Number of bytes has been actually read
 */
int csio_read(void * pbuf, int len);

/*! \fn int csio_write(const void * pbuf, int len)
 *  \brief Write data to central station.
 *  \param pbuf Write buffer
 *  \param len Number of bytes to be written
 *  \return Number of bytes has been actually written
 */
int csio_write(const void * pbuf, int len);

/*! \fn int csio_send_msg(int msgid, const void * data, int len)
 *  \brief Send message to central station with concerned protocol
 *  \param msgid Message identificator
 *  \param data Outcoming data
 *  \param len Number of bytes to be sent
 *  \return Number of bytes has been actually sent
 */
int csio_send_msg(int msgid, const void * data, int len);

/*! \fn void csio_send_slow_data(void)
 *  \brief Send slowly varying data to central station
 */
void csio_send_slow_data(void);

/*! \fn void csio_update(void)
 *  \brief Interface update function.
 */
void csio_update(void);

#endif // __CSIO_H
