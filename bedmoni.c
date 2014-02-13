/*! \file bedmoni.c
    \brief startup source file
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#ifdef UNIX
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/mount.h>
#endif
#ifdef WIN32
#include <direct.h>
#include <windows.h>
#endif

#include "port.h"
#include "bedmoni.h"
#include "dview.h"
#include "dio.h"
#include "sched.h"
#include "dproc.h"
#include "perio.h"
#include "crbpio.h"
#include "sched.h"
#include "sframe.h"
#include "utils.h"
#include "gview.h"
#include "stconf.h"

#ifdef UNIX
pthread_t tid[NUM_UPD_TASKS];
static pthread_attr_t tattr;
static struct sigaction sa;
#endif
#ifdef WIN32
static DWORD hThread[NUM_UPD_TASKS];
#endif

/*! \var
 *  \brief exit flag, sets to 1 to force quit application.
 */
volatile int exit_flag = 0;
/*! \var
 *  \brief exit code.
 */
int exit_code = EXIT_OK;
static volatile int safe_exit_done = 0;

int time_changed = 0;
/*! \var
 *  \brief SBDP board serial number.
 */
int serial_no = SERIAL_NO_UNINIT;

#if STDIO_LOGGING
#define STDOUT_LOG_FNAME        "./1.log"
#define STDERR_LOG_FNAME        "./2.log"
FILE *flogout = NULL, *flogerr = NULL;
static int stdio_logging = 0; // as default, logging disabled
#endif

task_info_t known_task_table[ ] = 
{
  {"DIO",   dio_update,    dio_init,    dio_deinit,     PRIOR_HIGH},
  {"DPROC", dproc_update,  dproc_init,  dproc_deinit,   PRIOR_NORM},
  {"DVIEW", dview_update,  dview_init,  dview_deinit,   PRIOR_NORM},
  {"PERIO", perio_update,  perio_init,  perio_deinit,   PRIOR_NORM},
  {"GVIEW", gview_update,  gview_init,  gview_deinit,   PRIOR_LOW},
  {"SCHED", sched_update,  sched_init,  sched_deinit,   PRIOR_LOW},
  {"DVFAST",dview_update_fast, NULL,    NULL,           PRIOR_HIGH},
};

static int prior_low_task(void * pArg);
static int prior_norm_task(void * pArg);
static int prior_high_task(void * pArg);

UPDTASKPROC upd_task[NUM_UPD_TASKS] = {prior_low_task, prior_norm_task, prior_high_task};

void debug(char *fmt, ...);

int demo_mode = 0;
int admin_mode = 0;

#ifdef WIN32
static HANDLE ystderr, ystdout, ystdin;
#endif
void yfprintf(FILE *stream, char *fmt, ...)
{
#ifdef WIN32
  char s[255];
#endif
  va_list  args;
  va_start(args, fmt);
#ifdef UNIX
  vfprintf(stream, fmt, args);
#if STDIO_LOGGING
  if (stream == stdout && flogout)
  {
    vfprintf(flogout, fmt, args);
    fflush(flogout);
  }
  if (stream == stderr && flogerr)
  {
    vfprintf(flogerr, fmt, args);
    fflush(flogerr);
  }
#endif
#endif
#ifdef WIN32
  if (stream == stdout)
  {
    vsprintf(s, fmt, args);
    WriteConsole(ystdout, s, strlen(s), NULL, NULL);
  }
  if (stream == stderr)
  {
    vsprintf(s, fmt, args);
    WriteConsole(ystderr, s, strlen(s), NULL, NULL);
  }
#endif
  va_end( args );
}

void debug(char *fmt, ...)
{
#if DEBUG
  char s[200];
  va_list  args;

  strcpy(s, "debug: ");
  va_start(args, fmt);
  vsnprintf(s+sizeof("debug: ")-1, sizeof(s)-sizeof("debug: "), fmt, args);
  va_end( args );
#ifdef UNIX
  fprintf(stdout, s);
#else
  yfprintf(stdout, s);
#endif
#endif
}

