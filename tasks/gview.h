/*! \file gview.h
    \brief Slow graphics functions.
*/

#ifndef __GVIEW_H
#define __GVIEW_H

/*! \fn int gview_init(void)
 *  \brief Slow graphics interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
int  gview_init(void);

/*! \fn void gview_deinit(void)
 *  \brief Slow graphics interface deinit function.
 */
void gview_deinit(void);

/*! \fn void gview_update(void)
 *  \brief Slow graphics interface update function.
 */
#ifdef UNIX
void gview_update(void);
#endif
#ifdef WIN32
unsigned long gview_update(void *);
#endif

#endif // __GVIEW_H


