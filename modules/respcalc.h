/*! \file respcalc.h
 *  \brief Resp rate and apnoe detection algorithm
 */

#ifndef __RESPCALC_H
#define __RESPCALC_H

#include "resp.h"

#define RESPCALC_PROC_INTERVAL        (4000) //(80*FD_RPG) - 80 seconds
#define RESPCALC_PROC_HALF_INTERVAL   (RESPCALC_PROC_INTERVAL / 2)
#define RESPCALC_MINCALC_INTERVAL     (500)  //(10*FD_RPG) - 10 seconds

//#define RESPCALC_CUBIC_SPLINE

/*! \fn void respcalc_init(void)
 *  \brief Init resp parameters calculation algorithm.
 */
void respcalc_init(void);

/*! \fn void respcalc_deinit(void)
 *  \brief Deinit resp parameters calculation algorithm.
 */
void respcalc_deinit(void);

/*! \fn void respcalc_reset(void)
 *  \brief Reset resp parameters calculation algorithm.
 */
void respcalc_reset(void);

/*! \fn void respcalc_pulse_beep(void)
 *  \brief Smooth impedance signal every pulse beep.
 */
void respcalc_pulse_beep(void);

/*! \fn void respcalc_add_value(short val , unsigned char break_byte, unsigned char lead)
 *  \brief Add new impedance value in processing.
 *  \param value Impedance value.
 *  \param break_byte Parameter indicating ECG/RPG electrode breaks.
 *  \param lead Lead on which RPG is registrating (RL of RF).
 */
void respcalc_add_value(short val , unsigned char break_byte, unsigned char lead);

#endif // __RESPCALC_H