void info(char *fmt, ...)
{
#if DEBUG
  char s[200];
  va_list  args;

  strcpy(s, "info:  ");
  va_start(args, fmt);
  vsnprintf(s+sizeof("info:  ")-1, sizeof(s)-sizeof("info:  "), fmt, args);
  va_end( args );
#ifdef UNIX
  fprintf(stdout, s);
#else
  yfprintf(stdout, s);
#endif
#endif
}

void error(char *fmt, ...)
{
#if DEBUG
  char s[200];
  va_list  args;

  strcpy(s, "error: ");
  va_start(args, fmt);
  vsnprintf(s+sizeof("error: ")-1, sizeof(s)-sizeof("error: "), fmt, args);
  va_end( args );
#ifdef UNIX
  fprintf(stderr, s);
#else
  yfprintf(stderr, s);
#endif
#endif
}

void yprintf(char *fmt, ...)
{
#ifdef WIN32
  char s[255];
#endif
  va_list  args;
  va_start(args, fmt);
#ifdef UNIX
  vfprintf(stdout, fmt, args);
#if STDIO_LOGGING
  if (flogout)
  {
    vfprintf(flogout, fmt, args);
    fflush(flogout);
  }
#endif
#endif
#ifdef WIN32
  vsprintf(s, fmt, args);
  WriteConsole(ystdout, s, strlen(s), NULL, NULL);
#endif
  va_end( args );
}

void safe_exit(int ec)
{
  int i;
  int all_threads_complete = 1;

  exit_flag = 1;

  do
  {
    all_threads_complete = 1;
    for (i=0; i<NUM_UPD_TASKS; i++)
    {
#ifdef WIN32
       unsigned long res[NUM_UPD_TASKS];
       if (hThread[i]) GetExitCodeThread((void*)hThread[i], &res[i]);
       all_threads_complete &= (res[i] != STILL_ACTIVE);
#endif
#ifdef UNIX
       if (tid[i]) pthread_join(tid[i], 0);   // wait for end of thread if it was started
#endif
    }
#ifdef WIN32
    Sleep(1000);
#endif
#ifdef UNIX
    usleep(1000*1000);
#endif
  } while( !all_threads_complete );

  exit_code = ec;
  safe_exit_done = 1;
}

void sighandler(int sig)
{
  struct tm ltm;

  switch (sig)
  {
    case SIGINT:
      yprintf("Terminating...\n");
      safe_exit(-1);
      break;
#ifdef UNIX
    case SIGALRM:
      if (exit_flag) break;
      read_hwclock_data(&ltm);
      if (time_changed)
      {
        sched_resume();
      }
      debug("T %02d:%02d\n", ltm.tm_hour, ltm.tm_min);
      sframe_command(SFRAME_SET_DATE, (struct tm*) &ltm);
      sframe_command(SFRAME_SET_TIME, (struct tm*) &ltm);
      dproc_process_tr();
      if (time_changed == 2)
      {
        // do not trig alarm call in order to prevent clone SIGALRM events when time is changed
        time_changed --;
        break;
      }
      if (time_changed)
        alarm(60 - ltm.tm_sec + 1);  // for time update
      else
        alarm(60); // call this updater since 60 seconds
      time_changed = 0;
      break;
#endif
    default:
      yprintf("SIGNAL %d\n", sig);
      break;
  }
}

static void start_upd_tasks(void)
{
  int i;
  // start upd tasks
#ifdef UNIX
  assert( pthread_attr_init(&tattr) == 0 );
#endif
  for (i=0; i<NUM_UPD_TASKS; i++)
  {
#ifdef UNIX
    assert (pthread_create(&tid[i], &tattr, (void*)(upd_task[i]), NULL) == 0 );
#endif
#ifdef WIN32
    CreateThread(NULL, (SIZE_T)NULL, (void*)(upd_task[i]), NULL, (DWORD)NULL, NULL);
#endif
    if (known_task_table[i].priority == PRIOR_LOW)
    {
#ifdef UNIX
      setpriority(PRIO_PROCESS, tid[i], -10);
#endif
    }
    if (known_task_table[i].priority == PRIOR_HIGH)
    {
#ifdef UNIX
      setpriority(PRIO_PROCESS, tid[i], 10);
#endif
    }
#ifdef UNIX
    pthread_attr_destroy(&tattr);
#endif
  }
#ifdef WIN32
//#error setpriority(PRIO_PROCESS, tid[DIO_TASK], 1);
#endif
}

