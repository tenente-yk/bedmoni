/*! \file resp.h
 *  \brief Resp channel functions
 */

#ifndef __RESP_H
#define __RESP_H

//#define RESP_DEBUG
#define RESPCALC_COLIBRI

#define FD_RPG                        50

/*! \fn void resp_process_packet(unsigned char * data, int len)
 *  \brief Process data of resp parameters in case of algorithm being realized outside this application.
 */
void resp_process_packet(unsigned char * data, int len);

/*! \fn void resp_soft_hw_reset(void)
 *  \brief Reset to zero-line by using hardware HP-filters.
 */
void resp_soft_hw_reset(void);

/*! \fn void resp_add_value(short val, unsigned char break_byte)
 *  \brief Add new data of resp signal.
 *  \param val Impedance value.
 *  \param break_byte Parameter indicating ECG/RPG electrode breaks.
 */
void resp_add_value(short val, unsigned char break_byte);

#endif // __RESPCALC_H
