#ifndef __DEBUG_H
#define __DEBUG_H

#define DEBUG_WINDOW
//#define DEBUG_PROMPT
#define DEBUG_FILE

#ifdef __cplusplus
extern "C" {
#endif

void debug_init(void);

/*! \fn void info(char *fmt, ...)
 *  \brief Print formatted information message to console.
 *  \param fmt Format control.
 *  \param ... Optional arguments.
 */
void info(char *fmt, ...);

/*! \fn void debug(char *fmt, ...)
 *  \brief Print formatted debug message to console.
 *  \param fmt Format control.
 *  \param ... Optional arguments.
 */
void debug(char *fmt, ...);

/*! \fn void debug_ex(char *prefix, char *fmt, ...)
 *  \brief Print formatted debug message to console.
 *  \param prefix Prefix.
 *  \param fmt Format control.
 *  \param ... Optional arguments.
 */
void debug_ex(char *prefix, char *fmt, ...);

/*! \fn void error(char *fmt, ...)
 *  \brief Print formatted error message to console.
 *  \param fmt Format control.
 *  \param ... Optional arguments.
 */
void error(char *fmt, ...);


#ifdef __cplusplus
}
#endif

#endif // __DEBUG_H