int main(int argc, char * argv[])
{
  int i;
#ifdef UNIX
  int pid;
#endif
  int daemon_mode = 0;
  char cmd[200];
  struct tm ltm;

#ifdef WIN32
  AllocConsole();
  ystdin = GetStdHandle(STD_INPUT_HANDLE);
  ystdout = GetStdHandle(STD_ERROR_HANDLE);
  ystderr = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

  yfprintf(stdout, "bedmoni v. %d.%d, release %s %s\n", HI_VER, LO_VER, __DATE__, __TIME__);

  demo_mode = 0;
  admin_mode = 0;

  for (i=0; i<argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch(argv[i][1])
      {
        default:
          break;
      }
      if (strcmp(argv[i], "--demo") == 0)
      {
        yprintf("DEMO MODE\n");
        demo_mode = 1;
      }
      if (strcmp(argv[i], "--daemon") == 0)
      {
        yprintf("DAEMON MODE\n");
        daemon_mode = 1;
      }
//       if (strcmp(argv[i], "--clean") == 0)
//       {
//         yprintf("CLEAN TRENDS\n");
//         clean_trends = 1;
//       }
#if STDIO_LOGGING
      if (strcmp(argv[i], "--log") == 0)
      {
        yprintf("LOGGING\n");
        stdio_logging = 1;
      }
#endif
    }
  }

#if 0 //def ARM
  sprintf(cmd, "umount %s\n", USBDISK_DEV);
  system(cmd);

  sprintf(cmd, "mount -t ext2 %s %s\n", USBDISK_DEV, USBDISK_PATH);
  system(cmd);

  if (clean_trends)
  {
    sprintf(cmd, "rm -R %s%s\n", USBDISK_PATH, "/trends.dat");
    system(cmd);
  }

  sprintf(cmd, "sync\n");
  system(cmd);

  usleep(10000000);
#endif

#if STDIO_LOGGING
  flogout = fopen(STDOUT_LOG_FNAME ,"wt");
  flogerr = fopen(STDERR_LOG_FNAME ,"wt");
#endif

#ifdef ARM
 // daemon_mode = 1;
#endif

#ifdef UNIX
  if (daemon_mode)
  {
    umask(0);
    setsid();

    pid = fork();
    if (pid > 0)
    {
      exit(0);
    }
    if (pid < 0)
    {
      error("Unable to create bedmoni daemon\n");
      exit(-1);
    }
    if (pid == 0) // all is ok, continue execution as daemon
    {
    }
  }
#endif

#ifdef UNIX
  // signals' handler overriding
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler;
  sigaction(SIGINT, &sa, 0);
  sigaction(SIGALRM, &sa, 0);
#endif
#ifdef WIN32
  signal(SIGINT, sighandler);
#endif

  sprintf(cmd, "%s/%s", USBDISK_PATH, DATA_DIR);
  if (access(cmd, R_OK) != 0)
  {
    error("bedmoni_data dir not found\n");
  }
#if 1
  mkdir(cmd, 0777);
#else
  for (i=10; i>0; i++)
  {
    if ( mkdir(cmd, 0777) == 0) break;
    usleep(200);
  }
  if (i < 0)
  {
    error("Unable to create %s folder\n", cmd);
  }
