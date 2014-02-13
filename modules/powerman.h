/*! \file powerman.h
    \brief Power management
*/

#ifndef __POWERMAN_H
#define __POWERMAN_H

/*! \fn void powerman_stat(unsigned long val)
 *  \brief Store new configuration of the power model to app variable.
 *  \param val value, containing current power information.
 */
void powerman_stat(unsigned long val);

/*! \fn void powerman_stat_get(unsigned long * pv)
 *  \brief Retrieve configuration of the power model.
 *  \param pv pointer to variable for storing power cfg.
 */
void powerman_stat_get(unsigned long * pv);

/*! \fn void sbdp_reset(void)
 *  \brief Forces to reset SBDP board.
 */
void sbdp_reset(void);

/*! \fn void lowbat_shutdown(void)
 *  \brief Forces shutting down be caused empty battery.
 */
void lowbat_shutdown(void);

#endif // __POWERMAN_H

