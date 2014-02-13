#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "bedmoni.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "tr.h"
#include "sched.h"
#include "resp.h"
#include "respcalc.h"
#include "cframe.h"
#include "mainmenu.h"
#include "datetime.h"

static void dt_none_func(void*);
static void dt_day_func(void*);
static void dt_mon_func(void*);
static void dt_year_func(void*);
static void dt_hour_func(void*);
static void dt_min_func(void*);
static void dt_sec_func(void*);
static void dt_exit_func(void*);
static void dt_set_func(void*);

inpfoc_funclist_t inpfoc_dt_funclist[DT_NUM_ITEMS+1] = 
{
  { DT_NONE,        dt_none_func         },
  { DT_DAY,         dt_day_func          },
  { DT_MON,         dt_mon_func          },
  { DT_YEAR,        dt_year_func         },
  { DT_HOUR,        dt_hour_func         },
  { DT_MIN,         dt_min_func          },
  { DT_SEC,         dt_sec_func          },
  { DT_SET,         dt_set_func          },
  { DT_EXIT,        dt_exit_func         },
  { -1            , dt_none_func         }, // -1 must be last
};

static void dt_none_func(void * parg)
{

}

static void dt_day_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  char s[10];

  pit = inpfoc_find_active(INPFOC_DT);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &dd);
  if (pcmd->delta > 0)
    if (++dd > 31) dd = 1;
  if (pcmd->delta < 0)
    if (--dd < 1) dd = 31;
  snprintf(s, sizeof(s), "%02d", dd);

  inpfoc_wnd_setcaption(pit, s);
}

static void dt_mon_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  char s[10];

  pit = inpfoc_find_active(INPFOC_DT);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &dd);
  if (pcmd->delta > 0)
    if (++dd > 12) dd = 1;
  if (pcmd->delta < 0)
    if (--dd < 1) dd = 12;
  snprintf(s, sizeof(s), "%02d", dd);

  inpfoc_wnd_setcaption(pit, s);
}

static void dt_year_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  char s[10];

  pit = inpfoc_find_active(INPFOC_DT);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%04d", &dd);
  if (pcmd->delta > 0)
    if (++dd > 2027) dd = 2011;
  if (pcmd->delta < 0)
    if (--dd < 2011) dd = 2027;
  snprintf(s, sizeof(s), "%04d", dd);

  inpfoc_wnd_setcaption(pit, s);
}

static void dt_hour_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  char s[10];

  pit = inpfoc_find_active(INPFOC_DT);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &dd);
  if (pcmd->delta > 0)
    if (++dd > 23) dd = 0;
  if (pcmd->delta < 0)
    if (--dd < 0) dd = 23;
  snprintf(s, sizeof(s), "%02d", dd);

  inpfoc_wnd_setcaption(pit, s);
}

static void dt_min_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  char s[10];

  pit = inpfoc_find_active(INPFOC_DT);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &dd);
  if (pcmd->delta > 0)
    if (++dd > 59) dd = 0;
  if (pcmd->delta < 0)
    if (--dd < 0) dd = 59;
  snprintf(s, sizeof(s), "%02d", dd);

  inpfoc_wnd_setcaption(pit, s);

}

static void dt_sec_func(void * parg)
{
#if 0
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int dd;
  char s[10];

  pit = inpfoc_find_active(INPFOC_DT);
  pifw = (inpfoc_wnd_t*) pit->parg;
  printf("s: %s\n", pifw->caption);
  sscanf(pifw->caption, "%02d", &dd);
  if (pcmd->delta > 0)
    if (++dd > 59) dd = 0;
  if (pcmd->delta < 0)
    if (--dd < 0) dd = 59;
  snprintf(s, sizeof(s), "%02d", dd);

  inpfoc_wnd_setcaption(pit, s);
#endif
  return;
}

static void dt_set_func(void * parg)
{
  char s[20];
  time_t t;
  struct tm *ptm;
  struct timeval tv;

  time(&t);
  ptm = localtime(&t);

  inpfoc_wnd_getcaption(inpfoc_find(INPFOC_DT, DT_DAY), s, sizeof(s));
  ptm->tm_mday = atoi(s);
  inpfoc_wnd_getcaption(inpfoc_find(INPFOC_DT, DT_MON), s, sizeof(s));
  ptm->tm_mon = atoi(s) - 1;
  inpfoc_wnd_getcaption(inpfoc_find(INPFOC_DT, DT_YEAR), s, sizeof(s));
  ptm->tm_year = atoi(s) - 1900;
  inpfoc_wnd_getcaption(inpfoc_find(INPFOC_DT, DT_HOUR), s, sizeof(s));
  ptm->tm_hour = atoi(s);
  inpfoc_wnd_getcaption(inpfoc_find(INPFOC_DT, DT_MIN), s, sizeof(s));
  ptm->tm_min = atoi(s);
  inpfoc_wnd_getcaption(inpfoc_find(INPFOC_DT, DT_SEC), s, sizeof(s));
  ptm->tm_sec = atoi(s);

  debug("date/time set: %02d.%02d.%04d %02d:%02d.%02d\n", ptm->tm_mday, ptm->tm_mon+1, +ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

  sched_suspend();
  time_changed = 2;
  // sched_resume will be called in alarm sig handler

  t = mktime(ptm);
  tv.tv_sec = t;
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);

  // store to rtc onboard chip
  int ret = system("hwclock -w");
  if ( WEXITSTATUS(ret) != 0)
  {
    error("hwclock -w error\n");
  }

  // TODO: add notification about trends' removal

#if defined (RESPCALC_COLIBRI)
  respcalc_reset();
#endif

  tr_reset();

  // call alarm sig handler to update date/time in the sframe
  alarm(1);
 // sighandler(SIGALRM);
}

static void dt_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
}

void menu_datetime_openproc(void)
{
  time_t t;
  struct tm *ptm;
  char s[10];

  time(&t);
  ptm = localtime(&t);

  sprintf(s, "%02d", ptm->tm_mday);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DT, DT_DAY), s);
  sprintf(s, "%02d", ptm->tm_mon+1);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DT, DT_MON), s);
  sprintf(s, "%04d", ptm->tm_year+1900);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DT, DT_YEAR), s);
  sprintf(s, "%02d", ptm->tm_hour);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DT, DT_HOUR), s);
  sprintf(s, "%02d", ptm->tm_min);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DT, DT_MIN), s);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_DT, DT_SEC), "--");

  inpfoc_set(INPFOC_DT, DT_EXIT);
}