#endif

  sprintf(cmd, "%s/%s", USBDISK_PATH, DEMOFILES_DIR);
  if (access(cmd, R_OK) != 0)
  {
    error("demofiles dir not found\n");
    mkdir(cmd, 0777);
  }

  char buf[BUFSIZ], *s;
  memset(buf, 0, BUFSIZ);
  sprintf(cmd, "%s/%s", USBDISK_PATH, "sno.txt");
  if (access(cmd, R_OK) == 0)
  {
    FILE * f;
    f = fopen(cmd, "rt");
    fgets(buf, BUFSIZ, f);
    if (f) fclose(f);
  }
  else
  {
#if 0 // hangs system
    FILE * pipe;
    strcpy(cmd, "/usr/local/bin/fw_printenv serial_no");
    if ((pipe = popen(cmd, "r")) != NULL)
    {
      fgets(buf, BUFSIZ, pipe);
    }
    else
    {
      error("no serial_no in env, return uninit\n");
    }
    if (pipe) pclose(pipe);
#endif
#ifdef ARM
    FILE *f;
    strcpy(cmd, "/usr/local/bin/fw_printenv serial_no > /mnt/usbdisk/sno_output");
    system(cmd);
    f = fopen("/mnt/usbdisk/sno_output", "rt");
    if (f)
    {
      fgets(buf, BUFSIZ, f);
      fclose(f);
    }
    unlink("/mnt/usbdisk/sno_output");
#endif
  }
  s = strstr(buf, "serial_no");
  if (s)
  {
    int serial_no_temp = -1;
    s+=strlen("serial_no=");
    serial_no_temp = atoi(s);
   // debug("serial_no_temp = %d\n", serial_no_temp);
    if (serial_no_temp != -1) serial_no = serial_no_temp;
    debug("serial_no = %d\n", serial_no);
    sprintf(cmd, "%s/%s", USBDISK_PATH, "sno.txt");
    FILE * f = fopen(cmd, "wt");
    if (f)
    {
      fprintf(f, buf);
      fclose(f);
    }
  }

  stconf_init();

  for (i=0; i<sizeof(known_task_table) / sizeof(task_info_t) /*MAX_NUM_TASKS*/; i++)
  {
    if (known_task_table[i].iniproc) known_task_table[i].iniproc();
  }

  debug("init done...\n");

#ifdef UNIX
  read_hwclock_data(&ltm);
  if (ltm.tm_year + 1900 < 2000)
  {
    struct timeval tv;
    time_t t;
    ltm.tm_year = 2013 - 1900;
    t = mktime(&ltm);
    tv.tv_sec = t;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    sched_suspend();
    time_changed = 1;
   // alarm(1);

    // store to rtc onboard chip
    system("hwclock -w");
  }

  start_upd_tasks();

  if (time_changed)
    alarm(1);
  else
    alarm(60 - ltm.tm_sec + 1);  // for time update
#endif
#ifdef WIN32
#pragma comment (message: "to do: replace alarm")
#endif

  for(;!safe_exit_done;)
  {
#ifdef UNIX
    usleep(1000000);
#endif
#ifdef WIN32
    Sleep(1*1000);
#endif
  }

  volatile int execute_shutdown = 0;
  int pid1;
  pid1 = fork();
  if (pid1 < 0)
  {
    error("fork for execute_shutdown: unable to fork (%d - %s)\n", errno, strerror(errno));
   // return -1;
  }
  if (pid1 == 0)
  {
    do
    {
      crbp_resp_stat();
      usleep(100*1000);
    }
    while (execute_shutdown == 0);
    debug("execute_shutdown\n");
    exit(exit_code);
  }

  debug("deinit...\n");

#if 0
  system("/usr/bin/free");
#endif
//printf("exit code 0: %d\n", exit_code);

  crbp_resp_stat();
  crbp_resp_stat();

  for (i=0; i<sizeof(known_task_table) / sizeof(task_info_t) /*MAX_NUM_TASKS*/; i++)
  {
    if (known_task_table[i].dniproc) known_task_table[i].dniproc();
  }

  stconf_deinit();

  crbpio_init();

  crbp_resp_stat();
  crbp_resp_stat();

#ifdef ARM
  // store current version in u-boot env
  sprintf(cmd, "fw_setenv bedmoni_ver %d.%d > /dev/null", HI_VER, LO_VER);
  system(cmd);
