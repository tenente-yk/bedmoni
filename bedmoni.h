/*! \file bedmoni.h
    \brief General header file
*/

#ifndef __BEDMONI_H
#define __BEDMONI_H

/*!
 *  \brief     bedmoni application.
 *  \details   This is general header file.
 *  \author    Yuriy Kirpichenko
 *  \version   1.0
 *  \date      2011-2012
 *  \pre       compile with gcc.
 *  \bug       no bugs.
 */

// bedmoni
// developed by Y.Kirpichenko ( mailto: insaziable@list.ru )

#include <stdio.h>
#include "defs.h"
#include "ids.h"
#include "lang.h"

#include "autogen_config.h"

#define HI_VER                1
#define LO_VER                0

#define STDIO_LOGGING         0
#define DEBUG                 1
#define MAKE_FB_NULL          // framebuffer is not presented

#ifdef ARM
#define USBDISK_DEV           "/dev/sda1"
#else
#define USBDISK_DEV           "/dev/sdb1"
#endif
#ifdef ARM
#define USBDISK_PATH          "/mnt/usbdisk/"
#define USBDISK_BINPATH       "/mnt/usbdisk/bin/"
#else
#define USBDISK_PATH          BEDMONI_DIR
#define USBDISK_BINPATH       BEDMONI_DIR"/bin/"
#endif
#define APP_NAME              "bedmoni"
#define DATA_DIR              "bedmoni_data"
#define DEMOFILES_DIR         "demofiles"

#define EXIT_OK               55
#define EXIT_UPDATED          33
#define EXIT_RESTART          22
#define EXIT_LOWBAT           44

#define SFRAME_BMONI_TS
//#define TEST_IDSTEXT_MODE
#ifdef ARM
#undef TEST_IDSTEXT_MODE
#endif

#if 1

#define HP_UPD_DELAY_MS       15
#define NP_UPD_DELAY_MS       30
#define LP_UPD_DELAY_MS       50

/*! \enum
 * Runtime tasks priorities.
 */
enum
{
  PRIOR_LOW,
  PRIOR_NORM,
  PRIOR_HIGH,
  NUM_UPD_TASKS
};

#ifdef UNIX
typedef void (*UPDPROC)(void);
#endif
#ifdef WIN32
typedef unsigned long (*UPDPROC)(void*);
#endif
typedef int  (*INIPROC)(void);
typedef void (*DNIPROC)(void);

typedef int (*UPDTASKPROC)(void*);

/*! \struct dl_t
 * task information.
 */
typedef struct
{
  char     name[12]; ///<- task name.
  UPDPROC  updproc; ///<- task step update.
  INIPROC  iniproc; ///<- init routine.
  DNIPROC  dniproc; ///<- deinit routine.
  int      priority; ///<- task priority.
} task_info_t;

void sighandler(int sig);

/*! \fn void safe_exit(int exit_code)
 *  \brief Safe exit application with exit code \a exit_code.
 *  \param exit_code Exit status.
 */
void safe_exit(int exit_code);

int ext_task(void* arg);

void yfprintf(FILE *stream, char *fmt, ...);
void yprintf(char *fmt, ...);

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

/*! \fn void error(char *fmt, ...)
 *  \brief Print formatted error message to console.
 *  \param fmt Format control.
 *  \param ... Optional arguments.
 */
void error(char *fmt, ...);

#ifdef UNIX
#define DBG(msg, args...) do {              \
  debug("[%s] " msg, __FUNCTION__, ##args); \
 } while (0);
#endif

extern volatile int exit_flag;
extern int demo_mode;
extern int admin_mode;
extern int time_changed;
extern int exit_code;
#endif

/*! \var int exit_code
 *  \brief Contains exit code of application
 */

#define SERIAL_NO_UNINIT          0xffffffff
extern int serial_no;

#endif // BEDMONI_H
