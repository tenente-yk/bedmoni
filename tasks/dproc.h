/*! \file dproc.h
    \brief Data processing functions.
*/

#ifndef __DPROC_H
#define __DPROC_H

#include "tr.h"

#define    DPROC_DELAY_US           (20*1000) // 20 ms
#define    DPROC_DELAY_MS           (20)

/*! \fn int dproc_init(void)
 *  \brief Data processing interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
int  dproc_init(void);

/*! \fn void dproc_deinit(void)
 *  \brief Data processing interface deinit function.
 */
void dproc_deinit(void);

/*! \fn void dproc_add_frame(unsigned int id, unsigned int ts, int len, unsigned char *pp)
 *  \brief Process incoming SBDP message.
 *  \param id SBDP module identificator.
 *  \param ts Timestamp (not used).
 *  \param len Message length.
 *  \param pp Message data.
 */
void dproc_add_frame(unsigned int id, unsigned int ts, int len, unsigned char *pp);

/*! \fn void dproc_check_frame(unsigned int id, unsigned int ts, int len, unsigned char *pp)
 *  \brief Process incoming SBDP message.
 *  \param id SBDP module identificator.
 *  \param ts Timestamp (not used).
 *  \param len Message length.
 *  \param pp Message data.
 */
void dproc_check_frame(unsigned int id, unsigned int ts, int len, unsigned char *pp);

/*! \fn void dproc_add_trv(int chno, int param, int value, unsigned char st)
 *  \brief Add measurement to trends.
 *  \param chno Module ID.
 *  \param param Parameter, to be added to trends.
 *  \param value Value, to be added to trends.
 *  \param st Flags.
 */
void dproc_add_trv(int chno, int param, int value, unsigned char st);

/*! \fn void dproc_process_tr(void)
 *  \brief Process all incoming trend data during time quant to logs.
 */
void dproc_process_tr(void);

/*! \fn void dproc_retrieve_sno(void)
 *  \brief Retrieve SBDP board serial number.
 */
void dproc_retrieve_sno(void);

/*! \fn void dproc_update(void)
 *  \brief Data processing interface update function.
 */
#ifdef UNIX
void dproc_update(void);
#endif
#ifdef WIN32
unsigned long dproc_update(void *);
#endif

#endif // __DPROC_H