#endif

  crbp_resp_stat();
  crbp_resp_stat();

#if defined (ARM)
  if (exit_code == EXIT_OK)
  {
    debug("umount %s\n", USBDISK_PATH);
    umount(USBDISK_PATH);

    crbp_resp_stat();
    crbp_resp_stat();

    sync();
  }
#else
  usleep(1000*1000);
#endif

  crbp_resp_stat();
  crbp_resp_stat();

  debug("exit code: %d\n", exit_code);

#ifdef UNIX
  // wait 200 ms to sync usbdisk
  for (i=0; i<10; i++)
  {
    crbp_resp_stat();
    usleep(200*1000/10);
  }
 // usleep(200*1000);
#endif

  crbp_resp_stat();
  crbp_resp_stat();

  execute_shutdown = 1;
 // usleep(200*1000);
  if (pid1>0) kill(pid1, SIGKILL);
 // usleep(200*1000);

  crbp_resp_stat();
  crbp_resp_stat();

  // send power off command
  if (exit_code == EXIT_OK)
  {
    info("power down\n");
    crbpio_send_msg(CRBP_POWEROFF_OK, 0x00, 0x00, 0x00, 0x00);
  }

  crbpio_deinit();

  exit(exit_code);
}

static int prior_low_task(void * pArg)
{
  int i;

  for(;!exit_flag;)
  {
    for (i=0; i<sizeof(known_task_table) / sizeof(task_info_t) /*MAX_NUM_TASKS*/; i++)
    {
      if (known_task_table[i].priority == PRIOR_LOW)
      {
       // int t0 = gettimemillis();
//debug("task %d\n", i);
#ifdef UNIX
        known_task_table[i].updproc();
#endif
#ifdef WIN32
        known_task_table[i].updproc(NULL);
#endif
//debug("ok\n");
       // printf("%s\t\t%d\n", known_task_table[i].name, gettimemillis()-t0);
      }
    }
#ifdef UNIX
    usleep(LP_UPD_DELAY_MS*1000);
#endif
#ifdef WIN32
    Sleep(LP_UPD_DELAY_MS);
#endif
  }
  return 0;
}

static int prior_norm_task(void * pArg)
{
  int i;

  for(;!exit_flag;)
  {
    for (i=0; i<sizeof(known_task_table) / sizeof(task_info_t) /*MAX_NUM_TASKS*/; i++)
    {
      if (known_task_table[i].priority == PRIOR_NORM)
      {
       // int t0 = gettimemillis();
//debug("task %d\n", i);
#ifdef UNIX
        known_task_table[i].updproc();
#endif
#ifdef WIN32
        known_task_table[i].updproc(NULL);
#endif
//debug("ok\n");
       // printf("%s\t\t%d\n", known_task_table[i].name, gettimemillis()-t0);
      }
    }
#ifdef UNIX
    usleep(NP_UPD_DELAY_MS*1000);
#endif
#ifdef WIN32
    Sleep(NP_UPD_DELAY_MS);
#endif

  }
  return 0;
}

static int prior_high_task(void * pArg)
{
  int i;

  for(;!exit_flag;)
  {
    for (i=0; i<sizeof(known_task_table) / sizeof(task_info_t) /*MAX_NUM_TASKS*/; i++)
    {
      if (known_task_table[i].priority == PRIOR_HIGH)
      {
       // int t0 = gettimemillis();
//debug("task %d\n", i);
#ifdef UNIX
        known_task_table[i].updproc();
#endif
#ifdef WIN32
        known_task_table[i].updproc(NULL);
#endif
//debug("ok\n");
       // printf("%s\t\t%d\n", known_task_table[i].name, gettimemillis()-t0);
      }
    }

#ifdef UNIX
    usleep(HP_UPD_DELAY_MS*1000);
#endif
#ifdef WIN32
    Sleep(HP_UPD_DELAY_MS);
#endif
  }
  return 0;
}


