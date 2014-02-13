/*! \file sched.h
    \brief Software scheduler.
*/

#ifndef __SCHED_H
#define __SCHED_H

#define SCHED_NORMAL        0x0
#define SCHED_LOOPED        0x0
#define SCHED_DO_ONCE       0x1

/*! \enum
 * Scheduler tasks identificators.
 */
enum
{
  SCHED_ANY, /*!< launches task with any vacant common id */ // launch sched task with any id (use it only for SCHED_ONCE tasks)
  SCHED_NIBP,
  SCHED_ALARMS,
  SCHED_IFRAME_MSGS,
  SCHED_CRBP_RESP_STAT,
  SCHED_NIBP_INPUT,
  SCHED_NIBP_REQDATA,
  SCHED_QRS_WAIT,
  SCHED_TPRN,
  SCHED_RESP,
  SCHED_PM_NORESP,
  SCHED_SBDP,
  SCHED_ECGM,
  SCHED_HEARTBEAT,
  SCHED_PMBEAT,
  SCHED_EP_WAIT,
 // SCHED_RESP_RST,
  SCHED_MISC1, /*!< common id */
  SCHED_MISC2, /*!< common id */
  SCHED_MISC3, /*!< common id */
  SCHED_MISC4, /*!< common id */
  SCHED_MISC5, /*!< common id */
  SCHED_MISC6, /*!< common id */
  SCHED_MISC7, /*!< common id */
  SCHED_MISC8, /*!< common id */
  SCHED_MISC_LAST = SCHED_MISC8,
  SCHED_CS_SLOW,
  SCHED_LOWBAT_SHUTDOWN,
  SCHED_NUM_TASKS,
};

typedef void (*SCHEDPROC)(void);

/*! \struct dl_t
 * scheduler task internal info.
 */
typedef struct
{
  int           active;
  int           id;
  unsigned long dly_ms;
  unsigned long t0;
  unsigned long flags;
  SCHEDPROC     func;
} sched_task_t;

//int sched_task(void * arg);

/*! \fn int sched_init(void)
 *  \brief Scheduler interface init function.
 *  \return 0 - if okay, -1 otherwise.
 */
int  sched_init(void);

/*! \fn void sched_deinit(void)
 *  \brief Scheduler interface deinit function.
 */
void sched_deinit(void);

/*! \fn void sched_reinit(void)
 *  \brief Scheduler interface reinit function.
 */
void sched_reinit(void);

/*! \fn void sched_update(void)
 *  \brief Scheduler interface update function.
 */
#ifdef UNIX
void sched_update(void);
#endif
#ifdef WIN32
unsigned long sched_update(void *);
#endif

/*! \fn void sched_start(int id, int reload_time_ms, SCHEDPROC proc, unsigned long flags)
 *  \brief Add new task to scheduler.
 *  \param id task id.
 *  \param reload_time_ms reload time in milliseconds.
 *  \param proc call this routine after \a reload_time_ms milliseconds.
 *  \param flags options.
 *     SCHED_DO_ONCE - call only once proc routine
 */
void sched_start(int id, int reload_time_ms, SCHEDPROC proc, unsigned long flags);

/*! \fn void sched_stop(int id)
 *  \brief Remove task from scheduler.
 *  \param id task id.
 */
void sched_stop(int id);

/*! \fn int sched_is_started(int id)
 *  \brief Check has the task already started.
 *  \param id task id.
 *  \return 1 - task is running, 0 - task is not running.
 */
int  sched_is_started(int id);

/*! \fn void sched_exec(int id)
 *  \brief Exucute <i>proc</i> routine associated with <i>id</i>
 *  \param id task id.
 */
void sched_exec(int id);

/*! \fn void sched_suspend(void)
 *  \brief Susdend scheduler
 */
void sched_suspend(void);

/*! \fn void sched_resume(void)
 *  \brief Resume scheduler
 */
void sched_resume(void);

#endif // __SCHED_H
